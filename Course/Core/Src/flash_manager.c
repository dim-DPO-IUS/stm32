#include "flash_manager.h"

HAL_StatusTypeDef Flash_EraseSector4(void)
{
    HAL_StatusTypeDef status = HAL_OK;

    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    FLASH_EraseInitTypeDef erase = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3,
        .Sector = FLASH_SECTOR_4,
        .NbSectors = 1
    };

    uint32_t sector_error = 0;
    status = HAL_FLASHEx_Erase(&erase, &sector_error);

    HAL_FLASH_Lock();
    return status;
}

HAL_StatusTypeDef Flash_WriteData(uint32_t addr, const uint8_t* data, uint32_t len)
{
    HAL_StatusTypeDef status = HAL_OK;

    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    for (uint32_t i = 0; i < len; i += 4)
    {
        uint32_t word = 0xFFFFFFFF;
        for (uint32_t j = 0; j < 4 && i + j < len; j++)
        {
            ((uint8_t*)&word)[j] = data[i + j];
        }

        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i, word);
        if (status != HAL_OK) break;
    }

    HAL_FLASH_Lock();
    return status;
}
