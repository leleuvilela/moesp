#ifndef UI_H
#define UI_H

#include "battery.h"
#include "config.h"
#include "display_manager.h"
#include "icons.h"
#include "messages.h"
#include "time_manager.h"
#include "weather.h"
#include "wifi_manager.h"

// Draw status bar (WiFi + Battery) in top right corner
inline void drawStatusBar(DisplayManager &display) {
  int x = display.width() - 2;

  // Battery icon (rightmost)
  const unsigned char *battIcon;
  if (isCharging()) {
    battIcon = icon_battery_charging;
  } else {
    battIcon = getBatteryIcon(getBatteryPercentage());
  }
  x -= BATTERY_ICON_WIDTH;
  display.drawBitmap(x, 8, battIcon, BATTERY_ICON_WIDTH, BATTERY_ICON_HEIGHT);

  // Small gap
  x -= 4;

  // WiFi icon
  x -= WIFI_ICON_WIDTH;
  if (isWiFiConnected()) {
    display.drawBitmap(x, 8, icon_wifi, WIFI_ICON_WIDTH, WIFI_ICON_HEIGHT);
  } else {
    display.drawBitmap(x, 8, icon_wifi_off, WIFI_ICON_WIDTH, WIFI_ICON_HEIGHT);
  }
}

// Draw the main screen with weather, time, and messages
inline void drawMainScreen(DisplayManager &display, WeatherClient &weather,
                           bool showMorningMessage) {
  display.clear();

  // Draw status bar (WiFi + Battery) in top right
  drawStatusBar(display);

  // === Top section: Weather info ===
  if (weather.isValid()) {
    WeatherData w = weather.getWeather();

    // Weather icon based on condition
    const unsigned char *icon = getWeatherIcon(w.condition, w.isDay);
    display.drawBitmap(2, 2, icon, ICON_WIDTH, ICON_HEIGHT);

    // Temperature (big) next to icon
    display.setFont(&FreeMonoBold18pt7b);
    display.drawTextAt(weather.getTemperatureString(), 48, 30);

    // Condition text (max 10 chars)
    display.setFont(&FreeSans9pt7b);
    char condTrunc[11];
    strncpy(condTrunc, w.condition, 10);
    condTrunc[10] = '\0';
    display.drawTextAt(condTrunc, 48, 48);

    // Min/Max temperature
    display.drawTextAt(weather.getMinMaxString(), 150, 30);

    // Rain chance
    char rainStr[15];
    snprintf(rainStr, sizeof(rainStr), "Chuva: %d%%", weather.getChanceOfRain());
    display.drawTextAt(rainStr, 150, 48);

    // Toggle between morning message and day suggestion
    String messageToShow = "";

    if (isMorning() && showMorningMessage) {
      messageToShow = getMorningMessage(getDayOfYear());
    } else {
      messageToShow = weather.getDaySuggestion();
    }

    if (messageToShow.length() > 0) {
      if (messageToShow.length() > 25) {
        display.setFont(&FreeSans9pt7b);
      } else {
        display.setFont(&FreeSansBold9pt7b);
      }
    }
    display.drawText(messageToShow, 68, ALIGN_CENTER);
  } else {
    // No weather data - draw placeholder
    display.drawRect(2, 2, ICON_WIDTH, ICON_HEIGHT);
    display.setFont(&FreeSans9pt7b);
    display.drawTextAt("Clima: --", 48, 30);
  }

  // Separator line
  display.drawLine(0, 76, display.width(), 76);

  // === Bottom section: Time and Date ===
  display.setFont(&FreeMonoBold18pt7b);
  display.drawText(getTimeStr(), 102, ALIGN_CENTER);

  display.setFont(&FreeSans9pt7b);
  display.drawText(getDateStr(), 120, ALIGN_CENTER);

  display.update();
}

#endif // UI_H
