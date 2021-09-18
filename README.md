# esp-moisture-notifier
A really basic platformio project to trigger an IFTTT notification if a plant's moisture sensor reads low values.
The IFTTT webhook trigger uses plain html call! Only use this solution, if your use-case allow this not really secure implementation.

Hardware is a D1-mini, with a capacitive soil moisture sensor, supplied by a 18650 power cell. I made a simple d1 mini shield for the parts to connect to.

![Sensor connections](img/setup.jpg)

## Discord/GoogleSheets
Changed IFTTT for a native Discord webhook, and GoogleSheets implementation. (TODO: proper description)

## calibration

Calibration is key when using a moisture sensor. The values differ based on many aspects (soil denseness, saltness, sensor position, depth etc.). Be sure to find out the proper value for the `DRYNESS_ALARM_VALUE` before you trust the plant's life on the system.

PRO TIP: you can make the sensor's pcb sides, and smds waterproof with nail polish.

## power-manager

Errr... The project started as a simple moisture sensor, but that power manager lib was born out of it. That has so much value compared to a basic sensor's code. Nvm I just keep that here at the moment, and we will see where to go with it.
