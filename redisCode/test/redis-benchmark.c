/* Redis benchmark utility.
 *
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "fmacros.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <assert.h>

#include "ae.h"
#include "hiredis.h"
#include "sds.h"
#include "adlist.h"
#include "zmalloc.h"

#define REDIS_NOTUSED(V) ((void) V)
#define RANDPTR_INITIAL_SIZE 8

/* config������Ϣ�ṹ�壬static��̬������ʹ��ȫ��ֻ��һ�� */
static struct config {
	//��Ϣ�¼�
    aeEventLoop *el;
    const char *hostip;
    int hostport;
    //�ݴ��ж��Ƿ��Ǳ��ز���
    const char *hostsocket;
    //Client������
    int numclients;
    int liveclients;
    //���������
    int requests;
    int requests_issued;
    //������ɵ�����
    int requests_finished;
    int keysize;
    int datasize;
    int randomkeys;
    int randomkeys_keyspacelen;
    int keepalive;
    int pipeline;
    long long start;
    long long totlatency;
    long long *latency;
    const char *title;
    //Client�б����������ᾭ������
    list *clients;
    int quiet;
    int csv;
    //�ж��Ƿ�loopѭ������
    int loop;
    int idlemode;
    int dbnum;
    sds dbnumstr;
    char *tests;
    char *auth;
} config;

typedef struct _client {
	//redis������
    redisContext *context;
    //�˻����������ں���Ķ�дhandler
    sds obuf;
    //randָ������
    char **randptr;         /* Pointers to :rand: strings inside the command buf */
    //randptr��ָ�����
    size_t randlen;         /* Number of pointers in client->randptr */
    //randptr��û�б�ʹ�õ�ָ�����
    size_t randfree;        /* Number of unused pointers in client->randptr */
    unsigned int written;   /* Bytes of 'obuf' already written */
    //����ķ���ʱ��
    long long start;        /* Start time of a request */
    //�������ʱ
    long long latency;      /* Request latency */
    //����ĵȴ�����
    int pending;            /* Number of pending requests (replies to consume) */
    int selectlen;  /* If non-zero, a SELECT of 'selectlen' bytes is currently
                       used as a prefix of the pipline of commands. This gets
                       discarded the first time it's sent. */
} *client;

/* Prototypes */
/* ����ԭ�� */
static void createMissingClients(client c); /* ����û��Command����Ҫ���clint */
static long long ustime(void) /* ���ص���ʱ��ĵ�λΪ΢��ĸ�ʽ */
static long long mstime(void) /* ���ص���ʱ��ĵ�λΪ����ĸ�ʽ */
static void freeClient(client c) /* �ͷ�Client */
static void freeAllClients(void) /* �ͷ����е�Client */
static void resetClient(client c) /* ����Client */
static void randomizeClientKey(client c) /* ������client���randptr�е�keyֵ */
static void clientDone(client c) /* Client��ɺ�ĵ��÷��� */
static void readHandler(aeEventLoop *el, int fd, void *privdata, int mask) /* ���¼��Ĵ����� */
static void writeHandler(aeEventLoop *el, int fd, void *privdata, int mask) /* д�¼��������� */
static client createClient(char *cmd, size_t len, client from) /* ����һ����׼��Client */
static int compareLatency(const void *a, const void *b) /* �Ƚ���ʱ */
static void showLatencyReport(void) /* ���������ʱ */
static void benchmark(char *title, char *cmd, int len) /* ��ָ����CMD���������ܲ��� */
int parseOptions(int argc, const char **argv) /* ���ݶ���Ĳ���������config�����ļ� */
int showThroughput(struct aeEventLoop *eventLoop, long long id, void *clientData) /* ��ʾRequestִ�е��ٶȣ����RPS */
int test_is_selected(char *name) /* ���config�е������Ƿ�ѡ�� */
	
/* ����һ��struct timeval�ṹ��
struct timeval�ṹ����time.h�еĶ���Ϊ��
struct timeval
{
  __time_t tv_sec;        // Seconds. 
  __suseconds_t tv_usec;    // Microseconds. (΢��)
}; */
	
