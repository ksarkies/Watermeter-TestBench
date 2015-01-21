/*    Extend RTC to hundredths of a second precision.

 Ken W Sarkies 2014-2015

A timer ISR is reset when the RTC seconds count changes, to give a higher time precision.
*/

int tick = 0;

void setup(){

cli();//stop interrupts

//set timer0 interrupt at 2kHz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  
  // set compare match register for 500Hz increments
  OCR0A = 124;      // = (16*10^6) / (500*256) - 1  for 2kHz interrupts with prescale 256 (must be <256)
  
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  
  // Set CS02 bits for 256 prescaler
  TCCR0B |= (1 << CS02);   
  
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

sei();//allow interrupts
Serial.begin(9600);
}

ISR(Timer0_COMPA_vect)   {    // timer0 compare interrupt service routine
tick++ ;
}

void loop() {
 Serial.println (tick, DEC);
 delay(10000);
}
