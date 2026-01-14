#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "config.h"
#include <Arduino.h>
#include <time.h>

// Global time info structure
static struct tm timeInfo;
static char dateStr[30];
static char timeStr[10];
static int dayOfYear = 0;

static const char *weeksday[] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};
static const char *months[] = {"Jan", "Fev", "Mar", "Abr", "Mai", "Jun",
                               "Jul", "Ago", "Set", "Out", "Nov", "Dez"};

// Sync time from NTP server
inline bool syncTime() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  if (!getLocalTime(&timeInfo, 5000)) {
    Serial.println("Failed to get time from NTP");
    return false;
  }

  // Update formatted strings
  strftime(timeStr, sizeof(timeStr), "%H:%M", &timeInfo);
  snprintf(dateStr, sizeof(dateStr), "%s, %02d %s %d",
           weeksday[timeInfo.tm_wday], timeInfo.tm_mday,
           months[timeInfo.tm_mon], timeInfo.tm_year + 1900);
  dayOfYear = timeInfo.tm_yday;

  Serial.printf("Time synced: %s %s\n", dateStr, timeStr);
  return true;
}

// Get pointer to time info structure
inline struct tm *getTimeInfo() {
  return &timeInfo;
}

// Get formatted time string (HH:MM)
inline const char *getTimeStr() {
  return timeStr;
}

// Get formatted date string
inline const char *getDateStr() {
  return dateStr;
}

// Get day of year (0-365)
inline int getDayOfYear() {
  return dayOfYear;
}

// Get current time as minutes since midnight
inline int getMinutesSinceMidnight() {
  return timeInfo.tm_hour * 60 + timeInfo.tm_min;
}

// Check if it's morning (6:00 - 12:00)
inline bool isMorning() {
  return timeInfo.tm_hour >= 6 && timeInfo.tm_hour < 12;
}

#endif // TIME_MANAGER_H
