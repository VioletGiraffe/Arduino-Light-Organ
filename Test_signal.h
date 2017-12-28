#pragma once

#ifdef USE_TEST_SIGNAL

#include <math.h>

inline void generateTestSignal(uint16_t frequency, uint16_t peakValue, uint8_t adcPrescaler)
{
	const float samplingPeriod = 1.0f / (F_CPU / 13.0f / adcPrescaler); // seconds
	const float angularFrequency = 2.0f * M_PI * frequency; // Hz
	for (int i = 0; i < FHT_N; ++i)
		fht_input[i] = round(peakValue / 2 * cos(angularFrequency * i * samplingPeriod));
}

#else

inline void generateTestSignal(uint16_t /*frequency*/, uint16_t /*peakValue*/, uint8_t /*adcPrescaler*/)
{
}

#endif