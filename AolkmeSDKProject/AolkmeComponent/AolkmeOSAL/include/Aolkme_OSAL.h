/**
 * @file Aolkme.h
 * @brief 内核
 * @author Aolkme
 * @
 * 
 */

// 头部引用格式



#ifndef AOLKME_OSAL_H
#define AOLKME_OSAL_H

// #pragma once

#include "Aolkme_platform.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * Task_Create
 */
T_AolkmeReturnCode A_Osal_TaskCreate(const char *name, void *(*taskFunc)(void *), uint32_t stackSize, void *arg, T_AolkmeTaskHandle *task);
/**
 * Task_Destroy
 */
T_AolkmeReturnCode A_Osal_TaskDestroy(T_AolkmeTaskHandle task);
/**
 * Task_Dealy
 */
T_AolkmeReturnCode A_Osal_TaskSleepMs(uint32_t timeMs);
/**
 * Mutex_Create
 */
T_AolkmeReturnCode A_Osal_MutexCreate(T_AolkmeMutexHandle *mutex);
/**
 * Mutex_Destry
 */
T_AolkmeReturnCode A_Osal_MutexDestroy(T_AolkmeMutexHandle mutex);
/**
 * Mutex_Lock
 */
T_AolkmeReturnCode A_Osal_MutexLock(T_AolkmeMutexHandle mutex);
/**
 * Mutex_Unlock
 */
T_AolkmeReturnCode A_Osal_MutexUnlock(T_AolkmeMutexHandle mutex);
/**
 * Semaphore_Create
 */
T_AolkmeReturnCode A_Osal_SemaphoreCreate(uint32_t initValue, T_AolkmeSemaHandle *semaphore);
/**
 * Binary_Semaphore_Create
 */
T_AolkmeReturnCode A_Osal_BinarySemaphoreCreate(T_AolkmeSemaHandle *semaphore);
/**
 * Semaphore_Destroy
 */
T_AolkmeReturnCode A_Osal_SemaphoreDestroy(T_AolkmeSemaHandle semaphore);
/**
 * Semaphore_Timed_Wait
 */
T_AolkmeReturnCode A_Osal_SemaphoreTimedWait(T_AolkmeSemaHandle semaphore, uint32_t waitTimeMs);
/**
 * Semaphore_Wait
 */
T_AolkmeReturnCode A_Osal_SemaphoreWait(T_AolkmeSemaHandle semaphore);
/**
 * Semaphore_Post
 */
T_AolkmeReturnCode A_Osal_SemaphorePost(T_AolkmeSemaHandle semaphore);
/**
 * Get_TimeMs
 */
T_AolkmeReturnCode A_Osal_GetTimeMs(uint32_t *ms);
/**
 * Get_TimeUs
 */
T_AolkmeReturnCode A_Osal_GetTimeUs(uint64_t *us);
/**
 * Get_Random_Num
 */
T_AolkmeReturnCode A_Osal_GetRandomNum(uint16_t *randomNum);


void *Osal_Malloc(uint32_t size);
void Osal_Free(void *ptr);


/********************** 队列操作补充 **********************/
/**
 * 创建队列
 * @param queueLength 队列长度（最大元素数量）
 * @param itemSize 每个队列元素的大小（字节）
 * @param queue 返回的队列句柄
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueCreate(uint32_t queueLength, uint32_t itemSize, T_AolkmeQueueHandle *queue);

/**
 * 销毁队列
 * @param queue 要销毁的队列句柄
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueDestroy(T_AolkmeQueueHandle queue);

/**
 * 发送数据到队列（后入队）
 * @param queue 队列句柄
 * @param item 要发送的数据指针
 * @param waitTimeMs 等待时间（毫秒）
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueSend(T_AolkmeQueueHandle queue, const void *item, uint32_t waitTimeMs);

/**
 * 发送数据到队列前部（前入队）
 * @param queue 队列句柄
 * @param item 要发送的数据指针
 * @param waitTimeMs 等待时间（毫秒）
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueSendToFront(T_AolkmeQueueHandle queue, const void *item, uint32_t waitTimeMs);

/**
 * 从队列接收数据
 * @param queue 队列句柄
 * @param buffer 接收数据的缓冲区
 * @param waitTimeMs 等待时间（毫秒）
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueReceive(T_AolkmeQueueHandle queue, void *buffer, uint32_t waitTimeMs);

/**
 * 获取队列中当前元素数量
 * @param queue 队列句柄
 * @param count 返回的元素数量
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueMessageCount(T_AolkmeQueueHandle queue, uint32_t *count);

/**
 * 重置队列（清空所有元素）
 * @param queue 队列句柄
 * @return 操作结果状态码
 */
T_AolkmeReturnCode A_Osal_QueueReset(T_AolkmeQueueHandle queue);



#ifdef __cplusplus
}
#endif


#endif

