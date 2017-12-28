#pragma once

#include <math.h>

inline void setBit(volatile uint8_t& reg, uint8_t bit)
{
	reg |= (1 << bit);
}

inline void clearBit(volatile uint8_t& reg, uint8_t bit)
{
	reg &= ~(1 << bit);
}

inline String paddedString(const String& s, const uint8_t width, const bool leftJustify = true /* otherwise right-justify */)
{
	String padded;

	if (leftJustify)
		padded = s;

	for (int i = 0; i < (int)s.length() - width; ++i) // If len < s.length(), the loop will handle it correctly, i. e. do nothing
		padded += ' ';

	if (!leftJustify)
		padded += s;

	return padded;
}

// Source: http://openaudio.blogspot.com/2017/02/faster-log10-and-pow.html
inline float log2f_approx(float X)
{
	int E;
	const float F = frexpf(fabsf(X), &E);

	float Y = 1.23149591368684f;
	Y *= F;
	Y += -4.11852516267426f;
	Y *= F;
	Y += 6.02197014179219f;
	Y *= F;
	Y += -3.13396450166353f;
	Y += E;

	return Y;
}

#define log10f_fast(x) (log2f_approx(x) * 0.3010299956639812f)

inline uint16_t fast_sqrt32(const uint32_t n)
{
	uint32_t c = 0x8000;
	uint32_t g = 0x8000;

	for (;;)
	{
		if (g*g > n)
			g ^= c;

		c >>= 1;
		if (c == 0)
			return g;

		g |= c;
	}
}