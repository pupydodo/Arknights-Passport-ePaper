#include "w25q64.h"
 
extern SPI_HandleTypeDef W25Q64_SPI;
 
// === 片选控制 ===
static void W25Q64_CS_LOW(void) {
    HAL_GPIO_WritePin(W25Q64_CS_GPIO, W25Q64_CS_PIN, GPIO_PIN_RESET);
}
static void W25Q64_CS_HIGH(void) {
    HAL_GPIO_WritePin(W25Q64_CS_GPIO, W25Q64_CS_PIN, GPIO_PIN_SET);
}
 
// === 写使能 ===
void W25Q64_WriteEnable(void) {
    uint8_t cmd = W25Q64_CMD_WRITE_ENABLE;
    W25Q64_CS_LOW();
    HAL_SPI_Transmit(&W25Q64_SPI, &cmd, 1, HAL_MAX_DELAY);
    W25Q64_CS_HIGH();
}
 
// === 读取状态寄存器1 ===
uint8_t W25Q64_ReadStatus1(void) {
    uint8_t cmd = W25Q64_CMD_READ_STATUS1;
    uint8_t status;
    W25Q64_CS_LOW();
    HAL_SPI_Transmit(&W25Q64_SPI, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&W25Q64_SPI, &status, 1, HAL_MAX_DELAY);
    W25Q64_CS_HIGH();
    return status;
}
 
// === 等待空闲（WIP=0） ===
void W25Q64_WaitBusy(void) {
    while (W25Q64_ReadStatus1() & 0x01);
}
 
// === 读取 JEDEC ID ===
uint32_t W25Q64_ReadID(void) {
    uint8_t cmd = W25Q64_CMD_READ_ID;
    uint8_t id_buf[3];
    W25Q64_CS_LOW();
    HAL_SPI_Transmit(&W25Q64_SPI, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&W25Q64_SPI, id_buf, 3, HAL_MAX_DELAY);
    W25Q64_CS_HIGH();
    return (id_buf[0] << 16) | (id_buf[1] << 8) | id_buf[2];
}
 
// === 读取数据 ===
void W25Q64_ReadData(uint32_t addr, uint8_t* buf, uint32_t len) {
    uint8_t cmd[4];
    cmd[0] = W25Q64_CMD_READ_DATA;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;
    
    W25Q64_CS_LOW();
    HAL_SPI_Transmit(&W25Q64_SPI, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Receive(&W25Q64_SPI, buf, len, HAL_MAX_DELAY);
    W25Q64_CS_HIGH();
}
 
// === 页编程（最大 256 字节，不能跨页） ===
void W25Q64_PageProgram(uint32_t addr, const uint8_t* buf, uint32_t len) {
    if (len > W25Q64_PAGE_SIZE) len = W25Q64_PAGE_SIZE;
 
    W25Q64_WriteEnable();
 
    uint8_t cmd[4];
    cmd[0] = W25Q64_CMD_PAGE_PROGRAM;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;
 
    W25Q64_CS_LOW();
    HAL_SPI_Transmit(&W25Q64_SPI, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&W25Q64_SPI, (uint8_t*)buf, len, HAL_MAX_DELAY);
    W25Q64_CS_HIGH();
 
    W25Q64_WaitBusy();
}
 
// === 扇区擦除（按4KB） ===
void W25Q64_SectorErase(uint32_t addr) {
    W25Q64_WriteEnable();
 
    uint8_t cmd[4];
    cmd[0] = W25Q64_CMD_SECTOR_ERASE;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;
 
    W25Q64_CS_LOW();
    HAL_SPI_Transmit(&W25Q64_SPI, cmd, 4, HAL_MAX_DELAY);
    W25Q64_CS_HIGH();
 
    W25Q64_WaitBusy();
}
 
// === 整片擦除 ===
void W25Q64_ChipErase(void) {
    W25Q64_WriteEnable();
 
    uint8_t cmd = W25Q64_CMD_CHIP_ERASE;
    W25Q64_CS_LOW();
    HAL_SPI_Transmit(&W25Q64_SPI, &cmd, 1, HAL_MAX_DELAY);
    W25Q64_CS_HIGH();
 
    W25Q64_WaitBusy();
}
