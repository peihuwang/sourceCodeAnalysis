#ifndef CRC64_H
#define CRC64_H

#include <stdint.h>

/* Crc64ѭ�����������㷨��crc:����ֵ0��s����������ݣ�l:���ݳ��� */
uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);

#endif
