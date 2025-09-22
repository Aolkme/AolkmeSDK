#include "flash_if.h"


static uint32_t GetSector(uint32_t Address);










/**
 * @brief  This function does an erase of all user flash area
 * @param  StartAddress: start address for erasing. Must be a multiple of sector size.
 * @param  EndAddress: end address for erasing. Must be a multiple of sector size.
 * @retval FLASHIF_OK if successful, FLASHIF_ERASE_ERROR if error occurred
 */
uint32_t FLASH_If_Erase(uint32_t StartAddress, uint32_t EndAddress)
{
    uint32_t StartSector = 0, EndSector = 0, SectorError = 0;
    FLASH_EraseInitTypeDef pEraseInitStruct;
    uint8_t SectorCount;
    uint32_t ret;

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    /* Clear pending flags (if any) */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    /* Get the sector where start the user flash area */
    StartSector = GetSector(StartAddress);
    /* Get the sector where ends the user flash area */
    EndSector = GetSector(EndAddress);
    /* Get the number of sector to erase from 1st sector */
    SectorCount = (EndSector - StartSector) + 1;
    /* Fill EraseInit structure*/
    pEraseInitStruct.TypeErase = TYPEERASE_SECTORS;
    pEraseInitStruct.VoltageRange = VOLTAGE_RANGE_3;
    pEraseInitStruct.Sector = StartSector;
    pEraseInitStruct.NbSectors = SectorCount;

    if (HAL_FLASHEx_Erase(&pEraseInitStruct, &SectorError) != HAL_OK) {
        /*
          Error occurred while sector erase.
          User can add here some code to deal with this error.
          SectorError will contain the faulty sector and then to know the code error on this sector,
          user can call function 'HAL_FLASH_GetError()'
        */
        ret = FLASHIF_ERASE_ERROR;
        goto out;
    }
    ret = FLASHIF_OK;
    

out:
    /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

    return ret;

}


/**
 * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
 * @note   After writing data buffer, the flash content is checked.
 * @param  FlashAddress: start address for writing data buffer. Must be a multiple of 4.
 * @param  pData: pointer on data buffer
 * @param  DataLength: length of data buffer (unit is byte)
 * @retval FLASHIF_OK if successful, FLASHIF_WRITINGCTRL_ERROR if writing control failed,
 *         FLASHIF_WRITING_ERROR if program error occurred
 */
uint32_t FLASH_If_Write(uint32_t FlashAddress, uint32_t *pData, uint32_t DataLength)
{
    uint32_t i = 0;
    uint32_t DataLengthWords = DataLength / 4;
    uint32_t WriteAddress = FlashAddress;
    uint32_t ret = 0;


    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    /* Clear pending flags (if any) */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    /* Program the user Flash area word by word */
    for (i = 0; (i < DataLengthWords) && (WriteAddress < (FLASH_END_ADDRESS - 4)); i++) {

        /* Device voltage range supposed to be [2.7V to 3.6V], the operation will be done by word */
        if (HAL_FLASH_Program(TYPEPROGRAM_WORD, WriteAddress, *(uint32_t *)(pData + 4 * i)) == HAL_OK) {
            /* Check the written value */
            if (*(uint32_t *)WriteAddress != *(uint32_t *)(pData + 4 * i)) {
                /* Flash content doesn't match SRAM content */
                ret = FLASHIF_WRITINGCTRL_ERROR;
                goto out;
            }
            /* Increment FLASH destination address */
            WriteAddress += 4;
        } else {
            /* Error occurred while writing data in Flash memory */
            ret = FLASHIF_WRITING_ERROR;
            goto out;
        }
    }
    for (i = 0; i < DataLength % 4; i++) {
        if (HAL_FLASH_Program(TYPEPROGRAM_BYTE, WriteAddress, *(uint8_t *)(pData + WriteAddress - FlashAddress)) == HAL_OK) {
            /* Check the written value */
            if (*(uint8_t *)WriteAddress != *(uint8_t *)(pData + WriteAddress - FlashAddress)) {
                /* Flash content doesn't match SRAM content */
                ret = FLASHIF_WRITINGCTRL_ERROR;
                goto out;
            }
            /* Increment FLASH destination address */
            WriteAddress += 1;
        } else {
            /* Error occurred while writing data in Flash memory */
            ret = FLASHIF_WRITING_ERROR;
            goto out;
        }
    }    
    ret = FLASHIF_OK;

out:
    /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

    return ret;

}



