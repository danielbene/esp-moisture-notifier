#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "local.h"
/* CONTENTS OF LOCAL.H
char ssid[] = "";       // your network SSID (name)
char password[] = "";   // your network key
char key[] = "";        // from this page https://ifttt.com/maker_webhooks/settings - get the token from the URL
char event[] = "";      // name of the event to trigger
*/

#define SEC 1000000.0
#define SLEEP_DURATION 1 * 60 * 60 * SEC
#define DRYNESS_ALARM_VALUE 100
#define DEBUG false

WiFiClient client;
HTTPClient http;

boolean triggerEvent(int value) {
  boolean isSuccess = false;
  String url = "http://maker.ifttt.com/trigger/";
  url += event;
  url += "/with/key/";
  url += key;
  url += "?" + value;

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == 200) {
        isSuccess = true;
    } else {
        isSuccess = false;
    }
  }

  http.end();
  return isSuccess;
}

void wifiSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    if (DEBUG) Serial.print(".");
    delay(500);
  }
}

void setup() {
  int currentValue = analogRead(A0);

  if (DEBUG) {
    Serial.begin(115200);
    Serial.println();
    Serial.println(currentValue);
  }

  if (currentValue > DRYNESS_ALARM_VALUE) {
    wifiSetup();

    if(triggerEvent(currentValue)){
      if (DEBUG) Serial.println("Successfully sent!");
    } else {
      if (DEBUG) Serial.println("Failed!");
    }
  }

  if (DEBUG) Serial.println("Going to sleep...");
  delay(100);
  ESP.deepSleep(SLEEP_DURATION);
}

void loop() {
}
