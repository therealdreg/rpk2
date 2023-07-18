/*
 * spiflash.c
 *
 * Author : Jusepe ITasahobby
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include "spiflash.h"

/* ATMEGA32u4 Port B SPI Pins */
#define CS (1<<PB0)
#define SCK (1<<PB1)
#define MOSI (1<<PB2)
#define MISO (1<<PB3)

/* Winbound W25Q512JV instruction set */

#define SPIF_INST_READ_RESPONSE				0x69
#define SPIF_INST_READ_STATUS					0x05
#define SPIF_INST_ENABLE_WRITE				0x06
#define SPIF_INST_4B_WRITE						0x12
#define SPIF_INST_4B_READ							0x13
#define SPIF_INST_4B_ERASE_SECT				0x21
#define SPIF_INST_ERASE_32BLOCK				0x52
#define SPIF_INST_ERASE								0xC7


/* Winbound W25Q512JV status register */
#define SPIF_STAT_BUSY								0x01
#define SPIF_STAT_WRITE_ENABLE				0x02

/*
** Winbound specs
** SPIF_SIZE is 67108864, last sector is used temporally on writing operations.
*/
#define SPIF_PAGE_SIZE								256
#define SPIF_SECTOR_SIZE							4096
#define SPIF_VIRT_SIZE								67104768
#define SPIF_SIZE											67108864

/************************************************************************/
/*                         AUXILIARY FUNCTIONS                          */
/************************************************************************/

/* Initializes ATMEGA32u4 SPI */
void SPIF_init(void)
{
	/* Sets DDRB to output in CS, MOSI and SCK */
	DDRB |= CS + MOSI + SCK;

	/* Pull ups Atmega32u4 PB3 (MISO) */
	PORTB |= MISO;
}

/* Enable SPI, Master, set clock rate fck/64 */
void SPIF_slow(void)
{
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(0<<SPR0);
}

/* Set ATMega32u4 CS high */
void SPIF_CS_enable(void)
{
	PORTB &= ~CS;
}

/* Set ATMega32u4 CS low */
void SPIF_CS_disable(void)
{
	PORTB |= CS;
}

/* Send data from Atmega32u4 to W25Q512JV flash */
unsigned char SPIF_send_inst(uint8_t data)
{
	/* Start transmission */
	SPDR = data;
	/* Wait for transmission and reception */
	while (!(SPSR & (1 << SPIF))) {}
	/* Return result*/
	return SPDR;
}

/*
** Reads Status Register. May be used at any time, even while a Program,
** Erase or Write Status Register cycle is in progress
*/
uint8_t SPIF_read_status()
{
	uint8_t status;
	SPIF_CS_disable();
	SPIF_CS_enable();

	SPIF_send_inst(SPIF_INST_READ_STATUS);
	status = SPIF_send_inst(SPIF_INST_READ_RESPONSE);
	SPIF_CS_disable();
	return status;
}

/*
** Sets WEL bit to 1, keep in mind that Write enable bit is
** automatically reset after completion of the Write Status
** Register, Erase/Program Security Registers, Page Program,
** Sector Erase, Block Erase, Chip Erase among others.
*/
void SPIF_enable_write()
{
	SPIF_CS_disable();
	SPIF_CS_enable();

	SPIF_send_inst(SPIF_INST_ENABLE_WRITE);

	while (!(SPIF_read_status() & SPIF_STAT_WRITE_ENABLE)) {}

	SPIF_CS_disable();
	return;
}

/*
** Auxiliary function to read data from flash and write into the given buffer.
** It has all memory address space available, including the last sector used 
** as temporary storage.
*/
SPIF_RET_t SPIF_uncheck_read(uint32_t address, uint8_t* buff, uint32_t size)
{

	if (address > SPIF_SIZE) return SPIF_ERR_MEM_ADDR_OUTOF_RANGE;
	if (address + size > SPIF_SIZE) return SPIF_ERR_SIZE_OUTOF_RANGE;

	while (SPIF_read_status() & SPIF_STAT_BUSY) {}

	SPIF_CS_disable();
	SPIF_CS_enable();

	SPIF_send_inst(SPIF_INST_4B_READ);
	SPIF_send_inst(address >> 24);
	SPIF_send_inst(address >> 16);
	SPIF_send_inst(address >> 8);
	SPIF_send_inst(address);

	for (uint32_t j = 0; j < size; j++)
	{
		buff[j] = SPIF_send_inst(SPIF_INST_READ_RESPONSE);
	}

	SPIF_CS_disable();

	return SPIF_OK;
}

