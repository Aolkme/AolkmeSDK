/**
 * @file Aolkme.h
 * @brief 内核
 * @author Aolkme
 * @
 * 
 */

// 头部引用格式





#ifndef AOLKME_LOGGER_H
#define AOLKME_LOGGER_H

//#pragma once


#include "Aolkme_platform.h"
//#include "logger_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
* @brief The console method that needs to be registered.
* @note  Before registering the console method, you need to test the methods that need to be registered to ensure
*        that they can be used normally.
*/
typedef T_AolkmeReturnCode (*ConsoleOutputFunc)(const uint8_t *data, uint16_t dataLen);

/**
 * @brief Logger console level.
 */
typedef enum{
    AOLKME_LOGGER_CONSOLE_LOG_LEVEL_FATAL = 0,          // <! fatal error（System unusable）
    AOLKME_LOGGER_CONSOLE_LOG_LEVEL_ERROR,              // <! error（Function unavailable）
    AOLKME_LOGGER_CONSOLE_LOG_LEVEL_WARN,               // <! warning（Exception but recoverable）
    AOLKME_LOGGER_CONSOLE_LOG_LEVEL_INFO,               // <! info（Important event）
    AOLKME_LOGGER_CONSOLE_LOG_LEVEL_DEBUG,              // <! debug（Development debugging information）
    AOLKME_LOGGER_CONSOLE_LOG_LEVEL_TRACE,              // <! trace（Detailed execution process）
    AOLKME_LOGGER_CONSOLE_LOG_LEVEL_MAX                 // <! max（Maximum log level）
} E_AolkmeLoggerConsoleLogLevel;

/**
 * @brief Logger console content.
 */
typedef struct 
{
    E_AolkmeLoggerConsoleLogLevel level;                // <! Log level
    uint16_t buffer_size;                               // <! Buffer size
    bool isSupportColor;                                // <! Color support
} T_AolkmeLoggerConfig;



/**
 * @brief Initialize the logger.
 * 
 * @param config 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeLogger_Init(const T_AolkmeLoggerConfig *config);



T_AolkmeReturnCode AolkmeLogger_Deinit(void);


/**
 * @brief Add the console function and level for Aolkme_SDK
 */
T_AolkmeReturnCode AolkmeLogger_AddOutput(ConsoleOutputFunc output_func);

/**
 * @brief Log output function.
 */
void AolkmeLogger_Output(E_AolkmeLoggerConsoleLogLevel level, const char *tag,
                        const char *file, int line, const char *func, const char *format, ...);


/**
 * @brief Remove the console function and level for Aolkme_SDK
 */
T_AolkmeReturnCode AolkmeLogger_RemoveOutput(ConsoleOutputFunc output_func);


size_t AolkmeGetBlockSize(void);

/**
 * @brief Log output macro
 */

#define ALOG_FATAL(tag, format, ...) \
    AolkmeLogger_Output(AOLKME_LOGGER_CONSOLE_LOG_LEVEL_FATAL, tag, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define ALOG_ERROR(tag, format, ...) \
    AolkmeLogger_Output(AOLKME_LOGGER_CONSOLE_LOG_LEVEL_ERROR, tag, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define ALOG_WARN(tag, format, ...) \
    AolkmeLogger_Output(AOLKME_LOGGER_CONSOLE_LOG_LEVEL_WARN, tag, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define ALOG_INFO(tag, format, ...) \
    AolkmeLogger_Output(AOLKME_LOGGER_CONSOLE_LOG_LEVEL_INFO, tag, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define ALOG_DEBUG(tag, format, ...) \
    AolkmeLogger_Output(AOLKME_LOGGER_CONSOLE_LOG_LEVEL_DEBUG, tag, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define ALOG_TRACE(tag, format, ...) \
    AolkmeLogger_Output(AOLKME_LOGGER_CONSOLE_LOG_LEVEL_TRACE, tag, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)




#ifdef __cplusplus
}
#endif



#endif // AOLKME_H


