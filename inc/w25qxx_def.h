#ifndef w25q32_def
#define w25q32_def


#define SPI_FLASH_TIMEOUT 				30 * 1000

#define CMD_DUMMY           			0x00
#define CMD_Reg_1_Write     			0x01
#define CMD_Page_Program				0x02
#define CMD_Reg_1_Read      			0x05
#define CMD_Fast_Read					0x0B
#define CMD_Reg_2_Read      			0x35
#define CMD_Reg_2_Write     			0x31 
#define CMD_Reg_3_Read      			0x15
#define CMD_Reg_3_Write     			0x11
#define CMD_Write_Disable   			0x04
#define CMD_Write_Enable    			0x06
#define CMD_Write_Enable_SR 			0x50
#define CMD_Device_ID       			0xAB
#define CMD_JEDEC_ID        			0x9F
#define CMD_Manufacture_ID  			0x90
#define CMD_Unique_ID       			0x4B
#define CMD_Erase_Chip      			0xC7
#define CMD_Erase_Sector				0x20
#define CMD_Erase_Block_64K 			0xD8

#define CMD_Erase_Sector_4_Byte_Addr 	0x21
#define CMD_Erase_Block_64K_4_Byte_Addr 0xDC
#define CMD_Page_Program_4_Byte_Addr	0x12
#define CMD_Fast_Read_4_Byte_Addr       0x0C


#define SR1_S0_BUSY			(unsigned char)(1<<0)
#define SR1_S1_WEL  		(unsigned char)(1<<1)
#define SR2_S9_QE 			(unsigned char)(1<<1)
#define SR2_S15_SUS 		(unsigned char)(1<<7)
#define SR3_S18_WPS 		(unsigned char)(1<<3)
#define SR3_S16_ADS 		(unsigned char)(1<<0)
#define SR3_S17_ADP 		(unsigned char)(1<<1)
#define SR_ADDR_EX_A24_EA0  (unsigned char)(1<<0)


typedef enum{
	W25Q10 = 1,
	W25Q20,
	W25Q40,
	W25Q80,
	W25Q16,
	W25Q32,
	W25Q64,
	W25Q128,
	W25Q256,
	W25Q512,
}w25qxx_t;


#endif