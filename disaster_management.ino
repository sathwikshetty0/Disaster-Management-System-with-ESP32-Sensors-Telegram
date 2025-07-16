#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// ========================= Pin Definitions =========================
#define DHTPIN 4
#define DHTTYPE DHT11
#define WATERLEVEL_PIN 32
#define RAINFALL_PIN 5
#define SEISMIC_PIN 35

// ========================= Sensor Setup ============================
DHT dht(DHTPIN, DHTTYPE);

// ========================= Wi-Fi Credentials =======================
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// ========================= Telegram Bot ============================
const char* botToken = "YOUR_BOT_TOKEN";
const char* chatID = "YOUR_CHAT_ID";

// ========================= Weather API =============================
const char* city = "YOUR_CITY";
const char* apiKey = "YOUR_API_KEY";

// ========================= Weather Variables =======================
String weatherCondition = "";
float temperatureAPI = 0.0;
float humidityAPI = 0.0;
float dewPointAPI = 0.0;
float windSpeedAPI = 0.0;
int cloudinessAPI = 0;
int visibilityAPI = 0;
int rainForecastInHours = 0;

// ========================= Server & Bot ============================
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);
WebServer server(80);

// ========================= Timing Control ==========================
unsigned long lastNotificationTime = 0;
const unsigned long notificationInterval = 30000;  // 30 seconds

// ========================= Thresholds ==============================
const float TEMP_THRESHOLD = 30.0;
const float HUMIDITY_THRESHOLD = 70.0;
const float WATERLEVEL_THRESHOLD = 500.0;
const int RAINFALL_THRESHOLD = 1;
const float SEISMIC_THRESHOLD = 1000.0;

// ========================= Prediction Functions ====================

String predictFlood(float temp, float humidity, float waterLevel, int rainfall) {
    if (waterLevel > WATERLEVEL_THRESHOLD && humidity > HUMIDITY_THRESHOLD && temp < TEMP_THRESHOLD && rainfall == RAINFALL_THRESHOLD) {
        return "Flood: Yes";
    }
    return "Flood: No";
}

String predictEarthquake(float seismic) {
    return (seismic > SEISMIC_THRESHOLD) ? "Earthquake: Yes" : "Earthquake: No";
}

String predictFloodTime(int forecastRainInHours) {
    return (forecastRainInHours > 0) ? String(forecastRainInHours) + " hours" : "Not predicted";
}

// ========================= Weather Data Fetch ======================

void fetchWeatherData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String weatherApiUrl = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&appid=" + String(apiKey) + "&units=metric";
        
        http.begin(weatherApiUrl);
        int httpResponseCode = http.GET();

        if (httpResponseCode == 200) {
            String payload = http.getString();
            StaticJsonDocument<1024> doc;
            deserializeJson(doc, payload);

            weatherCondition = doc["weather"][0]["main"].as<String>();
            temperatureAPI = doc["main"]["temp"];
            humidityAPI = doc["main"]["humidity"];
            dewPointAPI = doc["main"]["dew_point"] | 0.0;  // Optional field
            windSpeedAPI = doc["wind"]["speed"];
            cloudinessAPI = doc["clouds"]["all"];
            visibilityAPI = doc["visibility"];
            rainForecastInHours = doc["rain"]["1h"] ? 1 : 0;

            Serial.println("[Weather API] Data fetched successfully");
        } else {
            Serial.println("[Weather API] Error code: " + String(httpResponseCode));
        }
        http.end();
    } else {
        Serial.println("[WiFi] Not connected. Weather fetch failed.");
    }
}

// ========================= Telegram Notification ===================

void sendTelegramNotification(String message) {
    if (millis() - lastNotificationTime > notificationInterval) {
        if (WiFi.status() == WL_CONNECTED) {
            bot.sendMessage(chatID, message, "");
            lastNotificationTime = millis();
        } else {
            Serial.println("[Telegram] WiFi not connected. Cannot send message.");
        }
    } else {
        Serial.println("[Telegram] Notification suppressed (interval limit).");
    }
}

// ========================= Web Dashboard Handler ===================

