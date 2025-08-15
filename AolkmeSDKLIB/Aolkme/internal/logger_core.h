/**
 * @file Aolkme.h
 * @brief core
 * @author Aolkme
 * @
 * 
 */




#ifndef LOGGER_CORE_H
#define LOGGER_CORE_H

//#pragma once


#include "Aolkme_logger.h"

#include "logger_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @brief Maximum output backends
 */
#define MAX_OUTPUTS 5

/**
 * @brief Maximum log length
 */
#define MAX_LOG_LENGTH 256


/**
 * @brief Logger output structure
 */
typedef struct{
    ConsoleOutputFunc func;                     // !< Output function
    E_AolkmeLoggerConsoleLogLevel min_level;        // !< Log level
} T_AolkmeLoggerOutput;


/**
 * @brief Logger global state structure
 */
typedef struct{
    bool initialized;                                   // !< Initialized flag
    E_AolkmeLoggerConsoleLogLevel global_level;         // !< Log level
    bool color_enabled;                                 // !< Color output enabled

    T_AolkmeLoggerOutput outputs[MAX_OUTPUTS];          // !< Output backends
    uint8_t output_count;                               // !< Current backend count

    // Performance counters
    uint32_t log_count;                                  // !< Log count
    uint32_t unlog_count;                                // !< Unlogged/dropped log count

} T_AolkmeLoggerState;




// Global logger state
extern T_AolkmeLoggerState g_aolkme_logger_state;


#ifdef __cplusplus
}
#endif



#endif // LOGGER_CORE_H


