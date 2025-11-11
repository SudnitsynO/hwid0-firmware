#pragma once
#include <stm32f1xx_hal.h>
#include "onewirebase.h"

/********************************************************
 *				Реализация OneWire для STM32F1 HAL
 *  1.) Выбрать таймер для OneWire
 *  2.) Замкнуть канал 1 и канал 2
 *  3.) В Cube настроить работу таймера с частотой 1 МГц
 *      режим с периодом 65535 мкс
 *  4.) Канал 1 включить в режим PWM output, CH1 polarity Low, 
		Output compare preload: Disable
 *  5.) Канал 2 включить в режим InputCapture
 *  6.) Канал 3 включить в режим OutputCompare no output 
 *  7.) выход канала CH1 настроить как открытый сток
 *  8.) Инициализировать класс вызвав Init
 ********************************************************/

class OneWire : public OneWireBase
{
public:
	OneWire();
	void Init(TIM_HandleTypeDef& htim);
	void Test();
	void send_bit(bool bit) override;
	bool read_bit() override;
	bool reset() override;
private:
	void Wait();
	void stop_timer();
	void start_timer();
	void set_pulse_lenght(int pulse_time_us, int total_time);
public:
	
private:
	TIM_HandleTypeDef* _htim;
	//volatile bool complete;
	//volatile bool last_read_bit;
	//volatile int _max_timer_value;
};
