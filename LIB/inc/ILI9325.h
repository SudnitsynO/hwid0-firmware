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
class ILI9325 : public Graphic<uint16_t>
{
public:
	void Init();
	virtual void SetPixel(int x, int y, uint16_t Color);
	virtual uint16_t GetPixel(int x, int y);
	virtual int Width();
	virtual int Height();
	virtual void DrawText(const char* text, uint16_t Color, Font<uint16_t>* font);
	virtual void DrawText(const char* text, uint16_t Color = 0xFFFF);
	virtual void SetTextPos(int x, int y);
	virtual void UpdateDisplay();
	virtual void Clear(uint16_t color = 0);
	void Fill(int x1, int y1, int x2, int y2, unsigned short Color);
	virtual void Invalidate(int x1, int y1, int x2, int y2);
	void WriteCommand(uint16_t index);
	void WriteData(uint16_t data);
	void write_reg(uint16_t reg, uint16_t value);
	void DrawBitmap(int x, int y, Bitmap<uint16_t>* bitmap);
	void Test();
	void set_orientation(bool xy_excange, bool mirror_x, bool mirror_y);
private:
	int x_text;
	int y_text;

	uint16_t ReadData();

	uint16_t ReadState();

	void Wait();

	//void set_frame(int x1,int y1,int x2,int y2);

	int DrawSystemChar(int x, int y, uint16_t Color, char symbol);

	//	uint16_t Lcd_Read_Reg(uint16_t reg_addr);
	//void Lcd_Write_Reg(uint16_t reg, uint16_t value);
	volatile uint16_t test1, test2, test3, test4;
};

template<size_t REG_ADDR, size_t DATA_ADDR>
void ILI9325<REG_ADDR, DATA_ADDR>::Test()
{
	//set_frame(0, 0, 319, 239);
	//Clear(0xFF00);
	//Line(0, 0, 100, 100, 0xFF);
	//Line(100, 100, 200, 100, 0xFF);
	//Fill(100, 100, 110, 130, 0);
	//SetTextPos(100, 100);
	//DrawText("HELP", 0);
	//Circle(100, 100, 50, BLUE_MASK);

	//write_reg(0x20, 239-100);
	//write_reg(0x21, 319-100);
	//WriteCommand(0x22);
	//for (size_t i = 0; i < 10000; i++)
	//{
		//WriteData(i);
	//}
	//for (;;);
}

