#pragma once
//#include "stdint.h"
#include "graphic.h"
#include "bitmap.h"
//#include "stddef.h"
#include "gpio_stm32f1.h"
#include "delay.h"
#include "fonts.h"
#include "font5x8.h"
#include "utf8_converter.h"
typedef volatile  uint16_t DisplayReg;

#define RED_MASK (0x001F)
#define GREEN_MASK (0x07E0)
#define BLUE_MASK (0xF800)

#define RGB(R,G,B) ((((R) >> 3) & 0x1F) | ((((G) >> 2) & 0x3F) << 5) | ((((B) >> 3) & 0x1F) << 11))

template < size_t REG_ADDR, size_t DATA_ADDR>
class SSD1289 : public Graphic<uint16_t>
{
public:
	void Init();
	virtual void SetPixel(int x, int y, uint16_t Color);
	virtual uint16_t GetPixel(int x, int y);
	virtual int Width();
	virtual int Height();
	virtual void DrawText(const char *text, uint16_t Color, Font<uint16_t> *font);
	virtual void DrawText(const char *text, uint16_t Color = 0xFFFF);
	virtual void SetTextPos(int x, int y);
	virtual void UpdateDisplay();
	virtual void Clear(uint16_t color = 0);
	void Fill(int x1, int y1, int x2, int y2, unsigned short Color);
	virtual void Invalidate(int x1, int y1, int x2, int y2);
	void WriteCommand(uint16_t index);
	void WriteData(uint16_t data);
	void DrawBitmap(int x, int y, Bitmap<uint16_t> *bitmap);
	void Test();
private:
	int x_text;
	int y_text;

	uint16_t ReadData();

	uint16_t ReadState();

	void Wait();

	int DrawSystemChar(int x, int y, uint16_t Color, char symbol);

//	uint16_t Lcd_Read_Reg(uint16_t reg_addr);
	void Lcd_Write_Reg(uint16_t reg, uint16_t value);
	volatile uint16_t test1, test2, test3,test4;
};

template<size_t REG_ADDR, size_t DATA_ADDR>
void SSD1289<REG_ADDR, DATA_ADDR>::Test()
{
	/*WriteCommand(0xC0);
	WriteData(0x23);

	WriteCommand(0xC1);
		WriteData(0x10);

	WriteCommand(0xC5);
	WriteData(0x3e);
	WriteData(0x28);

	WriteCommand(0xC7);
	WriteData(0x86);

	WriteCommand(0x3A);
	WriteData(0x55);

	WriteCommand(0xB1);
	WriteData(0x00);
	WriteData(0x18);

	WriteCommand(0xB6);
	WriteData(0x08);
	WriteData(0x82);
	WriteData(0x27);

	WriteCommand(0xF2);
	WriteData(0x00);

	WriteCommand(0x26);
	WriteData(0x01);

	WriteCommand(0xE0);
	WriteData(0x0F);
	WriteData(0x31);
	WriteData(0x2B);
	WriteData(0x0C);
	WriteData(0x0E);
	WriteData(0x08);
	WriteData(0x4E);
	WriteData(0xF1);
	WriteData(0x37);
	WriteData(0x07);
	WriteData(0x10);
	WriteData(0x03);
	WriteData(0x0E);
	WriteData(0x09);
	WriteData(0x00);

	WriteCommand(0xE1);
	WriteData(0x00);
	WriteData(0x0E);
	WriteData(0x14);
	WriteData(0x03);
	WriteData(0x11);
	WriteData(0x07);
	WriteData(0x31);
	WriteData(0xC1);
	WriteData(0x48);
	WriteData(0x08);
	WriteData(0x0F);
	WriteData(0x0C);
	WriteData(0x31);
	WriteData(0x36);
	WriteData(0x0F);

	WriteCommand(0x11);

	Delay_ms(120);

	WriteCommand(0x29);*/
}

