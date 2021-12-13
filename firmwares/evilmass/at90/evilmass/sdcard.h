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
 *  Header file for SDCardManager.c.
 */

#ifndef _SD_MANAGER_H
#define _SD_MANAGER_H

/* Includes: */
#include <avr/io.h>

#include "evilmass.h"
#include "descriptors.h"

#include <LUFA/Common/Common.h>
#include <LUFA/Drivers/USB/USB.h>

/* Defines: */
/** Block size of the device. This is kept at 512 to remain compatible with the OS despite the underlying
 *  storage media (Dataflash) using a different native block size. Do not change this value.
 */
#define VIRTUAL_MEMORY_BLOCK_SIZE           512

#define LUN_MEDIA_BLOCKS           (sdcard_GetNbBlocks() / TOTAL_LUNS)


void delete_stages_files(void);
/* Function Prototypes: */
bool sdcard_Init(void);
uint32_t sdcard_GetNbBlocks(void);
void sdcard_WriteBlocks(USB_ClassInfo_MS_Device_t* MSInterfaceInfo,
                        const uint32_t BlockAddress, uint16_t TotalBlocks);
void sdcard_ReadBlocks(USB_ClassInfo_MS_Device_t* MSInterfaceInfo,
                       uint32_t BlockAddress, uint16_t TotalBlocks);
void sdcard_WriteBlocks_RAM(const uint32_t BlockAddress, uint16_t TotalBlocks,
                            uint8_t* BufferPtr) ATTR_NON_NULL_PTR_ARG(3);
void SDCardManagerManager_ReadBlocks_RAM(const uint32_t BlockAddress,
        uint16_t TotalBlocks,
        uint8_t* BufferPtr) ATTR_NON_NULL_PTR_ARG(3);
void sdcard_ResetDataflashProtections(void);
bool sdcard_CheckDataflashOperation(void);

#endif
