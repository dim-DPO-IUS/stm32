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
// =======================================================================
#include "crypto.h"
// =======================================================================
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// =======================================================================
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
// =======================================================================
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
// =======================================================================

/*Это тот "текст", который нужно защитить.
 * Это 32 байта данных, каждый из которых равен 0x44 (буква D в ASCII)*/
const uint8_t PlainText[] =
  {
    0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
    0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
  };
/*Это секретная информация, которая определяет, как именно будет зашифрован текст.
 * Разные ключи = разный способ запутывания.
 * Ключ знает только отправитель и получатель → безопасно*/
const uint8_t Key[] =
  {
    0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
  };
/* Tweak (Sequence Number) for XTS: 16 bytes */
/*Это дополнительный параметр, чтобы одинаковые блоки данных шифровались по-разному*/
const uint8_t SequenceNumber[] =
  {
    0x33, 0x33, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };
/* Expected ciphertext (from IEEE P1619 test vector) */
/*Это результат шифрования — бессмысленный набор байтов.*/
const uint8_t CipherText[] =
  {
    0xaf, 0x85, 0x33, 0x6b, 0x59, 0x7a, 0xfc, 0x1a, 0x90, 0x0b, 0x2e, 0xb2, 0x1e, 0xc9, 0x49, 0xd2,
    0x92, 0xdf, 0x4c, 0x04, 0x7e, 0x0b, 0x21, 0x53, 0x21, 0x86, 0xa5, 0x97, 0x1a, 0x22, 0x7a, 0x89,
  };

int32_t check; /* check whether obtained ciphertext during encryption
  * and obtained plaintext during decryption are equal
  * to expected vectors */

uint8_t temp[512]; /* temporary buffer to store result */
static GPIO_InitTypeDef  GPIO_InitStruct;
// =======================================================================
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
// =======================================================================
int32_t STM32_AES_XTS_Encrypt(const uint8_t *InputMessage,
                              uint32_t   InputMessageLength,
                              const uint8_t *AES_Key,
                              uint32_t   KeyLength,
                              const uint8_t *Tweak,
                              uint8_t   *OutputMessage);

int32_t STM32_AES_XTS_Decrypt(const uint8_t *InputMessage,
                              uint32_t   InputMessageLength,
                              const uint8_t *AES_Key,
                              uint32_t   KeyLength,
                              const uint8_t *Tweak,
                              uint8_t   *OutputMessage);

TestStatus Buffercmp(const uint8_t* pBuffer,
                     uint8_t* pBuffer1,
                     uint16_t BufferLength);
//static void SystemClock_Config(void);
static void Led_Config(void);
static void Led_Toggle( __IO uint32_t toggle_delay);
//static void Error_Handler(void);
// =======================================================================
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
// =======================================================================
	int32_t status = AES_SUCCESS;
