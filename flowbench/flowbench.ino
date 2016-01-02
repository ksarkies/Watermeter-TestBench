/*    Flow Bench Measurement and Transmission

Firmware for measurement of physical quantities in a water flow apparatus for 
testing a water meter.

K W Sarkies 28/1/2015
Paul Rix

Tested on: Arduino Uno ATMega328 at 16MHz.

A timer ISR at 1kHz detects rising edge changes in the flowmeter and watermeter
signals and measures the time between rising edges of the pulses. Results are
averaged over the time between transmissions if more than one pulse is present.

Flow meter FS200A, 450 counts per litre, 25 litres/minute max, pulse rate 188Hz max.
Connect three pin plug (black = gnd, red = 5V, white = signal) to header 3 of
the Arduino Sensor Shield

Water meter.
Connect three pin plug (black = gnd, red = 5V, white = signal) to header 6 of
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

Transmission of data is csv ASCII without leading zeros and CR terminated:
- sample time in ISO date format plus fractional seconds,
- flow meter count 16 bits,
- flow meter period 32 bits,
- pressure sensor 16 bits,
- temperature sensor 16 bits,
- test meter period 32 bits,
- Switch position 1 bit.

NOTE: do NOT use the Arduino serial monitor. Use putty or another monitor.

Linux:       /dev/ttyACMn, n = 0,1,2....
Windows:     COMn, n = 3,4,5..
*/

#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

#define Rref 400      // enter 400 if PT100 used, enter 4000 if PT1000 used
#define WIRE 2        // PT100/1000 has 2,3 or 4 wire connection

#define CS 9

#define FLOWMETER 3     // pin connected to flowmeter output
#define PUSHBUTTON 4
#define SOLENOID 5
#define WATERMETER 6    // pin connected to watermeter photointerruptor output
#define PRESSURE A0
#define TEMPERATURE A1
#define BAUDRATE 38400

unsigned int running = 0;               // Experiment is running
unsigned long flowmeterPeriod = 0;      // Period between flowmeter pulses
unsigned long flowmeterPeriodSum = 0;
unsigned int flowmeterCount = 0;        // Pulse count for flowmeter
unsigned long watermeterPeriod = 0;     // Period between flowmeter pulses
unsigned long watermeterPeriodSum = 0;
unsigned int watermeterCount = 0;       // Pulse count for flowmeter
unsigned int timetick = 0;              // Clock fractional seconds in ms
unsigned int lastSecond = 0;
unsigned int switchVal;                 // Level of switch input
unsigned int lastSwitchVal;             // Previous level of switch input
unsigned int cycleTime;                 // Counts 10 capture cycles for sending data
unsigned int messageIndex;              // Valid received message count
char command;                           // Command sent

//-----------------------------------------------------------------------------
double CallendarVanDusen(double R){
  double a = 3.8418E-03;
//  double a = 3.9083E-03;
  double b = -1.587E-07;
//  double b = -5.7750E-07;
  signed long R0=100.14;
//  signed long R0=Rref/4;
  return (-R0*a+sqrt(R0*R0*a*a-4*R0*b*(R0-R)))/(2*R0*b);  
}

