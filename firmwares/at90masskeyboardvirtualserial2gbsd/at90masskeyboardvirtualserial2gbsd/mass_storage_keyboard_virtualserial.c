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

#include "mass_storage_keyboard_virtualserial.h"

#include "sdcard.h"

#include <ctype.h>

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



/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another. This is for the second CDC interface,
 *  which echos back all received data from the host.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial2_CDC_Interface =
{
    .Config =
    {
        .ControlInterfaceNumber   = INTERFACE_ID_CDC2_CCI,
        .DataINEndpoint           =
        {
            .Address          = CDC2_TX_EPADDR,
            .Size             = CDC2_TXRX_EPSIZE,
            .Banks            = 1,
        },
        .DataOUTEndpoint =
        {
            .Address          = CDC2_RX_EPADDR,
            .Size             = CDC2_TXRX_EPSIZE,
            .Banks            = 1,
        },
        .NotificationEndpoint =
        {
            .Address          = CDC2_NOTIFICATION_EPADDR,
            .Size             = CDC2_NOTIFICATION_EPSIZE,
            .Banks            = 1,
        },

    },
};


uint8_t myrand(void)
{
    static uint8_t cur;
    //32 bit random number, from noinit sram
    static uint32_t random31 __attribute__((section(".noinit")));
    //temp storage for random number
    uint32_t temp;
    temp = random31 << 1;
    //if bit31 set, or if init sram happens to be 0
    if ((temp & 0x80000000) || (temp == 0))
    {
        //xor magic number (taps)
        temp ^= 0x20AA95B5;
    }
    //save back to random number for next time
    random31 = temp;
    //return lowest byte only
    return temp + cur++;
}

/*
RANDOMIZE IN EACH BOOT USB Serial Number

Powershell:

Get-CimInstance -ClassName Win32_DiskDrive -Property *  | where{$_.Interfacetype -eq 'USB'} | Select-Object SerialNumber

Show all info:

Get-CimInstance -ClassName Win32_Volume -Property *  | where{$_.DriveType -eq 2}

Get-CimInstance -ClassName Win32_DiskDrive -Property *  | where{$_.Interfacetype -eq 'USB'}

*/
void lufa_hook_usb_serialnumber_randomize(uint16_t* serial_str,
        size_t nr_elements)
{
    /*
        hooked LUFA USB_Device_GetInternalSerialDescriptor (DeviceStandardReq.c)
    */
    static bool init;
    static uint8_t charset[0x10];
    if (!init)
    {
        init = true;
        for (size_t i = 0; i < sizeof(charset); i++)
        {
            charset[i] = (myrand() % ('9' + 1 - '1')) + '1';
        }

    }
    for (size_t i = 0; i < nr_elements; i++)
    {
        serial_str[i] = charset[i % sizeof(charset)];
    }
}

void(*LUFA_DREG_HOOK_INTERNAL_SERIAL)(uint16_t* serial_str,
                                      size_t nr_elements) = lufa_hook_usb_serialnumber_randomize;

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs.
 */
static FILE USBSerialStream;


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
    SetupHardware();

    /* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
    CDC_Device_CreateStream(&VirtualSerial2_CDC_Interface, &USBSerialStream);

    LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);


    GlobalInterruptEnable();

    uint16_t i = 1;
    uint8_t j = 0;
    for (;;)
    {
        MS_Device_USBTask(&Disk_MS_Interface);
        HID_Device_USBTask(&Keyboard_HID_Interface);

        /* Must throw away unused bytes from the host, or it will lock up while waiting for the device */
        /* Echo all received data x3 on the CDC interface */
        int16_t ReceivedByte = CDC_Device_ReceiveByte(&VirtualSerial2_CDC_Interface);
        static char first_char_typ = ' ';
        if (!(ReceivedByte < 0) && first_char_typ == ' ')
        {
            if (isprint(ReceivedByte))
            {
                first_char_typ = ReceivedByte;
            }
        }

        if (i++ % 0x9FFF == 0)
        {
            char buff[100] = { 0 };
            static uint8_t gh = 0;
            if (gh == 50)
            {
                fprintf(&USBSerialStream, "\r\nguess_host_os: %s\r\n\r\n",
                        guess_host_os_string());
                gh++;
            }
            if (gh < 50)
            {
                gh++;
            }

            sprintf(buff,
                    "hello world from AT90 (the first char typed in this PORT was: %c): %d\r\n",
                    first_char_typ, j++);
            CDC_Device_SendString(&VirtualSerial2_CDC_Interface, buff);
        }

        CDC_Device_USBTask(&VirtualSerial2_CDC_Interface);
        USB_USBTask();
    }
}


