#include "config.h"
#include "display_manager.h"
#include "remote_mode.h"
#include "sleep_manager.h"
#include "time_manager.h"
#include "ui.h"
#include "weather.h"
#include "wifi_manager.h"
#include <Arduino.h>

// ==================== RTC Memory ====================
// These variables persist through deep sleep
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int lastWeatherMinute = -1;
RTC_DATA_ATTR int lastFullRefreshCount = 0;
RTC_DATA_ATTR bool showMorningMessage = true;
RTC_DATA_ATTR WeatherData savedWeather = {0}; // Persisted weather data

// Remote mode state (persists through deep sleep)
RTC_DATA_ATTR bool remoteMode = false;
RTC_DATA_ATTR uint8_t remoteImageBuffer[BITMAP_SIZE + 100];
RTC_DATA_ATTR size_t remoteImageSize = 0;
RTC_DATA_ATTR int remoteSleepDuration = REMOTE_REFRESH_SEC;

// ==================== Global Objects ====================
DisplayManager display;
WeatherClient weather;

void setup() {
  // Increment boot counter
  bootCount++;

  // Initialize serial
  Serial.begin(115200);
  Serial.println("\n=== Morning ESP32 E-Ink Display ===");
  Serial.printf("Boot count: %d\n", bootCount);
  Serial.printf("Remote mode: %s\n", remoteMode ? "YES" : "NO");

  // Print wakeup reason
  printWakeupReason();

  // Check if this is a button wake
  bool buttonWake = isButtonWakeup();

  // Initialize display
  display.begin();

  // Load saved weather data from RTC memory
  if (savedWeather.valid) {
    weather.setWeather(savedWeather);
    Serial.println("Loaded weather from RTC memory");
  }

  // Show startup message on first boot
  if (bootCount == 1) {
    display.clear();
    display.setFont(&FreeSans9pt7b);
    display.drawText("Starting up...", 60, ALIGN_CENTER);
    display.update();
  }

  // Connect to WiFi
  bool wifiConnected = connectWiFi();

  // ==================== Remote Mode Check ====================
  // Check remote mode if:
  // - Currently in remote mode (need to refresh/check if still active)
  // - Button pressed (manual check)
  // - Every REMOTE_CHECK_CYCLES wakes in normal mode
  bool shouldCheckRemote = remoteMode || buttonWake ||
                           (bootCount % REMOTE_CHECK_CYCLES == 0);

  if (shouldCheckRemote && wifiConnected) {
    Serial.println("Checking remote mode status...");
    RemoteModeResponse response =
        checkRemoteMode(remoteImageBuffer, &remoteImageSize);

    if (response.success) {
      if (response.isRemote) {
        remoteMode = true;
        remoteSleepDuration = response.refreshSeconds;
        Serial.printf("Entering remote mode (refresh: %ds)\n",
                      remoteSleepDuration);
      } else {
        if (remoteMode) {
          Serial.println("Exiting remote mode, returning to normal");
        }
        remoteMode = false;
      }
    } else {
      Serial.println("Remote check failed, keeping current mode");
    }
  }

  // ==================== Display Update ====================
  if (remoteMode && remoteImageSize > 0) {
    // Remote mode: draw the remote image
    drawRemoteImage(display, remoteImageBuffer, remoteImageSize);
  } else {
    // Normal mode: weather and time display

    // Sync time from NTP
    bool timeOk = syncTime();
    if (!timeOk) {
      Serial.println("Time sync failed, will retry next wake");
    }

    // Check if weather needs update
    int currentMinute = getMinutesSinceMidnight();
    bool needWeather = false;

    if (buttonWake) {
      Serial.println("Button pressed - forcing weather update");
      needWeather = true;
    } else if (lastWeatherMinute < 0) {
      Serial.println("First boot - fetching weather");
      needWeather = true;
    } else {
      int minutesSinceWeather = currentMinute - lastWeatherMinute;
      if (minutesSinceWeather < 0) {
        minutesSinceWeather += 24 * 60;
      }
      if (minutesSinceWeather >= WEATHER_UPDATE_MIN) {
        Serial.printf("Weather update needed (%d min since last)\n",
                      minutesSinceWeather);
        needWeather = true;
      }
    }

    // Fetch weather if needed
    if (needWeather && wifiConnected) {
      if (weather.fetchWeather()) {
        lastWeatherMinute = currentMinute;
        savedWeather = weather.getWeather();
        Serial.printf("Weather updated and saved, next update in %d min\n",
                      WEATHER_UPDATE_MIN);
      }
    }

    // Toggle morning message on each wake
    showMorningMessage = !showMorningMessage;

    // Check if full refresh is needed (prevents ghosting)
    lastFullRefreshCount++;
    if (lastFullRefreshCount >= FULL_REFRESH_CYCLES) {
      Serial.println("Performing full display refresh");
      lastFullRefreshCount = 0;
    }

    // Draw the main screen
    drawMainScreen(display, weather, showMorningMessage);
  }

  // Disconnect WiFi to save power
  disconnectWiFi();

  // Configure sleep duration based on mode
  if (remoteMode) {
    configureSleepDuration(remoteSleepDuration);
  } else {
    configureSleep();
  }

  enterDeepSleep();

  // This line is never reached
}

void loop() {
  // Never reached - we deep sleep from setup()
  // ESP32 restarts from setup() on each wake
}
