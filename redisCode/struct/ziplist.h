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

/* ����б�ͷ�ڵ��β���ı�ʶ */
#define ZIPLIST_HEAD 0
#define ZIPLIST_TAIL 1

unsigned char *ziplistNew(void);    //�������б�
unsigned char *ziplistPush(unsigned char *zl, unsigned char *s, unsigned int slen, int where);  //���б�����������
unsigned char *ziplistIndex(unsigned char *zl, int index);   //������λ���б��ĳ��λ��
unsigned char *ziplistNext(unsigned char *zl, unsigned char *p);   //��ȡ��ǰ�б�λ�õ���һ��ֵ
unsigned char *ziplistPrev(unsigned char *zl, unsigned char *p);   //��ȡ�����б�λ�õ�ǰһ��ֵ
unsigned int ziplistGet(unsigned char *p, unsigned char **sval, unsigned int *slen, long long *lval);   //��ȡ�б����Ϣ
unsigned char *ziplistInsert(unsigned char *zl, unsigned char *p, unsigned char *s, unsigned int slen); //���б��в�������
unsigned char *ziplistDelete(unsigned char *zl, unsigned char **p); //�б���ɾ��ĳ�����
unsigned char *ziplistDeleteRange(unsigned char *zl, unsigned int index, unsigned int num);   //��index������Ӧ�Ľ�㿪ʼ����ɾ��num�����
unsigned int ziplistCompare(unsigned char *p, unsigned char *s, unsigned int slen);   //�б��ıȽϷ���
unsigned char *ziplistFind(unsigned char *p, unsigned char *vstr, unsigned int vlen, unsigned int skip); //���б���Ѱ��ĳ�����
unsigned int ziplistLen(unsigned char *zl);   //�����б�ĳ���
size_t ziplistBlobLen(unsigned char *zl);   //�����б�Ķ����Ƴ��ȣ����ص����ֽ���
