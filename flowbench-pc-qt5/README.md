Watermeter Testbench GUI
========================

This is a convenience program to collect incoming serial data from the
Arduino microcontroller, scale it for real-time display, and save to a file.
It also collects incoming serial data on a second serial interface to be used
by the Watermeter serial output.

A button is provided for software start and stop of the data collection process.
This also activates a solenoid via the Arduino to control water flow.

The GUI can display results from both the Arduino output and a special test
program loaded to the watermeter AVR.

1. If the watermeter is used, connect up the interface board and connect its 
   serial output to a serial-USB adapter cable.
2. Ensure the correct serial source is selected. Normally /dev/ttyACM0 for the
   Arduino and /dev/ttyUSB0 for the watermeter (if used).
3. Click on "Connect" for the Arduino and the watermeter (if used).
4. Open the file to record.
5. The displays should show data from the sensors, but recording will not
   commence until "Start" is clicked. If the solenoid is connected this will be
   activated and the recording will start.
6. When finished, click "Stop". Recording will end and the solenoid will be
   deactivated.
7. Close the data file.

The data file is provided in csv format. The first column is a time stamp in
ISO 8601 format. The second column is a value that represents the fractional
seconds times 100.

Thus the sample time is the sum of the first column and the second column
divided by 100. Although ISO 8601 supports fractional seconds, LibreOffice Calc
does not.

The remaining columns contain the values from the watermeter and Arduino:

* Column 5 Pressure.
* Column 6 Temperature.
* Column 7 Count from Arduino in decimal format.
* Column 8 Period from Arduino (only needed for flow rate estimates).
* Column 9 Count from Watermeter in hexadecimal format.
* Column 10 Switch setting for solenoid (always 1 for Arduino samples).

The watermeter samples can be recognised by blank columns 3-8.

K. Sarkies, 28/03/2016

