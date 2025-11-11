#pragma once
#include "KeyboardBase.h"
#include "gpio_stm32f1.h"
#include "FreeRTOS.h"
#include "queue.h"


template <class VirtualPort, uint16_t InitState, bool Repeat,int RepeatPeriod = 20>
class StaticButtonKeyboard : public Keyboard
{
public:
	void Init(int buffer_length);
	// Унаследовано через Keyboard
	virtual void SetTimeout(int timeout) override;
	virtual void Scan() override;
	virtual int ReadKey() override;
private:
	uint16_t old_state;
	QueueHandle_t rx_queue;
	int timer;
	int _timeout;
};

template <class VirtualPort, uint16_t InitState, bool Repeat, int RepeatPeriod>
inline void StaticButtonKeyboard<VirtualPort, InitState, Repeat, RepeatPeriod>::Init(int buffer_length)
{
//	_buffer_length = 
	rx_queue = xQueueCreate(buffer_length, sizeof(uint16_t));
	_timeout = 0;
}

// Унаследовано через Keyboard
template <class VirtualPort, uint16_t InitState, bool Repeat, int RepeatPeriod >
inline void StaticButtonKeyboard<VirtualPort, InitState, Repeat, RepeatPeriod>::SetTimeout(int timeout)
{
	_timeout = timeout;
}

template <class VirtualPort, uint16_t InitState, bool Repeat, int RepeatPeriod>
inline void StaticButtonKeyboard<VirtualPort, InitState, Repeat, RepeatPeriod>::Scan()
{
	uint16_t new_state = VirtualPort::Read() ^ InitState;	//read new port state
	uint16_t xor_state = new_state ^ old_state; //calculate difference
	old_state = new_state;	//save current state;
	//uint16_t key_code;
	if (new_state == 0)
	{
		//no key pressed
		timer = 0;
/*		if (xor_state == 0)
			return;					//return, if no difference
		else
			old_state = new_state;*/
	}
	else
	{
		uint16_t new_pressed = xor_state & new_state;	//calculate, how button was pressed
		for (int i = 0; i < 16; i++)	//scan for all 16 buttons
		{
			if (new_pressed & 1)	//if current button pressed and changed state
				xQueueSend(rx_queue, &i, 0);		//add key code to buffer
			new_pressed >>= 1;
		}
		timer++;
		if (timer == 100)
		{
			for (int i = 0x80; i < (0x80 + 16); i++)	//scan for all 16 buttons
			{
				if (new_state & 1)	//if current button pressed and changed state
					xQueueSend(rx_queue, &i, 0);		//add key code to buffer
				new_state >>= 1;
			}
			if (Repeat) timer -= RepeatPeriod;	//10 times in sec
		}
		else if (timer > 100)
		{
			timer = 101;
		}
	}

}

template <class VirtualPort, uint16_t InitState, bool Repeat, int RepeatPeriod>
inline int StaticButtonKeyboard<VirtualPort, InitState, Repeat,RepeatPeriod>::ReadKey()
{
	uint16_t buffer;
	if (xQueueReceive(rx_queue, &buffer, _timeout) == pdTRUE)
		return buffer;
	return -1;
}
