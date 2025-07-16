

# 🌐 Disaster Management & Prediction System using ESP32

A real-time embedded system designed to **monitor environmental parameters** and **predict disasters** like floods or earthquakes using live sensor data, weather API integration, and automated Telegram alerts. Built with **ESP32**, this project also features a **local web dashboard** for real-time data visualization.

---

## 📸 Project Demo

> _Add a screenshot or YouTube link here for better visualization_  
> Example:  
> [![Demo Video](https://img.youtube.com/vi/YOUR_VIDEO_ID/0.jpg)](https://youtu.be/YOUR_VIDEO_ID)

---

## 📦 Table of Contents

- [🔧 Hardware Components](#-hardware-components)
- [📍 Circuit Diagram & Connections](#-circuit-diagram--connections)
- [🧰 Software Requirements](#-software-requirements)
- [🚀 Features](#-features)
- [🧠 How It Works](#-how-it-works)
- [💡 Prediction Logic](#-prediction-logic)
- [📡 Telegram & API Integration](#-telegram--api-integration)
- [🌐 Web Dashboard](#-web-dashboard)
- [⚙️ Configuration](#️-configuration)
- [🛠️ Troubleshooting](#️-troubleshooting)
- [👤 Author](#-author)
- [📜 License](#-license)

---

## 🔧 Hardware Components

| Component              | Description                               |
|------------------------|-------------------------------------------|
| ESP32 Dev Board        | Main microcontroller with Wi-Fi           |
| DHT22                  | Digital Temperature & Humidity Sensor     |
| Rainfall Sensor        | Digital raindrop detection module         |
| Water Level Sensor     | Analog level sensing probe (float-based)  |
| Seismic Sensor         | Vibration sensor (analog/LM393 module)    |
| Breadboard + Jumper Wires | For circuit assembly                    |
| USB Cable              | For flashing and power                    |

---

## 📍 Circuit Diagram & Connections

| Sensor / Module      | ESP32 Pin      | Notes                       |
|----------------------|----------------|-----------------------------|
| DHT22                | GPIO 4         | Use 10K pull-up resistor    |
| Rainfall Sensor      | GPIO 32        | Digital output              |
| Water Level Sensor   | GPIO 34 (ADC1) | Analog input                |
| Seismic Sensor       | GPIO 35 (ADC1) | Analog input or digital     |
| GND                  | GND            | Common ground               |
| VCC                  | 3.3V / 5V      | Depending on module         |

**⚠️ NOTE:**  
- Use **resistors** where required (e.g., pull-up on DHT22).
- Power analog sensors using **3.3V** for ESP32 safety.
- Some analog pins (like GPIO 36/39) are input-only.

---

## 🧰 Software Requirements

| Tool                  | Purpose                                  |
|-----------------------|------------------------------------------|
| Arduino IDE           | Programming ESP32                        |
| ESP32 Board Manager   | Board package for Arduino IDE            |
| Telegram Bot API      | Sends alerts via Telegram                |
| OpenWeatherMap API    | Real-time weather integration            |

### Required Arduino Libraries

Install these via Library Manager:

- `WiFi.h`
- `WebServer.h`
- `DHT sensor library`
- `ArduinoJson`
- `HTTPClient`
- `UniversalTelegramBot`
- `WiFiClientSecure`

---

## 🚀 Features

- 🌡️ **Reads Temperature & Humidity** using DHT22  
- 🌧️ **Detects Rainfall, Water Level, and Seismic Activity**  
- ☁️ **Fetches live weather condition** using OpenWeatherMap API  
- ⚠️ **Predicts potential disasters** based on thresholds  
- 📲 **Sends real-time alerts via Telegram bot**  
- 🌐 **Serves live dashboard** via local web server

---

## 🧠 How It Works

1. ESP32 connects to your Wi-Fi.
2. Reads all sensor values every second.
3. Every 5 minutes:
   - Fetches weather info via OpenWeatherMap API.
   - Predicts disaster if thresholds exceeded.
   - Sends alert via Telegram.
4. Hosts an HTML dashboard showing:
   - Sensor values
   - Weather status
   - Disaster prediction

---

## 💡 Prediction Logic

### Disaster Scenarios:

| Condition                                                              | Prediction        |
|------------------------------------------------------------------------|-------------------|
| Temperature > 30°C + Humidity > 80% + Water Level > 500               | 🚨 Flood          |
| Seismic activity > 1000 (analog value)                                 | 🚨 Earthquake     |
| Weather API says "Rainy" or "Cloudy"                                   | ⚠️ Possible Flood |
| None of the above                                                      | ✅ No Disaster     |

The prediction is displayed both in:
- HTML dashboard
- Telegram message (only once per minute to avoid spam)

---

## 📡 Telegram & API Integration

### ✅ Telegram Bot Setup

1. Message [@BotFather](https://t.me/BotFather)
2. Create new bot: `/newbot`
3. Get token: `123456789:ABC-XYZ`
4. Paste it into `const char* botToken`

Find your `chatID` using this link:  
`https://api.telegram.org/bot<YourBOTToken>/getUpdates` after messaging your bot.

### ☁️ OpenWeatherMap Setup

1. Go to https://openweathermap.org/api
2. Sign up and get free API key
3. Replace `yourCity` and `yourAPIKey` in the code

---

## 🌐 Web Dashboard

📟 Displays all sensor values and prediction visually.  
🧑‍💻 Access it via your ESP32's IP address (shown in Serial Monitor).

HTML interface includes:

- Styled title & container
- Temperature & Humidity
- Water Level & Rainfall
- Seismic reading
- Weather condition
- Final Prediction status

---

## ⚙️ Configuration

Before uploading the code, edit these lines:

```cpp
const char* ssid = "yourSSID";
const char* password = "yourPASSWORD";

const char* botToken = "yourBOTToken";
const char* chatID = "yourChatID";

const char* city = "yourCity";
const char* apiKey = "yourAPIKey";
````

---

## 🛠️ Troubleshooting

| Problem                    | Solution                                              |
| -------------------------- | ----------------------------------------------------- |
| Webpage not loading        | Check IP in Serial Monitor, ensure connected to Wi-Fi |
| Telegram not working       | Double-check token, chat ID, and API permissions      |
| Weather not fetching       | Verify OpenWeatherMap key & city name                 |
| Sensor values are `nan`    | Check wiring and sensor power                         |
| Flood warning always shown | Adjust thresholds based on local conditions           |

---

## 👤 Author

**Sathwik Shetty N**

* [GitHub](https://github.com/sathwikshetty0)
* [LinkedIn](https://www.linkedin.com/in/sathwikshettyn)
* Email: [sathwikshettyn0@gmail.com](mailto:sathwikshettyn0@gmail.com)

---

## 📜 License

This project is licensed under the **MIT License** – use freely, just give credit 😊

```

---

✅ Let me know if you'd like a Fritzing circuit diagram, code documentation (`.ino` structure), or a one-page PDF for project presentation!
```
