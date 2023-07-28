#include <stdint.h>
#include "w25qxx.h"

#define SIZE_1_BYTE sizeof(char)

#define ERROR_CHECK(x) ({ \
	if(!x){				  \
		return false;	  \
	}					  \
})


w25q32_init_t w25qxx;

static void w25qxx_powerUp(void)
{
	char dummy = CMD_DUMMY;

    w25qxx.interface_enable(true);

	w25qxx.interface_write_byte(CMD_Device_ID);

	for(int i=0; i<3; ++i) w25qxx.interface_write_byte(CMD_DUMMY);

	w25qxx.interface_read(&w25qxx.device_id, SIZE_1_BYTE);

	w25qxx.interface_enable(false);
	/*
		   POWER UP INSTRUCTION 
		|-----------------------|
		|BYTE1|BYTE2|BYTE3|BYTE4|
		|-----------------------|
		|0xAB |DUMMY|DUMMY|DUMMY|
		|-----------------------| 
	*/
}


static uint16_t w25qxx_getManDeviceID(void)
{
	char id;

	w25qxx.interface_enable(true);

	w25qxx.interface_write_byte(CMD_Device_ID);
	for(int i=0; i<3; ++i) w25qxx.interface_write_byte(CMD_DUMMY);

	w25qxx.interface_read(&id, SIZE_1_BYTE);

	w25qxx.interface_enable(false);

	return (CMD_Manufacture_ID << 8) | id;
	/*
		Read Manufacturer Device INSTRUCTION 
		|-----------------------|
		|BYTE1|BYTE2|BYTE3|BYTE4|
		|-----------------------|
		|0x90 |DUMMY|DUMMY|DUMMY|
		|-----------------------| 
	*/
}



static uint32_t w25qxx_getJedecID(void)
{
	char buffer[3];

	w25qxx.interface_enable(true);

	w25qxx.interface_write_byte(CMD_JEDEC_ID);

	w25qxx.interface_read(buffer, SIZE_1_BYTE*3);

	w25qxx.interface_enable(false);
	
	return (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];

	/*
		POWER UP INSTRUCTION 
		|-----|
		|BYTE1|
		|-----|
		|0x9F |
		|-----| 
	*/
}

static void w25qxx_getUniqID(void)
{
	w25qxx.interface_enable(true);

	w25qxx.interface_write_byte(CMD_Unique_ID);
	for (uint8_t i = 0; i < 4; i++) w25qxx.interface_write_byte(CMD_DUMMY);

	w25qxx.interface_read(w25qxx.uniq_id, SIZE_1_BYTE*8);

	w25qxx.interface_enable(false);
	/*
		Read Manufacturer Device INSTRUCTION 
		|-----------------------------|
		|BYTE1|BYTE2|BYTE3|BYTE4|BYTE5|
		|-----------------------------|
		|0x90 |DUMMY|DUMMY|DUMMY|DUMMY|
		|-----------------------------| 
	*/
}



static void w25qxx_enableWrite(void)
{
	w25qxx.interface_enable(true);

	w25qxx.interface_write_byte(CMD_Write_Enable);

	w25qxx.interface_enable(false);
}


static void w25qxx_enableWriteSR(void)
{
	w25qxx.interface_enable(true);

	w25qxx.interface_write_byte(CMD_Write_Enable_SR);

	w25qxx.interface_enable(false);
}



static void w25qxx_disableWrite(void)
{
	w25qxx.interface_enable(true);

	w25qxx.interface_write_byte(CMD_Write_Enable);

	w25qxx.interface_enable(false);
}


static bool w25qxx_waitForWriteEnd(void)
{
	w25qxx.interface_enable(true);

	uint32_t current_time = w25qxx.get_time();
	uint32_t useTime = 0;
	uint8_t reg_res;
	w25qxx.interface_write_byte(CMD_Reg_1_Read);
	do
	{
		reg_res = w25qxx.interface_write_byte(CMD_DUMMY);

		useTime = w25qxx.get_time() - current_time;
	} while (((reg_res & SR1_S0_BUSY) == SR1_S0_BUSY) && (useTime < SPI_FLASH_TIMEOUT));

	w25qxx.interface_enable(true);

	if (useTime >= SPI_FLASH_TIMEOUT)	// timeOut return 1
		return false;
	return true;	// passed return 0
}