/* Implementation */
/* ���ص���ʱ��ĵ�λΪ΢��ĸ�ʽ */
static long long ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}

/* ���ص���ʱ��ĵ�λΪ����ĸ�ʽ */
static long long mstime(void) {
    struct timeval tv;
    long long mst;

    gettimeofday(&tv, NULL);
    mst = ((long long)tv.tv_sec)*1000;
    mst += tv.tv_usec/1000;
    return mst;
}

/* �ͷ�Client */
static void freeClient(client c) {
    listNode *ln;
    //ɾ���ļ��Ķ�д�¼�
    aeDeleteFileEvent(config.el,c->context->fd,AE_WRITABLE);
    aeDeleteFileEvent(config.el,c->context->fd,AE_READABLE);
    //�ͷ�Client����ռ�Ŀռ�
    redisFree(c->context);
    sdsfree(c->obuf);
    zfree(c->randptr);
    zfree(c);
    //�������ýṹ����liveclients���ڵ�Client������
    config.liveclients--;
    //Ѱ��c��Ӧ�Ľ�㣬��config�е�Clients��ɾ���˽��
    ln = listSearchKey(config.clients,c);
    assert(ln != NULL);
    listDelNode(config.clients,ln);
}

/* �ͷ����е�Client */
static void freeAllClients(void) {
    listNode *ln = config.clients->head, *next;

    while(ln) {
    	//����ѭ���������ͷſռ�
        next = ln->next;
        freeClient(ln->value);
        ln = next;
    }
}

/* ����Client�ͻ��� */
static void resetClient(client c) {
	//ɾ��д�¼�
    aeDeleteFileEvent(config.el,c->context->fd,AE_WRITABLE);
    //ɾ�����¼�
    aeDeleteFileEvent(config.el,c->context->fd,AE_READABLE);
    //�������¼������Ұ�д�Ĵ����¼������������������洫�룬�������˺�����̵�˼��
    aeCreateFileEvent(config.el,c->context->fd,AE_WRITABLE,writeHandler,c);
    c->written = 0;
    //pending����˼�ǿͻ��˴�������������
    c->pending = config.pipeline;
}

/* ������client���randptr�е�keyֵ */
static void randomizeClientKey(client c) {
    size_t i;

    for (i = 0; i < c->randlen; i++) {
        char *p = c->randptr[i]+11;
        size_t r = random() % config.randomkeys_keyspacelen;
        size_t j;

        for (j = 0; j < 12; j++) {
            *p = '0'+r%10;
            r/=10;
            p--;
        }
    }
}

/* Client��ɺ�ĵ��÷��� */
static void clientDone(client c) {
    if (config.requests_finished == config.requests) {
    	//�������ȫ����ɣ��ͷſͻ���
        freeClient(c);
        aeStop(config.el);
        return;
    }
    if (config.keepalive) {
    	//��������ļ�������Ҫ����keepalive״̬�����ͷſͻ��ˣ�ֱ������
        resetClient(c);
    } else {
    	//����������û�Ծ��Client����1
        config.liveclients--;
        createMissingClients(c);
        config.liveclients++;
        freeClient(c);
    }
}

