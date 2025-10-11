/**
 * @file BootService.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-09-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#ifndef BOOT_SERVICE_H
#define BOOT_SERVICE_H


#include "Aolkme_platform.h"



#ifdef __cplusplus
extern "C" {
#endif


T_AolkmeReturnCode BootServiceStart(void);


void JumpToApp(void);



#ifdef __cplusplus
}
#endif



#endif // BOOT_SERVICE_H