static uint32_t w25qxx_pageToSector(uint32_t page_addr)
{
	return ((page_addr * w25qxx.page_size) / w25qxx.sector_size);
}


static uint32_t w25qxx_pageToBlock(uint32_t page_addr)
{
	return ((page_addr * w25qxx.page_size) / w25qxx.block_size);
}


static uint32_t w25qxx_sectorToPage(uint32_t sector_addr)
{
	return (sector_addr * w25qxx.sector_size) / w25qxx.page_size;
}


static uint32_t w25qxx_blockToPage(uint32_t bloack_addr)
{
	return (bloack_addr * w25qxx.block_size) / w25qxx.page_size;
}



/**
  * @brief  Read Status Register-1, 2, 3(05h, 35h, 15h)
  * @param  reg_x: [in] 1,2,3
  * @retval retrun SR_x value [Byte]
  */
uint8_t w25qxx_readRegX(uint8_t reg_x)
{
	uint8_t buff;
	
	w25qxx.interface_enable(true);

	switch(reg_x)
	{
		case 1:	// reg 1
			w25qxx.interface_write_byte(CMD_Reg_1_Read);
			break;
		case 2:	// reg 2
			w25qxx.interface_write_byte(CMD_Reg_2_Read);
			break;
		case 3:	// reg 3
			w25qxx.interface_write_byte(CMD_Reg_3_Read);
			break;
		default:
			break;
	}
	w25qxx.interface_read(&buff, SIZE_1_BYTE);

	w25qxx.interface_enable(false);

	return buff;
}

/** 
  * @brief Write Status Register-1, 2, 3 (01h, 31h, 11h)
  * @param reg_x: [in] reg_1,2,3
  * @param data:  [in] input reg_x data
  */
static void w25qxx_writeRegX(uint8_t reg_x, uint8_t data)
{
	w25qxx.interface_enable(true);

	switch (reg_x)
	{
		case 1:
			w25qxx.interface_write_byte(CMD_Reg_1_Write);
			break;
		case 2:
			w25qxx.interface_write_byte(CMD_Reg_2_Write);
			break;
		case 3:
			w25qxx.interface_write_byte(CMD_Reg_3_Write);
			break;
		default:
			break;
	}
	w25qxx.interface_write_byte(data);

	w25qxx.interface_enable(false);
}


int8_t w25q32_eraseChip(void)
{
	ERROR_CHECK(w25qxx_waitForWriteEnd());

	w25qxx_enableWrite();

	w25qxx.interface_enable(true);

	w25qxx.interface_write_byte(CMD_Erase_Chip);

	w25qxx.interface_enable(false);

	ERROR_CHECK(w25qxx_waitForWriteEnd());

	return true;
}


/** 
  * @brief  Sector erase 4KB
  * @param  sector_addr: [in] 0 ~ W25Qxxx_SectorCount-1
  * @retval status 0:passed  1:failed
  */
bool w25q32_eraseSector(uint32_t sector_addr)
{
	ERROR_CHECK(w25qxx_waitForWriteEnd());

	sector_addr = sector_addr * w25qxx.sector_size;
	w25qxx_enableWrite();

	w25qxx.interface_enable(true);
	if (w25qxx.type >= W25Q256){

		w25qxx.interface_write_byte(CMD_Erase_Sector_4_Byte_Addr);
		w25qxx.interface_write_byte((sector_addr & 0xFF000000) >> 24);
	}else{
		w25qxx.interface_write_byte(CMD_Erase_Sector);
	}
	w25qxx.interface_write_byte((sector_addr & 0xFF0000) >> 16);
	w25qxx.interface_write_byte((sector_addr & 0xFF00) >> 8);
	w25qxx.interface_write_byte(sector_addr & 0xFF);


	w25qxx.interface_enable(false);

	ERROR_CHECK(w25qxx_waitForWriteEnd());

	return true;
}



/** 
  * @brief Erase block 64KB
  * @param block_addr: [in] 0 ~ W25Qxxx_BlockCount-1
  * @retval status 0:passed  1:failed
  */
