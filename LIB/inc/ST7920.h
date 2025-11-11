#pragma once

#include "string.h"
#include <algorithm>
//#include "graphic_display.h"
#include "TextDisplay.h"
#include "gpio_interface.h"
#include "delay.h"

#define LCD_DELAY 1

extern const char font5x8[][5];
//IoPort &data, IoBit &RS_Line, IoBit &RW_Line, IoBit &E_Line, IoBit &Reset)
template <class DataPort, class RS_Pin, class RW_Pin, class E_Pin, class ResetPin >
class ST7920 : public GraphicDisplay, public TextDisplay
{
public:
private:
	volatile bool valid;
	static constexpr size_t VRAM_SIZE = 128 * 64 / 8;
	uint8_t VRAM[VRAM_SIZE];
	int invX1;
	int invY1;
	int invX2;
	int invY2;
	int _row;
	int _column;
	int cx;
	int cy;
  CursorType _cursor_type;

public:
	ST7920()
	{
		valid = true;
	}

	virtual void SetPixel(int x, int y, uint32_t Color)
	{
		int y1 = (y & 0x07);
		int Line = (y & 0xF8) >> 3;
		int a = Line * 128 + x;
		Color &= 1;
		int c = Color << y1;
		VRAM[a] = (VRAM[a] & ~(1 << y1)) | c;
		Invalidate(x, y, x, y);
	}

	virtual uint32_t GetPixel(int x, int y)
	{
		int y1 = (y & 0x07);
		int Line = (y & 0xF8) >> 3;
		int a = Line * 128 + x;
		uint32_t color = VRAM[a] & (1 << y1) ? 1 : 0;
		return color;
	}

	virtual int Width()
	{
		return 128;
	}

	virtual int Height()
	{
		return 64;
	}

	void virtual DrawText(int x, int y, uint32_t Color, char* text)
	{
		Color &= 1;
		int startLine = y / 8;
		int y_shift = y % 8;
		int a = startLine * Width() + x;
		while (*text != 0)
		{
			for (int i = 0; i < 5; i++)
			{
				if (a + i + Width() >= VRAM_SIZE) return;
				uint8_t mask = font5x8[*text][i] << y_shift;
				VRAM[a + i] = VRAM[a + i] & ~mask | (Color == 1 ? mask : 0);
				mask = font5x8[*text][i] >> (8 - y_shift);
				VRAM[a + i + Width()] = VRAM[a + i + Width()] & ~mask | (Color == 1 ? mask : 0);
			}
			a += 6;
			text++;
			Invalidate(x, y, x + 6, y + 8);
			x += 6;
		}
	}

	void virtual UpdateDisplay()
	{
		if (valid)
		{
			invX1 = 0;
			invY1 = 0;
			invX2 = Width() - 1;
			invY2 = Height() - 1;
		}

		//���������� ����������
		for (int y = invY1; y <= invY2; y++)
		{
			int x = invX1 & 0xF0;
			//����� ������ � ���
			int line = y / 8;
			//����� ���� � ������ ���
			int bit = y % 8;

			//��������� ������ ������ �������
			if (y < 32)
				LCDCmdOut(0x80 + y);
			else
				LCDCmdOut(0x80 + y - 32);
			//��������� ������ �� �����������
			if (y < 32)
				LCDCmdOut(0x80 + x / 16);
			else
				LCDCmdOut(0x80 + x / 16 + 8);
			while (x < invX2)
			{
				//�������� ������� 16 ���
				int to_send = 0;
				for (int i = 0; i < 16; i++)
					to_send = (VRAM[x + i + line * 128] & (1 << bit)) ? to_send | (1 << (15 - i)) : to_send;
				LCDDataOut(to_send >> 8);
				LCDDataOut(to_send);
				x += 16;
			}
		}
	}

	void virtual Invalidate(int x1, int y1, int x2, int y2)
	{
		if (x1 > x2)
		{
			std::swap(x1, x2);
		}
		if (y1 > y2)
		{
			std::swap(y1, y2);
		}
		if (valid)
		{
			invX1 = x1;
			invX2 = x2;
			invY1 = y1;
			invY2 = y2;
			invX1 = x1;
			valid = false;
		}
		else
		{
			//if (invX1 > x1) invX1 = x1;
			invX1 = std::min(invX1, x1);
			//if (invX2 < x2) invX2 = x2;
			invX2 = std::max(invX2, x2);
			//if (invY1 > y1) invY1 = y1;
			invY1 = std::min(invY1, y1);
			//if (invY2 < y2) invY2 = y2;
			invY2 = std::max(invY2, y2);
		}
	}

