#ifndef REMOTE_MODE_H
#define REMOTE_MODE_H

#include "config.h"
#include "display_manager.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <mbedtls/base64.h>

// Display dimensions (GxDEPG0213BN is 250x122)
#define DISPLAY_WIDTH 250
#define DISPLAY_HEIGHT 122
#define BITMAP_SIZE ((DISPLAY_WIDTH * DISPLAY_HEIGHT + 7) / 8) // ~3813 bytes

// Remote mode response structure
struct RemoteModeResponse {
  bool isRemote;
  bool success;
  int refreshSeconds;
};

// Check remote mode status from API
// Returns response with mode info
inline RemoteModeResponse checkRemoteMode(uint8_t *imageBuffer,
                                          size_t *imageSize) {
  RemoteModeResponse response = {false, false, REMOTE_REFRESH_SEC};

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping remote check");
    return response;
  }

  HTTPClient http;
  Serial.println("Checking remote mode...");
  http.begin(REMOTE_API_URL);
  http.setTimeout(10000); // 10 second timeout

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      const char *mode = doc["mode"];
      response.success = true;

      if (mode && strcmp(mode, "remote") == 0) {
        response.isRemote = true;

        // Get refresh rate if provided
        if (doc["refresh_seconds"].is<int>()) {
          response.refreshSeconds = doc["refresh_seconds"];
        }

        // Decode base64 image if present
        if (doc["image"].is<const char*>()) {
          const char *base64Image = doc["image"];
          if (base64Image && strlen(base64Image) > 0) {
            // Decode base64 to binary using mbedtls
            size_t base64Len = strlen(base64Image);
            size_t decodedLen = 0;

            // First call to get required size
            mbedtls_base64_decode(NULL, 0, &decodedLen,
                                  (const unsigned char *)base64Image, base64Len);

            if (decodedLen <= BITMAP_SIZE + 100) { // Allow some margin
              int ret = mbedtls_base64_decode(
                  imageBuffer, BITMAP_SIZE + 100, imageSize,
                  (const unsigned char *)base64Image, base64Len);

              if (ret == 0) {
                Serial.printf("Remote image decoded: %d bytes\n", *imageSize);
              } else {
                Serial.printf("Base64 decode failed: %d\n", ret);
                response.isRemote = false;
              }
            } else {
              Serial.printf("Image too large: %d bytes (max %d)\n", decodedLen,
                            BITMAP_SIZE);
              response.isRemote = false;
            }
          }
        }

        Serial.printf("Remote mode active, refresh: %ds\n",
                      response.refreshSeconds);
      } else {
        Serial.println("Normal mode");
      }
    } else {
      Serial.println("JSON parsing failed");
    }
  } else {
    Serial.printf("HTTP error: %d\n", httpCode);
  }

  http.end();
  return response;
}

// Draw remote image on display
inline void drawRemoteImage(DisplayManager &display, const uint8_t *imageBuffer,
                            size_t imageSize) {
  if (imageSize == 0) {
    Serial.println("No remote image to draw");
    return;
  }

  display.clear();
  display.drawBitmap(0, 0, imageBuffer, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  display.update();

  Serial.println("Remote image drawn");
}

#endif // REMOTE_MODE_H
