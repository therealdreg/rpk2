#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))

int main (void)
{

    set_bit(DDRD, DDD7);
    clear_bit(PORTD, PD7);
    
    set_bit(DDRF, DDF0);
    set_bit(PORTF, PF0);
    
    set_bit(DDRF, DDF1);
    set_bit(PORTF, PF1);
    
    set_bit(DDRB, DDB5);
    set_bit(PORTB, PB5);

    while(1)
    {
        
        toogle_bit(PORTB, PB5);
        _delay_ms(1000);
    }
}