	void virtual Clear()
	{
		memset(VRAM, 0, VRAM_SIZE);
		Invalidate(0, 0, Width() - 1, Height() - 1);
		_row = 0;
		_column = 0;
		cx = 0;
		cy = 0;
	}

	uint8_t StatusRead()
	{
		E_Pin::Reset();
		//	_CS_1->SetValue(CS_n == 1);
		//	_CS_2->SetValue(CS_n == 2);
		RW_Pin::Set();	//R
		DataPort::SetMode(Mode::Input);
		RS_Pin::Reset();	//I
		Delay_us(LCD_DELAY);
		E_Pin::Set();
		Delay_us(LCD_DELAY);
		uint8_t stat = DataPort::Read();
		E_Pin::Reset();
		return stat;
	}

	void WaitLCD()
	{
		while (StatusRead() & 0x80);
	}

	void LCDCmdOut(uint8_t cmd)
	{
		WaitLCD();
		E_Pin::Reset();
		RW_Pin::Reset();	//W
		DataPort::SetMode(Mode::Output);
		DataPort::Write(cmd);
		RS_Pin::Reset();	//I
		Delay_us(LCD_DELAY);
		E_Pin::Set();
		Delay_us(LCD_DELAY);
		E_Pin::Reset();
		Delay_us(LCD_DELAY);
	}

	void LCDDataOut(uint8_t data)
	{
		E_Pin::Reset();
		RW_Pin::Reset();	//W
		DataPort::SetMode(Mode::Output);
		DataPort::Write(data);
		RS_Pin::Set();	//D
		Delay_us(LCD_DELAY);
		E_Pin::Set();
		Delay_us(LCD_DELAY);
		E_Pin::Reset();
		Delay_us(LCD_DELAY);
	}

	void Init()
	{
		//_RS_Line->SetModeOutput();
		//_RW_Line->SetModeOutput();
		//_E_Line->SetModeOutput();
		//_Reset->SetModeOutput();

		E_Pin::Reset(); //E=0
		ResetPin::Reset(); //Reset
		Delay_ms(100);
		ResetPin::Set(); //Reset  = 1
		Delay_ms(100);
		LCDCmdOut(0x30); //Display on
		Delay_ms(100);
		LCDCmdOut(0x36); //Display on
		Delay_ms(100);
		LCDCmdOut(0x36); //Display on
		Delay_ms(100);
		//	LCDCmdOut(0x80); //Display on
		//	Task::Sleep(100);
		//	LCDCmdOut(0x80); //Display on
		//	Task::Sleep(100);
		//LCDCmdOut(0x0F); //Display on
		for (int y = 0; y < 32; y++)
		{
			LCDCmdOut(0x80 + y);
			//Task::Sleep(1);
			LCDCmdOut(0x80);
			//Task::Sleep(1);
			for (int x = 0; x < 16; x++)
			{
				LCDDataOut(0xAA);
				LCDDataOut(0xAA);
			}
		}


		//TextCursor.Mode = TC_VERTICAL;
		cx = 0;
		cy = 0;
		invX1 = 0;
		invY1 = 0;
		invX2 = Width() - 1;
		invY2 = Height() - 1;
		valid = true;
	}

	virtual void SetCursorPos(uint16_t column, uint16_t row)
	{
		_column = column;
		_row = row;
		if (_column >= ColumnCount())
		{
			_column = 0;
			_row++;
		}
		if (_row >= RowCount())
			return;
	}

	 virtual void SetCursorType(CursorType cursor_type)
	{
		_cursor_type = cursor_type;
	}

	virtual uint16_t ColumnCount()
	{
		return 21;
	}

	virtual uint16_t RowCount()
	{
		return 8;		
	}

	virtual void putchar(char chr)
	{
		if (chr == '\n')
		{
			_row++;
		}
		else if (chr == '\r')
			_column = 0;
		else
		{
			Invalidate(_column * 6, _row * 8, _column * 6 + 6, _row * 8 + 8);
			size_t a = _row * 128 + _column * 6;
			for (size_t i = 0; i < 5; i++)
			{
				VRAM[a++] = font5x8[chr][i];
			}
			_column++;
		}
		if (_column >= ColumnCount())
		{
			_column = 0;
			_row++;
		}
		if (_row >= RowCount())
			return;
	}

	void DrawCursor()
	{
		switch (_cursor_type)
		{
		case CURSOR_ON:
			VRAM[_row * 128 + _column * 6] ^= 0xFF;
			Invalidate(_column * 6, _row * 8, _column * 6, _row * 8 + 8);
			break;
		case CURSOR_BLINKING:
		{
			size_t a = _row * 128 + _column * 6;
			for (size_t i = 0; i < 6; i++)
			{
				VRAM[a++] ^= 0xFF;
			}
		}
		break;
		}
	}
};

#undef LCD_DELAY