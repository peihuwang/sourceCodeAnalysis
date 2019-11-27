/* A simple event-driven programming library. Originally I wrote this code
 * for the Jim's event-loop (Jim is a Tcl interpreter) but later translated
 * it in form of a library for easy reuse.
 *
 * ae������д��һ���򵥵��¼������⣬���������ת������ø�Ϊ�򵥵ĸ���
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __AE_H__
#define __AE_H__

#define AE_OK 0
#define AE_ERR -1

#define AE_NONE 0
#define AE_READABLE 1
#define AE_WRITABLE 2

#define AE_FILE_EVENTS 1
#define AE_TIME_EVENTS 2
#define AE_ALL_EVENTS (AE_FILE_EVENTS|AE_TIME_EVENTS)
#define AE_DONT_WAIT 4

#define AE_NOMORE -1

/* Macros */
#define AE_NOTUSED(V) ((void) V)

struct aeEventLoop;

/* Types and data structures */
/* ������һЩ���� */
typedef void aeFileProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);
typedef int aeTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData);
typedef void aeEventFinalizerProc(struct aeEventLoop *eventLoop, void *clientData);
typedef void aeBeforeSleepProc(struct aeEventLoop *eventLoop);

/* File event structure */
/* �ļ��¼��ṹ�� */
typedef struct aeFileEvent {
	//ֻΪ���¼�����д�¼��е�1��
    int mask; /* one of AE_(READABLE|WRITABLE) */
    //������
    aeFileProc *rfileProc;
    //д����
    aeFileProc *wfileProc;
    //�ͻ�������
    void *clientData;
} aeFileEvent;

/* Time event structure */
/* ʱ���¼��ṹ�� */
typedef struct aeTimeEvent {
	//ʱ���¼�id
    long long id; /* time event identifier. */
    //ʱ������
    long when_sec; /* seconds */
    //ʱ�����
    long when_ms; /* milliseconds */
    //ʱ���¼��еĴ�����
    aeTimeProc *timeProc;
    //��ɾ����ʱ�򽫻���õķ���
    aeEventFinalizerProc *finalizerProc;
    //�ͻ�������
    void *clientData;
    //ʱ��ṹ���ڵ���һ���ṹ��
    struct aeTimeEvent *next;
} aeTimeEvent;

/* A fired event */
/* fired�ṹ�壬������ʾ��Ҫ��������ļ��¼� */
typedef struct aeFiredEvent {
	//�ļ�������id
    int fd;
    int mask;
} aeFiredEvent;

/* State of an event based program */
typedef struct aeEventLoop {
	//Ŀǰ��������ߵ��ļ�������
    int maxfd;   /* highest file descriptor currently registered */
    int setsize; /* max number of file descriptors tracked */
    //��һ��ʱ���¼�id
    long long timeEventNextId;
    time_t lastTime;     /* Used to detect system clock skew */
    //3���¼�����
    aeFileEvent *events; /* Registered events */
    aeFiredEvent *fired; /* Fired events */
    aeTimeEvent *timeEventHead;
    //�¼�ֹͣ��־��
    int stop;
    //�����ŵ���event API�����ݣ�����epoll��select���¼�
    void *apidata; /* This is used for polling API specific data */
    aeBeforeSleepProc *beforesleep;
} aeEventLoop;

