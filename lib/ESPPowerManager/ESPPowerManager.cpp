#include "ESPPowerManager.h"

ESPPowerManager::ESPPowerManager(String ssid, String password, IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns) {
	this->ssid = ssid;
	this->password = password;
	this->ip = ip;
	this->gateway = gateway;
	this->subnet = subnet;
	this->dns = dns;
}

// initial RF modul turn off - must be called first in setup
void ESPPowerManager::begin() {
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(1);
}

// reactivating RF and disabling network persistance
void ESPPowerManager::wakeWifi() {
	WiFi.forceSleepWake();
	delay(1);
	WiFi.persistent(false);
}

void ESPPowerManager::setupWifi(u_int64_t sleepMicroSecs) {
	int retries = 0;
	boolean isValidRouterData = isRouterDataValid();

	WiFi.mode(WIFI_STA);
	WiFi.config(ip, gateway, subnet, dns);

	if (isValidRouterData) {
		// saved data ok - quick connect
		WiFi.begin(ssid, password, routerData.channel, routerData.bssid, true);
	} else {
		// saved data invalid - regular connect
		WiFi.begin(ssid, password);
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

			isValidRouterData = false;	// saved data out of date - flag for invalid
		}

		if (retries == 300) {
			// after 15 sec go to sleep
			deepSleep(sleepMicroSecs);
		}

		delay(50);
	}

	if (!isValidRouterData){
		// connestion is successfull - save router data to rtc memory - the existing not usable
		routerData.channel = WiFi.channel();
		memcpy(routerData.bssid, WiFi.BSSID(), 6);
		routerData.crc32 = calculateCRC32(((uint8_t*)&routerData) + 4, sizeof(routerData) - 4);
		// memory offset read/write is 128 byte because of the OTA behaviour (read the doc/note on this method)
		ESP.rtcUserMemoryWrite(ROUTER_DATA_MEMORY_OFFSET, (uint32_t*)&routerData, sizeof(routerData));
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

	if (sleepMicroSecs != 0) {
		ESP.deepSleep(sleepMicroSecs, WAKE_RF_DISABLED);
	} else {
		ESP.deepSleep(MAX_SLEEP, WAKE_RF_DISABLED);
	}
}

void ESPPowerManager::handleExtendedDeepSleep(uint32_t hours, uint8_t checksumNumber) {
	// basically this is for 3+ hours of deepsleep
	// plan is to wake the esp every hour (without wifi and things, so its low power)
	// check counter in rtc memory
	// than increment and go back to sleep, or do job if "hours" reached

	// question is how the esp will know on wake that its extendedDS or not without costly actions

	if (ESP.rtcUserMemoryRead(DS_CYCLE_MEMORY_OFFSET, (uint32_t*)&extendedDSData, sizeof(extendedDSData))) {
		if (extendedDSData.flagNumber != checksumNumber) {
			// first run, or memory corrupted
			extendedDSData.firstBoot = 1;
			extendedDSData.flagNumber = checksumNumber;
			extendedDSData.cycleCounter = 0;
		} else {
			extendedDSData.cycleCounter += 1;
			extendedDSData.firstBoot = 0;

			if (extendedDSData.cycleCounter >= hours) {
				extendedDSData.cycleCounter = 0;
				ESP.rtcUserMemoryWrite(DS_CYCLE_MEMORY_OFFSET, (uint32_t*)&extendedDSData, sizeof(extendedDSData));
				return;	// reached specified sleep count, let the jobs run
			}
		}

		ESP.rtcUserMemoryWrite(DS_CYCLE_MEMORY_OFFSET, (uint32_t*)&extendedDSData, sizeof(extendedDSData));
		deepSleep(ONE_HOUR);
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
