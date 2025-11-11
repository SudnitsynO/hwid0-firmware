#pragma once
#include <stdint.h>
#include <cmath>

constexpr float PI = 3.141592653589793238463;

template <class ColorType>
class Graphic;

#include "fonts.h"

template <class ColorType>
class Graphic
{
public:
	virtual void SetPixel(int x, int y, ColorType Color) = 0;
	virtual ColorType GetPixel(int x, int y) = 0;
	virtual int Width() = 0;
	virtual int Height() = 0;
	virtual void DrawText(const char* text, uint16_t Color, Font<uint16_t>* font) = 0;
	virtual void DrawText(const char* text, uint16_t Color) = 0;
	void Circle(int x0, int y0, int radius, ColorType Color);
	void Arc(int x0, int y0, int radius, float a, float b, ColorType Color);
	void Line(int x1, int y1, int x2, int y2, ColorType Color);
	void Rectangle(int x1, int y1, int x2, int y2, ColorType Color);
	virtual void Fill(int x1, int y1, int x2, int y2, ColorType Color) = 0;
	virtual void UpdateDisplay() = 0;
	virtual void Clear(ColorType color = 0) = 0;

	virtual void Invalidate(int x1, int y1, int x2, int y2) = 0;
	virtual void SetTextPos(int x, int y) = 0;
};

template <class ColorType>
void Graphic<ColorType>::Line(int x1, int y1, int x2, int y2, ColorType Color)
{
	Invalidate(x1, y1, x2, y2);
	//Color &= 1;
	int deltax = x2 - x1;
	int deltay = y2 - y1;
	if (deltax < 0) deltax = -deltax;
	if (deltay < 0) deltay = -deltay;
	int stepX = (x1 <= x2) ? 1 : -1;
	int stepY = (y1 <= y2) ? 1 : -1;
	int error = 0;
	int y = y1;
	int x = x1;

	SetPixel(x1, y1, Color);

	if (deltay < deltax)
	{
		//шагаем по x
		while (x != x2)
		{
			x += stepX;
			error = error + deltay;
			if (((2 * error) >= deltax) && (deltax != 0))
			{
				y += stepY;
				error = error - deltax;
			}
			SetPixel(x, y, Color);
		}
	}
	else
	{
		//шагаем по y
		while (y != y2)
		{
			y += stepY;
			error = error + deltax;
			if (((2 * error) >= deltay) && (deltay != 0))
			{
				x += stepX;
				error = error - deltay;
			}
			SetPixel(x, y, Color);
		}
	}
}

template <class ColorType>
void Graphic<ColorType>::Rectangle(int x1, int y1, int x2, int y2, ColorType Color)
{
	Line(x1, y1, x1, y2, Color);
	Line(x1, y1, x2, y1, Color);
	Line(x1, y2, x2, y2, Color);
	Line(x2, y1, x2, y2, Color);
}

template <class ColorType>
void Graphic<ColorType>::Circle(int x0, int y0, int radius, ColorType Color)
{
	int x = radius;
	int y = 0;
	int radiusError = 1 - x;
	while (x >= y)
	{
		SetPixel(x + x0, y + y0, Color);
		SetPixel(y + x0, x + y0, Color);
		SetPixel(-x + x0, y + y0, Color);
		SetPixel(-y + x0, x + y0, Color);
		SetPixel(-x + x0, -y + y0, Color);
		SetPixel(-y + x0, -x + y0, Color);
		SetPixel(x + x0, -y + y0, Color);
		SetPixel(y + x0, -x + y0, Color);
		y++;
		if (radiusError < 0)
		{
			radiusError += 2 * y + 1;
		}
		else
		{
			x--;
			radiusError += 2 * (y - x + 1);
		}
	}
}

