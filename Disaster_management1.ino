#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// ================== Pin Definitions ==================
#define DHTPIN 4
#define DHTTYPE DHT22
#define WATERLEVEL_PIN 34
#define RAINFALL_PIN 32
#define SEISMIC_PIN 35

// ================== Sensor Setup ==================
DHT dht(DHTPIN, DHTTYPE);

// ================== WiFi Credentials ==================
const char* ssid = "yourSSID";
const char* password = "yourPASSWORD";

// ================== Telegram Credentials ==================
const char* botToken = "yourBOTToken";
const char* chatID = "yourChatID";

// ================== OpenWeather API ==================
const char* city = "yourCity";
const char* apiKey = "yourAPIKey";
String weatherCondition = "";

// ================== Instances ==================
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);
WebServer server(80);

// ================== Notification Timer ==================
unsigned long lastNotificationTime = 0;
const unsigned long notificationInterval = 60000; // 1 minute

// ================== Thresholds ==================
const float TEMP_THRESHOLD = 30.0;
const float HUMIDITY_THRESHOLD = 80.0;
const float WATERLEVEL_THRESHOLD = 500.0;
const int RAINFALL_THRESHOLD = 1;
const float SEISMIC_THRESHOLD = 1000.0;

// ========== Predict Disaster Based on Sensor & Weather ==========
String predictDisaster(float temp, float humidity, float waterLevel, int rainfall, float seismic) {
  if (temp > TEMP_THRESHOLD && humidity > HUMIDITY_THRESHOLD && waterLevel > WATERLEVEL_THRESHOLD) {
    return "Flood";
  } else if (seismic > SEISMIC_THRESHOLD) {
    return "Earthquake";
  } else if (weatherCondition == "Rain" || weatherCondition == "Clouds") {
    return "Possible Flood (Weather API)";
  }
  return "No Disaster";
}

// ========== Fetch Weather Data ==========
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

      const char* condition = doc["weather"][0]["main"];
      weatherCondition = String(condition);
      Serial.println("Weather: " + weatherCondition);
    } else {
      Serial.println("Failed to fetch weather: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi not connected. Skipping weather fetch.");
  }
}

// ========== Send Telegram Alert ==========
void sendTelegramNotification(String message) {
  if (millis() - lastNotificationTime > notificationInterval) {
    if (WiFi.status() == WL_CONNECTED) {
      bot.sendMessage(chatID, message, "");
      lastNotificationTime = millis();
    } else {
      Serial.println("WiFi not connected. Cannot send Telegram message.");
    }
  } else {
    Serial.println("Notification throttled.");
  }
}

// ========== Web Dashboard ==========
void handleRoot() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float waterLevel = analogRead(WATERLEVEL_PIN);
  int rainfall = digitalRead(RAINFALL_PIN);
  float seismic = analogRead(SEISMIC_PIN);

  if (isnan(temperature) || isnan(humidity)) {
    server.send(200, "text/plain", "Sensor error. Try again.");
    return;
  }

  String prediction = predictDisaster(temperature, humidity, waterLevel, rainfall, seismic);

  // Optional: Send alert via Telegram here
  if (prediction != "No Disaster") {
    sendTelegramNotification("⚠️ ALERT: " + prediction + "\n🌡️ Temp: " + temperature + "°C\n💧 Humidity: " + humidity + "%\n🌊 Water Level: " + waterLevel);
  }

  String html = "<html><head><title>Disaster System</title>";
  html += "<meta http-equiv='refresh' content='15'>";
  html += "<style>body{font-family:Arial;text-align:center;background:#f4f4f9;color:#333;}";
  html += ".container{max-width:600px;margin:auto;background:white;padding:20px;border-radius:10px;box-shadow:0 4px 8px rgba(0,0,0,0.1);}";
  html += "h1{color:#0056b3;}</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>🌍 Disaster Management System</h1>";
  html += "<h2>Prediction: <span style='color:red;'>" + prediction + "</span></h2>";
  html += "<h3>Sensor Data</h3>";
  html += "<p>🌡️ Temperature: " + String(temperature) + " °C</p>";
  html += "<p>💧 Humidity: " + String(humidity) + " %</p>";
  html += "<p>🌊 Water Level: " + String(waterLevel) + "</p>";
  html += "<p>🌧️ Rainfall: " + String(rainfall) + "</p>";
  html += "<p>📈 Seismic Activity: " + String(seismic) + "</p>";
  html += "<p>☁️ Weather: " + weatherCondition + "</p>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  dht.begin();
  client.setInsecure();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");

  fetchWeatherData();

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started");
}

// ========== Loop ==========
void loop() {
  server.handleClient();

  // Refresh weather every 5 mins
  static unsigned long lastWeatherFetch = 0;
  if (millis() - lastWeatherFetch > 300000) {
    fetchWeatherData();
    lastWeatherFetch = millis();
  }

  // Monitor sensors & auto-notify (background monitoring)
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float waterLevel = analogRead(WATERLEVEL_PIN);
  int rainfall = digitalRead(RAINFALL_PIN);
  float seismic = analogRead(SEISMIC_PIN);

  if (!isnan(temperature) && !isnan(humidity)) {
    String prediction = predictDisaster(temperature, humidity, waterLevel, rainfall, seismic);
    if (prediction != "No Disaster") {
      sendTelegramNotification("⚠️ Real-Time Alert: " + prediction);
    }
  }

  delay(10000); // Reduce CPU usage
}
