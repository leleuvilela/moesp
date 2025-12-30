# MoESP - Morning ESP32 E-Ink Dashboard

A cute desk display for my girlfriend, made with ESP32 and an E-Ink screen.

## What does it do?

This project turns an ESP32 with an E-Ink display into a desk dashboard that shows:

- **Time and date**
- **Current weather** with temperature, condition and icon
- **Day forecast** (min/max temperature and rain chance)
- **Day suggestions** - alerts if you need an umbrella, jacket, or if it's a beautiful day
- **Good morning messages** - messages that alternate during the morning
- **Status** - WiFi and battery in the top right corner

## Hardware

- ESP32 (tested with LILYGO T5 2.13")
- E-Ink Display 2.13" (GxDEPG0213BN)
- LiPo battery (optional)

### Pinout

| Function    | GPIO |
| ----------- | ---- |
| SPI MOSI    | 23   |
| SPI CLK     | 18   |
| E-Ink CS    | 5    |
| E-Ink DC    | 17   |
| E-Ink RST   | 16   |
| E-Ink BUSY  | 4    |
| Battery ADC | 35   |
| Button      | 39   |

## Configuration

Edit the file `include/config.h`:

```cpp
// WiFi
#define WIFI_SSID "YourNetwork"
#define WIFI_PASSWORD "YourPassword"

// Timezone (Portugal = GMT+0)
#define GMT_OFFSET_SEC (0 * 3600)

// Location for weather
#define WEATHER_LOCATION "Porto,PT"

// WeatherAPI.com API Key (free)
#define WEATHER_API_KEY "your_api_key"
```

### Get Weather API Key

1. Go to [weatherapi.com](https://www.weatherapi.com/)
2. Create a free account
3. Copy your API key to `config.h`

## Features

### Good Morning Messages

The display alternates between messages and the day suggestion:

Customize in `include/messages.h`.

### Smart Suggestions

The display analyzes the weather and suggests:

| Condition   | Message                                                                        |
| ----------- | ------------------------------------------------------------------------------ |
| Rain ≥70%   | "Leve guarda-chuva!" (Bring an umbrella!)                                      |
| Rain 40-69% | "Talvez chova, leve casaco" (It might rain, bring a jacket)                    |
| Temp ≤10°C  | "Esta frio, leve um casaco!" (It's cold, bring a coat!)                        |
| Temp ≤15°C  | "Leve um casaquinho" (Bring a light jacket)                                    |
| Max ≥30°C   | "Vai estar quente, beba agua!" (It'll be hot, drink water!)                    |
| Nice day    | "O dia vai estar lindo, igual voce" (The day will be beautiful, just like you) |

### Weather Icons

Icons for: sun, moon, cloudy, rain, partly cloudy, thunderstorm, snow, fog.

### Efficient Updates

- **Partial update** every minute (time and message)
- **Full refresh** only when necessary:
  - New day
  - Weather update (every 30 min)
  - Anti-ghosting (every hour)

## Build

### PlatformIO (recommended)

```bash
# Build
pio run

# Upload
pio run --target upload

# Serial monitor
pio device monitor
```

### Arduino IDE

1. Install the libraries:
   - GxEPD
   - ArduinoJson
2. Select ESP32 board
3. Compile and upload

## Project Structure

```
moesp/
├── src/
│   └── main.cpp           # Main code
├── include/
│   ├── config.h           # Settings (WiFi, API, pins)
│   ├── display_manager.h  # E-Ink display control
│   ├── weather.h          # Weather API client
│   ├── messages.h         # Good morning messages
│   └── icons.h            # Bitmap icons
├── platformio.ini         # PlatformIO configuration
└── README.md
```

## Display Layout

```
┌─────────────────────────────────────────┐
│ [icon]  18°C      12°/22°   [wifi][bat] │
│ [    ]  Sol       Rain: 10%             │
│                                         │
│  "O dia vai estar lindo, igual voce"    │
├─────────────────────────────────────────┤
│                 14:35                   │
│            Ter, 30 Dez 2024             │
└─────────────────────────────────────────┘
```

## License

MIT - Use it however you want!

---

Made with love to make her day more beautiful.
