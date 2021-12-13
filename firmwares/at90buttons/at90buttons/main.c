#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))

int main(int argc, char **argv)
{
    MCUSR &= ~_BV(WDRF);
    wdt_disable();
    
    CLKPR = (1<<CLKPCE);
    CLKPR = 0;
    
    set_bit(DDRD, DDD6);
    clear_bit(PORTD, PD6);
    set_bit(DDRD, DDD7);
    clear_bit(PORTD, PD7);

    clear_bit(DDRD, DDD5);
    set_bit(PORTD, PD5);
    clear_bit(DDRE, DDE2);
    set_bit(PORTE, PE2);
    

    while (1)
    {
        if (PINE & _BV(PINE2))
        {
            clear_bit(PORTD, PD6);
        }
        else
        {
            set_bit(PORTD, PD6);
        }
        
        if (PIND & _BV(PIND5))
        {
            clear_bit(PORTD, PD7);
        }
        else
        {
            set_bit(PORTD, PD7);
        }
    }
    
    return 0;
}
