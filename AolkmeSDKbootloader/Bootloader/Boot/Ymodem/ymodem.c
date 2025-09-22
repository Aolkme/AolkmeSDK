#include "ymodem.h"
#include "Aolkme_OSAL.h"








HAL_StatusTypeDef Uart_ReadWithTimeOut(E_UartNum uartNum, uint8_t *data, uint16_t len, uint32_t timeOut)
{
    int res;
    uint16_t alreadyReadLen = 0;
		uint32_t loop_count = 0;
	
    while (loop_count <= timeOut) {
        res = UART_Read(uartNum, data + alreadyReadLen, len - alreadyReadLen);
        if (res > 0) {
            alreadyReadLen += res;
        }
        if (alreadyReadLen == len) {
            return HAL_OK;
        }
        A_Osal_TaskSleepMs(1);
				loop_count++;
    }

    return HAL_TIMEOUT;
}

HAL_StatusTypeDef Uart_WriteWithTimeOut(E_UartNum uartNum, uint8_t *data, uint16_t len, uint32_t timeOut)
{
    int res;

    res = UART_Write(uartNum, data, len);
    if (res == len) {
        return HAL_OK;
    } else {
        return HAL_ERROR;
    }
}















