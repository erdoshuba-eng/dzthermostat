#include "ssd1306_091.h"
#include <Wire.h>

#define MIN_RSSI -100
#define MAX_RSSI -40

static const unsigned char PROGMEM wifiOn[] = {
	B00000000,B00000000,
	B00000000,B00000000,
	B00000111,B11100000,
	B00011111,B11111000,
	B00111100,B00111100,
	B01110000,B00001110,
	B11110000,B00001111,
	B11000000,B00000011,
	B00000011,B11000000,
	B00000111,B11100000,
	B00011100,B00111000,
	B00011000,B00011000,
	B00000001,B10000000,
	B00000011,B11000000,
	B00000011,B11000000,
	B00000000,B00000000
};

static const unsigned char PROGMEM heating[] = {
	B00000000,B00000000,
	B00000110,B00000110,
	B00000011,B00000011,
	B00000011,B00000011,
	B00000110,B00000110,
	B00001100,B00001100,
	B00011000,B00011000,
	B00110000,B00110000,
	B01110000,B01110000,
	B01100000,B01100000,
	B11100000,B11100000,
	B11100000,B11100000,
	B01100000,B01100000,
	B00110000,B00110000,
	B00011000,B00011000,
	B00000000,B00000000
};

SSD1306_091::SSD1306_091(uint8_t address) :
_address(address),
displ(SSD1306_WIDTH, SSD1306_HEIGHT, &Wire, SSD1306_RESET) {
    temperature = -1;
	refTemp = 17;
    wifiStrength = -125;
    wifiConnected = false;
    needsHeating = false;
	textColor = SSD1306_WHITE;
}

SSD1306_091::~SSD1306_091() {
    // Nothing to do
}

bool SSD1306_091::init() {
	if (!displ.begin(SSD1306_SWITCHCAPVCC, _address)) {
		return false;
	}
	displ.clearDisplay(); // clear the display buffer
	displ.setTextColor(textColor); // draw white text
	displ.cp437(true); // use full 256 char 'Code Page 437' font
	return true;
}

void SSD1306_091::showText(const char* txt) {
	displ.clearDisplay();
	if (strlen(txt) > 1) {
		displ.drawRect(0, 0, SSD1306_WIDTH, SSD1306_HEIGHT, textColor);
		displ.setCursor(12, 5); // start at top-left corner
		displ.print(txt);
	} else {
		// to recognize that the display is under power, show a pixel in the top left corner
		displ.drawPixel(0, 0, textColor);
	}
	displ.display();
}

void SSD1306_091::showTemperature(double value) {
	char tmp[7];
//   sprintf(tmp, "%0.2fC", median);
	dtostrf(value, 5, 1, tmp);
	// strcat(tmp, "C");
	displ.setTextSize(3);
	showText(tmp);
}

void SSD1306_091::showTemperature() {
	char tmp[7];
	dtostrf(temperature, 4, 1, tmp);
	displ.setTextSize(3);
	displ.setCursor(56, 5); // start at top-left corner
	displ.print(tmp);
}

void SSD1306_091::showWiFiStrength() {
	int clamped = max(min((int)wifiStrength, MAX_RSSI), MIN_RSSI);
	int fill = map(clamped, MIN_RSSI, MAX_RSSI, 0, SSD1306_HEIGHT); // value, fromLow, fromHigh, toLow, toHigh
	displ.drawRect(0, 0, 6, SSD1306_HEIGHT, textColor);
	displ.fillRect(0, SSD1306_HEIGHT - fill, 6, fill, textColor);
}

void SSD1306_091::showRefTemp() {
	char tmp[7];
	dtostrf(refTemp, 4, 1, tmp);
	displ.setTextSize(1);
	displ.setCursor(7, 24); // start at top-left corner
	displ.print(tmp);
}

void SSD1306_091::updateScreen() {
	displ.clearDisplay();

	showWiFiStrength();
	if (wifiConnected) {
		displ.drawBitmap(8, 0, wifiOn, 16, 16, textColor);
	}
    if (needsHeating){
		displ.drawBitmap(26, 0, heating, 16, 16, textColor);
	}
	showRefTemp();
	showTemperature();
	// displ.setTextSize(1);
	// displ.setCursor(35, 0); // start at top-left corner
	// displ.print('k');
	displ.display();
}