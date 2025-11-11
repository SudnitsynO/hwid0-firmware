#pragma once

class MutexBase
{
public:
	virtual void lock() = 0;	//захватит мютекс возвратит true
	virtual bool lock(int timeout) = 0;	//пытается захватить мютекс в течении времени timeout(мс),вернет false если не удалось
	virtual void release() = 0;	//освобождает мьютекс
private:
};