/* ���¼��Ĵ����� */
static void readHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
    client c = privdata;
    void *reply = NULL;
    REDIS_NOTUSED(el);
    REDIS_NOTUSED(fd);
    REDIS_NOTUSED(mask);

    /* Calculate latency only for the first read event. This means that the
     * server already sent the reply and we need to parse it. Parsing overhead
     * is not part of the latency, so calculate it only once, here. */
    //������ʱ��Ȼ��Ƚ���ʱ��ȡ�õ�һ��read ��event�¼�
    if (c->latency < 0) c->latency = ustime()-(c->start);

    if (redisBufferRead(c->context) != REDIS_OK) {
    	//�����ж��ܷ��
        fprintf(stderr,"Error: %s\n",c->context->errstr);
        exit(1);
    } else {
        while(c->pending) {
            if (redisGetReply(c->context,&reply) != REDIS_OK) {
                fprintf(stderr,"Error: %s\n",c->context->errstr);
                exit(1);
            }
            if (reply != NULL) {
            	//��ȡreply�ظ�������������Ҳ��ֱ���˳�
                if (reply == (void*)REDIS_REPLY_ERROR) {
                    fprintf(stderr,"Unexpected error reply, exiting...\n");
                    exit(1);
                }

                freeReplyObject(reply);
					
                if (c->selectlen) {
                    size_t j;

                    /* This is the OK from SELECT. Just discard the SELECT
                     * from the buffer. */
                    //ִ�е���������Ѿ�ִ�гɹ����ȴ�����������1
                    c->pending--;
                    sdsrange(c->obuf,c->selectlen,-1);
                    /* We also need to fix the pointers to the strings
                     * we need to randomize. */
                    for (j = 0; j < c->randlen; j++)
                        c->randptr[j] -= c->selectlen;
                    c->selectlen = 0;
                    continue;
                }

                if (config.requests_finished < config.requests)
                    config.latency[config.requests_finished++] = c->latency;
                c->pending--;
                if (c->pending == 0) {
                	//���ÿͻ���done��ɺ�ķ���
                    clientDone(c);
                    break;
                }
            } else {
                break;
            }
        }
    }
}

/* д�¼��Ĵ����� */
static void writeHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
    client c = privdata;
    REDIS_NOTUSED(el);
    REDIS_NOTUSED(fd);
    REDIS_NOTUSED(mask);

    /* Initialize request when nothing was written. */
    if (c->written == 0) {
        /* Enforce upper bound to number of requests. */
        if (config.requests_issued++ >= config.requests) {
            freeClient(c);
            return;
        }

        /* Really initialize: randomize keys and set start time. */
        if (config.randomkeys) randomizeClientKey(c);
        c->start = ustime();
        c->latency = -1;
    }

    if (sdslen(c->obuf) > c->written) {
        void *ptr = c->obuf+c->written;
        //����д�¼��Ĺؼ�
        int nwritten = write(c->context->fd,ptr,sdslen(c->obuf)-c->written);
        if (nwritten == -1) {
            if (errno != EPIPE)
                fprintf(stderr, "Writing to socket: %s\n", strerror(errno));
            freeClient(c);
            return;
        }
        c->written += nwritten;
        if (sdslen(c->obuf) == c->written) {
        	//д�¼���ɣ�ɾ��д�¼�
            aeDeleteFileEvent(config.el,c->context->fd,AE_WRITABLE);
            //�������¼�
            aeCreateFileEvent(config.el,c->context->fd,AE_READABLE,readHandler,c);
        }
    }
}

/* Create a benchmark client, configured to send the command passed as 'cmd' of
 * 'len' bytes.
 *
 * The command is copied N times in the client output buffer (that is reused
 * again and again to send the request to the server) accordingly to the configured
 * pipeline size.
 *
 * Also an initial SELECT command is prepended in order to make sure the right
 * database is selected, if needed. The initial SELECT will be discarded as soon
 * as the first reply is received.
 *
 * To create a client from scratch, the 'from' pointer is set to NULL. If instead
 * we want to create a client using another client as reference, the 'from' pointer
 * points to the client to use as reference. In such a case the following
 * information is take from the 'from' client:
 *
 * 1) The command line to use.
 * 2) The offsets of the __rand_int__ elements inside the command line, used
 *    for arguments randomization.
 *
 * Even when cloning another client, the SELECT command is automatically prefixed
 * if needed. */
