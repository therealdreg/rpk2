#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))


#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)


#define XMEM_START ((void *)0x2200)
#define XMEM_END ((void *)0xFFFF)


void init_uart(void)
{
	UCSR1B |= (1 << TXEN1) | (1 << RXEN1);
	UBRR1L = BAUD_PRESCALE;
	UBRR1H = (BAUD_PRESCALE >> 8);
	UCSR1C = (0<<UMSEL11)|(0<<UMSEL10)|(0<<UPM11)|(0<<UPM10)|
	(0<<USBS1)|(0<<UCSZ12)|(1<<UCSZ11)|(1<<UCSZ10);
}


void uart_putc(uint8_t c)
{
	while( !(UCSR1A & (1 << UDRE1)) );
	UDR1 = c;
}


uint8_t uart_readchar(bool eco)
{
    uint8_t c;
    
    while(!(UCSR1A & (1<<RXC1)));
    
    c = UDR1;
    if (eco)
    {
        uart_putc(c);
    }
    
    return c;
}


void uart_printhexchar(uint8_t charprint)
{
    const char hex[] = "0123456789ABCDEF";

    uart_putc(hex[(charprint>>4)&0xF]);
    uart_putc(hex[(charprint)&0xF]);
}

void uart_puts(uint8_t* s)
{
	static uint8_t last = 0;
	
	while(*s)
	{
		if ((*s == '\n') && (last != '\r'))
		{
			uart_putc('\r');
			uart_putc('\n');
		}
		else
		{
			uart_putc(*s);
		}
		last = *s;
		s++;
	}
}

void uart_putsnl(uint8_t* s)
{
    uart_puts(s);
    uart_puts("\r\n");
}


void uart_printoa(uint32_t number, int base)
{
    char buffer[50] = { 0 };
    
    itoa(number, buffer, base);
    
    uart_puts(buffer);
}


void uart_printdecint32(uint32_t number)
{
    uart_printoa(number, 10);
}



void set_bank(uint8_t bank) 
{
    PORTF = (PORTF & 0xF8) | (bank & 0x7);
}

void zeroed_bank(void)
{
    char* charptr;

    uart_putsnl("setting all to zero....");
    charptr = (char*) XMEM_START;
    while (charptr != 0)
    {
        *charptr = 0;
        charptr++;
    }
}

bool test_zero_bank(void)
{
    uart_putsnl("checking if all bank is zero....");
    uint8_t* charptr = (char*) XMEM_START;
    while (charptr != 0)
    {
        if (0 != *charptr)
        {
            uart_putsnl("error charptr is not zero....");
            uart_puts("byte readed: ");
            uart_printhexchar(*charptr);
            uart_puts("\r\n");
            return false;
        }
        charptr++;
    }
    uart_putsnl("ok all XRAM is zero....");
    
    return true;
}

void zero_banks(void)
{
    uart_putsnl("setting to zero all xmem banks....");
    for (uint8_t bank = 0; bank < 8; bank++)
    {
        uart_puts("\r\nzeroing bank: ");
        uart_printdecint32(bank);
        uart_puts("\r\n");
        set_bank(bank);
        zeroed_bank();
    }
}


bool test_bank(void)
{
    char* charptr;
    uint8_t last_char;
    
    if (!test_zero_bank())
    {
        return false;
    }
 
    uart_putsnl("setting all xmem to pattern starting from 0x69....");
    last_char = 0x68;
    charptr = (char*) XMEM_START;
    while (charptr != 0)
    {
        *charptr = last_char + 1;
        last_char++;
        charptr++;
    }

    uart_putsnl("checking if the pattern is valid....");
    charptr = (char*) XMEM_START;
    last_char = 0x69;
    while (charptr != 0)
    {
        if (last_char != *charptr)
        {
            uart_putsnl("error charptr invalid pattern....");
            uart_puts("byte readed: ");
            uart_printhexchar(*charptr);
            uart_puts("\r\n");
            return false;
        }
        last_char++;
        charptr++;
    }
    uart_putsnl("ok all XRAM pattern valid....");
    
    return true;
}

