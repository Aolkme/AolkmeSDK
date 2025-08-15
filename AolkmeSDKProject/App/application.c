/**
 * @file application.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-08-11
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#include "application.h"
#include "Aolkme_OSAL.h"
#include "cmsis_os.h"
#include "main.h"
#include "usart.h"


#include "Aolkme_logger.h"
#include "Aolkme_event.h"
#include "AolkmeOSAL_SysMon.h"
#include "Aolkme_core.h"

#include "AolkmeSDK_User_info.h"
#include "Aolkme_misc.h"
#include "AolkmeOSAL_SysMon.h"



static T_AolkmeReturnCode Aolkme_FillInUserInfo(T_AolkmeUserInfo *userInfo);
static T_AolkmeReturnCode AolkmeUser_PrintConsole(const uint8_t *data, uint16_t dataLen);

 T_AolkmeTaskHandle eventgetTaskHandle;

 void *event_get(void *arg);

void AolkmeUser_StartTask(void *arg)
{
    T_AolkmeReturnCode returnCode;
	T_AolkmeUserInfo userInfo;
	
	
	
    T_AolkmeOSALHandler osalHandler = {
        .TaskCreate = A_Osal_TaskCreate,
        .TaskDestroy = A_Osal_TaskDestroy,
        .TaskSleepMs = A_Osal_TaskSleepMs,
        .MutexCreate = A_Osal_MutexCreate,
        .MutexDestroy = A_Osal_MutexDestroy,
        .MutexLock = A_Osal_MutexLock,
        .MutexUnlock = A_Osal_MutexUnlock,
        .SemaCreate = A_Osal_SemaphoreCreate,
        .BinarySemaphoreCreate = A_Osal_BinarySemaphoreCreate,
        .SemaDestroy = A_Osal_SemaphoreDestroy,
        .SemaWait = A_Osal_SemaphoreWait,
		.SemaTimedWait = A_Osal_SemaphoreTimedWait,
        .SemaPost = A_Osal_SemaphorePost,
        .GetTimeMs = A_Osal_GetTimeMs,
        .GetRandomNum = A_Osal_GetRandomNum,
        .Malloc = Osal_Malloc,
        .Free = Osal_Free,
    };


    T_AolkmeLoggerConfig loggerConfig = {
        .level = AOLKME_LOGGER_CONSOLE_LOG_LEVEL_MAX,
        .isSupportColor = false,
        .buffer_size = 30 * AolkmeGetBlockSize(),
    };

	T_AolkmeEventSystemConfig eventConfig = {
        .queue_size = 64,
        .task_stack_size = 2048,
        .task_priority = 5,
        .max_handlers = 16,
        .enable_auto_processing = true,
    };

	
	
    // Register OSAL handler
    returnCode = AolkmePlatform_RegOSALHandle(&osalHandler);
	if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
	{
		printf("register osal handler error\r\n");
		goto out;
	}
	printf("AolkmeOSAL init is OK!\r\n");
	
    returnCode = Aolkme_FillInUserInfo(&userInfo);
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        printf("Aolkme_FillInUserInfo is error\r\n");
        goto out;
    }
    printf("Aolkme_FillInUserInfo is OK!\r\n");

    returnCode = Aolkme_Core_Init(&userInfo);
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        printf("Aolkme_Core_Init is error\r\n");
        goto out;
    }
    printf("Aolkme_Core_Init is OK!\r\n");

	// Initialize logger
    returnCode = AolkmeLogger_Init(&loggerConfig);
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        printf("AolkmeLogger_Init is error\r\n");
        goto out;
    }
	printf("AolkmeLogger_Init is OK!\r\n");

	returnCode = AolkmeLogger_AddOutput(AolkmeUser_PrintConsole);
	if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        printf("AolkmeLogger_AddOutput is error\r\n");
        goto out;
    }
	printf("AolkmeLogger_AddOutput is OK!\r\n");

    // Initialize event system
    returnCode = AolkmeEvent_Init(&eventConfig);
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        printf("AolkmeEvent_Init is error\r\n");
        goto out;
    }
    printf("AolkmeEvent_Init is OK!\r\n");

    //    ALOG_INFO("Aolkme","Aolkme SDK app is Begining");

	
	Aolkme_Core_Application_Start();


    A_Osal_SystemMonitorInit(5000); // 5秒刷新一次
    A_Osal_SystemMonitorStart();

    osalHandler.TaskCreate("event_get_task", event_get, 2048, NULL, &eventgetTaskHandle);

	while(1){
		osalHandler.TaskSleepMs(500);
		HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);
   
        uint8_t queue_usage, handler_count;
        T_AolkmeReturnCode ret = AolkmeEvent_GetStatus(&queue_usage, &handler_count);
        
        if (ret == AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
        {
//            printf("Event System Status: Queue Usage=%d%%, Handlers=%d\n", 
//						queue_usage, handler_count);
        }
	}
	
out:
	osThreadExit();
}











void *event_get(void *arg)
{

	AolkmeEvent_SubscribeEvent(SystemMonitorEventHandler);
    while (1)
    {
		osDelay(100);
		
    }
}






static T_AolkmeReturnCode Aolkme_FillInUserInfo(T_AolkmeUserInfo *userInfo)
{
    memset(userInfo, 0, sizeof(T_AolkmeUserInfo));

    strncpy(userInfo->appName, USER_APP_NAME, sizeof(userInfo->appName) - 1);
    memcpy(userInfo->appId, USER_APP_ID, AOLKME_UTIL_MIN(sizeof(userInfo->appId), sizeof(USER_APP_ID)));
    memcpy(userInfo->appKey, USER_APP_KEY, AOLKME_UTIL_MIN(sizeof(userInfo->appKey), sizeof(USER_APP_KEY)));
    memcpy(userInfo->appVersion, USER_APP_VERSION, AOLKME_UTIL_MIN(sizeof(userInfo->appVersion), sizeof(USER_APP_VERSION)));

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}















static T_AolkmeReturnCode AolkmeUser_PrintConsole(const uint8_t *data, uint16_t dataLen)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)data, dataLen, AOLKME_OSAL_MAXDELAY);
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}


















