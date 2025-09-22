#ifndef _FLASH_IF_H
#define _FLASH_IF_H

#include "stm32f4xx_hal.h"



#ifdef __cplusplus
extern "C" {
#endif


/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */


/* Error code */
enum {
    FLASHIF_OK = 0,                         // 
    FLASHIF_ERASE_ERROR,                    // Flash erase error
    FLASHIF_WRITINGCTRL_ERROR,              // Flash writing control error
    FLASHIF_PROTECTION_ERROR,               // Flash protection error
    FLASHIF_INVALID_ADDR,                   // Invalid flash address
    FLASHIF_NOT_ERASED,                     // Flash not erased
    FLASHIF_WRITING_ERROR                   // Flash writing error
};

enum {
    FLASHIF_PROTECTION_NONE = 0,                    // No protection
    FLASHIF_PROTECTION_PCROPENABLED = 0x1,          // Proprietary code readout protection enabled
    FLASHIF_PROTECTION_WRPENABLED = 0x2,            // Write protection enabled
    FLASHIF_PROTECTION_RDPENABLED = 0x4             // Read protection enabled
};


// End address of the flash
#define FLASH_END_ADDRESS               0x080FFFFF

/* Define the address from where user application will be loaded.
   Note: the 1st sector 0x08000000-0x08007FFF is reserved for the IAP code */
#define APPLICATION_ADDRESS		            ADDR_FLASH_SECTOR_4
#define APPLICATION_ADDRESS_END             (ADDR_FLASH_SECTOR_8 - 1)

/* Define the user application size */
#define APPLICATION_SIZE                    (APPLICATION_ADDRESS_END - APPLICATION_ADDRESS + 1)

/* Define the address from where user application will be stored in upgrade mode */
#define APPLICATION_UPGRADE_ADDRESS         ADDR_FLASH_SECTOR_8
#define APPLICATION_UPGRADE_ADDRESS_END     FLASH_END_ADDRESS

/* Define the address for param store */
#define APPLICATION_PARAM_STORE_ADDRESS     ADDR_FLASH_SECTOR_2
#define APPLICATION_PARAM_STORE_ADDRESS_END (ADDR_FLASH_SECTOR_4 - 1)


/* Define bitmap representing user flash area that could be write protected (check restricted to pages 8-39). */
#define FLASH_SECTOR_TO_BE_PROTECTED (OB_WRP_SECTOR_0 | OB_WRP_SECTOR_1 | OB_WRP_SECTOR_2 | OB_WRP_SECTOR_3 |\
                                      OB_WRP_SECTOR_4 | OB_WRP_SECTOR_5 | OB_WRP_SECTOR_6 | OB_WRP_SECTOR_7 |\
                                      OB_WRP_SECTOR_8 | OB_WRP_SECTOR_9 | OB_WRP_SECTOR_10 | OB_WRP_SECTOR_11 )




uint32_t FLASH_If_Erase(uint32_t StartAddress, uint32_t EndAddress);

uint32_t FLASH_If_Write(uint32_t FlashAddress, uint32_t *pData, uint32_t DataLength);

uint16_t FLASH_If_GetWriteProtectionStatus(void);

HAL_StatusTypeDef FLASH_IF_WriteProtectionConfig(uint32_t modifier);


#ifdef __cplusplus
}
#endif



#endif // _FLASH_H



