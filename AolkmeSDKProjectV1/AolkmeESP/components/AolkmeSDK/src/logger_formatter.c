#include "logger_formatter.h"
#include "logger_core.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h> 



/**
 * @brief log level strings
 * 
 */
static const char *LEVEL_STRINGS[] = {
    [AOLKME_LOGGER_CONSOLE_LOG_LEVEL_FATAL] = "FATAL",
    [AOLKME_LOGGER_CONSOLE_LOG_LEVEL_ERROR] = "ERROR",
    [AOLKME_LOGGER_CONSOLE_LOG_LEVEL_WARN]  = "WARN",
    [AOLKME_LOGGER_CONSOLE_LOG_LEVEL_INFO]  = "INFO",
    [AOLKME_LOGGER_CONSOLE_LOG_LEVEL_DEBUG] = "DEBUG",
    [AOLKME_LOGGER_CONSOLE_LOG_LEVEL_TRACE] = "TRACE",
};

/**
 * @brief colors
 * 
 */
static const char *LEVEL_COLORS[] = {
    [AOLKME_LOGGER_CONSOLE_LOG_LEVEL_FATAL] = "\033[1;31m",     // 红
    [AOLKME_LOGGER_CONSOLE_LOG_LEVEL_ERROR] = "\033[0;31m",     // 亮红
    [AOLKME_LOGGER_CONSOLE_LOG_LEVEL_WARN]  = "\033[0;33m",     // 黄
    [AOLKME_LOGGER_CONSOLE_LOG_LEVEL_INFO]  = "\033[0;32m",     // 绿
    [AOLKME_LOGGER_CONSOLE_LOG_LEVEL_DEBUG] = "\033[0;34m",     // 蓝
    [AOLKME_LOGGER_CONSOLE_LOG_LEVEL_TRACE] = "\033[0;35m",     // 紫
};

/**
 * @brief format log message
 * 
 * @param buf 
 * @param size 
 * @param timestamp 
 * @param level 
 * @param tag 
 * @param file 
 * @param line 
 * @param func 
 * @param format 
 * @return int 
 */
int AolkmeLogger_FormatterFormat(char *buf, size_t size, uint32_t timestamp, E_AolkmeLoggerConsoleLogLevel level,
                                const char *tag, const char *file, int line, const char *func,
                                const char *format, va_list args)
{
    // Extract filename
    const char *base_file = strrchr(file, '/');        // Search backward from the end of the path string for the last '/' character.
    if(base_file == NULL){
        base_file = strrchr(file, '\\');               // Search backward from the end of the path string for the last '\\' character.
    }
    base_file = base_file ? base_file + 1 : file;

    // Format prefix
    int pos = 0;

    // Add color if enabled
    if(g_aolkme_logger_state.color_enabled && level < AOLKME_LOGGER_CONSOLE_LOG_LEVEL_MAX)
    {
        pos += snprintf(buf + pos, size - pos, "%s-", LEVEL_COLORS[level]);
    }

    // Timestamp
    pos += snprintf(buf + pos, size - pos, "%6" PRIu32 "-", timestamp);

    // Log level
    pos += snprintf(buf + pos, size - pos, "[%s-]", level < AOLKME_LOGGER_CONSOLE_LOG_LEVEL_MAX ? LEVEL_STRINGS[level] : "???");


    // Tag (if provided)
    if (tag && *tag)
    {
        pos += snprintf(buf + pos, size - pos, "[%s]", tag);
    }

    // File and line
    pos += snprintf(buf + pos, size - pos, "-%s:%d -> %s() ->> :", base_file, line, func);

    // Format message
    int msg_len = vsnprintf(buf + pos, size - pos, format, args);
    if(msg_len < 0) return -1;
    pos += msg_len;

    // Add color reset
    if (g_aolkme_logger_state.color_enabled && level < AOLKME_LOGGER_CONSOLE_LOG_LEVEL_MAX)
    {
        if (pos < size - 5) 
        {
            pos += snprintf(buf + pos, size - pos, "\033[0m");
        }
    }

    // Add newline
    if (pos < size - 2)
    {
        buf[pos ++] = '\r';
        buf[pos ++] = '\n';
        buf[pos] = '\0';
    }else if (pos < size) {
        buf[pos] = '\0'; // 确保字符串终止
    }

    return pos;
}





