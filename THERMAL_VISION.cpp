// Do not remove the include below
#include "THERMAL_VISION.h"

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include "TemperatureColorMapper.h"


#define TFT_CS     26
#define TFT_RST    27
#define TFT_DC     25

TemperatureColorMapper colorMapper;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
Adafruit_AMG88xx amg;

#define AMG_COLS 8
#define AMG_ROWS 8
#define INTERPOLATED_COLS 32
#define INTERPOLATED_ROWS 32

float pixels[AMG_COLS * AMG_ROWS];
float dest_2d[INTERPOLATED_ROWS * INTERPOLATED_COLS];


float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols,
                       float *dest, uint8_t dest_rows, uint8_t dest_cols);


void drawpixels(float rawPixels[], float *p, uint8_t rows, uint8_t cols, uint8_t boxWidth, uint8_t boxHeight, boolean showVal)
{
	for (int y=0; y<rows; y++)
	{
		for (int x=0; x<cols; x++)
		{
			float colorTemp = get_point(p, rows, cols, x, y);
			word rgbColor = colorMapper.getColor(colorTemp);
			tft.fillRect(boxWidth * x, boxHeight * y, boxWidth, boxHeight, rgbColor);
		}
	}
}

void setup()
{
	Serial.begin(115200);
	Serial.println("\n\nAMG88xx Interpolated Thermal Camera!");

	tft.initR(INITR_BLACKTAB);
	tft.fillScreen(ST7735_BLACK);
	//tft.setRotation(2);

	Wire.begin(19, 17);
	/*
	 * Do not forget to comment out default I2C initialization
	 * void Adafruit_AMG88xx::_i2c_init()
	 */

	// default settings
	if (!amg.begin())
	{
		Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
		while (1) { delay(1); }
	}
}

void mirrorPixels(float pixelArray[], int rows, int cols)
{
	float swapTemp;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols / 2; j++)
		{
			int idx1 = i * rows + j;
			int idx2 = i * rows + (cols - j - 1);
			swapTemp = pixelArray[idx1];
			pixelArray[idx1] = pixelArray[idx2];
			pixelArray[idx2] = swapTemp;
		}
	}
}

void rotatePixels(float pixelArray[], int rows, int cols)
{
	float swapTemp;
	int size = rows * cols;
	for (int i = 0; i < size/2; i++)
	{
		swapTemp = pixelArray[i];
		pixelArray[i] = pixelArray[size - i - 1];
		pixelArray[size - i - 1] = swapTemp;
	}
}

float centralTemperature(float pixelArray[], int rows, int cols)
{
	float sum = 0;
	for (int i = rows/2-1; i <= rows/2; i++)
	{
		for (int j = cols/2-1; j <= cols/2; j++)
		{
			sum = sum + pixelArray[i * rows + j];
		}
	}

	return sum/4;
}

void loop()
{
//	int t_Start = millis();

	//read all the pixels
	amg.readPixels(pixels);
	rotatePixels(pixels, AMG_ROWS, AMG_COLS);
//	mirrorPixels(pixels, AMG_ROWS, AMG_COLS);

	colorMapper.updateScale(pixels, AMG_ROWS * AMG_COLS);


	interpolate_image(pixels, AMG_ROWS, AMG_COLS, dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS);

	uint8_t boxWidth = 128 / INTERPOLATED_COLS;
	uint8_t boxHeight = 128 / INTERPOLATED_COLS;

	//  Serial.print(boxWidth); Serial.print("X"); Serial.println(boxHeight);

	drawpixels(pixels, dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS, boxWidth, boxHeight, false);

//	int t_End = millis();

	tft.setCursor(0, 138);
	tft.fillRect(0, 129, 128, 160 - 128, tft.color565(0, 0, 127));
	tft.setTextColor(ST77XX_WHITE);
	tft.setTextSize(2);
	//tft.print(t_End - t_Start);
	tft.print(colorMapper.getMinTemp(), 0);
	tft.print("/");
	tft.print(colorMapper.getMaxTemp(), 0);
	tft.print(" ");
	tft.print(centralTemperature(pixels, AMG_ROWS, AMG_COLS), 1);
}

