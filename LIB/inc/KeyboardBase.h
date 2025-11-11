#pragma once
class Keyboard
{
public:
	virtual void SetTimeout(int timeout) = 0;	//set key read timeout
	virtual void Scan() = 0;	//scan keyboard buttons (100Hz)
	virtual int ReadKey() = 0;	//read pressed key (-1 is empty)
};