#define LED1 8
#define LED2 9

#define USER_BUTTON 12

#include "Keyboard.h"

#include <avr/io.h>
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))


void initVariant(void) 
{
  //modified by Dreg
  asm volatile ("sbi 0x10,0");
  asm volatile ("cbi 0x11,0");
  asm volatile ("sbi 0x10,1");
  asm volatile ("sbi 0x11,1");
  // -
}

void switch_usb_to_at90(void)
{  
  delay(4000);

  set_bit(DDRD, DDD7);
  delay(500);
  clear_bit(PORTD, PD7);
  delay(500);
  clear_bit(PORTF, PF1);
  delay(500);
  set_bit(DDRF, DDF0);
  delay(500);
  set_bit(PORTF, PF0);
  delay(500);
  set_bit(DDRF, DDF1);
  delay(500);
  set_bit(PORTF, PF1);
  delay(500);
}

void setup() {
  pinMode(USER_BUTTON, INPUT_PULLUP);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  
  Serial.begin(9600);
  Keyboard.begin();
  delay(1000);

} 

void(* self_reset) (void) = 0;

void loop() {
  if (digitalRead(USER_BUTTON) == LOW)
  {
      Serial.print("\r\nbutton pressed! typing keyboard....\r\n");
      Keyboard.print("hellow dreg!");
      Serial.print("\r\nSwitching USB to AT90, next wait aprox 30 secs and SWITCH to LEO again, please wait...\r\n");
      switch_usb_to_at90();
      delay(20000);
      set_bit(DDRD, DDD7);
      set_bit(PORTD, PD7);
      delay(5000);
      initVariant();
  }
  // put your main code here, to run repeatedly:
  Serial.print("\r\nkeep pressing LEO user button a few secs to type in keyboard + AT90 usb switch, or press reset button to reboot\r\n");
  digitalWrite(LED1, HIGH);
  delay(500);
  digitalWrite(LED2, HIGH);
  delay(500);
  digitalWrite(LED1, LOW);
  delay(500);
  digitalWrite(LED2, LOW);
  delay(500);

}
