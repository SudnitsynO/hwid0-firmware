/*********************************************************************************/
// Team 296's Math Library
// Developed by Patrick Diez and Patrick Fairbank
//
// Functions and their use:
//
// sin	- Returns, on a 24 bit scale (-8,388,608 to 8,388,607), the sine of an angle given in 24 bit binary radians (a short long).
//		- Use: "short long sin(short long [angle])"
//
// cos	- Returns, on a 24 bit scale (-8,388,608 to 8,388,607), the cosine of an angle given in 24 bit binary radians (a short long).
//		- Use: "short long cos(short long [angle])"
//
// atan - Returns an arctangent, given both the denominator and the numerator, in a 24 bit angle in binary radians (-8,388,608 to 8,388,607).
//		- Use: "short long atan(short long [denominator], short long [numerator])"
//
// abs	- Returns the absolute value of a number.
//		- Use: "long atan(long [number])"
//
// sgn	- Returns the sign of a number (positive = 1, zero = 0, negative = -1).
//		- Use: "char sgn(long [number])"
//
// max 	- Limits a number to a maximum.
//		- Use: "long max(long [number], long [maximum value])"
//
// min 	- Limits a number to a minimum.
//		- Use: "long min(long [number], long [minimum value])"
//
// sqrt - Returns floating-point square root of a floating-point number.
//		- Use: "float sqrt(float [number])"
//
//
// Thank you for using our code!
//
/*********************************************************************************/

#include "cordic.h"

#define K 5094007

Int32 e[23] = { 2097152, 1238021, 654136, 332050, 166669, 83416, 41718, 20860, 10430, 5215, 2607, 1304, 652, 326, 163, 81, 41, 20, 10, 5, 2, 1, 1 };

Int32 Cordic24::sin(Int32 ang)
{
	return cordic(ang, 0);
}

Int32 Cordic24::cos(Int32 ang)
{
	return cordic(ang, 1);
}

Int32 Cordic24::cordic(Int32 ang, unsigned char r)
{
	unsigned char i;
	Int32 X = K, Y = 0, t = ang;
	Int32 dx, dy;
	if ((Int32)abs((Int32)ang) > 4194304)
		t = (Int32)sgn((Int32)ang) * (8388608 - (Int32)abs(ang));
	for (i = 0; i < 23; i++)
	{
		dx = sgn((Int32)X) * ((UInt32)abs((Int32)X) >> i);
		dy = sgn((Int32)Y) * ((UInt32)abs((Int32)Y) >> i);
		X -= (t > 0) ? dy : -dy;
		Y += (t > 0) ? dx : -dx;
		t -= (t > 0) ? e[i] : -e[i];
	}
	return (r == 0) ? Y : ((abs((Int32)ang) <= 4194304) ? X : -X);
}

Int32 Cordic24::atan(Int32 a, Int32 b)
{
	unsigned char i;
	Int32 X = (Int32)abs((Int32)a), Y = b, t = 0;
	Int32 dx, dy;
	for (i = 0; i < 23; i++)
	{
		t -= (Y > 0) ? e[i] : -e[i];
		dx = sgn((Int32)X) * ((UInt32)abs((Int32)X) >> i);
		dy = sgn((Int32)Y) * ((UInt32)abs((Int32)Y) >> i);
		X += (Y > 0) ? dy : -dy;
		Y -= (Y > 0) ? dx : -dx;
	}
	return (a > 0) ? -t : t + sgn(t) * 8388608; 
}

/******* Default math functions *******/
Int32 Cordic24::abs(Int32 x)
{
	return sgn(x) * x;
}

Int32 Cordic24::sgn(Int32 x)
{
	return (x > 0) ? 1 : ((x == 0) ? 0 : -1);
}

Int32 Cordic24::max(Int32 x, Int32 y)
{
	return (x > y) ? y : x;
}

Int32 Cordic24::min(Int32 x, Int32 y)
{
	return (x < y) ? y : x;
}

/******* Square Root Function *******/

float Cordic24::sqrt(float n)
{ 
	int i;
	float x = 2;
	for (; n > 1; n -= 2 * x++ - 1);
	n += (x - 1) * (x - 1);
	for(i = 0; i < 24; i++)
	{
		x += (x * x < n) ? (float) 1 / (1 << i) : (float) -1 / (1 << i);
	} 
	return x;
}

