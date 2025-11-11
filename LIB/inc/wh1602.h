#pragma once
#include "TextDisplay.h"
#include "gpio_interface.h"
#include "delay.h"

static const char LCD_Table[] =
//таблица для преобразование букв русского алфавита
//начинается с 0xC0
{
	'A',
	0xA0,
	'B',
	0xA1,
	0xE0,
	'E',
	0xA3,
	0xA4,
	0xA5,
	0xA6,
	'K',
	0xA7,
	'M',
	'H',
	'O',
	0xA8,
	'P',
	'C',
	'T',
	0xA9,
	0xAA,
	'X',
	0xE1,
	0xAB,
	0xAC,
	0xE2,
	0xAD,
	0xAE,
	'b',
	0xAF,
	0xB0,
	0xB1,
	'a',
	0xB2,
	0xB3,
	0xB4,
	0xE3,
	'e',
	0xB6,
	0xB7,
	0xB8,
	0xB9,
	0xBA,
	0xBB,
	0xBC,
	0xBD,
	'o',
	0xBE,
	'p',
	'c',
	0xBF,
	'y',
	0xE4,
	'x',
	0xE5,
	0xC0,
	0xC1,
	0xE6,
	0xC2,
	0xC3,
	0xC4,
	0xC5,
	0xC6,
	0xC7
};

template<class DataPort, class RW_Pin, class RS_Pin, class E_Pin>
class WH1602 : public TextDisplay
{
public:
	//производит инициализацию класса-драйвера дисплея
	void Init();
	virtual uint16_t ColumnCount();
	virtual uint16_t RowCount();
	virtual void SetCursorPos(uint16_t column, uint16_t row);
	virtual void SetCursorType(CursorType cursor_type);
	virtual void putchar(char chr);
	virtual void Clear();
	bool failure;	//true, когда дисплей не найден
private:
	bool _wait();	//ожидает готовность дисплея возвратит false в случае таймаута
	uint8_t _read();//читает байт с дисплея
	void _write(uint8_t data);	//записывает байт в дисплей
	void _send_cmd(uint8_t cmd);	//посылает команду
};

template <class DataPort, class RW_Pin, class RS_Pin, class E_Pin>
void WH1602<DataPort, RW_Pin, RS_Pin, E_Pin>::Init()
{
	failure = false;
	DataPort::SetMode(Output);
	DataPort::Write(0);
	RW_Pin::Reset();
	RS_Pin::Reset();
	E_Pin::Reset();
	Delay_ms(40);
	_write(0x30);		//function set
	Delay_us(37);
	_write(0x30);		//function set
	Delay_us(37);
	_write(0x38);
	Delay_us(37);
	_write(0x80);	//display on/off
	Delay_us(37);
	_write(0x01);	//display clear
	Delay_ms(2);
	_write(0x06);	//entry mode set

	if (!_wait()) return;


	//	_send_cmd(0x38);	//font, 8-bit, 2 line;
	_send_cmd(0x0F);	//display on
	_send_cmd(0x01);	//display clear

}

template <class DataPort, class RW_Pin, class RS_Pin, class E_Pin>
uint16_t WH1602<DataPort, RW_Pin, RS_Pin, E_Pin>::ColumnCount()
{
	return 16;
}

template <class DataPort, class RW_Pin, class RS_Pin, class E_Pin>
uint16_t WH1602<DataPort, RW_Pin, RS_Pin, E_Pin>::RowCount()
{
	return 2;
}

template <class DataPort, class RW_Pin, class RS_Pin, class E_Pin>
void WH1602<DataPort, RW_Pin, RS_Pin, E_Pin>::SetCursorPos(uint16_t column, uint16_t row)
{
	_send_cmd(0x80 | (row * 0x40 + column));
}

template <class DataPort, class RW_Pin, class RS_Pin, class E_Pin>
void WH1602<DataPort, RW_Pin, RS_Pin, E_Pin>::SetCursorType(CursorType cursor_type)
{
		uint8_t cmd = 0x0C;
		switch (cursor_type)
		{
		case CURSOR_OFF:
			cmd |= 0;
			break;
		case CURSOR_ON:
			cmd |= 2;
			break;
		case CURSOR_BLINKING:
			cmd |= 3;
			break;
		default:
			break;
		}
		_send_cmd(cmd);
	}

template <class DataPort, class RW_Pin, class RS_Pin, class E_Pin>
void WH1602<DataPort, RW_Pin, RS_Pin, E_Pin>::putchar(char chr)
{
	_wait();
	RS_Pin::Set();
	_write(chr < 0xC0 ? chr : LCD_Table[chr - 0xC0]);
}

template <class DataPort, class RW_Pin, class RS_Pin, class E_Pin>
void WH1602<DataPort, RW_Pin, RS_Pin, E_Pin>::Clear()
{
	_send_cmd(1);
}


template <class DataPort, class RW_Pin, class RS_Pin, class E_Pin>
bool WH1602<DataPort, RW_Pin, RS_Pin, E_Pin>::_wait()
{
	RS_Pin::Reset();
	int n = 1000;
	while (_read() & 0x80)
	{
		if(!(n--))
		{
			failure = true;
			return false;
		}
	}
	return true;
}

template <class DataPort, class RW_Pin, class RS_Pin, class E_Pin>
uint8_t WH1602<DataPort, RW_Pin, RS_Pin, E_Pin>::_read()
{
	uint8_t data = 0;
	RW_Pin::Set();
	Delay_us(1);
	DataPort::SetMode(Input);
	E_Pin::Set();
	Delay_us(1);
	data = DataPort::Read();
	E_Pin::Reset();
	Delay_us(1);
	return data;
}

template <class DataPort, class RW_Pin, class RS_Pin, class E_Pin>
void WH1602<DataPort, RW_Pin, RS_Pin, E_Pin>::_write(uint8_t data)
{
	RW_Pin::Reset();
	DataPort::SetMode(Output);
	DataPort::Write(data);
	Delay_us(1);
	E_Pin::Set();
	Delay_us(1);
	E_Pin::Reset();
	Delay_us(1);	//?
}

template <class DataPort, class RW_Pin, class RS_Pin, class E_Pin>
void WH1602<DataPort, RW_Pin, RS_Pin, E_Pin>::_send_cmd(uint8_t cmd)
{
	_wait();
	RS_Pin::Reset();
	_write(cmd);
}



