#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))



/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>

/*End of auto generated code by Atmel studio */

#include <AltSoftSerial.h>
//Beginning of Auto generated function prototypes by Atmel Studio
//End of Auto generated function prototypes by Atmel Studio



AltSoftSerial altSerial;

void setup() {
  altSerial.begin(9600);
  altSerial.println("Hello World");
}

uint32_t i = 0;
void loop() {
  altSerial.print("loop ");
  altSerial.println(i++);
  if (altSerial.available()) {
    altSerial.println();
    altSerial.println("-");
    char c = altSerial.read();
    altSerial.println(c);
    altSerial.println("-");
  }
  _delay_ms(100);
}
