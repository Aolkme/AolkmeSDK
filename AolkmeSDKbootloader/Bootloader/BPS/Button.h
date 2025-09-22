#ifndef _BUTTON_H
#define _BUTTON_H

#include "main.h"
#include "Aolkme_platform.h"



#ifdef __cplusplus
extern "C" {
#endif


#define BIOS_BUTTON_PORT        GPIOA
#define BIOS_BUTTON_PIN         GPIO_PIN_0


uint8_t Check_BIOS(void);


#ifdef __cplusplus
}
#endif



#endif // _BUTTON_H



