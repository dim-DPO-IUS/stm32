#include "bootloader.h"
#include "main.h"

//extern IWDG_HandleTypeDef hiwdg;

/**
 * @brief Чтение регистра RCC_CSR и заполнение структуры
 */
void Read_Raw_CSR(RawCSR_t* csr)
{
    csr->raw_value = RCC->CSR;

    csr->lsion   = (csr->raw_value & RCC_CSR_LSION) ? 1 : 0;
    csr->lsirdy  = (csr->raw_value & RCC_CSR_LSIRDY) ? 1 : 0;
    csr->borrst  = (csr->raw_value & RCC_CSR_BORRSTF) ? 1 : 0;
    csr->pinrst  = (csr->raw_value & RCC_CSR_PINRSTF) ? 1 : 0;
    csr->porrst  = (csr->raw_value & RCC_CSR_PORRSTF) ? 1 : 0;
    csr->sftrst  = (csr->raw_value & RCC_CSR_SFTRSTF) ? 1 : 0;
    csr->iwdgrst = (csr->raw_value & RCC_CSR_IWDGRSTF) ? 1 : 0;
    csr->wwdgrst = (csr->raw_value & RCC_CSR_WWDGRSTF) ? 1 : 0;
    csr->lpwrrst = (csr->raw_value & RCC_CSR_LPWRRSTF) ? 1 : 0;
    csr->rmvf    = (csr->raw_value & RCC_CSR_RMVF) ? 1 : 0;
}

/**
 * @brief Переход в System Bootloader (USB DFU режим)
 */
void JumpToSystemBootloader(void)
{
    __disable_irq();
    HAL_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // Настройка HSI для System Bootloader
    RCC->CR |= RCC_CR_HSION;
    while(!(RCC->CR & RCC_CR_HSIRDY));
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);

    void (*SysBootLoader)(void);
    SysBootLoader = (void (*)(void))(*((uint32_t *)(SYSTEM_BOOTLOADER_ADDR + 4)));

    __set_MSP(*(uint32_t *)SYSTEM_BOOTLOADER_ADDR);
    SysBootLoader();

    while(1);
}

/**
 * @brief Запуск приложения
 */
//void AppStart(void)
//{
//    uint32_t app_stack = *((volatile uint32_t*)APP_START_ADDRESS);
//    uint32_t app_reset = *((volatile uint32_t*)(APP_START_ADDRESS + 4));
//
//    if (app_stack >= 0x20000000 && app_stack <= 0x20020000 &&
//        app_reset >= 0x08000000 && app_reset <= 0x0807FFFF)
//    {
//        __disable_irq();
//        HAL_DeInit();
//
//        SysTick->CTRL = 0;
//        SysTick->LOAD = 0;
//        SysTick->VAL = 0;
//
//        void (*AppReset)(void);
//        AppReset = (void (*)(void))app_reset;
//
//        __set_MSP(app_stack);
//        AppReset();
//    }
//    else
//    {
//        Error_Handler();
//    }
//}
#define FIRMWARE_START_ADDRESS 0x0800C000

void AppStart(void)
{
	uint32_t appJumpAddress;

	void (*GoToApp)(void);
	appJumpAddress = *((volatile uint32_t*)(FIRMWARE_START_ADDRESS + 4));
	GoToApp = (void (*)(void))appJumpAddress;

	HAL_DeInit();
	__disable_irq();
	__set_MSP(*((volatile uint32_t*)FIRMWARE_START_ADDRESS));

	GoToApp();
}


/**
 * @brief Кормление Watchdog
 */
void FeedWatchdog(void)
{
    IWDG->KR = 0xAAAA;
}
