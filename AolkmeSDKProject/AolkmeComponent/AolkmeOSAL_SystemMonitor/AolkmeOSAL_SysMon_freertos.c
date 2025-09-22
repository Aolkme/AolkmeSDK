/**
 * @file AolkmeOSAL_SysMon.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-08-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */



#include "AolkmeOSAL_SysMon.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>


#ifndef configGENERATE_RUN_TIME_STATS
#error "请在 FreeRTOSConfig.h 中启用 configGENERATE_RUN_TIME_STATS 和 configUSE_TRACE_FACILITY"
#endif

typedef struct {
    void *taskHandle;
    uint16_t stackTotalWords;
} TaskRegistry_t;


// 定义监控报告数据结构
typedef struct {
    T_AolkmeSystemResource resource;
    uint32_t taskCount;
    T_AolkmeTaskStatus *tasks;
} T_AolkmeMonitorReport;




#define MAX_TASK_REGISTRY  32



static TaskRegistry_t g_taskRegistry[MAX_TASK_REGISTRY];
static TaskHandle_t monitorTaskHandle = NULL;
static uint32_t monitorPeriodTicks = 0;


static const char *stateToStr(T_AolkmeTaskState state) {
    switch (state) {
        case A_TASK_RUNNING:   return "Running";
        case A_TASK_READY:     return "Ready";
        case A_TASK_BLOCKED:   return "Blocked";
        case A_TASK_SUSPENDED: return "Suspended";
        case A_TASK_DELETED:   return "Deleted";
        default:               return "Invalid";
    }
}




void A_Osal_RegisterTaskStackSize(void *taskHandle, uint16_t stackDepthWords) {
    for (int i = 0; i < MAX_TASK_REGISTRY; i++) {
        if (g_taskRegistry[i].taskHandle == NULL) {
            g_taskRegistry[i].taskHandle = taskHandle;
            g_taskRegistry[i].stackTotalWords = stackDepthWords;
            return;
        }
    }
}

static uint16_t findStackTotal(void *handle) {
    for (int i = 0; i < MAX_TASK_REGISTRY; i++) {
        if (g_taskRegistry[i].taskHandle == handle) {
            return g_taskRegistry[i].stackTotalWords;
        }
    }
    return 0;
}

