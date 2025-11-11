#pragma once

class ToString
{
public:
	explicit ToString(int string_lenght);
	~ToString();
	void Convert(double value, int precision, bool exp_format);
	void Convert(long long int_value, bool zeros = false);
	char * RightAllign();
	char * LeftAllign();
private:
	int _lenght;
	char * string;

};
