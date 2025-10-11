#include "BootService.h"
#include "Aolkme_OSAL.h"

//#include "FreeRTOS.h"
//#include "task.h"
#include "main.h"
//#include "cmsis_os.h"

#include "Button.h"
#include "uart.h"
#include "BIOS_menu.h"

typedef void (*pFunction)(void);

pFunction JumpToApplication;
uint32_t JumpAddress;




T_AolkmeTaskHandle BootServiceTaskHandle = NULL;

void *BootService_Task(void *arg);

T_AolkmeReturnCode BootServiceStart(void)
{
	
	UART_Init(Aolkme_CONSOLE_UART_NUM, Aolkme_CONSOLE_UART_BAUD);
	
	A_Osal_TaskCreate("BootServiceTask", BootService_Task, 512, NULL, &BootServiceTaskHandle);
	
	return 0;
}






#define APPLICATION_ADDRESS		((uint32_t)0x08010000)





void *BootService_Task(void *arg)
{
    A_Osal_TaskSleepMs(500);

	
	Show_BIOS_Menu();
	
	
	
    if (Check_BIOS() == 1)
    {
        Show_BIOS_Menu();
    }

    //JumpToApp();

	CCMRAM static uint8_t App;
	
	for (;;)
	{
		// HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_10);
		A_Osal_TaskSleepMs(100);
	}
	

}




/**
 * @brief  Jump to user application
 * @param  None
 */
void JumpToApp(void)
{
    __disable_irq();
    __disable_fiq();

    /* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
    if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
    { 
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);

        /* Jump to user application */
        JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
        JumpToApplication = (pFunction) JumpAddress;
        
        JumpToApplication();
    }else
    {
        Error_Handler();
    }
}














