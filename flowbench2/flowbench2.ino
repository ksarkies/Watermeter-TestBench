




// flowbench data recording, timer and solenoid valve control.
//  GO switch is mechanically (?) latched microswitch, switching between GND and 5V with debounce (?)

// void loop
// START = 1  ?  then, auto-update time from laptop & print date time (date_and_time ROUTINE)
// print "what is flow rotameter reading in LPH ?" 
// Serial input if new value ... store LPH = xx
// print "flow rotameter reading is" xx "LPH"  Y/N ?
// print "what is the size of the calibrated vessel in litres (default = 54 litres) ?" 
// Serial input if new value ... store LPH = xx
// print "vessel size is" xx "LPH"  Y/N ?
// calculate vessel filling time and print it
// print "number of readings required ?"
// Serial input value,  or default to 500

// if GO=0,   
// stop stopwatch count
// close solenoid valve
// print stopwatch mmm:ss:hh, temperature, pressure,  flow, watermeter output
// set stopwatch = 00:00:00
// print "END"

// IF GO= 1, 
// open solenoid valve
//  set stopwatch = STOPWATCH(time)
// temperature = (input * factor) + offset
// pressure =  (input * factor) + offset
// flow = FLOW ROUTINE
// watermeter = WATERMETER ROUTINE
// print stopwatch mmm:ss:hh, temperature, pressure,  flow, watermeter output
// 



#include <Wire.h>    // connect DS3231 RTC via I2C and Wire Lib
#include "RTClib.h"  // Adafruit RTC library
RTC_DS1307 RTC;

void setup() {
  Serial.begin(9600);

  // Instantiate the RTC
  Wire.begin();
  RTC.begin();
 
  // Check if the RTC is running.
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running");
  }

  // This section grabs the current datetime and compares it to
  // the compilation time.  If necessary, the RTC is updated.
  DateTime now = RTC.now();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (now.unixtime() < compiled.unixtime()) {
    Serial.println("RTC is older than compile time! Updating");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  Serial.println("Real Time Clock is synchronized with laptop time");
}

int intFailedTries=0;   //counter for fetching serial input data
int readings = 500;     // default number of readings to be taken
float Volume = 54.44;   // volume of larest calibrated vessel (default)
boolean start = 0;
void loop () {
  if (start == 1){
    date_and_time();
repeatLPH: 
    Serial.println ("What is the flowrate reading shown on the Rotameter (litres/hour) ? ");
    int lph = fetchNumber();
    Serial.print ("Flow rate is ");
    Serial.print (lph);
    Serial.println (" litres per hour  ...   Y/N ?");
    int YorN = Serial.read();
    if  (( YorN == 'N')  ||  (YorN == 'n'))  
    {
      goto repeatLPH;   
    }
repeatVolume: 
    Serial.println ("Is the calibrated vessel a) 54.44 litres OR  b)  1.000 litres ? ");
    if (Serial.read() == 'b')  {
      Volume = 1.000; 
    }
    Serial.print ("Volume is ");
    Serial.print (Volume);
    Serial.println (" litres   ...   Y/N ?");
    YorN = Serial.read();
    if  (( YorN == 'N')  ||  (YorN == 'n'))  
    {
      goto repeatVolume;
    }
    float filltime = 60*Volume/float(lph);
    Serial.print ("Calculated filling time is ");
    Serial.print (filltime);
    Serial.println ("   minutes");
repeatReadings: 
    Serial.println ("How many readings will be needed ?");
    readings = fetchNumber();
    Serial.print ("Readings will be ");
    Serial.print (filltime*60/readings);
    Serial.println ("   seconds apart    ...   Would you like to change the number of readings  Y/N  ?");
    YorN = Serial.read();
    if  (( YorN == 'N')  ||  (YorN == 'n'))  
    {
      goto repeatReadings;
    }
      
    }

 

// if GO=0,   
// stop stopwatch count
// close solenoid valve
// print stopwatch HH:mm:ss:hh, temperature, pressure,  flow, watermeter output
// set stopwatch = 00:00:00
// print "END"

// IF GO= 1, 
// open solenoid valve
//  set stopwatch = STOPWATCH(time)
// temperature = (input * factor) + offset
// pressure =  (input * factor) + offset
// flow = FLOW ROUTINE
// watermeter = WATERMETER ROUTINE
// print stopwatch HH:mm:ss:hh, temperature, pressure,  flow, watermeter output
// 

 }


void date_and_time() {
  // Get the current time
  DateTime now = RTC.now();   

  // Display the current time
  Serial.print("Current time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  delay(10000);
}

void stopwatch() {
  // Get the current time
  DateTime now = RTC.now();   

  // Display the current time
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.print(':');
  Serial.print (hundredths(),DEC);
}

void hundredths ()  {
  // count in hundredths of a second and reset when an interrupt occurs from the RTC every second
}

int fetchNumber(){  //Accepts ASCI input from the IDE's serial monitor, and returns an integer
  int intInput=0;
  intFailedTries=0;
  while (Serial.available()==0) {
    ;
  };  //do nothing (except continue to check) until something has been sent.
  while ((Serial.available()>0) || (intFailedTries<1000))
  {
    intInput=FetchNextCharacterAndAddToAccumulator(intInput);
  };
  return(intInput);
}

int FetchNextCharacterAndAddToAccumulator(int intAccumulator)
{ 
  int intIncomingByte;
  intIncomingByte=intAsciiToBinary(Serial.read());
  if (intIncomingByte!=-1) {
    intAccumulator=(intAccumulator*10)+intIncomingByte;
  };
  intFailedTries++;
  return intAccumulator;
}

int intAsciiToBinary(int intIn)
/*this function is for use while reading a number
 from the serial stream. It will convert the Ascii for
 digit to the binary for the same, and return -1 for all
 other input.*/
{
  int intTmp=-1;
  if ((intIn>47) && (intIn<58)) {
    intTmp=intIn-48;
  };
  return intTmp;
}