void handleRoot() {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    float waterLevel = analogRead(WATERLEVEL_PIN);
    int rainfall = digitalRead(RAINFALL_PIN);
    float seismic = analogRead(SEISMIC_PIN);

    String floodPrediction = predictFlood(temperature, humidity, waterLevel, rainfall);
    String earthquakePrediction = predictEarthquake(seismic);
    String floodTime = predictFloodTime(rainForecastInHours);

    String html = "<html><head><title>Disaster Management System</title><style>";
    html += "body{font-family:Arial;text-align:center;background:#f4f4f9;color:#333;}";
    html += ".container{max-width:600px;margin:auto;background:white;padding:20px;border-radius:10px;box-shadow:0 4px 8px rgba(0,0,0,0.1);}";
    html += "h1{color:#0056b3;}</style></head><body><div class='container'>";
    html += "<h1>Disaster Management System</h1><h2>Results:</h2>";
    html += "<p>" + floodPrediction + "</p>";
    html += "<p>Flood in: " + floodTime + "</p>";
    html += "<p>" + earthquakePrediction + "</p>";
    html += "<p>Weather: " + weatherCondition + "</p>";
    html += "<h3>Local Sensors:</h3>";
    html += "<p>Temperature: " + String(temperature) + " °C</p>";
    html += "<p>Humidity: " + String(humidity) + " %</p>";
    html += "<p>Water Level: " + String(waterLevel) + "</p>";
    html += "<p>Rainfall: " + String(rainfall) + "</p>";
    html += "<p>Seismic: " + String(seismic) + "</p>";
    html += "<h3>API (City Data):</h3>";
    html += "<p>Temperature: " + String(temperatureAPI) + " °C</p>";
    html += "<p>Humidity: " + String(humidityAPI) + " %</p>";
    html += "<p>Dew Point: " + String(dewPointAPI) + " °C</p>";
    html += "<p>Wind Speed: " + String(windSpeedAPI) + " m/s</p>";
    html += "<p>Cloudiness: " + String(cloudinessAPI) + "%</p>";
    html += "<p>Visibility: " + String(visibilityAPI) + " meters</p>";
    html += "<p>Rain Forecast: " + String(rainForecastInHours) + " hours</p>";
    html += "<button onclick=\"fetch('/message')\">Send Alert</button>";
    html += "</div></body></html>";

    server.send(200, "text/html", html);
}

// ========================= Manual Alert via Web ====================

void sendTelegramMessage() {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    float waterLevel = analogRead(WATERLEVEL_PIN);
    int rainfall = digitalRead(RAINFALL_PIN);
    float seismic = analogRead(SEISMIC_PIN);

    String message = "🛑 Disaster Prediction Report:\n";
    message += predictFlood(temperature, humidity, waterLevel, rainfall) + "\n";
    message += "Flood in: " + predictFloodTime(rainForecastInHours) + "\n";
    message += predictEarthquake(seismic) + "\n";
    message += "\n📡 Weather: " + weatherCondition + "\n";
    message += "🌡️ Temp: " + String(temperature) + " °C\n";
    message += "💧 Humidity: " + String(humidity) + " %\n";
    message += "🌊 Water Level: " + String(waterLevel) + "\n";
    message += "☔ Rainfall: " + String(rainfall) + "\n";
    message += "📉 Seismic: " + String(seismic) + "\n";

    sendTelegramNotification(message);
    server.send(200, "text/html", "<h1>Alert Sent</h1>");
}

// ========================= Setup ================================

void setup() {
    Serial.begin(115200);
    dht.begin();
    pinMode(RAINFALL_PIN, INPUT);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");

    client.setInsecure(); // Skip SSL verification for Telegram
    server.on("/", handleRoot);
    server.on("/message", sendTelegramMessage);
    server.begin();
}

// ========================= Loop ================================

void loop() {
    server.handleClient();

    static unsigned long lastFetch = 0;
    if (millis() - lastFetch > 60000) { // Fetch every 60s
        fetchWeatherData();
        sendTelegramMessage();
        lastFetch = millis();
    }
}
