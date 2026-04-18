#include "crc_utils.h"

uint8_t crc_hex_char_to_byte(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

uint8_t crc8_compute(const uint8_t* data, size_t len)
{
    uint16_t sum = 0;
    for (size_t i = 0; i < len; i++)
    {
        sum += data[i];
    }
    // Дополнительный код младшего байта суммы
    return (uint8_t)((0x100 - (sum & 0xFF)) & 0xFF);
}
