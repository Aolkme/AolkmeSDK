/**
 * @file logger_core.c
 * @brief core
 * @author Aolkme
 * @
 * 
 */

#include "logger_core.h"
#include "logger_formatter.h"
#include <stdbool.h>
#include <stdarg.h>


// Global logger state
T_AolkmeLoggerState g_aolkme_logger_state = {0};


/**
 * @brief Initialize the logger.
 * @param config Logger configuration.
 * @return T_AolkmeReturnCode
 */
T_AolkmeReturnCode AolkmeLogger_Init(const T_AolkmeLoggerConfig *config)
{
    if (g_aolkme_logger_state.initialized != false) {
        printf("AolkmeLogger has been initialized\r\n");
        return AOLKME_ERROR_LOGGER_MODULE_CODE_INITIALIZATION_HAS_BEEN_DONE;
    }

    memset(&g_aolkme_logger_state, 0, sizeof(T_AolkmeLoggerState));
    g_aolkme_logger_state.global_level = config->level;
    g_aolkme_logger_state.color_enabled = config->isSupportColor;

    T_AolkmeReturnCode returnCode;
    returnCode = AolkmeLogger_BufferInit(config->buffer_size);
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("AolkmeLogger_BufferInit is error\r\n");
        return returnCode;
    }

    g_aolkme_logger_state.initialized = true;
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}





/**
 * @brief Deinitialize the logger.
 * @return T_AolkmeReturnCode
 */
T_AolkmeReturnCode AolkmeLogger_Deinit(void)
{
    if (g_aolkme_logger_state.initialized != true) {
        printf("AolkmeLogger is not initialized\r\n");
        return AOLKME_ERROR_LOGGER_MODULE_CODE_INITIALIZATION_NOT_DONE;
    }

    T_AolkmeReturnCode returnCode;

    returnCode = AolkmeLogger_BufferFlush();
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("AolkmeLogger_BufferFlush is error\r\n");
        return returnCode;
    }

    returnCode = AolkmeLogger_BufferDeinit();
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("AolkmeLogger_BufferDeinit is error\r\n");
        return returnCode;
    }

    memset(&g_aolkme_logger_state, 0, sizeof(T_AolkmeLoggerState));
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}


/**
 * @brief Add a new output function to the logger.
 * @param output_func The output function to add.
 * @return T_AolkmeReturnCode
 */
T_AolkmeReturnCode AolkmeLogger_AddOutput(ConsoleOutputFunc output_func)
{
    if (g_aolkme_logger_state.initialized != true) {
        printf("AolkmeLogger is not initialized\r\n");
        return AOLKME_ERROR_LOGGER_MODULE_CODE_INITIALIZATION_NOT_DONE;
    }

    if (g_aolkme_logger_state.output_count >= MAX_OUTPUTS) {
        printf("AolkmeLogger output count is full\r\n");
        return AOLKME_ERROR_LOGGER_MODULE_CODE_NO_RESOURCE;
    }

    T_AolkmeLoggerOutput new_output = {
        .func = output_func,
        .min_level = g_aolkme_logger_state.global_level
    };


    g_aolkme_logger_state.outputs[g_aolkme_logger_state.output_count++] = new_output;
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}


/**
 * @brief Output a log message.
 * @param level Log level.
 * @param tag Log tag.
 * @param file Source file name.
 * @param line Source line number.
 * @param func Source function name.
 * @param format Log message format.
 * @param ... Log message arguments.
 */
void AolkmeLogger_Output(E_AolkmeLoggerConsoleLogLevel level, const char *tag,
                        const char *file, int line, const char *func, const char *format, ...)
{
    if (g_aolkme_logger_state.initialized != true) {
        printf("AolkmeLogger is not initialized\r\n");
        return;
    }

    if (level > g_aolkme_logger_state.global_level) {
        g_aolkme_logger_state.unlog_count++;
        return;
    }

    T_AolkmeOSALHandler *osal_handler = AolkmePlatform_GetOSALHandle();
    if (osal_handler == NULL) {
        printf("A_Osal_GetHandler is error\r\n");
        return;
    }

    char formatted[256];

    va_list args;
    va_start(args, format);

	uint32_t time;
	osal_handler->GetTimeMs(&time);
	
    int len = AolkmeLogger_FormatterFormat(formatted, sizeof(formatted), time, level, tag, file, line, func, format, args);
    if (len < 0) {
        printf("AolkmeLogger_FormatterFormat is error\r\n");
        return;
    }

    va_end(args);

    if (len > 0)
    {
        // 
        AolkmeLogger_BufferPut(level, (uint8_t *)formatted, (uint16_t)len);
        g_aolkme_logger_state.log_count++;
    }

}



