/*
	 epoll��Linux(�ں˰汾2.6������֧��)�¶�·����IO�ӿ�select/poll����ǿ�汾��
	 ����������߳����ڴ�������������ֻ��������Ծ������µ�ϵͳCPU�����ʣ�
	 ��Ϊ���Ḵ���ļ����������������ݽ����������ʹ������ÿ�εȴ��¼�֮ǰ����
	 ������׼��Ҫ���������ļ����������ϣ���һ��ԭ����ǻ�ȡ�¼���ʱ��������
	 ��������������������������ֻҪ������Щ���ں�IO�¼��첽���Ѷ�����Ready��
	 �е����������Ͼ����ˡ�

    Linux��select ģ�ͺ�epollģ������
    �������ڴ�ѧ���飬ס������¥�кܶ�䷿�䣬�������Ҫ�����㡣 select����
    �ܴ���ͻ����������Ѱ�������ȥ�ң�ֱ���ҵ���Ϊֹ���� epoll���޹ܴ����
    �ȼ���ÿλͬѧ�ķ���ţ����������ʱ��ֻ��������������ס���ĸ����伴��
    ���������Դ��������������¥���ˡ�������� 10000���ˣ���Ҫ���Լ�ס�ⶰ¥
    ��ͬѧʱ�� select���epoll ���޹ܴ��裬˭��Ч�ʸ��ߣ�����������ͬ���ڸ�
    �����������У���ѯ I/O�����ʱ��Ĳ���֮һ�� select��epoll ������˭������
    ���ߣ�ͬ��ʮ�����ˡ�*/
    /* select�¼���ģ�� */
   /*��1����������ע���¼������������ϣ�fd_set)������һ����������
   		���Թ�ע������Ķ�(read)��д(write)���쳣(exception)�¼���
   		����ͨ����Ҫ��������fd_set�� һ�������ռ���ע���¼�������
   		����һ�������ռ���עд�¼���������������һ�������ռ���ע
   		�쳣�¼������������ϡ�
   	��2����ѯ����fd_set�е�ÿһ��fd ������Ƿ�����Ӧ���¼�����������У��ͽ��д���*/
   	
   	/* poll������������ǿ��Ը����ļ��������������һ���ļ���Ҫ��ѯ3���ļ����������ϣ���epollֻ��Ҫһ����Ч�ʸ��� */
   	/* epoll��poll�������汾�����������б����ںˣ�һ�����¼��������ں˰ѷ����¼����������б�֪ͨ�����̣������ͱ�
   		������ѯ�����������б�Ч�ʼ������ */	
   	/*kqueue �� FreeBSD �ϵ�һ�ֵĶ�·���û���
   	   kqueue����Դ�ͳ�� select/poll ����������ļ����������ܽϵ�Ч�����������ġ�ע��һ���������� kqueue �Ժ󣬵�����
   		��������״̬�����仯ʱ��kqueue ��һ����֪ͨӦ�ó�����Щ�������ɶ�����д������ˡ�*/
	/* evport��ĳһ��������ض� event �� Event port ����� */
/* Prototypes */
aeEventLoop *aeCreateEventLoop(int setsize); /* ����aeEventLoop���ڲ���fileEvent��Fired�¼��ĸ���ΪsetSize�� */
void aeDeleteEventLoop(aeEventLoop *eventLoop); /* ɾ��EventLoop���ͷ���Ӧ���¼���ռ�Ŀռ� */
void aeStop(aeEventLoop *eventLoop); /* ����eventLoop�е�ֹͣ����Ϊ1 */
int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask,
        aeFileProc *proc, void *clientData); /* ��eventLoop�д����ļ��¼� */
void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask); /* ɾ���ļ��¼� */
int aeGetFileEvents(aeEventLoop *eventLoop, int fd); //�����ļ�������id���ҳ��ļ������ԣ��Ƕ��¼�����д�¼�
long long aeCreateTimeEvent(aeEventLoop *eventLoop, long long milliseconds,
        aeTimeProc *proc, void *clientData,
        aeEventFinalizerProc *finalizerProc); /* ��eventLoop�����ʱ���¼���������ʱ��Ϊ��ǰʱ������Լ������ʱ�� */
int aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id); //����ʱ��id��ɾ��ʱ���¼����漰����Ĳ���
int aeProcessEvents(aeEventLoop *eventLoop, int flags); /* ����eventLoop�е����������¼� */
int aeWait(int fd, int mask, long long milliseconds); /* ��ĳ�¼��ȴ� */
void aeMain(aeEventLoop *eventLoop); /* ae�¼�ִ�������� */
char *aeGetApiName(void);
void aeSetBeforeSleepProc(aeEventLoop *eventLoop, aeBeforeSleepProc *beforesleep); /* ÿ��eventLoop�¼�ִ����������¿�ʼִ��ʱ���� */
int aeGetSetSize(aeEventLoop *eventLoop); /* ��ȡeventLoop�Ĵ�С */
int aeResizeSetSize(aeEventLoop *eventLoop, int setsize); /* EventLoop���µ�����С */

#endif
