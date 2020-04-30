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
	boolean isValidRouterData = isRouterDataValid();

	WiFi.mode(WIFI_STA);
	WiFi.config(ip, gateway, subnet);

	if (isValidRouterData) {
		// saved data ok - quick connect
		WiFi.begin(ssid, password, routerData.channel, routerData.bssid, true );
		Serial.println("Starting quick connect");
	} else {
		// saved data invalid - regular connect
		WiFi.begin(ssid, password);
		Serial.println("Starting regular connect");
	}

	while (WiFi.status() != WL_CONNECTED) {
		retries++;

		if (retries == 100 && isValidRouterData) {
			// quick connect is not working, reset WiFi and try regular connection
			WiFi.disconnect();
			delay(10);
			WiFi.forceSleepBegin();
			delay(10);
			WiFi.forceSleepWake();
			delay(10);
			WiFi.begin(ssid, password);

			// possibly a change made the saved data useless
			isValidRouterData = false;

			Serial.println("Quick connect failed - try regular");
		}

		if (retries == 300) {
			Serial.println("Connection timeout - going to sleep");
			// after 15 sec go to sleep
			deepSleep();
		}

		delay(50);
	}

	if (!isValidRouterData){
		// connestion is successfull - save router data to rtc memory if the existing not usable
		routerData.channel = WiFi.channel();
		memcpy(routerData.bssid, WiFi.BSSID(), 6);	// Copy 6 bytes of BSSID (AP's MAC address)
		routerData.crc32 = calculateCRC32(((uint8_t*)&routerData) + 4, sizeof(routerData) - 4);
		ESP.rtcUserMemoryWrite(0, (uint32_t*)&routerData, sizeof(routerData));

		Serial.print("Saving data to rtc memory: ");
		Serial.print(routerData.channel);
		Serial.print(" / ");
		Serial.print(routerData.bssid[0]);
		Serial.print(routerData.bssid[1]);
		Serial.print(routerData.bssid[2]);
		Serial.print(routerData.bssid[3]);
		Serial.print(routerData.bssid[4]);
		Serial.println(routerData.bssid[5]);
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

uint32_t ESPPowerManager::calculateCRC32(const uint8_t *data, size_t length) {
	uint32_t crc = 0xffffffff;
	while (length--) {
		uint8_t c = *data++;
		for (uint32_t i = 0x80; i > 0; i >>= 1) {
			bool bit = crc & 0x80000000;
			if (c & i) {
				bit = !bit;
			}

			crc <<= 1;
			if (bit) {
				crc ^= 0x04c11db7;
			}
		}
	}

	return crc;
}

boolean ESPPowerManager::isRouterDataValid() {
	if (ESP.rtcUserMemoryRead(0, (uint32_t*)&routerData, sizeof(routerData))) {
		// Calculate the CRC of what we just read from RTC memory, but skip the first 4 bytes as that's the checksum itself.
		uint32_t crc = calculateCRC32(((uint8_t*)&routerData) + 4, sizeof(routerData) - 4);
		if (crc == routerData.crc32) {
			return true;
		}
	}

	return false;
}
