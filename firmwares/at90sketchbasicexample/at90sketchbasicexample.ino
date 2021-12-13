/*
  Compile with Arduino 1.8.16 + Teensyduino Version 1.55 https://www.pjrc.com/teensy/td_download.html
  
  Tools --> Board --> Teensyduino --> Teensy++ 2.0
  USB Type: Serial
  CPU SPEED: 16mhz
*/

#define LED1 6
#define LED2 7

#include <avr/io.h>
#include <avr/wdt.h>
int InitAT90(void)
{
    MCUSR &= ~(1<<WDRF);
    wdt_disable();
    CLKPR = (1<<CLKPCE);
    CLKPR = 0;
    return 1;
}
void initVariant(void) 
{
  InitAT90();
  delay(3000);
}

void setup() {
  InitAT90();
  Serial.begin(9600);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
}

int i = 0;
// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED1, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for half second
  digitalWrite(LED2, HIGH); 
  delay(500);  
  digitalWrite(LED2, LOW);    // turn the LED off by making the voltage LOW
  delay(500);                       
  digitalWrite(LED1, LOW);
  delay(500);
  Serial.print("hello dreg: ");
  Serial.println(i++);
}
