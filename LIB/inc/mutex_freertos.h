#pragma once
//#include "yanisutil.h"
#include "FreeRTOS.h"
#include <semphr.h>
#include "MutexBase.h"

class Mutex : public MutexBase
{
public:
	Mutex();
	virtual void lock();
	virtual bool lock(int timeout);
	virtual void lock_from_isr();
	virtual void release();
	virtual void release_from_isr();
	~Mutex();
private:
	SemaphoreHandle_t _mutex_handle;
};
