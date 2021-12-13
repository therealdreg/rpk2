/*
MIT License - Copyright 2021
-
David Reguera Garcia aka Dreg - dreg@fr33project.org
-
http://www.rootkit.es/
http://github.com/David-Reguera-Garcia-Dreg/therealdreg
http://www.fr33project.org/
-
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

WARNING! this is a POC, the code is CRAP
*/

#include "at90lib.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <ctype.h>

#define CS (1<<PB0)
#define MOSI (1<<PB2)
#define MISO (1<<PB3)
#define SCK (1<<PB1)
#define CS_DDR DDRB
#define CS_ENABLE() (PORTB &= ~CS)
#define CS_DISABLE() (PORTB |= CS)

#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))

void on_sd(void)
{
    set_bit(DDRB, DDB6);
    clear_bit(PORTB, PB6);
}

void off_sd(void)
{
    clear_bit(DDRB, DDB6);
}

void init_uart(void)
{
    UCSR1B |= (1 << TXEN1) | (1 << RXEN1);
    UBRR1L = BAUD_PRESCALE;
    UBRR1H = (BAUD_PRESCALE >> 8);
    UCSR1C = (0 << UMSEL11) | (0 << UMSEL10) | (0 << UPM11) | (0 << UPM10) |
             (0 << USBS1) | (0 << UCSZ12) | (1 << UCSZ11) | (1 << UCSZ10);
}


void uart_putc(uint8_t c)
{
    while ( !(UCSR1A & (1 << UDRE1)) );
    UDR1 = c;
}


uint8_t uart_readchar(bool eco)
{
    uint8_t c;

    while (!(UCSR1A & (1 << RXC1)));

    c = UDR1;
    if (eco)
    {
        uart_putc(c);
    }

    return c;
}


