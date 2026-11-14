#include <WiFi.h>
#include <HTTPClient.h>
#include "soc/gpio_reg.h"

// Wokwi ভার্চুয়াল ওয়াই-ফাই ক্রেন্ডেনশিয়ালস
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// পিন ডিফিনিশন
const int SENSOR_PIN = 4;   // LDR সেন্সর (AO পিন) GPIO 4 এ সংযুক্ত
const int LED_PIN = 2;      // স্ট্যাটাস LED GPIO 2 এ সংযুক্ত

unsigned long lastMillis = 0;
const long interval = 3000; // ৩ সেকেন্ডের বিরতি

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  analogSetAttenuation(ADC_11db);

  // ওয়াই-ফাই কানেক্ট করার চেষ্টা
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    // ওয়াই-ফাই কানেক্ট হওয়ার সময় রেজিস্টার ব্যবহার করে LED ব্লিঙ্ক করানো (সঠিক ফরম্যাট)
    REG_WRITE(GPIO_OUT_W1TS_REG, (1 << LED_PIN)); // LED অন
    delay(200);
    REG_WRITE(GPIO_OUT_W1TC_REG, (1 << LED_PIN)); // LED অফ
  }
  
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // ওয়াই-ফাই কানেক্ট হওয়ার পর LED স্থায়ীভাবে অন রাখা
  REG_WRITE(GPIO_OUT_W1TS_REG, (1 << LED_PIN));
}

void loop() {
  // LDR সেন্সর থেকে ১২-বিট অ্যানালগ মান (০ থেকে ৪০৯৫) পড়া
  int sensorValue = analogRead(SENSOR_PIN);
  
  // সিরিয়াল মনিটরে মান প্রিন্ট করা
  Serial.print("Sensor Value (GPIO 4): ");
  Serial.println(sensorValue);

  // প্রতি ৩ সেকেন্ড পর পর HTTP GET রিকোয়েস্ট পাঠানো
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis >= interval) {
    lastMillis = currentMillis;

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      
      String serverPath = "http://httpbin.org/get?sensor=" + String(sensorValue);
      
      http.begin(serverPath.c_str());
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      
      http.end();
    } else {
      Serial.println("WiFi Disconnected. Reconnecting...");
      WiFi.begin(ssid, password);
    }
  }

  delay(100);
}