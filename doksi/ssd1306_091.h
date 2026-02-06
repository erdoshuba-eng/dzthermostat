#ifndef SSD1306_091_H
#define SSD1306_091_H

#include <Arduino.h>
// #include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 32
#define SSD1306_ADDRESS 0x3C // 0x3D for 128x64, 0x3C for 128x32
#define SSD1306_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

class SSD1306_091 {
public:
    double temperature;
    double refTemp; // reference temperature, used to switch on/off the furnace
    int8_t wifiStrength;
    bool wifiConnected;
    bool needsHeating;
    SSD1306_091(uint8_t address = SSD1306_ADDRESS);
    ~SSD1306_091();

    bool init();
    void showText(const char* txt);
    void showTemperature(double value);
    void updateScreen();
/*    void begin();
    void clear();
    void display();
    void drawPixel(int16_t x, int16_t y, uint16_t color = 1);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color = 1);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color = 1);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color = 1);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color = 1);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color = 1);
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color = 1, uint16_t bg = 0, uint8_t size = 1);
    void drawText(int16_t x, int16_t y, const char* text, uint16_t color = 1, uint16_t bg = 0, uint8_t size = 1);
    void setCursor(int16_t x, int16_t y);
    void setTextSize(uint8_t s);
    void setTextColor(uint16_t c);
    void setTextColor(uint16_t c, uint16_t bg);*/

private:
    uint8_t _address;
    uint16_t textColor;
    Adafruit_SSD1306 displ;
    void showWiFiStrength();
    void showTemperature();
    void showRefTemp();
/*    uint8_t _buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
    int16_t _cursor_x, _cursor_y;
    uint8_t _textsize;
    uint16_t _textcolor, _textbgcolor;

    void sendCommand(uint8_t command);
    void sendData(uint8_t data);
    void sendBuffer();
    void initDisplay();*/
};

#endif // SSD1306_091_H