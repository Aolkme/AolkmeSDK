/**
  ******************************************************************************
  * @file    ymodem.h 
  * @author  MCD Application Team
  * @brief   YModem protocol header file
  * @version V2.0.1 - 修复版本
  ******************************************************************************
  * @attention
  * 修复问题：
  * 1. 保持与原始版本完全相同的API接口
  * 2. 使用Common.h中的错误码定义
  * 3. 移除所有重复定义
  ******************************************************************************
  */


#ifndef _YMODEM_H
#define _YMODEM_H


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "Common.h"
#include "uart.h"
#include <stdint.h>

/* Public defines ------------------------------------------------------------*/
/* YModem协议定义 - 保持原有定义不变 */

#define PACKET_HEADER_SIZE      ((uint32_t)3)
#define PACKET_DATA_INDEX       ((uint32_t)4)
#define PACKET_START_INDEX      ((uint32_t)1)
#define PACKET_NUMBER_INDEX     ((uint32_t)2)
#define PACKET_CNUMBER_INDEX    ((uint32_t)3)
#define PACKET_TRAILER_SIZE     ((uint32_t)2)
#define PACKET_OVERHEAD_SIZE    (PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE - 1)
#define PACKET_SIZE             ((uint32_t)128)
#define PACKET_1K_SIZE          ((uint32_t)1024)

/* /-------- Packet in IAP memory ------------------------------------------\
 * | 0      |  1    |  2     |  3   |  4      | ... | n+4     | n+5  | n+6  |
 * |------------------------------------------------------------------------|
 * | unused | start | number | !num | data[0] | ... | data[n] | crc0 | crc1 |
 * \------------------------------------------------------------------------/
 * the first byte is left unused for memory alignment reasons                 */
#define FILE_SIZE_LENGTH        ((uint32_t)16)
#define FILE_NAME_LENGTH        ((uint32_t)64)


/* Protocol control characters */
#define SOH                     ((uint8_t)0x01)  /* start of 128-byte data packet */
#define STX                     ((uint8_t)0x02)  /* start of 1024-byte data packet */
#define EOT                     ((uint8_t)0x04)  /* end of transmission */
#define ACK                     ((uint8_t)0x06)  /* acknowledge */
#define NAK                     ((uint8_t)0x15)  /* negative acknowledge */
#define CA                      ((uint32_t)0x18) /* two of these in succession aborts transfer */
#define CRC16                   ((uint8_t)0x43)
#define NEGATIVE_BYTE           ((uint8_t)0xFF)

#define ABORT1                  ((uint8_t)0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  ((uint8_t)0x61)  /* 'a' == 0x61, abort by user */

#define NAK_TIMEOUT             ((uint32_t)0x100000)
#define DOWNLOAD_TIMEOUT        ((uint32_t)5000) /* Five second retry delay */

#define MAX_ERRORS              ((uint32_t)5)




/* CRC option */
#define CRC16_F       /* activate the CRC16 integrity */





/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern uint8_t aFileName[FILE_NAME_LENGTH];
// __IO uint32_t flashdestination;
/* @note ATTENTION - please keep this variable 32bit alligned */





/* Public function prototypes ------------------------------------------------*/
COM_StatusTypeDef Ymodem_Receive(uint32_t *p_size);
COM_StatusTypeDef Ymodem_Transmit(uint8_t *p_buf, const uint8_t *p_file_name, uint32_t file_size);




/* 工具函数声明 */
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t *p_data, uint32_t size);
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size);






HAL_StatusTypeDef Uart_ReadWithTimeOut(E_UartNum uartNum, uint8_t *data, uint16_t len, uint32_t timeOut);

HAL_StatusTypeDef Uart_WriteWithTimeOut(E_UartNum uartNum, uint8_t *data, uint16_t len, uint32_t timeOut);





#ifdef __cplusplus
}
#endif



#endif // _YMODEM_H



