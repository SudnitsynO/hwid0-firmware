#pragma once
#include <stdint.h>
#include <stdlib.h>
#include "utf8_converter.h"
//#include <bitmap.h>

template<class ColorType>
class Font;

#include "graphic.h"

// This structure describes a single character's display information
typedef struct
{
	const uint8_t widthBits;				// width, in bits (or pixels), of the character
	const size_t offset;					// offset of the character's bitmap, in bytes, into the the FONT_INFO's data array

} FONT_CHAR_INFO;

// Describes a single font
typedef struct
{
	const uint8_t 			heightPages;	// height, in pages (8 pixels), of the font's characters
	const uint16_t 			startChar;		// the first character in the font (e.g. in charInfo and data)
	const uint16_t 			endChar;		// the last character in the font
	const uint8_t			spacePixels;	// number of pixels that a space character takes up
	const FONT_CHAR_INFO*	charInfo;		// pointer to array of char information
	const uint8_t*			data;			// pointer to generated array of character visual representation

} FONT_INFO;

// Font data for Microsoft Sans Serif 8pt
extern const uint8_t microsoftSansSerif_8ptBitmaps[];
extern const FONT_INFO microsoftSansSerif_8ptFontInfo;
extern const FONT_CHAR_INFO microsoftSansSerif_8ptDescriptors[];

// Font data for Microsoft Sans Serif 14pt
extern const uint8_t microsoftSansSerif_14ptBitmaps[];
extern const FONT_INFO microsoftSansSerif_14ptFontInfo;
extern const FONT_CHAR_INFO microsoftSansSerif_14ptDescriptors[];
// Font data for Anonymous Pro 14pt
extern const uint8_t anonymousPro_14ptBitmaps[];
extern const FONT_INFO anonymousPro_14ptFontInfo;
extern const FONT_CHAR_INFO anonymousPro_14ptDescriptors[];


template <class ColorType>
class Font
{
public:
	Font(FONT_INFO font_info);
	int DrawChar(char symbol, int x, int y, ColorType color, Graphic<ColorType> *graphic);
	int Heaght();
	int GetMaxWidth();
	int GetTextWidth(const char * text);
private:
	FONT_INFO _font_info;
	int Generate(uint16_t index, int x, int y, ColorType color, Graphic<ColorType> *graphic);	//generate bitmap;
	int GetSymbolIndex(const char symbol);
	int GetSymbolWidth(const char symbol);
	int _max_width;
};

template<class ColorType>
inline Font<ColorType>::Font(FONT_INFO font_info)
	:_font_info(font_info)
{
	_max_width = 0;
	for (size_t i = 0; i < 158; i++)
	{
		if (_font_info.charInfo[i].widthBits > _max_width)
			_max_width = _font_info.charInfo[i].widthBits;
	}
}

template<class ColorType>
inline int Font<ColorType>::DrawChar(char symbol, int x, int y, ColorType color, Graphic<ColorType> *graphic)
{
	auto index = GetSymbolIndex(symbol);
	if (index == -1) return _font_info.spacePixels * 2;
	else
		return Generate(index, x, y, color, graphic);
}

template<class ColorType>
inline int Font<ColorType>::Generate(uint16_t index, int x, int y, ColorType color, Graphic<ColorType> *graphic)
{
	//create new bitmap
	int width = _font_info.charInfo[index].widthBits;
	int heaght = _font_info.heightPages;
	size_t offset = _font_info.charInfo[index].offset;
	for (int y_offset = 0; y_offset < heaght; y_offset++)
	{
		for (int x_offset = 0; x_offset < width; x_offset++)
		{
			if (_font_info.data[offset + x_offset / 8] & (1 << (7 - (x_offset % 8))))
				graphic->SetPixel(x + x_offset, y + y_offset,color);
		}
		offset+= ((width-1) / 8) + 1;
	}
	return width + _font_info.spacePixels;
}

template<class ColorType>
inline int Font<ColorType>::GetSymbolIndex(const char symbol)
{
	if (symbol == ' ') return -1;
	else if ((symbol >= 0x21) && (symbol <= 0x7E))
	{
		return symbol - 0x21;
	}
	else if ((symbol >= 0xC0) && (symbol <= 0xFF))
	{
		return symbol - 0xC0 + 94;
	}
	return 0;
}

template<class ColorType>
inline int Font<ColorType>::GetSymbolWidth(const char symbol)
{
	auto index = GetSymbolIndex(symbol);
	if(index == -1)
	{
		return _font_info.spacePixels * 2;
	}
	else
		return _font_info.charInfo[index].widthBits +_font_info.spacePixels;
}

template<class ColorType>
inline int Font<ColorType>::Heaght()
{
	return _font_info.heightPages;
}

template<class ColorType>
inline int Font<ColorType>::GetMaxWidth()
{
	return _max_width;
}

template<class ColorType>
inline int Font<ColorType>::GetTextWidth(const char * text)
{
	auto str = convert_utf8_to_windows1251(std::string{ text });
	int width(0);

	for (auto ch : str)
	{
		width += GetSymbolWidth(ch);
	}
	return width;
}
