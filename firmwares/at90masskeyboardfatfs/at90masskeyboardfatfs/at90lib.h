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

#ifndef AT90_H_
#define AT90_H_

#ifndef F_CPU
    #define F_CPU  16000000UL
#endif

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#define SPI_SLOW() SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(0<<SPR0);
#define SPI_FAST() SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);

#define RECOMMEND_WAIT_TIMEOUT_SPI 100000

#define CMD0    0x40 // card a reset
#define CMD1    0x41 // SEND_OPERATING_STATE
#define CMD9    0x49 // SEND_CSD
#define CMD12   0x4c // STOP_TRANSMISSION
#define CMD16   0x50 // SET_BLOCKLEN
#define CMD17   0x51 // READ_SINGLE_BLOCK
#define CMD18   0x52 // READ_MULTIPLE_BLOCK
#define CMD24   0x58 // WRITE_SINGLE_BLOCK


#define CMD24_SUCCESS_BYTE (0xE5)

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum sdcard_ret_e
{
    SD_INIT_OK = 0,
    SD_INIT_ERR_CANT_CLEAR,
    SD_INIT_ERR_DONT_RESPOND,
    SD_INIT_ERR_DONT_OUT_IDLE,
} sdcard_ret_t;

void on_sd(void);
void off_sd(void);
void init_uart(void);
void uart_putc(uint8_t c);
uint8_t uart_readchar(bool eco);
void uart_puts(char* s);
void uart_putsnl(char* s);
void uart_printhexchar(uint8_t charprint);
void uart_printhexdump(uint8_t* buff, size_t size);
void uart_printoa(uint32_t number, int base);
void uart_printundecint32(uint32_t number);
void uart_printdecint32(uint32_t number);
void uart_printhexint32(uint32_t number);
void uart_printbinint32(uint32_t number);
void uart_readbuff(uint8_t* buff, size_t size, bool eco);
size_t uart_readbuff_until_newline(uint8_t* buff, size_t size, bool eco);
void ioinit (void);
sdcard_ret_t SD_init(void);
void SPI_init(void);
unsigned char SPI_write(uint8_t ch);
unsigned char SD_command(unsigned char cmd, unsigned long arg,
                         unsigned char crc, unsigned char read);
bool read_spi_while_byte_is_not(uint8_t byte, uint32_t max_timeout);
bool SD_read_sector(unsigned long sector, uint8_t* buffer);
bool SD_write_sector(unsigned long sector, uint8_t* buffer);
bool sd_get_number_of_sectors(uint32_t* number_of_sectors);
void sd_raw_initmultiread(uint32_t sector);
void sd_raw_stopmultiread(void);
void read_multi_buff(uint8_t* buff);
void ignore_fuses(void);



#endif /* AT90_H_ */
