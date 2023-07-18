#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <Arduino.h>
#include "spiflash.h"

#define Serial Serial1

void test_size(void)
{
	/* Test GetPageSize */
	if (SPIF_get_page_size() == 256)
	{
		Serial.println("GetPageSize: Passed");
	} else
	{
		Serial.println("GetPageSize: Didn't pass");
	}
	
	/* Test GetSectorSize */
	if (SPIF_get_sector_size() == 4096){
		Serial.println("GetSectorSize: Passed");
	} else
	{
		Serial.println("GetSectorSize: Didn't pass");
	}
	
	/* Test GetFlashSize */
	uint32_t result_size = SPIF_get_size();
	uint32_t expected_size = 1024 * 1024 * 64 - 4096;
	if (SPIF_get_size() == 67104768 ){
		Serial.println("GetSize: Passed");
	} else
	{
		Serial.println("GetSize: Didn't pass");
	}
}

void test_full_erase_read(void)
{
	uint8_t test_passed = 0;
	uint8_t buff[256] = { 0 };
		
	/* Test FullErase-Read */
	SPIF_erase();
	SPIF_read(0, buff, 256);
	for (uint16_t i = 0; i < 256; i++)
	{
		if(buff[i] != 0xFF) test_passed = 1;
	}
	if (test_passed == 0){
		Serial.println("FullErase-Read: Passed");
	} else
	{
		Serial.println("FullErase-Read: Didn't pass");
	}
}

void test_fastwrite(void)
{
	uint16_t page_size = 256;
	uint8_t read_buff[page_size * 3] = { 0 };
	uint8_t write_buff[page_size * 3] = { 0 };
	uint8_t write_value = 0x69;
	memset(write_buff, write_value, page_size * 3);
	
	
	uint16_t sizes[4] = { 50, 256, 512, 600 };
		
	/* Write different sizes at different offsets */
	
	for (int i=0; i < 4; i++)
	{
		for (int j=0; i < 4; i++)
		{
				SPIF_4B_erase_sector(0);
				SPIF_fast_write(sizes[j], write_buff, sizes[i]);
				SPIF_read(sizes[j], read_buff, sizes[i]);
				if ( (read_buff[0] != write_value) || (read_buff[sizes[i] - 1] != write_value) )
				{
					Serial.println("Fastwrite: Didn't pass");
					return;
				}
				memset(read_buff, 0, page_size * 3);
		}
	}
	
	Serial.println("Fastwrite: Passed");
	return;
}

void test_sector_erase(void)
{
	uint16_t sector_size = SPIF_get_sector_size();
	uint8_t read_buff[512] = { 0 };
	uint8_t write_buff[512] = { 0 };
	uint8_t write_value = 0x69;
	memset(write_buff, write_value, 512);
	
	SPIF_4B_erase_sector(0);
	SPIF_4B_erase_sector(sector_size);
	
	SPIF_fast_write(sector_size - 256, write_buff, 512);
	SPIF_4B_erase_sector(0);
	SPIF_read(sector_size - 256, read_buff, 512);
	if ( (read_buff[0] != 0xFF) || (read_buff[255] != 0xFF) || (read_buff[256] != write_value) || (read_buff[511] != write_value))
	{
		Serial.println("Sector erase: Didn't pass");
		return;
	}

	Serial.println("Sector erase: Passed");
	return;
}

void test_fastwrite_corrupt(void)
{
	uint16_t sector_size = SPIF_get_sector_size();
	uint8_t read_buff[2] = { 0 };
	uint8_t write_buff[2] = { 0x69, 0x69 };
	
	SPIF_4B_erase_sector(0);
	SPIF_fast_write(0, write_buff, 2);
	write_buff[0] = 0x00;
	write_buff[1] = 0xE9;
	SPIF_fast_write(0, write_buff, 2);
	SPIF_read(0, read_buff, 2);
	if( (read_buff[0] != 0x00) || (read_buff[1] != 0x69) )
	{
		Serial.println("Fastwrite corrupted: Didn't pass");
		return;
	}
	Serial.println("Fastwrite corrupted: Passed");
	return;
}

void test_write(void)
{
	uint16_t sector_size = SPIF_get_sector_size();
	uint8_t read_buff[260] = { 0 };
	uint8_t write_buff[260] = { 0 };
	memset(write_buff, 0xE9, 260);
	
	SPIF_4B_erase_sector(0);
	SPIF_fast_write(0, write_buff, 260);
	
	memset(write_buff, 0x69, 260);
	write_buff[256] = 0xFF;
	
	SPIF_RET_t err_code = SPIF_write(0, write_buff, 260);
	SPIF_read(0, read_buff, 260);
	if ( (err_code != SPIF_ERR_INCOMPATIBLE_WRITE) || (read_buff[0] != 0x69), (read_buff[256] != 0xE9) )
	{
		Serial.println("Generic write: Didn't pass");
		return;
	}

	Serial.println("Generic write: Passed");
	return;
}

int test_forcewrite(void)
{
	uint8_t read_buff[512] = { 0 };
	uint8_t write_buff[512] = { 0 };
	uint8_t zero_buff[512] = { 0 };
	uint8_t write_value = 0x69;
	memset(write_buff, write_value, 512);
	
	uint32_t addr_offsets[5] = { 0, 260,  3836, 4000, 4100 };
	uint32_t sizes[4] = { 10, 256, 300, 550 };
	
	for (int i=0; i < 5; i++)
	{
		for (int j=0; i < 4; i++)
		{
			SPIF_4B_erase_sector(0);
			SPIF_4B_erase_sector(4096);
			
			SPIF_fast_write(addr_offsets[i], zero_buff, sizes[j]);
			SPIF_force_write(addr_offsets[i], write_buff, sizes[j]);
			SPIF_read(addr_offsets[i], read_buff, sizes[j]);
			if ( (read_buff[0] != write_value) || (read_buff[sizes[j] - 1] != write_value) )
			{
				Serial.println("Forcewrite: Didn't pass");
				return j;
			}
			memset(read_buff, 0, 512);
			
		}
	}
	Serial.println("Forcewrite: Passed");
	return 0;
}

void run_tests(void)
{
	/* Initialize SPI Flash */
	SPIF_init();
	SPIF_slow();

	/* Run tests */	
	test_size();
	test_full_erase_read();
	test_fastwrite();
	test_sector_erase();
	test_fastwrite_corrupt();
	test_write();
	test_forcewrite();
}

void setup()
{

	Serial.begin(9600);
	
	Serial.println("Starting...");
	run_tests();
}

void loop()
{
	delay(200000);
}