#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))

int main(int argc, char** argv)
{
    MCUSR &= ~_BV(WDRF);
    wdt_disable();

    set_bit(DDRB, DDB5);
    clear_bit(PORTB, PB5);
    set_bit(DDRB, DDB4);
    clear_bit(PORTB, PB4);

    clear_bit(DDRD, DDD6);
    set_bit(PORTD, PD6);

    clear_bit(DDRE, DDE2);
    set_bit(PORTE, PE2);

    set_bit(DDRD, DDD1);
    clear_bit(PORTD, PD1);
    set_bit(DDRD, DDD5);
    clear_bit(PORTD, PD5);


    while (1)
    {
        if (!(PINE & _BV(PINE2)))
        {
            clear_bit(PORTB, PB4);
            clear_bit(PORTB, PB5);

            set_bit(PORTD, PD1);
            _delay_ms(1000);
            set_bit(PORTD, PD5);
            _delay_ms(1000);
            clear_bit(PORTD, PD1);
            clear_bit(PORTD, PD5);
            _delay_ms(200);
            set_bit(PORTD, PD5);
            _delay_ms(1000);

        }

        if (!(PIND & _BV(PIND6)))
        {
            clear_bit(PORTD, PD1);
            clear_bit(PORTD, PD5);

            set_bit(PORTB, PB4);
            _delay_ms(1000);
            set_bit(PORTB, PB5);
            _delay_ms(1000);
            clear_bit(PORTB, PB4);
            clear_bit(PORTB, PB5);
            _delay_ms(200);
            set_bit(PORTB, PB5);
            _delay_ms(1000);
        }
    }

    return 0;
}
