/* sparkline.h -- ASCII Sparklines header file
 *
 * ---------------------------------------------------------------------------
 *
 * Copyright(C) 2011-2014 Salvatore Sanfilippo <antirez@gmail.com>
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

#ifndef __SPARKLINE_H
#define __SPARKLINE_H

/* sparkline��һ����Ϣ���С�������ܶȸߵ�ͼ��Ŀǰ��������һЩ������
 *��صı仯����Ϣ���ֵķ�ʽ����ƽ���¶ȣ����н�Ͷ��Ծ��sparkline������һ���������ʽ��������״ͼ������ͼ���С�
 *�������Ϊһ��ͼ����Ϣ */
/* A sequence is represented of many "samples" */
/* �������Ϊͼ���ϵ�һ����Ϣ�㣬�����֣���ֵ�Ĵ�С */
struct sample {
    double value;
    char *label;
};

/* ͼ����Ϣ�ṹ�壬����n��Ԫ�ص㣬���Ծݴ�������ͼ����ͼ�Ŀɲ���ֱ�Ӱ����ֱֵ�ӻ��Ƶ� */
struct sequence {
	//��ǰԪ�ص����
    int length;
    //�ܹ������ָ�������Щ��û��label������ΪNULL
    int labels;
    //Ԫ�ص��б�
    struct sample *samples;
    //Ԫ���е����ֵ����Сֵ
    double min, max;
};

/* ������һЩ��Ⱦͼʱ��һЩ���Բ������� */
#define SPARKLINE_NO_FLAGS 0
#define SPARKLINE_FILL 1      /* Fill the area under the curve. */
#define SPARKLINE_LOG_SCALE 2 /* Use logarithmic scale. */

struct sequence *createSparklineSequence(void);  //����ͼ�����нṹ��
void sparklineSequenceAddSample(struct sequence *seq, double value, char *label); //��ͼ�����������һ����Ϣ��
void freeSparklineSequence(struct sequence *seq);   //�ͷ�ͼ������
sds sparklineRenderRange(sds output, struct sequence *seq, int rows, int offset, int len, int flags); //��Ⱦͼ������Ϊһ��ͼ����ʵ���ǵõ�һ���ַ�����ɵ�ͼ
sds sparklineRender(sds output, struct sequence *seq, int columns, int rows, int flags); //����ͬ�ϣ�ֻ���ٿ�һ��ƫ����

#endif /* __SPARKLINE_H */
