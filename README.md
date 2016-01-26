Watermeter Testbench
====================

The testbench provides regulated water flows and electronics for testing the
watermeter. It consists of the sensors listed below. An interface board
allows the debug connector on the watermeter to pass out various signals,
including the squared photosensor signal and the serial output. It also
provides an AVR serial programming interface.

An Arduino UNO is used with a SensorShield V4 to interface a number of sensors
for test and calibration of the watermeter.  This is programmed with the sensor
interface conversion software. Data is transmitted to an external PC in csv
format for later spreadsheet analysis. For temperature measurements a MAX31865
RTD to Digital converter board is used with a PT50 temperature sensor. A small
adapter board has been built to facilitate connection to the Arduino.

Sensors are:

* FS200A flowmeter, 450 pulses per litre.
* Temperature Sensor PT100 via MAX31865 board using SPI to the Arduino.
* Solenoid operated water valve (?).
* Water pressure sensor (?).
* Start button.
* Chronodot real-time clock based on the DS3231 RTC chip.
* Watermeter photointerrupter output from comparator.

The watermeter used is that for the XBee-Acquisition project
(https://github.com/ksarkies/XBee-Acquisition).
It is programmed to count pulses and transmit the resulting count once a second
via its serial interface. This is converted to RS232 in the interface board
on the testbench for passing to a PC.

A GUI is provided to allow the Arduino and the Watermeter serial interfaces to
pass data to be displayed and stored.

Tests to be done are:
* Read the Watermeter photointerrupter output and compare with flow
  measurements to identify the scaling factor for the watermeter in litre/min.
* Read the Watermeter count as processed by the Watermeter and transmitted
  serially in csv format for comparison with flowmeter readings. This will
  verify the validity of the Watermeter measurements.
* Read the Watermeter count as processed by the Watermeter and transmitted
  via the XBee to verify the overall system operation.

The code is for the Arduino IDE. It requires an additional subfolder "libraries"
with the following libraries:

* "RTClib" for the Chronodot RTC https://github.com/adafruit/RTClib

K. Sarkies, 26/1/2016