bool w25qxx_eraseBlock(uint32_t block_addr)
{
	ERROR_CHECK(w25qxx_waitForWriteEnd());

	block_addr = block_addr * w25qxx.block_size;
	w25qxx.interface_enable(true);

	if (w25qxx.type >= W25Q256){
		w25qxx.interface_write_byte(CMD_Erase_Block_64K_4_Byte_Addr);
		w25qxx.interface_write_byte((block_addr & 0xFF000000) >> 24);
	}else{
		w25qxx.interface_write_byte(CMD_Erase_Block_64K_4_Byte_Addr);
		w25qxx.interface_write_byte(CMD_Erase_Block_64K);
	}
	w25qxx.interface_write_byte((block_addr & 0xFF0000) >> 16);
	w25qxx.interface_write_byte((block_addr & 0xFF00) >> 8);
	w25qxx.interface_write_byte(block_addr & 0xFF);

	w25qxx.interface_enable(false);

	ERROR_CHECK(w25qxx_waitForWriteEnd());

	return true;
}





/** 
  * @brief write one Byte to w25qxxx flash
  * @param pBuffer: [in] input data
  * @param WriteAddr_inBytes: [in] indicate address
  * @retval status 0:passed  1:failed
  */
bool w25qxx_writeByte(const uint8_t* buff, uint32_t WriteAddr_inBytes)
{
	ERROR_CHECK(w25qxx_waitForWriteEnd());

	w25qxx_enableWrite();

	w25qxx.interface_enable(true);

	if (w25qxx.type >= W25Q256){
		w25qxx.interface_write_byte(CMD_Page_Program_4_Byte_Addr);
		w25qxx.interface_write_byte((WriteAddr_inBytes & 0xFF000000) >> 24);
	}else{
		w25qxx.interface_write_byte(CMD_Page_Program);
	}

	w25qxx.interface_write_byte((WriteAddr_inBytes & 0xFF0000) >> 16);
	w25qxx.interface_write_byte((WriteAddr_inBytes & 0xFF00) >> 8);
	w25qxx.interface_write_byte(WriteAddr_inBytes & 0xFF);
	w25qxx.interface_write(buff, SIZE_1_BYTE);

	w25qxx.interface_enable(false);

	ERROR_CHECK(w25qxx_waitForWriteEnd());

	return true;
}



/** 
  * @brief write Byte data to indicate page address
  * @param *pBuffer: [in] Byte data array
  * @param Page_Address: [in] page address (0 - W25Qxxx_PageCount-1)
  * @param OffsetInByte: [in] offset address
  * @retval status 0:passed  1:failed
  */
bool w25qxx_writePage(const uint8_t *buff, uint32_t page_addr, 
						uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize)
{
	if (((NumByteToWrite_up_to_PageSize + OffsetInByte) > w25qxx.page_size) || (NumByteToWrite_up_to_PageSize == 0))
		NumByteToWrite_up_to_PageSize = w25qxx.page_size - OffsetInByte;

	ERROR_CHECK(w25qxx_waitForWriteEnd());

	w25qxx_enableWrite();

	w25qxx.interface_enable(true);

	page_addr = (page_addr * w25qxx.page_size) + OffsetInByte;
	if (w25qxx.type >= W25Q256){
		w25qxx.interface_write_byte(CMD_Page_Program_4_Byte_Addr);
		w25qxx.interface_write_byte((page_addr & 0xFF000000) >> 24);
	}else{
		w25qxx.interface_write_byte(CMD_Page_Program);
	}
	w25qxx.interface_write_byte((page_addr & 0xFF0000) >> 16);
	w25qxx.interface_write_byte((page_addr & 0xFF00) >> 8);
	w25qxx.interface_write_byte(page_addr & 0xFF);

	//HAL_SPI_Transmit(&hspi_flash, (uint8_t *)pBuffer, NumByteToWrite_up_to_PageSize, SPI_FLASH_TIMEOUT);
	w25qxx.interface_write((char*)buff, NumByteToWrite_up_to_PageSize);

	w25qxx.interface_enable(false);

	ERROR_CHECK(w25qxx_waitForWriteEnd());

	//TODO delay function
	w25qxx.delay(1);
	//HAL_Delay(1);

	return true;
}

