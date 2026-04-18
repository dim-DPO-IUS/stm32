#include "led_indicator.h"

// ============================================================================
//  ВНУТРЕННИЕ ПЕРЕМЕННЫЕ
// ============================================================================
static GPIO_TypeDef* _led_port = NULL;
static uint16_t      _led_pin  = 0;
static uint8_t       _led_state = LED_STATE_IDLE;

static uint32_t _led_last_toggle = 0;
static bool     _led_current_level = false;
static uint8_t  _error_blink_phase = 0;

// ============================================================================
//  ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ
// ============================================================================
static void _led_write(bool level)
{
    if (_led_port != NULL)
    {
        HAL_GPIO_WritePin(_led_port, _led_pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    _led_current_level = level;
}

// ============================================================================
//  ПУБЛИЧНЫЕ ФУНКЦИИ
// ============================================================================

void led_init(GPIO_TypeDef* port, uint16_t pin)
{
    _led_port = port;
    _led_pin  = pin;
    _led_state = LED_STATE_IDLE;
    _led_last_toggle = HAL_GetTick();
    _led_current_level = false;
    _error_blink_phase = 0;
    led_off();
}

void led_set_state(uint8_t state)
{
    _led_state = state;
    _led_last_toggle = HAL_GetTick();
    _error_blink_phase = 0;

    // Немедленная реакция на смену состояния
    switch (state)
    {
        case LED_STATE_PROG_MODE:
            led_on();
            break;
        case LED_STATE_SUCCESS:
            led_off();
            break;
        case LED_STATE_ACTIVITY:
            _led_current_level = false;
            led_off();
            break;
        case LED_STATE_ERROR:
            _led_current_level = false;
            led_off();
            break;
        case LED_STATE_IDLE:
        default:
            break;
    }
}

uint8_t led_get_state(void)
{
    return _led_state;
}

void led_update(void)
{
    uint32_t now = HAL_GetTick();

    switch (_led_state)
    {
        case LED_STATE_IDLE:
            // В этом состоянии управление передаётся внешнему коду
            break;

        case LED_STATE_PROG_MODE:
            // Ровное горение (уже установлено в led_set_state)
            break;

        case LED_STATE_ACTIVITY:
            // Быстрое мигание: 50 мс ON / 50 мс OFF (10 Гц)
            if ((now - _led_last_toggle) >= 50)
            {
                _led_last_toggle = now;
                led_toggle();
            }
            break;

        case LED_STATE_SUCCESS:
            // Выключен (уже установлено в led_set_state)
            break;

        case LED_STATE_ERROR:
            // Короткая вспышка 100 мс + длинная пауза 900 мс
            if (_error_blink_phase == 0)
            {
                if (!_led_current_level)
                {
                    led_on();
                }
                if ((now - _led_last_toggle) >= 100)
                {
                    _led_last_toggle = now;
                    _error_blink_phase = 1;
                }
            }
            else
            {
                if (_led_current_level)
                {
                    led_off();
                }
                if ((now - _led_last_toggle) >= 900)
                {
                    _led_last_toggle = now;
                    _error_blink_phase = 0;
                }
            }
            break;
    }
}

void led_on(void)
{
    _led_write(true);
}

void led_off(void)
{
    _led_write(false);
}

void led_toggle(void)
{
    _led_write(!_led_current_level);
}