/*
** Auxiliary function to write given buffer to flash. It has all memory address
** space available, including the last sector used as temporary storage.
*/
SPIF_RET_t SPIF_uncheck_write(uint32_t address, uint8_t* buff, uint32_t size)
{
	uint32_t offset = 0;
	uint32_t page_address = 0;
	uint32_t first_page = address - (address % SPIF_PAGE_SIZE);
	uint32_t last_page = ( (address + size - 1) / SPIF_PAGE_SIZE ) * SPIF_PAGE_SIZE;
	int32_t pages_count = (size / SPIF_PAGE_SIZE) - 1;
	uint32_t write_count = 0;

	if (size <= 0 ) return SPIF_ERR_MEM_INVALID_ADDR;
	if (address > SPIF_SIZE) return SPIF_ERR_MEM_ADDR_OUTOF_RANGE;
	if (address + size > SPIF_SIZE) return SPIF_ERR_SIZE_OUTOF_RANGE;

	while (SPIF_read_status() & SPIF_STAT_BUSY) {}

	/* Write first page */

	SPIF_enable_write();

	SPIF_CS_disable();
	SPIF_CS_enable();

	SPIF_send_inst(SPIF_INST_4B_WRITE);
	SPIF_send_inst(address >> 24);
	SPIF_send_inst(address >> 16);
	SPIF_send_inst(address >> 8);
	SPIF_send_inst(address);


	if ( (address + size) >= (first_page + SPIF_PAGE_SIZE) )
	{
		write_count = first_page + SPIF_PAGE_SIZE - address;
	}
	else
	{
		write_count = size;
	}

	for (uint16_t j = 0; j < write_count; j++)
	{
		SPIF_send_inst(buff[j]);
	}

	while (SPIF_read_status() & SPIF_STAT_BUSY) {}

	SPIF_CS_disable();

	/* Write pages in the middle */

	for (int32_t i = 0; i < pages_count; i++)
	{
		offset = i * SPIF_PAGE_SIZE + write_count;
		page_address = first_page +  SPIF_PAGE_SIZE + (i * SPIF_PAGE_SIZE);
		SPIF_enable_write();

		SPIF_CS_disable();
		SPIF_CS_enable();

		SPIF_send_inst(SPIF_INST_4B_WRITE);
		SPIF_send_inst(page_address >> 24);
		SPIF_send_inst(page_address >> 16);
		SPIF_send_inst(page_address >> 8);
		SPIF_send_inst(page_address);


		for (uint16_t j = 0; j < SPIF_PAGE_SIZE; j++)
		{
			SPIF_send_inst(buff[offset + j]);
		}

		while (SPIF_read_status() & SPIF_STAT_BUSY) {}

		SPIF_CS_disable();
	}


	if(last_page && (first_page != last_page) )
	{
		offset = last_page - address;
		write_count = address + size - last_page;

		SPIF_enable_write();

		SPIF_CS_disable();
		SPIF_CS_enable();

		SPIF_send_inst(SPIF_INST_4B_WRITE);
		SPIF_send_inst(last_page >> 24);
		SPIF_send_inst(last_page >> 16);
		SPIF_send_inst(last_page >> 8);
		SPIF_send_inst(last_page);

		for (uint16_t j = 0; j < write_count ; j++)
		{
			SPIF_send_inst(buff[offset + j]);
		}

		while (SPIF_read_status() & SPIF_STAT_BUSY) {}

		SPIF_CS_disable();
	}

	return SPIF_OK;
}

/************************************************************************/
/*                          EXPORTED FUNCTIONS                          */
/************************************************************************/

/* Fill the whole flash with 0xFF, it may take some time. */
void SPIF_erase(void)
{
	while ((SPIF_read_status() & SPIF_STAT_BUSY)) {}

	SPIF_enable_write();

	SPIF_CS_disable();
	SPIF_CS_enable();

	SPIF_send_inst(SPIF_INST_ERASE);

	SPIF_CS_disable();

	return;
}

/* Fill the given sector with 0xFF, */
void SPIF_4B_erase_sector(uint32_t address)
{
	while (SPIF_read_status() & SPIF_STAT_BUSY) {}

	SPIF_enable_write();

	SPIF_CS_disable();
	SPIF_CS_enable();

	SPIF_send_inst(SPIF_INST_4B_ERASE_SECT);

	SPIF_send_inst(address >> 24);
	SPIF_send_inst(address >> 16);
	SPIF_send_inst(address >> 8);
	SPIF_send_inst(address);


	SPIF_CS_disable();

	return;
}

