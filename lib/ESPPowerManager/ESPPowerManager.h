#ifndef ESPPOWERMANAGER_H
#define ESPPOWERMANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

/// Memory offset is 128 bytes because of the OTA/Update class behaviour (rtcUserMemoryRead method warning)
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
 * 	@note Turned off wifi module also helps with more precise analog readings because
 * 	voltage do not gets fluctuated. RTC memory kept during deepsleep/resets, only gets
 * 	lost on power outage.
 *
 * 	@author Dániel Bene - https://github.com/danielbene
 */
class ESPPowerManager {
	public:
		ESPPowerManager(String ssid, String password, IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns);
		u_int64_t ONE_HOUR = 3600000000;

		/// deepSleepMax() value - about ~3.5h - caused issues, using fix 3 hours as maximum sleep timer for the basic mode
		u_int64_t MAX_SLEEP = 3 * ONE_HOUR;

		/**
		 * Begin function for non-extended deepsleep usage (<=3h sleeps).
		 * In this mode the sleep is uninterrupted, and handled by the inner RTC. This is
		 * unprecise, and loses about 3-5 minutes on a 60 minute sleep. Sleep duration can
		 * be specified with microsec accuracy.
		 *
		 * One of the begin functions MUST be called in the beggining of the setup function!
		 */
		void beginBasicMode();

		/**
		 * Begin function for extended deepsleep usage (3h < sleep < 65535h).
		 * Extended sleep mode works with the use of the inner RTC module's memory. Values
		 * in it kept between sleeps.
		 * A counter is placed in the memory. Every hour the ESP wakes in low power mode,
		 * checks the counter and either do the specified jobs (if counter reached the spec
		 * sleep hours), or goes back to sleep after a counter increment.
		 * Sleep duration is hour based smaller values cannot be specified.
		 *
		 * One of the begin functions MUST be called in the beggining of the setup function!
		 *
		 * @note If wake hour not reached, than code execution will not leave the begin
		 * 		 function.
		 *
		 * @param checksumNumber This is for data validation. Pick a random number between
		 * 						 0 and 65535.
		 * @param hours The required sleep hours between the job execution. It can be set
		 * 				between 4 and 65535. (Use simple mode for 1-3.)
		 */
		void beginEDSMode(uint16_t checksumNumber, uint16_t hours);

		/// In basic mode this will sleep the ESP for 3 hours. In EDS mode it does a sleep cycle.
		void deepSleep();

		/// In basic mode this will sleep the ESP for the specified microsecs. In EDS mode it does a sleep cycle.
		void deepSleep(u_int64_t sleepMicroSecs);

		/**
		 * Reactivates RF and tries to connect to the AP described by the constructor params.
		 * If connection cannot be established for 15 seconds, it goes back to sleep.
		 * In basic modes it sleeps for the MAX_SLEEP, in EDS mode it does a sleep cycle,
		 * and because it's in execution the EDS counter restarts.
		 */
		void setupWifi();

		/// Same as setupWifi() but with setable sleep timer for failure in basic mode.
		void setupWifi(u_int64_t sleepMicroSecs);

	private:
		boolean isEDSMode = false;
		String ssid;
		String password;
		IPAddress ip;
		IPAddress dns;
		IPAddress gateway;
		IPAddress subnet;
		uint32_t calculateCRC32(const uint8_t *data, size_t length);
		boolean isRouterDataValid();
		void sleepWifi();
		void turnOffWifi();
		void wakeWifi();
		struct {
			uint32_t crc32;		// validator checksum (4 byte)
			uint8_t channel;	// wifi channel (1 byte)
			uint8_t bssid[6];	// BSSID (MAC address) of the router (6 byte)
			uint8_t padding;	// additional byte for 12 total (1 byte)
		} routerData;
		struct {
			uint16_t cycleCounter;		// hour counter (2 byte) - [0, 65535]
			uint16_t manualChecksum;	// simple number "checksum" (2 byte) - [0, 65535]
		} extendedDSData;

};

#endif
