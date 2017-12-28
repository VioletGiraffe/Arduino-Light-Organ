#pragma once

#include "RingBuffer.h"

#include <math.h>

struct VU_Data
{
	uint16_t rms;
	uint16_t peak;
};

static CRingBuffer<uint16_t, 8> rmsHistory;
uint16_t peakLevel = 0;

inline void processNewSample(uint16_t newSample)
{
	// 50 ms is the period of a 20 Hz wave. Averaging over a shorter period of time will not correctly handle low frequencies. 4096 samples is slightly more than 50 ms.
	// At 38400 Hz sample rate, uint32_t buffer and 10 bit precision (1024 max value), the maximum length of the window that will not trigger overflow is 99 ms.
	static constexpr uint16_t NumberOfSamplesToAverage = 4096;
	static uint16_t numSamplesAccumulatedInBuffer = 0; // How many samples have been added to the buffer during the analysis of the current window (the window is NumberOfSamplesToAverage long).
	static uint32_t RMSBuffer = 0;
	static uint16_t peakLevelTemp = 0;

	if (numSamplesAccumulatedInBuffer == NumberOfSamplesToAverage)
	{
		// Updating the latched values
		peakLevel = peakLevelTemp;
		rmsHistory.pushValue(fast_sqrt32(RMSBuffer / NumberOfSamplesToAverage));

		// Clearing the running values
		RMSBuffer = 0;
		numSamplesAccumulatedInBuffer = 0;
		peakLevelTemp = 0;
	}
	else
		++numSamplesAccumulatedInBuffer;

	// Update the running values
	if (peakLevelTemp < newSample)
		peakLevelTemp = newSample;

	RMSBuffer += newSample * newSample;
}