template<size_t REG_ADDR, size_t DATA_ADDR>
void SSD1289<REG_ADDR, DATA_ADDR>::Init()
{

	x_text = 0;
	y_text = 0;
	//PA5::SetMode(Mode::Output);
	PB10::Reset();
	Delay_ms(120);
	PB10::Set();


	Lcd_Write_Reg(0X0007, 0X0021);   //����� ���������� � �������� ��������
	Lcd_Write_Reg(0X0000, 0X0001);
	Lcd_Write_Reg(0X0007, 0X0023);
	Lcd_Write_Reg(0X0010, 0X0000);
	Delay_ms(30);
	Lcd_Write_Reg(0X0007, 0X0033);
	Lcd_Write_Reg(0X0011, 0X6808);
	Lcd_Write_Reg(0X0002, 0X0600);
	//WriteCommand(0X0011);
	//test1 = ReadData();
	//test2 = ReadData();
	//test3 = ReadData();
	//test4 = ReadData();
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void SSD1289<REG_ADDR, DATA_ADDR>::SetPixel(int x, int y, uint16_t Color)
{
	Lcd_Write_Reg(0x004e, 239-y);
	Lcd_Write_Reg(0x004f, 319-x);
	Lcd_Write_Reg(0x0022, Color);
	//WriteData(Color);
	//Wait();
}

template <size_t REG_ADDR, size_t DATA_ADDR>
uint16_t SSD1289<REG_ADDR, DATA_ADDR>::GetPixel(int x, int y)
{
	return 0;
}

template <size_t REG_ADDR, size_t DATA_ADDR>
int SSD1289<REG_ADDR, DATA_ADDR>::Width()
{
	return 320;
}

template <size_t REG_ADDR, size_t DATA_ADDR>
int SSD1289<REG_ADDR, DATA_ADDR>::Height()
{
	return 240;
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void SSD1289<REG_ADDR, DATA_ADDR>::UpdateDisplay()
{
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void SSD1289<REG_ADDR, DATA_ADDR>::Clear(uint16_t color)
{
	Lcd_Write_Reg(0x004e, 239);
	Lcd_Write_Reg(0x004f, 319);
	WriteCommand(0x0022);
	for (size_t i = 0; i < (320 * 240); i++)
	{
		WriteData(color);
	}
	x_text = 0;
	y_text = 0;
	//Delay_ms(10);
	//SetPixel(10, 10, 0);
	//Delay_ms(10);
	//Lcd_Write_Reg(0x004e, 239);
	//Lcd_Write_Reg(0x004f, 319);
	//WriteCommand(0x0022);
	//Wait();
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void SSD1289<REG_ADDR, DATA_ADDR>::Fill(int x1, int y1, int x2, int y2, unsigned short Color)
{
	for (size_t y = y1; y <= y2; y++)
	{
		Lcd_Write_Reg(0x004e, 239 - y);
		Lcd_Write_Reg(0x004f, 319 - x1);
		WriteCommand(0x0022);
		for (size_t x = x1; x <= x2; x++)
		{
			WriteData(Color);
		}
	}
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void SSD1289<REG_ADDR, DATA_ADDR>::Invalidate(int x1, int y1, int x2, int y2)
{
}

template <size_t REG_ADDR, size_t DATA_ADDR>
inline void SSD1289<REG_ADDR, DATA_ADDR>::WriteCommand(uint16_t index)
{
	*(DisplayReg *)(REG_ADDR) = index;//((index << 8) & 0xFF00) ;
}

template <size_t REG_ADDR, size_t DATA_ADDR>
inline void SSD1289<REG_ADDR, DATA_ADDR>::WriteData(uint16_t data)
{
	*(DisplayReg *)(DATA_ADDR) = data;
}

template <size_t REG_ADDR, size_t DATA_ADDR>
inline uint16_t SSD1289<REG_ADDR, DATA_ADDR>::ReadData()
{
	return *(DisplayReg *)(DATA_ADDR);
}

template <size_t REG_ADDR, size_t DATA_ADDR>
inline uint16_t SSD1289<REG_ADDR, DATA_ADDR>::ReadState()
{
	return *(DisplayReg *)(REG_ADDR);
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void SSD1289<REG_ADDR, DATA_ADDR>::Lcd_Write_Reg(uint16_t reg, uint16_t value)
{
	WriteCommand(reg);
	WriteData(value);
}

template<size_t REG_ADDR, size_t DATA_ADDR>
inline void SSD1289<REG_ADDR, DATA_ADDR>::Wait()
{
	uint16_t test = 0;
	Lcd_Write_Reg(0x004e, 239);
	Lcd_Write_Reg(0x004f, 319);
	do
	{
		test = ReadState();
	} while (test != 0x8989);
}

template<size_t REG_ADDR, size_t DATA_ADDR>
inline void SSD1289<REG_ADDR, DATA_ADDR>::DrawBitmap(int x, int y, Bitmap<uint16_t> * bitmap)
{
	if (bitmap == nullptr) return;
	for (size_t i = 0; i < bitmap->Height(); i++)
	{
		//draw string
		Lcd_Write_Reg(0x004e, 239 - (y + i));
		Lcd_Write_Reg(0x004f, 319 - x);
		WriteCommand(0x0022);
		for (size_t j = 0; j < bitmap->Width(); j++)
		{
			WriteData(bitmap->bitmap[j + i * bitmap->Width()]);
		}
	}	
}

template<size_t REG_ADDR, size_t DATA_ADDR>
inline void SSD1289<REG_ADDR, DATA_ADDR>::DrawText(const char * text, uint16_t Color, Font<uint16_t> * font)
{
	auto str = convert_utf8_to_windows1251(text);
	//int index = 0;
	//convert_utf8_to_windows1251(text, str, 100);
	for ( auto ch : str)
	{
		if (font == nullptr)
		{
			x_text += DrawSystemChar(x_text, y_text, Color, ch);
			if ((x_text + 6) > Width())
			{
				x_text = 0;
				y_text += 8;
			}
		}
		else
		{
			x_text += font->DrawChar(ch, x_text, y_text, Color, this);
			if ((x_text + font->GetMaxWidth()) > Width())
			{
				x_text = 0;
				y_text += font->Heaght();
			}
		}
	}
}

template<size_t REG_ADDR, size_t DATA_ADDR>
inline void SSD1289<REG_ADDR, DATA_ADDR>::DrawText(const char * text, uint16_t Color)
{
	auto str = convert_utf8_to_windows1251(text);
	for (auto ch : str)
	{
		x_text += DrawSystemChar(x_text, y_text, Color, ch);
		if ((x_text + 6) > Width())
		{
			x_text = 0;
			y_text += 8;
		}
	}
}

template<size_t REG_ADDR, size_t DATA_ADDR>
inline void SSD1289<REG_ADDR, DATA_ADDR>::SetTextPos(int x, int y)
{
	x_text = x;
	y_text = y;
}

template<size_t REG_ADDR, size_t DATA_ADDR>
int SSD1289<REG_ADDR, DATA_ADDR>::DrawSystemChar(int x, int y, uint16_t Color, char symbol)
{
	//int startLine = y / 8;
	//int y_shift = y % 8;
	//int a = startLine * Width() + x;

	for (int i = 0; i < 5; i++)
	{
		uint8_t column = font5x8[symbol][i];
		for (size_t j = 0; j < 8; j++)
		{
			if (column & 1) SetPixel(x + i, y + j, Color);
			column >>= 1;
		}
		//if (a + i + Width() >= VRAM_SIZE) return;
		//uint8_t mask = font5x8[symbol][i] << y_shift;
		//VRAM[a + i] = VRAM[a + i] & ~mask | (Color == 1 ? mask : 0);
		//mask = font5x8[*text][i] >> (8 - y_shift);
		//VRAM[a + i + Width()] = VRAM[a + i + Width()] & ~mask | (Color == 1 ? mask : 0);
	}
	Invalidate(x, y, x + 6, y + 8);
	return 6;
}
