Watermeter Testbench

An Arduino Uno is used with Screwshield and Sensor Shield to interface a number
of sensors in a flowbench for test and calibration of a watermeter.

Sensors are:

- FS200A flowmeter, 450 pulses per litre.
- Temperature Sensor (?).
- Solenoid operated water valve (?).
- Water pressure sensor (?).
- Start button.
- Chronodot real-time clock based on the DS3231 RTC chip.

The watermeter used is that for the XBee-Acquisition project
https://github.com/ksarkies/XBee-Acquisition

The code is for the Arduino IDE. It requires an additional subfolder "libraries"
with the following libraries:

- RTClib for the Chronodot RTC.
    https://github.com/adafruit/RTClib
- Wire for the TWI (I^2C) to communicate with the Chronodot.
    https://code.google.com/p/arduino/source/browse/trunk/libraries/Wire/?r=1092

K. Sarkies, 22/1/2015

