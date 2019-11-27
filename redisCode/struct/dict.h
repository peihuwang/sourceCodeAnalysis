/* Hash Tables Implementation.
 *
 * This file implements in-memory hash tables with insert/del/replace/find/
 * get-random-element operations. Hash tables will auto-resize if needed
 * tables of power of two in size are used, collisions are handled by
 * chaining. See the source code for more information... :)
 *
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

#include <stdint.h>

#ifndef __DICT_H
#define __DICT_H

/* �����˳ɹ�������ֵ */
#define DICT_OK 0
#define DICT_ERR 1

/* Unused arguments generate annoying warnings... */
/* dictû���õ�ʱ��������ʾ����� */
#define DICT_NOTUSED(V) ((void) V)

/* �ֵ�ṹ�壬����K-Vֵ�Ľṹ�� */
typedef struct dictEntry {
	//�ֵ�key����ָ��
    void *key;
    union {
        void *val;
        //�޷�������ֵ
        uint64_t u64;
        //�з�������ֵ
        int64_t s64;
        double d;
    } v;
    //��һ�ֵ���
    struct dictEntry *next;
} dictEntry;

/* �ֵ����� */
typedef struct dictType {
	//��ϣ���㷽�����������α���
    unsigned int (*hashFunction)(const void *key);
    //����key����
    void *(*keyDup)(void *privdata, const void *key);
    //����val����
    void *(*valDup)(void *privdata, const void *obj);
    //keyֵ�ȽϷ���
    int (*keyCompare)(void *privdata, const void *key1, const void *key2);
    //key����������
    void (*keyDestructor)(void *privdata, void *key);
    //val����������
    void (*valDestructor)(void *privdata, void *obj);
} dictType;

/* This is our hash table structure. Every dictionary has two of this as we
 * implement incremental rehashing, for the old to the new table. */
/* ��ϣ��ṹ�� */
typedef struct dictht {
	//�ֵ�ʵ��
    dictEntry **table;
    //���������ֵ�����
    unsigned long size;
    unsigned long sizemask;
    //���ڱ�ʹ�õ�����
    unsigned long used;
} dictht;

/* �ֵ��������� */
typedef struct dict {
	//�ֵ�����
    dictType *type;
    //˽������ָ��
    void *privdata;
    //�ֵ��ϣ����2�ţ�һ�žɵģ�һ���µ�
    dictht ht[2];
    //�ض�λ��ϣʱ���±�
    long rehashidx; /* rehashing not in progress if rehashidx == -1 */
    //��ǰ����������
    int iterators; /* number of iterators currently running */
} dict;

/* If safe is set to 1 this is a safe iterator, that means, you can call
 * dictAdd, dictFind, and other functions against the dictionary even while
 * iterating. Otherwise it is a non safe iterator, and only dictNext()
 * should be called while iterating. */
/* �ֵ������������ǰ�ȫ����������safe����Ϊ1�����Ե���dicAdd��dictFind */
/* ����ǲ���ȫ�ģ���ֻ�ܵ���dicNext����*/
typedef struct dictIterator {
	//��ǰ�ֵ�
    dict *d;
    //�±�
    long index;
    //��񣬺Ͱ�ȫֵ�ı�������Ǿɵı�񣬻����µı��
    int table, safe;
    //�ֵ�ʵ��
    dictEntry *entry, *nextEntry;
    /* unsafe iterator fingerprint for misuse detection. */
    /* ָ�Ʊ�ǣ����ⲻ��ȫ�ĵ������������� */
    long long fingerprint;
} dictIterator;

/* �ֵ�ɨ�跽�� */
typedef void (dictScanFunction)(void *privdata, const dictEntry *de);

/* This is the initial size of every hash table */
/* ��ʼ����ϣ�����Ŀ */
#define DICT_HT_INITIAL_SIZE     4

/* ------------------------------- Macros ------------------------------------*/
/* �ֵ��ͷ�val����ʱ����ã����dict�е�dictType�������������ָ�룬 */
#define dictFreeVal(d, entry) \
    if ((d)->type->valDestructor) \
        (d)->type->valDestructor((d)->privdata, (entry)->v.val)
    
/* �ֵ�val��������ʱ����ã����dict�е�dictType�������������ָ�룬 */
#define dictSetVal(d, entry, _val_) do { \
    if ((d)->type->valDup) \
        entry->v.val = (d)->type->valDup((d)->privdata, _val_); \
    else \
        entry->v.val = (_val_); \
} while(0)

/* ����dictEntry�й�����v���з������͵�ֵ */
#define dictSetSignedIntegerVal(entry, _val_) \
    do { entry->v.s64 = _val_; } while(0)

/* ����dictEntry�й�����v���޷������͵�ֵ */
#define dictSetUnsignedIntegerVal(entry, _val_) \
    do { entry->v.u64 = _val_; } while(0)

/* ����dictEntry�й�����v��double���͵�ֵ */
#define dictSetDoubleVal(entry, _val_) \
    do { entry->v.d = _val_; } while(0)

