#pragma once
#include <string>

class EditableValue
{
public:
	EditableValue(int value, int max_value, int min_value, int step, int precision, const char* message);

	std::string to_string() const;
	float to_float() const;
	int get() const;
	void validate_value();
	void increase();
	void decrease();
	void set_value(const int new_value);
    const char * const _message;
private:
	int _value;
	int _max_value;
	int _min_value;
	int _step;
	int _precision;
	int _pow;
};
