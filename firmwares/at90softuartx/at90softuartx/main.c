
#define F_CPU 16000000UL

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))

#define SOFTBAUD 38400

//atmega328p, PD6
static void txHigh  ()
{
    set_bit(PORTB, PB5);
}
static void txLow   ()
{
    clear_bit(PORTB, PB5);
}
static void txIdle  ()
{
    set_bit(DDRB, DDB5);    //output, high
}


static int txPut(char c, FILE* f)
{

    uint8_t sreg = SREG;

    asm("cli");
    txLow(); //start 1 bit low
    __builtin_avr_delay_cycles( F_CPU / SOFTBAUD - 3 ); //tweak as needed
    for ( uint8_t i = 0; i < 8; i++, c >>= 1 )
    {
        if ( c & 1 )
        {
            txHigh();
        }
        else
        {
            txLow();
        }
        __builtin_avr_delay_cycles( F_CPU / SOFTBAUD - 8 );
    }
    txHigh(); //stop 2 bits

    SREG = sreg; //timing no longer important
    __builtin_avr_delay_cycles( F_CPU / SOFTBAUD *
                                2 ); //not important if takes longer
    return 0;
}

static FILE soft_uart = FDEV_SETUP_STREAM(txPut, NULL, _FDEV_SETUP_WRITE);



int main(void)
{
    MCUSR &= ~_BV(WDRF);
    wdt_disable();

    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    txIdle();
    fprintf(&soft_uart, "\r\n\r\nsoftuart demo\r\n\r\n");
    _delay_ms(2000);

    stdout = &soft_uart;

    while (1)
    {
        printf("hi Dreg\r\n");
    }
}

