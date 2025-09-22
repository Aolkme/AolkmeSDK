
#include "Aolkme_uart.h"
#include "Aolkme_ringbuffer.h"
#include "main.h"

#define UART_RX_BUFFER_SIZE		1024
#define UART_TX_BUFFER_SIZE		1024


// UART read ring buffer structure
static T_AolkmeRingBuffer UartReadRingBuffer;
// UART read buffer state
static T_UartBufferState UartReadBufferState;
// UART write ring buffer structure
static T_AolkmeRingBuffer UartWriteRingBuffer;
// UART write buffer state
static T_UartBufferState UartWriteBufferState;
// UART read buffer
CCMRAM static uint8_t UartReadBuffer[UART_RX_BUFFER_SIZE];
// UART write buffer
CCMRAM static uint8_t UartWriteBuffer[UART_TX_BUFFER_SIZE];

// UART Mutex
static T_AolkmeMutexHandle UartMutexHandle;
// UART handle
static UART_HandleTypeDef *bootloaderUartHandle;



/**
 * @brief Initialize the UART interface
 * 
 * @param huart Pointer to the UART handle
 */
void AolkmeUart_Init(UART_HandleTypeDef *huart)
{
    bootloaderUartHandle = huart;
    
    // Initialize ring buffers
    RingBuf_Init(&UartReadRingBuffer, UartReadBuffer, UART_RX_BUFFER_SIZE);
    RingBuf_Init(&UartWriteRingBuffer, UartWriteBuffer, UART_TX_BUFFER_SIZE);

    // Create mutex
    A_Osal_MutexCreate(&UartMutexHandle);
    
    __HAL_UART_ENABLE_IT(bootloaderUartHandle, UART_IT_RXNE);


}



/**
 * @brief UART interrupt handler, should be called in the actual UART IRQ handler
 */
int AolkmeUart_Read(uint8_t *buf, uint16_t readSize)
{
    uint16_t readRealSize = 0;

    A_Osal_MutexLock(UartMutexHandle);

    readRealSize = RingBuf_Get(&UartReadRingBuffer, buf, readSize);

    A_Osal_MutexUnlock(UartMutexHandle);

    return readRealSize;
}




int AolkmeUart_Write(const uint8_t *buf, uint16_t writeSize)
{
    uint16_t writeRealSize = 0;
    

    A_Osal_MutexLock(UartMutexHandle);

    writeRealSize = RingBuf_Put(&UartWriteRingBuffer, buf, writeSize);

    A_Osal_MutexUnlock(UartMutexHandle);

    // Enable TXE interrupt to start transmission
    __HAL_UART_ENABLE_IT(bootloaderUartHandle, UART_IT_TXE);

    return writeRealSize;
}



















