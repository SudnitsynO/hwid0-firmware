#include "RTC.h"
#include "stm32f1xx_hal.h"

void RTC_DateTime::init()
{
	//test rtc settings
	wait_to_read();

}

DateTime RTC_DateTime::get_time()
{
	wait_to_read();
	uint64_t sec = RTC->CNTH;
	sec = (sec << 16) | RTC->CNTL;

	uint64_t div = RTC->DIVH;
	div = (div << 16) | RTC->DIVL;

	DateTime time(sec * 1000 + (div * 1000 / 0x8FFF));
	return  time;
}

void RTC_DateTime::set_time(DateTime& time)
{
	auto total_seconds = time.total_milliseconds() / 1000;
	unlock_write();
	RTC->CNTH = total_seconds >> 16;
	RTC->CNTL = total_seconds;
	lock_write();
	wait_after_write();
}

void RTC_DateTime::wait_to_read()
{
	RTC->CRL &= ~RTC_CRL_RSF;
	while ((RTC->CRL & RTC_CRL_RSF) == 0);
}

void RTC_DateTime::wait_after_write()
{
	while ((RTC->CRL & RTC_CRL_RTOFF) == 0);
}

void RTC_DateTime::unlock_write()
{
	RTC->CRL |= RTC_CRL_CNF;
}

void RTC_DateTime::lock_write()
{
	RTC->CRL &= ~RTC_CRL_CNF;
}
