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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <ctype.h>

FILE soft_uart;

FILE soft_uart_echo;

rx_cb_t rx_cb;

volatile uint8_t sRxDone;               /* Ready to read Rx byte */

static volatile uint8_t sTxCount;
static volatile uint8_t sTxData;
static volatile uint8_t sRxData;
static volatile uint8_t sRxMask;
static volatile uint8_t sRxTemp;

/*****************************************************************************/
/* TIMER1 Input Capture Interrupt Handler                                    */
/* When Rx pin detects start bit, interrupt handler is called                */
/* When interrupt handler is called, TCNT1 value is written to ICR1 register */
/*****************************************************************************/

#ifdef SWUSE_TIMER1
ISR(TIMER1_CAPT_vect)
{
    /* Set OCR1B to read Rx */
    /* Output Compare B interrupt will occurs 1.5 BIT_TIME counter(time) later */
    OCR1B = ICR1 + (uint16_t)(BIT_TIME * 1.5);
    /* Clear temporary variable */
    sRxTemp = 0;
    /* Set bit mask to read first bit */
    sRxMask = 1;
    /* Clear Output Compare B interrupt flag */
    TIFR1 = _BV(OCF1B);

    /* If RX pin is low, It means start bit */
    if (!(SWU_PIN & _BV(SWU_RX)))
    {
        /* Disable input capture(Detecting start bit), Enable Output Compare A(for Tx) / B(for Rx) interrupt */
        TIMSK1 = _BV(OCIE1A) | _BV(OCIE1B);
    }
}
#else //TIMER3:
ISR(TIMER3_CAPT_vect)
{
    /* Set OCR3B to read Rx */
    /* Output Compare B interrupt will occurs 1.5 BIT_TIME counter(time) later */
    OCR3B = ICR3 + (uint16_t)(BIT_TIME * 1.5);
    /* Clear temporary variable */
    sRxTemp = 0;
    /* Set bit mask to read first bit */
    sRxMask = 1;
    /* Clear Output Compare B interrupt flag */
    TIFR3 = _BV(OCF3B);

    /* If RX pin is low, It means start bit */
    if (!(SWU_PIN & _BV(SWU_RX)))
    {
        /* Disable input capture(Detecting start bit), Enable Output Compare A(for Tx) / B(for Rx) interrupt */
        TIMSK3 = _BV(OCIE3A) | _BV(OCIE3B);
    }
}
#endif /* SWUSE_TIMER1 */


/*****************************************************************************/
/* TIMER1 Output Compare B Interrupt Handler                                 */
/* This handler is enabled to read data when system detects start bit        */
/* sRxMask is true while system reads 8 bits from Rx pin                     */
/*****************************************************************************/
#ifdef SWUSE_TIMER1
ISR(TIMER1_COMPB_vect)
{
    if (sRxMask)
    {
        if (SWU_PIN & _BV(SWU_RX))
        {
            sRxTemp |= sRxMask;    /* if Rx input is high, write 1 to bit */
        }
        sRxMask <<= 1;     /* Shift 1 bit */
        OCR1B += BIT_TIME;  /* Set OCR1B to read next bit */
    }
    else
    {
        if (NULL != rx_cb)
        {
            rx_cb(sRxTemp);
        }
        else
        {
            sRxDone = 1;          /* Reading 8 bits is done */
            sRxData = sRxTemp;   /* Store Rx data */
        }
        TIFR1 = _BV(ICF1);    /* Clear input capture interrupt flag to read new byte */
        TIMSK1 = _BV(ICIE1) | _BV(OCIE1A);  /* Enable input capture + COMPA interrupt */
    }
}
#else // TIMER3:
ISR(TIMER3_COMPB_vect)
{
    if (sRxMask)
    {
        if (SWU_PIN & _BV(SWU_RX))
        {
            sRxTemp |= sRxMask;    /* if Rx input is high, write 1 to bit */
        }
        sRxMask <<= 1;     /* Shift 1 bit */
        OCR3B += BIT_TIME;  /* Set OCR3B to read next bit */
    }
    else
    {
        if (NULL != rx_cb)
        {
            rx_cb(sRxTemp);
        }
        else
        {
            sRxDone = 1;          /* Reading 8 bits is done */
            sRxData = sRxTemp;   /* Store Rx data */
        }
        TIFR3 = _BV(ICF3);    /* Clear input capture interrupt flag to read new byte */
        TIMSK3 = _BV(ICIE3) | _BV(OCIE3A);  /* Enable input capture + COMPA interrupt */
    }
}
#endif /* SWUSE_TIMER1 */