/** 
  * @brief write Byte data to indicate sector address  4KB Max based on page Write
  * @param *pBuffer: [in] Byte data array
  * @param Page_Address: [in] page address (0 - W25Qxxx_SectorCount-1)
  * @param OffsetInByte: [in] offset byte number
  * @param NumByteToWrite_up_to_SectorSize: [in] Byte data number
  * @retval status 0:passed  1:failed
  */
bool w25qxx_writeSector(const uint8_t *buff, uint32_t sector_addr, 
						uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize)
{
	uint32_t start_page;
	int32_t  remain_bytes;
	uint32_t local_offset;

	if ((NumByteToWrite_up_to_SectorSize > w25qxx.sector_size) || 
											(NumByteToWrite_up_to_SectorSize == 0))
		NumByteToWrite_up_to_SectorSize = w25qxx.sector_size;

	if (OffsetInByte >= w25qxx.sector_size){
		return false;
	}

	if ((OffsetInByte + NumByteToWrite_up_to_SectorSize) > w25qxx.sector_size){
		remain_bytes = w25qxx.sector_size - OffsetInByte;
	}else{
		remain_bytes = NumByteToWrite_up_to_SectorSize;
	}
		

	start_page = w25qxx_sectorToPage(sector_addr) + (OffsetInByte / w25qxx.page_size);
	local_offset = OffsetInByte % w25qxx.page_size;
	
	do{
		uint8_t res = w25qxx_writePage(buff, start_page, local_offset, remain_bytes);
		if (res != 0)
			return false;
		start_page++;
		remain_bytes -= w25qxx.page_size - local_offset;
		buff += w25qxx.page_size - local_offset;
		local_offset = 0;
	}while(remain_bytes > 0);

	return true;
}


/** 
  * @brief write Byte data to indicate block address  64KB Max base on page Write
  * @param *pBuffer: [in] Byte data array
  * @param Block_Address: [in] page address (0 - W25Qxxx_BlockCount-1)
  * @param OffsetInByte: [in] offset byte number
  * @param NumByteToWrite_up_to_BlockSize: [in] Byte data number
  * @retval status 0:passed  1:failed
  */
bool w25qxx_writeBlock(const uint8_t *buff, uint32_t block_addr, 
							uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize)
{
	uint32_t start_page;
	int32_t  bytes_to_write;
	uint32_t local_offset;

	if ((NumByteToWrite_up_to_BlockSize > w25qxx.block_size) || 
											(NumByteToWrite_up_to_BlockSize == 0))
		NumByteToWrite_up_to_BlockSize = w25qxx.block_size;

	if (OffsetInByte >= w25qxx.block_size){
		return false;
	}

	if ((OffsetInByte + NumByteToWrite_up_to_BlockSize) > w25qxx.block_size){
		bytes_to_write = w25qxx.block_size - OffsetInByte;
	}else{
		bytes_to_write = NumByteToWrite_up_to_BlockSize;
	}
		

	start_page = w25qxx_blockToPage(block_addr) + (OffsetInByte / w25qxx.block_size);
	local_offset = OffsetInByte % w25qxx.page_size;

	do{
		uint8_t res = w25qxx_writePage(buff, start_page, local_offset, bytes_to_write);
		if (res != 0)
			return false;
		start_page++;
		bytes_to_write -= w25qxx.page_size - local_offset;
		buff += w25qxx.page_size - local_offset;
		local_offset = 0;
	}while(bytes_to_write > 0);

	return true;
}




/** ############################################################################################
  * @brief  read one Byte data from indicate address
  * @param  *pBuffer: [out] receive read byte data
  * @param  Bytes_Address: [in] address 0 ~ (W25Qxxx_CapacityInKiloByte-1)*1024
  * @retval status 0:passed  1:failed
  */
bool w25qxx_readByte(uint8_t *buff, uint32_t bytes_addr)
{
	ERROR_CHECK(w25qxx_waitForWriteEnd());

	w25qxx.interface_enable(true);

	if (w25qxx.type >= W25Q256){
		w25qxx.interface_write_byte(CMD_Fast_Read_4_Byte_Addr);
		w25qxx.interface_write_byte((bytes_addr & 0xFF000000) >> 24);
	}else{
		w25qxx.interface_write_byte(CMD_Fast_Read);
	}

	w25qxx.interface_write_byte((bytes_addr & 0xFF0000) >> 16);
	w25qxx.interface_write_byte((bytes_addr & 0xFF00) >> 8);
	w25qxx.interface_write_byte(bytes_addr & 0xFF);
	//TODO
	w25qxx.interface_write_byte(CMD_DUMMY);

	w25qxx.interface_read(buff, SIZE_1_BYTE);

	w25qxx.interface_enable(false);

	return true;
}



