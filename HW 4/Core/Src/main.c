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
#include "stm32f4xx_ll_rcc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* ------------------- НАСТРОЙКИ IWDG (сторожевой таймер) ------------------- */
// ============================================================================

/* ------------------- НАСТРОЙКИ LED ИНДИКАЦИИ ----------------------------- */
/* LED2 на NUCLEO-F411RE подключен к PA5 */
#define LED_PORT                GPIOA
#define LED_PIN                 GPIO_PIN_5

/* ------------------- НАСТРОЙКИ КНОПКИ ------------------------------------ */
/* Пользовательская кнопка на NUCLEO-F411RE подключена к PC13 */
#define BUTTON_PORT             GPIOC
#define BUTTON_PIN              GPIO_PIN_13
#define BUTTON_PRESSED          GPIO_PIN_RESET      /* Кнопка замыкает на GND */
#define BUTTON_DEBOUNCE_MS      50                  /* Антидребезг, мс */

/* Настройки таймингов */
#define MAIN_LOOP_PERIOD_MS     1       /* Частота основного цикла */
#define BUTTON_POLL_PERIOD_MS   10      /* Опрос кнопки каждые 10 мс */
#define DEBOUNCE_TIME_MS        50      /* Время антидребезга */
#define RELEASE_TIMEOUT_MS      2000    /* Макс. время удержания кнопки */
// ============================================================================

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
// ============================================================================

/* Структура для хранения сырых битов регистра CSR (для отладки) */
typedef struct
{
    uint32_t raw_value;      /* Полное значение регистра CSR */

    /* Биты состояния LSI (биты 0-1) */
    uint8_t lsion_0;           /* Бит 0 - LSI включен */
    uint8_t lsirdy_1;          /* Бит 1 - LSI готов */

    /* Флаги сброса (биты 25-31) */
    uint8_t borrst_25;          /* Бит 25 - BOR reset flag */
    uint8_t pinrst_26;          /* Бит 26 - PIN reset flag (кнопка Reset) */
    uint8_t porrst_27;          /* Бит 27 - POR/PDR reset flag (включение питания) */
    uint8_t sftrst_28;          /* Бит 28 - Software reset flag */
    uint8_t iwdgrst_29;          /* Бит 29 - IWDG reset flag */
    uint8_t wwdgrst_30;         /* Бит 30 - WWDG reset flag */
    uint8_t lpwrrst_31;         /* Бит 31 - Low-power reset flag */

    /* Бит управления */
    uint8_t rmvf_24;            /* Бит 24 - Remove reset flag (очистка) */
} RawCSR_t;

RawCSR_t g_csr = {0};    /* Для отладки: сырые флаги ДО очистки */

/* Таймеры для неблокирующей работы (в мс) */
typedef struct
{
    uint32_t main_loop;      /* Для периодических задач в main */
    uint32_t button_check;   /* Для опроса кнопки */
    uint32_t debounce_start; /* Начало дебаунса */
    uint32_t release_wait;   /* Таймаут ожидания отпускания */

    /* Флаги состояния кнопки */
    uint8_t button_state;    /* 0=отпущена, 1=нажата, 2=дебаунс, 3=ожидание отпускания */
    uint8_t feed_requested;  /* Флаг: нужно покормить IWDG */

} Timing_t;

Timing_t g_tim = {0};


/* Переменные для неблокирующего мигания LED */
static uint32_t last_led_toggle = 0;
static uint8_t led_state = 0;


// ============================================================================
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */
// ============================================================================
static void Read_Raw_CSR(RawCSR_t *raw);
//static void Check_And_Feed_Watchdog(void);
void Error_Handler(void);
static uint8_t Check_Button_Simple(void);

