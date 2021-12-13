#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))


/*
// https://my.eng.utah.edu/~cs5789/2010/slides/Interruptsx2.pdf
// https://www.instructables.com/Arduino-Timer-Interrupts/
// https://www.arduinoslovakia.eu/application/timer-calculator
// http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
// TIMER3:
TCCR3A = 0;
TCCR3B = 0;
TCNT3 = 0;
OCR3A = 624; // 100 Hz (16000000/((624+1)*256))
TCCR3B |= (1 << WGM32); // CTC
TCCR3B |= (1 << CS32); // Prescaler 256
TIMSK3 |= (1 << OCIE3A); // Output Compare Match A Interrupt Enable

// TIMER1:
TCCR1A = 0;
TCCR1B = 0;
TCNT1 = 0;
OCR1A = 624; // 100 Hz (16000000/((624+1)*256))
TCCR1B |= (1 << WGM12); // CTC
TCCR1B |= (1 << CS12); // Prescaler 256
TIMSK1 |= (1 << OCIE1A);  // Output Compare Match A Interrupt Enable

*/

#define USE_TIMER3_FATFS 1

int main(int argc, char** argv)
{
    MCUSR &= ~_BV(WDRF);
    wdt_disable();

    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    set_bit(DDRD, DDD3);
    clear_bit(PORTD, PD3);
    set_bit(DDRD, DDD7);
    clear_bit(PORTD, PD7);

// 100hz timer (every 10ms)
#ifdef USE_TIMER3_FATFS
    // TIMER3
    TCCR3A = 0;
    TCCR3B = 0;
    TCCR3B |= (1 << WGM32);   //CTC mode; clock timer comare
    TIMSK3 |= (1 << OCIE3A); //ENABLE INTERUPTs
    OCR3A = 0x4E20;
    TCCR3B |=  (1 << CS31); //PRESCALER
#else
    //TIMER1
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12);   //CTC mode; clock timer comare
    TIMSK1 |= (1 << OCIE1A); //ENABLE INTERUPTs
    OCR1A = 0x4E20;
    TCCR1B |=  (1 << CS11); //PRESCALER
#endif

    sei();

    while (1)
    {

        toogle_bit(PORTD, PD7);
        _delay_ms(1000);
    }

    return 0;
}


#ifdef USE_TIMER3_FATFS
// TIMER3
ISR(TIMER3_COMPA_vect)
{
    toogle_bit(PORTD, PD3);
}
#else
//TIMER1
ISR(TIMER1_COMPA_vect)
{
    toogle_bit(PORTD, PD3);
}
#endif
