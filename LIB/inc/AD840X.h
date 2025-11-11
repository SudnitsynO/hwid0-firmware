#pragma once
#include "gpio_interface.h"
#include "delay.h"

template <class  MOSI_pin, class SCK_pin, class SYNC_pin>
class AD840X
{
public:
	void set_position(uint8_t channel, uint8_t position);
	void init();
private:
};

template <class MOSI_pin, class SCK_pin, class SYNC_pin>
void AD840X<MOSI_pin, SCK_pin, SYNC_pin>::set_position(uint8_t channel, uint8_t position)
{
	SCK_pin::Reset();
	SYNC_pin::Set();
	Delay_us(100);
	SYNC_pin::Reset();
	Delay_us(100);
	uint16_t b = ((channel & 3) << 8) | position;
	for (int i = 9; i >= 0; i--)
	{
		MOSI_pin::Write(b & (1 << i));
		Delay_us(100);
		SCK_pin::Set();
		Delay_us(100);
		SCK_pin::Reset();
	}
	SYNC_pin::Set();
}

template <class MOSI_pin, class SCK_pin, class SYNC_pin>
void AD840X<MOSI_pin, SCK_pin, SYNC_pin>::init()
{
	MOSI_pin::SetMode(Mode::Output);
	SCK_pin::SetMode(Mode::Output);
	SYNC_pin::SetMode(Mode::Output);
}

