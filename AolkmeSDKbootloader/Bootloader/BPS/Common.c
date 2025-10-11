#include "Common.h"
#include "main.h"
#include "uart.h"





/**
 * @brief Convert an integer to a string.
 *
 * @param p_str Pointer to the output string buffer.
 * @param intnum The integer to convert.
 */
void Int2Str(uint8_t *p_str, uint32_t intnum)
{
    uint32_t i, divider = 1000000000, pos = 0, leading_zero = 1;
    for (i = 0; i < 10; i++)
    {
        uint8_t digit = (intnum / divider) % 10;
        if (digit != 0 || !leading_zero || divider == 1)
        {
            leading_zero = 0;
            p_str[pos++] = digit + '0';
        }
        divider /= 10;
    }
    if (pos == 0)
    {
        p_str[pos++] = '0';
    }
    p_str[pos] = '\0';

}



/**
 * @brief Convert a string to an integer.
 *
 * @param p_inputstr Pointer to the input string.
 * @param p_intnum Pointer to the output integer.
 * @return uint32_t 1 if successful, 0 otherwise.
 */
uint32_t Str2Int(uint8_t *p_inputstr, uint32_t *p_intnum)
{
    uint32_t i = 0, res = 0;
    uint32_t val = 0;

    if ((p_inputstr[0] == '0') && ((p_inputstr[1] == 'x') || (p_inputstr[1] == 'X'))) {
        i = 2;
        while ((i < 11) && (p_inputstr[i] != '\0')) {
            if (ISVALIDHEX(p_inputstr[i])) {
                val = (val << 4) + CONVERTHEX(p_inputstr[i]);
            } else {
                /* Return 0, Invalid input */
                res = 0;
                break;
            }
            i++;
        }

        /* valid result */
        if (p_inputstr[i] == '\0') {
            *p_intnum = val;
            res = 1;
        }
    } else /* max 10-digit decimal input */
    {
        while ((i < 11) && (res != 1)) {
            if (p_inputstr[i] == '\0') {
                *p_intnum = val;
                /* return 1 */
                res = 1;
            } else if (((p_inputstr[i] == 'k') || (p_inputstr[i] == 'K')) && (i > 0)) {
                val = val << 10;
                *p_intnum = val;
                res = 1;
            } else if (((p_inputstr[i] == 'm') || (p_inputstr[i] == 'M')) && (i > 0)) {
                val = val << 20;
                *p_intnum = val;
                res = 1;
            } else if (ISVALIDDEC(p_inputstr[i]))
      {
        val = val * 10 + CONVERTDEC(p_inputstr[i]);
      }
      else
      {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
      i++;
    }
  }

  return res;
}


/**
 * @brief Send a string over UART.
 *
 * @param p_string Pointer to the string to send.
 */
void Serial_PutString(uint8_t *p_string)
{
    uint16_t length = 0;

  while (p_string[length] != '\0')
  {
    length++;
  }

  UART_Write(Aolkme_CONSOLE_UART_NUM, p_string, length);
}


/**
 * @brief Send a byte over UART.
 *
 * @param param Pointer to the byte to send.
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef Serial_PutByte( uint8_t param )
{
  UART_Write(Aolkme_CONSOLE_UART_NUM, &param, 1);
  return HAL_OK;
}






































