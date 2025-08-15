/**
 * @file Aolkme_event.c
 * @author Aolkme
 * @brief 
 * @version 0.1
 * @date 2025-08-11
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#define AOLKME_CORE_IMPLEMENTATION


#include "Aolkme_event.h"
#include "Aolkme_core_private.h"


#define EVENT_FLAG_DYNAMIC_DATA 0x01  ///< 事件数据为动态分配，需要释放


// event system context
typedef struct 
{
    // event handler
    AolkmeEventHandler* handlers;
    T_AolkmeEvent*                  queue;                     ///< Queue for storing events
    T_AolkmeMutexHandle             mutex;                     ///< Mutex for synchronizing access to the event system
    T_AolkmeSemaHandle              event_sem;                 ///< Semaphore for signaling event processing
    T_AolkmeTaskHandle              task_handle;               ///< Task handle for event processing
    
    // queue
    uint16_t                        queue_capacity;             ///< Maximum size of the event queue
    uint16_t                        head;                       ///< Head pointer for the event queue
    uint16_t                        tail;                       ///< Tail pointer for the event queue
    // handler array
    uint16_t                        handler_capacity;           ///< Maximum number of event handlers
    uint16_t                        handler_count;              ///< Number of registered event handlers

    
    bool                            initialized;                  ///< Flag indicating if the event system is initialized
    bool                            task_running;                 ///< Flag indicating if the event processing task is running

} T_AolkmeEventSystemContext;

// event system context
static T_AolkmeEventSystemContext g_event_system_context = {0};


// Event processing task
static void *event_processing_task(void* arg);





// 内部函数声明
static T_AolkmeReturnCode event_system_lock(void);
static T_AolkmeReturnCode event_system_unlock(void);
static void free_event_data(T_AolkmeEvent* event);







/**
 * @brief Initialize the event system.
 * 
 * @param config Pointer to the event system configuration.
 * @return T_AolkmeReturnCode Result code indicating success or failure.
 */
