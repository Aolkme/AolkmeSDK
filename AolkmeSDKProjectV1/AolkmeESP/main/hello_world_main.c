/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"


#include "Aolkme_OSAL.h"
#include "Aolkme_Platform.h"
#include "Aolkme_core.h"
#include "Aolkme_typedef.h"









void app_main(void)
{
    
    T_AolkmeReturnCode returnCode;
	T_AolkmeUserInfo userInfo;
	
	
	
    T_AolkmeOSALHandler osalHandler = {
        .TaskCreate = A_Osal_TaskCreate,
        .TaskDestroy = A_Osal_TaskDestroy,
        .TaskSleepMs = A_Osal_TaskSleepMs,
        .MutexCreate = A_Osal_MutexCreate,
        .MutexDestroy = A_Osal_MutexDestroy,
        .MutexLock = A_Osal_MutexLock,
        .MutexUnlock = A_Osal_MutexUnlock,
        .SemaCreate = A_Osal_SemaphoreCreate,
        .BinarySemaphoreCreate = A_Osal_BinarySemaphoreCreate,
        .SemaDestroy = A_Osal_SemaphoreDestroy,
        .SemaWait = A_Osal_SemaphoreWait,
		.SemaTimedWait = A_Osal_SemaphoreTimedWait,
        .SemaPost = A_Osal_SemaphorePost,
        .GetTimeMs = A_Osal_GetTimeMs,
        .GetRandomNum = A_Osal_GetRandomNum,
        .Malloc = Osal_Malloc,
        .Free = Osal_Free,
        .QueueCreate = A_Osal_QueueCreate,
        .QueueDestroy = A_Osal_QueueDestroy,
        .QueueSend = A_Osal_QueueSend,
        .QueueReceive = A_Osal_QueueReceive,
        .QueueMessageCount = A_Osal_QueueMessageCount,
        .QueueReset = A_Osal_QueueReset
    };


	
    // Register OSAL handler
    returnCode = AolkmePlatform_RegOSALHandle(&osalHandler);
	if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
	{
		printf("register osal handler error\r\n");
	}
	printf("AolkmeOSAL init is OK!\r\n");
	
//     returnCode = Aolkme_FillInUserInfo(&userInfo);
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        printf("Aolkme_FillInUserInfo is error\r\n");
    }
    printf("Aolkme_FillInUserInfo is OK!\r\n");

    returnCode = Aolkme_Core_Init(&userInfo);
    if (returnCode != AOLKME_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        printf("Aolkme_Core_Init is error\r\n");
    }
    printf("Aolkme_Core_Init is OK!\r\n");



    

    while (1)
    {
        printf("Hello world!\n");
        osalHandler.TaskSleepMs(1000);
    }
    

    /* Print chip information */
    /*
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
    */
}









