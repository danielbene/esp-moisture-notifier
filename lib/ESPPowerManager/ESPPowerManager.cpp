#include "ESPPowerManager.h"

ESPPowerManager::ESPPowerManager(String ssid, String password, IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns) {
	this->ssid = ssid;
	this->password = password;
	this->ip = ip;
	this->gateway = gateway;
	this->subnet = subnet;
	this->dns = dns;
}

void ESPPowerManager::beginBasicMode() {
	sleepWifi();
	isEDSMode = false;
}

void ESPPowerManager::beginEDSMode(uint16_t checksumNumber, uint16_t sleepHours) {
	sleepWifi();
	isEDSMode = true;

	if (ESP.rtcUserMemoryRead(DS_CYCLE_MEMORY_OFFSET, (uint32_t*)&extendedDSData, sizeof(extendedDSData))) {
		// Serial.println("EDS COUNTER: " + String(extendedDSData.cycleCounter));
		if (extendedDSData.manualChecksum != checksumNumber) {
			// first run, or memory got corrupted
			extendedDSData.manualChecksum = checksumNumber;
			extendedDSData.cycleCounter = 1;
		} else {
			extendedDSData.cycleCounter += 1;

			if (extendedDSData.cycleCounter > sleepHours) {
				extendedDSData.cycleCounter = 1;
				ESP.rtcUserMemoryWrite(DS_CYCLE_MEMORY_OFFSET, (uint32_t*)&extendedDSData, sizeof(extendedDSData));
				return;	// reached specified sleep count, let the jobs run
			}
		}

		ESP.rtcUserMemoryWrite(DS_CYCLE_MEMORY_OFFSET, (uint32_t*)&extendedDSData, sizeof(extendedDSData));
		deepSleep(ONE_HOUR);
	}
}

void ESPPowerManager::setupWifi() {
	setupWifi(MAX_SLEEP);
}

void ESPPowerManager::setupWifi(u_int64_t sleepMicroSecs) {
	wakeWifi();

	int retries = 0;
	boolean isValidRouterData = isRouterDataValid();

	WiFi.mode(WIFI_STA);
	WiFi.config(ip, gateway, subnet, dns);
	resetWifiState();

	if (isValidRouterData) {
		// saved data ok - quick connect
		WiFi.begin(ssid, password, routerData.channel, routerData.bssid, true);
	} else {
		// saved data invalid - regular connect
		WiFi.begin(ssid, password);
	}

	while (WiFi.status() != WL_CONNECTED) {
		retries++;

		if (retries % 50 == 0) {
			// quick connect is not working, reset WiFi and try regular connection
			resetWifiState();
			WiFi.begin(ssid, password);

			isValidRouterData = false;
		}

		if (retries == 300) {
			// after 30 sec go to sleep
			deepSleep(sleepMicroSecs);
		}

		// faster loop caused deadlock (bcause regular conn takes about 3 sec - quick connect is ~200ms)
		delay(100);
	}

	if (!isValidRouterData){
		// connestion is successfull - save router data to rtc memory - the existing not usable
		routerData.channel = WiFi.channel();
		memcpy(routerData.bssid, WiFi.BSSID(), 6);
		routerData.crc32 = calculateCRC32(((uint8_t*)&routerData) + 4, sizeof(routerData) - 4);
		ESP.rtcUserMemoryWrite(ROUTER_DATA_MEMORY_OFFSET, (uint32_t*)&routerData, sizeof(routerData));
	}
}

// trigger ESP seepSleep with the maximum stable value (3h)
void ESPPowerManager::deepSleep() {
	deepSleep(0);
}

void ESPPowerManager::deepSleep(u_int64_t sleepMicroSecs) {
	turnOffWifi();

	if (!isEDSMode) {
		if (sleepMicroSecs != 0 && sleepMicroSecs < MAX_SLEEP) {
			ESP.deepSleep(sleepMicroSecs, WAKE_RF_DISABLED);
		}

		ESP.deepSleep(MAX_SLEEP, WAKE_RF_DISABLED);
	} else {
		ESP.deepSleep(ONE_HOUR, WAKE_RF_DISABLED);
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
	if (ESP.rtcUserMemoryRead(ROUTER_DATA_MEMORY_OFFSET, (uint32_t*)&routerData, sizeof(routerData))) {
		// Calculate the CRC of what we just read from RTC memory, but skip the first 4 bytes as that's the checksum itself.
		uint32_t crc = calculateCRC32(((uint8_t*)&routerData) + 4, sizeof(routerData) - 4);
		if (crc == routerData.crc32) {
			return true;
		}
	}

	return false;
}

// reactivating RF and disabling network persistance
void ESPPowerManager::wakeWifi() {
	WiFi.forceSleepWake();
	delay(1);
	WiFi.persistent(false);
}

void ESPPowerManager::sleepWifi() {
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(1);
}

void ESPPowerManager::turnOffWifi() {
	WiFi.disconnect(true);
	delay(1);
	WiFi.mode(WIFI_OFF);
}

void ESPPowerManager::resetWifiState() {
	WiFi.disconnect();
	WiFi.forceSleepBegin();
	WiFi.forceSleepWake();
}
