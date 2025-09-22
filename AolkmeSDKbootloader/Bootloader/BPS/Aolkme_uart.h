#ifndef _AOLKME_UART_H
#define _AOLKME_UART_H


#include "Aolkme_OSAL.h"

#ifdef __cplusplus
extern "C" {
#endif




typedef struct
{
    uint32_t CountOfLostData;           /*!< Count of data lost, unit: byte. */
    uint16_t MaxUsedCapacityOfBuffer;   /*!< Max used capacity of buffer, unit: byte. */
} T_UartBufferState;






#ifdef __cplusplus
}
#endif



#endif // _AOLKME_UART_H



