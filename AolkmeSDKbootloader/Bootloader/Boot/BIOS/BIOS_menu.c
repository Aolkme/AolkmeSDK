#include "BIOS_menu.h"
#include "Common.h"
#include "uart.h"
#include "flash_if.h"
#include "ymodem.h"


uint32_t FlashProtection = 0;













void Show_BIOS_Menu(void)
{
    uint8_t key = 0;

    // Get user input for menu selection (One byte : 0x00,0x01 ~ 0xEF,0xF0~0xFF are reserved)
    uint8_t user_input_key = 0;
    Serial_PutString((uint8_t *)"\r\n====================================================================\r\n");
    Serial_PutString((uint8_t *)"\r\n=                   Open source.Author : AOLKME                    =\r\n");
    Serial_PutString((uint8_t *)"\r\n=                   Function : Aolkme BIOS Menu                    =\r\n");
    Serial_PutString((uint8_t *)"\r\n=                        C: olkme@163.com                          =\r\n");
    Serial_PutString((uint8_t *)"\r\n====================================================================\r\n");
    Serial_PutString((uint8_t *) "\r\n\r\n");


    while (1) {
        /* Test if any sector of Flash memory where user application will be loaded is write protected */
        FlashProtection = FLASH_If_GetWriteProtectionStatus();

        Serial_PutString((uint8_t *)"\r\n============================ Aolkme BIOS Menu ============================\r\n");
        Serial_PutString((uint8_t *)"       Download user application in the Flash -------------- Command: 0x01\r\n");
        Serial_PutString((uint8_t *)"       Upload user application from the Flash -------------- Command: 0x02\r\n");
        Serial_PutString((uint8_t *)"       Execute the loaded application ---------------------- Command: 0x03\r\n");

        if (FlashProtection != FLASHIF_PROTECTION_NONE)
        {
            Serial_PutString((uint8_t *)"       Disable the write protection of application area ---- Command: 0x04\r\n");
        }
        else
        {
            Serial_PutString((uint8_t *)"       Enable the write protection of application area ---- Command: 0x04\r\n");
        }

        Serial_PutString((uint8_t *)"\r\n\r\n");
        Serial_PutString((uint8_t *)"Notice: \r\n");




        Serial_PutString((uint8_t *)"\r\n==========================================================================\r\n");

        Uart_ReadWithTimeOut(Aolkme_CONSOLE_UART_NUM, &key, 1, RX_TIMEOUT);


        switch (key)
        {
        case 0x01:
            /* Download user application */
            break;
        case 0x02:
            /* Upload user application */
            break;
        
        
        default:
            break;
        }













    }


}































