/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "spi.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "EPD_Test.h"
#include "w25q64.h"
#include "EPD_3in6e.h"
#include "fatfs.h"
#include "DEV_Config.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define READ_MODE 0
#define USB_MODE 1

int16_t mode=READ_MODE;

void EPD_Show_File(const char* filename);
void EPD_Test_Color(UBYTE color);

int current_photo_id = 1; 
uint8_t Check_Double_Click(void);

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  // MX_USB_DEVICE_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */

  // EPD_Test();
  uint32_t flash_id = W25Q64_ReadID();
  printf("Flash ID: 0x%X\r\n", flash_id);
  if(flash_id != 0xEF4017) {
    // EPD_Test();
      printf("flash id error\r\n");
  }
  HAL_Delay(1000);

  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == 1){
    mode = USB_MODE;
    printf("Mode: USB\r\n");
  } else {
    mode = READ_MODE;
    printf("Mode: READ\r\n");
  }

  if(mode == USB_MODE) {
    // GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 1. 强行接管 PA12 (USB D+)，配置为推挽输出
    // __HAL_RCC_GPIOA_CLK_ENABLE();
    // GPIO_InitStruct.Pin = GPIO_PIN_12;
    // GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    // GPIO_InitStruct.Pull = GPIO_NOPULL;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // // 2. 拉低电平 (告诉电脑：我拔掉了)
    // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
    
    // // 3. 延时 (给电脑一点反应时间，通常 500ms - 1000ms)
    // HAL_Delay(1000);

    MX_USB_DEVICE_Init();
    printf("USB Device Initialized\r\n");
  }else{
    // === 模式 B：FatFs 读取模式 (显示图片) ===
        printf("Mode: READ\r\n");

        // 1. 定义文件系统对象
        FATFS fs;
        FRESULT res;

        // 2. 挂载文件系统 (这一步是必须的！)
        // 参数说明: &fs是对象, "0:"是驱动号, 1表示立即挂载并检查
        printf("Mounting Filesystem...\r\n");
        res = f_mount(&fs, "0:", 1);

        if (res == FR_OK) 
        {
            printf("Mount Success!\r\n");
            
            // 3. 只有挂载成功了，才能读取文件显示
            // 调用我们之前写好的流式显示函数
            // EPD_Show_File("out.bin");
            // EPD_Test_Color(0x03);
            
            // 4. 显示完成后卸载 (可选)
            // f_mount(NULL, "0:", 0);
            while(1){
              if(Check_Double_Click()){
                printf("Double Click Detected!\r\n");
                
                // 2. 尝试寻找下一张图
                // 逻辑：尝试打开 "当前ID + 1" 的文件
                // 如果存在 -> ID加1
                // 如果不存在 -> ID重置为1
                
                int next_id = current_photo_id + 1;
                char filename[20];
                sprintf(filename, "%d.bin", next_id); // 拼接文件名，例如 "2.out"
                
                FIL test_file;
                if(f_open(&test_file, filename, FA_READ) == FR_OK) 
                {
                    // 下一张文件存在，使用它
                    f_close(&test_file);
                    current_photo_id = next_id;
                }
                else
                {
                    // 下一张文件不存在，回到第一张
                    printf("Next file %s not found. Loop back to 1.bin\r\n", filename);
                    current_photo_id = 1;
                    sprintf(filename, "1.bin");
                }
                
                // 3. 刷新屏幕
                // 注意：刷新过程需要几十秒，这期间按键是不起作用的（正好防抖）
                EPD_Show_File(filename);
                
                printf("Display Done. Waiting for next command...\r\n");
              }
              HAL_Delay(10);
            }
        } 
        else 
        {
            // 如果挂载失败 (例如错误 13 FR_NO_FILESYSTEM)，说明 U 盘没格式化
            printf("Mount Failed! Error: %d\r\n", res);
            printf("Please format disk to FAT/FAT32 via USB mode.\r\n");
        }
        
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1) {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
			HAL_Delay(10000);

    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


// 缓冲区：4KB (太小会慢，太大会爆栈)
#define BUFFER_SIZE 4096 
uint8_t img_buffer[BUFFER_SIZE];

/* * 核心功能：分块读取 FatFs 文件并直接刷入墨水屏
 */
void EPD_Show_File(const char* filename)
{
    FIL file;
    FRESULT res;
    UINT br;
    uint32_t total_sent = 0;
    // 3.6寸屏显存: 400x600像素 / 2 = 120,000 字节
    const uint32_t TOTAL_SIZE = 120000; 

    printf("\r\n=== Start Showing: %s ===\r\n", filename);

    // 1. 【关键】模块级初始化 (开启电源 PWR 引脚)
    // 参考 EPD_Test: 第一步必须是这个
    if(DEV_Module_Init() != 0) {
        printf("DEV_Module_Init Failed!\r\n");
        return;
    }

    // 2. 墨水屏硬件复位与初始化
    printf("EPD Init...\r\n");
    EPD_3IN6E_Init();
    
    // (可选) 如果想先清屏，可以解开下面这行，但会增加一次闪烁
    // EPD_3IN6E_Clear(EPD_3IN6E_WHITE);

    // 3. 打开文件
    res = f_open(&file, filename, FA_READ);
    if(res != FR_OK) {
        printf("Open File Failed! Res=%d\r\n", res);
        DEV_Module_Exit(); // 失败也要记得关电源
        return;
    }

    // 4. 发送“写显存”命令
    // EPD_3IN6E 使用 0x10 命令开始传输图像数据
    // 注意：需要使用我们之前暴露出来的公开接口，或者修改 static
    // 如果还没修改驱动，请参照上文修改 EPD_3in6e.c 去掉 static
    EPD_3IN6E_Write_Command(0x10); 

    printf("Streaming Data...\r\n");

    // 5. 流式传输 (Chunked Transfer)
    while(total_sent < TOTAL_SIZE)
    {
        // 计算本次读取大小
        UINT to_read = sizeof(img_buffer);
        if(TOTAL_SIZE - total_sent < to_read) {
            to_read = TOTAL_SIZE - total_sent;
        }

        // 读文件
        res = f_read(&file, img_buffer, to_read, &br);
        
        // 校验
        if(res != FR_OK || br == 0) {
            printf("Read Error or EOF! Sent: %d\r\n", total_sent);
            break;
        }

        // 【核心发送逻辑】
        // 模拟 EPD_3IN6E_Display 内部的发送行为
        DEV_Digital_Write(EPD_DC_PIN, 1); // DC=1 (数据)
        DEV_Digital_Write(EPD_CS_PIN, 0); // CS=0 (选中)
        
        // 使用 HAL 库批量发送，速度快且稳定
        DEV_SPI_Write_nByte(img_buffer, br);
        
        DEV_Digital_Write(EPD_CS_PIN, 1); // CS=1 (结束本次)

        total_sent += br;
    }

    printf("Transmission Done. Total: %d bytes\r\n", total_sent);
    f_close(&file);

    // 6. 刷新屏幕
    // EPD_3IN6E_Display() 内部调用的是 TurnOnDisplay
    // 我们手动调用它来触发刷新
    printf("Refreshing...\r\n");
    
    // 如果 EPD_3IN6E_TurnOnDisplay 还是 static，请去 .c 文件删掉 static
    EPD_3IN6E_Refresh(); 

    // 7. 休眠与断电 (参考 EPD_Test 结尾)
    printf("Sleep & Power Off...\r\n");
    EPD_3IN6E_Sleep();
    DEV_Delay_ms(2000); // 等待刷新完成再断电
    DEV_Module_Exit();  // 关闭 PWR 引脚，省电
}

// 记录当前显示的是第几张图
// int current_photo_id = 1; 

// 按键引脚定义 (请根据你的实际引脚修改，假设是 PA0)
#define KEY_GPIO_PORT button_GPIO_Port
#define KEY_PIN       button_Pin

/**
 * @brief 检测按键是否被双击
 * @return 1: 双击事件发生, 0: 无事发生
 */
uint8_t Check_Double_Click(void)
{
    static uint32_t last_click_time = 0;
    static uint8_t key_released = 1; // 记录按键是否松开过
    
    // 1. 读取按键状态 (假设按下是高电平 1，如果是低电平请改为 == 0)
    if(HAL_GPIO_ReadPin(KEY_GPIO_PORT, KEY_PIN) == GPIO_PIN_SET) 
    {
        // 只有当按键之前是松开状态，现在被按下，才算一次“点击”
        if(key_released == 1) 
        {
            key_released = 0; // 标记为已按下
            uint32_t now = HAL_GetTick();
            
            // 计算距离上次点击的时间差
            if(now - last_click_time < 500) // 500ms 内连按两次
            {
                last_click_time = 0; // 重置时间，防止三连击触发两次
                return 1; // 触发双击！
            }
            else
            {
                last_click_time = now; // 更新本次点击时间
            }
        }
    }
    else
    {
        key_released = 1; // 按键松开了
    }
    
    return 0;
}
/* USER CODE END 0 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    while(1) {
			printf("_Error_Handler\r\n");
    }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