/** 
  * @brief read a page from indicate page-address
  * @param *pBuffer: [out] receive bytes
  * @param Page_Address: [in] page address (0 - W25Qxxx_PageCount-1)
  * @param OffsetInByte: [in] offset byte number   [0 --- offset ------ 255]
  * @param NumByteToRead_up_to_PageSize: [in] read byte number  max 256Bytes
  * @retval status 0:passed   1:failed
  */
bool w25qxx_readPage(uint8_t *buff, uint32_t page_addr, 
						uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize)
{
	if ((NumByteToRead_up_to_PageSize > w25qxx.page_size) || (NumByteToRead_up_to_PageSize == 0))
		NumByteToRead_up_to_PageSize = w25qxx.page_size;
	
	if ((OffsetInByte + NumByteToRead_up_to_PageSize) > w25qxx.page_size)
		NumByteToRead_up_to_PageSize = w25qxx.page_size - OffsetInByte;

	page_addr = page_addr * w25qxx.page_size + OffsetInByte;

	w25qxx.interface_enable(true);

	if (w25qxx.type >= W25Q256){
		w25qxx.interface_write_byte(CMD_Fast_Read_4_Byte_Addr);
		w25qxx.interface_write_byte((page_addr & 0xFF000000) >> 24);
	}else{
		w25qxx.interface_write_byte(CMD_Fast_Read);
	}

	w25qxx.interface_write_byte((page_addr & 0xFF0000) >> 16);
	w25qxx.interface_write_byte((page_addr & 0xFF00) >> 8);
	w25qxx.interface_write_byte(page_addr & 0xFF);
	w25qxx.interface_write_byte(CMD_DUMMY);
	//HAL_SPI_Receive(&hspi_flash, pBuffer, NumByteToRead_up_to_PageSize, SPI_FLASH_TIMEOUT);

	w25qxx.interface_read(buff, NumByteToRead_up_to_PageSize);

	w25qxx.interface_enable(false);

	return true;
}



/** ############################################################################################
  * @brief read a sector from indicate sector-address
  * @param *pBuffer: [out] receive bytes
  * @param Sector_Address: [in] sector address (0 - W25Qxxx_SectorCount-1)
  * @param OffsetInByte: [in] offset byte number
  * @param NumByteToRead_up_to_SectorSize: [in] read byte number  max 4096Bytes
  * @retval status 0:passed   1:failed
  */
bool w25qxx_readSector(uint8_t *buff, uint32_t sector_addr, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize)
{
	uint32_t start_page;
	int32_t  remain_bytes;
	uint32_t local_offset;

	if ((NumByteToRead_up_to_SectorSize > w25qxx.sector_size) || 
											(NumByteToRead_up_to_SectorSize == 0))
		NumByteToRead_up_to_SectorSize = w25qxx.sector_size;
	
	if (OffsetInByte >= w25qxx.sector_size){
		return false;
	}

	if ((OffsetInByte + NumByteToRead_up_to_SectorSize) > w25qxx.sector_size){
		remain_bytes = w25qxx.sector_size - OffsetInByte;
	}else{
		remain_bytes = NumByteToRead_up_to_SectorSize;
	}
		
	start_page = w25qxx_sectorToPage(sector_addr) + (OffsetInByte / w25qxx.page_size);
	local_offset = OffsetInByte % w25qxx.page_size;
	
	do{
		w25qxx_readPage(buff, start_page, local_offset, remain_bytes);
		start_page++;
		remain_bytes -= w25qxx.page_size - local_offset;
		buff += w25qxx.page_size - local_offset;
		local_offset = 0;
	}while(remain_bytes > 0);

	return true;
}


