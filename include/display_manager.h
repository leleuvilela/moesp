#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <GxEPD.h>
#include <GxDEPG0213BN/GxDEPG0213BN.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "config.h"

enum TextAlignment {
    ALIGN_LEFT = 0,
    ALIGN_CENTER,
    ALIGN_RIGHT
};

class DisplayManager {
private:
    GxIO_Class* io;
    GxEPD_Class* display;

public:
    DisplayManager() : io(nullptr), display(nullptr) {}

    void begin() {
        SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, ELINK_SS);
        io = new GxIO_Class(SPI, ELINK_SS, ELINK_DC, ELINK_RESET);
        display = new GxEPD_Class(*io, ELINK_RESET, ELINK_BUSY);

        display->init();
        display->setRotation(DISPLAY_ROTATION);
        display->fillScreen(GxEPD_WHITE);
        display->setTextColor(GxEPD_BLACK);
    }

    void clear() {
        display->fillScreen(GxEPD_WHITE);
    }

    void update() {
        display->update();
    }

    void partialUpdate(int16_t x, int16_t y, int16_t w, int16_t h) {
        display->updateWindow(x, y, w, h, true);
    }

    void setFont(const GFXfont* font) {
        display->setFont(font);
    }

    void drawText(const String& text, int16_t y, TextAlignment align) {
        int16_t x = 0;
        int16_t x1, y1;
        uint16_t w, h;

        display->getTextBounds(text, 0, y, &x1, &y1, &w, &h);

        switch (align) {
            case ALIGN_LEFT:
                x = 2;
                break;
            case ALIGN_CENTER:
                x = (display->width() - w) / 2;
                break;
            case ALIGN_RIGHT:
                x = display->width() - w - 2;
                break;
        }

        display->setCursor(x, y);
        display->print(text);
    }

    void drawTextAt(const String& text, int16_t x, int16_t y) {
        display->setCursor(x, y);
        display->print(text);
    }

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
        display->drawLine(x0, y0, x1, y1, GxEPD_BLACK);
    }

    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h) {
        display->drawRect(x, y, w, h, GxEPD_BLACK);
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, bool black = true) {
        display->fillRect(x, y, w, h, black ? GxEPD_BLACK : GxEPD_WHITE);
    }

    void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h) {
        display->drawBitmap(x, y, bitmap, w, h, GxEPD_BLACK);
    }

    int16_t width() { return display->width(); }
    int16_t height() { return display->height(); }

    GxEPD_Class* getDisplay() { return display; }
};

#endif // DISPLAY_MANAGER_H