/**
 * @brief  Get the write protection status
 * @retval FLASHIF_PROTECTION_NONE: no protection
 *         FLASHIF_PROTECTION_WRPENABLED: write protection is enabled
 *         FLASHIF_PROTECTION_RDPENABLED: read protection is enabled
 *         FLASHIF_PROTECTION_PCROPENABLED: proprietary code readout protection is enabled
 */
uint16_t FLASH_If_GetWriteProtectionStatus(void)
{
    uint32_t ProtectedSECTOR = 0xFFF;
    FLASH_OBProgramInitTypeDef OptionsBytesStruct;

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    /* Check if there are write protected sectors inside the user flash area ****/
    HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);


    /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

    /* Get pages already write protected ****************************************/
    ProtectedSECTOR = (OptionsBytesStruct.WRPSector) & FLASH_SECTOR_TO_BE_PROTECTED;

    /* Check if desired pages are already write protected ***********************/
    if (ProtectedSECTOR != 0) {
        /* Some pages are write protected */
        return FLASHIF_PROTECTION_WRPENABLED;
    } else {
        /* No write protected pages */
        return FLASHIF_PROTECTION_NONE;
    }
}






/**
 * @brief Get the Sector object
 * @param Address 
 * @return uint32_t 
 */
static uint32_t GetSector(uint32_t Address)
{
    uint32_t sector = 0;

    if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0)) {
        sector = FLASH_SECTOR_0;
    } else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1)) {
        sector = FLASH_SECTOR_1;
    } else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2)) {
        sector = FLASH_SECTOR_2;
    } else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3)) {
        sector = FLASH_SECTOR_3;
    } else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4)) {
        sector = FLASH_SECTOR_4;
    } else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5)) {
        sector = FLASH_SECTOR_5;
    } else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6)) {
        sector = FLASH_SECTOR_6;
    } else if ((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7)) {
        sector = FLASH_SECTOR_7;
    } else if ((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8)) {
        sector = FLASH_SECTOR_8;
    } else if ((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9)) {
        sector = FLASH_SECTOR_9;
    } else if ((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10)) {
        sector = FLASH_SECTOR_10;
    } else /* (Address < FLASH_END_ADDRESS) && (Address >= ADDR_FLASH_SECTOR_11) */ {
        sector = FLASH_SECTOR_11;
    }

    return sector;

}




HAL_StatusTypeDef FLASH_IF_WriteProtectionConfig(uint32_t modifier)
{
    uint32_t ProtectedSECTOR = 0xFFF;
    FLASH_OBProgramInitTypeDef config_new, config_old;
    HAL_StatusTypeDef result = HAL_OK;

    /* Get pages write protection status ****************************************/
    HAL_FLASHEx_OBGetConfig(&config_old);

    /* The parameter says whether we turn the protection on or off */
    config_new.WRPState = modifier;

    /* We want to modify only the Write protection */
    config_new.OptionType = OPTIONBYTE_WRP;

    /* No read protection, keep BOR and reset settings */
    config_new.RDPLevel = OB_RDP_LEVEL_0;
    config_new.USERConfig = config_old.USERConfig;
    /* Get pages already write protected ****************************************/
    ProtectedSECTOR = config_old.WRPSector | FLASH_SECTOR_TO_BE_PROTECTED;

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    /* Unlock the Options Bytes *************************************************/
    HAL_FLASH_OB_Unlock();

    config_new.WRPSector = ProtectedSECTOR;
    result = HAL_FLASHEx_OBProgram(&config_new);

    return result;



}






