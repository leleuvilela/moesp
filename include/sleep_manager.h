#ifndef SLEEP_MANAGER_H
#define SLEEP_MANAGER_H

#include "config.h"
#include <Arduino.h>
#include <esp_sleep.h>

// Get the reason we woke up from sleep
inline esp_sleep_wakeup_cause_t getWakeupReason() {
  return esp_sleep_get_wakeup_cause();
}

// Print wakeup reason to serial (for debugging)
inline void printWakeupReason() {
  esp_sleep_wakeup_cause_t reason = getWakeupReason();

  switch (reason) {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup: Button press (EXT0)");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup: External (EXT1)");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup: Timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup: Touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup: ULP program");
    break;
  default:
    Serial.printf("Wakeup: First boot or reset (%d)\n", reason);
    break;
  }
}

// Check if wakeup was from button press
inline bool isButtonWakeup() {
  return getWakeupReason() == ESP_SLEEP_WAKEUP_EXT0;
}

// Check if wakeup was from timer
inline bool isTimerWakeup() {
  return getWakeupReason() == ESP_SLEEP_WAKEUP_TIMER;
}

// Configure sleep wake sources with default duration
inline void configureSleep() {
  // Timer wakeup - wake after SLEEP_DURATION_SEC seconds
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_SEC * 1000000ULL);

  // Button wakeup (EXT0) - wake on LOW level (button pressed)
  // GPIO 39 is input-only and supports RTC wakeup
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);

  Serial.printf("Sleep configured: timer=%ds, button=GPIO39\n",
                SLEEP_DURATION_SEC);
}

// Configure sleep with custom duration (for remote mode)
inline void configureSleepDuration(int durationSec) {
  // Timer wakeup - wake after specified seconds
  esp_sleep_enable_timer_wakeup(durationSec * 1000000ULL);

  // Button wakeup (EXT0) - wake on LOW level (button pressed)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);

  Serial.printf("Sleep configured: timer=%ds, button=GPIO39\n", durationSec);
}

// Enter deep sleep mode
// Note: This function does not return - device restarts on wake
inline void enterDeepSleep() {
  Serial.println("Entering deep sleep...");
  Serial.flush();

  // Small delay to ensure serial output completes
  delay(100);

  // Enter deep sleep - execution stops here until wake
  esp_deep_sleep_start();

  // This line is never reached
}

#endif // SLEEP_MANAGER_H
