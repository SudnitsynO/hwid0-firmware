#pragma once
#include <stdint.h>
#include <stddef.h>
#include <graphic.h>

template <class ColorType>
class Bitmap : public Graphic<ColorType>
{
public:
	ColorType * bitmap;
private:
	int _x_size;
	int _y_size;
public:
	// Унаследовано через Graphic
	virtual void SetPixel(int x, int y, ColorType Color) 
	{
		if (bitmap == nullptr) return;
		bitmap[x + y * _x_size] = Color;
	}
	
	virtual ColorType GetPixel(int x, int y) 
	{
		if (bitmap == nullptr) return ColorType();
		return bitmap[x + y * _x_size];
	}
	
	virtual int Width() 
	{
		return _x_size;
	}
	
	virtual int Height() 
	{
		return _y_size;
	}
	
	virtual void UpdateDisplay() 
	{
	}
	
	virtual void Clear(ColorType color) 
	{
		for (size_t i = 0; i < _x_size * _y_size; i++)
		{
			bitmap[i] = color;
		}
	}
	virtual void Invalidate(int x1, int y1, int x2, int y2) 
	{
	}
	
	Bitmap(int x_size, int y_size)
		: _x_size(x_size), _y_size(y_size)
	{
		bitmap = new ColorType[_x_size * _y_size];
	}
	
	~Bitmap()
	{
		delete[] bitmap;
	}
};
