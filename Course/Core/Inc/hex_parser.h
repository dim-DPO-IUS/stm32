#ifndef HEX_PARSER_H
#define HEX_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Типы записей Intel HEX
 */
typedef enum {
    HEX_RECORD_DATA = 0x00,
    HEX_RECORD_EOF  = 0x01,
    HEX_RECORD_ESA  = 0x02,
    HEX_RECORD_SSA  = 0x03,
    HEX_RECORD_ELA  = 0x04,
    HEX_RECORD_SLA  = 0x05
} HexRecordType;

/**
 * @brief Структура распарсенной записи Intel HEX
 */
typedef struct {
    HexRecordType type;
    uint32_t      address;
    uint8_t       data[256];
    uint8_t       data_len;
    bool          crc_valid;
} HexRecord;

/**
 * @brief Инициализация парсера (сброс расширенных адресов)
 */
void hex_parser_init(void);

/**
 * @brief Парсинг одной строки Intel HEX
 * @param line   Строка, начинающаяся с ':'
 * @param record Указатель на структуру для результата
 * @return       true, если строка успешно распарсена
 */
bool hex_parser_parse_line(const char* line, HexRecord* record);

/**
 * @brief Получить текущий расширенный линейный адрес (ELA)
 */
uint32_t hex_parser_get_ela(void);

/**
 * @brief Получить текущий расширенный сегментный адрес (ESA)
 */
uint16_t hex_parser_get_esa(void);

/**
 * @brief Проверить, используется ли линейная адресация
 */
bool hex_parser_using_linear(void);

#ifdef __cplusplus
}
#endif

#endif /* HEX_PARSER_H */
