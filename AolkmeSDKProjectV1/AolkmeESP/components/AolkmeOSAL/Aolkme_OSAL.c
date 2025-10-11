#include "limits.h"
#include "Aolkme_OSAL.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "semphr.h"
#include "stdlib.h"



#define SEM_MUTEX_WAIT_FOREVER          0xFFFFFFFF
#define TASK_PRIORITY_NORMAL            0



// 包装函数结构体
typedef struct {
    void *(*userFunc)(void *);
    void *userArg;
} TaskParams;



// 包装函数 - 符合FreeRTOS的函数签名
static void taskFuncWrapper(void *param)
{
    TaskParams *params = (TaskParams *)param;
    if (params && params->userFunc) {
        // 调用用户函数并忽略返回值
        params->userFunc(params->userArg);
    }
    
    // 清理参数内存
    if (params) {
        vPortFree(params);
    }
    
    // FreeRTOS任务应该永不返回，如果需要返回则删除任务
    vTaskDelete(NULL);
}









/***   此为旧版 Task_Create 出现参数 警告（旧）
 * Task_Create
 * @brief 创建一个任务
 * @param name 任务名称
 * @param taskFunc 任务函数
 * @param stackSize 任务栈大小
 * @param arg 任务函数参数
 * @param task 任务句柄
 * @return T_AolkmeReturnCode
 *
T_AolkmeReturnCode A_Osal_TaskCreate(const char *name, void *(*taskFunc)(void *), uint32_t stackSize, void *arg, T_AolkmeTaskHandle *task)
{
    uint32_t stackDepth;
    char nameDealed[16] = {0};

    //attention :  freertos use stack depth param, stack size = (stack depth) * sizeof(StackType_t)
    if (stackSize % sizeof(StackType_t) == 0) {
        stackDepth = stackSize / sizeof(StackType_t);
    } else {
        stackDepth = stackSize / sizeof(StackType_t) + 1;
    }

    if (name != NULL)
        strncpy(nameDealed, name, sizeof(nameDealed) - 1);
    if (xTaskCreate((TaskFunction_t) taskFunc, nameDealed, stackDepth, arg, TASK_PRIORITY_NORMAL, (TaskHandle_t *)task) != pdPASS) {
        *task = NULL;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}
*/



/** 新版本创建任务，修复警告（新）
 * Task_Create
 * @brief 创建一个任务
 * @param name 任务名称
 * @param taskFunc 任务函数
 * @param stackSize 任务栈大小
 * @param arg 任务函数参数
 * @param task 任务句柄
 * @return T_AolkmeReturnCode
 *
 */
