/**
  ******************************************************************************
  * @file    ymodem.h 
  * @author  MCD Application Team
  * @brief   YModem protocol header file
  * @version V2.0.0 - 重构版本
  ******************************************************************************
  * @attention
  * 重构改进点：
  * 1. 增加状态机管理
  * 2. 细化错误码
  * 3. 添加调试支持
  * 4. 改进超时处理
  * 5. 分离协议层和硬件层
  ******************************************************************************
  */

#ifndef __YMODEM_H__
#define __YMODEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
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

#define FILE_SIZE_LENGTH        ((uint32_t)16)
#define FILE_NAME_LENGTH        ((uint32_t)64)

/* Protocol control characters */
#define SOH                     ((uint8_t)0x01)
#define STX                     ((uint8_t)0x02)
#define EOT                     ((uint8_t)0x04)
#define ACK                     ((uint8_t)0x06)
#define NAK                     ((uint8_t)0x15)
#define CA                      ((uint8_t)0x18)
#define CRC16                   ((uint8_t)0x43)
#define NEGATIVE_BYTE           ((uint8_t)0xFF)

#define ABORT1                  ((uint8_t)0x41)
#define ABORT2                  ((uint8_t)0x61)

/* Timeout definitions - 改进：使用毫秒单位 */
#define NAK_TIMEOUT_MS          ((uint32_t)1000)      /* 1秒 */
#define DOWNLOAD_TIMEOUT_MS     ((uint32_t)5000)      /* 5秒 */
#define MAX_ERRORS              ((uint32_t)5)

/* 重构新增：应用程序地址定义 */
#define APPLICATION_ADDRESS     ((uint32_t)0x08010000)
#define APPLICATION_FLASH_SIZE  ((uint32_t)0x70000)   /* 448KB */
#define APPLICATION_ADDRESS_END (APPLICATION_ADDRESS + APPLICATION_FLASH_SIZE - 1)

/* 重构新增：错误码细化 */
typedef enum {
    COM_OK = 0,                     /* 成功 */
    COM_ERROR,                      /* 一般错误 */
    COM_ABORT,                      /* 用户中止 */
    COM_TIMEOUT,                    /* 超时 */
    COM_CRC_ERROR,                  /* CRC校验错误 */
    COM_SEQUENCE_ERROR,             /* 包序号错误 */
    COM_FLASH_ERROR,                /* Flash操作错误 */
    COM_LIMIT,                      /* 大小超限 */
    COM_PROTOCOL_ERROR,             /* 协议错误 */
    COM_DATA_ERROR                  /* 数据错误 */
} COM_StatusTypeDef;

/* 重构新增：YModem状态机 */
typedef enum {
    YMODEM_STATE_IDLE = 0,          /* 空闲状态 */
    YMODEM_STATE_WAIT_HEADER,       /* 等待文件头 */
    YMODEM_STATE_RECEIVING,         /* 接收数据中 */
    YMODEM_STATE_COMPLETE,          /* 传输完成 */
    YMODEM_STATE_ERROR              /* 错误状态 */
} YModem_StateTypeDef;

/* 重构新增：会话上下文结构 */
typedef struct {
    YModem_StateTypeDef state;      /* 当前状态 */
    uint32_t file_size;             /* 文件大小 */
    uint32_t bytes_received;        /* 已接收字节数 */
    uint32_t write_address;         /* 写入地址 */
    uint8_t packet_number;          /* 当前包序号 */
    uint32_t error_count;           /* 错误计数 */
    uint32_t session_start_time;    /* 会话开始时间 */
    char filename[FILE_NAME_LENGTH]; /* 文件名 */
} YModem_SessionTypeDef;

/* 重构新增：调试支持 */
#define YMODEM_DEBUG_ENABLE     1   /* 启用调试输出 */

#if YMODEM_DEBUG_ENABLE
    #define YMODEM_DEBUG(fmt, ...)  printf("[YModem] " fmt "\r\n", ##__VA_ARGS__)
#else
    #define YMODEM_DEBUG(fmt, ...)
#endif

/* External variables --------------------------------------------------------*/
/* 保持原有外部变量声明 */
extern uint8_t aFileName[FILE_NAME_LENGTH];
extern __IO uint32_t flashdestination;

/* Public function prototypes ------------------------------------------------*/
/* 保持原有函数声明不变 */
COM_StatusTypeDef Ymodem_Receive(uint32_t *p_size);
COM_StatusTypeDef Ymodem_Transmit(uint8_t *p_buf, const uint8_t *p_file_name, uint32_t file_size);

/* 重构新增：工具函数声明 */
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t *p_data, uint32_t size);
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size);

/* 重构新增：硬件抽象层函数声明 - 需要用户实现 */
void Serial_PutByte(uint8_t data);
uint32_t Get_TickCount(void);
void Delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* __YMODEM_H__ */