// ============================================================================

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
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */

  // ===========================================================================
  /* Чтение сырых флагов CSR ДО очистки (для отладки) */
  Read_Raw_CSR(&g_csr);

  /* LED индикация причины сброса (при старте) */
  /* LED индикация причины сброса (неблокирующая версия) */
  if (g_csr.iwdgrst_29)
  {
      uint32_t blink_start = HAL_GetTick();
      uint8_t blink_count = 0;
      uint8_t blink_state = 0;

      while (blink_count < 6)  /* 3 вспышки = 6 переключений */
      {
          uint32_t now = HAL_GetTick();
          if (now - blink_start >= 100)  /* 100 мс на состояние */
          {
              blink_start = now;
              blink_state = !blink_state;
              HAL_GPIO_WritePin(LED_PORT, LED_PIN, blink_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
              if (blink_state == 0) blink_count++;  /* Считаем только выключения */
          }
      }
      HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);  /* Гарантированно выключить */
  }

  // ===========================================================================

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // ===========================================================================
  /* Инициализация переменных для неблокирующего тайминга */
  last_led_toggle = HAL_GetTick();

  while (1)
  {
      uint32_t now = HAL_GetTick();

      /* 1. Проверка кнопки и кормление watchdog*/
      if (Check_Button_Simple()) {
          HAL_IWDG_Refresh(&hiwdg);

          /* Визуальный отклик: короткая вспышка */
          HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
          /* Сбросим отклик через 50 мс в следующем шаге */
      }

      /* 2. Периодическое мигание LED (индикация работы) - 1 Гц */
      if (now - last_led_toggle >= 500)
      {
          last_led_toggle = now;
          led_state = !led_state;

          /* Меняем LED только если нет активного отклика на кнопку */
          /* (отклик сбрасывается автоматически при следующем проходе) */
          HAL_GPIO_WritePin(LED_PORT, LED_PIN, led_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
      }


      // ===========================================================================
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

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
// ===========================================================================

/* ============================================================================
 * ФУНКЦИЯ ЧТЕНИЯ СЫРЫХ ФЛАГОВ CSR (ДЛЯ ОТЛАДКИ)
 * ============================================================================ */
/**
 * @brief  Чтение регистра RCC_CSR и заполнение структуры RawCSR_t
 * @param  raw  Указатель на структуру для заполнения
 */
static void Read_Raw_CSR(RawCSR_t *raw)
{
    uint32_t csr = RCC->CSR;

    raw->raw_value = csr;

    /* Биты LSI (биты 0-1) */
    raw->lsion_0  = (csr & RCC_CSR_LSION) ? 1 : 0;
    raw->lsirdy_1 = (csr & RCC_CSR_LSIRDY) ? 1 : 0;

    /* Флаги сброса (биты 25-31) */
    raw->borrst_25  = (csr & RCC_CSR_BORRSTF) ? 1 : 0;   /* Бит 25 */
    raw->pinrst_26  = (csr & RCC_CSR_PINRSTF) ? 1 : 0;   /* Бит 26 */
    raw->porrst_27  = (csr & RCC_CSR_PORRSTF) ? 1 : 0;   /* Бит 27 */
    raw->sftrst_28  = (csr & RCC_CSR_SFTRSTF) ? 1 : 0;   /* Бит 28 */
    raw->iwdgrst_29 = (csr & RCC_CSR_IWDGRSTF) ? 1 : 0;  /* Бит 29 */
    raw->wwdgrst_30 = (csr & RCC_CSR_WWDGRSTF) ? 1 : 0;  /* Бит 30 */
    raw->lpwrrst_31 = (csr & RCC_CSR_LPWRRSTF) ? 1 : 0;  /* Бит 31 */

    /* Бит управления */
    raw->rmvf_24 = (csr & RCC_CSR_RMVF) ? 1 : 0;          /* Бит 24 */

    /* Очистка флагов сброса для следующего запуска */
    LL_RCC_ClearResetFlags();
}

/* ============================================================================
 * ФУНКЦИИ РАБОТЫ СО СТОРОЖЕВЫМ ТАЙМЕРОМ
 * ============================================================================ */

/**
 * @brief Упрощённая неблокирующая проверка кнопки с антидребезгом
 * @retval 1 если кнопка была корректно нажата (можно кормить IWDG)
 * @note Вызывать каждые ~10 мс из основного цикла
 */
static uint8_t Check_Button_Simple(void)
{
	/* Упрощённая обработка кнопки */
	static uint8_t last_btn = 1;        /* 1 = отпущена (pull-up) */
	static uint8_t debounce_cnt = 0;    /* Счётчик антидребезга */

    uint8_t cur = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);

    /* Если состояние изменилось — сбрасываем счётчик дебаунса */
    if (cur != last_btn) {
        debounce_cnt = 0;
        last_btn = cur;
    }
    /* Если состояние стабильно — считаем */
    else {
        /* Считаем только если кнопка нажата */
        if (cur == BUTTON_PRESSED) {
            if (++debounce_cnt >= 5) {  /* 5 * ~10мс = 50мс дебаунс */
                debounce_cnt = 5;        /* Защита от переполнения */
                last_btn = 1;            /* Сброс для следующего нажатия */
                return 1;                /* Нажатие подтверждено! */
            }
        } else {
            /* Кнопка отпущена — сбрасываем счётчик */
            debounce_cnt = 0;
        }
    }
    return 0;  /* Нажатие не подтверждено */
}

// ===========================================================================
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
      HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
      HAL_Delay(100);
      HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
      HAL_Delay(100);
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
