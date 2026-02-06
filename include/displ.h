#ifndef DISPL_H
#define DISPL_H

#include <Arduino.h>

#define SSD1306
// #define LCD_28_TOUCH

#ifdef SSD1306
#include <Adafruit_SSD1306.h>

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 32
#define SSD1306_ADDRESS 0x3C // 0x3D for 128x64, 0x3C for 128x32
#define SSD1306_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

class SSD1306_091 {
public:
    int8_t wifiStrength;
    bool wifiConnected;
    bool needsHeating;
    SSD1306_091(uint8_t address = SSD1306_ADDRESS);
    ~SSD1306_091();

    bool init();
    void showText(const char* txt);
    void updateScreen();

    void setIsReading(bool value) { isReadingTemperature = value; }
    void setRefTemp(double value) {
        if (value == refTemp) { return; }
        // dtostrf(refTemp, 4, 1, _refTemp);
        refTemp = value;
    }
    void setTemperature(double value) {
        if (value == temperature) { return; }
        // dtostrf(temperature, 4, 1, _temperature);
        temperature = value;
    }

private:
    uint8_t _address;
    uint16_t textColor;
    Adafruit_SSD1306 displ;

    bool isReadingTemperature;
    double temperature;
    // char _temperature[7];
    double refTemp; // reference temperature, used to switch on/off the furnace
    // char _refTemp[7];
    void showWiFiStrength();
    void showTemperature();
    void showRefTemp();
};
#endif

#ifdef LCD_28_TOUCH
#include <Adafruit_ILI9341.h>

class LCD28Touch {
public:
    int8_t wifiStrength;
    bool wifiConnected;
    bool needsHeating;
    LCD28Touch();
    ~LCD28Touch();

    bool init();
    void clearScreen() { displ.fillScreen(bgColor); }
    void diagnostics();
    void updateScreen();

    void setIsReading(bool value) { isReadingTemperature = value; }
    void setRefTemp(double value) {
        if (value == refTemp) { return; }
        dtostrf(refTemp, 4, 1, _refTemp);
        refTemp = value;
        refTempChanged = true;
    }
    void setTemperature(double value) {
        if (value == temperature) { return; }
        dtostrf(temperature, 4, 1, _temperature);
        temperature = value;
        temperatureChanged = true;
    }

private:
    uint16_t bgColor;
    uint16_t textColor;
    Adafruit_ILI9341 displ;

    bool isReadingTemperature;
    double temperature;
    char _temperature[7];
    bool temperatureChanged;
    double refTemp; // reference temperature, used to switch on/off the furnace
    char _refTemp[7];
    bool refTempChanged;
    void showButtons();
    void showNeedsHeating();
    void showIsReading();
    void showRefTemp();
    void showTemperature();
    void showWiFi();
    void showWiFiStrength();

    unsigned long testFillScreen();
    unsigned long testText();
    unsigned long testLines(uint16_t color);
    unsigned long testFastLines(uint16_t color1, uint16_t color2);
    unsigned long testRects(uint16_t color);
    unsigned long testFilledRects(uint16_t color1, uint16_t color2);
    unsigned long testFilledCircles(uint8_t radius, uint16_t color);
    unsigned long testCircles(uint8_t radius, uint16_t color);
    unsigned long testTriangles();
    unsigned long testFilledTriangles();
    unsigned long testRoundRects();
    unsigned long testFilledRoundRects();
};
#endif

#endif /* DISPL_H */