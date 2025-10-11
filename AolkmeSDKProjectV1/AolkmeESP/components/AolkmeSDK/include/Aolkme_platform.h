/**
 * @file Aolkme_platform.h
 * @brief 内核
 * @author Aolkme
 * @
 * 
 */

// 头部引用格式




#ifndef AOLKME_PLATFORM_H
#define AOLKME_PLATFORM_H

//#pragma once

#include "Aolkme_typedef.h"


#ifdef __cplusplus
extern "C" {
#endif


#define    AOLKME_OSAL_QUEUE            1
#define    AOLKME_OSAL_MAXDELAY         0xFFFFFFFF



/**
 * @brief Platform handle of UART operation.
 */
typedef void *T_AolkmeUartHandle;


/**
* @brief Platform handle of i2c device operation.
*/
typedef void *T_AolkmeI2cHandle;

/**
* @brief Platform handle of thread task operation.
*/
typedef void *T_AolkmeTaskHandle;

/**
* @brief Platform handle of thread mutex operation.
*/
typedef void *T_AolkmeMutexHandle;

/**
* @brief Platform handle of semaphore operation.
*/
typedef void *T_AolkmeSemaHandle;

/**
* @brief Platform handle of queue operation.
*/
typedef void *T_AolkmeQueueHandle;





typedef enum
{
    AOLKME_HAL_UART_NUM_1,
    AOLKME_HAL_UART_NUM_2,
    AOLKME_HAL_UART_NUM_3
}E_AolkmeHalUartNum;


typedef enum
{
    AOLKME_HAL_UART_STATE_IDLE,        /*!< UART is idle */
    AOLKME_HAL_UART_STATE_BUSY,        /*!< UART is busy */
    AOLKME_HAL_UART_STATE_ERROR,       /*!< UART has an error */
    AOLKME_HAL_UART_STATE_TIMEOUT       /*!< UART operation timed out */
} E_AolkmeHalUartState;



typedef struct 
{
    uint16_t pid;
    uint16_t vid;
} E_AolkmeHalUartDeviceInfo;




typedef struct {
    uint32_t i2cSpeed;
    uint16_t devAddress;
} T_AolkmeHalI2cConfig;




/**
 * @brief Aolkme time structure.
 * 
 */
typedef struct{
    uint16_t year;       /*!< Year */
    uint8_t month;      /*!< Month */
    uint8_t day;        /*!< Day */
    uint8_t hour;       /*!< Hour */
    uint8_t minute;     /*!< Minute */
    uint8_t second;     /*!< Second */
}T_AolkmeTime;





/**
 * @brief Aolkme UART handle structure.
 * 
 */
typedef struct 
{
    T_AolkmeReturnCode (*UartInit)(E_AolkmeHalUartNum uartNum, uint32_t baudRate, T_AolkmeUartHandle *uartHandle);
    T_AolkmeReturnCode (*UartDeInit)(T_AolkmeUartHandle uartHandle);
    T_AolkmeReturnCode (*UartWriteData)(T_AolkmeUartHandle uartHandle, const uint8_t *buf, uint32_t len, uint32_t realLen);
    T_AolkmeReturnCode (*UartReadData)(T_AolkmeUartHandle uartHandle, uint8_t *buf, uint32_t len, uint32_t *realLen);
    T_AolkmeReturnCode (*UartGetState)(T_AolkmeUartHandle uartHandle, E_AolkmeHalUartState *state);
}T_AolkmeHalUartHandler;




typedef struct 
{
    T_AolkmeReturnCode (*I2cInit)(T_AolkmeHalI2cConfig i2cConfig, T_AolkmeI2cHandle *i2cHandle);
}T_AolkmeHalI2cHandler;

//typedef struct 
//{
//}T_AolkmeHalSpiHandler;


/**
 * @brief Aolkme task handle structure.
 * 
 */
typedef struct 
{
    T_AolkmeReturnCode (*TaskCreate)(const char *name, void *(*taskFunc)(void *), uint32_t stackSize, void *arg, T_AolkmeTaskHandle *task);
    T_AolkmeReturnCode (*TaskDestroy)(T_AolkmeTaskHandle task);
    T_AolkmeReturnCode (*TaskSleepMs)(uint32_t timeMs);
    T_AolkmeReturnCode (*MutexCreate)(T_AolkmeMutexHandle *mutex);
    T_AolkmeReturnCode (*MutexDestroy)(T_AolkmeMutexHandle mutex);
    T_AolkmeReturnCode (*MutexLock)(T_AolkmeMutexHandle mutex);
    T_AolkmeReturnCode (*MutexUnlock)(T_AolkmeMutexHandle mutex);
    T_AolkmeReturnCode (*SemaCreate)(uint32_t initValue, T_AolkmeSemaHandle *semaphore);
    T_AolkmeReturnCode (*BinarySemaphoreCreate)(T_AolkmeSemaHandle *semaphore);
    T_AolkmeReturnCode (*SemaDestroy)(T_AolkmeSemaHandle semaphore);
    T_AolkmeReturnCode (*SemaWait)(T_AolkmeSemaHandle semaphore);
    T_AolkmeReturnCode (*SemaTimedWait)(T_AolkmeSemaHandle semaphore, uint32_t waitTimeMs);
    T_AolkmeReturnCode (*SemaPost)(T_AolkmeSemaHandle semaphore);
//	T_AolkmeReturnCode (*SemaPostFromISR)(T_AolkmeSemaHandle semaphore, int *pxHigherPriorityTaskWoken);
    T_AolkmeReturnCode (*GetTimeMs)(uint32_t *ms);
    T_AolkmeReturnCode (*GetTimeUs)(uint32_t *us);
    T_AolkmeReturnCode (*GetRandomNum)(uint16_t *randomNum);
    void *(*Malloc)(uint32_t size);
    void (*Free)(void *ptr);

#if AOLKME_OSAL_QUEUE
    T_AolkmeReturnCode (*QueueCreate)(uint32_t queueLength, uint32_t itemSize, T_AolkmeQueueHandle *queue);
    T_AolkmeReturnCode (*QueueDestroy)(T_AolkmeQueueHandle queue);
    T_AolkmeReturnCode (*QueueSend)(T_AolkmeQueueHandle queue, const void *item, uint32_t waitTimeMs);
    T_AolkmeReturnCode (*QueueReceive)(T_AolkmeQueueHandle queue, void *buffer, uint32_t waitTimeMs);
    
    T_AolkmeReturnCode (*QueueMessageCount)(T_AolkmeQueueHandle queue, uint32_t *count);
    T_AolkmeReturnCode (*QueueReset)(T_AolkmeQueueHandle queue);

#endif

} T_AolkmeOSALHandler;




T_AolkmeReturnCode AolkmePlatform_RegOSALHandle(const T_AolkmeOSALHandler *osalHandle);

T_AolkmeOSALHandler *AolkmePlatform_GetOSALHandle(void);





#ifdef __cplusplus
}
#endif



#endif // AOLKME_H


