#include "ESPPowerManager.h"

ESPPowerManager::ESPPowerManager(String ssid, String password, IPAddress ip, IPAddress gateway, IPAddress subnet) {
	this->ssid = ssid;
	this->password = password;
	this->ip = ip;
	this->gateway = gateway;
	this->subnet = subnet;
}

// initial RF modul turn off - must be called first in setup
void ESPPowerManager::begin() {
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(1);
}

// reactivating the module and disabling network persistance
void ESPPowerManager::wakeWifi() {
	WiFi.forceSleepWake();
	delay(1);
	WiFi.persistent(false);
}

void ESPPowerManager::setupWifi() {
	int retries = 0;

	WiFi.mode(WIFI_STA);
	WiFi.config(ip, gateway, subnet);
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

// trigger ESP seepSleep with the maximum stable value (3h)
void ESPPowerManager::deepSleep() {
	deepSleep(0);
}

void ESPPowerManager::deepSleep(u_int64_t sleepMicroSecs) {
	WiFi.disconnect(true);
	delay(1);
	WiFi.mode(WIFI_OFF);
	delay(10);

	if (sleepMicroSecs != 0) {
		ESP.deepSleep(sleepMicroSecs, WAKE_RF_DISABLED);
	} else {
		ESP.deepSleep(MAX_SLEEP, WAKE_RF_DISABLED);
	}
}
