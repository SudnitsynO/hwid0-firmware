//#include "pch.h"
#include "random.h"
#include <cstring>

XorShift::XorShift() : state{0x418B2B91,0x125E16C8,0x5FF57607,0xBD1A8DF8}
{
}

uint32_t XorShift::xorshift128()
{/* Algorithm "xor128" from p. 5 of Marsaglia, "Xorshift RNGs" */
	uint32_t s, t = state[3];
	t ^= t << 11;
	t ^= t >> 8;
	state[3] = state[2]; state[2] = state[1]; state[1] = s = state[0];
	t ^= s;
	t ^= s >> 19;
	state[0] = t;
	return t;
}

void XorShift::Seed(void* random_data, int data_length)
{
	if (data_length > sizeof(state))
		data_length = sizeof(state);
	memcpy(state, random_data, data_length);
	for (auto i = 0; i < 4; i++)
	{
		if (state[i] == 0)
			state[i] = (0xB71A1F9B << i) ^ (0x09359117 >> i);
	}
}

void XorShift::Reset()
{
	state[0] = 0x418B2B91;
	state[1] = 0x125E16C8;
	state[2] = 0x5FF57607;
	state[3] = 0xBD1A8DF8;
}

uint64_t XorShift::random64()
{
	return static_cast<uint64_t>(xorshift128()) | (static_cast<uint64_t>(xorshift128()) << 32);
}

uint64_t XorShift::random64(uint64_t max_int)
{
	return xorshift128() % max_int;
}

uint32_t XorShift::random32()
{
	return xorshift128();
}

uint32_t XorShift::random32(uint32_t max_int)
{
	return xorshift128() % max_int;
}

uint16_t XorShift::random16()
{
	return xorshift128();
}

uint16_t XorShift::random16(uint16_t max_int)
{
	return xorshift128() % max_int;
}

uint8_t XorShift::random8()
{
	return xorshift128();
}

float XorShift::random_single()
{
	return float(xorshift128()) / UINT32_MAX;
}