#define FIRST_BYTE_PATT 0x03
#define END_BYTE_PATT 0xF0
bool test_all_banks(void)
{
    uint8_t pattern_byte[8] = { 0 };
    
    zero_banks();
    uart_putsnl("\r\ntesting all xmem banks....");
    for (uint8_t bank = 0; bank < 8; bank++) 
    {
        uart_puts("\r\ntesting bank: ");
        uart_printdecint32(bank);
        uart_puts("\r\n");
        set_bank(bank);
        if (!test_bank())
        {
            return false;
        }
    }    
    
    uart_putsnl("\r\ntesting all banks is zero....\r\n");
    zero_banks();
    for (uint8_t bank = 0; bank < 8; bank++)
    {
        uart_puts("\r\ntesting bank: ");
        uart_printdecint32(bank);
        uart_puts("\r\n");
        set_bank(bank);
        if (!test_zero_bank())
        {
            return false;
        }
    }
    
    uint8_t* charptr = (char*) XMEM_START;
    uint8_t last_char = FIRST_BYTE_PATT;
    uart_putsnl("\r\ncreate pattern in all banks");
    for (uint8_t bank = 0; bank < 8; bank++)
    {
        uart_puts("pattern bank: ");
        uart_printdecint32(bank);
        uart_puts(" - first byte: ");
        uart_printhexchar(last_char);
        uart_puts("\r\n");
        set_bank(bank);
        charptr = (char*) XMEM_START;
        pattern_byte[bank] = last_char;
        while (charptr != 0)
        {
            *charptr = last_char;
            charptr++;
            if (last_char++ == END_BYTE_PATT)
            {
                last_char = FIRST_BYTE_PATT;
            }               
        }
    }    
    
    uart_putsnl("\r\ncheck if all start bank pattern is unique");
    for (uint8_t i = 0; i < 8; i++)
    {
        uart_puts("\r\n");
        for (uint8_t j = 0; j < 8; j++)
        {
            if (i != j)
            {
                uart_printhexchar(pattern_byte[i]);
                uart_puts(" == ");
                uart_printhexchar(pattern_byte[j]);
                uart_putsnl("?");
                
                if (pattern_byte[i] == pattern_byte[j])
                {
                    uart_putsnl("error! pattern byte repeated, use others FIRST_BYTE_PATT END_BYTE_PATT");   
                    return false;
                }
            }
        }
    }
    uart_putsnl("\r\nok all start bank pattern is unique");

    charptr = (char*) XMEM_START;
    last_char = FIRST_BYTE_PATT;
    uart_putsnl("\r\ntesting pattern in all banks");
    for (uint8_t bank = 0; bank < 8; bank++)
    {
        uart_puts("\r\npattern bank: ");
        uart_printdecint32(bank);
        uart_puts("\r\n");
        set_bank(bank);
        charptr = (char*) XMEM_START;
        while (charptr != 0)
        {
            if (*charptr != last_char)
            {
                uart_putsnl("error charptr invalid pattern....");
                return false;
            }
            if (charptr == XMEM_START)
            {
                uart_puts("ok, first byte is: ");
                uart_printhexchar(*charptr);
                uart_puts(" - testing the others....\r\n");
            }
            charptr++;
            if (last_char++ == END_BYTE_PATT)
            {
                last_char = FIRST_BYTE_PATT;
            }
        }
        uart_putsnl("ok all XRAM pattern valid....");
    }
    
    return true;
}

void init_xmem(void)
{
    set_bit(DDRF, DDF0);
    set_bit(DDRF, DDF1);
    set_bit(DDRF, DDF2);
    set_bit(DDRF, DDF3);
    
    set_bit(PORTF, PF0);
    set_bit(PORTF, PF1);
    set_bit(PORTF, PF2);
    clear_bit(PORTF, PF3);
    
    XMCRB=0; // need all 64K. no pins released
    XMCRA=1<<SRE; // enable xmem, no wait states
}

int main(void)
{
    MCUSR &= ~_BV(WDRF);
    wdt_disable();
      
    CLKPR = (1<<CLKPCE);
    CLKPR = 0;
      
    init_uart();
    
    set_bit(DDRD, DDD7);
    set_bit(DDRD, DDD6);
    
    init_xmem();
    
    bool retf = true;
    for (int i = 0; i < 3 && retf; i++)
    {
        retf = test_all_banks();
        
        uart_puts("\r\nend test loop: ");
        uart_printdecint32(i + 1);
                
        if (retf)
        {
            uart_putsnl("\r\n\r\nSUCCESS!! ALL XRAM BANKS are working well\r\n");
        }
        else
        {
            uart_putsnl("\r\n\r\nERROR !! - XRAM BANKS are not working well\r\n");
        }
        uart_putsnl("----------\r\n");
    }

    while (1) 
    {
        toogle_bit(PORTD, PD7);
        _delay_ms(1000);
        toogle_bit(PORTD, PD6);
        _delay_ms(1000);
    }
}

