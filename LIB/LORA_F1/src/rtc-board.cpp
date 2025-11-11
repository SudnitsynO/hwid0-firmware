#include "rtc-board.h"

#include <cmsis_os2.h>
#include <list>
#include <cstdint>
#include "LIB/LORA_F1/inc/delay.h"

volatile uint64_t time;
volatile uint64_t alarm;
volatile uint64_t reference;
volatile uint64_t alarm_set_time;
volatile bool alarm_is_active;
volatile bool init_conplete = false;

void RtcInit()
{
	time = 0;
	alarm = 0;
	reference = 0;
	alarm_set_time = 0;
	alarm_is_active = false;
	init_conplete = true;
}

uint32_t RtcGetMinimumTimeout()
{
	return 1;
}

uint32_t RtcMs2Tick(TimerTime_t milliseconds)
{
	return milliseconds;
}

TimerTime_t RtcTick2Ms(uint32_t tick)
{
	return  tick;
}

void RtcDelayMs(TimerTime_t milliseconds)
{
	osDelay(milliseconds);
}

void RtcSetAlarm(uint32_t timeout)
{
	alarm_set_time = reference;
	alarm = time + timeout;
	alarm_is_active = true;
}

void RtcStopAlarm()
{
	alarm_is_active = false;
}

void RtcStartAlarm(uint32_t timeout)
{
	alarm_set_time = reference;
	alarm = reference + timeout;
	alarm_is_active = true;
}

uint32_t RtcSetTimerContext()
{
	reference = time;
	return reference;
}

uint32_t RtcGetTimerContext()
{
	return reference;
}

uint32_t RtcGetCalendarTime(uint16_t* milliseconds)
{
	return time / 1000;
}

uint32_t RtcGetTimerValue()
{
	return time;
}

uint32_t RtcGetTimerElapsedTime()
{
	//uint32_t et = alarm - time;
	//if (et & (1UL << 31))
		return 0;
	//return  et;
}

uint32_t bk1;

uint32_t bk2;

void RtcBkupWrite(uint32_t data0, uint32_t data1)
{
	bk1 = data0;
	bk2 = data1;
}

void RtcBkupRead(uint32_t* data0, uint32_t* data1)
{
	*data0 = bk1;
	*data1 = bk2;
}

void RtcProcess()
{
	time++;
	if(alarm_is_active)
		if((alarm - time) & (1ULL << 63))
		{
			alarm_is_active = false;
			TimerIrqHandler();
		}
}

TimerTime_t RtcTempCompensation(TimerTime_t period, float temperature)
{
	return  period;
}

void DelayMs(uint32_t ms)
{
	osDelay(ms);
}