
/*    flow Bench Measurement and Transmission

Firmware for measurement of physical quantities in a water flow apparatus for 
testing a water meter.

K W Sarkies 28/1/2015
Paul Rix

Tested on: Arduino Uno ATMega328 at 16MHz.

Flow meter FS200A, 450 counts per litre, 25 litres/minute max, pulse rate 188Hz max.
Connect three pin plug (black = gnd, red = 5V, white = signal) to header 3 of
the Arduino Sensor Shield

A timer ISR at 1kHz detects rising edge changes in the flowmeter signal and
measures the time between rising edges of the pulses. Result is averaged over
the time between transmissions if more than one pulse is present.

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

NOTE: do NOT use the Arduino serial monitor. Use putty or another monitor on /dev/ttyACM0.
*/

#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

#define FLOWMETER 3                          // pin connected to flowmeter switch
#define PUSHBUTTON 4
#define SOLENOID 5
#define PRESSURE A0
#define TEMPERATURE A1
#define BAUDRATE 38400

unsigned int running = 0;               // Experiment is running
unsigned long flowmeterPeriod = 0;      // Period between flowmeter pulses
unsigned long flowmeterPeriodSum = 0;
unsigned int flowmeterCount = 0;        // Pulse count for flowmeter
unsigned int timetick = 0;              // Clock fractional seconds in ms
unsigned int lastSecond = 0;
unsigned int switchVal;                 // Level of switch input
unsigned int lastSwitchVal;             // Previous level of switch input
unsigned int cycleTime;                 // 10 capture cycles per second for sending data

void setup(){

// Set I/O ports as input or output
  pinMode(1,OUTPUT);                    // tx
  pinMode(FLOWMETER,INPUT);             // flowmeter switch
  pinMode(PUSHBUTTON,INPUT);            // start switch
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
  OCR1A = 1999;      // = (16*10^6) / (1000*8) - 1 for 1kHz interrupt with prescale 8
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
  
  Serial.begin(BAUDRATE);

#ifdef AVR
  Wire.begin();  // Arduinos with AVR microcontrollers
#else
  Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
#endif
  rtc.begin();
  
/* This sets the RTC with a compile date & time taken from FLASH. Accuracy depends on the
time between compilation and upload being negligible. It will reset to this value each
time the processor is reset unless the RTC is battery backed.
Chronodot needs CR1620 to CR1632. */
//  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));   
//  }
  lastSecond = rtc.now().second();
  timetick = 0;
}

/* 1 ms clock interrupt.
On interrupt, see if the flowmeter signal has changed from low to high. If so,
register a count and period. */

ISR(TIMER1_COMPA_vect) {
  static unsigned int lastFlowmeterVal = 0;
  static unsigned long lastFlowmeterTime = 0;
  static unsigned long tick = 0;                 // Tick time in ms for flowmeter period
  timetick++;                                    // Tick time for improved RTC precision
  tick++;
// Flowmeter period
  int flowmeterVal = digitalRead(FLOWMETER);     // Flowmeter signal input
  if ((flowmeterVal == 0) && (lastFlowmeterVal > 0)) {
    flowmeterCount++;                            // New pulse, add to pulse count
    flowmeterPeriod = tick - lastFlowmeterTime - 1;
    if (flowmeterCount > 1) {
      flowmeterPeriodSum += flowmeterPeriod;
    }
    lastFlowmeterTime = tick;
  }
  lastFlowmeterVal = flowmeterVal;
}

void loop(){

// Check if the run is to start or stop looking for rising edge on switch signal
  switchVal = digitalRead(PUSHBUTTON);
  if ((switchVal == 0) && (lastSwitchVal > 0)) running = ~running;
  lastSwitchVal = switchVal;

  DateTime now = rtc.now();
  if (now.second() != lastSecond) {
    timetick = 0;
    lastSecond = now.second();
  }
 
  if (running > 0) {
    digitalWrite(SOLENOID,LOW);

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
      int currentCount = flowmeterCount;
      flowmeterCount = 0;
      sei();
      Serial.print(currentCount);
      Serial.print(",");

// Print flow meter period last measured.
      Serial.print(flowmeterPeriod);
      Serial.print(",");

// Print pressure value
      Serial.print(analogRead(PRESSURE));
      Serial.print(",");
    
// Print temperature value
      Serial.print(analogRead(TEMPERATURE));
//      Serial.print(",");
    
      Serial.println("");

      cycleTime = 0;
    }
  }
  else
        digitalWrite(SOLENOID,HIGH);
  delay(10);
}

