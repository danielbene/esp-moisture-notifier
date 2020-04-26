#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "local.h"
/* CONTENTS OF LOCAL.H
char ssid[] = "";               // your network SSID (name)
char password[] = "";           // your network key
char key[] = "";                // from this page https://ifttt.com/maker_webhooks/settings - get the token from the specified URL on the page
char log_event[] = "";          // name of the logging event to trigger
char notify_event[] = "";       // name of the notification event to trigger
*/

#define SLEEP_HOURS 8
#define DRYNESS_ALARM_VALUE 500 // the higher the value, the dryer the soil (water value = ~300, air value = ~730 with my sensor)

WiFiClient client;
HTTPClient http;

double getSleepValue() {
  double sec = 1000000.0;
  double hour = 60 * 60 * sec;

  return SLEEP_HOURS * hour;
}

boolean triggerEvent(char event[], int value) {
  boolean isSuccess = false;
  String url = "http://maker.ifttt.com/trigger/";
  url += event;
  url += "/with/key/";
  url += key;

  if (value != -1) {
    url += "?value1=";
    url += value;
    // secondary value like this: url += "&value2=xyz"
  }

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

boolean triggerEvent(char event[]) {
  return triggerEvent(event, -1);
}

void deepSleep() {
  delay(100);
  ESP.deepSleep(getSleepValue());
}

void wifiSetup() {
  int retries = 0;

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    if (retries < 20) {
      delay(500);
      retries++;
    } else {
      // if connection cannot be established than go to sleep - otherwise it dries the battery out
      deepSleep();
    }
  }
}

void setup() {
  int currentValue = analogRead(A0);
  wifiSetup();

  triggerEvent(log_event, currentValue);

  if (currentValue > DRYNESS_ALARM_VALUE) {
    triggerEvent(notify_event, currentValue);
  }

  deepSleep();
}

void loop() {
}
