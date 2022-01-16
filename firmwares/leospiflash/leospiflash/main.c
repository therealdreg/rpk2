/*
MIT License - Copyright 2021
-
David Reguera Garcia aka Dreg - dreg@fr33project.org
-
http://www.rootkit.es/
http://github.com/David-Reguera-Garcia-Dreg/
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

Credits:
    A second UART in software
    P. Dannegger danni@specs.de
    https://community.atmel.com/projects/second-uart-software

    AVR_SWUART
    kiki (eziya)
    https://github.com/eziya/AVR_SWUART/blob/master/AVR_SWUART/328P_SWUART.c
    https://blog.naver.com/eziya76/221244587970
*/

#include "at90swuart.h"

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/eeprom.h>

#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))

#define W_EN    0x06    //write enable
#define W_DE    0x04    //write disable
#define R_SR1   0x05    //read status reg 1
#define R_SR2   0x35    //read status reg 2
#define W_SR    0x01    //write status reg
#define PAGE_PGM    0x02    //page program
#define QPAGE_PGM   0x32    //quad input page program
#define BLK_E_64K   0xD8    //block erase 64KB
#define BLK_E_32K   0x52    //block erase 32KB
#define SECTOR_E    0x20    //sector erase 4KB
#define CHIP_ERASE  0xc7    //chip erase
#define CHIP_ERASE2 0x60    //=CHIP_ERASE
#define E_SUSPEND   0x75    //erase suspend
#define E_RESUME    0x7a    //erase resume
#define PDWN        0xb9    //power down
#define HIGH_PERF_M 0xa3    //high performance mode
#define CONT_R_RST  0xff    //continuous read mode reset
#define RELEASE     0xab    //release power down or HPM/Dev ID (deprecated)
#define R_MANUF_ID  0x90    //read Manufacturer and Dev ID (deprecated)
#define R_UNIQUE_ID 0x4b    //read unique ID (suggested)
#define R_JEDEC_ID  0x9f    //read JEDEC ID = Manuf+ID (suggested)
#define READ        0x03
#define FAST_READ   0x0b

#define SR1_BUSY_MASK   0x01
#define SR1_WEN_MASK    0x02

#define WINBOND_MANUF   0xef

#define DEFAULT_TIMEOUT 200

#define W25Q512JVFIM_ID 0x7020


#define CS (1<<PB0)
#define MOSI (1<<PB2)
#define MISO (1<<PB3)
#define SCK (1<<PB1)
#define CS_DDR DDRB
#define CS_ENABLE() (PORTB &= ~CS)
#define CS_DISABLE() (PORTB |= CS)

#define SPI_SLOW() SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(0<<SPR0);
#define SPI_FAST() SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);


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

void ioinit (void)
{
    //pull up
    PORTB = 0b11111111;
    PORTC = 0b11111111;
    PORTD = 0b11111111;
    PORTF = 0b11111111;
}


#define MEM_STAT_WEN 0x02
#define MEM_STAT_BUSY 0x01


uint8_t getMemStatus()
{
    uint8_t out;
    CS_DISABLE();
    CS_ENABLE();

    SPI_write(0x05);
    out = SPI_write(0x00);
    CS_DISABLE();
    return out;
}




void memRead256ToStdOut(uint32_t address)
{
    uint8_t data[16];
    while ((getMemStatus() & MEM_STAT_BUSY)) {}

    CS_DISABLE();
    CS_ENABLE();
    SPI_write(READ);
    SPI_write(address >> 16);
    SPI_write(address >> 8);
    SPI_write(address);

    for (uint8_t i = 0; i < 16; i++)
    {
        for (uint8_t j = 0; j < 16; j++)
        {
            data[j] = SPI_write(0x00);
            printf("%02x ", data[j]);
        }

        printf("  ");

        for (uint8_t j = 0; j < 16; j++)
        {
            if (isprint(data[j]))
            {
                printf("%c", data[j]);
            }
            else
            {
                printf(".");
            }
        }
        printf("\r\n");
    }

    CS_DISABLE();
    return;
}

void memEnableWrite()
{
    printf("Enabling Write\r\n");
    CS_DISABLE();
    CS_ENABLE();

    SPI_write(0x06);

    while (!(getMemStatus() & MEM_STAT_WEN)) {}

    CS_DISABLE();
    return;
}

void memWrite256(uint32_t address, uint8_t* data)
{
    while ((getMemStatus() & MEM_STAT_BUSY)) {}

    memEnableWrite();

    CS_DISABLE();
    CS_ENABLE();

    SPI_write(0x02);
    SPI_write(address >> 16);
    SPI_write(address >> 8);
    SPI_write(address);


    for (uint16_t i = 0; i < 256; i++)
    {
        SPI_write(*data);
        data++;
    }

    CS_DISABLE();
    return;

}

void memEraseSector(uint32_t address)
{
    while ((getMemStatus() & MEM_STAT_BUSY)) {}

    memEnableWrite();



    CS_DISABLE();
    CS_ENABLE();

    SPI_write(0x20);
    SPI_write(address >> 16);
    SPI_write(address >> 8);
    SPI_write(address);


    CS_DISABLE();
    return;
}


int main(int argc, char** argv)
{
    MCUSR &= ~_BV(WDRF);
    wdt_disable();

    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    ioinit();

    SWU_Initialize();

    _delay_ms(1000);

    stdin = stdout = &soft_uart;

    printf("\r\n\r\nAT90SWUART version: %s\r\n\r\n", AT90SWUART_VER_STR);

    SPI_init();

    SPI_SLOW();

    CS_DISABLE();
    CS_ENABLE();
    SPI_write(RELEASE);
    CS_DISABLE();
    _delay_ms(500);

    uint8_t manuf;
    uint16_t id;

    CS_DISABLE();
    CS_ENABLE();
    SPI_write(R_JEDEC_ID);
    manuf = SPI_write(0x00);
    id = SPI_write(0x00) << 8;
    id |= SPI_write(0x00);
    CS_DISABLE();

    printf("\r\nMANUF=0x%X , ID=0x%X\r\n", manuf, id);
    if (manuf == WINBOND_MANUF)
    {
        printf("OK! WINBOND_MANUF\r\n");
        if (id == W25Q512JVFIM_ID)
        {
            printf("OK! W25Q512JVFIM\r\n");
        }
        else
        {
            printf("ERROR NOT W25Q512JVFIM DETECTED\r\n");
            return;
        }
    }
    else
    {
        printf("ERROR! WINBOND_MANUF\r\n");
        return;
    }
    printf("\r\n");

    printf("Status: 0x%02x\r\n", getMemStatus());

    printf("\r\n0 address content:\r\n");
    memRead256ToStdOut(0);

    printf("\r\nerasing 0 address content\r\n");
    memEraseSector(0);

    printf("\r\n0 address content:\r\n");
    memRead256ToStdOut(0);

    unsigned char buff[256] = { 0 };
    memset(buff, eeprom_read_byte((uint8_t*)0x69), sizeof(buff));
    eeprom_write_byte((uint8_t*)0x69, eeprom_read_byte((uint8_t*)0x69) + 1);
    strcpy(buff, "Hi Dreg!");
    printf("\r\nwriting '%s' to 0 address\r\n", buff);
    memWrite256(0, buff);

    printf("\r\n0 address content:\r\n");
    memRead256ToStdOut(0);

    printf("\r\n");

    printf("Status: 0x%02x\r\n", getMemStatus());

    while (1)
    {

        _delay_ms(1000);
    }

}


