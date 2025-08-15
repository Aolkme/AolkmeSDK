/**
 * @file logger_buffer.h
 * @brief 日志缓冲管理
 * 
 * 注意：此头文件仅供组件内部使用
 */

// 头部引用格式




#ifndef LOGGER_BUFFER_H
#define LOGGER_BUFFER_H

//#pragma once




#include "Aolkme_logger.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef struct {
    E_AolkmeLoggerConsoleLogLevel level;
    uint16_t length;
    uint8_t data[];
} T_AolkmeLoggerBlock;














/**
 * @brief Initialize the logger buffer.
 * @param size The size of the buffer.
 * @return T_AolkmeReturnCode
 */
T_AolkmeReturnCode AolkmeLogger_BufferInit(size_t size);

/**
 * @brief Deinitialize the logger buffer.
 * 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeLogger_BufferDeinit(void);

/**
 * @brief Add log to buffer
 * 
 * @param data Log data
 * @param datalen Data length
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeLogger_BufferPut(E_AolkmeLoggerConsoleLogLevel level, uint8_t *data, uint16_t datalen);


/**
 * @brief Flush the buffer (ensure all logs are output)
 * 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeLogger_BufferFlush(void);








#ifdef __cplusplus
}
#endif


#endif // LOGGER_BUFFER_H

