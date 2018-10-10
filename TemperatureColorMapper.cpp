/*
 * TemperatureColorMapper.cpp
 *
 *  Created on: Oct 10, 2018
 *      Author: vkozlov
 */

#include "TemperatureColorMapper.h"

TemperatureColorMapper::TemperatureColorMapper()
{
	minTempTrendFilter = new Biquad(bq_type_lowpass, 0.1 / 10, 0.707, 0);
	maxTempTrendFilter = new Biquad(bq_type_lowpass, 0.1 / 10, 0.707, 0);
}

float TemperatureColorMapper::getMinTemp()
{
	return minTemp;
}

float TemperatureColorMapper::getMaxTemp()
{
	return maxTemp;
}

void TemperatureColorMapper::updateScale(float data[], int dataSize)
{
	minTemp = 500;
	maxTemp =-500;

	for (int i=0; i<dataSize; i++)
	{
		minTemp = min(minTemp, data[i]);
		maxTemp = max(maxTemp, data[i]);
	}

	minTemp = minTempTrendFilter->process(minTemp);
	maxTemp = maxTempTrendFilter->process(maxTemp) + 1;

	updateCoefficients();
}

uint16_t TemperatureColorMapper::getColor(float value)
{
	/*
	pass in value and figure out R G B
	several published ways to do this I basically graphed R G B and developed simple linear equations
	again a 5-6-5 color display will not need accurate temp to R G B color calculation

	equations based on
	http://web-tech.ga-usa.com/2012/05/creating-a-custom-hot-to-cold-temperature-color-gradient-for-use-with-rrdtool/index.html

	*/
	byte red = 0, green = 0, blue = 0;

	red = constrain(255.0 / (c - b) * value - ((b * 255.0) / (c - b)), 0, 255);

	if ((value > minTemp) & (value < a))
	{
		green = constrain(255.0 / (a - minTemp) * value - (255.0 * minTemp) / (a - minTemp), 0, 255);
	}
	else if ((value >= a) & (value <= c))
	{
		green = 255;
	}
	else if (value > c)
	{
		green = constrain(255.0 / (c - d) * value - (d * 255.0) / (c - d), 0, 255);
	}
	else if ((value > d) | (value < a))
	{
		green = 0;
	}

	if (value <= b)
	{
		blue = constrain(255.0 / (a - b) * value - (255.0 * b) / (a - b), 0, 255);
	}
	else if ((value > b) & (value <= d))
	{
		blue = 0;
	}
	else if (value > d)
	{
		blue = constrain(240.0 / (maxTemp - d) * value - (d * 240.0) / (maxTemp - d), 0, 240);
	}

	// use the displays color mapping function to get 5-6-5 color palet (R=5 bits, G=6 bits, B-5 bits)
	return color565(red, green, blue);
}

void TemperatureColorMapper::updateCoefficients()
{
	a = minTemp + (maxTemp - minTemp) * 0.2121;
	b = minTemp + (maxTemp - minTemp) * 0.3182;
	c = minTemp + (maxTemp - minTemp) * 0.4242;
	d = minTemp + (maxTemp - minTemp) * 0.8182;
}

uint16_t TemperatureColorMapper::color565(uint8_t red, uint8_t green, uint8_t blue)
{
	return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3);
}

