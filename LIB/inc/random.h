#pragma once
#include <cstdint>

class XorShift
{
public:
	XorShift();
	uint32_t xorshift128();
	void Seed(void * random_data, int data_length);
	void Reset();
	
	uint64_t random64();
	uint64_t random64(uint64_t max_int);
	uint32_t random32();
	uint32_t random32(uint32_t max_int);
	uint16_t random16();
	uint16_t random16(uint16_t max_int);
	uint8_t random8();
	float random_single();
private:
	uint32_t state[4];
};
