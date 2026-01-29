#ifndef __W25Q64_H
#define __W25Q64_H
 
#include "stm32f1xx_hal.h"
#include "gpio.h"
 
// ==== SPI 与引脚定义 ====
// 在 main.c 或其他初始化文件中定义实际使用的 SPI 句柄和 CS 引脚
#define W25Q64_SPI         hspi2
#define W25Q64_CS_GPIO     GPIOB
#define W25Q64_CS_PIN      GPIO_PIN_12
 
// ==== 指令码定义 ====
#define W25Q64_CMD_WRITE_ENABLE     0x06
#define W25Q64_CMD_WRITE_DISABLE    0x04
#define W25Q64_CMD_READ_STATUS1     0x05
#define W25Q64_CMD_READ_DATA        0x03
#define W25Q64_CMD_PAGE_PROGRAM     0x02
#define W25Q64_CMD_SECTOR_ERASE     0x20
#define W25Q64_CMD_CHIP_ERASE       0xC7
#define W25Q64_CMD_READ_ID          0x9F
 
// ==== 容量参数 ====
#define W25Q64_SECTOR_SIZE          4096
#define W25Q64_PAGE_SIZE            256
#define W25Q64_TOTAL_SIZE           (8 * 1024 * 1024) // 8MB
 
// ==== 函数声明 ====
uint32_t W25Q64_ReadID(void);
void     W25Q64_WriteEnable(void);
uint8_t  W25Q64_ReadStatus1(void);
void     W25Q64_WaitBusy(void);
void     W25Q64_ReadData(uint32_t addr, uint8_t* buf, uint32_t len);
void     W25Q64_PageProgram(uint32_t addr, const uint8_t* buf, uint32_t len);
void     W25Q64_SectorErase(uint32_t addr);
void     W25Q64_ChipErase(void);
 
#endif
