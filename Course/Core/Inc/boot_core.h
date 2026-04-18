#ifndef BOOT_CORE_H
#define BOOT_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные
extern uint8_t g_last_error;
extern bool g_hex_processing_done;

// Коды ошибок
#define ERROR_NONE               0
#define ERROR_USB_INIT           1
#define ERROR_CRC                2
#define ERROR_ADDR_OUT_OF_RANGE  3
#define ERROR_ADDR_NOT_FOUND     4
#define ERROR_FLASH_ERASE        5
#define ERROR_FLASH_WRITE        6
#define ERROR_RX_TIMEOUT         7

// Функции
void HexProcessing_Init(void);
bool HexProcessing_ProcessLine(char* line);
bool HexProcessing_Finalize(void);
void SetError(uint8_t error_code);
HAL_StatusTypeDef Flash_EraseSector4(void);
HAL_StatusTypeDef Flash_WriteData(uint32_t addr, const uint8_t* data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* BOOT_CORE_H */
