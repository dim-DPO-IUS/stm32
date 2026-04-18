#ifndef FLASH_MANAGER_H
#define FLASH_MANAGER_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Стирание Сектора 4 Flash-памяти
 * @return HAL_StatusTypeDef (HAL_OK или ошибка)
 */
HAL_StatusTypeDef Flash_EraseSector4(void);

/**
 * @brief Запись данных во Flash-память
 * @param addr  Адрес для записи
 * @param data  Указатель на данные
 * @param len   Длина данных в байтах
 * @return      HAL_StatusTypeDef (HAL_OK или ошибка)
 */
HAL_StatusTypeDef Flash_WriteData(uint32_t addr, const uint8_t* data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* FLASH_MANAGER_H */
