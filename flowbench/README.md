Watermeter Testbench Firmware

This is the Arduino firmware for the testbench experiments.

Flow meter FS200A, 450 counts per litre, 25 litres/minute max, pulse rate 188Hz max.
Connect three pin plug (black = gnd, red = 5V, white = signal) to header 3 of
the Arduino Sensor Shield

Pressure sensor unknown. 0.5V - 4.5V for 0MPa - 1.2MPa.
Connect three pin plug (black = gnd, red = 5V, white = signal) to header A0 of
the Arduino Sensor Shield.

Temperature sensor PT50 platinum wire plus bridge to current loop 4-20mA.
240 ohm resistor gives 1-5V.
Connect three pin plug (black = gnd, red = 5V, yellow = signal) to header A1 of
the Arduino Sensor Shield.

Start switch normally high, momentarily low.
Connect three pin plug (black = gnd, red = 5V, white = signal) to header 4 of
the Arduino Sensor Shield.

Solenoid 2W-200-20 high is off, low is on.
Connect three pin plug (black = gnd, red = 5V, white = signal) to header 5 of
the Arduino Sensor Shield.


K. Sarkies, 26/2/2015

