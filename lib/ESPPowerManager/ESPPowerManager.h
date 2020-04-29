#ifndef ESPPOWERMANAGER_H
#define ESPPOWERMANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

/**
 *	ESPPowerManager implements practices in connection with power efficiency for the
 *	esp8266 based chips. The main motivation is to get the most lifespan out of the
 *	battery cells, while keeping the full functionality.
 *
 * 	Note: turned off wifi module helps with more precise analog readings because
 * 	voltage do not gets fluctuated.
 */
class ESPPowerManager {
	public:
		ESPPowerManager(String ssid, String password, IPAddress ip, IPAddress gateway, IPAddress subnet);
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
		IPAddress gateway;
		IPAddress subnet;
};

#endif
