#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))

/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>

/*End of auto generated code by Atmel studio */

#include "Keyboard.h"
//Beginning of Auto generated function prototypes by Atmel Studio
//End of Auto generated function prototypes by Atmel Studio



void setup() {
    MCUSR &= ~_BV(WDRF);
    wdt_disable();
    
    set_bit(DDRF, DDF0);
    clear_bit(PORTF, PF0);
        
    set_bit(DDRF, DDF1);
    set_bit(PORTF, PF1);
        
    set_bit(DDRB, DDB4);
    clear_bit(PORTB, PB4);

    clear_bit(DDRD, DDD6);
    set_bit(PORTD, PD6);
    
    Keyboard.begin();
}

void loop() {
    if (PIND & _BV(PIND6))
    {
        clear_bit(PORTB, PB4);
    }
    else
    {
        Keyboard.print("hellow dreg");
        set_bit(PORTB, PB4);
        _delay_ms(200);
    }
}