//-----------------------------------------------------------------------------
void setup(){

// Set I/O ports as input or output
  pinMode(1,OUTPUT);                    // tx
  pinMode(FLOWMETER,INPUT_PULLUP);      // flowmeter pulses
  pinMode(WATERMETER,INPUT_PULLUP);     // watermeter pulses
  pinMode(PUSHBUTTON,INPUT_PULLUP);     // start switch
  pinMode(SOLENOID,OUTPUT);             // solenoid

/* TIMER SETUP- the timer interrupt allows precise timed measurements of the
Flowmeter switch for more info about configuration of arduino timers see
http://arduino.cc/playground/Code/Timer1
*/
  
  cli();                               // stop interrupts

//set timer1 interrupt at 1kHz
  TCCR1A = 0;        // set entire TCCR1A register to 0
  TCCR1B = 0;        // set entire TCCR1B register to 0  
  TCNT1  = 0;        //initialize counter value to 0;
  
// set timer count for 1khz increments
  OCR1A = 1999;      // = (16*10^6) / (1000*8) - 1 for 1kHz interrupt, prescale 8
// turn on CTC mode (clear timer OCR1A on compare match)
  TCCR1B |= (1 << WGM12);
// Set CS11 bit for 8 times prescale
  TCCR1B |= (1 << CS11);   
// enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  sei();             // allow interrupts
//END TIMER SETUP

  switchVal = digitalRead(PUSHBUTTON);
  lastSwitchVal = switchVal;

// Initialise serial comms
  Serial.begin(BAUDRATE);

#ifdef AVR
  Wire.begin();  // Arduinos with AVR microcontrollers
#else
  Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
#endif
  rtc.begin();
  
/* This sets the RTC with a compile date & time taken from FLASH. Accuracy
depends on the time between compilation and upload being negligible. It will
reset to this value each time the processor is reset unless the RTC is battery
backed. Chronodot needs CR1620 to CR1632. */
//  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));   
//  }
  lastSecond = rtc.now().second();
  timetick = 0;

// Setup SPI for temperature measurements
  SPI.begin();
  SPI.setClockDivider(200);
  SPI.setDataMode(SPI_MODE3);
  pinMode(CS, OUTPUT);

// Initialise MAX31625 module
  digitalWrite(CS, HIGH);
  delay(100);
  digitalWrite(CS, LOW);
  SPI.transfer(0x80);
  if (WIRE==2 || WIRE==4) SPI.transfer(0xC2);
  if (WIRE==3) SPI.transfer(0xD2);
  digitalWrite(CS, HIGH);
  delay(50);

  messageIndex = 0;
  command = 0;
}

//-----------------------------------------------------------------------------
/* 1 ms clock interrupt.
On interrupt, see if the flowmeter signal has changed from low to high. If so,
register a count and a period. */

ISR(TIMER1_COMPA_vect) {
  static unsigned int lastFlowmeterVal = 0;
  static unsigned long lastFlowmeterTime = 0;
  static unsigned int lastWatermeterVal = 0;
  static unsigned long lastWatermeterTime = 0;
  static unsigned long tick = 0;        // Tick time in ms for flowmeter period
  timetick++;                           // Tick time for improved RTC precision
  tick++;
  
// Flowmeter period
  int flowmeterVal = digitalRead(FLOWMETER);     // Flowmeter signal input
  if ((flowmeterVal == 0) && (lastFlowmeterVal > 0)) {
    flowmeterCount++;                   // New pulse, add to pulse count
    flowmeterPeriod = tick - lastFlowmeterTime - 1;
    if (flowmeterCount > 1) {
      flowmeterPeriodSum += flowmeterPeriod;
    }
    lastFlowmeterTime = tick;
  }
  lastFlowmeterVal = flowmeterVal;
  
// Watermeter period
  int watermeterVal = digitalRead(WATERMETER);    // Watermeter signal input
  if ((watermeterVal == 0) && (lastWatermeterVal > 0)) {
    watermeterCount++;                  // New pulse, add to pulse count
    watermeterPeriod = tick - lastWatermeterTime - 1;
    if (watermeterCount > 1) {
      watermeterPeriodSum += watermeterPeriod;
    }
    lastWatermeterTime = tick;
  }
  lastWatermeterVal = watermeterVal;
}

