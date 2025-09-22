#ifndef _COMMON_H
#define _COMMON_H



#include "stm32f4xx.h"


#ifdef __cplusplus
extern "C" {
#endif

#define FILE_NAME_LENGTH    ((uint32_t)64)

#define TX_TIMEOUT          ((uint32_t)100)
#define RX_TIMEOUT          ((uint32_t)0xFFFFFFFF)


#define IS_CAP_LETTER(c)    (((c) >= 'A') && ((c) <= 'F'))
#define IS_LC_LETTER(c)     (((c) >= 'a') && ((c) <= 'f'))
#define IS_09(c)            (((c) >= '0') && ((c) <= '9'))
#define ISVALIDHEX(c)       (IS_CAP_LETTER(c) || IS_LC_LETTER(c) || IS_09(c))
#define ISVALIDDEC(c)       IS_09(c)
#define CONVERTDEC(c)       (c - '0')

#define CONVERTHEX_ALPHA(c) (IS_CAP_LETTER(c) ? ((c) - 'A'+10) : ((c) - 'a'+10))
#define CONVERTHEX(c)       (IS_09(c) ? ((c) - '0') : CONVERTHEX_ALPHA(c))
#define CRC16               ((uint8_t)0x43)  /* 'C' == 0x43, request 16-bit CRC */



typedef enum
{
    COM_OK       = 0x00,
    COM_ERROR    = 0x01,
    COM_ABORT    = 0x02,
    COM_TIMEOUT  = 0x03,
    COM_DATA     = 0x04,
    COM_LIMIT    = 0x05
} COM_StatusTypeDef;




void In2Str(uint8_t *p_str, uint32_t intnum);
uint32_t Str2Int(uint8_t *p_inputstr, uint32_t *p_intnum);
void Serial_PutString(uint8_t *p_string);
HAL_StatusTypeDef Serial_PutByte( uint8_t param );




#ifdef __cplusplus
}
#endif



#endif // _COMMON_H