T_AolkmeReturnCode A_Osal_TaskCreate(const char *name, void *(*taskFunc)(void *), uint32_t stackSize, void *arg, T_AolkmeTaskHandle *task)
{
    uint32_t stackDepth;
    char nameDealed[16] = {0};

    // attention: freertos use stack depth param, stack size = (stack depth) * sizeof(StackType_t)
    if (stackSize % sizeof(StackType_t) == 0) {
        stackDepth = stackSize / sizeof(StackType_t);
    } else {
        stackDepth = stackSize / sizeof(StackType_t) + 1;
    }

    if (name != NULL)
        strncpy(nameDealed, name, sizeof(nameDealed) - 1);
    
    // 创建包装函数来适配FreeRTOS的函数签名
    TaskFunction_t wrapperFunc = (TaskFunction_t)taskFuncWrapper;
    
    // 创建任务参数结构体
    TaskParams *params = pvPortMalloc(sizeof(TaskParams));
    if (params == NULL) {
        *task = NULL;
        return AOLKME_ERROR_OSAL_MODULE_CODE_OUT_OF_MEMORY;
    }
    
    params->userFunc = taskFunc;
    params->userArg = arg;
    
    if (xTaskCreate(wrapperFunc, nameDealed, stackDepth, params, TASK_PRIORITY_NORMAL, (TaskHandle_t *)task) != pdPASS) {
        vPortFree(params);
        *task = NULL;
        return AOLKME_ERROR_OSAL_MODULE_CODE_MUTEXCREATE_FAILED;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}













/**
 * Task_Destroy
 */
T_AolkmeReturnCode A_Osal_TaskDestroy(T_AolkmeTaskHandle task)
{
    if (task == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    vTaskDelete(task);

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * Task_Dealy
 */
T_AolkmeReturnCode A_Osal_TaskSleepMs(uint32_t timeMs)
{
    TickType_t ticks;

    ticks = timeMs / portTICK_PERIOD_MS;

    /* Minimum delay = 1 tick */
    vTaskDelay(ticks ? ticks : 1);

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * Mutex_Create
 */
T_AolkmeReturnCode A_Osal_MutexCreate(T_AolkmeMutexHandle *mutex)
{
    if (mutex == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    *mutex = xSemaphoreCreateMutex();
    if (*mutex == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * Mutex_Destroy
 */
T_AolkmeReturnCode A_Osal_MutexDestroy(T_AolkmeMutexHandle mutex)
{
    if (mutex == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    vQueueDelete((SemaphoreHandle_t) mutex);

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * Mutex_Lock
 */
T_AolkmeReturnCode A_Osal_MutexLock(T_AolkmeMutexHandle mutex)
{
    TickType_t ticks;

    if (mutex == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    ticks = portMAX_DELAY;

    if (xSemaphoreTake(mutex, ticks) != pdTRUE) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * Mutex_Unlock
 */
T_AolkmeReturnCode A_Osal_MutexUnlock(T_AolkmeMutexHandle mutex)
{
    if (mutex == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    if (xSemaphoreGive(mutex) != pdTRUE) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * Semaphore_Create
 */
T_AolkmeReturnCode A_Osal_SemaphoreCreate(uint32_t initValue, T_AolkmeSemaHandle *semaphore)
{
    uint32_t maxCount = UINT_MAX;

    if (semaphore == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    *semaphore = xSemaphoreCreateCounting(maxCount, initValue);

    if (*semaphore == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * Binary_Semaphore_Create
 */
T_AolkmeReturnCode A_Osal_BinarySemaphoreCreate(T_AolkmeSemaHandle *semaphore)
{
    if (semaphore == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    *semaphore = xSemaphoreCreateBinary();
    if (*semaphore == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}


/**
 * Semaphore_Destroy
 */
T_AolkmeReturnCode A_Osal_SemaphoreDestroy(T_AolkmeSemaHandle semaphore)
{
    if (semaphore == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    vSemaphoreDelete(semaphore);

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * Semaphore_Timed_Wait
 */
T_AolkmeReturnCode A_Osal_SemaphoreTimedWait(T_AolkmeSemaHandle semaphore, uint32_t waitTimeMs)
{
    TickType_t ticks;

    if (semaphore == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    ticks = 0;
    if (waitTimeMs == SEM_MUTEX_WAIT_FOREVER) {
        ticks = portMAX_DELAY;
    } else if (waitTimeMs != 0) {
        ticks = waitTimeMs / portTICK_PERIOD_MS;
        if (ticks == 0) {
            ticks = 1;
        }
    }
    if (xSemaphoreTake(semaphore, ticks) != pdTRUE) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * Semaphore_Wait
 */
T_AolkmeReturnCode A_Osal_SemaphoreWait(T_AolkmeSemaHandle semaphore)
{
    A_Osal_SemaphoreTimedWait(semaphore, SEM_MUTEX_WAIT_FOREVER);

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * Semaphore_Post
 */
T_AolkmeReturnCode A_Osal_SemaphorePost(T_AolkmeSemaHandle semaphore)
{
    if (semaphore == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    if (xSemaphoreGive(semaphore) != pdTRUE) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * Get_TimeMs
 */
T_AolkmeReturnCode A_Osal_GetTimeMs(uint32_t *ms)
{
    if (ms == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    *ms = xTaskGetTickCount();

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * Get_TimeUs
 */
T_AolkmeReturnCode A_Osal_GetTimeUs(uint64_t *us)
{
    if (us == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    *us = xTaskGetTickCount() * 1000;

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}


/**
 * Get_Random_Num
 */
T_AolkmeReturnCode A_Osal_GetRandomNum(uint16_t *randomNum)
{
    *randomNum = rand() % 65535;

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}


void *Osal_Malloc(uint32_t size)
{
    return pvPortMalloc(size);
}

void Osal_Free(void *ptr)
{
    vPortFree(ptr);
}


/********************** 队列操作补充 **********************/
// 补充时间 2025/7/13 | 04点14分
/**
 * 创建队列
 * @param queueLength 队列长度（最大元素数量）
 * @param itemSize 每个队列元素的大小（字节）
 * @param queue 返回的队列句柄
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueCreate(uint32_t queueLength, uint32_t itemSize, T_AolkmeQueueHandle *queue)
{
    if (queue == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    *queue = xQueueCreate(queueLength, itemSize);
    if (*queue == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * 销毁队列
 * @param queue 要销毁的队列句柄
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueDestroy(T_AolkmeQueueHandle queue)
{
    if (queue == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    vQueueDelete(queue);
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * 发送数据到队列（后入队）
 * @param queue 队列句柄
 * @param item 要发送的数据指针
 * @param waitTimeMs 等待时间（毫秒）
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueSend(T_AolkmeQueueHandle queue, const void *item, uint32_t waitTimeMs)
{
    TickType_t ticks;
    BaseType_t result;

    if (queue == NULL || item == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    // 转换等待时间为系统tick
    if (waitTimeMs == SEM_MUTEX_WAIT_FOREVER) {
        ticks = portMAX_DELAY;
    } else {
        ticks = waitTimeMs / portTICK_PERIOD_MS;
    }

    result = xQueueSend(queue, item, ticks);
    
    if (result != pdPASS) {
        return (result == errQUEUE_FULL) ? 
               AOLKME_ERROR_SYSTEM_MODULE_CODE_QUEUE_FULL : 
               AOLKME_ERROR_SYSTEM_MODULE_CODE_TIMEOUT;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * 发送数据到队列前部（前入队）
 * @param queue 队列句柄
 * @param item 要发送的数据指针
 * @param waitTimeMs 等待时间（毫秒）
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueSendToFront(T_AolkmeQueueHandle queue, const void *item, uint32_t waitTimeMs)
{
    TickType_t ticks;
    BaseType_t result;

    if (queue == NULL || item == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    // 转换等待时间为系统tick
    if (waitTimeMs == SEM_MUTEX_WAIT_FOREVER) {
        ticks = portMAX_DELAY;
    } else {
        ticks = waitTimeMs / portTICK_PERIOD_MS;
    }

    result = xQueueSendToFront(queue, item, ticks);
    
    if (result != pdPASS) {
        return (result == errQUEUE_FULL) ? 
               AOLKME_ERROR_SYSTEM_MODULE_CODE_QUEUE_FULL : 
               AOLKME_ERROR_SYSTEM_MODULE_CODE_TIMEOUT;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * 从队列接收数据
 * @param queue 队列句柄
 * @param buffer 接收数据的缓冲区
 * @param waitTimeMs 等待时间（毫秒）
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueReceive(T_AolkmeQueueHandle queue, void *buffer, uint32_t waitTimeMs)
{
    TickType_t ticks;
    BaseType_t result;

    if (queue == NULL || buffer == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    // 转换等待时间为系统tick
    if (waitTimeMs == SEM_MUTEX_WAIT_FOREVER) {
        ticks = portMAX_DELAY;
    } else {
        ticks = waitTimeMs / portTICK_PERIOD_MS;
    }

    result = xQueueReceive(queue, buffer, ticks);
    
    if (result != pdPASS) {
        return (result == errQUEUE_EMPTY) ? 
               AOLKME_ERROR_SYSTEM_MODULE_CODE_QUEUE_EMPTY : 
               AOLKME_ERROR_SYSTEM_MODULE_CODE_TIMEOUT;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * 获取队列中当前元素数量
 * @param queue 队列句柄
 * @param count 返回的元素数量
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueMessageCount(T_AolkmeQueueHandle queue, uint32_t *count)
{
    if (queue == NULL || count == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    *count = uxQueueMessagesWaiting(queue);
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * 重置队列（清空所有元素）
 * @param queue 队列句柄
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueReset(T_AolkmeQueueHandle queue)
{
    if (queue == NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    xQueueReset(queue);
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

// ... 保持原有的内存管理函数不变 ...