/* ����һ����׼��Client */
static client createClient(char *cmd, size_t len, client from) {
    int j;
    client c = zmalloc(sizeof(struct _client));

    if (config.hostsocket == NULL) {
    	//ָ����ip��ַ���˿ںŵĲ���
        c->context = redisConnectNonBlock(config.hostip,config.hostport);
    } else {
    	//����host����ʱ
        c->context = redisConnectUnixNonBlock(config.hostsocket);
    }
    
    //�ж������Ƿ�ɹ�
    if (c->context->err) {
        fprintf(stderr,"Could not connect to Redis at ");
        if (config.hostsocket == NULL)
            fprintf(stderr,"%s:%d: %s\n",config.hostip,config.hostport,c->context->errstr);
        else
            fprintf(stderr,"%s: %s\n",config.hostsocket,c->context->errstr);
        exit(1);
    }
    /* Suppress hiredis cleanup of unused buffers for max speed. */
    c->context->reader->maxbuf = 0;

    /* Build the request buffer:
     * Queue N requests accordingly to the pipeline size, or simply clone
     * the example client buffer. */
    c->obuf = sdsempty();

    if (config.auth) {
        char *buf = NULL;
        int len = redisFormatCommand(&buf, "AUTH %s", config.auth);
        c->obuf = sdscatlen(c->obuf, buf, len);
        free(buf);
    }

    /* If a DB number different than zero is selected, prefix our request
     * buffer with the SELECT command, that will be discarded the first
     * time the replies are received, so if the client is reused the
     * SELECT command will not be used again. */
    if (config.dbnum != 0) {
        c->obuf = sdscatprintf(c->obuf,"*2\r\n$6\r\nSELECT\r\n$%d\r\n%s\r\n",
            (int)sdslen(config.dbnumstr),config.dbnumstr);
        c->selectlen = sdslen(c->obuf);
    } else {
        c->selectlen = 0;
    }

    /* Append the request itself. */
    if (from) {
        c->obuf = sdscatlen(c->obuf,
            from->obuf+from->selectlen,
            sdslen(from->obuf)-from->selectlen);
    } else {
        for (j = 0; j < config.pipeline; j++)
            c->obuf = sdscatlen(c->obuf,cmd,len);
    }

    c->written = 0;
    c->pending = config.pipeline;
    c->randptr = NULL;
    c->randlen = 0;
    if (c->selectlen) c->pending++;

    /* Find substrings in the output buffer that need to be randomized. */
    //��from��һЩ���Ը�ֵ���´�����client
    if (config.randomkeys) {
        if (from) {
            c->randlen = from->randlen;
            c->randfree = 0;
            c->randptr = zmalloc(sizeof(char*)*c->randlen);
            /* copy the offsets. */
            for (j = 0; j < (int)c->randlen; j++) {
                c->randptr[j] = c->obuf + (from->randptr[j]-from->obuf);
                /* Adjust for the different select prefix length. */
                c->randptr[j] += c->selectlen - from->selectlen;
            }
        } else {
            char *p = c->obuf;

            c->randlen = 0;
            c->randfree = RANDPTR_INITIAL_SIZE;
            c->randptr = zmalloc(sizeof(char*)*c->randfree);
            while ((p = strstr(p,"__rand_int__")) != NULL) {
                if (c->randfree == 0) {
                    c->randptr = zrealloc(c->randptr,sizeof(char*)*c->randlen*2);
                    c->randfree += c->randlen;
                }
                c->randptr[c->randlen++] = p;
                c->randfree--;
                p += 12; /* 12 is strlen("__rand_int__). */
            }
        }
    }
    
    //�ոմ�����ͻ��˺󣬻�����֧�ֶ��¼�������ֻ������д�¼�
    aeCreateFileEvent(config.el,c->context->fd,AE_WRITABLE,writeHandler,c);
    listAddNodeTail(config.clients,c);
    config.liveclients++;
    return c;
}

/* ����û��Command����Ҫ���clint */
static void createMissingClients(client c) {
    int n = 0;

    while(config.liveclients < config.numclients) {
    	//��һ��ʱ�䣬���ӻ�Ծ��Client����
        createClient(NULL,0,c);

        /* Listen backlog is quite limited on most systems */
        if (++n > 64) {
            usleep(50000);
            n = 0;
        }
    }
}

