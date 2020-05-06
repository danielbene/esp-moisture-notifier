#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include "ESPPowerManager.h"

#include "local.h"
/* CONTENTS OF LOCAL.H
String ssid = "";                   // your network SSID (name)
String password = "";               // your network key
char key[] = "";                    // from this page https://ifttt.com/maker_webhooks/settings - get the token from the specified URL on the page
char log_event[] = "";              // name of the logging event to trigger
char notify_event[] = "";           // name of the notification event to trigger
IPAddress ip(192, 168, x, x);       // static (not used) ip address for this device
IPAddress gateway(192, 168, x, x);  // address of the network router
IPAddress subnet(255, 255, 255, 0); // subnet mask of the local network
IPAddress dns(1, 1, 1, 1);			// DNS server address
*/

#define DRYNESS_ALARM_VALUE 600		// the higher the value, the dryer the soil (water ~300, air ~730)
//#define SLEEP_USECS powerManager.MAX_SLEEP

ESPPowerManager powerManager(ssid, password, ip, gateway, subnet, dns);
HTTPClient http;
WiFiClient client;

boolean triggerEvent(String event, int value) {
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

boolean triggerEvent(String event) {
	return triggerEvent(event, -1);
}

void setup() {
	//powerManager.beginBasicMode();
	powerManager.beginEDSMode(54321, 8);

	int currentValue = analogRead(A0);

	//powerManager.setupWifi(SLEEP_USECS);
	powerManager.setupWifi();

	triggerEvent(log_event, currentValue);

	if (currentValue > DRYNESS_ALARM_VALUE) {
		triggerEvent(notify_event, currentValue);
	}

	//powerManager.deepSleep(SLEEP_USECS);
	powerManager.deepSleep();
}

void loop() {
}
