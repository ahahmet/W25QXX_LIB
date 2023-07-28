#ifndef __W25Q32__
#define __W25Q32__

#include <stdint.h>
#include <stdbool.h>

#include "w25qxx_def.h"

typedef uint8_t (*w25qxx_interface_read_t)(char* buffer, int len);
typedef uint8_t (*w25qxx_interface_write_t)(char* data, int len);
typedef uint8_t (*w25qxx_interface_write_byte_t)(char data);
typedef void    (*w25qxx_interface_enable_t)(bool en);
typedef int32_t (*w25qxx_get_time_t)(void);
typedef void    (*w25qxx_delay_t)(uint32_t ms);

typedef struct
{
    w25qxx_interface_write_byte_t interface_write_byte;
    // Function pointers
    w25qxx_interface_read_t   interface_read;
    w25qxx_interface_write_t  interface_write;
    w25qxx_interface_enable_t interface_enable;
    w25qxx_get_time_t         get_time;
    w25qxx_delay_t            delay;


    w25qxx_t type;
    uint8_t  device_id;
    uint16_t man_device_id;
    uint32_t jedec_id;
    uint8_t  uniq_id[8];
    uint16_t page_size;
    uint32_t page_count;
    uint32_t sector_size;
    uint32_t sector_count;
    uint32_t block_size;
    uint32_t block_count;
    uint32_t capacity_kb;// kilobyte

}w25q32_init_t;


/* Read Functions */

bool w25qxx_readByte(uint8_t *buff, uint32_t bytes_addr);

bool w25qxx_readPage(uint8_t *buff, uint32_t page_addr, 
						uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize);

bool w25qxx_readSector(uint8_t *buff, uint32_t sector_addr, 
                        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize);

bool w25qxx_readBlock(uint8_t *buff, uint32_t block_addr, 
                        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize);


/* Write Functions */

bool w25qxx_writeByte(const uint8_t* buff, uint32_t WriteAddr_inBytes);

bool w25qxx_writePage(const uint8_t *buff, uint32_t page_addr, 
						uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize);

bool w25qxx_writeSector(const uint8_t *buff, uint32_t sector_addr, 
						uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize);

bool w25qxx_writeBlock(const uint8_t *buff, uint32_t block_addr, 
							uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize);


/* Erease Functions */

bool w25qxx_eraseBlock(uint32_t block_addr);

bool w25q32_eraseSector(uint32_t sector_addr);

int8_t w25q32_eraseChip(void);


/* Init Function */
bool w25qxx_init(void);

/* Read Register */

uint8_t w25qxx_readRegX(uint8_t reg_x);


/*  */

w25q32_init_t* w25qxx_getStruct(void);

#endif