/* �Ƚ���ʱ */
static int compareLatency(const void *a, const void *b) {
    return (*(long long*)a)-(*(long long*)b);
}

/* ���������ʱ */
static void showLatencyReport(void) {
    int i, curlat = 0;
    float perc, reqpersec;

    reqpersec = (float)config.requests_finished/((float)config.totlatency/1000);
    if (!config.quiet && !config.csv) {
        printf("====== %s ======\n", config.title);
        printf("  %d requests completed in %.2f seconds\n", config.requests_finished,
            (float)config.totlatency/1000);
        printf("  %d parallel clients\n", config.numclients);
        printf("  %d bytes payload\n", config.datasize);
        printf("  keep alive: %d\n", config.keepalive);
        printf("\n");

		//��������ʱ����
        qsort(config.latency,config.requests,sizeof(long long),compareLatency);
        for (i = 0; i < config.requests; i++) {
            if (config.latency[i]/1000 != curlat || i == (config.requests-1)) {
                curlat = config.latency[i]/1000;
                perc = ((float)(i+1)*100)/config.requests;
                printf("%.2f%% <= %d milliseconds\n", perc, curlat);
            }
        }
        printf("%.2f requests per second\n\n", reqpersec);
    } else if (config.csv) {
        printf("\"%s\",\"%.2f\"\n", config.title, reqpersec);
    } else {
        printf("%s: %.2f requests per second\n", config.title, reqpersec);
    }
}

/* ��ָ����CMD���������ܲ��� */
static void benchmark(char *title, char *cmd, int len) {
    client c;

    config.title = title;
    config.requests_issued = 0;
    config.requests_finished = 0;

    c = createClient(cmd,len,NULL);
    createMissingClients(c);

    config.start = mstime();
    aeMain(config.el);
    //���ͨ����������ʱ����ʾ��ʱ���棬�������ܲ��ԵĽ��
    config.totlatency = mstime()-config.start;

    showLatencyReport();
    freeAllClients();
}

