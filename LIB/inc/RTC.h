#pragma once
#include "DateTime.h"

class RTC_DateTime
{
public:
	void init();
	DateTime get_time();
	void set_time(DateTime& time);

private:
	void wait_to_read();
	void wait_after_write();
	void unlock_write();
	void lock_write();
};