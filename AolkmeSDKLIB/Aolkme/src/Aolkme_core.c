/**
 * @file Aolkme_core.c
 * @author Aolkme
 * @brief 
 * @version 0.1
 * @date 2025-08-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */




// Ensure this header is only included in the implementation files
#define AOLKME_CORE_IMPLEMENTATION


#include "Aolkme_core_private.h"
#include <string.h>




/* Default application credentials */
static const char appName[32] = "AolkmeSDK";
static const char appId[16] = "AolkmeSDKadmin";       
static const char AppKey[32] = "AK17WIZJENEY2WKTZVLMF4FG2FJI3";

/* Global core context */
static T_AolkmeCoreContext s_aolkme_core_context = {
    .state = AOLKME_CORE_STATE_UNINIT,
    .osalHandler = NULL,
    .mutexHandle = NULL,
    .initTimeMs = 0,
};


/* State strings for diagnostics */
static const char* const s_aolkme_core_state_strings[] = {
    [AOLKME_CORE_STATE_UNINIT]          =    "UNINIT",
    [AOLKME_CORE_STATE_INIT]            =    "INIT",
    [AOLKME_CORE_STATE_READY]           =    "READY",
    [AOLKME_CORE_STATE_RUNNING]         =    "RUNNING",
    [AOLKME_CORE_STATE_RUNNING_VIP]     =    "RUNNING_VIP",
    [AOLKME_CORE_STATE_RUNNING_SVIP]    =    "RUNNING_SVIP",
    [AOLKME_CORE_STATE_STOPPED]         =    "STOPPED",
    [AOLKME_CORE_STATE_ERROR]           =    "ERROR"
};



bool AolkmeCore_VerifyUserInfo(T_AolkmeUserInfo *userInfo);




/**
 * @brief Initialize the Aolkme SDK core in blocking mode.
 * 
 * @param userInfo Pointer to user information structure.
 * @return T_AolkmeReturnCode Returns success or error code.
 */
T_AolkmeReturnCode Aolkme_Core_Init(T_AolkmeUserInfo *userInfo)
{
	T_AolkmeReturnCode returnCode;
    // Check initialization state
    if (s_aolkme_core_context.state != AOLKME_CORE_STATE_UNINIT) {
        printf("Aolkme core is already initialized.\r\n");
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    // Check userInfo validity
    if (userInfo == NULL) {
        printf("User information is not valid.\r\n");
        return AOLKME_ERROR_CORE_MODULE_CODE_INVALID_PARAMETER;
    }

    // Check userInfo validity
    if (!AolkmeCore_VerifyUserInfo(userInfo)) {
        printf("User information is not valid.\r\n");
        return AOLKME_ERROR_CORE_MODULE_CODE_INVALID_PARAMETER;
    }

    // Copy user information
    memcpy(&s_aolkme_core_context.userInfo, userInfo, sizeof(T_AolkmeUserInfo));

    // Initialize OSAL handler
    s_aolkme_core_context.osalHandler = AolkmePlatform_GetOSALHandle();

    // Get current time in milliseconds
    s_aolkme_core_context.osalHandler->GetTimeMs(&s_aolkme_core_context.initTimeMs);

    
    returnCode = s_aolkme_core_context.osalHandler->MutexCreate(&s_aolkme_core_context.mutexHandle);
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("Failed to create mutex.\r\n");
        return returnCode;
    }

    returnCode = AolkmeCore_TransitionState(AOLKME_CORE_STATE_INIT);
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("Failed to transition to INIT state.\r\n");
        return returnCode;
    }


    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}


T_AolkmeReturnCode Aolkme_Core_Application_Start(void)
{
    if (AolkmeCore_TransitionState(AOLKME_CORE_STATE_RUNNING) != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("Failed to transition to RUNNING state.\r\n");
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}  

/**
 * @brief DeInitialize the Aolkme SDK core in blocking mode.
 * 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode Aolkme_Core_DeInit(void)
{
    // De-initialization logic here
    // For now, we just return success
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}











// <! ------------------- State Transition ---------------------- !>


T_AolkmeReturnCode AolkmeCore_TransitionState(E_AolkmeCoreState newState)
{
    // Check if the new state is valid
    if (newState > AOLKME_CORE_STATE_ERROR) {
        printf("Invalid state transition requested: %d\r\n", newState);
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    // Lock the mutex before changing the state
    s_aolkme_core_context.osalHandler->MutexLock(s_aolkme_core_context.mutexHandle);

    // Transition to the new state
    s_aolkme_core_context.state = newState;

    // Log the state transition
    printf("Aolkme core state transitioned to: %s\r\n", s_aolkme_core_state_strings[newState]);

    s_aolkme_core_context.osalHandler->MutexUnlock(s_aolkme_core_context.mutexHandle);

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}


/**
 * @brief Get the current state of the Aolkme core.
 * 
 * @return E_AolkmeCoreState 
 */
E_AolkmeCoreState AolkmeCore_GetState(void)
{
    s_aolkme_core_context.osalHandler->MutexLock(s_aolkme_core_context.mutexHandle);

    E_AolkmeCoreState currentState = s_aolkme_core_context.state;

    s_aolkme_core_context.osalHandler->MutexUnlock(s_aolkme_core_context.mutexHandle);

    return currentState;
}





















// <! ------------------- User Information Verification ---------------------- !>



/**
 * @brief Verify user information.
 * 
 * @param userInfo Pointer to user information structure.
 * @return true if verification is successful, false otherwise.
 */
bool AolkmeCore_VerifyUserInfo(T_AolkmeUserInfo *userInfo)
{
    bool result = true;
    if(strncmp(userInfo->appName, appName, sizeof(appName) - 1) == 0) {
        result = Aolkme_GenerateKey(userInfo->appId, userInfo->appKey, sizeof(userInfo->appKey));
        if (result != true) {
            printf("Failed to generate key for user info verification.\r\n");
            return false;
        }
        result = Aolkme_VerifyKey(userInfo->appId, userInfo->appKey);
        if (result != true) {
            printf("Failed to verify key for user info verification.\r\n");
            return false;
        }
    }else{
        result = Aolkme_VerifyKey(userInfo->appId, userInfo->appKey);
        if (result != true) {
            printf("Failed to verify key for user info verification.\r\n");
            return false;
        }
    }
    return true;
}









