#ifndef BATTERY_H
#define BATTERY_H

#include "config.h"
#include <Arduino.h>

// Read battery voltage from ADC
inline float getBatteryVoltage() {
  int raw = analogRead(BATTERY_PIN);
  // 12-bit ADC, 3.3V reference, voltage divider factor of 2
  float voltage = (raw / 4095.0) * 3.3 * 2;
  return voltage;
}

// Get battery percentage (0-100)
inline int getBatteryPercentage() {
  float voltage = getBatteryVoltage();
  int percentage =
      (int)((voltage - BATTERY_MIN_V) / (BATTERY_MAX_V - BATTERY_MIN_V) * 100);

  if (percentage > 100)
    percentage = 100;
  if (percentage < 0)
    percentage = 0;

  return percentage;
}

// Check if device is charging (USB power detected)
// Returns true if voltage is above max battery voltage
inline bool isCharging() {
  float voltage = getBatteryVoltage();
  return voltage > BATTERY_MAX_V + 0.1;
}

#endif // BATTERY_H
