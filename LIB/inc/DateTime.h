#pragma once
#include <cstdint>
#include <string>

using namespace std;

enum class DayOfWeek : int
{
	Monday = 0,
	Tuesday = 1,
	Wednesday = 2,
	Thursday = 3,
	Friday = 4,
	Saturday = 5,
	Sunday = 6
};

class DateTime
{
public:
	
	DayOfWeek day_of_week();
	string to_string_date();
	string to_string_time(bool with_seconds = true);
	string to_string(bool with_seconds = true);
	DateTime();
	DateTime(int seconds, int minutes = 0, int hours = 0, int day = 1, int month = 1, int year = 1);
	DateTime(uint64_t milliseconds);
	DateTime(const DateTime& other);
	int day();
	int day_of_year();
	int year();
	bool is_leap_year();
	int hour() const;
	int millisecond() const;
	int minute() const;
	int month();
	int second() const;
	uint64_t total_milliseconds() const;
	/*operators*/
	friend DateTime operator+(const DateTime& lv, const DateTime& rv);
	friend DateTime operator+=(DateTime& lv, const DateTime& rv);
	friend bool operator==(const DateTime& lv, const DateTime& rv);
	friend bool operator>(const DateTime& lv, const DateTime& rv);
	friend bool operator<(const DateTime& lv, const DateTime& rv);
	friend bool operator<=(const DateTime& lv, const DateTime& rv);
	friend bool operator>=(const DateTime& lv, const DateTime& rv);
	DateTime& operator=(const DateTime& rv);
	//void Init(int seconds, int minutes, int hours, int day, int month, int year);
private:
	uint32_t total_days();	//прошло дней с начала столетия
	uint64_t _milliseconds;	//количество миллисекунд с начала столетия
};

