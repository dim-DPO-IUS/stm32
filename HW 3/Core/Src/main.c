/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// ===========================================================================
#include <string.h>
#include <stdio.h>
// ===========================================================================
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// ===========================================================================
#define FLASH_USER_START_ADDR   0x08060000  // Sector 7 start
#define FLASH_SECTOR            7           // Sector number

// String sizes
#define GSMCONFIG_PhoneNumLen     15
#define GSMCONFIG_SecretPINLen    4
#define GSMCONFIG_PINLen          4
#define GSMCONFIG_DeviceNameLen   16
#define GSMCONFIG_MessageLen      32

// Magic number for validation
#define CONFIG_MAGIC_NUM        0x12345678
// ==================== TIME STRUCTURE ====================
typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} Time_struct;

// ==================== CONFIG STRUCTURE ==================
typedef struct {
    // General
    uint32_t MagicNum;
    char MasterPhone[GSMCONFIG_PhoneNumLen + 1];
    char SecretPIN[GSMCONFIG_SecretPINLen + 1];
    char PIN[GSMCONFIG_PINLen + 1];
    char DeviceName[GSMCONFIG_DeviceNameLen];
    char GetBalanceUSSD[10];

    // Phones
    char PhoneNumber[5][GSMCONFIG_PhoneNumLen + 1];

    // Relay
    uint32_t RelayDelay[3];
    uint8_t  RelayBitmask[3];
    uint8_t  RelayBitmaskInv[3];
    uint8_t  RelayInv[3];

    // Information messages
    char StateMessage[2][7][GSMCONFIG_MessageLen + 1];
    uint8_t PhoneInfoMask[5];
    Time_struct Info_time;
    uint8_t Daily_info;
    uint8_t TimeShift;
} __attribute__((packed)) Config_t;

// ==================== UNION ====================
union NVRAM {
    Config_t config;
    uint32_t data32[(sizeof(Config_t) + 3) / 4];
} DevNVRAM;

