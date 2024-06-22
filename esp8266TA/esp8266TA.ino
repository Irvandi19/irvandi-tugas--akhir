#include <Arduino.h>
#include <Servo.h> // Library untuk servo
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Pin definitions for relays
#define RELAY_LAMPU_PIN 5 // D1
#define RELAY_AC_PIN 4    // D2
#define RELAY_PINTU_PIN 0 // D3
#define RELAY_SUHU_PIN 2  // D4
#define RELAY_MANUAL_LAMPU_PIN 13 // D7
#define RELAY_ARUS_PIN 12 // D6


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

Servo myservo;

// Inisialisasi NTP Client untuk sinkronisasi waktu
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 7 * 3600; // Offset WIB +7 jam
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 60000);

void setup() {
    // Initialize relays
    pinMode(RELAY_LAMPU_PIN, OUTPUT);
    pinMode(RELAY_AC_PIN, OUTPUT);
    pinMode(RELAY_PINTU_PIN, OUTPUT);
    pinMode(RELAY_SUHU_PIN, OUTPUT);
    pinMode(RELAY_MANUAL_LAMPU_PIN, OUTPUT);
    pinMode(RELAY_ARUS_PIN, OUTPUT);

    // Initialize servo
    myservo.attach(14, 500, 2400); // Assuming the servo is connected to pin 14
    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    // Sign up
    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("ok");
        signupOK = true;
    } else {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    // Assign the callback function for the long running token generation task
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // Inisialisasi NTP Client
    timeClient.begin();
}

