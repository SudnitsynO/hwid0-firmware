#pragma once
#include <inttypes.h>

template <class SCLK, class SDI, class SDO, class nSEL>
class SI4463
{
public:
	void Init();
	uint8_t send(uint8_t byte_to_send);
private:
};

template <class SCLK, class SDI, class SDO, class nSEL>
uint8_t SI4463<SCLK, SDI, SDO, nSEL>::send(uint8_t byte_to_send)
{
	return 0;
}
