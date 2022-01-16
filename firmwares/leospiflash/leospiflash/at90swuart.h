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

#ifndef AT90SWUART_H_
#define AT90SWUART_H_

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define AT90SWUART_VER 2
#define AT90SWUART_VER_STR TOSTRING(AT90SWUART_VER)

//#define DISABLE_FDEV_SETP 1 // uncomment this line to disable fdev_setup_stream

//#define USE_ARDUINO_UNO  1 // uncomment this line to switch to atmega328p

// ---- modify this for your board:
#ifndef F_CPU
    #define F_CPU   16000000L
#endif

//#define SWUSE_TIMER1 1 // comment this line to use TIMER3

#ifdef SWUSE_TIMER1 // TIMER1
    #ifdef USE_ARDUINO_UNO // atmega328p
        #define SWU_DDR DDRB
        #define SWU_PORT PORTB
        #define SWU_PIN PINB
        #define SWU_RX PB0
        #define SWU_TX PB1
    #else // AT90USB128
        // (unusable PWM PB6, PB7)
        #define SWU_DDR     DDRB
        #define SWU_PORT    PORTB
        #define SWU_PIN     PIND
        #define SWU_RX      PD4
        #define SWU_TX      PB5
    #endif /* USE_ARDUINO_UNO */

#else // TIMER3:
    #ifdef USE_ARDUINO_UNO // atmega328p
        #error "Arduino UNO boards (atmega328p) dont have TIMER3"
    #endif /* USE_ARDUINO_UNO */

    // AT90USB128
    // (unusable PWM PC4, PC5)
    #define SWU_DDR     DDRC
    #define SWU_PORT    PORTC
    #define SWU_PIN     PINC
    #define SWU_RX      PC7
    #define SWU_TX      PC6
#endif /* SWUSE_TIMER1 */

#define BAUDRATE    9600
#define BIT_TIME    (uint16_t)(F_CPU / BAUDRATE + 0.5)  /* Clocks per one bit (rounded) */
// ----

#include <stdint.h>
#include <stdio.h>


extern volatile uint8_t sRxDone;

extern FILE soft_uart;

extern FILE soft_uart_echo;

typedef void (*rx_cb_t)(uint8_t byte);

void SWU_Initialize(void);
int SWU_RxByte(FILE* f);
void SWU_RegisterRxCb(rx_cb_t cb);
int SWU_RxByte_echo(FILE* f);
int SWU_RxByte_echo_isprint(FILE* f);
int SWU_RxByte_echo_isprint_nl(FILE* f);
int SWU_TxByte(char data, FILE* f);
void SWU_RxBuffer(uint8_t* buffer, uint8_t length);
void SWU_TxBuffer(uint8_t* buffer, uint8_t length);
void SWU_RxBuffer_echo(uint8_t* buffer, uint8_t length);

#endif /* AT90SWUART_H_ */