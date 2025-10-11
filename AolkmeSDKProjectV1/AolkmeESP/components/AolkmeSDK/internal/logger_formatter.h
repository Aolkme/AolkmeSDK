/**
 * @file logger_formatter.h
 * @brief 
 * @author Aolkme
 * @
 * 
 */





#ifndef LOGGER_FORMATTER_H
#define LOGGER_FORMATTER_H

//#pragma once


#include "Aolkme_logger.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

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
                                const char *format, va_list args);





#ifdef __cplusplus
}
#endif



#endif // LOGGER_FORMATTER_H

