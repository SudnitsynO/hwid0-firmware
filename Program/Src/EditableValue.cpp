#include <EditableValue.h>
#include <string>
#include <algorithm>
#include "Format.h"

using namespace std;

EditableValue::EditableValue(int value, int max_value, int min_value, int step, int precision, const char* message):
	_message(message),
	_value(value),
	_max_value(max_value),
	_min_value(min_value),
	_step(step),
	_precision(precision)
{
	_pow = 1;
	for (int i = 0; i < _precision; i++)
	{
		_pow *= 10;
	}
	validate_value();
}

std::string EditableValue::to_string() const
{
	ToString str(20);
	str.Convert(_value / _pow);
	string result(str.LeftAllign());
	if (_precision > 0)
	{
		result.push_back(',');
		str.Convert(_value % _pow);
		result.append(str.LeftAllign());
	}
	return result;
}

float EditableValue::to_float() const
{
	return static_cast<float>(_value) / _pow;
}

int EditableValue::get() const
{
	return _value;
}

void EditableValue::validate_value()
{
	if (_value > _max_value)
		_value = _max_value;
	if (_value < _min_value)
		_value = _min_value;
}

void EditableValue::increase()
{
	_value += _step;
	validate_value();
}

void EditableValue::decrease()
{
	_value -= _step;
	validate_value();
}

void EditableValue::set_value(const int new_value)
{
	_value = new_value;
}
