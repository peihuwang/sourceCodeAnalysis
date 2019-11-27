/* adlist.h - A generic doubly linked list implementation
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

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node, List, and Iterator are the only data structures used currently. */
/* listNode��� */
typedef struct listNode {
	//����ǰһ���
    struct listNode *prev;
    //������һ���
    struct listNode *next;
    //Node�ĺ���ָ��
    void *value;
} listNode;

/* list��������ֻ��Ϊ���� */
typedef struct listIter {
	//��ǰ����λ�õ���һ���
    listNode *next;
    //�������ķ���
    int direction;
} listIter;

/* listNode �б� */
typedef struct list {
	//�б�ͷ���
    listNode *head;
    //�б�β���
    listNode *tail;
    
    /* ����3������Ϊ���н�㹫�õķ������ֱ�����Ӧ����»ص��� */
    //���ƺ���ָ��
    void *(*dup)(void *ptr);
    //�ͷź���ָ��
    void (*free)(void *ptr);
   	//ƥ�亯��ָ��
    int (*match)(void *ptr, void *key);
    //�б���
    unsigned long len;
} list;

/* Functions implemented as macros */
/* �궨����һЩ�������� */
#define listLength(l) ((l)->len)   //��ȡlist����
#define listFirst(l) ((l)->head)   //��ȡ�б��ײ�
#define listLast(l) ((l)->tail)    //��ȡ�б�β��
#define listPrevNode(n) ((n)->prev)  //����������һ���
#define listNextNode(n) ((n)->next)  //����������һ�ڵ�
#define listNodeValue(n) ((n)->value) //����Ľ���ֵ�����value����һ����ֵ���ͣ�����һ������ָ��

#define listSetDupMethod(l,m) ((l)->dup = (m))  //�б�ĸ��Ʒ���������
#define listSetFreeMethod(l,m) ((l)->free = (m)) //�б���ͷŷ���������
#define listSetMatchMethod(l,m) ((l)->match = (m)) //�б��ƥ�䷽��������

#define listGetDupMethod(l) ((l)->dup) //�б�ĸ��Ʒ����Ļ�ȡ
#define listGetFree(l) ((l)->free)     //�б���ͷŷ����Ļ�ȡ
#define listGetMatchMethod(l) ((l)->match) //�б��ƥ�䷽���Ļ�ȡ

/* Prototypes */
/* �����˷�����ԭ�� */
list *listCreate(void);   //����list�б�
void listRelease(list *list);  //�б���ͷ�
list *listAddNodeHead(list *list, void *value);  //����б�ͷ���
list *listAddNodeTail(list *list, void *value);  //����б�β���
list *listInsertNode(list *list, listNode *old_node, void *value, int after);  //ĳλ���ϲ��뼰���
void listDelNode(list *list, listNode *node);  //�б���ɾ�������Ľ��
listIter *listGetIterator(list *list, int direction);  //��ȡ�б���������ϵĵ�����
listNode *listNext(listIter *iter);  //��ȡ�������ڵ���һ���
void listReleaseIterator(listIter *iter);  //�ͷ��б������ 
list *listDup(list *orig);  //�б�ĸ���
listNode *listSearchKey(list *list, void *key); //�ؼ�������������
listNode *listIndex(list *list, long index);   //�±���������Ľ��
void listRewind(list *list, listIter *li);    // ���õ�����Ϊ�����ͷ��ʼ 
void listRewindTail(list *list, listIter *li); //���õ�����Ϊ�����β����ʼ 
void listRotate(list *list);  //�б���ת������������˵�ĺ�����������ֻ�ܵ�ʵ����ȥ����

/* Directions for iterators */
/* ����2���������򣬴�ͷ����ʼ��β�����ڶ�����β����ʼ��ͷ�� */
#define AL_START_HEAD 0
#define AL_START_TAIL 1

#endif /* __ADLIST_H__ */