template <class ColorType>
void Graphic<ColorType>::Arc(int x0, int y0, int radius, float a, float b, ColorType Color)
{
	int x = radius;
	int y = 0;
	int radiusError = 1 - x;
	int s_begin = (a > 0 ? a : a - 45) / 45; //номер сегмента начала дуги
	int s_end = (b > 0 ? b : b - 45) / 45; //номер сегмента конца дуги
	int y_begin = sin(a / 180 * PI) * radius;
	int y_end = sin(b / 180 * PI) * radius;
	int x_begin = cos(a / 180 * PI) * radius;
	int x_end = cos(b / 180 * PI) * radius;

	{
		while (x >= y)
		{
			if (s_begin != s_end)
			{
				if (((s_end > -1) && (s_begin < -1)) || ((s_begin == -1) && (y <= -y_begin)) || ((s_end == -1) && (y >=
					-y_end))) SetPixel(x + x0, y + y0, Color); // -45/0		-1
				if (((s_end > -2) && (s_begin < -2)) || ((s_begin == -2) && (y >= x_begin)) || ((s_end == -2) && (y <=
					x_end))) SetPixel(y + x0, x + y0, Color); //-90/-45		-2
				if (((s_end > -4) && (s_begin < -4)) || ((s_begin == -4) && (y >= -y_begin)) || ((s_end == -4) && (y <=
					-y_end))) SetPixel(-x + x0, y + y0, Color); //-180/-135	-4
				if (((s_end > -3) && (s_begin < -3)) || ((s_begin == -3) && (y <= -x_begin)) || ((s_end == -3) && (y >=
					-x_end))) SetPixel(-y + x0, x + y0, Color); //-135/-90	-3
				if (((s_end > +3) && (s_begin < +3)) || ((s_begin == +3) && (y <= y_begin)) || ((s_end == +3) && (y >=
					y_end))) SetPixel(-x + x0, -y + y0, Color); //135/180		+3
				if (((s_end > +2) && (s_begin < +2)) || ((s_begin == +2) && (y >= -x_begin)) || ((s_end == +2) && (y <=
					-x_end))) SetPixel(-y + x0, -x + y0, Color); //90/135	+2
				if (((s_end > +0) && (s_begin < +0)) || ((s_begin == +0) && (y >= y_begin)) || ((s_end == +0) && (y <=
					y_end))) SetPixel(x + x0, -y + y0, Color); //0/45			+0
				if (((s_end > +1) && (s_begin < +1)) || ((s_begin == +1) && (y <= x_begin)) || ((s_end == +1) && (y >=
					x_end))) SetPixel(y + x0, -x + y0, Color); //45/90		+1
			}
			else
			{
				if ((s_end == -1) && (y <= -y_begin) && (y >= -y_end)) SetPixel(x + x0, y + y0, Color); // -45/0		-1
				if ((s_end == -2) && (y >= x_begin) && (y <= x_end)) SetPixel(y + x0, x + y0, Color); //-90/-45		-2
				if ((s_end == -4) && (y >= -y_begin) && (y <= -y_end)) SetPixel(-x + x0, y + y0, Color); //-180/-135	-4
				if ((s_end == -3) && (y <= -x_begin) && (y >= -x_end)) SetPixel(-y + x0, x + y0, Color); //-135/-90		-3
				if ((s_end == +3) && (y <= y_begin) && (y >= y_end)) SetPixel(-x + x0, -y + y0, Color); //135/180		+3
				if ((s_end == +2) && (y >= -x_begin) && (y <= -x_end)) SetPixel(-y + x0, -x + y0, Color); //90/135		+2
				if ((s_end == +0) && (y >= y_begin) && (y <= y_end)) SetPixel(x + x0, -y + y0, Color); //0/45			+0
				if ((s_end == +1) && (y <= x_begin) && (y >= x_end)) SetPixel(y + x0, -x + y0, Color); //45/90			+1
			}
			y++;
			if (radiusError < 0)
			{
				radiusError += 2 * y + 1;
			}
			else
			{
				x--;
				radiusError += 2 * (y - x + 1);
			}
		}
	}
}
