#pragma once

#include <vector>
#include "Buffer.h"

unsigned short Crc16(unsigned short crc, unsigned char byte);
unsigned short Crc16_block(unsigned short crc, unsigned char *pcBlock, unsigned int len);
#define CRC16_INIT_VALUE (0xFFFFUS)

unsigned short Crc_16_IBM(unsigned short crc, unsigned char byte);
unsigned short Crc_16_IBM_block(unsigned short crc, unsigned char *pcBlock, unsigned int len);

#define CRC32_INIT_VALUE (0)
unsigned long Crc32Block(unsigned long crc, const void *buf, int size);
unsigned long Crc32Block(const std::vector<uint8_t> &buf, unsigned long crc = CRC32_INIT_VALUE);
unsigned long Crc32Block(const Buffer &buf, unsigned long crc = CRC32_INIT_VALUE);
unsigned long Crc32(unsigned long crc, unsigned char byte);

template <class T>
unsigned long Crc32OfObject(T &object, unsigned long crc_init_value = CRC32_INIT_VALUE)
{
	return Crc32Block(crc_init_value, &object, sizeof(object));
}
