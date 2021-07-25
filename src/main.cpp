#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#include "ESPPowerManager.h"
#include "local.h"
/* CONTENTS OF LOCAL.H
#include <Arduino.h>
#include <ESP8266HTTPClient.h>

String ssid = "";                   // your network SSID (name)
String password = "";               // your network key
String webhook_url = "";            // Discord channel webhook URL
IPAddress ip(192, 168, x, x);       // static (not used) ip address for this device
IPAddress gateway(192, 168, x, x);  // address of the network router
IPAddress subnet(255, 255, 255, 0); // subnet mask of the local network
IPAddress dns(1, 1, 1, 1);          // DNS server address
*/

#define DRYNESS_ALARM_VALUE 600	    // the higher the value, the dryer the soil (water ~300, air ~730)
#define SENSOR_POWER_PIN D1
//#define SLEEP_USECS powerManager.MAX_SLEEP

ESPPowerManager powerManager(ssid, password, ip, gateway, subnet, dns);
HTTPClient http;
WiFiClientSecure clientSecure;

boolean notifyEvent(int value) {
	// TODO: cert - https://www.youtube.com/watch?v=Wm1xKj4bKsY
	// https://github.com/maditnerd/discord_test/blob/master/discord_test_esp8266/discord.h
	clientSecure.setInsecure();
	boolean isSuccess = false;

	if (http.begin(clientSecure, webhook_url)) {
		http.addHeader("Content-Type", "application/json");

		// TODO: add TS to content
		String payload = "{ \"content\": \" Avocado needs a drink! Value: [" + String(value) + "]\" }";
		int httpCode = http.POST(payload);
		http.end();

		if (httpCode > 0) {
			if (httpCode == 200 || httpCode == 204) {
				isSuccess = true;
			} else {
				isSuccess = false;
			}
		}
	}

	return isSuccess;
}

void setup() {
	//Serial.begin(115200);
	powerManager.beginBasicMode();
	//powerManager.beginEDSMode(54545, 12);

	pinMode(SENSOR_POWER_PIN, OUTPUT);
	digitalWrite(SENSOR_POWER_PIN, HIGH);
	delay(50);  // sensor needs some time to init, or measurement will be incorrect
	int currentValue = analogRead(A0);
	digitalWrite(SENSOR_POWER_PIN, LOW);

	//powerManager.setupWifi(SLEEP_USECS);
	powerManager.setupWifi();

	if (WiFi.status() == WL_CONNECTED) {
		if (!notifyEvent(currentValue)) {
			// TODO: log error
			//Serial.println("notify ERROR");
		}

		// TODO: spreadsheet log
	}

	//powerManager.deepSleep(SLEEP_USECS);
	powerManager.deepSleep();
}

void loop() {
}