void uart_puts(char* s)
{
    static uint8_t last = 0;

    while (*s)
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

void uart_putsnl(char* s)
{
    uart_puts(s);
    uart_puts("\r\n");
}

void uart_printhexchar(uint8_t charprint)
{
    const char hex[] = "0123456789ABCDEF";

    uart_putc(hex[(charprint >> 4) & 0xF]);
    uart_putc(hex[(charprint) & 0xF]);
}

void uart_printhexdump(uint8_t* buff, size_t size)
{
    uint8_t* aux_buff;

    if (size == 0)
    {
        return;
    }

    for (int i = 0; i < size; i++)
    {
        uart_printhexchar(*buff++);
        uart_putc(' ');
        if ((i + 1) % 16 == 0 || i == size - 1)
        {
            aux_buff = buff - 16;
            for (int j = 0; j < 16; j++)
            {
                if (isprint(*aux_buff))
                {
                    uart_putc(*aux_buff);
                }
                else
                {
                    uart_putc('.');
                }
                aux_buff++;
            }
            uart_puts("\r\n");
        }
    }
    uart_puts("\r\n");
}

void uart_printoa(uint32_t number, int base)
{
    char buffer[50] = { 0 };

    itoa(number, buffer, base);

    uart_puts(buffer);
}

void uart_printundecint32(uint32_t number)
{
    char buffer[50] = { 0 };

    ltoa(number, buffer, 10);

    uart_puts(buffer);
}

void uart_printdecint32(uint32_t number)
{
    uart_printoa(number, 10);
}

void uart_printhexint32(uint32_t number)
{
    uart_printoa(number, 16);
}

void uart_printbinint32(uint32_t number)
{
    uart_printoa(number, 2);
}


void uart_readbuff(uint8_t* buff, size_t size, bool eco)
{
    for (int i = 0; i < size; i++)
    {
        buff[i] = uart_readchar(eco);
    }
}

size_t uart_readbuff_until_newline(uint8_t* buff, size_t size, bool eco)
{
    uint8_t c = 0;
    size_t i = 0;

    if (0 == size)
    {
        return 0;
    }

    for (i = 0; i < size; i++)
    {
        c = uart_readchar(false);
        if ('\n' == c  || '\r' == c)
        {
            break;
        }
        if (eco)
        {
            uart_putc(c);
        }

        buff[i] = c;
    }

    if (eco)
    {
        uart_puts("\r\n");
    }


    return i;
}

void ignore_fuses(void)
{
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    CLKPR = (1 << CLKPCE);
    CLKPR = 0;
}

void ioinit (void)
{
    //pull up

    PORTA = 0b11111111;
    PORTB = 0b11111111;
    PORTC = 0b11111111;
    PORTD = 0b11111111;
    //PORTE = 0b11111111;
    PORTF = 0b11111111;
    //PORTG = 0b00011111;
}

void SPI_init(void)
{
    CS_DDR |= CS; // SD card circuit select as output
    DDRB |= MOSI + SCK; // MOSI and SCK as outputs
    PORTB |= MISO; // pullup in MISO, might not be needed
}

unsigned char SPI_write(uint8_t ch)
{
    SPDR = ch;
    while (!(SPSR & (1 << SPIF))) {}
    return SPDR;
}

void wait_sd_some_cycl(void)
{
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
}


bool read_spi_while_byte_is_not(uint8_t byte, uint32_t max_timeout)
{
    for (uint32_t i = 0; i < max_timeout; i++)
    {
        if (SPI_write(0xFF) == byte)
        {
            return true;
        }

        if ((i + 1) % 1000 == 0)
        {
            uart_putsnl("\r\nsd waiting for data...");
        }
    }

    uart_putsnl("\r\nERROR sd waiting for data timeout...");

    return false;
}

unsigned char SD_command(unsigned char cmd,
                         unsigned long arg,
                         unsigned char crc,
                         unsigned char read)
{
    unsigned char i;
    unsigned char buffer[32] = { 0 };
    unsigned char ret = 0xFF;

    wait_sd_some_cycl();

    uart_puts("CMD ");
    uart_printhexchar(cmd);
    uart_putc(' ');

    SPI_write(cmd);
    SPI_write(arg >> 24);
    SPI_write(arg >> 16);
    SPI_write(arg >> 8);
    SPI_write(arg);
    SPI_write(crc);

    for (i = 0; i < read; i++)
    {
        buffer[i] = SPI_write(0xFF);
    }

    for (i = 0; i < read; i++)
    {
        uart_printhexchar(buffer[i]);
        uart_putc(' ');
        if (buffer[i] != 0xFF)
        {
            ret = buffer[i];
        }
    }

    uart_puts("\r\n");

    return ret;
}

sdcard_ret_t SD_init(void)
{
    char i;

    SPI_SLOW();
    // ]r:10
    CS_DISABLE();
    for (i = 0; i < 10; i++) // idle for 1 bytes / 80 clocks
    {
        SPI_write(0xFF);
    }

    uart_putsnl("clearing sd...");
    CS_ENABLE();
    if (!read_spi_while_byte_is_not(0xFF, RECOMMEND_WAIT_TIMEOUT_SPI))
    {
        return SD_INIT_ERR_CANT_CLEAR;
    }
    CS_DISABLE();

    // [0x40 0x00 0x00 0x00 0x00 0x95 r:8] until we get "1"
    for (i = 0; i < 10; i++)
    {
        CS_ENABLE();
        if (SD_command(CMD0, 0x00000000, 0x95, 8) == 1)
        {
            break;
        }
        CS_DISABLE();
        _delay_ms(100);
    }
    if (i == 10) // card did not respond to initialization
    {
        return SD_INIT_ERR_DONT_RESPOND;
    }

    // CMD1 until card comes out of idle, but maximum of 10 times
    for (i = 0; i < 10; i++)
    {
        CS_ENABLE();
        if (SD_command(CMD1, 0x00000000, 0xFF, 8) == 0)
        {
            break;
        }
        CS_DISABLE();
        _delay_ms(100);
    }
    if (i == 10) // card did not come out of idle
    {
        return SD_INIT_ERR_DONT_OUT_IDLE;
    }

    // SET_BLOCKLEN to 512
    CS_ENABLE();
    SD_command(CMD16, 512, 0xFF, 8);
    CS_DISABLE();

    SPI_FAST();

    return SD_INIT_OK;
}

// TODO: This function will not exit gracefully if SD card does not do what it should
bool SD_read_sector(unsigned long sector, uint8_t* buffer)
{
    unsigned short i = 0;
    bool retf = false;

    CS_ENABLE();
    wait_sd_some_cycl();

    SPI_write(CMD17);
    SPI_write(sector >> 15); // sector*512 >> 24
    SPI_write(sector >> 7); // sector*512 >> 16
    SPI_write(sector << 1); // sector*512 >> 8
    SPI_write(0);          // sector*512
    SPI_write(0xFF);

    // wait for 0
    if (read_spi_while_byte_is_not(0, RECOMMEND_WAIT_TIMEOUT_SPI))
    {
        // wait for data Start Block token
        if (read_spi_while_byte_is_not(0xFE, RECOMMEND_WAIT_TIMEOUT_SPI))
        {
            for (i = 0; i < 512; i++)
            {
                buffer[i] = SPI_write(0xFF);
            }
            // skip checksum
            SPI_write(0xFF);
            SPI_write(0xFF);

            retf = true;
        }

    }

    CS_DISABLE();

    return retf;
}

bool SD_write_sector(unsigned long sector, uint8_t* buffer)
{
    unsigned short i = 0;
    bool retf = false;

    CS_ENABLE();
    wait_sd_some_cycl();

    SPI_write(CMD24);
    SPI_write(sector >> 15); // sector*512 >> 24
    SPI_write(sector >> 7); // sector*512 >> 16
    SPI_write(sector << 1); // sector*512 >> 8
    SPI_write(0);          // sector*512
    SPI_write(0xFF);

    // wait for 0
    if (read_spi_while_byte_is_not(0, RECOMMEND_WAIT_TIMEOUT_SPI))
    {
        // wait for 0xff
        if (read_spi_while_byte_is_not(0xFF, RECOMMEND_WAIT_TIMEOUT_SPI))
        {
            // SEND Start Block token
            SPI_write(0xFE);
            for (i = 0; i < 512; i++)
            {
                SPI_write(buffer[i]);
            }

            // skip checksum
            SPI_write(0);
            SPI_write(0);

            if (SPI_write(0) == CMD24_SUCCESS_BYTE)
            {
                // wait for idle
                if (read_spi_while_byte_is_not(0xFF, RECOMMEND_WAIT_TIMEOUT_SPI))
                {
                    retf = true;
                }
            }
            else
            {
                uart_putsnl("\r\nERROR! CMD24_SUCCESS_BYTE dont recved\r\n");
            }
        }
    }

    CS_DISABLE();

    return retf;
}

bool sd_get_number_of_sectors(uint32_t* number_of_sectors)
{
    uint32_t ret = 1;
    uint8_t csd_read_bl_len = 0;
    uint8_t csd_c_size_mult = 0;
    uint32_t csd_c_size = 0;

    CS_ENABLE();
    wait_sd_some_cycl();

    SPI_write(CMD9);
    SPI_write(0);
    SPI_write(0);
    SPI_write(0);
    SPI_write(0);

    if (read_spi_while_byte_is_not(0xFE, RECOMMEND_WAIT_TIMEOUT_SPI))
    {
        for (uint8_t i = 0; i < 18; i++)
        {
            uint8_t b = SPI_write(0xFF);

            switch (i)
            {
                case 5:
                    csd_read_bl_len = b & 0x0f;
                    break;

                case 6:
                    csd_c_size = b & 0x03;
                    csd_c_size <<= 8;
                    break;

                case 7:
                    csd_c_size |= b;
                    csd_c_size <<= 2;
                    break;

                case 8:
                    csd_c_size |= b >> 6;
                    csd_c_size++;
                    break;

                case 9:
                    csd_c_size_mult = b & 0x03;
                    csd_c_size_mult <<= 1;
                    break;

                case 10:
                    csd_c_size_mult |= b >> 7;
                    ret = (uint32_t) csd_c_size << (csd_c_size_mult + csd_read_bl_len + 2);
                    break;
            }
        }
    }

    CS_DISABLE();

    if (ret % 512 != 0)
    {
        uart_putsnl("\r\nERROR! number of bytes is not % 512\r\n");
        return false;
    }

    *number_of_sectors = ret / 512;

    return true;
}

void sd_raw_initmultiread(uint32_t sector)
{
    CS_ENABLE();
    wait_sd_some_cycl();

    SPI_write(CMD18);
    SPI_write(sector >> 15); // sector*512 >> 24
    SPI_write(sector >> 7); // sector*512 >> 16
    SPI_write(sector << 1); // sector*512 >> 8
    SPI_write(0);          // sector*512
    SPI_write(0xFF);

    read_spi_while_byte_is_not(0xFE, RECOMMEND_WAIT_TIMEOUT_SPI);
}

void read_multi_buff(uint8_t* buff)
{
    for (int j = 0; j < 512; j++)
    {
        buff[j] = SPI_write(0xFF);
    }
    read_spi_while_byte_is_not(0xFE, RECOMMEND_WAIT_TIMEOUT_SPI);
}

void sd_raw_stopmultiread(void)
{
    wait_sd_some_cycl();

    SPI_write(CMD12);
    SPI_write(0);
    SPI_write(0);
    SPI_write(0);
    SPI_write(0);

    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);

    read_spi_while_byte_is_not(0xFF, RECOMMEND_WAIT_TIMEOUT_SPI);

    CS_DISABLE();
}



