
/*    Flow Bench Measurement and Transmission

K W Sarkies 2015

Flow meter FS200A, 450 counts per litre, 25 litres/minute max, pulse rate 188Hz max.
Connect three pin plug (black = gnd, red = 5V, white = signal) to header 3 of
the Arduino Sensor Shield

A timer ISR at 1kHz detects rising edge changes in the flowmeter signal and
counts the number of pulses over a given time period (1 second). Divide by
450*60 to get the number of litres per minute.

Pressure sensor unknown. 1.2MPa.
Connect three pin plug (black = gnd, red = 5V, white = signal) to header A0 of
the Arduino Sensor Shield. Read once per second.

Temperature sensor PT50 platinum wire plus bridge to current loop 4-20mA.
240 ohm resistor gives 0-5V.
Connect three pin plug (black = gnd, red = 5V, yellow = signal) to header A1 of
the Arduino Sensor Shield. Read once per second.

Start switch normally high, momentarily low.
NB need to hold the button down for at least a second for detection.
Connect three pin plug (black = gnd, red = 5V, white = signal) to header 4 of
the Arduino Sensor Shield

Solenoid 2W-200-20 high is off, low is on.
Connect three pin plug (black = gnd, red = 5V, white = signal) to header 5 of
the Arduino Sensor Shield

*/

#define reed 3                          // pin connected to reed switch
#define pushbutton 4
#define solenoid 5

unsigned int running = 0;               // experiment is running
unsigned int reedVal;
unsigned long count = 0.00;             // pulse count for flowmeter
unsigned int switchVal;
unsigned int oldSwitchVal;
unsigned int cycleTime;                 // 10 capture cycles per second for sending data

void setup(){
  
  pinMode(1,OUTPUT);                    // tx
  pinMode(reed,INPUT);                  // reed switch
  pinMode(pushbutton,INPUT);            // start switch
  pinMode(solenoid,OUTPUT);             // solenoid
  
 
  Serial.write(12);                     // clear screen
  
/* TIMER SETUP- the timer interrupt allows precise timed measurements of the
reed switch for more info about configuration of arduino timers see
http://arduino.cc/playground/Code/Timer1
*/
  
  cli();                               // stop interrupts

//set timer1 interrupt at 1kHz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// set entire TCCR1B register to 0  
  TCNT1  = 0;//initialize counter value to 0;
  
// set timer count for 1khz increments
  OCR1A = 1999;                      // = (16*10^6) / (1000*8) - 1 for 1kHz interrupt with prescale 8
// turn on CTC mode (clear timer OCR1A on compare match)
  TCCR1B |= (1 << WGM12);
// Set CS11 bit for 8 times prescale
  TCCR1B |= (1 << CS11);   
// enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  sei();                              // allow interrupts
//END TIMER SETUP
  
  switchVal = digitalRead(pushbutton);
  oldSwitchVal = switchVal;
  
  Serial.begin(57600);
}

/* On interrupt, see if the meter signal has changed from low to high. If so,
register a count. */

ISR(TIMER1_COMPA_vect) {                // Interrupt at freq of 1kHz to measure reed switch
  static int oldReedVal = 0;
  int reedVal = digitalRead(reed);      // Signal input
  if ((reedVal > 0) && (oldReedVal == 0)) {
    count++;                            // New pulse, add to pulse count
  }
  oldReedVal = reedVal;
}

void displayCount(int count){
//  Serial.write(12);//clear
  Serial.print("Flow = ");
  Serial.print(count);
  Serial.println(" pulses ");
//  Serial.print((float)count*1000/(450*60));
//  Serial.write(" millilitres per minute ");
  
}

void loop(){

// Check if the run is to start or stop looking for rising edge on switch signal
  switchVal = digitalRead(pushbutton);
  if ((switchVal == 0) && (oldSwitchVal > 0))
    running = ~running;
  oldSwitchVal = switchVal;
 
  if (running > 0) {
    digitalWrite(solenoid,LOW);
    if  (cycleTime++ > 10) {
// Protect and capture flowmeter count value and reset it.
      cli();
      int currentCount = count;
      count = 0;
      sei();
      displayCount(currentCount);

// Print pressure value
      Serial.write(" Pressure = ");
      Serial.println(analogRead(A0));
    
// Print pressure value
      Serial.write(" Temperature = ");
      Serial.println(analogRead(A1));
    
      cycleTime = 0;
    }
  }
  else
        digitalWrite(solenoid,HIGH);
  delay(100);
}

