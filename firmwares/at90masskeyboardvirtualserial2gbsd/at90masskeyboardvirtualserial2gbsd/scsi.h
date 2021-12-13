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
 *  Header file for SCSI.c.
 */

#ifndef _SCSI_H_
#define _SCSI_H_

	/* Includes: */
		#include <avr/io.h>
		#include <avr/pgmspace.h>

		#include <LUFA/Drivers/USB/USB.h>

		#include "mass_storage_keyboard_virtualserial.h"
		#include "descriptors.h"
		#include "sdcard.h"
		#include "Config/AppConfig.h"

	/* Macros: */
		/** Macro to set the current SCSI sense data to the given key, additional sense code and additional sense qualifier. This
		 *  is for convenience, as it allows for all three sense values (returned upon request to the host to give information about
		 *  the last command failure) in a quick and easy manner.
		 *
		 *  \param[in] Key    New SCSI sense key to set the sense code to
		 *  \param[in] Acode  New SCSI additional sense key to set the additional sense code to
		 *  \param[in] Aqual  New SCSI additional sense key qualifier to set the additional sense qualifier code to
		 */
		#define SCSI_SET_SENSE(Key, Acode, Aqual)  do { SenseData.SenseKey                 = (Key);   \
		                                                SenseData.AdditionalSenseCode      = (Acode); \
		                                                SenseData.AdditionalSenseQualifier = (Aqual); } while (0)

		/** Macro for the \ref SCSI_Command_ReadWrite_10() function, to indicate that data is to be read from the storage medium. */
		#define DATA_READ           true

		/** Macro for the \ref SCSI_Command_ReadWrite_10() function, to indicate that data is to be written to the storage medium. */
		#define DATA_WRITE          false

		/** Value for the DeviceType entry in the SCSI_Inquiry_Response_t enum, indicating a Block Media device. */
		#define DEVICE_TYPE_BLOCK   0x00

		/** Value for the DeviceType entry in the SCSI_Inquiry_Response_t enum, indicating a CD-ROM device. */
		#define DEVICE_TYPE_CDROM   0x05

	/* Function Prototypes: */
		bool SCSI_DecodeSCSICommand(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);

		#if defined(INCLUDE_FROM_SCSI_C)
			static bool SCSI_Command_Inquiry(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);
			static bool SCSI_Command_Request_Sense(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);
			static bool SCSI_Command_Read_Capacity_10(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);
			static bool SCSI_Command_Send_Diagnostic(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);
			static bool SCSI_Command_ReadWrite_10(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo,
			                                      const bool IsDataRead);
			static bool SCSI_Command_ModeSense_6(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);
		#endif

#endif

