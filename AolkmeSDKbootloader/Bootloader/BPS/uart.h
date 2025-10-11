/**
 ******************************************************************************
 * @file    uart.h
 * @version V1.0.0
 * @date    2025/09/22
 * @brief   This is the header file for "uart.c".
 *
 * @copyright (c) 2025 Aolkme. All rights reserved.
 * 
 * All information contained herein is, and remains, the property of Aolkme.
 * The intellectual and technical concepts contained herein are proprietary
 * to Aolkme and may be covered by U.S. and foreign patents, patents in process,
 * and protected by trade secret or copyright law.  Dissemination of this
 * information, including but not limited to data and other proprietary
 * material(s) incorporated within the information, in any form, is strictly
 * prohibited without the express written consent of Aolkme.
 * 
 * If you receive this source code without Aolkme’s authorization, you may not
 * further disseminate the information, and you must immediately remove the
 * source code and notify Aolkme of its removal. Aolkme reserves the right to pursue
 * legal actions against you for any loss(es) or damage(s) caused by your
 * failure to do so.
 * 
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "usart.h"
/* Exported constants --------------------------------------------------------*/
#define USING_UART_PORT_1
#define USING_UART_PORT_2
#define USING_UART_PORT_3
#define USING_UART_PORT_4




#define UART_ERROR      (-1)

/* Exported macros -----------------------------------------------------------*/
#define Aolkme_CONSOLE_UART_NUM             UART_NUM_2
#define Aolkme_CONSOLE_UART_BAUD            115200


extern UART_HandleTypeDef huart2;
#define s_uart2Handle huart2



extern UART_HandleTypeDef huart4;
#define s_uart4Handle huart4




/* Exported types ------------------------------------------------------------*/
typedef enum {
    UART_NUM_1 = 1,
    UART_NUM_2 = 2,
    UART_NUM_3 = 3,
	UART_NUM_4 = 4,
} E_UartNum;

typedef struct {
    uint32_t countOfLostData; /*!< Count of data lost, unit: byte. */
    uint16_t maxUsedCapacityOfBuffer; /*!< Max capacity of buffer that have been used, unit: byte. */
} T_UartBufferState;

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void UART_Init(E_UartNum uartNum, uint32_t baudRate);
int UART_Read(E_UartNum uartNum, uint8_t *buf, uint16_t readSize);
int UART_Write(E_UartNum uartNum, const uint8_t *buf, uint16_t writeSize);
void UART_GetBufferState(E_UartNum uartNum, T_UartBufferState *readBufferState, T_UartBufferState *writeBufferState);

/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

#endif
