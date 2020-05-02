#ifndef ESPPOWERMANAGER_H
#define ESPPOWERMANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define ROUTER_DATA_MEMORY_OFFSET 33
#define DS_CYCLE_MEMORY_OFFSET (ROUTER_DATA_MEMORY_OFFSET + 4)
#define FREE_USER_MEMORY_OFFSET (DS_CYCLE_MEMORY_OFFSET + 1)

/**
 * 	@file ESPPowerManager.h
 *
 *	@brief ESPPowerManager implements practices to get the most lifespan out of
 *	battery cells, while keeping the full functionality of the esp8266 chips.
 *
 *	Lib was inspired by the esp posts on https://www.bakke.online
 *
 * 	Note: turned off wifi module also helps with more precise analog readings because
 * 	voltage do not gets fluctuated.
 *
 * 	@author Dániel Bene - https://github.com/danielbene
 */
class ESPPowerManager {
	public:
		ESPPowerManager(String ssid, String password, IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns);
		u_int64_t ONE_HOUR = 3600000000;
		// deepSleepMax() - that's about ~3.5h - caused issues for me, so using stable 3 hours as maximum sleep timer
		u_int64_t MAX_SLEEP = 3 * ONE_HOUR;
		void begin();
		void deepSleep();
		void deepSleep(u_int64_t sleepMicroSecs);
		void handleExtendedDeepSleep(uint32_t hours, uint8_t checksumNumber);
		void wakeWifi();
		void setupWifi(u_int64_t sleepMicroSecs);

	private:
		String ssid;
		String password;
		IPAddress ip;
		IPAddress dns;
		IPAddress gateway;
		IPAddress subnet;
		uint32_t calculateCRC32(const uint8_t *data, size_t length);
		boolean isRouterDataValid();
		struct {
			uint32_t crc32;		// validator checksum (4 byte)
			uint8_t channel;	// wifi channel (1 byte)
			uint8_t bssid[6];	// MAC address of the router (6 byte)
			uint8_t padding;	// additional byte for 12 total (1 byte)
		} routerData;
		struct {
			uint16_t cycleCounter;	// hour counter (2 byte) - [0, 65535]
			uint8_t flagNumber;		// simple number "checksum" (1 byte) - [0, 255]
			uint8_t firstBoot;		// flag for checking if this is the first boot (1 byte)
		} extendedDSData;

};

#endif
