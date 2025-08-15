/**
 * @file Aolkme_event.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-08-11
 * 提供基于发布-订阅模式的事件管理系统，实现模块间解耦通信
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef AOLKME_EVENT_H
#define AOLKME_EVENT_H



#include "Aolkme_platform.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


#define     MAX_EVENT_SYSTEM_HANDLERS           16
#define     MAX_EVENT_SYSTEM_QUEUE_SIZE         32





typedef uint32_t E_AolkmeEventID;


typedef struct {
    E_AolkmeEventID                 ID;                   // !> Event ID
    uint32_t                        timestamp;            // !> Event timestamp
    void*                           source;               // !> Event source
    void*                           data;                 // !> Event data (can be NULL)
    size_t                          data_size;            // !> Event size
    const char*                     name;                 // !> Event name
    uint8_t                         flags;                // !> Event flags (e.g., priority, persistence)
} T_AolkmeEvent;






/**
 * @brief Event callback function type.
 * 
 */
typedef void (*AolkmeEventHandler)(T_AolkmeEvent event);


/**
 * @brief Event system configuration structure.
 */
typedef struct {
    uint16_t queue_size;          // !> Size of the event queue
    uint16_t task_stack_size;     // !> Event task stack size
    uint8_t task_priority;        // !> Event task priority
    uint8_t max_handlers;         // !> Maximum number of event handlers
    bool enable_auto_processing;  // !> Enable automatic event processing

    uint8_t reserved[2];          // !> Reserved for future use, must be zero
} T_AolkmeEventSystemConfig;



T_AolkmeReturnCode AolkmeEvent_Init(const T_AolkmeEventSystemConfig* config);

T_AolkmeReturnCode AolkmeEvent_Deinit(void);

T_AolkmeReturnCode AolkmeEvent_PublishEvent(T_AolkmeEvent* event);

T_AolkmeReturnCode AolkmeEvent_SubscribeEvent(AolkmeEventHandler handler);

T_AolkmeReturnCode AolkmeEvent_UnsubscribeEvent(AolkmeEventHandler handler);



T_AolkmeReturnCode AolkmeEvent_GetStatus(uint8_t* queue_usage, uint8_t* handler_count);















#ifdef __cplusplus
}
#endif




#endif // AOLKME_EVENT_H