T_AolkmeReturnCode A_Osal_SystemMonitorGetResource(T_AolkmeSystemResource *info) {
    if (!info) return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    info->freeHeapBytes = xPortGetFreeHeapSize();
    info->minEverFreeHeapBytes = xPortGetMinimumEverFreeHeapSize();
    info->taskCount = uxTaskGetNumberOfTasks();
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_AolkmeReturnCode A_Osal_SystemMonitorGetTaskList(T_AolkmeTaskStatus *tasks, uint32_t *taskCount) {
    if (!tasks || !taskCount) return AOLKME_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;

    TaskStatus_t *taskArray = pvPortMalloc((*taskCount) * sizeof(TaskStatus_t));
    if (!taskArray) return AOLKME_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;

    uint32_t totalRunTime;
    uint32_t actualCount = uxTaskGetSystemState(taskArray, *taskCount, &totalRunTime);

    for (uint32_t i = 0; i < actualCount; i++) {
        tasks[i].taskName = taskArray[i].pcTaskName;
        tasks[i].stackFreeWords = taskArray[i].usStackHighWaterMark;
        tasks[i].stackTotalWords = findStackTotal(taskArray[i].xHandle);
        tasks[i].cpuUsagePercent = (totalRunTime > 0) ?
                                   (taskArray[i].ulRunTimeCounter * 100UL / totalRunTime) : 0;
        tasks[i].runTimeTicks = taskArray[i].ulRunTimeCounter;
        tasks[i].priority = taskArray[i].uxCurrentPriority;
        tasks[i].taskHandle = taskArray[i].xHandle;
        tasks[i].taskState = (T_AolkmeTaskState)taskArray[i].eCurrentState;
    }

    *taskCount = actualCount;
    vPortFree(taskArray);
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}


#if 0
/**
 * @brief 
 * 
 * @param pvParameters 
 */
static void AolkmeMonitorTask(void *pvParameters)
{
    while (1) {
        T_AolkmeSystemResource res;
        A_Osal_SystemMonitorGetResource(&res);

        printf("[SysMon] Heap Free: %u, Min Ever: %u, Task Count: %u\n",
               res.freeHeapBytes, res.minEverFreeHeapBytes, res.taskCount);

        T_AolkmeTaskStatus *tasks = pvPortMalloc(res.taskCount * sizeof(T_AolkmeTaskStatus));
        if (tasks) {
            uint32_t count = res.taskCount;
            A_Osal_SystemMonitorGetTaskList(tasks, &count);
            for (uint32_t i = 0; i < count; i++) {
                printf("Task: %-16s Pri: %u Stack: %u Free: %u CPU: %u%% State: %s Runtime: %lu ticks\n",
                       tasks[i].taskName,
                       tasks[i].priority,
                       tasks[i].stackTotalWords,
                       tasks[i].stackFreeWords,
                       tasks[i].cpuUsagePercent,
                       stateToStr(tasks[i].taskState),
                       (unsigned long)tasks[i].runTimeTicks);
            }
            vPortFree(tasks);
        }

        vTaskDelay(monitorPeriodTicks);
    }
}

#else
/**
 * @brief System Monitor Task - publish system snapshot as event
 */
static void AolkmeMonitorTask(void *pvParameters)
{
    while (1) {
        T_AolkmeSystemResource res;
        A_Osal_SystemMonitorGetResource(&res);

        // 计算一次性分配所需大小
        size_t totalSize = sizeof(T_AolkmeMonitorReport) + res.taskCount * sizeof(T_AolkmeTaskStatus);
        T_AolkmeMonitorReport *report = pvPortMalloc(totalSize);
        if (report) {
            memset(report, 0, totalSize);
            report->resource = res;
            report->taskCount = res.taskCount;
            report->tasks = (T_AolkmeTaskStatus *)(report + 1); // 指向紧随其后的数组

            uint32_t count = res.taskCount;
            if (A_Osal_SystemMonitorGetTaskList(report->tasks, &count) == 
                AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) 
            {
                // 构造事件
                T_AolkmeEvent monitorEvent;
                monitorEvent.ID        = AOLKME_EVENT_SYSTEM_MONITOR_REPORT;
                monitorEvent.timestamp = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
                monitorEvent.source    = NULL;
                monitorEvent.data      = report;
                monitorEvent.data_size = totalSize;
                monitorEvent.name      = "SystemMonitorReport";
                monitorEvent.flags     = EVENT_FLAG_DYNAMIC_DATA;  // 标记需要释放

                // 发布事件
                if (AolkmeEvent_PublishEvent(&monitorEvent) != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
                    // 发布失败，手动释放
                    vPortFree(report);
                }
            } else {
                vPortFree(report);
            }
        }

        vTaskDelay(monitorPeriodTicks);
    }
}

#endif








void SystemMonitorEventHandler(T_AolkmeEvent event)
{
    if (event.ID == AOLKME_EVENT_SYSTEM_MONITOR_REPORT) {
        T_AolkmeMonitorReport *report = (T_AolkmeMonitorReport *)event.data;

        printf("[SysMon] Heap Free: %u, Min Ever: %u, Task Count: %u\n",
               report->resource.freeHeapBytes,
               report->resource.minEverFreeHeapBytes,
               report->taskCount);

        for (uint32_t i = 0; i < report->taskCount; i++) {
            printf("Task: %-16s Pri: %u, Stack: %u Free: %u, CPU: %u%%, State: %s\n",
                   report->tasks[i].taskName,
                   report->tasks[i].priority,
                   report->tasks[i].stackTotalWords,
                   report->tasks[i].stackFreeWords,
                   report->tasks[i].cpuUsagePercent,
                   //(int)report->tasks[i].taskState,
					stateToStr(report->tasks[i].taskState));
        }

        // ⚠️ 注意：这里不需要 vPortFree，事件系统会在分发完成后自动释放
    }
}






T_AolkmeReturnCode A_Osal_SystemMonitorInit(uint32_t periodMs) {
    memset(g_taskRegistry, 0, sizeof(g_taskRegistry));
    monitorPeriodTicks = pdMS_TO_TICKS(periodMs);
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_AolkmeReturnCode A_Osal_SystemMonitorStart(void) {
    if (monitorTaskHandle != NULL) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }
    if (xTaskCreate(AolkmeMonitorTask, "SysMon", 128, NULL, tskIDLE_PRIORITY + 1, &monitorTaskHandle) != pdPASS) {
        return AOLKME_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }
    A_Osal_RegisterTaskStackSize(monitorTaskHandle, 128);
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_AolkmeReturnCode A_Osal_SystemMonitorStop(void) {
    if (monitorTaskHandle) {
        vTaskDelete(monitorTaskHandle);
        monitorTaskHandle = NULL;
    }
    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}





