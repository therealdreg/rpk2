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
 *  Header file for Descriptors.c.
 */

#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

/* Includes: */
#include <avr/pgmspace.h>

#include <LUFA/Drivers/USB/USB.h>

#include "Config/AppConfig.h"

/* Macros: */
/** Endpoint address of the Keyboard HID reporting IN endpoint. */
#define KEYBOARD_EPADDR              (ENDPOINT_DIR_IN  | 1)

/** Size in bytes of the Keyboard HID reporting IN endpoint. */
#define KEYBOARD_EPSIZE              8

/** Endpoint address of the Mass Storage device-to-host data IN endpoint. */
#define MASS_STORAGE_IN_EPADDR       (ENDPOINT_DIR_IN  | 3)

/** Endpoint address of the Mass Storage host-to-device data OUT endpoint. */
#define MASS_STORAGE_OUT_EPADDR      (ENDPOINT_DIR_OUT | 4)

/** Size in bytes of the Mass Storage data endpoints. */
#define MASS_STORAGE_IO_EPSIZE       64

/** Endpoint address of the second CDC interface's device-to-host data IN endpoint. */
#define CDC2_TX_EPADDR                 (ENDPOINT_DIR_IN  | 5)

/** Endpoint address of the second CDC interface's host-to-device data OUT endpoint. */
#define CDC2_RX_EPADDR                 (ENDPOINT_DIR_OUT | 6)

/** Endpoint address of the second CDC interface's device-to-host notification IN endpoint. */
#define CDC2_NOTIFICATION_EPADDR       (ENDPOINT_DIR_IN  | 7)

/** Size in bytes of the CDC device-to-host notification IN endpoints. */
#define CDC2_NOTIFICATION_EPSIZE        8

/** Size in bytes of the CDC data IN and OUT endpoints. */
#define CDC2_TXRX_EPSIZE                16

/* Type Defines: */
/** Type define for the device configuration descriptor structure. This must be defined in the
 *  application code, as the configuration descriptor contains several sub-descriptors which
 *  vary between devices, and which describe the device's usage to the host.
 */
typedef struct
{
    USB_Descriptor_Configuration_Header_t Config;

    // Mass Storage Interface
    USB_Descriptor_Interface_t            MS_Interface;
    USB_Descriptor_Endpoint_t             MS_DataInEndpoint;
    USB_Descriptor_Endpoint_t             MS_DataOutEndpoint;

    // Generic HID Interface
    USB_Descriptor_Interface_t            HID_KeyboardInterface;
    USB_HID_Descriptor_HID_t              HID_KeyboardHID;
    USB_Descriptor_Endpoint_t             HID_ReportINEndpoint;

    // CDC Control Interface
    USB_Descriptor_Interface_Association_t   CDC2_IAD;
    USB_Descriptor_Interface_t               CDC2_CCI_Interface;
    USB_CDC_Descriptor_FunctionalHeader_t    CDC2_Functional_Header;
    USB_CDC_Descriptor_FunctionalACM_t       CDC2_Functional_ACM;
    USB_CDC_Descriptor_FunctionalUnion_t     CDC2_Functional_Union;
    USB_Descriptor_Endpoint_t                CDC2_ManagementEndpoint;

    // CDC Data Interface
    USB_Descriptor_Interface_t               CDC2_DCI_Interface;
    USB_Descriptor_Endpoint_t                CDC2_DataOutEndpoint;
    USB_Descriptor_Endpoint_t                CDC2_DataInEndpoint;
} USB_Descriptor_Configuration_t;

/** Enum for the device interface descriptor IDs within the device. Each interface descriptor
 *  should have a unique ID index associated with it, which can be used to refer to the
 *  interface from other descriptors.
 */
enum InterfaceDescriptors_t
{
    INTERFACE_ID_MassStorage = 0, /**< Mass storage interface descriptor ID */
    INTERFACE_ID_Keyboard    = 1, /**< Keyboard interface descriptor ID */
    INTERFACE_ID_CDC2_CCI    = 2, /**< CDC2 CCI interface descriptor ID */
    INTERFACE_ID_CDC2_DCI    = 3, /**< CDC2 DCI interface descriptor ID */
};

/** Enum for the device string descriptor IDs within the device. Each string descriptor should
 *  have a unique ID index associated with it, which can be used to refer to the string from
 *  other descriptors.
 */
enum StringDescriptors_t
{
    STRING_ID_Language     = 0, /**< Supported Languages string descriptor ID (must be zero) */
    STRING_ID_Manufacturer = 1, /**< Manufacturer string ID */
    STRING_ID_Product      = 2, /**< Product string ID */
};

// Based from https://github.com/keyboardio/FingerprintUSBHost/blob/master/src/FingerprintUSBHost.cpp
typedef enum guess_os_e
{
    GO_WINDOWS = 0,
    GO_LINUX,
    GO_MAC
} guess_os_t;

typedef struct guess_s
{
    uint8_t maybe_linux: 1;
    uint8_t maybe_win: 1;
    uint8_t maybe_mac: 1;
    uint8_t not_linux: 1;
    uint8_t not_win: 1;
    uint8_t not_mac: 1;
} guess_t;

extern guess_t guess_os;

guess_os_t guess_host_os(void);

const char* guess_host_os_string(void);

/* Function Prototypes: */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint16_t wIndex,
                                    const void** const DescriptorAddress)
ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif

