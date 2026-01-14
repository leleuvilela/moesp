#ifndef WEATHER_H
#define WEATHER_H

#include "config.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

// RTC-compatible struct (no dynamic allocation)
// Can be stored in RTC memory to persist across deep sleep
struct WeatherData {
  float temperature;
  float feelsLike;
  int humidity;
  char condition[16];      // Fixed size for RTC memory
  char icon[64];           // Fixed size for RTC memory
  bool isDay;
  bool valid;
  // Forecast data
  int chanceOfRain;        // % chance of rain today
  float maxTemp;
  float minTemp;
  char forecastCondition[16];
};

class WeatherClient {
private:
  WeatherData currentWeather;
  unsigned long lastUpdate;

  String buildUrl() {
    return String("http://api.weatherapi.com/v1/forecast.json?key=") +
           WEATHER_API_KEY + "&q=" + WEATHER_LOCATION + "&days=1&lang=pt";
  }

public:
  WeatherClient() : lastUpdate(0) { currentWeather.valid = false; }

  bool fetchWeather() {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi not connected, skipping weather update");
      return false;
    }

    if (String(WEATHER_API_KEY) == "YOUR_API_KEY_HERE") {
      Serial.println("Weather API key not configured");
      currentWeather.valid = false;
      return false;
    }

    HTTPClient http;
    String url = buildUrl();

    Serial.println("Fetching weather from WeatherAPI...");
    http.begin(url);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        // Current weather
        currentWeather.temperature = doc["current"]["temp_c"];
        currentWeather.feelsLike = doc["current"]["feelslike_c"];
        currentWeather.humidity = doc["current"]["humidity"];

        // Copy strings to fixed-size buffers
        const char* condText = doc["current"]["condition"]["text"];
        strncpy(currentWeather.condition, condText ? condText : "", sizeof(currentWeather.condition) - 1);
        currentWeather.condition[sizeof(currentWeather.condition) - 1] = '\0';

        const char* iconText = doc["current"]["condition"]["icon"];
        strncpy(currentWeather.icon, iconText ? iconText : "", sizeof(currentWeather.icon) - 1);
        currentWeather.icon[sizeof(currentWeather.icon) - 1] = '\0';

        currentWeather.isDay = doc["current"]["is_day"] == 1;

        // Forecast data for today
        JsonObject forecastDay = doc["forecast"]["forecastday"][0]["day"];
        currentWeather.chanceOfRain = forecastDay["daily_chance_of_rain"];
        currentWeather.maxTemp = forecastDay["maxtemp_c"];
        currentWeather.minTemp = forecastDay["mintemp_c"];

        const char* forecastText = forecastDay["condition"]["text"];
        strncpy(currentWeather.forecastCondition, forecastText ? forecastText : "", sizeof(currentWeather.forecastCondition) - 1);
        currentWeather.forecastCondition[sizeof(currentWeather.forecastCondition) - 1] = '\0';

        currentWeather.valid = true;
        lastUpdate = millis();

        Serial.printf(
            "Weather: %.1f°C, %s, Chuva: %d%%\n", currentWeather.temperature,
            currentWeather.condition, currentWeather.chanceOfRain);

        http.end();
        return true;
      } else {
        Serial.println("JSON parsing failed");
      }
    } else {
      Serial.printf("HTTP error: %d\n", httpCode);
    }

    http.end();
    currentWeather.valid = false;
    return false;
  }

  WeatherData getWeather() { return currentWeather; }

  // Get pointer to weather data (for RTC memory storage)
  WeatherData* getWeatherPtr() { return &currentWeather; }

  // Load weather data from external source (RTC memory)
  void setWeather(const WeatherData& data) { currentWeather = data; }

  bool isValid() { return currentWeather.valid; }

  bool needsUpdate() {
    // With deep sleep, we track updates differently using RTC memory
    // This function is kept for compatibility but not used
    return (millis() - lastUpdate) > (WEATHER_UPDATE_MIN * 60000UL);
  }

  String getTemperatureString() {
    if (!currentWeather.valid)
      return "--°C";
    char buf[10];
    snprintf(buf, sizeof(buf), "%.0f°C", currentWeather.temperature);
    return String(buf);
  }

  String getHumidityString() {
    if (!currentWeather.valid)
      return "--%";
    char buf[10];
    snprintf(buf, sizeof(buf), "%d%%", currentWeather.humidity);
    return String(buf);
  }

  String getDaySuggestion() {
    if (!currentWeather.valid)
      return "";

    // Rain takes priority
    if (currentWeather.chanceOfRain >= 70) {
      return "Leva guarda-chuva!!!";
    }
    if (currentWeather.chanceOfRain >= 40) {
      return "Cuidado, pode chover";
    }

    // Cold weather check
    if (currentWeather.temperature <= 10 || currentWeather.minTemp <= 8) {
      return "Frio pa porra, momoti!";
    }
    if (currentWeather.temperature <= 15 || currentWeather.minTemp <= 12) {
      return "Leve um casaquinho";
    }

    // Hot weather
    if (currentWeather.maxTemp >= 30) {
      return "Socorro, que calor!";
    }

    // Nice day - no rain, pleasant temperature
    if (currentWeather.chanceOfRain < 20 && currentWeather.maxTemp >= 18 &&
        currentWeather.maxTemp <= 28) {
      return "O dia lindo, igual voce";
    }

    return "GOSTOSA!";
  }

  String getMinMaxString() {
    if (!currentWeather.valid)
      return "";
    char buf[20];
    snprintf(buf, sizeof(buf), "%.0f° / %.0f°", currentWeather.minTemp,
             currentWeather.maxTemp);
    return String(buf);
  }

  int getChanceOfRain() {
    return currentWeather.valid ? currentWeather.chanceOfRain : 0;
  }
};

#endif // WEATHER_H
