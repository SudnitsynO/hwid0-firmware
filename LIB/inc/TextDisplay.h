#pragma once
#include "stdint.h"

class TextDisplay
{
public:
	enum CursorType
	{
		CURSOR_OFF,
		CURSOR_ON,
		CURSOR_BLINKING
	};
	virtual void SetCursorPos(uint16_t column, uint16_t row) = 0;
	virtual void SetCursorType(CursorType cursor_type) = 0;
	virtual uint16_t ColumnCount() = 0; //возвращает число столбцов экрана
	virtual uint16_t RowCount() = 0;	//возвращает число строк экрана
	virtual void putchar(char chr) = 0;	//выводит символ на дисплей
	virtual void Clear() =0;	//очистка дисплея
	virtual void WriteString(const char * str)
	{
		while (*str != 0)
		{
			putchar(*str);
			str++;
		}
	}
};