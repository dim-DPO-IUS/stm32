#include "boot_core.h"
#include "hex_parser.h"
#include "flash_manager.h"
#include <string.h>
#include <stdlib.h>

// ============================================================================
//  КОНФИГУРАЦИЯ (СЕКТОР 4)
// ============================================================================

#define SECTOR_4_START          0x08010000UL
#define SECTOR_4_END            0x0801FFFFUL
#define SECTOR_4_SIZE           65536U

// Целевые адреса (5 адресов в секторе 4)
static const uint32_t target_addresses[] = {
    0x08010000,
    0x08010010,
    0x08010020,
    0x08010030,
    0x08010040
};

#define TARGET_ADDR_COUNT       (sizeof(target_addresses) / sizeof(target_addresses[0]))
#define MAX_DATA_PER_ADDRESS    16

// ============================================================================
//  ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ============================================================================
uint8_t g_last_error = 0;
bool g_hex_processing_done = false;

// ============================================================================
//  СТАТИЧЕСКИЕ ПЕРЕМЕННЫЕ (БУФЕРЫ ДЛЯ НАКОПЛЕНИЯ ДАННЫХ)
// ============================================================================
static bool _addr_found[TARGET_ADDR_COUNT];
static uint8_t _addr_data[TARGET_ADDR_COUNT][MAX_DATA_PER_ADDRESS];
static uint8_t _addr_data_len[TARGET_ADDR_COUNT];
static bool _sector4_erased = false;

// ============================================================================
//  РЕАЛИЗАЦИИ ПУБЛИЧНЫХ ФУНКЦИЙ
// ============================================================================

void HexProcessing_Init(void)
{
    hex_parser_init();
    _sector4_erased = false;
    g_hex_processing_done = false;

    for (uint8_t i = 0; i < TARGET_ADDR_COUNT; i++)
    {
        _addr_found[i] = false;
        _addr_data_len[i] = 0;
        memset(_addr_data[i], 0xFF, MAX_DATA_PER_ADDRESS);
    }
}

bool HexProcessing_ProcessLine(char* line)
{
    // Очистка от \r и \n ПЕРЕД всеми проверками
    char* nl = strchr(line, '\r');
    if (nl) *nl = '\0';
    nl = strchr(line, '\n');
    if (nl) *nl = '\0';

    if (strlen(line) == 0) return true;
    if (line[0] != ':') return true;

    HexRecord record;
    if (!hex_parser_parse_line(line, &record))
    {
        return true;
    }

    // Если CRC невалидный - ошибка
    if (!record.crc_valid)
    {
        SetError(ERROR_CRC);
        return false;
    }

    // Обработка по типу записи
    if (record.type == HEX_RECORD_ELA || record.type == HEX_RECORD_ESA)
    {
        // Расширенные адреса уже обработаны в hex_parser
        return true;
    }

    if (record.type == HEX_RECORD_EOF)
    {
        g_hex_processing_done = true;
        return true;
    }

    if (record.type == HEX_RECORD_SSA || record.type == HEX_RECORD_SLA)
    {
        return true;
    }

    if (record.type == HEX_RECORD_DATA)
    {
        // Проверка границ Сектора 4
        if (record.address < SECTOR_4_START || record.address > SECTOR_4_END)
        {
            SetError(ERROR_ADDR_OUT_OF_RANGE);
            return false;
        }

        for (uint8_t i = 0; i < TARGET_ADDR_COUNT; i++)
        {
            if (record.address == target_addresses[i] && !_addr_found[i])
            {
                _addr_found[i] = true;
                _addr_data_len[i] = (record.data_len < MAX_DATA_PER_ADDRESS) ? record.data_len : MAX_DATA_PER_ADDRESS;
                memcpy(_addr_data[i], record.data, _addr_data_len[i]);
                break;
            }
        }
    }

    return true;
}

bool HexProcessing_Finalize(void)
{
    // Проверка, что все целевые адреса найдены
    for (uint8_t i = 0; i < TARGET_ADDR_COUNT; i++)
    {
        if (!_addr_found[i])
        {
            SetError(ERROR_ADDR_NOT_FOUND);
            return false;
        }
    }

    // Стирание Сектора 4
    if (!_sector4_erased)
    {
        if (Flash_EraseSector4() != HAL_OK)
        {
            SetError(ERROR_FLASH_ERASE);
            return false;
        }
        _sector4_erased = true;
    }

    // Запись данных
    for (uint8_t i = 0; i < TARGET_ADDR_COUNT; i++)
    {
        if (_addr_data_len[i] > 0)
        {
            if (Flash_WriteData(target_addresses[i], _addr_data[i], _addr_data_len[i]) != HAL_OK)
            {
                SetError(ERROR_FLASH_WRITE);
                return false;
            }
        }
    }

    return true;
}

void SetError(uint8_t error_code)
{
    g_last_error = error_code;
}
