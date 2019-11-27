/*
 * Copyright (c) 2009-2012, Pieter Noordhuis <pcnoordhuis at gmail dot com>
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "config.h"

/* �궨�����ķ��� */
#define ERROR(...) { \
    char __buf[1024]; \
    sprintf(__buf, __VA_ARGS__); \
    //��������λ�ã��ʹ��������
    sprintf(error, "0x%16llx: %s", (long long)epos, __buf); \
}

static char error[1024];
static off_t epos;

/* ����API */
int consumeNewline(char *buf) /* ����bufǰ��Ļ��з������Ƚ�buf�ַ����е�ǰ2���ַ� */
int readLong(FILE *fp, char prefix, long *target) /* ���ļ��ж�ȡlong����ֵ */
int readBytes(FILE *fp, char *target, long length) /* ���ļ��ж�ȡ�ֽ� */
int readString(FILE *fp, char** target) /* �ļ��ж�ȡ�ַ��� */
int readArgc(FILE *fp, long *target) /* �ļ��ж�ȡ����,���ַ��ԡ�*����ͷ */
off_t process(FILE *fp) /* ����fp�ļ���ƫ���� */
	
/* ����bufǰ��Ļ��з������Ƚ�buf�ַ����е�ǰ2���ַ� */
int consumeNewline(char *buf) {
    if (strncmp(buf,"\r\n",2) != 0) {
    	//������ǵ���"\r\n"������ʾ����
        ERROR("Expected \\r\\n, got: %02x%02x",buf[0],buf[1]);
        return 0;
    }
    return 1;
}

/* ���ļ��ж�ȡlong����ֵ */
int readLong(FILE *fp, char prefix, long *target) {
    char buf[128], *eptr;
    //��λ���ļ��Ķ�ȡλ��
    epos = ftello(fp);
    //���ļ��е����ݶ�ȡ��buf��
    if (fgets(buf,sizeof(buf),fp) == NULL) {
    	//���Ϊ��ֱ�ӷ���
        return 0;
    }
    
    //�����ȡ�������ַ�������Ԥ��ֵ������ʾ����
    if (buf[0] != prefix) {
        ERROR("Expected prefix '%c', got: '%c'",buf[0],prefix);
        return 0;
    }
    
    //���ַ���ֵת��long����ֵ
    *target = strtol(buf+1,&eptr,10);
    
    return consumeNewline(eptr);
}

/* ���ļ��ж�ȡ�ֽ� */
int readBytes(FILE *fp, char *target, long length) {
    long real;
    //��λ���ļ��Ķ�ȡλ��
    epos = ftello(fp);
    //���ֽڶ�ȡ��Target�ַ�����,�����ַ����ĳ���
    real = fread(target,1,length,fp);
    if (real != length) {
    	//��������������Ԥ�ڳ��Ȳ��ȣ�����ʾ����
        ERROR("Expected to read %ld bytes, got %ld bytes",length,real);
        return 0;
    }
    return 1;
}

/* �ļ��ж�ȡ�ַ��� */
int readString(FILE *fp, char** target) {
    long len;
    *target = NULL;
    if (!readLong(fp,'$',&len)) {
        return 0;
    }

    /* Increase length to also consume \r\n */
    //���ӳ��ȣ�����"\r\n"���з�
    len += 2;
    *target = (char*)malloc(len);
    if (!readBytes(fp,*target,len)) {
        return 0;
    }
    if (!consumeNewline(*target+len-2)) {
        return 0;
    }
    (*target)[len-2] = '\0';
    return 1;
}

/* �ļ��ж�ȡ����,���ַ��ԡ�*����ͷ */
int readArgc(FILE *fp, long *target) {
    return readLong(fp,'*',target);
}

