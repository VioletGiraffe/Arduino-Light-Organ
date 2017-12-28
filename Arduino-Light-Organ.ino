#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>

#include <math.h>

#include "utils.h"
#include "VU_meter.h"
#include "Algorithms.h"

//#define USE_TEST_SIGNAL
#include "Test_signal.h"

#ifndef _PDQ_ST7735H_
#define TFT_CS 10
#define TFT_DC 8
#define TFT_RST 0  // you can also connect this to the Arduino reset, in which case, set this #define pin to 0!
#define ST7735_INITR_144GREENTAB INITR_144GREENTAB
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
#else
PDQ_ST7735 tft;
#endif

void setup()
{
	setupADC();

	tft.initR(ST7735_INITR_144GREENTAB); // initialize a ST7735S chip, 1.44" TFT, black tab
	tft.fillScreen(ST7735_BLACK);
	tft.setTextWrap(false);
	drawStaticUiElements();
}

void setupADC()
{
	pmc_enable_periph_clk(ID_ADC); // Enable ADC clocking
	adc_init(ADC, SystemCoreClock, ADC_FREQ_MAX, ADC_STARTUP_FAST); // initialize, set maximum posibble speed
	adc_set_resolution(ADC, ADC_12_BITS);
	adc_configure_power_save(ADC, 0, 0); // Disable sleep
	adc_configure_timing(ADC, 0, ADC_SETTLING_TIME_3, 1); // Set timings - standard values
	adc_set_bias_current(ADC, 1); // Bias current - maximum performance over current consumption
	adc_stop_sequencer(ADC); // not using it
	adc_disable_tag(ADC); // it has to do with sequencer, not using it
	adc_disable_ts(ADC); // deisable temperature sensor
	adc_disable_channel_differential_input(ADC, ADC_CHANNEL_7);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 1); // triggering from software, freerunning mode
	adc_disable_all_channel(ADC);
	adc_enable_channel(ADC, ADC_CHANNEL_7); // just one channel enabled
	adc_start(ADC);
}

// Variables for individual samples and statistics
volatile uint16_t maxSampleValue = 0;
volatile uint16_t minSampleValue = 65535;

// Variables for storing and managing a window (fixed-length span) of samples
volatile bool samplingWindowFull = false;
constexpr int FHT_N = 256;
uint8_t previousFhtValues[FHT_N / 2]; // The previous set of FHT results, used for optimizing the screen redraw

ISR(ADC_vect) //when new ADC value ready
{
	const uint16_t sample = (uint16_t)ADC->ADC_CDR[7];

	if (sample > maxSampleValue)
		maxSampleValue = sample;
	else if (sample < minSampleValue)
		minSampleValue = sample;

	processNewSample(sample);

	if (samplingWindowFull)
		return;

	static uint16_t currentSampleIndex = 0;

#ifndef USE_TEST_SIGNAL
//	fht_input[currentSampleIndex] = sample - 512; // fht_input is signed! Skipping this step will result in DC offset
#endif

	++currentSampleIndex;

	if (currentSampleIndex == FHT_N)
	{
		currentSampleIndex = 0;
		samplingWindowFull = true;
	}
}

void loop()
{
	if (samplingWindowFull)
	{
		// No-op if USE_TEST_SIGNAL is not defined
		generateTestSignal(1000 /* Hz */, 1024, 32);

//		memcpy(previousFhtValues, fht_log_out, FHT_N / 2);
//		runFHT();

		const auto start = millis();

		updateTextDisplay();
		updateSpectrumDisplay();
		updateVuDisplay();

		tft.setTextSize(2);
		tft.setTextColor(0x051F, 0x0000);
		tft.setCursor(0, 0);
		tft.print(paddedString(String(millis() - start), 4));

		samplingWindowFull = false; // Allow the new sample set to be collected - only after the delay. Else the sample set would be 60 ms stale by the time we get to process it.
	}
}

#define RGB_to_565(R, G, B) static_cast<uint16_t>(((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3))

constexpr uint16_t vuYpos = 25, vuHeight = 10;
constexpr uint16_t spectrumYpos = vuYpos + vuHeight;

constexpr int ScreenWidth = 128, ScreenHeight = 128;

inline void updateSpectrumDisplay()
{
	//for (int i = 1; i < 128; ++i) // What's the deal with bin 0?
	//{
	//	const auto freeSpaceHeight = tft.height() - fht_log_out[i] / 2;

	//	tft.drawFastVLine(i, 25, freeSpaceHeight, RGB_to_565(0, 0, 0));
	//	tft.drawFastVLine(i, freeSpaceHeight, 128, RGB_to_565(255, 255, 200));
	//}

	//for (int i = 1; i < 128; ++i) // What's the deal with bin 0?
	//{
	//	const int diff = (int)fht_log_out[i] / 2 - previousFhtValues[i] / 2;
	//	if (diff > 0)
	//		tft.drawFastVLine(i, ScreenHeight - fht_log_out[i] / 2, diff, RGB_to_565(255, 255, 200));
	//	else if (diff < 0)
	//		tft.drawFastVLine(i, ScreenHeight - previousFhtValues[i] / 2, -diff, RGB_to_565(0, 0, 0));
	//}


	// Symbol heights depending on text size: 1 - 10(?), 2 - 15, 3 - 25

	tft.setTextSize(2);

	//tft.setTextColor(RGB_to_565(0, 200, 255));
	//tft.setCursor(0, 0);
	//tft.print(minSampleValue);

	tft.setTextColor(RGB_to_565(255, 0, 10), RGB_to_565(0, 0, 0));
	tft.setCursor(45, 0);
	tft.print(paddedString(String(maxSampleValue), 4));

	tft.setTextColor(RGB_to_565(0, 255, 10), RGB_to_565(0, 0, 0));
	tft.setCursor(90, 0);
	tft.print(paddedString(String(rmsHistory.back()), 4));
}

inline void updateTextDisplay()
{

}

inline void updateVuDisplay()
{
	static auto previousPeak = peakLevel;

	auto peak = peakLevel;
	if (peak < previousPeak && peak >= 48)
		peak = previousPeak - 48;

	previousPeak = peak;

	const auto rms = rmsHistory.back();

	const auto barWidth = ((uint32_t)rms * (uint32_t)ScreenWidth) / 1024;
	auto peakLevelXpos = ((uint32_t)peak * (uint32_t)ScreenWidth) / 1024;
	if (peakLevelXpos < barWidth)
		peakLevelXpos = barWidth;

	const uint16_t vuBarColor = rms < 512 ? RGB_to_565(rms / 2, 255, 30) : RGB_to_565(255, (rms - 512) / 2, 30);

	tft.fillRect(0, vuYpos, barWidth, vuHeight, vuBarColor);
	tft.fillRect(barWidth + 1, vuYpos, ScreenWidth - barWidth, vuHeight, RGB_to_565(0, 0, 0));
	tft.drawFastVLine(peakLevelXpos, vuYpos, vuHeight, RGB_to_565(255, 0, 30));
}

inline void drawStaticUiElements()
{
}