// =======================================================================
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
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
  // =======================================================================
  /* Configure the green led */
  Led_Config();

  /* Toggle the green led before starting the algorithm */
  Led_Toggle(500);

  /* Enable CRC clock */
  __CRC_CLK_ENABLE();

  /* Encrypt DATA with AES in XTS mode */
  status = STM32_AES_XTS_Encrypt(PlainText, sizeof(PlainText), Key, CRL_AES128_KEY, SequenceNumber, temp);

  if (status ==  AES_SUCCESS)
  {
    /* do a memory compare for check accordance between calculated ciphertext
    * and expected ciphertext */
    if (Buffercmp(CipherText, temp, CRL_AES_BLOCK) == PASSED)
    {
    	/* Encryption OK */
    }
    else
    {
      Error_Handler();
    }
  }
  else
  {
    Error_Handler();
  }

  /* Decrypt DATA with AES in XTS mode */
  status = STM32_AES_XTS_Decrypt(CipherText, sizeof(CipherText), Key,
                                 CRL_AES128_KEY, SequenceNumber, temp);

  if (status == AES_SUCCESS)
  {
    /* do a memory compare for check accordance between calculated plaintext
    * and expected plaintext */
	  /* Compare FULL plaintext */
    if (Buffercmp(PlainText, temp, CRL_AES_BLOCK) == PASSED)
    {
    	/* Decryption OK */
    }
    else
    {
      Error_Handler();
    }

  }
  else
  {
    Error_Handler();
  }
  // =======================================================================
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // =======================================================================
  /* Turn on the green led in case of AES ECB operations are succssfuls*/
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
  // =======================================================================
  while (1)
  {
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
// =======================================================================
/**
  * @brief  AES-128-XTS Encryption
  * @param  InputMessage: pointer to input message to be encrypted
  * @param  InputMessageLength: input data message length in byte
  * @param  AES_Key: pointer to the AES key to be used in the operation
  * @param  KeyLength: length of the AES key
  * @param  Tweak: Initialization vector
  * @param  OutputMessage: pointer to output parameter that will handle the encrypted message
  * @retval error status: can be AES_SUCCESS if success or one of
  *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
  *         AES_ERR_BAD_PARAMETER if error occured.
  */
int32_t STM32_AES_XTS_Encrypt(const uint8_t *InputMessage,
                              uint32_t   InputMessageLength,
                              const uint8_t *AES_Key,
                              uint32_t   KeyLength,
                              const uint8_t *Tweak,
                              uint8_t   *OutputMessage)
{
	/*Инициализация контекста*/
  /* AES context, error status and output length */
  AESXTSctx_stt AESctx;
  uint32_t error_status = AES_SUCCESS;
  int32_t outputLength = 0;

  /* Set flag field to default value */
  AESctx.mFlags = E_SK_DEFAULT; // стандартные настройки
  /* Set the key size in AES status */
  AESctx.mKeySize = KeyLength; // CRL_AES128_KEY = 32 байта (256 бит для XTS)
  /* Set the Tweak size in AES status */
  AESctx.mTweakSize = CRL_AES_BLOCK; // 16 байт (размер tweak)
  /* Initialize the operation, by passing key and Tweak */
  error_status = AES_XTS_Encrypt_Init(&AESctx, AES_Key, Tweak);

  /* check for initialization errors */
  if (error_status == AES_SUCCESS)
  {
    /* Encrypt Data in XTS mode */
    error_status = AES_XTS_Encrypt_Append(&AESctx, InputMessage,
                                          InputMessageLength, OutputMessage, &outputLength);

    /* check for encryption errors in XTS mode */
    if (error_status == AES_SUCCESS)
    {
      /* Finalize data */
      error_status = AES_XTS_Encrypt_Finish(&AESctx, OutputMessage,
                                            &outputLength);
    }
  }

  return error_status;
}


/**
  * @brief  AES-128-XTS Decryption
  * @param  InputMessage: pointer to input message to be decrypted
  * @param  InputMessageLength: input data message length in byte
  * @param  AES_Key: pointer to the AES key to be used in the operation
  * @param  KeyLength: length of the AES key
  * @param  Tweak: Initialization vector
  * @param  OutputMessage: pointer to output parameter that will handle the decrypted message
  * @retval error status: can be AES_SUCCESS if success or one of
  *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
  *         AES_ERR_BAD_PARAMETER if error occured.
  */
int32_t STM32_AES_XTS_Decrypt(const uint8_t *InputMessage,
                              uint32_t   InputMessageLength,
                              const uint8_t *AES_Key,
                              uint32_t   KeyLength,
                              const uint8_t *Tweak,
                              uint8_t   *OutputMessage)
{
  /* AES context, error status and output length */
  AESXTSctx_stt AESctx;
  uint32_t error_status = AES_SUCCESS;
  int32_t outputLength = 0;

  /* Set flag field to default value */
  AESctx.mFlags = E_SK_DEFAULT;
  /* Set key size in AES status */
  AESctx.mKeySize = KeyLength;
  /* Set the Tweak size in AES status */
  AESctx.mTweakSize = CRL_AES_BLOCK;
  /* Initialize the operation, by passing the key and Tweak */
  error_status = AES_XTS_Decrypt_Init(&AESctx, AES_Key, Tweak);

  /* check for initialization errors */
  if (error_status == AES_SUCCESS)
  {
    /* Decrypt Data in XTS mode */
    error_status = AES_XTS_Decrypt_Append(&AESctx, InputMessage,
                                          InputMessageLength, OutputMessage, &outputLength);

    /* check for decryption errors in XTS mode */
    if (error_status == AES_SUCCESS)
    {
      /* Finalize data */
      error_status = AES_XTS_Decrypt_Finish(&AESctx, OutputMessage,
                                            &outputLength);
    }
  }

  return error_status;
}


TestStatus Buffercmp(const uint8_t* pBuffer, uint8_t* pBuffer1, uint16_t BufferLength)
{
  while (BufferLength--)
  {
    if (*pBuffer != *pBuffer1)
    {
      return FAILED;
    }

    pBuffer++;
    pBuffer1++;
  }

  return PASSED;
}

/**
  * @brief  Configures the green led.
  * @param  None
  * @retval None
  */
static void Led_Config(void)
{
  /* -1- Enable each GPIO Clock (to be able to program the configuration registers) */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* -2- Configure IOs in output push-pull mode to drive external LEDs */
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
  * @brief  Toggles the green led three times.
  * @param[in]  toggle_delay : toggle frequency
  * @retval None
  */
static void Led_Toggle(__IO uint32_t toggle_delay)
{
    for (int i = 0; i < 6; i++)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(toggle_delay);
    }
}
// =======================================================================
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
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      HAL_Delay(250);
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      HAL_Delay(250);
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
