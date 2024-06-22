#include "DHT.h"
#include <Arduino.h>
#include <Wire.h>
#include <INA219_WE.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#define DHTPIN 5       // pin D5 DHT11
#define DHTTYPE DHT11  // Sensor DHT11
#define flamePin 4     // pin D4
#define mqPin 18       // pin 18
#define buzzerPin 13   // Pin 13

#define I2C_ADDRESS 0x40 // I2C address for INA219
INA219_WE ina219 = INA219_WE(I2C_ADDRESS); // Initialize INA219 object

DHT dht(DHTPIN, DHTTYPE);

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "10TPro"
#define WIFI_PASSWORD "password"

#define API_KEY "AIzaSyBpFwwjelFLSBEznMLod7U_mgN24dhqv2A"
#define DATABASE_URL "https://smartserver-70a38-default-rtdb.asia-southeast1.firebasedatabase.app/"


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

void setup()
{
    pinMode(DHTPIN, INPUT);
    pinMode(flamePin, INPUT);
    pinMode(mqPin, INPUT);
    pinMode(buzzerPin, OUTPUT);
    dht.begin();
    Serial.begin(115200);
    Wire.begin(); // Initialize I2C
    if (!ina219.init())
    {
        Serial.println("INA219 not connected!");
    }
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("ok");
        signupOK = true;
    }
    else
    {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    ledcSetup(0, 1000, 8);       // PWM channel 0, frequency 1000 Hz, 8-bit resolution
    ledcAttachPin(buzzerPin, 0); // Attach buzzer pin to PWM channel 0
}

void loop()
{
    delay(1500);

    // Read temperature
    float temperatureC = dht.readTemperature();
    // Check if reading is successful
    if (!isnan(temperatureC))
    {
        processTemperature(temperatureC); // Process temperature data and send to Firebase
    }
    int flameValue = digitalRead(flamePin); // Read flame sensor value
    processFlame(flameValue);               // Process flame sensor data and send to Firebase

    int mqValue = digitalRead(mqPin);
    processMq(mqValue);

    float current_mA = ina219.getCurrent_mA(); // Read current from INA219
    processAmp(current_mA);                    // Process current data and send to Firebase
}

void processAmp(float ampValue)
{
    if (Firebase.ready() && signupOK)
    {
        Firebase.RTDB.setFloat(&fbdo, "sensors/arus", ampValue);
    }
}

void processTemperature(float temperatureC)
{
    String tempStatus;
    if (temperatureC < 20)
    {
        tempStatus = "Dingin";
    }
    else if (temperatureC >= 20.01 && temperatureC <= 35)
    {
        tempStatus = "Normal";
    }
    else if (temperatureC > 35.01 && temperatureC <= 50)
    {
        tempStatus = "Panas";
    }

    if (Firebase.ready() && signupOK)
    {
        // Write temperature data to Firebase RTDB
        Firebase.RTDB.setFloat(&fbdo, "sensors/suhu", temperatureC);
    }
}

void processFlame(int flameValue)
{
    String flameStatus;
    if (flameValue < 1)
    { // Adjust this threshold based on your sensor and environment
        flameStatus = "Terdeteksi";
        // Activate buzzer when flame is detected
        ledcWriteTone(0, 3000); // Turn on buzzer with 3000 Hz frequency
    }
    else
    {
        flameStatus = "Aman";
        ledcWriteTone(0, 0); // Turn off buzzer
    }

    if (Firebase.ready() && signupOK)
    {
        // Write flame status to Firebase RTDB
        Firebase.RTDB.setString(&fbdo, "sensors/api", flameStatus);
    }
}

void processMq(int mqValue)
{
    String statusAsap;
    if (mqValue < 1)
    { // Adjust this threshold based on your sensor and environment
        statusAsap = "Terdeteksi";
        // Activate buzzer when smoke is detected
        ledcWriteTone(0, 3000); // Turn on buzzer with 3000 Hz frequency
    }
    else
    {
        statusAsap = "Aman";
        // Deactivate buzzer when no smoke is detected
        ledcWriteTone(0, 0); // Turn off buzzer
    }

    if (Firebase.ready() && signupOK)
    {
        // Write MQ sensor status to Firebase RTDB
        Firebase.RTDB.setString(&fbdo, "sensors/asap", statusAsap);
    }
}
