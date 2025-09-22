#ifndef _YMODEM_H
#define _YMODEM_H


#include "stm32f4xx_hal.h"
#include "uart.h"

#ifdef __cplusplus
extern "C" {
#endif



HAL_StatusTypeDef Uart_ReadWithTimeOut(E_UartNum uartNum, uint8_t *data, uint16_t len, uint32_t timeOut);

HAL_StatusTypeDef Uart_WriteWithTimeOut(E_UartNum uartNum, uint8_t *data, uint16_t len, uint32_t timeOut);





#ifdef __cplusplus
}
#endif



#endif // _YMODEM_H



