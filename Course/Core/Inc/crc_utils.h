#ifndef CRC_UTILS_H
#define CRC_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Преобразует шестнадцатеричный символ в число
 * @param c Символ ('0'-'9', 'A'-'F', 'a'-'f')
 * @return Числовое значение (0-15)
 */
uint8_t crc_hex_char_to_byte(char c);

/**
 * @brief Вычисляет CRC8 по алгоритму Intel HEX
 * @param data Массив байт для вычисления
 * @param len  Длина массива
 * @return     Вычисленное значение CRC8
 */
uint8_t crc8_compute(const uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* CRC_UTILS_H */