/* ����dictType�����key�������� */
#define dictFreeKey(d, entry) \
    if ((d)->type->keyDestructor) \
        (d)->type->keyDestructor((d)->privdata, (entry)->key)

/* ����dictType�����key���ƺ�����û�ж���ֱ�Ӹ�ֵ */
#define dictSetKey(d, entry, _key_) do { \
    if ((d)->type->keyDup) \
        entry->key = (d)->type->keyDup((d)->privdata, _key_); \
    else \
        entry->key = (_key_); \
} while(0)

/* ����dictType�����key�ȽϺ�����û�ж���ֱ��keyֱֵ�ӱȽ� */
#define dictCompareKeys(d, key1, key2) \
    (((d)->type->keyCompare) ? \
        (d)->type->keyCompare((d)->privdata, key1, key2) : \
        (key1) == (key2))

#define dictHashKey(d, key) (d)->type->hashFunction(key)   //��ϣ��λ����
#define dictGetKey(he) ((he)->key)    //��ȡdictEntry��keyֵ
#define dictGetVal(he) ((he)->v.val)  //��ȡdicEntry�й�����v�ж����valֵ
#define dictGetSignedIntegerVal(he) ((he)->v.s64) //��ȡdicEntry�й�����v�ж�����з���ֵ
#define dictGetUnsignedIntegerVal(he) ((he)->v.u64)  //��ȡdicEntry�й�����v�ж�����޷���ֵ
#define dictGetDoubleVal(he) ((he)->v.d)  //��ȡdicEntry�й�����v�ж����double����ֵ
#define dictSlots(d) ((d)->ht[0].size+(d)->ht[1].size)  //��ȡdict�ֵ����ܵı��С
#define dictSize(d) ((d)->ht[0].used+(d)->ht[1].used)   //��ȡdict�ֵ����ܵı�������ڱ�ʹ�õ�����
#define dictIsRehashing(d) ((d)->rehashidx != -1)   //�ֵ����ޱ��ض�λ��

/* API */
dict *dictCreate(dictType *type, void *privDataPtr);   //����dict�ֵ�����
int dictExpand(dict *d, unsigned long size);    //�ֵ���������
int dictAdd(dict *d, void *key, void *val);    //�ֵ����key, val���һ���ֵ伯
dictEntry *dictAddRaw(dict *d, void *key);     //�ֵ����һ��ֻ��keyֵ��dicEntry
int dictReplace(dict *d, void *key, void *val); //���dict��һ���ֵ伯
dictEntry *dictReplaceRaw(dict *d, void *key);  //���dict�е�һ���ֵ䣬ֻ�ṩһ��keyֵ
int dictDelete(dict *d, const void *key);    //����keyɾ��һ���ֵ伯
int dictDeleteNoFree(dict *d, const void *key);  //�ֵ伯ɾ���ޡ�������free����
void dictRelease(dict *d);   //�ͷ�����dict
dictEntry * dictFind(dict *d, const void *key);  //����keyѰ���ֵ伯
void *dictFetchValue(dict *d, const void *key);  //����keyֵѰ����Ӧ��valֵ
int dictResize(dict *d);  //���¼����С
dictIterator *dictGetIterator(dict *d); //��ȡ�ֵ������
dictIterator *dictGetSafeIterator(dict *d);  //��ȡ�ֵ䰲ȫ������  
dictEntry *dictNext(dictIterator *iter);   //�����ֵ��������ȡ�ֵ伯����һ�ֵ伯
void dictReleaseIterator(dictIterator *iter); //�ͷŵ�����
dictEntry *dictGetRandomKey(dict *d);  //�����ȡһ���ֵ伯
void dictPrintStats(dict *d);  //��ӡ��ǰ�ֵ�״̬
unsigned int dictGenHashFunction(const void *key, int len); //�����keyֵ��Ŀ�곤�ȣ��˷���������������ֵ
unsigned int dictGenCaseHashFunction(const unsigned char *buf, int len); //�����ṩ��һ�ֱȽϼ򵥵Ĺ�ϣ�㷨
void dictEmpty(dict *d, void(callback)(void*)); //����ֵ�
void dictEnableResize(void);  //���õ�������
void dictDisableResize(void); //���õ�������
int dictRehash(dict *d, int n); //hash�ض�λ����Ҫ�Ӿɵı�ӳ�䵽�±���,��n�ֶ�λ
int dictRehashMilliseconds(dict *d, int ms);  //�ڸ���ʱ���ڣ�ѭ��ִ�й�ϣ�ض�λ
void dictSetHashFunctionSeed(unsigned int initval); //���ù�ϣ��������
unsigned int dictGetHashFunctionSeed(void);  //��ȡ��ϣ����
unsigned long dictScan(dict *d, unsigned long v, dictScanFunction *fn, void *privdata); //�ֵ�ɨ�跽��

/* Hash table types */
/* ��ϣ������  */
extern dictType dictTypeHeapStringCopyKey;
extern dictType dictTypeHeapStrings;
extern dictType dictTypeHeapStringCopyKeyValue;

#endif /* __DICT_H */