//-----------------------------------------------------------------------------
void loop(){
// Check for serial data incoming.
  if (Serial.available() > 0) {
    char receivedData = Serial.read();
/* Attempt to validate and act on command
Action commands are of the form "Anm", where n is a single character command
and m a list of parameters:
"As+", "As-" turn solenoid switch on or off. */
    switch(messageIndex) {
// Validate action request as a synchronization check
      case 0:
        if (receivedData == 'A') messageIndex++;
        break;
// Command
      case 1:
        command = receivedData;
        messageIndex++;
        break;
      case 2:
// Switch command on (+) or off (-)
        if (command == 's')
        {
          if (receivedData == '+'){
            digitalWrite(SOLENOID,LOW);
            running = 1;
          }
          if (receivedData == '-') {
            digitalWrite(SOLENOID,HIGH);
            running = 0;
          }
        }
        messageIndex = 0;     // reset index for next command
        break;
    }
  }

/* Check if the run is to start or stop, looking for rising edge on switch
signal. If found, flip running state (0 is off, 1 is on). This can be overridden
by software command from the GUI. */
  switchVal = digitalRead(PUSHBUTTON);
  if ((switchVal == 0) && (lastSwitchVal > 0)) running = (running++ & 0x01);
  lastSwitchVal = switchVal;

  DateTime now = rtc.now();
  if (now.second() != lastSecond) {
    timetick = 0;
    lastSecond = now.second();
  }
 
  if (running > 0) {
    digitalWrite(SOLENOID,LOW);

    unsigned char reg[8];          // array for all the 8 registers
    unsigned int i;
// 16 bit value of RTD MSB & RTD LSB, reg[1] & reg[2]
// RTDdata >> 1, 0th bit of RTDdata is RTD connection fault detection bit
    unsigned int RTDdata;
// 15 bit value of ADC
    unsigned int ADCcode;
    double Resistance;             // actual resistance of PT100(0) sensor

// Access the MAX31625 to get Pt100 resistance reading
    digitalWrite(CS, LOW);
    delay(10);
    SPI.transfer(0);                              // start reading from address=0
    for (i=0; i<8; i++) reg[i]=SPI.transfer(0);   // read all the 8 registers
    delay(10);
    digitalWrite(CS, HIGH);
    RTDdata = reg[1] << 8 | reg[2];
    ADCcode=RTDdata>>1;
    Resistance=(double)ADCcode*Rref/32768;

// Wait for 10 cycles of 10ms to transmit results every 100ms.
    if  (cycleTime++ > 10) {
    
      Serial.print(now.year(), DEC);
      Serial.print('-');
      if (now.month() < 10) Serial.print('0');
      Serial.print(now.month(), DEC);
      Serial.print('-');
      if (now.day() < 10) Serial.print('0');
      Serial.print(now.day(), DEC);
      Serial.print('T');
      if (now.hour() < 10) Serial.print('0');
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      if (now.minute() < 10) Serial.print('0');
      Serial.print(now.minute(), DEC);
      Serial.print(':');
      if (now.second() < 10) Serial.print('0');
      Serial.print(now.second(), DEC);
      Serial.print(".");
      if (timetick < 100)
        Serial.print('0');
      if (timetick < 10)
        Serial.print('0');
      Serial.print(timetick, DEC);
      Serial.print(",");

/* Protect and capture flowmeter count and period average values and reset them.
The flowmeter period is averaged if more than two pulses occur, otherwise
the last measured period is used. */
      cli();
      if (flowmeterCount > 2) {
        flowmeterPeriod = flowmeterPeriodSum/(flowmeterCount - 1);
        flowmeterPeriodSum = 0;
      }
      int currentFlowCount = flowmeterCount;
      flowmeterCount = 0;
      if (watermeterCount > 2) {
        watermeterPeriod = watermeterPeriodSum/(watermeterCount - 1);
        watermeterPeriodSum = 0;
      }
      int currentWaterCount = watermeterCount;
      watermeterCount = 0;
      sei();
// Print Flowmeter Count
      Serial.print(currentFlowCount);
      Serial.print(",");

// Print flowmeter period last measured.
      Serial.print(flowmeterPeriod);
      Serial.print(",");

// Print pressure value
      Serial.print(analogRead(PRESSURE));
      Serial.print(",");
    
// Print temperature value
      Serial.print(CallendarVanDusen(Resistance));
//      Serial.print(analogRead(TEMPERATURE));
      Serial.print(",");

// Print Watermeter Count
      Serial.print(currentWaterCount);
      Serial.print(",");

// Print watermeter period last measured.
      Serial.print(watermeterPeriod);
      Serial.print(",");

// Blank field
      Serial.print(",");

// Print running status.
      Serial.print(running);
//      Serial.print(",");
    
      Serial.println("");

      cycleTime = 0;
    }
  }
  else
    digitalWrite(SOLENOID,HIGH);
// 10ms delay
  delay(10);
}