/*****************************************************************************/
/* TIMER1 Output Compare A Interrupt Handler                                 */
/* This handler is enabled to send data                                      */
/* Be careful, sTxData is inverted data                                      */
/*****************************************************************************/
#ifdef SWUSE_TIMER1
ISR(TIMER1_COMPA_vect)
{
    uint8_t OutputMode;

    OCR1A += BIT_TIME;                              /* Set OCR1A to send next bit */

    if (sTxCount)                                   /* If there are bits to send */
    {
        sTxCount--;                                 /* Decrease counter */
        OutputMode = _BV(COM1A1);                   /* Set output as low */

        if (sTxCount != 9)                          /* If it's data bit */
        {
            if (!(sTxData & 0x01))
            {
                /* If bit is low, set output high (sTxData is inverted) */
                OutputMode = _BV(COM1A1) | _BV(COM1A0);
            }
            sTxData >>= 1;
        }

        TCCR1A = OutputMode;
    }
}
#else // TIMER3:
ISR(TIMER3_COMPA_vect)
{
    uint8_t OutputMode;

    OCR3A += BIT_TIME;                              /* Set OCR3A to send next bit */

    if (sTxCount)                                   /* If there are bits to send */
    {
        sTxCount--;                                 /* Decrease counter */
        OutputMode = _BV(COM3A1);                   /* Set output as low */

        if (sTxCount != 9)                          /* If it's data bit */
        {
            if (!(sTxData & 0x01))
            {
                /* If bit is low, set output high (sTxData is inverted) */
                OutputMode = _BV(COM3A1) | _BV(COM3A0);
            }
            sTxData >>= 1;
        }

        TCCR3A = OutputMode;
    }
}
#endif /* SWUSE_TIMER1 */

/*****************************************************************************/
/* Software UART Initialization                                          */
/* Enable Output Compare A interrupt to send data                            */
/* Enable Input Capture interrupt to receive data                            */
/*****************************************************************************/
void SWU_Initialize(void)
{
#ifndef DISABLE_FDEV_SETP
    fdev_setup_stream(&soft_uart, SWU_TxByte, SWU_RxByte, _FDEV_SETUP_RW);
    fdev_setup_stream(&soft_uart_echo, SWU_TxByte, SWU_RxByte_echo, _FDEV_SETUP_RW);
#endif /* DISABLE_FDEV_SETP */

#ifdef SWUSE_TIMER1
    /* Output Compare A interrupt is called immediately after initialization */
    OCR1A = TCNT1 + 1;
    /* Set output mode, set OC1A high(Idle), Timer1 mode 0 */
    TCCR1A = _BV(COM1A1) | _BV(COM1A0);
    /* Input capture noise canceler, falling edge detection, no prescaling */
    TCCR1B = _BV(ICNC1) | _BV(CS10);
    /* Clear capture flag */
    TIFR1 = _BV(ICF1);
    /* Enable input capture interrupt(Detect start bit) + Output comapre A match interrupt(Tx) */
    TIMSK1 = _BV(ICIE1) | _BV(OCIE1A);

    sTxCount = 0;                           /* No Tx Data */
    sRxDone = 0;                            /* No Rx Data */
    SWU_DDR |= _BV(SWU_TX);                 /* Set TX pin as output */

    sei();                                  /* Enable global interrupt */
#else // TIMER3
    /* Output Compare A interrupt is called immediately after initialization */
    OCR3A = TCNT3 + 1;
    /* Set output mode, set OC3A high(Idle), Timer3 mode 0 */
    TCCR3A = _BV(COM3A1) | _BV(COM3A0);
    /* Input capture noise canceler, falling edge detection, no prescaling */
    TCCR3B = _BV(ICNC3) | _BV(CS30);
    /* Clear capture flag */
    TIFR3 = _BV(ICF3);
    /* Enable input capture interrupt(Detect start bit) + Output comapre A match interrupt(Tx) */
    TIMSK3 = _BV(ICIE3) | _BV(OCIE3A);

    sTxCount = 0;                           /* No Tx Data */
    sRxDone = 0;                            /* No Rx Data */
    SWU_DDR |= _BV(SWU_TX);                 /* Set TX pin as output */

    sei();
#endif /* SWUSE_TIMER1 */
}

/*****************************************************************************/
/* Read Byte                                                                 */
/*****************************************************************************/
int SWU_RxByte(FILE* f)
{
    while ( !sRxDone );

    sRxDone = 0;

    return sRxData;
}

void SWU_RegisterRxCb(rx_cb_t cb)
{
    rx_cb = cb;
}

int SWU_RxByte_echo(FILE* f)
{
    int byte;

    byte = SWU_RxByte(NULL);
    SWU_TxByte((char) byte, NULL);

    return byte;
}

int SWU_RxByte_echo_isprint(FILE* f)
{
    int byte;

    byte = SWU_RxByte(NULL);
    if (isprint(byte))
    {
        SWU_TxByte((char) byte, NULL);
    }

    return byte;
}

int SWU_RxByte_echo_isprint_nl(FILE* f)
{
    int byte;

    byte = SWU_RxByte_echo_isprint(NULL);
    if (byte == '\r')
    {
        SWU_TxByte((char) '\r', NULL);
        SWU_TxByte((char) '\n', NULL);
    }

    return byte;
}

/*****************************************************************************/
/* Write Byte                                                                */
/*****************************************************************************/
int SWU_TxByte(char data, FILE* f)
{
    while (sTxCount);

    sTxData = ~data;            /* !!! Write inverted data for stop bit generation */
    sTxCount = 10;              /* Start(1) + data(8) + Stop(1) */

    return 0;
}

/*****************************************************************************/
/* Read Bytes                                                                */
/*****************************************************************************/
void SWU_RxBuffer(uint8_t* buffer, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        *buffer = SWU_RxByte(NULL);
        buffer++;
    }
}

void SWU_RxBuffer_echo(uint8_t* buffer, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        *buffer = SWU_RxByte_echo(NULL);
        buffer++;
    }
}

/*****************************************************************************/
/* Write Bytes                                                               */
/*****************************************************************************/
void SWU_TxBuffer(uint8_t* buffer, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        SWU_TxByte(*buffer, NULL);
        buffer++;
    }
}