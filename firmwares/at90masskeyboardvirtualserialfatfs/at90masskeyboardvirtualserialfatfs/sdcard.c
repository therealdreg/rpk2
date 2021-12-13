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

#include "ff.h"
#include "diskio.h"


/*--------------------------------------------------------------------*/
/* User Provided Timer Function for FatFs module                      */
/*--------------------------------------------------------------------*/
/* This is a real time clock service to be called from FatFs module.  */
/* Any valid time must be returned even if the system does not        */
/* support a real time clock. This is not required when FatFs is      */
/* configured for FF_FS_READONLY or FF_FS_NORTC = 1.                  */
/*--------------------------------------------------------------------*/

#include "rtc.h"

//BYTE Buff[4096];	/* Working buffer */

FATFS FatFs[1];		/* Filesystem object for each logical drive */
FIL File[1];		/* File object */

BYTE RtcOk;				/* RTC is available */

DWORD get_fattime (void)
{
    RTC rtc;


    if (!RtcOk) return 0;

    /* Get local time */
    rtc_gettime(&rtc);

    /* Pack date and time into a DWORD variable */
    return	  ((DWORD)(rtc.year - 1980) << 25)
    | ((DWORD)rtc.month << 21)
    | ((DWORD)rtc.mday << 16)
    | ((DWORD)rtc.hour << 11)
    | ((DWORD)rtc.min << 5)
    | ((DWORD)rtc.sec >> 1);
}



bool sdcard_create_sdload_times_file(void)
{
   #define FILE_NAM_TEMPLATE "DRG_X%02X.TXT"
    char file_name[120] = { 0 };
    FATFS fs;
    char line[120] = { 0 };
    FIL file1;
    UINT bw;
    FRESULT fr;
    bool retf = false;
   
   printf("mounting fs\r\n");
    fr = f_mount(&fs, "", 0);
    if (fr == FR_OK)
    {
        printf("mount fs success \r\n");
        
        sprintf(file_name, FILE_NAM_TEMPLATE, eeprom_read_byte((uint8_t*)0x69));
        printf("deleting last load file: %s\r\n", file_name);
        f_unlink(file_name);
               
        uint8_t new_id = eeprom_read_byte((uint8_t*)0x69) + 3;
        memset(file_name, 0, sizeof(file_name));
        
        sprintf(file_name, FILE_NAM_TEMPLATE, new_id);
        sprintf(line, "sd load nr times: 0x%02X\r\n", new_id);
        eeprom_write_byte((uint8_t*)0x69, new_id);
        
        printf("opening: %s\r\n", file_name);
        fr = f_open(&file1, file_name, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
        if (fr == FR_OK)
        {
            printf("file open success \r\n");
            printf("disk mounted and file opened \r\n");
            printf("writing: %s\r\n", line);
            f_write(&file1, line, strlen(line), &bw);
            if (fr == FR_OK)
            {
                printf("file write success \r\n");
                f_close(&file1);
                
                printf("openning the file again for reading...\r\n");
                fr = f_open(&file1, file_name, FA_READ);
                if (fr == FR_OK)
                {
                    printf("reading content of the file\r\n");
                    fr = f_read(&file1, line, strlen(line), &bw);
                    if (fr == FR_OK)
                    {
                        printf("content:\r\n%s\r\n", line);
                        
                        printf("\r\nsuccess!! FatFs is working good\r\n");
                        retf = true;
                    }               
                    f_close(&file1);
                }       
                if (!retf)
                {
                    printf("\r\nERROR!!\r\n");
                }                  
            }
            else
            {
                printf("file write error \r\n");
            }
        }
        else
        {
            printf("file open error \r\n");
        }
    }
    else
    {
        printf("mount error \r\n");
    }
    
    return retf;
}

bool sdcard_Init(void)
{
	printf_P(PSTR("\r\n\r\n"));
	do
	{
    	printf_P(PSTR("trying ... disk_initialize\r\n"));
	} while(disk_initialize(0) != FR_OK);

	printf_P(PSTR("disk_initialize success\r\n"));
    
    sdcard_GetNbBlocks();
        
    uint8_t readed_sector[512] = { 0 };
    SD_read_sector(0, readed_sector);
    printf_P(PSTR("first SD sector:\r\n"));
    uart_printhexdump(readed_sector, 512);
    printf_P(PSTR("\r\n\r\n"));
    
    sdcard_create_sdload_times_file();
    
    return true;
}

uint32_t sdcard_GetNbBlocks(void)
{
    static uint32_t CachedTotalBlocks = 0;
	
	DWORD ioct_rsp = 0;
	
	if (CachedTotalBlocks != 0)
    {
	    return CachedTotalBlocks;
    }        
	
	disk_ioctl(0, GET_SECTOR_COUNT, &ioct_rsp);

	CachedTotalBlocks = ioct_rsp;
	printf_P(PSTR("SD blocks: %li\r\n"), CachedTotalBlocks);
	
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
		  return 0;
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

void sdcard_WriteBlocks(USB_ClassInfo_MS_Device_t* MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks)
{
	int i;
	static uint8_t buff_to_write[512];
	
	printf_P(PSTR("W %li %i\r\n"), BlockAddress, TotalBlocks);

	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
	  return;
	
	while (TotalBlocks)
	{
		for (i = 0; i < 32; i++)
		{
			sdcard_WriteBlockHandler(buff_to_write + (16 * i), NULL, NULL);	
		}
		
        disk_write(0, buff_to_write, BlockAddress, 1);
				
		/* Check if the current command is being aborted by the host */
		if (MSInterfaceInfo->State.IsMassStoreReset)
		  return;
			
		/* Decrement the blocks remaining counter and reset the sub block counter */
		BlockAddress++;
		TotalBlocks--;
	}

	/* If the endpoint is empty, clear it ready for the next packet from the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	  Endpoint_ClearOUT();
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
		  return 0;
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

void sdcard_ReadBlocks(USB_ClassInfo_MS_Device_t* MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks)
{
	static uint8_t buff_rd[512];
	int i;
	
	printf_P(PSTR("R %li %i\r\n"), BlockAddress, TotalBlocks);
	
	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
	  return;
      
	while (TotalBlocks)
	{
        disk_read(0, buff_rd, BlockAddress, 1);
		for (i = 0; i < 32; i++)
		{
			sdcard_ReadBlockHandler(buff_rd + (16 * i), NULL, NULL);	
		}
		/* Decrement the blocks remaining counter */
		BlockAddress++;
		TotalBlocks--;
	}
    
	/* If the endpoint is full, send its contents to the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	  Endpoint_ClearIN();
}

/** Performs a simple test on the attached Dataflash IC(s) to ensure that they are working.
 *
 *  \return Boolean true if all media chips are working, false otherwise
 */
bool sdcard_CheckDataflashOperation(void)
{	
	return true;
}
