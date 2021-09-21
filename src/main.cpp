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
String discord_webhook_url = "";    // Discord channel webhook URL
String sheets_webapp_url = "";      // Google sheets script deployment webapp URL
IPAddress ip(192, 168, x, x);       // static (not used) ip address for this device
IPAddress gateway(192, 168, x, x);  // address of the network router
IPAddress subnet(255, 255, 255, 0); // subnet mask of the local network
IPAddress dns(1, 1, 1, 1);          // DNS server address
*/

#define DRYNESS_ALARM_VALUE 600	    // the higher the value, the dryer the soil (water ~300, air ~730)
#define SENSOR_POWER_PIN D8
//#define SLEEP_USECS powerManager.MAX_SLEEP

ESPPowerManager powerManager(ssid, password, ip, gateway, subnet, dns);
HTTPClient discordHttp, gSheetsHttp;
WiFiClientSecure clientSecure;

boolean notifyEvent(int value) {
	// TODO: cert - https://www.youtube.com/watch?v=Wm1xKj4bKsY
	// https://github.com/maditnerd/discord_test/blob/master/discord_test_esp8266/discord.h
	clientSecure.setInsecure();
	boolean isSuccess = false;

	if (discordHttp.begin(clientSecure, discord_webhook_url)) {
		discordHttp.addHeader("Content-Type", "application/json");

		String payload = "{ \"content\": \" Avocado needs a drink! Value: [" + String(value) + "]\" }";
		int httpCode = discordHttp.POST(payload);
		discordHttp.end();

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

// for setting up google sheets api follow this manual:
// https://github.com/StorageB/Google-Sheets-Logging/blob/master/README.md
void spreadsheetValueLog(int value) {
	gSheetsHttp.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
	if (gSheetsHttp.begin(clientSecure, sheets_webapp_url)) {
			gSheetsHttp.addHeader("Content-Type", "application/json");

			// TODO: extract sheet name to param
			String payload = "{ \"sheet_name\": \"Sheet1\", \"values\": \"" + String(value) + "\" }";
			gSheetsHttp.POST(payload);
			gSheetsHttp.end();
	}
}

void setup() {
	//Serial.begin(115200);
	//powerManager.beginBasicMode();
	powerManager.beginEDSMode(48317, 4);  // TODO: increase interval after test phase

	pinMode(SENSOR_POWER_PIN, OUTPUT);
	digitalWrite(SENSOR_POWER_PIN, HIGH);
	delay(50);  // sensor needs some time to init, or measurement will be incorrect
	int currentValue = analogRead(A0);
	digitalWrite(SENSOR_POWER_PIN, LOW);

	//powerManager.setupWifi(SLEEP_USECS);
	powerManager.setupWifi();  // EDS

	if (WiFi.status() == WL_CONNECTED) {
		if (currentValue > DRYNESS_ALARM_VALUE && !notifyEvent(currentValue)) {
			// TODO: log error (maybe to spreadsheet?)
		}

		spreadsheetValueLog(currentValue);
	}

	//powerManager.deepSleep(SLEEP_USECS);
	powerManager.deepSleep();  // EDS
}

void loop() {
}