/* Return page size in bytes. */
uint16_t SPIF_get_page_size(void)
{
	return SPIF_PAGE_SIZE;
}

/* Return sector size in bytes */
uint16_t SPIF_get_sector_size(void)
{
	return SPIF_SECTOR_SIZE;
}

/* Return usable flash size in bytes */
uint32_t SPIF_get_size(void)
{
	return SPIF_VIRT_SIZE;
}

/* Read from flash to buffer up to last sector*/
SPIF_RET_t SPIF_read(uint32_t address, uint8_t* buff, uint32_t size)
{

	if (address > SPIF_VIRT_SIZE )  return SPIF_ERR_MEM_ADDR_OUTOF_RANGE;
	if (address + size > SPIF_VIRT_SIZE ) return SPIF_ERR_SIZE_OUTOF_RANGE;

	return SPIF_uncheck_read(address, buff, size);
}

/*
** Attempts to write data, if any involved page has no compatible data (writing
** 1's where there is a 0's) then returns SPIF_ERR_INCOMPATIBLE_WRITE. Any previous
** pages that were compatible are writen.
*/
SPIF_RET_t SPIF_write(uint32_t address, uint8_t* buff, uint32_t size)
{
	uint32_t offset = 0;
	uint32_t page_address = 0;
	uint32_t remainder = size % SPIF_PAGE_SIZE;
	uint8_t read_byte = 0;
	uint8_t result = 0;

	/* Check for size errors */
	if (address > SPIF_VIRT_SIZE ) return SPIF_ERR_MEM_ADDR_OUTOF_RANGE;
	if (address + size > SPIF_VIRT_SIZE ) return SPIF_ERR_SIZE_OUTOF_RANGE;

	while (SPIF_read_status() & SPIF_STAT_BUSY) {}

	for (uint32_t i = 0; i < (size / SPIF_PAGE_SIZE); i++)
	{
		offset = i * SPIF_PAGE_SIZE;
		page_address = address + offset;

		/* Check if compatible */
		SPIF_CS_disable();
		SPIF_CS_enable();

		SPIF_send_inst(SPIF_INST_4B_READ);
		SPIF_send_inst(address >> 24);
		SPIF_send_inst(address >> 16);
		SPIF_send_inst(address >> 8);
		SPIF_send_inst(address);

		for (uint32_t j = 0; j < SPIF_PAGE_SIZE; j++)
		{
			read_byte = SPIF_send_inst(SPIF_INST_READ_RESPONSE);
			result = (~buff[offset + j]) | read_byte;
			if( result != 0xFF) return SPIF_ERR_INCOMPATIBLE_WRITE;
		}

		SPIF_CS_disable();

		/* Write page */
		SPIF_fast_write(page_address, buff+offset, SPIF_PAGE_SIZE);
	}

	if( remainder != 0 )
	{
		offset = size - remainder;
		page_address = address + offset;

		SPIF_CS_disable();
		SPIF_CS_enable();

		SPIF_send_inst(SPIF_INST_4B_READ);
		SPIF_send_inst(page_address >> 24);
		SPIF_send_inst(page_address >> 16);
		SPIF_send_inst(page_address >> 8);
		SPIF_send_inst(page_address);

		for (uint32_t j = 0; j < 1; j++)
		{
			read_byte = SPIF_send_inst(SPIF_INST_READ_RESPONSE);
			result = (~buff[offset + j]) | read_byte;
			if( result != 0xFF) return SPIF_ERR_INCOMPATIBLE_WRITE;
		}

		SPIF_CS_disable();

		SPIF_fast_write(page_address, buff+offset, remainder);

	}

	return SPIF_OK;
}

/*
** Write without erasing involved sectors. ONLY use when given sectors
** are filled with 0xFF (erased), otherwise the sectors may end up with
** corrupted data.
*/
SPIF_RET_t SPIF_fast_write(uint32_t address, uint8_t* buff, uint32_t size)
{
	if (address > SPIF_VIRT_SIZE ) return SPIF_ERR_MEM_ADDR_OUTOF_RANGE;
	if (address + size > SPIF_VIRT_SIZE ) return SPIF_ERR_SIZE_OUTOF_RANGE;

	return SPIF_uncheck_write(address, buff, size);
}

