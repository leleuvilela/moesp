#include "config.h"
#include "display_manager.h"
#include "icons.h"
#include "messages.h"
#include "weather.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

// Global objects
DisplayManager display;
WeatherClient weather;

// Time tracking
unsigned long lastTimeUpdate = 0;
unsigned long lastWeatherUpdate = 0;
bool showMorningMessage = true;
unsigned long lastFullRefresh = 0;
int lastMinute = -1;
int lastDay = -1;

// Button handling
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_DELAY = 300;  // ms

// Current time info
struct tm timeInfo;
char dateStr[30];
char timeStr[10];
int dayOfYear = 0;

const char *weeksday[] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};
const char *months[] = {"Jan", "Fev", "Mar", "Abr", "Mai", "Jun",
                        "Jul", "Ago", "Set", "Out", "Nov", "Dez"};

// Update time and date strings from timeInfo
void updateTimeStrings() {
  strftime(timeStr, sizeof(timeStr), "%H:%M", &timeInfo);
  snprintf(dateStr, sizeof(dateStr), "%s, %02d %s %d",
           weeksday[timeInfo.tm_wday], timeInfo.tm_mday,
           months[timeInfo.tm_mon], timeInfo.tm_year + 1900);
  dayOfYear = timeInfo.tm_yday;
}

// Battery reading
float getBatteryVoltage() {
  int raw = analogRead(BATTERY_PIN);
  Serial.printf("Battery raw ADC: %d\n", raw);
  const float voltage =
      (raw / 4095.0) * 3.3 * 2; // Assuming 12-bit ADC and 3.3V ref
  Serial.printf("Battery voltage: %.2f V\n", voltage);
  return voltage;
}

int getBatteryPercentage() {
  float voltage = getBatteryVoltage();
  int percentage =
      (int)((voltage - BATTERY_MIN_V) / (BATTERY_MAX_V - BATTERY_MIN_V) * 100);
  if (percentage > 100)
    percentage = 100;
  if (percentage < 0)
    percentage = 0;
  return percentage;
}

// Check if USB/charging (voltage above max battery = external power)
bool isCharging() {
  float voltage = getBatteryVoltage();
  return voltage > BATTERY_MAX_V + 0.1; // If above 4.3V, likely on USB power
}

// Check button press with debounce
bool checkButton() {
  if (digitalRead(BUTTON_PIN) == LOW) {  // Button pressed (active LOW)
    unsigned long now = millis();
    if (now - lastButtonPress > DEBOUNCE_DELAY) {
      lastButtonPress = now;
      return true;
    }
  }
  return false;
}

// Draw status bar (WiFi + Battery) in top right corner
void drawStatusBar() {
  int x = display.width() - 2; // Start from right edge

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
  if (WiFi.status() == WL_CONNECTED) {
    display.drawBitmap(x, 8, icon_wifi, WIFI_ICON_WIDTH, WIFI_ICON_HEIGHT);
  } else {
    display.drawBitmap(x, 8, icon_wifi_off, WIFI_ICON_WIDTH, WIFI_ICON_HEIGHT);
  }
}

void connectWiFi() {
  Serial.printf("Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 60) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(" Failed to connect");
  }
}

void syncTime() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  if (!getLocalTime(&timeInfo)) {
    Serial.println("Failed to get time from NTP");
    return;
  }

  updateTimeStrings();
  Serial.printf("Time synced: %s %s\n", dateStr, timeStr);
}

