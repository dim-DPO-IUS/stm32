#include "hex_parser.h"
#include "crc_utils.h"
#include <string.h>

// ============================================================================
//  ВНУТРЕННИЕ ПЕРЕМЕННЫЕ ПАРСЕРА
// ============================================================================
static uint32_t _ext_linear_addr = 0;
static uint16_t _ext_segment_addr = 0;
static bool     _use_linear = true;

// ============================================================================
//  ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================================================

static bool _hexCheckCRC(const char* line)
{
    uint16_t len = strlen(line);
    if (len < 11 || line[0] != ':') return false;

    uint8_t bytes[128];
    uint8_t byte_count = 0;

    // Читаем ВСЕ байты, включая CRC (i < len)
    for (uint16_t i = 1; i < len; i += 2)
    {
        if (i + 1 >= len) break;
        bytes[byte_count++] = (crc_hex_char_to_byte(line[i]) << 4) | crc_hex_char_to_byte(line[i+1]);
    }

    if (byte_count < 5) return false;

    // Вычисляем CRC по всем байтам, кроме последнего
    uint8_t crc_calculated = crc8_compute(bytes, byte_count - 1);
    uint8_t crc_received = bytes[byte_count - 1];

    return (crc_calculated == crc_received);
}

static void _hexUpdateExtendedAddress(uint8_t record_type, const uint8_t* data, uint8_t data_len)
{
    if (record_type == 0x04 && data_len >= 2)
    {
        _ext_linear_addr = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16);
        _use_linear = true;
    }
    else if (record_type == 0x02 && data_len >= 2)
    {
        _ext_segment_addr = (data[0] << 8) | data[1];
        _use_linear = false;
    }
}

// ============================================================================
//  ПУБЛИЧНЫЕ ФУНКЦИИ
// ============================================================================

void hex_parser_init(void)
{
    _ext_linear_addr = 0;
    _ext_segment_addr = 0;
    _use_linear = true;
}

bool hex_parser_parse_line(const char* line, HexRecord* record)
{
    if (line == NULL || record == NULL) return false;

    // Очистка структуры
    record->type = HEX_RECORD_DATA;
    record->address = 0;
    record->data_len = 0;
    record->crc_valid = false;

    uint16_t len = strlen(line);
    if (len < 11 || line[0] != ':') return false;

    // Проверка CRC
    record->crc_valid = _hexCheckCRC(line);

    // Извлечение байт
    uint8_t bytes[128];
    uint8_t byte_count = 0;

    for (uint16_t i = 1; i < len - 2; i += 2)
    {
        bytes[byte_count++] = (crc_hex_char_to_byte(line[i]) << 4) | crc_hex_char_to_byte(line[i+1]);
    }

    if (byte_count < 4) return false;

    uint8_t rec_len  = bytes[0];
    uint16_t addr    = (bytes[1] << 8) | bytes[2];
    uint8_t rtype    = bytes[3];

    record->type = (HexRecordType)rtype;

    if (_use_linear)
        record->address = _ext_linear_addr + addr;
    else
        record->address = ((uint32_t)_ext_segment_addr << 4) + addr;

    if (rec_len > 0 && byte_count >= 4 + rec_len)
    {
        record->data_len = rec_len;
        for (uint8_t i = 0; i < rec_len; i++)
            record->data[i] = bytes[4 + i];
    }

    // Обновление расширенных адресов
    if (rtype == 0x02 || rtype == 0x04)
    {
        _hexUpdateExtendedAddress(rtype, record->data, record->data_len);
    }

    return true;
}

uint32_t hex_parser_get_ela(void)
{
    return _ext_linear_addr;
}

uint16_t hex_parser_get_esa(void)
{
    return _ext_segment_addr;
}

bool hex_parser_using_linear(void)
{
    return _use_linear;
}
