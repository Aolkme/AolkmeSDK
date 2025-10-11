
#define AOLKME_CORE_IMPLEMENTATION

#include "logger_buffer.h"
#include "logger_core.h"

#include "Aolkme_Core_Private.h"

#define LOGGER_BUFFER_BLOCK_SIZE 256



static T_AolkmeQueueHandle s_AolkmeLoggerBlockQueue = NULL;
static T_AolkmeMutexHandle s_AolkmeLoggerMutex = NULL;
static T_AolkmeTaskHandle  s_AolkmeLoggerFlushTask = NULL;
static bool b_flush_task_running = false;

/**
 * @brief Logger buffer flush task
 * 
 * @param arg 
 */
static void *AolkmeLogger_BufferFlushTask(void *arg)
{
    T_AolkmeReturnCode returncode;
    T_AolkmeOSALHandler *osal_handler = AolkmePlatform_GetOSALHandle();
    if (osal_handler == NULL) {
        printf("AolkmePlatform_GetOSALHandle is error\r\n");
        return NULL;
    }

    while(b_flush_task_running)
    {
        T_AolkmeLoggerBlock *block = NULL;

        returncode = osal_handler->QueueReceive(s_AolkmeLoggerBlockQueue, &block, AOLKME_OSAL_MAXDELAY);
        if (returncode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
        {
            printf("Get receive s_AolkmeLoggerBlockQueue is error!\r\n");
        }
        else
        {
            if (AolkmeCore_GetState() == AOLKME_CORE_STATE_RUNNING)
            {
                // Send to output
                for (uint8_t i = 0; i < g_aolkme_logger_state.output_count; i ++)
                {
                    if (block->level <= g_aolkme_logger_state.outputs[i].min_level)
                    {
                        g_aolkme_logger_state.outputs[i].func(block->data, block->length);
                    }
                }
            }
            // printf("[Flush] Freeing block at 0x%p\n", block);
            osal_handler->Free(block);
        }
        osal_handler->TaskSleepMs(10);
    }
	return NULL;
}


/**
 * @brief Initialize the logger buffer.
 * 
 * @param size The size of the buffer.
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeLogger_BufferInit(size_t size)
{
    
    T_AolkmeReturnCode returncode;

    T_AolkmeOSALHandler *osal_handler = AolkmePlatform_GetOSALHandle();
    if (osal_handler == NULL) {
        printf("AolkmePlatform_GetOSALHandle is error\r\n");
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    // create mutex
    returncode = osal_handler->MutexCreate(&s_AolkmeLoggerMutex);
    if (returncode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        printf("s_AolkmeLoggerMutex create is error!\r\n");
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    // create queue
    returncode = osal_handler->QueueCreate(size / sizeof(T_AolkmeLoggerBlock*), sizeof(T_AolkmeLoggerBlock*), &s_AolkmeLoggerBlockQueue);
    if (returncode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        osal_handler->MutexDestroy(s_AolkmeLoggerMutex);
        printf("s_AolkmeLoggerBlockQueue create is error!\r\n");
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    // create flush task
    returncode = osal_handler->TaskCreate("Aolkmeloggerflushtask", AolkmeLogger_BufferFlushTask, 2048, NULL, &s_AolkmeLoggerFlushTask);
    if(returncode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        // delete mutex
        returncode = osal_handler->MutexDestroy(s_AolkmeLoggerMutex);
        // delete queue
        returncode = osal_handler->QueueDestroy(s_AolkmeLoggerBlockQueue);

        printf("Aolkmeloggerflushtask create is error!\r\n");
        return returncode;
    }

    // buffer is enabled
    b_flush_task_running = true;

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}




/**
 * @brief Buffer processing
 * 
 * @param data 
 * @param datalen 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeLogger_BufferPut(E_AolkmeLoggerConsoleLogLevel level, uint8_t *data, uint16_t datalen)
{
    T_AolkmeReturnCode returnCode;
    T_AolkmeOSALHandler *osal_handler = AolkmePlatform_GetOSALHandle();
    if (osal_handler == NULL) {
        printf("AolkmePlatform_GetOSALHandle is error\r\n");
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    returnCode = osal_handler->MutexLock(s_AolkmeLoggerMutex);
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        printf("Mutex lock failed!\r\n");
        g_aolkme_logger_state.unlog_count++;
        return returnCode;
    }

    // printf("[Buffer] Allocating %d bytes for log block\n", sizeof(T_AolkmeLoggerBlock) + datalen);

    // Allocate log block
    size_t total_size = sizeof(T_AolkmeLoggerBlock) + datalen;
    if (total_size > LOGGER_BUFFER_BLOCK_SIZE) {
        g_aolkme_logger_state.unlog_count++;
        returnCode = AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_REQUEST_PARAMETER;
    }

    T_AolkmeLoggerBlock *block = osal_handler->Malloc(total_size);
    if (block == NULL)
    {
        printf("[Buffer] Memory allocation failed!\n");
        g_aolkme_logger_state.unlog_count++;
        returnCode = AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_REQUEST_PARAMETER;
    }

    // Initialize log block
    block->level = level;
    block->length = datalen;
    memcpy(block->data, data, datalen);

    // Add to queue
    returnCode = osal_handler->QueueSend(s_AolkmeLoggerBlockQueue, &block, AOLKME_OSAL_MAXDELAY);
    if(returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        osal_handler->Free(block);
        g_aolkme_logger_state.unlog_count++;
        returnCode = AOLKME_ERROR_SYSTEM_MODULE_CODE_QUEUE_EMPTY;
    }

    osal_handler->MutexUnlock(s_AolkmeLoggerMutex);
    return returnCode;

}

/**
 * @brief Flush the buffer (ensure all logs are output)
 * 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeLogger_BufferFlush(void)
{
    if(s_AolkmeLoggerBlockQueue == NULL)
    {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
    }

    T_AolkmeOSALHandler *osal_handler = AolkmePlatform_GetOSALHandle();
    if (osal_handler == NULL) {
        printf("AolkmePlatform_GetOSALHandle is error\r\n");
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    // 等待队列为空
    uint32_t count;
    osal_handler->QueueMessageCount(s_AolkmeLoggerBlockQueue, &count);
    while (count)
    {
        osal_handler->QueueMessageCount(s_AolkmeLoggerBlockQueue, &count);
        osal_handler->TaskSleepMs(10);
    }


    if (s_AolkmeLoggerBlockQueue != NULL)
    {
        // Flush remaining logs
        while (osal_handler->QueueMessageCount(s_AolkmeLoggerBlockQueue, &count) > 0)
        {
            osal_handler->TaskSleepMs(10);
        }
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}



/**
 * @brief Deinitialize the logger buffer.
 * 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeLogger_BufferDeinit(void)
{
    b_flush_task_running = false;
    AolkmeLogger_BufferFlush();

    T_AolkmeReturnCode returncode;
    T_AolkmeOSALHandler *osal_handler = AolkmePlatform_GetOSALHandle();
    if (osal_handler == NULL) {
        printf("AolkmePlatform_GetOSALHandle is error\r\n");
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    // destroy flush task
    if (s_AolkmeLoggerFlushTask != NULL)
    {
        returncode = osal_handler->TaskDestroy(s_AolkmeLoggerFlushTask);
        if(returncode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
        {
            return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
        }
        s_AolkmeLoggerFlushTask = NULL;
    }

    // destroy queue
    if (s_AolkmeLoggerBlockQueue != NULL)
    {
        returncode = osal_handler->QueueDestroy(s_AolkmeLoggerBlockQueue);
        if(returncode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
        {
            return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
        }
        s_AolkmeLoggerBlockQueue = NULL;
    }
    // destroy mutex
    if (s_AolkmeLoggerMutex != NULL)
    {
        returncode = osal_handler->MutexDestroy(s_AolkmeLoggerMutex);
        if(returncode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
        {
            return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
        }
        s_AolkmeLoggerMutex = NULL;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}



size_t AolkmeGetBlockSize(void)
{
    return sizeof(T_AolkmeLoggerBlock);
}





