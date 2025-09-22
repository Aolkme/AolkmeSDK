#include "Button.h"
#include "Aolkme_OSAL.h"





uint8_t Check_BIOS_Button(void)
{
    if(HAL_GPIO_ReadPin(BIOS_BUTTON_PORT, BIOS_BUTTON_PIN) == GPIO_PIN_RESET)
    {
        return 1;
    }
    return 0;
}



uint8_t Check_BIOS(void)
{
    int i = 0;
    for(i = 0; i < 10; i++)
    {
        if(Check_BIOS_Button() == 1)
        {
            return 1;
        }
        A_Osal_TaskSleepMs(100);
    }
    return 0;
}


