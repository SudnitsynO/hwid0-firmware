#include "OneWireF0.h"
#include <cstdint>

OneWire::OneWire(): _htim(nullptr)
{
}

void OneWire::Init(TIM_HandleTypeDef& htim)
{
	_htim = &htim;
	HAL_TIM_PWM_Start(&htim,TIM_CHANNEL_1);
	HAL_TIM_IC_Start(&htim, TIM_CHANNEL_2);
	HAL_TIM_OC_Start(&htim, TIM_CHANNEL_3);
}
/*
void OneWire::OutputCompareISR(TIM_HandleTypeDef* htim)
{
	//interrupt
	if (htim != _htim) return;
	if (htim->Channel != HAL_TIM_ACTIVE_CHANNEL_3) return;
	int k = htim->Instance->CNT;
	if (k > _max_timer_value) return;
	StopTimer();
	complete = true;
}

void OneWire::InputCaptureISR(TIM_HandleTypeDef* htim)
{
	//interrupt
	if (htim != _htim) return;
	if (htim->Channel != HAL_TIM_ACTIVE_CHANNEL_2) return;
	if (htim->Instance->CCR2 > 20)
		last_read_bit = false;
	else last_read_bit = true;
}
*/

 uint64_t id1 = 0;
 uint64_t id2 = 0;

void OneWire::Test()
{
	reset_search();
	search_next(id2);
}

void OneWire::send_bit(bool bit)
{
	set_pulse_lenght(bit ? 5 : 60, 120);
	start_timer();
	Wait();	//ждем завершения операции
	stop_timer();
}

bool OneWire::read_bit()
{
	set_pulse_lenght(5, 120);
	start_timer();
	Wait();	//ждем завершения операции
	bool bit = _htim->Instance->CCR2 > 15 ? false : true;
	stop_timer();
	return bit;
}

bool OneWire::reset()
{
	set_pulse_lenght(600,1000);
	start_timer();
	Wait();	//ждем завершения операции
	bool bit = _htim->Instance->CCR2 > 650 ? true : false;
	stop_timer();
	return bit;
}


void OneWire::Wait()
{
	while (((_htim->Instance->SR & TIM_SR_CC3IF) == 0) && (_htim->Instance->CR1 & TIM_CR1_CEN));
}

void OneWire::stop_timer()
{
	_htim->Instance->CR1 &= ~TIM_CR1_CEN;
	_htim->Instance->CNT = -10;
	_htim->Instance->SR &= ~TIM_SR_CC3IF;
	_htim->Instance->SR &= ~TIM_SR_CC2IF;
}

void OneWire::start_timer()
{
	_htim->Instance->CR1 |= TIM_CR1_CEN;
}

void OneWire::set_pulse_lenght(int pulse_time_us, int total_time)
{
	stop_timer();	//timer off
	_htim->Instance->CCR1 = pulse_time_us;		//pulse time
	_htim->Instance->CCR3 = total_time;

	_htim->Instance->EGR |= TIM_EGR_UG;
}
