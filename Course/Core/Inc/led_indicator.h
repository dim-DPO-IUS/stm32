#ifndef LED_INDICATOR_H
#define LED_INDICATOR_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Состояния светодиода
#define LED_STATE_IDLE           0
#define LED_STATE_PROG_MODE      1
#define LED_STATE_ACTIVITY       2
#define LED_STATE_SUCCESS        3
#define LED_STATE_ERROR          4

/**
 * @brief Инициализация модуля светодиода
 * @param port Порт GPIO
 * @param pin  Пин
 */
void led_init(GPIO_TypeDef* port, uint16_t pin);

/**
 * @brief Установка состояния светодиода
 * @param state Состояние (LED_STATE_*)
 */
void led_set_state(uint8_t state);

/**
 * @brief Получение текущего состояния светодиода
 * @return Текущее состояние
 */
uint8_t led_get_state(void);

/**
 * @brief Обновление конечного автомата светодиода
 */
void led_update(void);

/**
 * @brief Включить светодиод
 */
void led_on(void);

/**
 * @brief Выключить светодиод
 */
void led_off(void);

/**
 * @brief Переключить светодиод
 */
void led_toggle(void);

#ifdef __cplusplus
}
#endif

#endif /* LED_INDICATOR_H */
