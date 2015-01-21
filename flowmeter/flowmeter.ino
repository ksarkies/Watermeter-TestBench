
/*    Flow meter FS200A flow rate measurement.

 Ken W Sarkies 2014-2015
 based on
http://www.instructables.com/id/Arduino-Timer-Interrupts/

Flow meter FS200A, 450 counts per litre, 25 litres/minute max, pulse rate 188Hz.

Connect three pin plug (black = gnd, red = 5V, white = signal) to header 3 of the Arduino Sensor Shield

A timer ISR at 1kHz detects rising edge changes in the watermeter signal and counts the number of pulses over
a given time period (1 second). Divide by 450*60 to get the number of litres per minute.
*/

#define reed 3                          //pin connected to reed switch

int reedVal;
int count = 0.00;

void setup(){
  
  pinMode(1,OUTPUT);                    //tx
  pinMode(reed,INPUT);                  //reed switch
  
 
  Serial.write(12);                    //clear screen
  
  // TIMER SETUP- the timer interrupt allows precise timed measurements of the reed switch
  //for more info about configuration of arduino timers see http://arduino.cc/playground/Code/Timer1
  
  cli();                              //stop interrupts

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
  
  sei();                              //allow interrupts
  //END TIMER SETUP
  
  Serial.begin(9600);
}

/* On interrupt, see if the meter signal has changed from low to high. If so, register a count. */

ISR(TIMER1_COMPA_vect) {                //Interrupt at freq of 1kHz to measure reed switch
  static int oldReedVal = 0;
  int reedVal = digitalRead(reed);      //Signal input
  if ((reedVal > 0) && (oldReedVal == 0)) {
    count++;                            //New pulse, add to pulse count
  }
  oldReedVal = reedVal;
}

void displayCount(){
 // Serial.write(12);//clear
  Serial.write(" Flow =");
  Serial.write(13);//start a new line
  Serial.print(count);
  Serial.write(" pulses ");
//  Serial.print((float)count*1000/(450*60));
//  Serial.write(" millilitres per minute ");
  
}

void loop(){
  //print count once a second
  displayCount();
  Serial.println();
  count = 0;
  delay(1000);
  }


