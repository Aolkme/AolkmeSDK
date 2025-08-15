/**
 * @file Aolkme_platform.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-08-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "Aolkme_platform.h"

static T_AolkmeHalUartHandler *g_halUartHandler = NULL;
static T_AolkmeHalI2cHandler *g_halI2cHandler = NULL;
static T_AolkmeOSALHandler *g_osalHandler = NULL;



/**
 * @brief Register the HAL UART handler.
 * 
 * @param halUartHandler Pointer to the HAL UART handler structure.
 * @return T_AolkmeReturnCode Returns success or error code.
 */
T_AolkmeReturnCode AolkmePlatform_RegHalUartHandler(const T_AolkmeHalUartHandler *halUartHandler)
{
    if (halUartHandler == NULL)
    {
        printf("HAL UART handler is not registered.\r\n");
        return AOLKME_ERROR_PLATFORM_MODULE_CODE_INVALID_PARAMETER;
    }

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * @brief Register the HAL I2C handler.
 * 
 * @param halI2cHandle Pointer to the HAL I2C handler structure.
 * @return T_AolkmeReturnCode Returns success or error code.
 */
T_AolkmeReturnCode AolkmePlatform_RegHalI2cHandler(const T_AolkmeHalI2cHandler *halI2cHandle)
{
    if (halI2cHandle == NULL)
    {
        printf("HAL I2C handler is not registered.\r\n");
        return AOLKME_ERROR_PLATFORM_MODULE_CODE_INVALID_PARAMETER;
    }

    // Register the I2C handler logic here

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * @brief Register the OSAL handler.
 * @param osalHandle Pointer to the OSAL handler structure.
 * @return T_AolkmeReturnCode Returns success or error code.
 */
T_AolkmeReturnCode AolkmePlatform_RegOSALHandle(const T_AolkmeOSALHandler *osalHandle)
{
    if (osalHandle == NULL)
    {
        printf("OSAL handler is not registered.\r\n");
        return AOLKME_ERROR_PLATFORM_MODULE_CODE_INVALID_PARAMETER;
    }
    g_osalHandler = (T_AolkmeOSALHandler *)osalHandle;

    return AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}






/**
 * @brief Get the OSAL handler.
 * 
 * @return T_AolkmeOSALHandler* Returns the OSAL handler or error code.
 */
T_AolkmeOSALHandler *AolkmePlatform_GetOSALHandle(void)
{
    if (g_osalHandler == NULL)
    {
        printf("OSAL handler is not registered.\r\n");
        return NULL;
    }
    return g_osalHandler;
}


















