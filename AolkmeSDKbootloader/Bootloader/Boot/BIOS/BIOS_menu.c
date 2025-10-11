#include "BIOS_menu.h"
#include "Common.h"
#include "uart.h"
#include "flash_if.h"
#include "ymodem.h"
#include "BootService.h"

uint32_t FlashProtection = 0;

uint8_t UART2_UART4_fORWARD = 0;


// 在您的应用程序中定义12字节密钥
const uint8_t my_secret_key[12] = {
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 
    0xDE, 0xF0, 0x11, 0x22, 0x33, 0x44
};

// 或者使用ASCII字符串密钥
const uint8_t my_ascii_key[12] = "MySecretKey!";




// ############# 检测结束 #################

// 静态变量定义
static const uint8_t* target_key = NULL;  // 目标密钥指针
static uint8_t key_length = 0;            // 密钥长度
static uint8_t current_index = 0;         // 当前匹配位置
static uint8_t key_matched = 0;           // 匹配完成标志




/**
  * @brief  重置检测器状态
  * @param  None
  * @retval None
  */
void KeyDetector_Reset(void)
{
    current_index = 0;
    key_matched = 0;
}




/**
  * @brief  初始化密钥检测器
  * @param  key: 密钥数组指针
  * @param  key_len: 密钥长度
  * @retval None
  */
void KeyDetector_Init(const uint8_t* key, uint8_t key_len)
{
    target_key = key;
    key_length = key_len;
    KeyDetector_Reset();
}


/**
  * @brief  处理接收到的字节
  * @param  data: 接收到的字节数据
  * @retval 1: 密钥匹配完成, 0: 密钥未匹配完成
  * @note   在串口中断中调用此函数
  */
uint8_t KeyDetector_ProcessByte(uint8_t data)
{
    // 如果已经匹配完成，直接返回
    if (key_matched) {
        return 1;
    }
    
    // 检查当前字节是否与密钥对应位置匹配
    if (data == target_key[current_index]) {
        current_index++;
        
        // 检查是否匹配完成
        if (current_index >= key_length) {
            key_matched = 1;
            return 1;
        }
    } else {
        // 不匹配，重置匹配状态
        // 但如果当前字节与密钥第一个字节匹配，则从第一个字节重新开始
        if (data == target_key[0]) {
            current_index = 1;  // 从第二个位置开始（因为第一个已经匹配）
        } else {
            current_index = 0;  // 完全重置
        }
    }
    
    return 0;
}


/**
  * @brief  检查密钥是否匹配完成
  * @param  None
  * @retval 1: 匹配完成, 0: 未匹配完成
  */
uint8_t KeyDetector_IsKeyMatched(void)
{
    return key_matched;
}

















