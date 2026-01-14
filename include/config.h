#ifndef CONFIG_H
#define CONFIG_H

// ==================== WiFi Configuration ====================
// #define WIFI_SSID "MEO-7678E0"
// #define WIFI_PASSWORD "8413d8d54c"
#define WIFI_SSID "Ainda sem internet"
#define WIFI_PASSWORD "taseminternet"

// ==================== Time Configuration ====================
#define NTP_SERVER "0.pt.pool.ntp.org"
#define GMT_OFFSET_SEC (0 * 3600) // GMT+0
#define DAYLIGHT_OFFSET_SEC 0

// ==================== Weather API Configuration ====================
// Get your free API key at: https://www.weatherapi.com/
#define WEATHER_API_KEY "beddfb8faa514e29948102955252612"
#define WEATHER_LOCATION "Porto,PT" // City name, coordinates, or IP address

// ==================== E-Ink Display Pins ====================
#define SPI_MOSI 23
#define SPI_MISO -1
#define SPI_CLK 18

#define ELINK_SS 5
#define ELINK_BUSY 4
#define ELINK_RESET 16
#define ELINK_DC 17

// ==================== SD Card Pins ====================
#define SDCARD_SS 13
#define SDCARD_CLK 14
#define SDCARD_MOSI 15
#define SDCARD_MISO 2

// ==================== Button Pin ====================
#define BUTTON_PIN 39

// ==================== Battery ADC Pin ====================
#define BATTERY_PIN 35    // ADC pin for battery voltage reading
#define BATTERY_MAX_V 4.2 // Max voltage (fully charged)
#define BATTERY_MIN_V 3.0 // Min voltage (empty)

// ==================== Display Settings ====================
#define DISPLAY_ROTATION 1 // 0-3 for different orientations

// ==================== Deep Sleep Configuration ====================
#define SLEEP_DURATION_SEC 60  // Wake every 60 seconds to update display
#define WEATHER_UPDATE_MIN 30  // Update weather every 30 minutes
#define FULL_REFRESH_CYCLES 60 // Full display refresh every 60 wakes (~1 hour)

// ==================== Remote Mode Configuration ====================
#define REMOTE_API_URL "http://192.168.1.173:3000/api/display/status"
#define REMOTE_CHECK_CYCLES 5 // Check every 5 wakes (5 min) in normal mode
#define REMOTE_REFRESH_SEC 60 // Refresh every 60 sec in remote mode

#endif // CONFIG_H
