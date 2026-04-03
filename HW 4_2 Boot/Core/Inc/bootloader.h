#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "stm32f4xx_hal.h"

// Адреса памяти
#define SYSTEM_BOOTLOADER_ADDR   0x1FFF0000
#define APP_START_ADDRESS        0x0800C000

// Используем макросы IDE для пинов
// LED: LD2 (PA5) - определено в main.h как LD2_GPIO_Port, LD2_Pin
// BUTTON: B1 (PC13) - определено в main.h как B1_GPIO_Port, B1_Pin
#define BUTTON_PRESSED           GPIO_PIN_RESET

// Структура для хранения флагов сброса
typedef struct
{
    uint32_t raw_value;
    uint8_t lsion;
    uint8_t lsirdy;
    uint8_t borrst;
    uint8_t pinrst;
    uint8_t porrst;
    uint8_t sftrst;
    uint8_t iwdgrst;
    uint8_t wwdgrst;
    uint8_t lpwrrst;
    uint8_t rmvf;
} RawCSR_t;

// Функции
void Read_Raw_CSR(RawCSR_t* csr);
void JumpToSystemBootloader(void);
void AppStart(void);
void FeedWatchdog(void);

#endif /* BOOTLOADER_H */
