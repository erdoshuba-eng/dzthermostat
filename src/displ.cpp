#include "displ.h"
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

#ifdef SSD1306
SSD1306_091::SSD1306_091(uint8_t address) :
_address(address),
displ(SSD1306_WIDTH, SSD1306_HEIGHT, &Wire, SSD1306_RESET) {
  isReadingTemperature = false;
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

// void SSD1306_091::showTemperature(double value) {
// 	char tmp[7];
// //   sprintf(tmp, "%0.2fC", median);
// 	dtostrf(value, 5, 1, tmp);
// 	// strcat(tmp, "C");
// 	displ.setTextSize(3);
// 	showText(tmp);
// }

void SSD1306_091::showTemperature() {
	displ.setTextSize(3);
	char tmp[7];
	dtostrf(temperature, 4, 1, tmp);
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
#endif

#ifdef LCD_28_TOUCH
#define TFT_CS 15 // D8
#define TFT_DC 0 // D3
#define TFT_RST 2 // D4
// #define TOUCH_CS 4 // D2

LCD28Touch::LCD28Touch() :
displ(TFT_CS, TFT_DC, TFT_RST) {
  isReadingTemperature = false;
  temperature = 0;
  dtostrf(temperature, 4, 1, _temperature);
  temperatureChanged = false;
	refTemp = 17;
  dtostrf(refTemp, 4, 1, _refTemp);
  refTempChanged = true;
  wifiStrength = -125;
  wifiConnected = false;
  needsHeating = false;
	textColor = ILI9341_WHITE;
}

LCD28Touch::~LCD28Touch() {}

bool LCD28Touch::init() {
	displ.begin();
	displ.setRotation(1);
  bgColor = ILI9341_BLACK;
  clearScreen();
  showButtons();
	return true;
}

void LCD28Touch::updateScreen() {
	showWiFiStrength();
  showWiFi();
  showNeedsHeating();

  showIsReading();
	showRefTemp();
	showTemperature();
}

void LCD28Touch::showButtons() {
  displ.drawLine(20, 92, displ.width() - 20, 92, textColor);

  uint8_t textSize = 5;
  int16_t dx = 10;
  int16_t dy = 5;
  int16_t x = 0;
  int16_t y = 190;
  int16_t w = (320 - 4 * textSize * 6) / 2;
  int16_t h = textSize * 8;
  displ.drawTriangle(x + dx, y + dy, x + w / 2, y + h - dy, x + w - dx, y + dy, textColor);
  x += w + 4 * textSize * 6;
  displ.drawTriangle(x + dx, y + h - dy, x + w / 2, y + dy, x + w - dx, y + h - dy, textColor);
}

void LCD28Touch::showWiFi() {
  int16_t x = 15;
  int16_t y = 110;
  displ.fillRect(x, y, 16, 16, bgColor);
	if (wifiConnected) {
		displ.drawBitmap(x, y, wifiOn, 16, 16, textColor);
	}
}

void LCD28Touch::showNeedsHeating() {
  int16_t x = 32;
  int16_t y = 110;
  displ.fillRect(x, y, 16, 16, bgColor);
  if (needsHeating){
		displ.drawBitmap(x, y, heating, 16, 16, textColor);
	}
}

void LCD28Touch::showIsReading() {
  int16_t x = 60;
  int16_t y = 110 + 8;
  if (isReadingTemperature) {
    displ.fillCircle(x, y, 8, ILI9341_MAGENTA);
  } else {
    displ.fillCircle(x, y, 8, ILI9341_BLACK);
  }
}

void LCD28Touch::showRefTemp() {
  if (!refTempChanged) { return; }
  uint8_t textSize = 5;
  int16_t x = (320 - 4 * textSize * 6) / 2;
  int16_t y = 190;
	displ.setTextSize(textSize);
  displ.setTextColor(bgColor);
	displ.setCursor(x, y); // start at top-left corner
	displ.print(_refTemp);
	char tmp[7];
	dtostrf(refTemp, 4, 1, tmp);
  displ.setTextColor(ILI9341_WHITE);
	displ.setCursor(x, y); // start at top-left corner
	displ.print(tmp);
  refTempChanged = false;
}

void LCD28Touch::showTemperature() {
  if (!temperatureChanged) { return; }
  uint8_t textSize = 10;
  int16_t x = (320 - 4 * textSize * 6) / 2;
  int16_t y = 5;
  // displ.fillRect(x, y, 4 * textSize * 8, textSize * 8, bgColor);
	displ.setTextSize(textSize);
  displ.setTextColor(bgColor);
	displ.setCursor(x, y); // start at top-left corner
	displ.print(_temperature);
  char tmp[7];
	dtostrf(temperature, 4, 1, tmp);
  displ.setTextColor(ILI9341_WHITE);
	displ.setCursor(x, y); // start at top-left corner
	displ.print(tmp);
  temperatureChanged = false;
}

void LCD28Touch::showWiFiStrength() {
	int clamped = max(min((int)wifiStrength, MAX_RSSI), MIN_RSSI);
  int16_t x = 5;
  int16_t y = 110;
  int16_t h = 48;
  int16_t w = 8;
	int fill = map(clamped, MIN_RSSI, MAX_RSSI, 0, h); // value, fromLow, fromHigh, toLow, toHigh
  displ.fillRect(x, y, w, h, textColor);
  displ.fillRect(x + 1, y + 1, w - 2, h - fill - 1, bgColor); // not filled area
}

void LCD28Touch::diagnostics() {
	// uint8_t x = displ.readcommand8(ILI9341_RDMODE);
	// Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
	// x = displ.readcommand8(ILI9341_RDMADCTL);
	// Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
	// x = displ.readcommand8(ILI9341_RDPIXFMT);
	// Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
	// x = displ.readcommand8(ILI9341_RDIMGFMT);
	// Serial.print("Image Format: 0x"); Serial.println(x, HEX);
	// x = displ.readcommand8(ILI9341_RDSELFDIAG);
	// Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);

	// displ.fillScreen(ILI9341_RED);  Serial.println(testFillScreen());

  Serial.printf("w: %u, h: %u\n", displ.width(), displ.height());
	Serial.print(F("Text                     "));
	Serial.println(testText());
	delay(3000);

	// Serial.print(F("Lines                    "));
	// Serial.println(testLines(ILI9341_CYAN));
	// delay(500);

	// Serial.print(F("Horiz/Vert Lines         "));
	// Serial.println(testFastLines(ILI9341_RED, ILI9341_BLUE));
	// delay(500);

	// Serial.print(F("Rectangles (outline)     "));
	// Serial.println(testRects(ILI9341_GREEN));
	// delay(500);

	// Serial.print(F("Rectangles (filled)      "));
	// Serial.println(testFilledRects(ILI9341_YELLOW, ILI9341_MAGENTA));
	// delay(500);

	// Serial.print(F("Circles (filled)         "));
	// Serial.println(testFilledCircles(10, ILI9341_MAGENTA));

	// Serial.print(F("Circles (outline)        "));
	// Serial.println(testCircles(10, ILI9341_WHITE));
	// delay(500);

	// Serial.print(F("Triangles (outline)      "));
	// Serial.println(testTriangles());
	// delay(500);

	// Serial.print(F("Triangles (filled)       "));
	// Serial.println(testFilledTriangles());
	// delay(500);

	// Serial.print(F("Rounded rects (outline)  "));
	// Serial.println(testRoundRects());
	// delay(500);

	// Serial.print(F("Rounded rects (filled)   "));
	// Serial.println(testFilledRoundRects());
	// delay(500);

	// Serial.println(F("Done!"));
}

unsigned long LCD28Touch::testFillScreen() {
	unsigned long start = micros();
	displ.fillScreen(ILI9341_BLACK);
	displ.fillScreen(ILI9341_RED);
	displ.fillScreen(ILI9341_GREEN);
	displ.fillScreen(ILI9341_BLUE);
	displ.fillScreen(ILI9341_BLACK);
	return micros() - start;
}

unsigned long LCD28Touch::testText() {
  displ.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  displ.setCursor(0, 0);
  displ.setTextColor(ILI9341_WHITE);  displ.setTextSize(1);
  displ.println("Hello World!");
  displ.setTextColor(ILI9341_YELLOW); displ.setTextSize(2);
  displ.println(1234.56);
  displ.setTextColor(ILI9341_RED);    displ.setTextSize(3);
  displ.println(0xDEADBEEF, HEX);
  displ.println();
  displ.setTextColor(ILI9341_GREEN);
  displ.setTextSize(5);
  displ.println("Groop");
  displ.setTextSize(2);
  displ.println("I implore thee,");
  displ.setTextSize(1);
  displ.println("my foonting turlingdromes.");
  displ.println("And hooptiously drangle me");
  displ.println("with crinkly bindlewurdles,");
  displ.println("Or I will rend thee");
  displ.println("in the gobberwarts");
  displ.println("with my blurglecruncheon,");
  displ.println("see if I don't!");
  return micros() - start;
}

unsigned long LCD28Touch::testLines(uint16_t color) {
  unsigned long start, t;
  int           x1, y1, x2, y2,
                w = displ.width(),
                h = displ.height();

  displ.fillScreen(ILI9341_BLACK);

  x1 = y1 = 0;
  y2    = h - 1;
  start = micros();
  for(x2=0; x2<w; x2+=6) displ.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for(y2=0; y2<h; y2+=6) displ.drawLine(x1, y1, x2, y2, color);
  t     = micros() - start; // fillScreen doesn't count against timing

  displ.fillScreen(ILI9341_BLACK);

  x1    = w - 1;
  y1    = 0;
  y2    = h - 1;
  start = micros();
  for(x2=0; x2<w; x2+=6) displ.drawLine(x1, y1, x2, y2, color);
  x2    = 0;
  for(y2=0; y2<h; y2+=6) displ.drawLine(x1, y1, x2, y2, color);
  t    += micros() - start;

  displ.fillScreen(ILI9341_BLACK);

  x1    = 0;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for(x2=0; x2<w; x2+=6) displ.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for(y2=0; y2<h; y2+=6) displ.drawLine(x1, y1, x2, y2, color);
  t    += micros() - start;

  displ.fillScreen(ILI9341_BLACK);

  x1    = w - 1;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for(x2=0; x2<w; x2+=6) displ.drawLine(x1, y1, x2, y2, color);
  x2    = 0;
  for(y2=0; y2<h; y2+=6) displ.drawLine(x1, y1, x2, y2, color);

  return micros() - start;
}

unsigned long LCD28Touch::testFastLines(uint16_t color1, uint16_t color2) {
  unsigned long start;
  int           x, y, w = displ.width(), h = displ.height();

  displ.fillScreen(ILI9341_BLACK);
  start = micros();
  for(y=0; y<h; y+=5) displ.drawFastHLine(0, y, w, color1);
  for(x=0; x<w; x+=5) displ.drawFastVLine(x, 0, h, color2);

  return micros() - start;
}

unsigned long LCD28Touch::testRects(uint16_t color) {
  unsigned long start;
  int           n, i, i2,
                cx = displ.width()  / 2,
                cy = displ.height() / 2;

  displ.fillScreen(ILI9341_BLACK);
  n     = min(displ.width(), displ.height());
  start = micros();
  for(i=2; i<n; i+=6) {
    i2 = i / 2;
    displ.drawRect(cx-i2, cy-i2, i, i, color);
  }

  return micros() - start;
}

unsigned long LCD28Touch::testFilledRects(uint16_t color1, uint16_t color2) {
  unsigned long start, t = 0;
  int           n, i, i2,
                cx = displ.width()  / 2 - 1,
                cy = displ.height() / 2 - 1;

  displ.fillScreen(ILI9341_BLACK);
  n = min(displ.width(), displ.height());
  for(i=n; i>0; i-=6) {
    i2    = i / 2;
    start = micros();
    displ.fillRect(cx-i2, cy-i2, i, i, color1);
    t    += micros() - start;
    // Outlines are not included in timing results
    displ.drawRect(cx-i2, cy-i2, i, i, color2);
  }

  return t;
}

unsigned long LCD28Touch::testFilledCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int x, y, w = displ.width(), h = displ.height(), r2 = radius * 2;

  displ.fillScreen(ILI9341_BLACK);
  start = micros();
  for(x=radius; x<w; x+=r2) {
    for(y=radius; y<h; y+=r2) {
      displ.fillCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long LCD28Touch::testCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int           x, y, r2 = radius * 2,
                w = displ.width()  + radius,
                h = displ.height() + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  start = micros();
  for(x=0; x<w; x+=r2) {
    for(y=0; y<h; y+=r2) {
      displ.drawCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long LCD28Touch::testTriangles() {
  unsigned long start;
  int           n, i, cx = displ.width()  / 2 - 1,
                      cy = displ.height() / 2 - 1;

  displ.fillScreen(ILI9341_BLACK);
  n     = min(cx, cy);
  start = micros();
  for(i=0; i<n; i+=5) {
    displ.drawTriangle(
      cx    , cy - i, // peak
      cx - i, cy + i, // bottom left
      cx + i, cy + i, // bottom right
      displ.color565(0, 0, i));
  }

  return micros() - start;
}

unsigned long LCD28Touch::testFilledTriangles() {
  unsigned long start, t = 0;
  int           i, cx = displ.width()  / 2 - 1,
                   cy = displ.height() / 2 - 1;

  displ.fillScreen(ILI9341_BLACK);
  start = micros();
  for(i=min(cx,cy); i>10; i-=5) {
    start = micros();
    displ.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
      displ.color565(0, i, i));
    t += micros() - start;
    displ.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
      displ.color565(i, i, 0));
  }

  return t;
}

unsigned long LCD28Touch::testRoundRects() {
  unsigned long start;
  int           w, i, i2,
                cx = displ.width()  / 2 - 1,
                cy = displ.height() / 2 - 1;

  displ.fillScreen(ILI9341_BLACK);
  w     = min(displ.width(), displ.height());
  start = micros();
  for(i=0; i<w; i+=6) {
    i2 = i / 2;
    displ.drawRoundRect(cx-i2, cy-i2, i, i, i/8, displ.color565(i, 0, 0));
  }

  return micros() - start;
}

unsigned long LCD28Touch::testFilledRoundRects() {
  unsigned long start;
  int           i, i2,
                cx = displ.width()  / 2 - 1,
                cy = displ.height() / 2 - 1;

  displ.fillScreen(ILI9341_BLACK);
  start = micros();
  for(i=min(displ.width(), displ.height()); i>20; i-=6) {
    i2 = i / 2;
    displ.fillRoundRect(cx-i2, cy-i2, i, i, i/8, displ.color565(0, i, 0));
  }

  return micros() - start;
}
#endif