// ===========================================================================
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
// ===========================================================================
Config_t myConfig;
uint32_t PageError = 0;
// ===========================================================================

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// ===========================================================================
// ==================== UART FUNCTIONS ====================
void UART_SendString(char *str) {
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

void UART_SendHexByte(uint8_t value) {
    char hex[4];
    sprintf(hex, "%02X", value);
    UART_SendString(hex);
}

// ==================== PRINT CONFIG ======================
void PrintConfig(Config_t *cfg) {
    char buf[128];
    int i, j;

    UART_SendString("\r\n========== CONFIG STRUCTURE ==========\r\n");

    // General
    sprintf(buf, "MagicNum:      0x%08lX\r\n", cfg->MagicNum);
    UART_SendString(buf);
    sprintf(buf, "MasterPhone:   %s\r\n", cfg->MasterPhone);
    UART_SendString(buf);
    sprintf(buf, "SecretPIN:     %s\r\n", cfg->SecretPIN);
    UART_SendString(buf);
    sprintf(buf, "PIN:           %s\r\n", cfg->PIN);
    UART_SendString(buf);
    sprintf(buf, "DeviceName:    %s\r\n", cfg->DeviceName);
    UART_SendString(buf);
    sprintf(buf, "GetBalanceUSSD: %s\r\n", cfg->GetBalanceUSSD);
    UART_SendString(buf);

    // Phone numbers
    UART_SendString("\r\n--- Phone Numbers ---\r\n");
    for(i = 0; i < 5; i++) {
        sprintf(buf, "Phone[%d]:      %s\r\n", i, cfg->PhoneNumber[i]);
        UART_SendString(buf);
    }

    // Relay settings
    UART_SendString("\r\n--- Relay Settings ---\r\n");
    for(i = 0; i < 3; i++) {
        sprintf(buf, "RelayDelay[%d]:     %lu\r\n", i, cfg->RelayDelay[i]);
        UART_SendString(buf);
        sprintf(buf, "RelayBitmask[%d]:   0x%02X\r\n", i, cfg->RelayBitmask[i]);
        UART_SendString(buf);
        sprintf(buf, "RelayBitmaskInv[%d]: 0x%02X\r\n", i, cfg->RelayBitmaskInv[i]);
        UART_SendString(buf);
        sprintf(buf, "RelayInv[%d]:       0x%02X\r\n", i, cfg->RelayInv[i]);
        UART_SendString(buf);
    }

    // State messages
    UART_SendString("\r\n--- State Messages ---\r\n");
    for(i = 0; i < 2; i++) {
        for(j = 0; j < 7; j++) {
            if(strlen(cfg->StateMessage[i][j]) > 0) {
                sprintf(buf, "StateMsg[%d][%d]: %s\r\n", i, j, cfg->StateMessage[i][j]);
                UART_SendString(buf);
            }
        }
    }

    // Phone info mask
    UART_SendString("\r\n--- Phone Info Mask ---\r\n");
    for(i = 0; i < 5; i++) {
        sprintf(buf, "PhoneInfoMask[%d]: 0x%02X\r\n", i, cfg->PhoneInfoMask[i]);
        UART_SendString(buf);
    }

    // Time info
    UART_SendString("\r\n--- Time Settings ---\r\n");
    sprintf(buf, "Info_time:   %02d:%02d:%02d %02d.%02d.%04d\r\n",
            cfg->Info_time.hour, cfg->Info_time.minute, cfg->Info_time.second,
            cfg->Info_time.day, cfg->Info_time.month, cfg->Info_time.year);
    UART_SendString(buf);
    sprintf(buf, "Daily_info:  %d\r\n", cfg->Daily_info);
    UART_SendString(buf);
    sprintf(buf, "TimeShift:   %d\r\n", cfg->TimeShift);
    UART_SendString(buf);

    sprintf(buf, "\r\nTotal size: %lu bytes\r\n", sizeof(Config_t));
    UART_SendString(buf);
}

// ==================== MEMORY DUMP ======================
void PrintMemoryDump(uint32_t address, uint32_t size) {
    uint8_t *ptr = (uint8_t*)address;
    char buf[64];

    UART_SendString("\r\n========== FLASH MEMORY DUMP ==========\r\n");
    sprintf(buf, "Address: 0x%08lX\r\n", address);
    UART_SendString(buf);
    sprintf(buf, "Size: %lu bytes\r\n\r\n", size);  // %lu вместо %d
    UART_SendString(buf);

    UART_SendString("Offset   00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F   ASCII\r\n");
    UART_SendString("-------- ------------------------------------------------  ----------------\r\n");

    for(uint32_t i = 0; i < size; i += 16) {
        sprintf(buf, "%08lX: ", address + i);
        UART_SendString(buf);

        for(uint32_t j = 0; j < 16; j++) {
            if(i + j < size) {
                UART_SendHexByte(ptr[i + j]);
                UART_SendString(" ");
            } else {
                UART_SendString("   ");
            }
            if(j == 7) UART_SendString(" ");
        }

        UART_SendString("  ");
        for(uint32_t j = 0; j < 16 && i + j < size; j++) {
            uint8_t c = ptr[i + j];
            char ascii[2] = {(c >= 32 && c <= 126) ? c : '.', 0};
            UART_SendString(ascii);
        }
        UART_SendString("\r\n");
    }
    UART_SendString("\r\n");
}

// ==================== FLASH OPERATIONS ==================
void SaveToFlash(void) {
    UART_SendString("\r\n--- Saving to Flash (Sector 7) ---\r\n");

    // Copy to union for 32-bit write
    memcpy(&DevNVRAM.config, &myConfig, sizeof(Config_t));

    // Unlock Flash
    HAL_FLASH_Unlock();

    // Erase sector
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector = FLASH_SECTOR;
    EraseInitStruct.NbSectors = 1;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    UART_SendString("Erasing sector 7... ");
    if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
        UART_SendString("FAILED\r\n");
        HAL_FLASH_Lock();
        return;
    }
    UART_SendString("OK\r\n");

    // Write data by 32-bit words
    UART_SendString("Writing... ");
    uint32_t address = FLASH_USER_START_ADDR;
    uint32_t dataSize = (sizeof(Config_t) + 3) / 4;

    for(uint32_t i = 0; i < dataSize; i++) {
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, DevNVRAM.data32[i]) != HAL_OK) {
            UART_SendString("FAILED\r\n");
            HAL_FLASH_Lock();
            return;
        }
        address += 4;
    }
    UART_SendString("OK\r\n");

    // Lock Flash
    HAL_FLASH_Lock();

    // Verify
    UART_SendString("Verifying... ");
    if(memcmp((void*)FLASH_USER_START_ADDR, &myConfig, sizeof(Config_t)) == 0) {
        UART_SendString("SUCCESS!\r\n");
        PrintConfig(&myConfig);
        PrintMemoryDump(FLASH_USER_START_ADDR, sizeof(Config_t));
    } else {
        UART_SendString("FAILED\r\n");
    }
}

