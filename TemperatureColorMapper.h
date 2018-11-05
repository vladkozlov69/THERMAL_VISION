/*
 * TemperatureColorMapper.h
 *
 *  Created on: Oct 10, 2018
 *      Author: vkozlov
 */

#ifndef TEMPERATURECOLORMAPPER_H_
#define TEMPERATURECOLORMAPPER_H_

#include "Arduino.h"
#include "Biquad.h"

class TemperatureColorMapper {
	float minTemp = 0, maxTemp = 0, minTempF = -1000, maxTempF = 1000;
	float a = 0, b = 0, c = 0, d = 0;
	Biquad * minTempTrendFilter;
	Biquad * maxTempTrendFilter;
public:
	TemperatureColorMapper();
	float getMinTemp();
	float getMaxTemp();
	void updateScale(float data[], int dataSize);
	uint16_t getColor(float value);
private:
	void updateCoefficients();
	uint16_t color565(uint8_t red, uint8_t green, uint8_t blue);
};

#endif /* TEMPERATURECOLORMAPPER_H_ */
