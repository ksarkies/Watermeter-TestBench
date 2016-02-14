Watermeter Testbench GUI
========================

This is a convenience program to collect incoming serial data from the
Arduino microcontroller, scale it for real-time display, and save to a file.
It also collects incoming serial data on a second serial interface to be used
by the Watermeter serial output.

A button is provided for software start and stop of the data collection process.

The GUI can display results from both the Arduino output and a special test
program loaded to the watermeter AVR.

1. If the watermeter is used, connect up the interface board and connect its 
   serial output to a serial-USB adapter cable.
2. Ensure the correct serial source is selected. Normally /dev/ttyACM0 for the
   Arduino and /dev/ttyUSB0 for the watermeter if used.
3. Click on "Connect" for the Arduino and the watermeter if used.
4. Open the file to record.
5. The displays should show data from the sensors, but recording will not
   commence until "Start" is clicked. If the solenoid is connected this will be
   activated and the recording will start.
6. When finished, click "Stop". Recording will end and the solenoid will be
   deactivated.
7. Close the data file.

K. Sarkies, 26/01/2016

