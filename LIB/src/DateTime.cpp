#include "DateTime.h"
#include "Format.h"

DayOfWeek DateTime::day_of_week()
{
	return static_cast<DayOfWeek>(total_days() % 7);
}

string DateTime::to_string_date()
{
	ToString _year(2);
	ToString _month(2);
	ToString _day(2);
	_year.Convert(year(),true);
	_month.Convert(month(),true);
	_day.Convert(day(),true);
	string response;
	response.append(_day.RightAllign());
	response.append(".");
	response.append(_month.RightAllign());
	response.append(".");
	response.append(_year.RightAllign());
	return response;
}

string DateTime::to_string_time(bool with_seconds)
{
	ToString _hour(2);
	ToString _minute(2);
	_hour.Convert(hour(), true);
	_minute.Convert(minute(), true);
	string response;
	response.append(_hour.RightAllign());
	response.append(":");
	response.append(_minute.RightAllign());
	if (with_seconds)
	{
		ToString _seconds(2);
		_seconds.Convert(second(), true);
		response.append(":");
		response.append(_seconds.RightAllign());
	}
	return response;

}

string DateTime::to_string(bool with_seconds)
{
	string response = to_string_date();
	response.append(" ");
	response.append(to_string_time(with_seconds));
	return response;
}

DateTime::DateTime(): _milliseconds(7305ULL * 24 * 60 * 60 * 1000 - 1)
{
}

DateTime::DateTime(int seconds, int minutes, int hours, int day, int month, int year)
{
	if ((seconds < 0) || (seconds > 59) || (minutes < 0) || (minutes > 59) || (hours < 0) || (hours > 23) || (day < 1)
		|| (day > 31) || (month < 1) || (month > 12) || (year < 1) || (year > 100))
	{
		_milliseconds = 0;
		return;
	}
	//_milliseconds = (((hours * 60) + minutes) * 60 + seconds) * 1000;
	int64_t n = (year - 1) / 4;	//количество циклов прошло
	const bool leap = year % 4 == 0;	//текущий год високосный
	//переходим на дни
	n = n * (365 * 4 + 1) + ((year - 1) % 4) * 365;
	if (month > 1)		//январь
		n += 31;
	if (month > 2) //февраль
		n += 28;
	if (month > 3) //март
		n += 31;
	if (month > 4) //апрель
		n += 30;
	if (month > 5) //май
		n += 31;
	if (month > 6) //июнь
		n += 30;
	if (month > 7) //июль
		n += 31;
	if (month > 8) //август
		n += 31;
	if (month > 9) //сентябрь
		n += 30;
	if (month > 10) //октябрь
		n += 31;
	if (month > 11) //ноябрь
		n += 30;

	if ((month > 2) && leap)
		n++;	//29 февраля

	n += day - 1;
	//переходим на часы
	n = n * 24 + hours;
	//переходим на минуты
	n = n * 60 + minutes;
	//переходим на секунды
	n = n * 60 + seconds;
	_milliseconds = n * 1000;
}

DateTime::DateTime(const uint64_t milliseconds): _milliseconds(milliseconds)
{
}

int DateTime::day()
{
	auto d = day_of_year();
	if (d <= 31)
		return d;	//январь
	else if ((d -= 31) <= (is_leap_year() ? 29 : 28))
		return d;	//февраль
	else if ((d -= is_leap_year() ? 29 : 28) <= 31)
		return d;	//март
	else if ((d -= 31) <= 30)
		return d;	//апрель
	else if ((d -= 30) <= 31)
		return d;	//май
	else if ((d -= 31) <= 30)
		return d;	//июнь
	else if ((d -= 30) <= 31)
		return d;	//июль	
	else if ((d -= 31) <= 31)
		return d;	//август
	else if ((d -= 31) <= 30)
		return d;	//сентябрь
	else if ((d -= 30) <= 31)
		return d;	//октябрь
	else if ((d -= 31) <= 30)
		return d;	//ноябрь
	else if ((d -= 30) <= 31)
		return d;	//декабрь
	return 0;
}

DateTime::DateTime(const DateTime& other): _milliseconds(other._milliseconds)
{
}

int DateTime::day_of_year()
{
	int d = total_days() - (year() - 1) * 365;
	d -= (year() - 1) / 4;	//скорректированное кол-во дней
	return d + 1;
}

int DateTime::year()
{
	const int n = total_days() / (365 * 4 + 1);	//количество циклов по 4 года
	const int d = total_days() - n * (365 * 4 + 1);//прошло дней в текущем цикле
	const auto y = d / 365;	//прошло лет внутри цикла
	if (y == 4)	//последний день цикла
		return n * 4 + 3 + 1;
	else
		return n * 4 + y + 1;
}

bool DateTime::is_leap_year()
{
	return year() % 4 == 0;
}

int DateTime::hour() const
{
	return _milliseconds / 1000 / 60 / 60 % 24;
}

int DateTime::millisecond() const
{
	return _milliseconds % 1000;
}

int DateTime::minute() const
{
	return _milliseconds / 1000 / 60 % 60;
}

int DateTime::month()
{
	auto d = day_of_year();
	if (d <= 31)
		return 1;	//январь
	else if ((d -= 31) <= (is_leap_year() ? 29 : 28))
		return 2;	//февраль
	else if ((d -= is_leap_year() ? 29 : 28) <= 31)
		return 3;	//март
	else if ((d -= 31) <= 30)
		return 4;	//апрель
	else if ((d -= 30) <= 31)
		return 5;	//май
	else if ((d -= 31) <= 30)
		return 6;	//июнь
	else if ((d -= 30) <= 31)
		return 7;	//июль	
	else if ((d -= 31) <= 31)
		return 8;	//август
	else if ((d -= 31) <= 30)
		return 9;	//сентябрь
	else if ((d -= 30) <= 31)
		return 10;	//октябрь
	else if ((d -= 31) <= 30)
		return 11;	//ноябрь
	else if ((d -= 30) <= 31)
		return 12;	//декабрь
	return 0;
}

int DateTime::second() const
{
	return _milliseconds / 1000 % 60;
}

uint64_t DateTime::total_milliseconds() const
{
	return _milliseconds;
}

DateTime& DateTime::operator=(const DateTime& rv)
{
	if (this != &rv)
	{
		_milliseconds = rv._milliseconds;
	}
	return *this;
}

uint32_t DateTime::total_days()
{
	return _milliseconds / 1000 / 60 / 60 / 24;
}

DateTime operator+(const DateTime& lv, const DateTime& rv)
{
	return {lv._milliseconds + rv._milliseconds};
}

DateTime operator+=(DateTime& lv, const DateTime& rv)
{
	return {lv._milliseconds += rv._milliseconds};
}

bool operator==(const DateTime& lv, const DateTime& rv)
{
	return lv._milliseconds == rv._milliseconds;
}

bool operator>(const DateTime& lv, const DateTime& rv)
{
	return lv._milliseconds > rv._milliseconds;
}

bool operator<(const DateTime& lv, const DateTime& rv)
{
	return lv._milliseconds < rv._milliseconds;
}

bool operator<=(const DateTime& lv, const DateTime& rv)
{
	return lv._milliseconds <= rv._milliseconds;
}

bool operator>=(const DateTime& lv, const DateTime& rv)
{
	return lv._milliseconds >= rv._milliseconds;
}