/*
** Checks if the data to write is compatible with already stored information,
** otherwise it writes data to an auxiliary sector and overwrites the whole
** sector.
*/
SPIF_RET_t SPIF_force_write(uint32_t address, uint8_t* buff, uint32_t size)
{
	/*
	** Write instruction can only write 0s so we need to read the involved
	** sectors, which is the smallest unit we can erase. Afterwards we
	** write the previously stored data updated with the new one.
	*/
	SPIF_RET_t ret = SPIF_write(address, buff, size);
	if (ret == SPIF_ERR_INCOMPATIBLE_WRITE)
	{
		return SPIF_slow_write(address, buff, size);
	}

	return ret;
}

/*
** Writes involved sectors to an auxiliary one even if it is not needed.
*/
SPIF_RET_t SPIF_slow_write(uint32_t address, uint8_t* buff, uint32_t size)
{
	uint32_t first_sector = address - (address % SPIF_SECTOR_SIZE);
	uint32_t last_sector = (address + size) - ( (address + size) % SPIF_SECTOR_SIZE);
	int32_t sector_count = (size / SPIF_SECTOR_SIZE) - 1;
	uint32_t sector_addr = 0;
	uint32_t page_addr = 0;
	uint32_t page_count = 0;
	uint8_t write_size = 0;
	int32_t buff_offset = 0;
	uint8_t remainder = 0;
	uint8_t tmp[SPIF_PAGE_SIZE] = { 0 };

	if (address > SPIF_SIZE ) return SPIF_ERR_MEM_ADDR_OUTOF_RANGE;
	if (address + size > SPIF_SIZE ) return SPIF_ERR_SIZE_OUTOF_RANGE;

	/*
	** Shouldn't be able to write more than one sector with a single instruction
	** due to lack of memory to store such buffer. Despite that this approach
	** still works in case you may write an indeterminate amount of sectors.
	*/

	/* First sector may be a mix of existing and new data */
	SPIF_4B_erase_sector(SPIF_VIRT_SIZE);

	/* We save previous data until we get to new data */
	page_count = (address - first_sector) / SPIF_PAGE_SIZE;
	for (uint32_t j = 0; j < page_count; j++)
	{
		page_addr = first_sector + (j * SPIF_PAGE_SIZE);
		SPIF_read(page_addr, tmp, SPIF_PAGE_SIZE);
		SPIF_uncheck_write( ( SPIF_VIRT_SIZE + (j * SPIF_PAGE_SIZE) ), tmp, SPIF_PAGE_SIZE);
	}
	page_addr = first_sector + (page_count * SPIF_PAGE_SIZE);
	write_size = address - page_addr;
	SPIF_read(page_addr, tmp, write_size);
	SPIF_uncheck_write( SPIF_VIRT_SIZE + page_addr, tmp, write_size);

	/* Save actual data */

	if ( (address + size) >= (first_sector + SPIF_SECTOR_SIZE) )
	{
		page_count = (first_sector + SPIF_SECTOR_SIZE - address) / SPIF_PAGE_SIZE;
		remainder = (first_sector + SPIF_SECTOR_SIZE) - (address + (page_count * SPIF_PAGE_SIZE) );
	}
	else
	{
		page_count = size / SPIF_PAGE_SIZE;
		remainder = size % SPIF_PAGE_SIZE;
	}

	for (uint32_t j = 0; j < page_count; j++)
	{
		SPIF_uncheck_write( SPIF_VIRT_SIZE + address + (j * SPIF_PAGE_SIZE), buff, SPIF_PAGE_SIZE);
	}
	buff_offset = (page_count * SPIF_PAGE_SIZE);
	SPIF_uncheck_write( SPIF_VIRT_SIZE + address + (page_count * SPIF_PAGE_SIZE), buff + buff_offset, remainder);

	if ( (address + size) < (first_sector + SPIF_SECTOR_SIZE) )
	{
		/* Save rest */
		page_count = ( first_sector + SPIF_SECTOR_SIZE - (address + size) ) / SPIF_PAGE_SIZE;
		for (uint32_t j = 0; j < page_count; j++)
		{
			page_addr = address + size + (j * SPIF_PAGE_SIZE);
			SPIF_read(page_addr, tmp, SPIF_PAGE_SIZE);
			SPIF_uncheck_write( ( SPIF_VIRT_SIZE + ( address + size) + (j * SPIF_PAGE_SIZE) ), tmp, SPIF_PAGE_SIZE);
		}

		page_addr = address + size + (page_count * SPIF_PAGE_SIZE);
		write_size = first_sector + SPIF_SECTOR_SIZE - page_addr;
		SPIF_read(page_addr, tmp, write_size);
		SPIF_uncheck_write( page_addr, tmp, write_size );

	}

	/* Write data	from auxiliary sector */

	SPIF_4B_erase_sector(first_sector);
	for (uint32_t j = 0; j < (SPIF_SECTOR_SIZE / SPIF_PAGE_SIZE); j++)
	{
		page_addr = first_sector + (j * SPIF_PAGE_SIZE);
		SPIF_uncheck_read( (SPIF_VIRT_SIZE + (j * SPIF_PAGE_SIZE) ) , tmp, SPIF_PAGE_SIZE);
		SPIF_fast_write(page_addr, tmp, SPIF_PAGE_SIZE);
	}

	// Sectors in between should always contain new data

	for (int32_t i = 0; i < sector_count; i++)
	{
		sector_addr = first_sector + ( i * SPIF_SECTOR_SIZE);

		SPIF_4B_erase_sector(SPIF_VIRT_SIZE);

		for (uint32_t j = 0; j < (SPIF_SECTOR_SIZE / SPIF_PAGE_SIZE); j++)
		{
			page_addr = sector_addr + (j * SPIF_PAGE_SIZE);
			SPIF_read(page_addr, tmp, SPIF_PAGE_SIZE);
			SPIF_uncheck_write( ( SPIF_VIRT_SIZE + (j * SPIF_PAGE_SIZE) ), tmp, SPIF_PAGE_SIZE);
		}

		SPIF_4B_erase_sector(sector_addr);

		for (uint32_t j = 0; j < (SPIF_SECTOR_SIZE / SPIF_PAGE_SIZE); j++)
		{
			page_addr = sector_addr + (j * SPIF_PAGE_SIZE);

			SPIF_uncheck_read( ( SPIF_VIRT_SIZE + (j * SPIF_PAGE_SIZE) ), tmp, SPIF_PAGE_SIZE);
			SPIF_fast_write(page_addr, tmp, SPIF_PAGE_SIZE);
		}

	}

	/* Last sector */
	if(last_sector && last_sector != first_sector){

		SPIF_4B_erase_sector(SPIF_VIRT_SIZE);

		page_count = (address + size - last_sector) / SPIF_PAGE_SIZE;
		remainder = (address + size - last_sector) - ( page_count * SPIF_PAGE_SIZE);
		buff_offset = size - (address + size - last_sector);

		for (uint32_t j = 0; j < page_count; j++)
		{
			SPIF_uncheck_write( SPIF_VIRT_SIZE + (j * SPIF_PAGE_SIZE), buff + buff_offset, SPIF_PAGE_SIZE);
		}
		buff_offset = buff_offset + (page_count * SPIF_PAGE_SIZE);
		SPIF_uncheck_write( SPIF_VIRT_SIZE + (page_count * SPIF_PAGE_SIZE), buff + address + size - last_sector, remainder);
		// Save rest
		page_count = ( last_sector + SPIF_SECTOR_SIZE - (address + size) ) / SPIF_PAGE_SIZE;
		for (uint32_t j = 0; j < page_count; j++)
		{
			page_addr = address + size + (j * SPIF_PAGE_SIZE);
			SPIF_read(4096+256, tmp, SPIF_PAGE_SIZE);
			SPIF_uncheck_write( ( SPIF_VIRT_SIZE + ( address + size - last_sector ) + (j * SPIF_PAGE_SIZE) ), tmp, SPIF_PAGE_SIZE);
		}

		page_addr = address + size + (page_count * SPIF_PAGE_SIZE);
		write_size = last_sector + SPIF_SECTOR_SIZE - page_addr;
		SPIF_read(page_addr, tmp, write_size);
		SPIF_uncheck_write( page_addr, tmp, write_size );

		SPIF_4B_erase_sector(last_sector);

		for (uint32_t j = 0; j < (SPIF_SECTOR_SIZE / SPIF_PAGE_SIZE); j++)
		{
			page_addr = last_sector + (j * SPIF_PAGE_SIZE);

			SPIF_uncheck_read( ( SPIF_VIRT_SIZE + (j * SPIF_PAGE_SIZE) ), tmp, SPIF_PAGE_SIZE);
			SPIF_fast_write(page_addr, tmp, SPIF_PAGE_SIZE);
		}

	}


	return SPIF_OK;
}