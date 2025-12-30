#ifndef WEATHER_H
#define WEATHER_H

#include "config.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

struct WeatherData {
  float temperature;
  float feelsLike;
  int humidity;
  String condition;
  String icon;
  bool isDay;
  bool valid;
  // Forecast data
  int chanceOfRain; // % chance of rain today
  float maxTemp;
  float minTemp;
  String forecastCondition;
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
        currentWeather.condition =
            doc["current"]["condition"]["text"].as<String>();
        currentWeather.icon = doc["current"]["condition"]["icon"].as<String>();
        currentWeather.isDay = doc["current"]["is_day"] == 1;

        // Forecast data for today
        JsonObject forecastDay = doc["forecast"]["forecastday"][0]["day"];
        currentWeather.chanceOfRain = forecastDay["daily_chance_of_rain"];
        currentWeather.maxTemp = forecastDay["maxtemp_c"];
        currentWeather.minTemp = forecastDay["mintemp_c"];
        currentWeather.forecastCondition =
            forecastDay["condition"]["text"].as<String>();

        currentWeather.valid = true;
        lastUpdate = millis();

        Serial.printf(
            "Weather: %.1f°C, %s, Chuva: %d%%\n", currentWeather.temperature,
            currentWeather.condition.c_str(), currentWeather.chanceOfRain);

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

  bool isValid() { return currentWeather.valid; }

  bool needsUpdate() {
    return (millis() - lastUpdate) > WEATHER_UPDATE_INTERVAL;
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