void ReadFromFlash(void) {
    Config_t *flashConfig = (Config_t*)FLASH_USER_START_ADDR;

    if(flashConfig->MagicNum != 0xFFFFFFFF) {
        UART_SendString("\r\nFound saved data in Flash!\r\n");
        PrintConfig(flashConfig);
        PrintMemoryDump(FLASH_USER_START_ADDR, sizeof(Config_t));
    } else {
        UART_SendString("\r\nFlash sector 7 is empty (all 0xFF)\r\n");
    }
}

// ==================== INIT CONFIG WITH TEST DATA ========
void InitTestConfig(void) {
    int i, j;

    // General
    myConfig.MagicNum = CONFIG_MAGIC_NUM;
    strcpy(myConfig.MasterPhone, "+79991234567");
    strcpy(myConfig.SecretPIN, "0000");
    strcpy(myConfig.PIN, "1234");
    strcpy(myConfig.DeviceName, "NucleoF411");
    strcpy(myConfig.GetBalanceUSSD, "*100#");

    // Phone numbers
    strcpy(myConfig.PhoneNumber[0], "+79161234567");
    strcpy(myConfig.PhoneNumber[1], "+79162345678");
    strcpy(myConfig.PhoneNumber[2], "+79163456789");
    strcpy(myConfig.PhoneNumber[3], "+79164567890");
    strcpy(myConfig.PhoneNumber[4], "+79165678901");

    // Relay settings
    for(i = 0; i < 3; i++) {
        myConfig.RelayDelay[i] = (i + 1) * 1000;
        myConfig.RelayBitmask[i] = 1 << i;
        myConfig.RelayBitmaskInv[i] = ~(1 << i);
        myConfig.RelayInv[i] = i % 2;
    }

    // State messages
    strcpy(myConfig.StateMessage[0][0], "System OK");
    strcpy(myConfig.StateMessage[0][1], "Low battery");
    strcpy(myConfig.StateMessage[1][0], "Relay ON");
    strcpy(myConfig.StateMessage[1][1], "Relay OFF");

    // Fill remaining messages with empty strings
    for(i = 0; i < 2; i++) {
        for(j = 0; j < 7; j++) {
            if(strlen(myConfig.StateMessage[i][j]) == 0) {
                myConfig.StateMessage[i][j][0] = '\0';
            }
        }
    }

    // Phone info mask
    for(i = 0; i < 5; i++) {
        myConfig.PhoneInfoMask[i] = 0x01;
    }

    // Time info
    myConfig.Info_time.hour = 12;
    myConfig.Info_time.minute = 30;
    myConfig.Info_time.second = 0;
    myConfig.Info_time.day = 20;
    myConfig.Info_time.month = 3;
    myConfig.Info_time.year = 2026;
    myConfig.Daily_info = 1;
    myConfig.TimeShift = 3;
}
// ===========================================================================
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
  // ===========================================================================
  // Initialize config with test data
  InitTestConfig();
  // ===========================================================================
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
  // ===========================================================================
  char buf[64];
  UART_SendString("\r\n========== FLASH DEMO STM32F411 ==========\r\n");
  sprintf(buf, "Sector 7: 0x%08lX (128 KB)\r\n", FLASH_USER_START_ADDR);
  UART_SendString(buf);
  sprintf(buf, "Config size: %lu bytes\r\n", sizeof(Config_t));
  UART_SendString(buf);
  UART_SendString("Press BLUE button (PC13) to save to Flash\r\n");
  UART_SendString("===========================================\r\n");

  // Show current config
  UART_SendString("\r\nCurrent test config:");
  PrintConfig(&myConfig);

  // Check if Flash has saved data
  ReadFromFlash();
  // ===========================================================================
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      // Check button press (PC13)
       if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
           HAL_Delay(50);  // Debounce
           if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
               // Update timestamp in config (using Info_time as example)
               myConfig.Info_time.hour = (HAL_GetTick() / 3600000) % 24;
               myConfig.Info_time.minute = (HAL_GetTick() / 60000) % 60;
               myConfig.Info_time.second = (HAL_GetTick() / 1000) % 60;

               SaveToFlash();

               // Wait for button release
               while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET);
               UART_SendString("\r\nPress button again to save updated config\r\n");
           }
       }

       // Blink LED
       HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
       HAL_Delay(500);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
