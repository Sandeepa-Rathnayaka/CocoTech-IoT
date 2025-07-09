#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // Sync with NTP server every 60 seconds

#define WIFI_SSID "huawey7"
#define WIFI_PASSWORD "sandeepa2000"
#define API_KEY "AIzaSyB6XCUD2CHzMkhvh0efNNxJ0p2hwH-WrMs"
#define DATABASE_URL "https://moisturesens-default-rtdb.firebaseio.com/" 
#define USER_EMAIL "sandeeparathna@gmail.com"
#define USER_PASSWORD "1234567"

// 🔹 Firebase Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// 🔹 Unique Device ID
#define DEVICE_ID "Food_Moisture_001"

// 🔹 Soil Moisture Sensor Pins
#define SENSOR_1_PIN 35

// 🔹 Push Button Pin
#define BUTTON_PIN 4

// 🔹 Soil Moisture Calibration (Adjust based on your sensor)
#define DRY_FOOD 3000  // Analog value in dry food
#define WET_FOOD 1000  // Analog value in wet food

// 🔹 OLED Display Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    timeClient.begin();

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println("\nConnected with IP: " + WiFi.localIP().toString());

    // 🔹 Initialize OLED Display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("SSD1306 allocation failed");
        for (;;);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 20);
    display.println("Moisture Analyzer");
    display.setCursor(10, 35);
    display.println("Initializing...");
    display.display();
    delay(2000);

    // 🔹 Firebase Initialization
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL;
    config.token_status_callback = tokenStatusCallback;
    
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096, 1024);
    fbdo.setResponseSize(2048);
    Firebase.begin(&config, &auth);
    Firebase.setDoubleDigits(5);
    config.timeout.serverResponse = 10 * 1000;

    Serial.println("Firebase Initialized");

    // 🔹 Initialize Button
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

// Function to map analog values to 0-100% range
int getSoilMoisturePercentage(int rawValue)
{
    int percentage = map(rawValue, DRY_FOOD, WET_FOOD, 0, 100);
    return constrain(percentage, 0, 100); // Ensure values stay between 0-100%
}

void loop()
{
    // Check if the button is pressed (LOW because of INPUT_PULLUP)
    if (digitalRead(BUTTON_PIN) == LOW)
    {
        Serial.println("Button Pressed! Sending Data...");
        sendSoilMoistureData();

        // Display confirmation message
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(10, 20);
        display.println("Data Sent !");
        display.display();
        delay(2000); // Show message for 2 seconds
    }

    displaySoilMoisture();
}

void sendMoistureData()
{
    timeClient.update();

    int moisture1 = getSoilMoisturePercentage(analogRead(SENSOR_1_PIN));

    unsigned long timestamp = timeClient.getEpochTime();  // Get actual Unix time

    Serial.println("Sending data to Firebase...");

    // Create a unique path with timestamp
    String path = "/soil_analyzer/" + String(DEVICE_ID) + "/history/" + String(timestamp);

    // Store data under the timestamped path
    Firebase.RTDB.setInt(&fbdo, path + "/sensor_1", moisture1);
    Firebase.RTDB.setInt(&fbdo, path + "/timestamp", timestamp);

    Serial.println("Data Stored!");
}

void displaySoilMoisture()
{
    int moisture1 = getSoilMoisturePercentage(analogRead(SENSOR_1_PIN));

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 5);
    display.println("Food Moisture (%)");

    display.setCursor(10, 20);
    display.print("Moisture : ");
    display.print(moisture1);
    display.print("%");

    display.display();
}
