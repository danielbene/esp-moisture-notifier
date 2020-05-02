#ifndef ESPPOWERMANAGER_H
#define ESPPOWERMANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

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
		// deepSleepMax() - that's about ~3.5h - caused issues for me, so using stable 3 hours as maximum sleep timer
		u_int64_t MAX_SLEEP = 3 * 3600000000;
		void begin();
		void deepSleep();
		void deepSleep(u_int64_t sleepMicroSecs);
		void wakeWifi();
		void setupWifi();

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
			uint32_t crc32;		// validator checksum
			uint8_t channel;	// wifi channel
			uint8_t bssid[6];	// MAC address of the router
			uint8_t padding;	// additional byte for 12 total
		} routerData;

};

#endif