template<size_t REG_ADDR, size_t DATA_ADDR>
void ILI9325<REG_ADDR, DATA_ADDR>::Init()
{

	x_text = 0;
	y_text = 0;

	//PA5::SetMode(Mode::Output);
	PB10::Reset();
	Delay_ms(120);
	PB10::Set();
	write_reg(0x00FF, 0x0001);//can we do 0xFF
	write_reg(0x00F3, 0x0008);

	write_reg(0x00,0x0001);
	write_reg(0x0001,0x0100);     // Driver Output Control Register (R01h)
	write_reg(0x0002,0x0700);     // LCD Driving Waveform Control (R02h)
	write_reg(0x0003,0x1030);     // Entry Mode (R03h)
	write_reg(0x0008,0x0302);
	write_reg(0x0009,0x0000);
	write_reg(0x0010,0x0000);     // Power Control 1 (R10h)
	write_reg(0x0011, 0x0007);    // Power Control 2 (R11h)
	write_reg(0x0012, 0x0000);     // Power Control 3 (R12h)
	write_reg(0x0013, 0x0000);     // Power Control 4 (R13h)
	Delay_ms(50);
	write_reg(0x0010, 0x14B0);     // Power Control 1 SAP=1, BT=4, APE=1, AP=3
	Delay_ms(10);
	write_reg(0x0011, 0x0007);     // Power Control 2 VC=7
	Delay_ms(10);
	write_reg(0x0012, 0x008E);     // Power Control 3 VCIRE=1, VRH=14
	write_reg(0x0013, 0x0C00);     // Power Control 4 VDV=12
	write_reg(0x0029, 0x0015);     // NVM read data 2 VCM=21
	Delay_ms(10);
	write_reg(0x0030, 0x0000);     // Gamma Control 1
	write_reg(0x0031, 0x0107);     // Gamma Control 2
	write_reg(0x0032, 0x0000);     // Gamma Control 3
	write_reg(0x0035, 0x0203);     // Gamma Control 6
	write_reg(0x0036, 0x0402);     // Gamma Control 7
	write_reg(0x0037, 0x0000);     // Gamma Control 8
	write_reg(0x0038, 0x0207);     // Gamma Control 9
	write_reg(0x0039, 0x0000);     // Gamma Control 10
	write_reg(0x003C, 0x0203);     // Gamma Control 13
	write_reg(0x003D, 0x0403);     // Gamma Control 14
	write_reg(0x0060, 0xA700);     // Driver Output Control (R60h) .kbv was 0xa700
	write_reg(0x0061, 0x0001);     // Driver Output Control (R61h)
	write_reg(0x0090, 0x0029);     // Panel Interface Control 1 (R90h)

		// Display On
	write_reg(0x0007, 0x0133);     // Display Control (R07h)

	//entry mode
	write_reg(0x0003, 0x0038);     // Entry Mode (R03h)
	//write_reg(0x0050, 0);     // Entry Mode (R03h)
	//write_reg(0x0051, 239);     // Entry Mode (R03h)
	//write_reg(0x0052, 0);     // Entry Mode (R03h)
	//write_reg(0x0053, 319);     // Entry Mode (R03h)

		
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void ILI9325<REG_ADDR, DATA_ADDR>::SetPixel(int x, int y, uint16_t Color)
{
	//set_frame(x, y, x, y);

	WriteCommand(0x20);//DRAM Horizontal Address
	WriteData(Height()- y-1 & 0xFF);
	WriteCommand(0x21);//DRAM Vertical Address Set
	WriteData( Width()-x-1 & 0x1FF);
	WriteCommand(0x22);//DRAM Horizontal Address
	WriteData(Color);
	//Wait();
}

template <size_t REG_ADDR, size_t DATA_ADDR>
uint16_t ILI9325<REG_ADDR, DATA_ADDR>::GetPixel(int x, int y)
{
	return 0;
}

template <size_t REG_ADDR, size_t DATA_ADDR>
int ILI9325<REG_ADDR, DATA_ADDR>::Width()
{
	return 320;
}

template <size_t REG_ADDR, size_t DATA_ADDR>
int ILI9325<REG_ADDR, DATA_ADDR>::Height()
{
	return 240;
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void ILI9325<REG_ADDR, DATA_ADDR>::UpdateDisplay()
{
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void ILI9325<REG_ADDR, DATA_ADDR>::Clear(uint16_t color)
{
	
	//set_frame(0, 0, Width()-1, Height()-1);
	write_reg(0x20, 0);
	write_reg(0x21, 0);

	WriteCommand(0x22);//Memory Write
	for (size_t i = 0; i < Width() * Height(); i++)
	{
		WriteData(color);
	}
	x_text = 0;
	y_text = 0;
	/*Lcd_Write_Reg(0x004e, 239);
	Lcd_Write_Reg(0x004f, 319);
	WriteCommand(0x0022);
	for (size_t i = 0; i < 320 * 240; i++)
	{
		WriteData(color);
	}
	x_text = 0;
	y_text = 0;*/
	//Delay_ms(10);
	//SetPixel(10, 10, 0);
	//Delay_ms(10);
	//Lcd_Write_Reg(0x004e, 239);
	//Lcd_Write_Reg(0x004f, 319);
	//WriteCommand(0x0022);
	//Wait();
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void ILI9325<REG_ADDR, DATA_ADDR>::Fill(int x1, int y1, int x2, int y2, unsigned short Color)
{
	auto w = x2 - x1;
	auto h = y2 - y1;
	if (w < 0) w = -w;
	if (h < 0) h = -h;

	w++;
	h++;

	for (size_t y = y1; y <= y2; y++)
	{
		write_reg(0x20, Height()-1 - y);
		write_reg(0x21, Width()-1 - x2);
		WriteCommand(0x22);
		for (size_t x = x1; x <= x2; x++)
		{
			WriteData(Color);
		}
	}
	//set_frame(0, 0, Width()-1, Height()-1);
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void ILI9325<REG_ADDR, DATA_ADDR>::Invalidate(int x1, int y1, int x2, int y2)
{
}

template <size_t REG_ADDR, size_t DATA_ADDR>
inline void ILI9325<REG_ADDR, DATA_ADDR>::WriteCommand(uint16_t index)
{
	*(DisplayReg*)(REG_ADDR) = index;//((index << 8) & 0xFF00) ;
}

template <size_t REG_ADDR, size_t DATA_ADDR>
inline void ILI9325<REG_ADDR, DATA_ADDR>::WriteData(uint16_t data)
{
	*(DisplayReg*)(DATA_ADDR) = data;
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void ILI9325<REG_ADDR, DATA_ADDR>::write_reg(uint16_t reg, uint16_t value)
{
	WriteCommand(reg);
	WriteData(value);
}

template <size_t REG_ADDR, size_t DATA_ADDR>
inline uint16_t ILI9325<REG_ADDR, DATA_ADDR>::ReadData()
{
	return *(DisplayReg*)(DATA_ADDR);
}

template <size_t REG_ADDR, size_t DATA_ADDR>
inline uint16_t ILI9325<REG_ADDR, DATA_ADDR>::ReadState()
{
	return *(DisplayReg*)(REG_ADDR);
}
/*
template <size_t REG_ADDR, size_t DATA_ADDR>
void ILI9325<REG_ADDR, DATA_ADDR>::Lcd_Write_Reg(uint16_t reg, uint16_t value)
{
	WriteCommand(reg);
	WriteData(value);
}
*/
template<size_t REG_ADDR, size_t DATA_ADDR>
inline void ILI9325<REG_ADDR, DATA_ADDR>::Wait()
{
	/*uint16_t test = 0;
	Lcd_Write_Reg(0x004e, 239);
	Lcd_Write_Reg(0x004f, 319);
	do
	{
		test = ReadState();
	} while (test != 0x8989);*/
}

template <size_t REG_ADDR, size_t DATA_ADDR>
void ILI9325<REG_ADDR, DATA_ADDR>::set_orientation(bool xy_excange, bool mirror_x, bool mirror_y)
{
	WriteCommand(0x36);	//Memory Access Control
	uint16_t value = 0;
	if (xy_excange) value |= 1 << 5;
	if (mirror_x) value |= 1 << 6;
	if (mirror_y) value |= 1 << 7;
	WriteData(value);
}

template<size_t REG_ADDR, size_t DATA_ADDR>
inline void ILI9325<REG_ADDR, DATA_ADDR>::DrawBitmap(int x, int y, Bitmap<uint16_t>* bitmap)
{
	//if (bitmap == nullptr) return;
	//for (size_t i = 0; i < bitmap->Height(); i++)
	//{
	//	//draw string
	//	Lcd_Write_Reg(0x004e, 239 - (y + i));
	//	Lcd_Write_Reg(0x004f, 319 - x);
	//	WriteCommand(0x0022);
	//	for (size_t j = 0; j < bitmap->Width(); j++)
	//	{
	//		WriteData(bitmap->bitmap[j + i * bitmap->Width()]);
	//	}
	//}
}

template<size_t REG_ADDR, size_t DATA_ADDR>
inline void ILI9325<REG_ADDR, DATA_ADDR>::DrawText(const char* text, uint16_t Color, Font<uint16_t>* font)
{
	auto str = convert_utf8_to_windows1251(text);
	//int index = 0;
	//convert_utf8_to_windows1251(text, str, 100);
	for (auto ch : str)
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
inline void ILI9325<REG_ADDR, DATA_ADDR>::DrawText(const char* text, uint16_t Color)
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
inline void ILI9325<REG_ADDR, DATA_ADDR>::SetTextPos(int x, int y)
{
	x_text = x;
	y_text = y;
}

template<size_t REG_ADDR, size_t DATA_ADDR>
int ILI9325<REG_ADDR, DATA_ADDR>::DrawSystemChar(int x, int y, uint16_t Color, char symbol)
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
	//Invalidate(x, y, x + 6, y + 8);
	return 6;
}
