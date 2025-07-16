#include <WiFi.h> #include <WebServer.h> #include <DHT.h> #include <ArduinoJson.h> #include <HTTPClient.h> #include <UniversalTelegramBot.h> #include <WiFiClientSecure.h>

// Pin definitions #define DHTPIN 4 #define DHTTYPE DHT22 #define WATERLEVEL_PIN 34 #define RAINFALL_PIN 32 #define SEISMIC_PIN 35

// Setup DHT sensor DHT dht(DHTPIN, DHTTYPE);

// WiFi credentials const char* ssid = "yourSSID"; const char* password = "yourPASSWORD";

// Telegram credentials const char* botToken = "yourBOTToken"; const char* chatID = "yourChatID";

// Weather API credentials const char* city = "yourCity"; const char* apiKey = "yourAPIKey"; String weatherCondition = "";

// Create Telegram bot instance WiFiClientSecure client; UniversalTelegramBot bot(botToken, client);

// Web server instance WebServer server(80);

// Notification control unsigned long lastNotificationTime = 0; const unsigned long notificationInterval = 60000;

// Threshold values (calculated from dataset) const float TEMP_THRESHOLD = 30.0; const float HUMIDITY_THRESHOLD = 80.0; const float WATERLEVEL_THRESHOLD = 500.0; const int RAINFALL_THRESHOLD = 1; const float SEISMIC_THRESHOLD = 1000.0;

// Function to predict disaster based on thresholds String predictDisaster(float temp, float humidity, float waterLevel, int rainfall, float seismic) { if (temp > TEMP_THRESHOLD && humidity > HUMIDITY_THRESHOLD && waterLevel > WATERLEVEL_THRESHOLD) { return "Flood"; } else if (seismic > SEISMIC_THRESHOLD) { return "Earthquake"; } else if (weatherCondition == "Rainy" || weatherCondition == "Cloudy") { return "Possible Flood (Based on Weather API)"; } return "No Disaster"; }

// Function to fetch weather data from API void fetchWeatherData() { if (WiFi.status() == WL_CONNECTED) { HTTPClient http; String weatherApiUrl = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&appid=" + String(apiKey) + "&units=metric";

http.begin(weatherApiUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
        String payload = http.getString();
        StaticJsonDocument<1024> doc;
        deserializeJson(doc, payload);

        const char* condition = doc["weather"][0]["main"];
        weatherCondition = String(condition);

        Serial.println("Weather Condition: " + weatherCondition);
    } else {
        Serial.println("Error fetching weather data: " + String(httpResponseCode));
    }
    http.end();
} else {
    Serial.println("WiFi not connected. Cannot fetch weather data.");
}

}

// Function to send Telegram notifications void sendTelegramNotification(String message) { if (millis() - lastNotificationTime > notificationInterval) { if (WiFi.status() == WL_CONNECTED) { bot.sendMessage(chatID, message, ""); lastNotificationTime = millis(); } else { Serial.println("WiFi not connected. Cannot send Telegram message."); } } else { Serial.println("Notification suppressed to avoid spamming."); } }

// Function to handle root web server request void handleRoot() { float humidity = dht.readHumidity(); float temperature = dht.readTemperature(); float waterLevel = analogRead(WATERLEVEL_PIN); int rainfall = digitalRead(RAINFALL_PIN); float seismic = analogRead(SEISMIC_PIN);

String prediction = predictDisaster(temperature, humidity, waterLevel, rainfall, seismic);

String html = "<html><head><title>Disaster Management System</title>";
html += "<style>\nbody { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f9; color: #333; margin: 0; padding: 0; }\n";
html += ".container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.1); }\n";
html += "h1 { color: #0056b3; }\n</style></head><body>";

html += "<div class='container'>";
html += "<h1>Disaster Management System</h1>";
html += "<h2>Prediction Result:</h2>";
html += "<p>Prediction: " + prediction + "</p>";
html += "<h3>Sensor Data:</h3>";
html += "<p>Temperature: " + String(temperature) + " &deg;C</p>";
html += "<p>Humidity: " + String(humidity) + " %</p>";
html += "<p>Water Level: " + String(waterLevel) + "</p>";
html += "<p>Rainfall: " + String(rainfall) + "</p>";
html += "<p>Seismic Activity: " + String(seismic) + "</p>";
html += "<p>Weather Condition: " + weatherCondition + "</p>";
html += "</div></body></html>";

server.send(200, "text/html", html);

}

void setup() { Serial.begin(115200); dht.begin();

// Connect to Wi-Fi
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
}
Serial.println("Connected to Wi-Fi!");

// Fetch initial weather data
fetchWeatherData();

// Start server
client.setInsecure();
server.on("/", handleRoot);
server.begin();
Serial.println("Server started!");

}

void loop() { server.handleClient();

// Periodically fetch weather data
static unsigned long lastWeatherFetch = 0;
if (millis() - lastWeatherFetch > 300000) {  // Fetch every 5 minutes
    fetchWeatherData();
    lastWeatherFetch = millis();
}

}

