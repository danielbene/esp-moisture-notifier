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

#define DRYNESS_ALARM_VALUE 500 // the higher the value, the dryer the soil (water ~300, air ~730)

WiFiClient client;
HTTPClient http;

// ---WIP:POWER_MANAGEMENT--------
IPAddress ip(192, 168, 100, 210);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);
// ---WIP:POWER_MANAGEMENT--------

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
  // theoretically deepSleepMax() is the longest value that can be reached without external parts (needs arduino core 2.4.1+ / ~3,5h)
  // that caused issues for me, so using stable 3 hours as maximum sleep timer
  u_int64_t maxSleep = 3 * 3600000000;

  // ---WIP:POWER_MANAGEMENT--------
  WiFi.disconnect(true);
  delay(1);
  WiFi.mode(WIFI_OFF);
  delay(10);
  ESP.deepSleep(maxSleep, WAKE_RF_DISABLED);
  // ---WIP:POWER_MANAGEMENT--------
}

void wifiSetup() {
  int retries = 0;

  WiFi.mode(WIFI_STA);

  // ---WIP:POWER_MANAGEMENT--------
  WiFi.config(ip, gateway, subnet);
  // ---WIP:POWER_MANAGEMENT--------

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
  // ---WIP:POWER_MANAGEMENT--------
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);
  // ---WIP:POWER_MANAGEMENT--------

  int currentValue = analogRead(A0);

  // ---WIP:POWER_MANAGEMENT--------
  WiFi.forceSleepWake();
  delay(1);
  WiFi.persistent(false);
  // ---WIP:POWER_MANAGEMENT--------

  wifiSetup();

  triggerEvent(log_event, currentValue);

  if (currentValue > DRYNESS_ALARM_VALUE) {
    triggerEvent(notify_event, currentValue);
  }

  deepSleep();
}

void loop() {
}
