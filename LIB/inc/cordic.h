/************* MATH.H ******************/
// Contains function prototypes for the math library
// Include this file in all c files which use the math functions

#include "../types.h"

class Cordic24
{
public:
	static Int32 sin(Int32 ang);

	static Int32 cos(Int32 ang);

	static Int32 cordic(Int32 ang, unsigned char r);

	static Int32 atan(Int32 a, Int32 b);

	static Int32 abs(Int32 x);

	static Int32 sgn(Int32 x);

	static Int32 max(Int32 x, Int32 y);

	static Int32 min(Int32 x, Int32 y);

	static float sqrt(float n);
};
