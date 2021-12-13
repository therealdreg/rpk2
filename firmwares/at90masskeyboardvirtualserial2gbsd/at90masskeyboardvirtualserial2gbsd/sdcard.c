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
 *  Functions to manage the physical dataflash media, including reading and writing of
 *  blocks of data. These functions are called by the SCSI layer when data must be stored
 *  or retrieved to/from the physical storage media. If a different media is used (such
 *  as a SD card or EEPROM), functions similar to these will need to be generated.
 */

#define  INCLUDE_FROM_sdcard_C

#include "at90lib.h"
#include "sdcard.h"


bool sdcard_Init(void)
{
    uint8_t ret;

    SPI_init();
    do
    {
        ret = SD_init();
        if (ret)
        {
            uart_puts("SD err: ");
            uart_printhexchar(ret);
            return false;
        }
    }   while (ret);

    uart_puts("SD init sucesss!\r\n");

    sdcard_GetNbBlocks();

    uint8_t readed_sector[512] = { 0 };
    SD_read_sector(0, readed_sector);
    printf_P(PSTR("first SD sector:\r\n"));
    uart_printhexdump(readed_sector, 512);
    printf_P(PSTR("\r\n\r\n"));

    return true;
}

uint32_t sdcard_GetNbBlocks(void)
{
    static uint32_t CachedTotalBlocks = 0;

    if (CachedTotalBlocks != 0)
    {
        return CachedTotalBlocks;
    }

    if (!sd_get_number_of_sectors(&CachedTotalBlocks))
    {
        uart_puts("\r\nERROR sd_get_number_of_sectors fails, forcing 2GB\r\n");
        CachedTotalBlocks = 3921920;  //2 GB
    }

    printf_P(PSTR("\r\nSD blocks: %li\r\n"), CachedTotalBlocks);

    return CachedTotalBlocks;
}

/** Writes blocks (OS blocks, not Dataflash pages) to the storage medium, the board dataflash IC(s), from
 *  the pre-selected data OUT endpoint. This routine reads in OS sized blocks from the endpoint and writes
 *  them to the dataflash in Dataflash page sized blocks.
 *
 *  \param[in] BlockAddress  Data block starting address for the write sequence
 *  \param[in] TotalBlocks   Number of blocks of data to write
 */
uintptr_t sdcard_WriteBlockHandler(uint8_t* buff_to_write, void* a, void* p)
{
    /* Check if the endpoint is currently empty */
    if (!(Endpoint_IsReadWriteAllowed()))
    {
        /* Clear the current endpoint bank */
        Endpoint_ClearOUT();

        /* Wait until the host has sent another packet */
        if (Endpoint_WaitUntilReady())
        {
            return 0;
        }
    }

    /* Write one 16-byte chunk of data to the dataflash */
    buff_to_write[0] = Endpoint_Read_8();
    buff_to_write[1] = Endpoint_Read_8();
    buff_to_write[2] = Endpoint_Read_8();
    buff_to_write[3] = Endpoint_Read_8();
    buff_to_write[4] = Endpoint_Read_8();
    buff_to_write[5] = Endpoint_Read_8();
    buff_to_write[6] = Endpoint_Read_8();
    buff_to_write[7] = Endpoint_Read_8();
    buff_to_write[8] = Endpoint_Read_8();
    buff_to_write[9] = Endpoint_Read_8();
    buff_to_write[10] = Endpoint_Read_8();
    buff_to_write[11] = Endpoint_Read_8();
    buff_to_write[12] = Endpoint_Read_8();
    buff_to_write[13] = Endpoint_Read_8();
    buff_to_write[14] = Endpoint_Read_8();
    buff_to_write[15] = Endpoint_Read_8();

    return 16;
}

