/**
 * @file Aolkme_core_private.h
 * @brief Private header file for the Aolkme core library.
 *
 * This file contains private declarations and definitions for the Aolkme core library.
 * It should only be included by source files within the Aolkme library.
 */




#ifndef AOLKME_CORE_PRIVATE_H
#define AOLKME_CORE_PRIVATE_H

#include "Aolkme_core.h"



// Ensure this header is only included in the implementation files
#if !defined(AOLKME_CORE_IMPLEMENTATION)
#error "This private header is only for use within Aolkme_core implementation"
#endif


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Enumeration of Aolkme core states.
 */
typedef enum {
    AOLKME_CORE_STATE_UNINIT = 0x00,
    AOLKME_CORE_STATE_INIT = 0x01,
    AOLKME_CORE_STATE_READY = 0x10,
    AOLKME_CORE_STATE_RUNNING = 0x90,
    AOLKME_CORE_STATE_RUNNING_VIP = 0xAA,
    AOLKME_CORE_STATE_RUNNING_SVIP = 0xAB,
    AOLKME_CORE_STATE_STOPPED = 0x04,
    AOLKME_CORE_STATE_ERROR = 0xFF,
} E_AolkmeCoreState;

/**
 * @brief Aolkme core context structure.
 * 
 */
typedef struct 
{
    // Core state
    E_AolkmeCoreState               state;                  /*!< Current state of the Aolkme core */
    T_AolkmeUserInfo                userInfo;               /*!< User information for the Aolkme core */
    T_AolkmeOSALHandler             *osalHandler;           /*!< OSAL handler for the Aolkme core */
    T_AolkmeMutexHandle             mutexHandle;           /*!< Mutex handle for the Aolkme core */
    uint32_t                        initTimeMs;             /*!< Initialization time in milliseconds */

    // Task management
    T_AolkmeTaskHandle               *taskHandle;           /*!< Task handle for the Aolkme core */
    uint32_t                        mainTaskStack;          /*!< Main task stack size in bytes */
    char                            mainTaskNameBuf[16];    /*!< Main task name buffer */


} T_AolkmeCoreContext;







T_AolkmeReturnCode AolkmeCore_TransitionState(E_AolkmeCoreState newState);

E_AolkmeCoreState AolkmeCore_GetState(void);




bool Aolkme_GenerateKey(char* id, char* outKey, size_t outSize);

bool Aolkme_VerifyKey(char* id, char* key);




#ifdef __cplusplus
}
#endif

#endif // AOLKME_CORE_PRIVATE_H