void loop() {
    if (Firebase.ready() && signupOK) {
        // Perbarui waktu
        timeClient.update();
        int currentHour = timeClient.getHours();
        int currentMinute = timeClient.getMinutes();
        int currentSecond = timeClient.getSeconds();

        // Cetak waktu saat ini (jam, menit, detik) dalam WIB
        Serial.print("Waktu WIB: ");
        Serial.print(currentHour);
        Serial.print(":");
        Serial.print(currentMinute);
        Serial.print(":");
        Serial.println(currentSecond);

        // Baca jadwal dari Firebase
        int startHour = 0, startMinute = 0, endHour = 0, endMinute = 0;
        bool scheduleAvailable = false;

        if (Firebase.RTDB.getInt(&fbdo, "currentSchedule/startHour")) {
            startHour = fbdo.intData();
            if (Firebase.RTDB.getInt(&fbdo, "currentSchedule/startMinute")) {
                startMinute = fbdo.intData();
                if (Firebase.RTDB.getInt(&fbdo, "currentSchedule/endHour")) {
                    endHour = fbdo.intData();
                    if (Firebase.RTDB.getInt(&fbdo, "currentSchedule/endMinute")) {
                        endMinute = fbdo.intData();
                        scheduleAvailable = true;
                    }
                }
            }
        }

        if (scheduleAvailable) {
            // Cek apakah saat ini berada dalam rentang waktu jadwal
            if (isWithinSchedule(currentHour, currentMinute, startHour, startMinute, endHour, endMinute)) {
                digitalWrite(RELAY_LAMPU_PIN, LOW); // Nyalakan relay lampu
                Firebase.RTDB.setString(&fbdo, "relays/saklarLampuOTO", "on");
            } else {
                digitalWrite(RELAY_LAMPU_PIN, HIGH); // Matikan relay lampu
                Firebase.RTDB.setString(&fbdo, "relays/saklarLampuOTO", "off");
            }
        }

        // Baca status flame dari RTDB
        if (Firebase.RTDB.getString(&fbdo, "sensors/api")) {
            String flameStatus = fbdo.stringData();
            Serial.print("Received Flame Status from Firebase: ");
            Serial.println(flameStatus);

            // Periksa status flame dan kontrol servo
            if (flameStatus == "Terdeteksi") {
                // Jika api terdeteksi, gerakkan servo
                myservo.write(45); // Ganti sudut sesuai kebutuhan
            } else if (flameStatus == "Aman") {
                // Jika aman, kembalikan servo ke posisi awal
                myservo.write(180); // Ganti sudut sesuai kebutuhan
            }
        } else {
            Serial.println("Failed to read flame status from Firebase!");
            Serial.println("Reason: " + fbdo.errorReason());
        }

        // Kontrol relay saklar lampu manual
        if (Firebase.RTDB.getString(&fbdo, "relays/saklarLampu")) {
            String saklarLampu = fbdo.stringData();
            Serial.print("Received Lampu Status from Firebase: ");
            Serial.println(saklarLampu);

            if (saklarLampu == "on") {
                digitalWrite(RELAY_MANUAL_LAMPU_PIN, LOW);
            } else if (saklarLampu == "off") {
                digitalWrite(RELAY_MANUAL_LAMPU_PIN, HIGH);
            }
        } else {
            Serial.println("Failed to read saklarLampu status from Firebase!");
            Serial.println("Reason: " + fbdo.errorReason());
        }

        // Kontrol relay saklar AC
        if (Firebase.RTDB.getString(&fbdo, "relays/saklarAC")) {
            String saklarAC = fbdo.stringData();
            Serial.print("Received AC Status from Firebase: ");
            Serial.println(saklarAC);

            if (saklarAC == "on") {
                digitalWrite(RELAY_AC_PIN, LOW);
            } else if (saklarAC == "off") {
                digitalWrite(RELAY_AC_PIN, HIGH);
            }
        } else {
            Serial.println("Failed to read saklarAC status from Firebase!");
            Serial.println("Reason: " + fbdo.errorReason());
        }

        // Kontrol relay saklar pintu
        if (Firebase.RTDB.getString(&fbdo, "relays/saklarPintu")) {
            String saklarPintu = fbdo.stringData();
            Serial.print("Received Pintu Status from Firebase: ");
            Serial.println(saklarPintu);

            if (saklarPintu == "buka") {
                digitalWrite(RELAY_PINTU_PIN, LOW);
            } else if (saklarPintu == "kunci") {
                digitalWrite(RELAY_PINTU_PIN, HIGH);
            }
        } else {
            Serial.println("Failed to read saklarPintu status from Firebase!");
            Serial.println("Reason: " + fbdo.errorReason());
        }

        // Kontrol relay suhu
        if (Firebase.RTDB.getFloat(&fbdo, "sensors/suhu")) {
            float suhu = fbdo.floatData();
            Serial.print("Received Temperature from Firebase: ");
            Serial.println(suhu);

            if (suhu > 35) {
                digitalWrite(RELAY_SUHU_PIN, LOW);
                Firebase.RTDB.setString(&fbdo, "relays/saklarACoto", "on");
            } else {
                digitalWrite(RELAY_SUHU_PIN, HIGH);
                Firebase.RTDB.setString(&fbdo, "relays/saklarACoto", "off");
            }
        } else {
            Serial.println("Failed to read temperature from Firebase!");
            Serial.println("Reason: " + fbdo.errorReason());
        }

        // Kontrol relay mati lampu
        if (Firebase.RTDB.getFloat(&fbdo, "sensors/arus")) {
            float arus = fbdo.floatData();
            Serial.print("Received Temperature from Firebase: ");
            Serial.println(arus);

            if (arus < 5) {
                digitalWrite(RELAY_ARUS_PIN, LOW);
            } else {
                digitalWrite(RELAY_ARUS_PIN, HIGH);
            }
        } else {
            Serial.println("Failed to read temperature from Firebase!");
            Serial.println("Reason: " + fbdo.errorReason());
        }
    }

    delay(500);
}

bool isWithinSchedule(int currentHour, int currentMinute, int startHour, int startMinute, int endHour, int endMinute) {
    int currentTime = currentHour * 60 + currentMinute;
    int startTime = startHour * 60 + startMinute;
    int endTime = endHour * 60 + endMinute;

    if (startTime < endTime) {
        return currentTime >= startTime && currentTime < endTime;
    } else {
        return currentTime >= startTime || currentTime < endTime;
    }
}
