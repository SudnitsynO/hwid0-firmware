#include "Format.h"
#include <string.h>

ToString::ToString(int string_lenght)
{
	_lenght = string_lenght;
	string = new char[string_lenght+1];
}

ToString::~ToString()
{
	delete[] string;
}

void ToString::Convert(double value, int precision, bool exp_format)
{
	bool sign = value < 0;

	if (sign)
	{
		value = -value;
	}

	if (exp_format)
	{
		int p = 0;
		while (value >= 10)
		{
			value /= 10;
			p++;
		}
		while (value < 1)
		{
			value *= 10;
			p--;
		}
		Convert(p);
		int pos = 0;
		for (int i = 0; i < _lenght; i++)
		{
			if (string[i] != ' ')
			{
				pos = i;
				break;
			}
		}
		if (pos > 4)
		{
			if (p < 0)
			{
				string[pos - 1] = 'E';
				pos -= 1;
			}
			else
			{
				string[pos - 1] = '+';
				string[pos - 2] = 'E';
				pos -= 2;
			}
			ToString temp(pos);
			temp.Convert(sign ? -value : value, precision, false);
			memcpy(string, temp.string, pos);
		}
		else
		string[pos] = '#';
	}
	else
	{
		for (size_t i = 0; i < precision; i++)
		{
			value *= 10;
		}
		value += .5;
		long long v = value;
		if (precision > 0)
		{
			int i = _lenght-1;
			int p = 0;
			while (i >= 0)
			{
				if (p == precision)
				{
					string[i] = '.';
					p++;
				}
				else
				{
					string[i] = v % 10 + '0';

					if (v == 0)
					{
						if (p > precision + 1)
						{
							if (sign)
							{
								string[i] = '-';
								sign = false;
							}
							else
							{
								string[i] = ' ';
							}
						}
					}
					v /= 10;
					p++;
				}

				i--;
			}
		}
		else
		{
			int i = _lenght - 1;
			int p = 0;
			while (i >= 0)
			{
				string[i] = v % 10 + '0';
				if (v == 0)
				{
					if (p > 0)
					{
						if (sign)
						{
							string[i] = '-';
							sign = false;
						}
						else
						{
							string[i] = ' ';
						}
					}
				}
				v /= 10;
				p++;
				i--;
			}
		}
		string[_lenght] = 0;
	}
}

void ToString::Convert(long long int_value, bool zeros)
{
	bool sign = int_value < 0;

	if (sign)
	{
		int_value = -int_value;
	}

	int i = _lenght - 1;

	while(i >= 0)
	{
		if (int_value || (i == _lenght - 1) )
		{
			string[i] = '0' + (int_value % 10);
			int_value /= 10;
		}
		else if (sign)
		{
			string[i] = '-';
			sign = false;
		}
		else
			string[i] = zeros ? '0':' ';
		i--;
	}

	string[_lenght] = 0;
}

char* ToString::RightAllign()
{
	return string;
}

char* ToString::LeftAllign()
{
	for (size_t i = 0; i < _lenght; i++)
	{
		if (string[i] != ' ')
		{
			return &string[i];
		}
	}
	return string;
}