void drawMainScreen() {
  display.clear();

  // Draw status bar (WiFi + Battery) in top right
  drawStatusBar();

  // === Top section: Weather info ===
  if (weather.isValid()) {
    WeatherData w = weather.getWeather();

    // Weather icon based on condition
    const unsigned char *icon = getWeatherIcon(w.condition, w.isDay);
    display.drawBitmap(2, 2, icon, ICON_WIDTH, ICON_HEIGHT);

    // Temperature (big) next to icon
    display.setFont(&FreeMonoBold18pt7b);
    display.drawTextAt(weather.getTemperatureString(), 48, 30);

    // Condition text
    display.setFont(&FreeSans9pt7b);
    display.drawTextAt(w.condition, 48, 48);

    // Min/Max temperature
    display.drawTextAt(weather.getMinMaxString(), 150, 30);

    // Rain chance
    char rainStr[15];
    snprintf(rainStr, sizeof(rainStr), "Chuva: %d%%",
             weather.getChanceOfRain());
    display.drawTextAt(rainStr, 150, 48);

    // Toggle between morning message and day suggestion
    bool isMorning = (timeInfo.tm_hour >= 6 && timeInfo.tm_hour < 12);
    String messageToShow = "";

    if (isMorning && showMorningMessage) {
      // Show morning message
      messageToShow = getMorningMessage(dayOfYear);
    } else {
      // Show day suggestion
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
  display.drawText(timeStr, 102, ALIGN_CENTER);

  display.setFont(&FreeSans9pt7b);
  display.drawText(dateStr, 120, ALIGN_CENTER);

  display.update();
}

// Partial update: only time and date at bottom
void partialUpdateTime() {
  display.fillRect(0, 77, display.width(), 45, false);

  display.setFont(&FreeMonoBold18pt7b);
  display.drawText(timeStr, 102, ALIGN_CENTER);

  display.setFont(&FreeSans9pt7b);
  display.drawText(dateStr, 120, ALIGN_CENTER);

  display.partialUpdate(0, 77, display.width(), 45);
}

// Partial update: message area (suggestion/morning message)
void partialUpdateMessage() {
  // Clear message area
  display.fillRect(0, 55, display.width(), 20, false);

  if (weather.isValid()) {
    bool isMorning = (timeInfo.tm_hour >= 6 && timeInfo.tm_hour < 12);
    String messageToShow = "";

    if (isMorning && showMorningMessage) {
      messageToShow = getMorningMessage(dayOfYear);
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
  }

  display.partialUpdate(0, 55, display.width(), 20);
}

// Full refresh: sync time, fetch weather, redraw
void refreshAll() {
  Serial.println("Button pressed - refreshing...");

  // Re-sync time
  syncTime();

  // Re-fetch weather
  if (WiFi.status() == WL_CONNECTED) {
    weather.fetchWeather();
  } else {
    // Try to reconnect WiFi
    connectWiFi();
    if (WiFi.status() == WL_CONNECTED) {
      syncTime();
      weather.fetchWeather();
    }
  }

  // Full screen refresh
  drawMainScreen();
  lastFullRefresh = millis();
  lastWeatherUpdate = millis();
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Morning ESP32 E-Ink Display ===");

  // Initialize button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize display
  display.begin();
  display.clear();
  display.setFont(&FreeSans9pt7b);
  display.drawText("Starting up...", 60, ALIGN_CENTER);
  display.update();

  // Connect to WiFi
  connectWiFi();

  // Sync time
  syncTime();

  // Fetch weather
  if (WiFi.status() == WL_CONNECTED) {
    weather.fetchWeather();
  }

  // Draw main screen
  drawMainScreen();

  lastTimeUpdate = millis();
  lastWeatherUpdate = millis();
  lastFullRefresh = millis();
  lastMinute = timeInfo.tm_min;
  lastDay = timeInfo.tm_yday;
}

void loop() {
  unsigned long now = millis();

  // Check button press for manual refresh
  if (checkButton()) {
    refreshAll();
    return;  // Skip rest of loop this iteration
  }

  // Update local time
  if (now - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
    getLocalTime(&timeInfo);
    updateTimeStrings();
    lastTimeUpdate = now;

    // Check if minute changed
    if (timeInfo.tm_min != lastMinute) {
      lastMinute = timeInfo.tm_min;

      // Toggle morning message every minute
      showMorningMessage = !showMorningMessage;

      // Check if day changed
      if (timeInfo.tm_yday != lastDay) {
        lastDay = timeInfo.tm_yday;
        dayOfYear = timeInfo.tm_yday;
        Serial.println("New day!");
        drawMainScreen(); // Full refresh for new day
      } else {
        // Partial updates only
        partialUpdateTime();
        partialUpdateMessage();
      }
    }
  }

  // Update weather periodically
  if (now - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL) {
    if (weather.fetchWeather()) {
      // Weather changed - partial update of weather area would be complex
      // So we do full refresh, but it's only every 30 min
      drawMainScreen();
    }
    lastWeatherUpdate = now;
  }

  // Full display refresh periodically (prevents ghosting)
  if (now - lastFullRefresh >= FULL_REFRESH_INTERVAL) {
    drawMainScreen();
    lastFullRefresh = now;
  }

  delay(1000); // Small delay to reduce CPU usage
}
