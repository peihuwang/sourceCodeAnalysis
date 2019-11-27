/* latency.h -- latency monitor API header file
 * See latency.c for more information.
 *
 * ----------------------------------------------------------------------------
 *
 * Copyright (c) 2014, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __LATENCY_H
#define __LATENCY_H

#define LATENCY_TS_LEN 160 /* History length for every monitored event. */

/* Representation of a latency sample: the sampling time and the latency
 * observed in milliseconds. */
/* ��ʱ��Ʒ���� */
struct latencySample {
	//��ʱSample������ʱ��
    int32_t time; /* We don't use time_t to force 4 bytes usage everywhere. */
    //��ʱ�ľ���ʱ�䣬 ��λΪ����
    uint32_t latency; /* Latency in milliseconds. */
};

/* The latency time series for a given event. */
/* ���ĳ���¼��ɼ���һϵ����ʱsample */
struct latencyTimeSeries {
	//��һ����ʱSample���±�
    int idx; /* Index of the next sample to store. */
    //������ʱ
    uint32_t max; /* Max latency observed for this event. */
    //�������ʱ��¼
    struct latencySample samples[LATENCY_TS_LEN]; /* Latest history. */
};

/* Latency statistics structure. */
/* ��ʱsample������ͳ�ƽ���ṹ�� */
struct latencyStats {
	//������ߵ���ʱʱ��
    uint32_t all_time_high; /* Absolute max observed since latest reset. */
    //ƽ��Sample��ʱʱ��
    uint32_t avg;           /* Average of current samples. */
    //Sample����С��ʱʱ��
    uint32_t min;           /* Min of current samples. */
    //Sample�������ʱʱ��
    uint32_t max;           /* Max of current samples. */
    //ƽ���������ƽ����ʱ���
    uint32_t mad;           /* Mean absolute deviation. */
    //samples������
    uint32_t samples;       /* Number of non-zero samples. */
    //�������ʱ��¼��Ĵ���ʱ��
    time_t period;          /* Number of seconds since first event and now. */
};

void latencyMonitorInit(void); /* ��ʱ������ʼ������������Event�ֵ���� */
void latencyAddSample(char *event, mstime_t latency); /* ���Sample��ָ����Event�����Sample�б��� */

/* Latency monitoring macros. */

/* Start monitoring an event. We just set the current time. */
/* ��ĳ���¼����ü�������������һ�µ�ǰ��ʱ�� */
#define latencyStartMonitor(var) if (server.latency_monitor_threshold) { \
    var = mstime(); \
} else { \
    var = 0; \
}

/* End monitoring an event, compute the difference with the current time
 * to check the amount of time elapsed. */
/* ����������������˶���ʱ�� */
#define latencyEndMonitor(var) if (server.latency_monitor_threshold) { \
    var = mstime() - var; \
}

/* Add the sample only if the elapsed time is >= to the configured threshold. */
/* �����ʱʱ�䳬��server.latency_monitor_threshold����Sample������ʱ�б��� */
#define latencyAddSampleIfNeeded(event,var) \
    if (server.latency_monitor_threshold && \
        (var) >= server.latency_monitor_threshold) \
          latencyAddSample((event),(var));

#endif /* __LATENCY_H */
