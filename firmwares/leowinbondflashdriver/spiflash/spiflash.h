/*
 * spiflash.h
 *
 *  Author: Jusepe ITasahobby
 */ 


#ifndef SPIFLASH_H_
#define SPIFLASH_H_

#ifdef __cplusplus
#define SPIF_API extern "C"
#else
#define SPIF_API
#endif

/* SPI management */
SPIF_API void SPIF_init(void);
SPIF_API void SPIF_slow(void);
/* Error types */
typedef enum {
	SPIF_OK = 0,
	SPIF_ERR_MEM_ADDR_OUTOF_RANGE = 1,
	SPIF_ERR_SIZE_OUTOF_RANGE = 2,
	SPIF_ERR_MEM_INVALID_ADDR = 3,
	SPIF_ERR_INCOMPATIBLE_WRITE = 4,
} SPIF_RET_t;
/* SPI Flash operations */
SPIF_API void SPIF_erase(void);
SPIF_API void SPIF_4B_erase_sector(uint32_t address);
SPIF_API uint16_t SPIF_get_page_size(void);
SPIF_API uint16_t SPIF_get_sector_size(void);
SPIF_API uint32_t SPIF_get_size(void);
SPIF_API SPIF_RET_t SPIF_read(uint32_t address, uint8_t* buff, uint32_t size);
SPIF_API SPIF_RET_t SPIF_write(uint32_t address, uint8_t* buff, uint32_t size);
SPIF_API SPIF_RET_t SPIF_fast_write(uint32_t address, uint8_t* buff, uint32_t size);
SPIF_API SPIF_RET_t SPIF_force_write(uint32_t address, uint8_t* buff, uint32_t size);
SPIF_API SPIF_RET_t SPIF_slow_write(uint32_t address, uint8_t* buff, uint32_t size);

#endif /* SPIFLASH_H_ */