T_AolkmeReturnCode AolkmeEvent_Init(const T_AolkmeEventSystemConfig* config)
{
    if (g_event_system_context.initialized) {
        return AOLKME_ERROR_EVENT_MODULE_CODE_INITIALIZATION_HAS_BEEN_DONE;
    }

    if (config == NULL || config->queue_size == 0 || config->max_handlers == 0) {
        return AOLKME_ERROR_EVENT_MODULE_CODE_INVALID_PARAMETER;
    }

    T_AolkmeOSALHandler* osal_handler = AolkmePlatform_GetOSALHandle();
    if (osal_handler == NULL) {
        return AOLKME_ERROR_EVENT_MODULE_CODE_INVALID_REQUEST_PARAMETER;
    }

    // Initialize mutex
    T_AolkmeReturnCode returncode;
    returncode = osal_handler->MutexCreate(&g_event_system_context.mutex);
    if (returncode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return AOLKME_ERROR_OSAL_MODULE_CODE_MUTEXCREATE_FAILED;
    }

    returncode = osal_handler->SemaCreate(0, &g_event_system_context.event_sem);
    if (returncode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        osal_handler->MutexDestroy(g_event_system_context.mutex);
        return returncode;
    }

    // Initialize event queue
    g_event_system_context.queue = (T_AolkmeEvent*)osal_handler->Malloc(config->queue_size * sizeof(T_AolkmeEvent));
    if (g_event_system_context.queue == NULL) {
        osal_handler->MutexDestroy(g_event_system_context.mutex);
        osal_handler->SemaDestroy(g_event_system_context.event_sem);
        return AOLKME_ERROR_OSAL_MODULE_CODE_OUT_OF_MEMORY;
    }

    // Allocate memory for handlers
    g_event_system_context.handlers = (AolkmeEventHandler*)osal_handler->Malloc(config->max_handlers * sizeof(AolkmeEventHandler));
    if (g_event_system_context.handlers == NULL) {
        osal_handler->Free(g_event_system_context.queue);
        osal_handler->MutexDestroy(g_event_system_context.mutex);
        osal_handler->SemaDestroy(g_event_system_context.event_sem);
        return AOLKME_ERROR_OSAL_MODULE_CODE_OUT_OF_MEMORY;
    }

    // Initialize event system context
    g_event_system_context.handler_capacity = config->max_handlers;
    g_event_system_context.queue_capacity = config->queue_size;
    g_event_system_context.head = 0;
    g_event_system_context.tail = 0;
    g_event_system_context.handler_count = 0;
    g_event_system_context.initialized = true;
    g_event_system_context.task_running = false;

    // Initialize event handlers
    for (uint16_t i = 0; i < g_event_system_context.handler_capacity; i++) {
        g_event_system_context.handlers[i] = NULL;
    }


    // Create event processing task if auto processing is enabled
    if (config->enable_auto_processing) {
        returncode = osal_handler->TaskCreate("EventTask", event_processing_task, config->task_stack_size, NULL, &g_event_system_context.task_handle);
        if (returncode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
            osal_handler->Free(g_event_system_context.handlers);
            osal_handler->Free(g_event_system_context.queue);
            osal_handler->MutexDestroy(g_event_system_context.mutex);
            osal_handler->SemaDestroy(g_event_system_context.event_sem);
            memset(&g_event_system_context, 0, sizeof(g_event_system_context));
            return returncode;
        }
        g_event_system_context.task_running = true;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}


/**
 * @brief Deinitialize the event system.
 * 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeEvent_Deinit(void) 
{
    if (!g_event_system_context.initialized) {
        return AOLKME_ERROR_EVENT_MODULE_CODE_INITIALIZATION_HAS_BEEN_DONE;
    }

    T_AolkmeOSALHandler* osal_handler = AolkmePlatform_GetOSALHandle();
    if (osal_handler == NULL) {
        return AOLKME_ERROR_EVENT_MODULE_CODE_INVALID_REQUEST_PARAMETER;
    }

    if (g_event_system_context.task_running) {
        // Signal the task to stop
        g_event_system_context.task_running = false;
        osal_handler->SemaPost(g_event_system_context.event_sem);
        osal_handler->TaskSleepMs(100);
    }

    // Unregister all handlers
    event_system_lock();
    while (g_event_system_context.head != g_event_system_context.tail) {
        T_AolkmeEvent* event = &g_event_system_context.queue[g_event_system_context.tail];
        free_event_data(event);
        g_event_system_context.tail = (g_event_system_context.tail + 1) % g_event_system_context.queue_capacity;
    }
    event_system_unlock();

    // Clean up resources
    osal_handler->Free(g_event_system_context.queue);
    osal_handler->Free(g_event_system_context.handlers);
    osal_handler->MutexDestroy(g_event_system_context.mutex);
    osal_handler->SemaDestroy(g_event_system_context.event_sem);

    memset(&g_event_system_context, 0, sizeof(g_event_system_context));

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * @brief Publish an event to the event system.
 * 
 * @param event 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeEvent_PublishEvent(T_AolkmeEvent* event)
{
    if (!g_event_system_context.initialized) {
        return AOLKME_ERROR_EVENT_MODULE_CODE_INITIALIZATION_HAS_BEEN_DONE;
    }

    T_AolkmeOSALHandler* osal_handler = AolkmePlatform_GetOSALHandle();
    if (osal_handler == NULL) {
        return AOLKME_ERROR_EVENT_MODULE_CODE_INVALID_REQUEST_PARAMETER;
    }


    // Get current timestamp
    uint32_t time_ms;
    osal_handler->GetTimeMs(&time_ms);
    event->timestamp = time_ms;

    // Lock the mutex
    T_AolkmeReturnCode returncode;
    returncode = event_system_lock();
    if (returncode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return returncode;
    }

    uint16_t next_head = (g_event_system_context.head + 1) % g_event_system_context.queue_capacity;
    if (next_head == g_event_system_context.tail) {
        event_system_unlock();
        return AOLKME_ERROR_EVENT_MODULE_CODE_EVENT_QUEUE_FULL;
    }

    // Publish the event
    g_event_system_context.queue[g_event_system_context.head] = *event;
    g_event_system_context.head = next_head;

    event_system_unlock();

    if (g_event_system_context.task_running) {
        osal_handler->SemaPost(g_event_system_context.event_sem);
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}


/**
 * @brief Subscribe to an event.
 * 
 * @param handler 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeEvent_SubscribeEvent(AolkmeEventHandler handler)
{
    if (!g_event_system_context.initialized) {
        return AOLKME_ERROR_EVENT_MODULE_CODE_INITIALIZATION_NOT_DONE;
    }

    if (handler == NULL) {
        return AOLKME_ERROR_EVENT_MODULE_CODE_INVALID_PARAMETER;
    }

    // Lock the mutex
    T_AolkmeReturnCode returncode;
    returncode = event_system_lock();
    if (returncode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return returncode;
    }

    // Check if already subscribed
    for (int i = 0; i < g_event_system_context.handler_capacity; i++) {
        if (g_event_system_context.handlers[i] == handler) {
            event_system_unlock();
            return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS; // 已订阅视为成功
        }
    }

    // Check for available slot
    for (int i = 0; i < g_event_system_context.handler_capacity; i++) {
        if (g_event_system_context.handlers[i] == NULL) {
            g_event_system_context.handlers[i] = handler;
            g_event_system_context.handler_count++;
            event_system_unlock();
            return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
        }
    }

    event_system_unlock();
    return AOLKME_ERROR_EVENT_MODULE_CODE_OUT_OF_RESOURCES;
}


/**
 * @brief Unsubscribe from an event.
 * 
 * @param handler 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeEvent_UnsubscribeEvent(AolkmeEventHandler handler)
{
    if (!g_event_system_context.initialized) {
        return AOLKME_ERROR_EVENT_MODULE_CODE_INITIALIZATION_NOT_DONE;
    }

    if (handler == NULL) {
        return AOLKME_ERROR_EVENT_MODULE_CODE_INVALID_PARAMETER;
    }

    // Lock the mutex
    T_AolkmeReturnCode returnCode;
    returnCode = event_system_lock();
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return returnCode;
    }

    // Find and remove the handler
    for (int i = 0; i < g_event_system_context.handler_capacity; i++) {
        if (g_event_system_context.handlers[i] == handler) {
            g_event_system_context.handlers[i] = NULL;
            g_event_system_context.handler_count--;
            event_system_unlock();
            return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
        }
    }

    event_system_unlock();
    return AOLKME_ERROR_EVENT_MODULE_CODE_HANDLER_NOT_FOUND;
}








/**
 * @brief Get the status of the event system.
 * 
 * @param queue_usage 
 * @param handler_count 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode AolkmeEvent_GetStatus(uint8_t* queue_usage, uint8_t* handler_count) 
{
    if (!g_event_system_context.initialized) {
        return AOLKME_ERROR_EVENT_MODULE_CODE_INITIALIZATION_NOT_DONE;
    }

    T_AolkmeReturnCode returnCode;
    returnCode = event_system_lock();
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return returnCode;
    }

    if (queue_usage) {
        uint16_t count = (g_event_system_context.head >= g_event_system_context.tail) ?
                         (g_event_system_context.head - g_event_system_context.tail) :
                         (g_event_system_context.queue_capacity - g_event_system_context.tail + g_event_system_context.head);
        *queue_usage = (uint8_t)((count * 100) / g_event_system_context.queue_capacity);
    }

    if (handler_count) {
        *handler_count = g_event_system_context.handler_count;
    }

    returnCode = event_system_unlock();
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return returnCode;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}





// ================= Task Management ================= //

static void *event_processing_task(void* arg)
{
    T_AolkmeOSALHandler *osal = AolkmePlatform_GetOSALHandle();
    if (!osal) {
        return NULL;
    }
    
    while (g_event_system_context.task_running) {
        // 等待事件或超时
        osal->SemaTimedWait(g_event_system_context.event_sem, 100);

        // 处理所有可用事件
        while (g_event_system_context.task_running && (AolkmeCore_GetState() == AOLKME_CORE_STATE_RUNNING)) {
            T_AolkmeEvent event;
            bool has_event = false;
            
            // 从队列获取事件
            if (event_system_lock() == AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
                if (g_event_system_context.tail != g_event_system_context.head) {
                    event = g_event_system_context.queue[g_event_system_context.tail];
                    g_event_system_context.tail = (g_event_system_context.tail + 1) % g_event_system_context.queue_capacity;
                    has_event = true;
                }
                event_system_unlock();
            }
            
            if (has_event) {
                // 分发到所有订阅者
                event_system_lock();
                for (int i = 0; i < g_event_system_context.handler_capacity; i++) {
                    if (g_event_system_context.handlers[i]) {
                        g_event_system_context.handlers[i](event);
                    }
                }
                event_system_unlock();
                
                // 释放事件数据
                free_event_data(&event);
            } else {
                // 没有更多事件，跳出内层循环
                break;
            }
        }
    }
    
    // 任务退出
    osal->TaskDestroy(NULL);
    return NULL;
}


 








// ================= 内部工具函数 ================= //

static T_AolkmeReturnCode event_system_lock(void) {
    T_AolkmeOSALHandler *osal = AolkmePlatform_GetOSALHandle();
    if (!osal) {
        return AOLKME_ERROR_PLATFORM_MODULE_CODE_INVALID_REQUEST_PARAMETER;
    }
    return osal->MutexLock(g_event_system_context.mutex);
}

static T_AolkmeReturnCode event_system_unlock(void) {
    T_AolkmeOSALHandler *osal = AolkmePlatform_GetOSALHandle();
    if (!osal) {
        return AOLKME_ERROR_PLATFORM_MODULE_CODE_INVALID_REQUEST_PARAMETER;
    }
    return osal->MutexUnlock(g_event_system_context.mutex);
}

static void free_event_data(T_AolkmeEvent* event) {
    if (event->data && (event->flags & EVENT_FLAG_DYNAMIC_DATA)) {
        T_AolkmeOSALHandler *osal = AolkmePlatform_GetOSALHandle();
        if (osal) {
            osal->Free(event->data);
        }
    }
}


