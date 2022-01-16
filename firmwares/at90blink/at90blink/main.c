#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/sfr_defs.h>
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))

int main(int argc, char** argv)
{
    MCUSR &= ~_BV(WDRF);
    wdt_disable();

    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    set_bit(DDRE, DDE6);
    clear_bit(PORTE, PE6);
    set_bit(DDRE, DDE7);
    clear_bit(PORTE, PE7);

    set_bit(DDRD, DDD6);
    clear_bit(PORTD, PD6);
    set_bit(DDRD, DDD7);
    clear_bit(PORTD, PD7);

    while (1)
    {
        set_bit(PORTD, PD6);
        _delay_ms(1000);
        set_bit(PORTD, PD7);
        _delay_ms(1000);
        clear_bit(PORTD, PD6);
        clear_bit(PORTD, PD7);
        _delay_ms(200);
        set_bit(PORTD, PD7);
        _delay_ms(1000);
        clear_bit(PORTD, PD6);
        clear_bit(PORTD, PD7);
        _delay_ms(1000);

        set_bit(PORTE, PE6);
        _delay_ms(1000);
        set_bit(PORTE, PE7);
        _delay_ms(1000);
        clear_bit(PORTE, PE6);
        clear_bit(PORTE, PE7);
        _delay_ms(200);
        set_bit(PORTE, PE7);
        _delay_ms(1000);
        clear_bit(PORTE, PE6);
        clear_bit(PORTE, PE7);
        _delay_ms(1000);
    }

    return 0;
}