/* �Ƚ���off_t�ĸ��off_t�Ĺٷ����� */
/* This is a data type defined in the sys/types.h header file (of fundamental type unsigned long) and is used to measure the file offset in bytes from the beginning of the file. It is defined as a signed, 32-bit integer, */
/* but if the programming environment enables large files off_t is defined to be a signed, 64-bit integer */
/* ��˼��off_t�Ǳ���Ϊ�����ļ��е��ֽ�ƫ�����ģ�Ĭ�������з���32λ������������ͻ���Ϊϵͳ�����ı���ı䣬��ʱ���Ϊ�з�������64�� */
/* ����fp�ļ���ƫ���� */
off_t process(FILE *fp) {
    long argc;
    off_t pos = 0;
    int i, multi = 0;
    char *str;

    while(1) {
    	//���multiΪ��ʼֵ�������ļ���λ
        if (!multi) pos = ftello(fp);
        //���ļ��ж�ȡ������ֵ������ֵΪ�ַ�������
        if (!readArgc(fp, &argc)) break;

		//�����ļ��е�����ַ���
        for (i = 0; i < argc; i++) {
            if (!readString(fp,&str)) break;
            if (i == 0) {
                if (strcasecmp(str, "multi") == 0) {
                    if (multi++) {
                        ERROR("Unexpected MULTI");
                        break;
                    }
                } else if (strcasecmp(str, "exec") == 0) {
                    if (--multi) {
                        ERROR("Unexpected EXEC");
                        break;
                    }
                }
            }
            free(str);
        }

        /* Stop if the loop did not finish */
        /* ֱ���˳����ѭ��û�н��� */
        if (i < argc) {
            if (str) free(str);
            break;
        }
    }

    if (feof(fp) && multi && strlen(error) == 0) {
        ERROR("Reached EOF before reading EXEC for MULTI");
    }
    if (strlen(error) > 0) {
        printf("%s\n", error);
    }
    return pos;
}

/* ���Գ��� */
int main(int argc, char **argv) {
    char *filename;
    int fix = 0;

    if (argc < 2) {
        printf("Usage: %s [--fix] <file.aof>\n", argv[0]);
        exit(1);
    } else if (argc == 2) {
        filename = argv[1];
    } else if (argc == 3) {
        if (strcmp(argv[1],"--fix") != 0) {
            printf("Invalid argument: %s\n", argv[1]);
            exit(1);
        }
        filename = argv[2];
        fix = 1;
    } else {
        printf("Invalid arguments\n");
        exit(1);
    }

    FILE *fp = fopen(filename,"r+");
    if (fp == NULL) {
        printf("Cannot open file: %s\n", filename);
        exit(1);
    }

    struct redis_stat sb;
    if (redis_fstat(fileno(fp),&sb) == -1) {
        printf("Cannot stat file: %s\n", filename);
        exit(1);
    }

    off_t size = sb.st_size;
    if (size == 0) {
        printf("Empty file: %s\n", filename);
        exit(1);
    }

    off_t pos = process(fp);
    //diffΪʣ��Ŀռ�
    off_t diff = size-pos;
    printf("AOF analyzed: size=%lld, ok_up_to=%lld, diff=%lld\n",
        (long long) size, (long long) pos, (long long) diff);
    if (diff > 0) {
        if (fix) {
            char buf[2];
            printf("This will shrink the AOF from %lld bytes, with %lld bytes, to %lld bytes\n",(long long)size,(long long)diff,(long long)pos);
            printf("Continue? [y/N]: ");
            if (fgets(buf,sizeof(buf),stdin) == NULL ||
                strncasecmp(buf,"y",1) != 0) {
                    printf("Aborting...\n");
                    exit(1);
            }
            
            //�ض��ļ��Ĳ��������ʼ�ͷ���������ƫ������û���õĿռ��ȥ
            if (ftruncate(fileno(fp), pos) == -1) {
                printf("Failed to truncate AOF\n");
                exit(1);
            } else {
                printf("Successfully truncated AOF\n");
            }
        } else {
            printf("AOF is not valid\n");
            exit(1);
        }
    } else {
        printf("AOF is valid\n");
    }

    fclose(fp);
    return 0;
}
