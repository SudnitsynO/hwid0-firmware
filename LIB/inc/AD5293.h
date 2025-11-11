#pragma once
#include "gpio_interface.h"
#include "delay.h"

template <class MOSI_pin, class SCK_pin, class SYNC_pin>
class AD5293
{
public:
	void init();
	void set_position(uint16_t pos);	//устанавливает положение движка резистора 0-1023
private:
	void send_data(uint8_t control, uint16_t pos);
};

template <class MOSI_pin, class SCK_pin, class SYNC_pin>
void AD5293<MOSI_pin, SCK_pin, SYNC_pin>::init()
{
	MOSI_pin::SetMode(Mode::Output);
	SCK_pin::SetMode(Mode::Output);
	SYNC_pin::SetMode(Mode::Output);
	send_data(0x8, 0);	//включение питания
	send_data(0x6, 0x006);//настройка
}

template <class MOSI_pin, class SCK_pin, class SYNC_pin>
void AD5293<MOSI_pin, SCK_pin, SYNC_pin>::set_position(uint16_t pos)
{
	send_data(0x1, pos);
}

template <class MOSI_pin, class SCK_pin, class SYNC_pin>
void AD5293<MOSI_pin, SCK_pin, SYNC_pin>::send_data(uint8_t control, uint16_t pos)
{
	SYNC_pin::Set();
	Delay_us(1);
	SCK_pin::Reset();
	Delay_us(1);
	SYNC_pin::Reset();
	Delay_us(1);
	MOSI_pin::Reset();
	Delay_us(1);
	for (int i = 0; i < 2; i++)
	{
		SCK_pin::Set();
		Delay_us(1);
		SCK_pin::Reset();
		Delay_us(1);
	}
	for (int i = 3; i >= 0; i--)
	{
		MOSI_pin::Write(control & (1 << i));
		Delay_us(1);
		SCK_pin::Set();
		Delay_us(1);
		SCK_pin::Reset();
	}
	for (int i = 9; i >= 0; i--)
	{
		MOSI_pin::Write(pos & (1 << i));
		Delay_us(1);
		SCK_pin::Set();
		Delay_us(1);
		SCK_pin::Reset();
	}
	Delay_us(1);
	SYNC_pin::Set();
}

