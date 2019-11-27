/* SDSLib, A C dynamic strings library
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __SDS_H
#define __SDS_H

/* �������ڴ�1M */
#define SDS_MAX_PREALLOC (1024*1024)

#include <sys/types.h>
#include <stdarg.h>

/* ������sds��һ��char���� */
typedef char *sds;

/* �ַ����ṹ������ */
struct sdshdr {
	//�ַ�����
    unsigned int len;
    //��ǰ���ÿռ�
    unsigned int free;
    //�������ַ���buf
    char buf[];
};

/* ����sds�ĳ��ȣ����ص�size_t���͵���ֵ */
/* size_t,����һ���������ص�unsigned���ͣ����С���Ա�֤�洢�ڴ��ж���Ĵ�С�� */
static inline size_t sdslen(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->len;
}

/* ����sdshdr�е�free��ǻ�ȡ���ÿռ� */
static inline size_t sdsavail(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->free;
}

sds sdsnewlen(const void *init, size_t initlen);   //���ݸ������ȣ�������һ��sds
sds sdsnew(const char *init);    //���ݸ�����ֵ������sds
sds sdsempty(void);    //���sds����
size_t sdslen(const sds s);   //��ȡsds�ĳ���
sds sdsdup(const sds s);   //sds�ĸ��Ʒ���
void sdsfree(sds s);   //sds��free�ͷŷ���
size_t sdsavail(const sds s);   //�ж�sds��ȡ���ÿռ�
sds sdsgrowzero(sds s, size_t len); // ��չ�ַ�����ָ���ĳ��� 
sds sdscatlen(sds s, const void *t, size_t len);
sds sdscat(sds s, const char *t);    //sds������char�ַ�
sds sdscatsds(sds s, const sds t);  //sds������sds
sds sdscpylen(sds s, const char *t, size_t len);  //�ַ����������
sds sdscpy(sds s, const char *t); //�ַ����������

sds sdscatvprintf(sds s, const char *fmt, va_list ap);   //�ַ�����ʽ��������������еķ���sprintf��Ч�ʲ��������Լ�д��
#ifdef __GNUC__
sds sdscatprintf(sds s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);
#endif

sds sdscatfmt(sds s, char const *fmt, ...);   //�ַ�����ʽ�����
sds sdstrim(sds s, const char *cset);       //�ַ�������
void sdsrange(sds s, int start, int end);   //�ַ�����ȡ����
void sdsupdatelen(sds s);   //�����ַ������µĳ���
void sdsclear(sds s);   //�ַ�����ղ���
int sdscmp(const sds s1, const sds s2);   //sds�ȽϺ���
sds *sdssplitlen(const char *s, int len, const char *sep, int seplen, int *count);  //�ַ����ָ����ַ���
void sdsfreesplitres(sds *tokens, int count);  //�ͷ����ַ�������
void sdstolower(sds s);    //sds�ַ�תСд��ʾ
void sdstoupper(sds s);    //sds�ַ�ͳһת��д
sds sdsfromlonglong(long long value);   //���������ַ���
sds sdscatrepr(sds s, const char *p, size_t len);
sds *sdssplitargs(const char *line, int *argc);   //�������
sds sdsmapchars(sds s, const char *from, const char *to, size_t setlen); //�ַ�ӳ��,"ho", "01", hӳ��Ϊ0�� oӳ��Ϊ1
sds sdsjoin(char **argv, int argc, char *sep);   //�Էָ��������ַ��������鹹���µ��ַ���

/* Low level functions exposed to the user API */
/* ���Ÿ�ʹ���ߵ�API */
sds sdsMakeRoomFor(sds s, size_t addlen);
void sdsIncrLen(sds s, int incr);
sds sdsRemoveFreeSpace(sds s);
size_t sdsAllocSize(sds s);

#endif