/* Returns number of consumed options. */
/* ���ݶ���Ĳ���������config�����ļ� */
int parseOptions(int argc, const char **argv) {
    int i;
    int lastarg;
    int exit_status = 1;

    for (i = 1; i < argc; i++) {
        lastarg = (i == (argc-1));
		
		//ͨ������if�жϣ������˸о������������ȶ����Բ�
        if (!strcmp(argv[i],"-c")) {
            if (lastarg) goto invalid;
            config.numclients = atoi(argv[++i]);
        } else if (!strcmp(argv[i],"-n")) {
            if (lastarg) goto invalid;
            config.requests = atoi(argv[++i]);
        } else if (!strcmp(argv[i],"-k")) {
            if (lastarg) goto invalid;
            config.keepalive = atoi(argv[++i]);
        } else if (!strcmp(argv[i],"-h")) {
            if (lastarg) goto invalid;
            config.hostip = strdup(argv[++i]);
        } else if (!strcmp(argv[i],"-p")) {
            if (lastarg) goto invalid;
            config.hostport = atoi(argv[++i]);
        } else if (!strcmp(argv[i],"-s")) {
            if (lastarg) goto invalid;
            config.hostsocket = strdup(argv[++i]);
        } else if (!strcmp(argv[i],"-a") ) {
            if (lastarg) goto invalid;
            config.auth = strdup(argv[++i]);
        } else if (!strcmp(argv[i],"-d")) {
            if (lastarg) goto invalid;
            config.datasize = atoi(argv[++i]);
            if (config.datasize < 1) config.datasize=1;
            if (config.datasize > 1024*1024*1024) config.datasize = 1024*1024*1024;
        } else if (!strcmp(argv[i],"-P")) {
            if (lastarg) goto invalid;
            config.pipeline = atoi(argv[++i]);
            if (config.pipeline <= 0) config.pipeline=1;
        } else if (!strcmp(argv[i],"-r")) {
            if (lastarg) goto invalid;
            config.randomkeys = 1;
            config.randomkeys_keyspacelen = atoi(argv[++i]);
            if (config.randomkeys_keyspacelen < 0)
                config.randomkeys_keyspacelen = 0;
        } else if (!strcmp(argv[i],"-q")) {
            config.quiet = 1;
        } else if (!strcmp(argv[i],"--csv")) {
            config.csv = 1;
        } else if (!strcmp(argv[i],"-l")) {
            config.loop = 1;
        } else if (!strcmp(argv[i],"-I")) {
            config.idlemode = 1;
        } else if (!strcmp(argv[i],"-t")) {
            if (lastarg) goto invalid;
            /* We get the list of tests to run as a string in the form
             * get,set,lrange,...,test_N. Then we add a comma before and
             * after the string in order to make sure that searching
             * for ",testname," will always get a match if the test is
             * enabled. */
            config.tests = sdsnew(",");
            config.tests = sdscat(config.tests,(char*)argv[++i]);
            config.tests = sdscat(config.tests,",");
            sdstolower(config.tests);
        } else if (!strcmp(argv[i],"--dbnum")) {
            if (lastarg) goto invalid;
            config.dbnum = atoi(argv[++i]);
            config.dbnumstr = sdsfromlonglong(config.dbnum);
        } else if (!strcmp(argv[i],"--help")) {
            exit_status = 0;
            goto usage;
        } else {
            /* Assume the user meant to provide an option when the arg starts
             * with a dash. We're done otherwise and should use the remainder
             * as the command and arguments for running the benchmark. */
            if (argv[i][0] == '-') goto invalid;
            return i;
        }
    }

    return i;

invalid:
	//������Чֱ������goto��䴦����
    printf("Invalid option \"%s\" or option argument missing\n\n",argv[i]);

usage:
	//goto����ʾ��Ϣ�����
    printf(
"Usage: redis-benchmark [-h <host>] [-p <port>] [-c <clients>] [-n <requests]> [-k <boolean>]\n\n"
" -h <hostname>      Server hostname (default 127.0.0.1)\n"
" -p <port>          Server port (default 6379)\n"
" -s <socket>        Server socket (overrides host and port)\n"
" -a <password>      Password for Redis Auth\n"
" -c <clients>       Number of parallel connections (default 50)\n"
" -n <requests>      Total number of requests (default 10000)\n"
" -d <size>          Data size of SET/GET value in bytes (default 2)\n"
" -dbnum <db>        SELECT the specified db number (default 0)\n"
" -k <boolean>       1=keep alive 0=reconnect (default 1)\n"
" -r <keyspacelen>   Use random keys for SET/GET/INCR, random values for SADD\n"
"  Using this option the benchmark will expand the string __rand_int__\n"
"  inside an argument with a 12 digits number in the specified range\n"
"  from 0 to keyspacelen-1. The substitution changes every time a command\n"
"  is executed. Default tests use this to hit random keys in the\n"
"  specified range.\n"
" -P <numreq>        Pipeline <numreq> requests. Default 1 (no pipeline).\n"
" -q                 Quiet. Just show query/sec values\n"
" --csv              Output in CSV format\n"
" -l                 Loop. Run the tests forever\n"
" -t <tests>         Only run the comma separated list of tests. The test\n"
"                    names are the same as the ones produced as output.\n"
" -I                 Idle mode. Just open N idle connections and wait.\n\n"
"Examples:\n\n"
" Run the benchmark with the default configuration against 127.0.0.1:6379:\n"
"   $ redis-benchmark\n\n"
" Use 20 parallel clients, for a total of 100k requests, against 192.168.1.1:\n"
"   $ redis-benchmark -h 192.168.1.1 -p 6379 -n 100000 -c 20\n\n"
" Fill 127.0.0.1:6379 with about 1 million keys only using the SET test:\n"
"   $ redis-benchmark -t set -n 1000000 -r 100000000\n\n"
" Benchmark 127.0.0.1:6379 for a few commands producing CSV output:\n"
"   $ redis-benchmark -t ping,set,get -n 100000 --csv\n\n"
" Benchmark a specific command line:\n"
"   $ redis-benchmark -r 10000 -n 10000 eval 'return redis.call(\"ping\")' 0\n\n"
" Fill a list with 10000 random elements:\n"
"   $ redis-benchmark -r 10000 -n 10000 lpush mylist __rand_int__\n\n"
" On user specified command lines __rand_int__ is replaced with a random integer\n"
" with a range of values selected by the -r option.\n"
    );
    exit(exit_status);
}