void sdcard_WriteBlocks(USB_ClassInfo_MS_Device_t* MSInterfaceInfo,
                        uint32_t BlockAddress, uint16_t TotalBlocks)
{
    int i;
    static uint8_t buff_to_write[512];

    printf_P(PSTR("W %li %i\r\n"), BlockAddress, TotalBlocks);

    /* Wait until endpoint is ready before continuing */
    if (Endpoint_WaitUntilReady())
    {
        return;
    }

    while (TotalBlocks)
    {
        for (i = 0; i < 32; i++)
        {
            sdcard_WriteBlockHandler(buff_to_write + (16 * i), NULL, NULL);
        }

        SD_write_sector(BlockAddress, buff_to_write);

        /* Check if the current command is being aborted by the host */
        if (MSInterfaceInfo->State.IsMassStoreReset)
        {
            return;
        }

        /* Decrement the blocks remaining counter and reset the sub block counter */
        BlockAddress++;
        TotalBlocks--;
    }

    /* If the endpoint is empty, clear it ready for the next packet from the host */
    if (!(Endpoint_IsReadWriteAllowed()))
    {
        Endpoint_ClearOUT();
    }
}

/** Reads blocks (OS blocks, not Dataflash pages) from the storage medium, the board dataflash IC(s), into
 *  the pre-selected data IN endpoint. This routine reads in Dataflash page sized blocks from the Dataflash
 *  and writes them in OS sized blocks to the endpoint.
 *
 *  \param[in] BlockAddress  Data block starting address for the read sequence
 *  \param[in] TotalBlocks   Number of blocks of data to read
 */

static inline uint8_t sdcard_ReadBlockHandler(uint8_t* buffer, void* a, void* p)
{
    /* Check if the endpoint is currently full */
    if (!(Endpoint_IsReadWriteAllowed()))
    {
        /* Clear the endpoint bank to send its contents to the host */
        Endpoint_ClearIN();

        /* Wait until the endpoint is ready for more data */
        if (Endpoint_WaitUntilReady())
        {
            return 0;
        }
    }

    Endpoint_Write_8(buffer[0]);
    Endpoint_Write_8(buffer[1]);
    Endpoint_Write_8(buffer[2]);
    Endpoint_Write_8(buffer[3]);
    Endpoint_Write_8(buffer[4]);
    Endpoint_Write_8(buffer[5]);
    Endpoint_Write_8(buffer[6]);
    Endpoint_Write_8(buffer[7]);
    Endpoint_Write_8(buffer[8]);
    Endpoint_Write_8(buffer[9]);
    Endpoint_Write_8(buffer[10]);
    Endpoint_Write_8(buffer[11]);
    Endpoint_Write_8(buffer[12]);
    Endpoint_Write_8(buffer[13]);
    Endpoint_Write_8(buffer[14]);
    Endpoint_Write_8(buffer[15]);

    return 1;
}

void sdcard_ReadBlocks(USB_ClassInfo_MS_Device_t* MSInterfaceInfo,
                       uint32_t BlockAddress, uint16_t TotalBlocks)
{
    static uint8_t buff_rd[512];
    int i;

    printf_P(PSTR("R %li %i\r\n"), BlockAddress, TotalBlocks);

    /* Wait until endpoint is ready before continuing */
    if (Endpoint_WaitUntilReady())
    {
        return;
    }

    //#define MULTIPLE_READ_SD

#ifdef MULTIPLE_READ_SD
    sd_raw_initmultiread(BlockAddress);
#endif
    while (TotalBlocks)
    {
#ifdef MULTIPLE_READ_SD
        read_multi_buff(buff_rd);
#else
        SD_read_sector(BlockAddress, buff_rd);
#endif
        //uart_printhexdump(buff_rd, 512);
        for (i = 0; i < 32; i++)
        {
            sdcard_ReadBlockHandler(buff_rd + (16 * i), NULL, NULL);
        }
        /* Decrement the blocks remaining counter */
        BlockAddress++;
        TotalBlocks--;
    }
#ifdef MULTIPLE_READ_SD
    sd_raw_stopmultiread();
#endif

    /* If the endpoint is full, send its contents to the host */
    if (!(Endpoint_IsReadWriteAllowed()))
    {
        Endpoint_ClearIN();
    }
}

/** Performs a simple test on the attached Dataflash IC(s) to ensure that they are working.
 *
 *  \return Boolean true if all media chips are working, false otherwise
 */
bool sdcard_CheckDataflashOperation(void)
{
    return true;
}
