#ifndef ESPPOWERMANAGER_H
#define ESPPOWERMANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

class ESPPowerManager {
	public:
		u_int64_t MAX_SLEEP = 3 * 3600000000;
		void deepSleep();
	private:

};

#endif