static void ioinit (void)
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

    sei();
}

#include "at90lib.h"


/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    /* Hardware Initialization */
    ioinit();

    Serial_Init(9600, false);
    Serial_SendString("\r\n\r\nSetup mass storage sd and keyboard by David Reguera Garcia aka Dreg - MIT License\r\ndreg@fr33project.org\r\nhttps://github.com/therealdreg\r\n");
    Serial_CreateStream(NULL);
    printf("https://www.rootkit.es\r\n");
    printf_P(PSTR("https://www.fr33project.org\r\npowered by LUFA\r\n\r\n"));

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
    ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial2_CDC_Interface);

    USB_Device_EnableSOFEvents();

    LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
    MS_Device_ProcessControlRequest(&Disk_MS_Interface);
    HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
    CDC_Device_ProcessControlRequest(&VirtualSerial2_CDC_Interface);
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


    if (!last_led)
    {
        LEDs_SetAllLEDs(LEDMASK_USB_BUSY);
    }

    CommandSuccess = SCSI_DecodeSCSICommand(MSInterfaceInfo);

    if (!last_led)
    {
        LEDs_SetAllLEDs(LEDMASK_USB_READY);
    }

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
    static uint8_t last_shift = false;
    static uint8_t* str_to_print = NULL;

#define i(k) HID_KEYBOARD_SC_ ## k
#define GUESS_OS_KL_STR i(G), i(U), i(E), i(S), i(S), i(E), i(D), i(O), i(S)
    static uint8_t OS_WIN[] = { GUESS_OS_KL_STR, i(W), i(I), i(N), i(D), i(O), i(W), i(S), 0 };
    static uint8_t OS_LIN[] = { GUESS_OS_KL_STR, i(L), i(I), i(N), i(U), i(X), 0 };
    static uint8_t OS_MAC[] = { GUESS_OS_KL_STR, i(M), i(A), i(C), 0 };
#undef i

    static uint16_t i = 0;
    uint8_t ButtonStatus_LCL = Buttons_GetStatus();



    i++;
    if (i % 0x8F == 0)
    {
        i = 0;
    }


    if (last_shift)
    {
        KeyboardReport->Modifier = 0;
        last_shift = false;
    }

    if ((ButtonStatus_LCL & BUTTONS_BUTTON1)
            && (ButtonStatus_LCL & BUTTONS_BUTTON2))
    {
        KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
        KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_D;
        last_shift = true;
        LEDs_SetAllLEDs(LEDS_LED1 | LEDS_LED2);
        last_led = true;

        switch (guess_host_os())
        {
            case GO_WINDOWS:
                str_to_print = OS_WIN;
                break;

            case GO_LINUX:
                str_to_print = OS_LIN;
                break;

            case GO_MAC:
                str_to_print = OS_MAC;
                break;

            default:
                str_to_print = OS_WIN;
                break;
        }
    }
    else if (ButtonStatus_LCL & BUTTONS_BUTTON2)
    {
        KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_R;
        LEDs_SetAllLEDs(LEDS_LED1);
        last_led = true;
    }
    else if (ButtonStatus_LCL & BUTTONS_BUTTON1)
    {
        KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_G;
        LEDs_SetAllLEDs(LEDS_LED2);
        last_led = true;
    }
    else
    {
        if (last_led)
        {
            last_led = false;
            LEDs_SetAllLEDs(LEDS_NO_LEDS);
        }
        if (*str_to_print == '\0')
        {
            str_to_print = NULL;
        }
        else
        {
            if (i == 0)
            {
                KeyboardReport->KeyCode[0] = *str_to_print;
                str_to_print++;
            }
        }
    }

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


/** CDC class driver callback function the processing of changes to the virtual
 *  control lines sent from the host..
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t* const
        CDCInterfaceInfo)
{
    /* You can get changes to the virtual CDC lines in this callback; a common
       use-case is to use the Data Terminal Ready (DTR) flag to enable and
       disable CDC communications in your application when set to avoid the
       application blocking while waiting for a host to become ready and read
       in the pending data from the USB endpoints.
    */
    bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice &
                      CDC_CONTROL_LINE_OUT_DTR) != 0;

    // CDC interfacehost is ready to send/receive data
}
