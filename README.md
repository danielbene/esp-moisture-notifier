# esp-moisture-notifier
A really basic platformio project to trigger an IFTTT notification if a plant's moisture sensor reads low values.
The IFTTT webhook trigger uses plain html call! Only use this solution, if your use-case allow this not really secure implementation.

Hardware is a D1-mini, with a capacitive soil moisture sensor, supplied by a 18650 power cell.

PRO TIP: you can make the sensor's smds waterproof with nail polish.

Calibration is key when using moisture sensor. The values differ based on many aspects (soil denseness, saltness, sensor position, depth etc.). Be sure to find out the proper value for the `DRYNESS_ALARM_VALUE` before you trust the plant's life on the system.