/* ��ʾRequestִ�е��ٶȣ����RPS */
int showThroughput(struct aeEventLoop *eventLoop, long long id, void *clientData) {
    REDIS_NOTUSED(eventLoop);
    REDIS_NOTUSED(id);
    REDIS_NOTUSED(clientData);

    if (config.liveclients == 0) {
        fprintf(stderr,"All clients disconnected... aborting.\n");
        exit(1);
    }

    if (config.csv) return 250;
    //�����ʼ�����ڵ�ʱ��
    float dt = (float)(mstime()-config.start)/1000.0;
    float rps = (float)config.requests_finished/dt;
    printf("%s: %.2f\r", config.title, rps);
    fflush(stdout);
    return 250; /* every 250ms */
}

/* Return true if the named test was selected using the -t command line
 * switch, or if all the tests are selected (no -t passed by user). */
/* ���config�е������Ƿ�ѡ�� */
int test_is_selected(char *name) {
    char buf[256];
    int l = strlen(name);

    if (config.tests == NULL) return 1;
    buf[0] = ',';
    memcpy(buf+1,name,l);
    buf[l+1] = ',';
    buf[l+2] = '\0';
    return strstr(config.tests,buf) != NULL;
}

/* ���Գ��� */
int main(int argc, const char **argv) {
    int i;
    char *data, *cmd;
    int len;

    client c;

    srandom(time(NULL));
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
	
	//��config��Ĭ������
    config.numclients = 50;
    config.requests = 10000;
    config.liveclients = 0;
    config.el = aeCreateEventLoop(1024*10);
    aeCreateTimeEvent(config.el,1,showThroughput,NULL,NULL);
    config.keepalive = 1;
    config.datasize = 3;
    config.pipeline = 1;
    config.randomkeys = 0;
    config.randomkeys_keyspacelen = 0;
    config.quiet = 0;
    config.csv = 0;
    config.loop = 0;
    config.idlemode = 0;
    config.latency = NULL;
    config.clients = listCreate();
    config.hostip = "127.0.0.1";
    config.hostport = 6379;
    config.hostsocket = NULL;
    config.tests = NULL;
    config.dbnum = 0;
    config.auth = NULL;
	
	//��������Ĳ�������
    i = parseOptions(argc,argv);
    argc -= i;
    argv += i;

    config.latency = zmalloc(sizeof(long long)*config.requests);

    if (config.keepalive == 0) {
        printf("WARNING: keepalive disabled, you probably need 'echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse' for Linux and 'sudo sysctl -w net.inet.tcp.msl=1000' for Mac OS X in order to use a lot of clients/requests\n");
    }

    if (config.idlemode) {
        printf("Creating %d idle connections and waiting forever (Ctrl+C when done)\n", config.numclients);
        c = createClient("",0,NULL); /* will never receive a reply */
        createMissingClients(c);
        aeMain(config.el);
        /* and will wait for every */
    }

    /* Run benchmark with command in the remainder of the arguments. */
    if (argc) {
        sds title = sdsnew(argv[0]);
        for (i = 1; i < argc; i++) {
            title = sdscatlen(title, " ", 1);
            title = sdscatlen(title, (char*)argv[i], strlen(argv[i]));
        }

        do {
            len = redisFormatCommandArgv(&cmd,argc,argv,NULL);
            //����cmd����������в���
            benchmark(title,cmd,len);
            free(cmd);
        } while(config.loop);

        return 0;
    }

    /* Run default benchmark suite. */
    do {
        data = zmalloc(config.datasize+1);
        memset(data,'x',config.datasize);
        data[config.datasize] = '\0';
		
		//������һЩ���õ���������ܲ��Բ���
        if (test_is_selected("ping_inline") || test_is_selected("ping"))
            benchmark("PING_INLINE","PING\r\n",6);

        if (test_is_selected("ping_mbulk") || test_is_selected("ping")) {
            len = redisFormatCommand(&cmd,"PING");
            benchmark("PING_BULK",cmd,len);
            free(cmd);
        }

        if (test_is_selected("set")) {
            len = redisFormatCommand(&cmd,"SET key:__rand_int__ %s",data);
            benchmark("SET",cmd,len);
            free(cmd);
        }

        if (test_is_selected("get")) {
            len = redisFormatCommand(&cmd,"GET key:__rand_int__");
            benchmark("GET",cmd,len);
            free(cmd);
        }

        if (test_is_selected("incr")) {
            len = redisFormatCommand(&cmd,"INCR counter:__rand_int__");
            benchmark("INCR",cmd,len);
            free(cmd);
        }

        if (test_is_selected("lpush")) {
            len = redisFormatCommand(&cmd,"LPUSH mylist %s",data);
            benchmark("LPUSH",cmd,len);
            free(cmd);
        }

        if (test_is_selected("lpop")) {
            len = redisFormatCommand(&cmd,"LPOP mylist");
            benchmark("LPOP",cmd,len);
            free(cmd);
        }

        if (test_is_selected("sadd")) {
            len = redisFormatCommand(&cmd,
                "SADD myset element:__rand_int__");
            benchmark("SADD",cmd,len);
            free(cmd);
        }

        if (test_is_selected("spop")) {
            len = redisFormatCommand(&cmd,"SPOP myset");
            benchmark("SPOP",cmd,len);
            free(cmd);
        }

        if (test_is_selected("lrange") ||
            test_is_selected("lrange_100") ||
            test_is_selected("lrange_300") ||
            test_is_selected("lrange_500") ||
            test_is_selected("lrange_600"))
        {
            len = redisFormatCommand(&cmd,"LPUSH mylist %s",data);
            benchmark("LPUSH (needed to benchmark LRANGE)",cmd,len);
            free(cmd);
        }

        if (test_is_selected("lrange") || test_is_selected("lrange_100")) {
            len = redisFormatCommand(&cmd,"LRANGE mylist 0 99");
            benchmark("LRANGE_100 (first 100 elements)",cmd,len);
            free(cmd);
        }

        if (test_is_selected("lrange") || test_is_selected("lrange_300")) {
            len = redisFormatCommand(&cmd,"LRANGE mylist 0 299");
            benchmark("LRANGE_300 (first 300 elements)",cmd,len);
            free(cmd);
        }

        if (test_is_selected("lrange") || test_is_selected("lrange_500")) {
            len = redisFormatCommand(&cmd,"LRANGE mylist 0 449");
            benchmark("LRANGE_500 (first 450 elements)",cmd,len);
            free(cmd);
        }

        if (test_is_selected("lrange") || test_is_selected("lrange_600")) {
            len = redisFormatCommand(&cmd,"LRANGE mylist 0 599");
            benchmark("LRANGE_600 (first 600 elements)",cmd,len);
            free(cmd);
        }

        if (test_is_selected("mset")) {
            const char *argv[21];
            argv[0] = "MSET";
            for (i = 1; i < 21; i += 2) {
                argv[i] = "key:__rand_int__";
                argv[i+1] = data;
            }
            len = redisFormatCommandArgv(&cmd,21,argv,NULL);
            benchmark("MSET (10 keys)",cmd,len);
            free(cmd);
        }

        if (!config.csv) printf("\n");
    } while(config.loop);

    return 0;
}
