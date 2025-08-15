/**
 * @file AolkmeOSAL_SysMon.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-08-14
 * 
 */



#ifndef AOLKME_OSAL_SYSTEM_MONITOR_H
#define AOLKME_OSAL_SYSTEM_MONITOR_H

#include "Aolkme_platform.h"
#include "Aolkme_event.h"
#include "Aolkme_event_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    A_TASK_RUNNING   = 0,
    A_TASK_READY     = 1,
    A_TASK_BLOCKED   = 2,
    A_TASK_SUSPENDED = 3,
    A_TASK_DELETED   = 4,
    A_TASK_INVALID   = 5
} T_AolkmeTaskState;





/**
 * @brief Structure to hold the status of the event system.
 */
typedef struct {
    const char *taskName;
    uint16_t stackFreeWords;
    uint16_t stackTotalWords;    // 总栈大小
    uint32_t cpuUsagePercent;
    uint32_t runTimeTicks;
    uint8_t  priority;
    void    *taskHandle;
    T_AolkmeTaskState taskState;
} T_AolkmeTaskStatus;


/**
 * @brief Structure to hold the resource usage of the system.
 * 
 */
typedef struct {
    uint32_t freeHeapBytes;
    uint32_t minEverFreeHeapBytes;
    uint32_t taskCount;
} T_AolkmeSystemResource;

void SystemMonitorEventHandler(T_AolkmeEvent event);

T_AolkmeReturnCode A_Osal_SystemMonitorInit(uint32_t periodMs);
T_AolkmeReturnCode A_Osal_SystemMonitorStart(void);
T_AolkmeReturnCode A_Osal_SystemMonitorStop(void);
T_AolkmeReturnCode A_Osal_SystemMonitorGetResource(T_AolkmeSystemResource *info);
T_AolkmeReturnCode A_Osal_SystemMonitorGetTaskList(T_AolkmeTaskStatus *tasks, uint32_t *taskCount);




#ifdef __cplusplus
}
#endif


#endif // AOLKME_OSAL_SYSTEM_MONITOR_H