void SerialDownload(void)
{

    uint8_t number[11] = {0};
    uint32_t size = 0;
    COM_StatusTypeDef result;

    Serial_PutString((uint8_t *) "Waiting for the file to be sent .f.. (press 'a' to abort)\n\r");
    
    result = Ymodem_Receive(&size);

    if (result == COM_OK) {
        Serial_PutString(
            (uint8_t *) "\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
        Serial_PutString(aFileName);
        Int2Str(number, size);
        Serial_PutString((uint8_t *) "\n\r Size: ");
        Serial_PutString(number);
        Serial_PutString((uint8_t *) " Bytes\r\n");
        Serial_PutString((uint8_t *) "-------------------\r\n");
		Serial_PutString((uint8_t *) "Over");
    } else if (result == COM_LIMIT) {
        Serial_PutString((uint8_t *) "\n\n\rThe image size is higher than the allowed space memory!\n\r");
    } else if (result == COM_DATA) {
        Serial_PutString((uint8_t *) "\n\n\rVerification failed!\n\r");
    } else if (result == COM_ABORT) {
        Serial_PutString((uint8_t *) "\r\n\nAborted by user.\n\r");
    } else {
        Serial_PutString((uint8_t *) "\n\rFailed to receive the file!\n\r");
    }


}

/**
  * @brief  Upload a file via serial port.
  * @param  None
  * @retval None
  */
void SerialUpload(void)
{
    uint8_t status = 0;

    Serial_PutString((uint8_t *) "\n\n\rSelect Receive File\n\r");

    Uart_ReadWithTimeOut(Aolkme_CONSOLE_UART_NUM, &status, 1, RX_TIMEOUT);
    if (status == CRC16) {
        /* Transmit the flash image through ymodem protocol */
        status = Ymodem_Transmit((uint8_t *) APPLICATION_ADDRESS, (const uint8_t *) "UploadedFlashImage.bin",
                                 APPLICATION_FLASH_SIZE);

        if (status != 0) {
            Serial_PutString((uint8_t *) "\n\rError Occurred while Transmitting File\n\r");
        } else {
            Serial_PutString((uint8_t *) "\n\rFile uploaded successfully \n\r");
        }
    }
}


uint8_t SerialDownloadOther(void)
{
	return UART2_UART4_fORWARD;
}


void ON_SerialDownloadOther(void)
{
	UART2_UART4_fORWARD = 1;
}

void OFF_SerialDownloadOther(void)
{
	UART2_UART4_fORWARD = 0;
}











void Show_BIOS_Menu(void)
{
	// CheckAllProtectionStatus();
	
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

		printf("Flash protection status: 0x%04X\n", FLASH_If_GetWriteProtectionStatus());
		
        Serial_PutString((uint8_t *)"\r\n============================ Aolkme BIOS Menu ============================\r\n");
        Serial_PutString((uint8_t *)"       Download user application in the Flash -------------- Command: 1\r\n");
        Serial_PutString((uint8_t *)"       Upload user application from the Flash -------------- Command: 2\r\n");
        Serial_PutString((uint8_t *)"       Execute the loaded application ---------------------- Command: 3\r\n");
		
		Serial_PutString((uint8_t *)"       Upload user application from the Flash -------------- Command: 2\r\n");
		
        if (FlashProtection != FLASHIF_PROTECTION_NONE)
        {
            Serial_PutString((uint8_t *)"       Disable the write protection of application area ---- Command: 4\r\n");
        }
        else
        {
            Serial_PutString((uint8_t *)"       Enable the write protection of application area ----- Command: 4\r\n");
        }

		
		
		
		
		
		
		
        Serial_PutString((uint8_t *)"\r\n\r\n");
        Serial_PutString((uint8_t *)"Notice: \r\n");


		

        Serial_PutString((uint8_t *)"\r\n==========================================================================\r\n");

        Uart_ReadWithTimeOut(Aolkme_CONSOLE_UART_NUM, &key, 1, RX_TIMEOUT);


        switch (key)
        {
        case 0x31:
			SerialDownload();
            /* Download user application */
            break;
        case 0x32:
			SerialUpload();
            /* Upload user application */
            break;
        
        case 0x33:
			JumpToApp();

			break;
		
		case 0x34:
			if (FlashProtection != FLASHIF_PROTECTION_NONE) {
                    /* Disable the write protection */
                    if (FLASH_If_WriteProtectionConfig(OB_WRPSTATE_DISABLE) == HAL_OK) {
                        Serial_PutString((uint8_t *) "Write Protection disabled...\r\n");
                        Serial_PutString((uint8_t *) "System will now restart...\r\n");
                        /* Launch the option byte loading */
                        HAL_FLASH_OB_Launch();
                        /* Ulock the flash */
                        HAL_FLASH_Unlock();
                    } else {
                        Serial_PutString((uint8_t *) "Error: Flash write un-protection failed...\r\n");
                    }
                } else {
                    if (FLASH_If_WriteProtectionConfig(OB_WRPSTATE_ENABLE) == HAL_OK) {
                        Serial_PutString((uint8_t *) "Write Protection enabled...\r\n");
                        Serial_PutString((uint8_t *) "System will now restart...\r\n");
                        /* Launch the option byte loading */
                        HAL_FLASH_OB_Launch();
                    } else {
                        Serial_PutString((uint8_t *) "Error: Flash write protection failed...\r\n");
                    }
                }
			break;

		case 0x35:
			UART_Write(UART_NUM_2, Menu_Cmd, 1);
			
		
		
			break;
		
		
		
		
		
        default:
            break;
        }













    }


}