/** 
  * @brief read a block bytes data from block-address
  * @param *pBuffer: [out] receive bytes
  * @param Block_Address: [in] sector address (0 - W25Qxxx_BLockCount-1)
  * @param OffsetInByte: [in] offset byte number
  * @param NumByteToRead_up_to_BlockSize: [in] read byte number  max 64KiBytes
  * @retval status 0:passed   1:failed
  */
bool w25qxx_readBlock(uint8_t *buff, uint32_t block_addr, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize)
{
	uint32_t start_page;
	int32_t remain_bytes;
	uint32_t local_offset;

	if ((NumByteToRead_up_to_BlockSize > w25qxx.block_size) || 
											(NumByteToRead_up_to_BlockSize == 0))
		NumByteToRead_up_to_BlockSize = w25qxx.block_size;

	if (OffsetInByte >= w25qxx.block_size){
		return false;
	}

	if ((OffsetInByte + NumByteToRead_up_to_BlockSize) > w25qxx.block_size)
		remain_bytes = w25qxx.block_size - OffsetInByte;
	else
		remain_bytes = NumByteToRead_up_to_BlockSize;
	start_page = w25qxx_blockToPage(block_addr) + (OffsetInByte / w25qxx.page_size);
	local_offset = OffsetInByte % w25qxx.page_size;
	do
	{
		w25qxx_readPage(buff, start_page, local_offset, remain_bytes);
		start_page++;
		remain_bytes -= w25qxx.page_size - local_offset;
		buff += w25qxx.page_size - local_offset;
		local_offset = 0;
	} while (remain_bytes > 0);

	return true;
}




static bool w25qxx_initCheck(void)
{
	uint32_t jedec_id;
	uint8_t  device_id;

	w25qxx_t type;

	while (w25qxx.get_time() < 20)
		w25qxx.delay(1);

	
	w25qxx.interface_enable(false);

	w25qxx.delay(20);

	w25qxx_powerUp();
	device_id = w25qxx_getManDeviceID();
	jedec_id = w25qxx_getJedecID();

	switch (jedec_id & 0x000000FF)
	{
		case 0x20: // 	W25Q512
			type = W25Q512;
			w25qxx.block_count = 1024;
			break;
		case 0x19: // 	W25Q256
			type = W25Q256;
			w25qxx.block_count = 512;
			break;
		case 0x18: // 	W25Q128
			type = W25Q128;
			w25qxx.block_count = 256;
			break;
		case 0x17: //	W25Q64
			type = W25Q64;
			w25qxx.block_count = 128;
			break;
		case 0x16: //	W25Q32
			type = W25Q32;
			w25qxx.block_count = 64;
			break;
		case 0x15: //	W25Q16
			type = W25Q16;
			w25qxx.block_count = 32;
			break;
		case 0x14: //	W25Q80
			type = W25Q80;
			w25qxx.block_count = 16;
			break;
		case 0x13: //	W25Q40
			type = W25Q40;
			w25qxx.block_count = 8;
			break;
		case 0x12: //	W25Q20
			type = W25Q20;
			w25qxx.block_count = 4;
			break;
		case 0x11: //	W25Q10
			type = W25Q10;
			w25qxx.block_count = 2;
			break;
		default:
			return false;
	}

	if((w25qxx.type != type) || (w25qxx.device_id != device_id)){
		return false;
	}


	w25qxx_getUniqID();

	uint8_t regVal = w25qxx_readRegX(1);

	if ((regVal&SR1_S0_BUSY) == SR1_S0_BUSY)
		return false;

	return true;
}


bool w25qxx_init(void)
{
	w25qxx.device_id = CMD_Device_ID;
	w25qxx.jedec_id = CMD_JEDEC_ID;
	w25qxx.man_device_id = CMD_Manufacture_ID;

	w25qxx.page_size = 256;			// 256  Byte
	w25qxx.sector_size = 0x1000;	// 4096 Byte
	w25qxx.sector_count = w25qxx.block_count*16;
	w25qxx.page_count = (w25qxx.sector_count * w25qxx.sector_size) / w25qxx.page_size;
	w25qxx.block_size = w25qxx.sector_size * 16;
	w25qxx.capacity_kb = (w25qxx.sector_count * w25qxx.sector_size) / 1024;

	if(!w25qxx_initCheck()){
		return false;
	}

}


w25q32_init_t* w25qxx_getStruct(void)
{
	return &w25qxx;
}
