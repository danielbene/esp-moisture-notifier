# esp-moisture-notifier (v2)
A simple platformio project to trigger a notification (through Discord) if my precious plant's moisture sensor reads low values.
Measurements gets logged into a Google Spreadsheet too (every 12 hours by default - but this is configureable). Proper https useage not yet implemented.

Dropped IFTTT from the original version because of their changes in the free plan. You can find the backup of the original as a release package in this repo.

Hardware is a D1-mini, with a capacitive soil moisture sensor, supplied by a 18650 power cell. I made a simple d1 mini shield for the parts to connect to.

![Sensor connections](https://raw.githubusercontent.com/danielbene/project-media/master/esp-moisture-notifier/setup.jpg)

## Discord/GoogleSheets
Discord has a really good open api for messages and lately it's becoming my go-to notifyer. You just need to generate a channel webhook and add it to the local.h file as the pattern in main.cpp shows. [Intro-to-webhooks](https://support.discord.com/hc/en-us/articles/228383668-Intro-to-Webhooks)

With some research I found a way to append a Google Spreadsheet with simple rest calls. It's needs some setup in the Google side to work, but in my opinion this is a really great utility that I will totally use in future project too. You can find a tutorial [here](https://github.com/StorageB/Google-Sheets-Logging/blob/master/README.md). I just got rid of the uneccessary parts from the example GScript, and it works like a charm.

So basically both of the events is a simple rest api call without auth, data mod, etc to reduce the worktime of the esp. (Like before, with the IFTTT, but it's better.)

## calibration

Calibration is key when using a moisture sensor. The values differ based on many aspects (soil denseness, saltness, sensor position, depth etc.). Be sure to find out the proper value for the `DRYNESS_ALARM_VALUE` before you trust the plant's life on the system.

PRO TIP: you can make the sensor's pcb sides, and smds waterproof with nail polish.

## power-manager

The project started as a simple moisture sensor, but that power manager lib was born with it. That has so much value compared to the basic sensor's code. Nvm I just keep that here at the moment, and we will see where to go with it.
