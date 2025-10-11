#ifndef _BIOS_MENU_H
#define _BIOS_MENU_H

#include "main.h"
#include "Aolkme_platform.h"



#ifdef __cplusplus
extern "C" {
#endif



uint8_t Menu_Cmd[5] = {0x31, 0x32, 0x33, 0x34, 0x35};


// 全局标志
// volatile uint8_t g_key_received = 0;




uint8_t KeyDetector_ProcessByte(uint8_t data);



uint8_t SerialDownloadOther(void);
void ON_SerialDownloadOther(void);
void OFF_SerialDownloadOther(void);


void Show_BIOS_Menu(void);



















#ifdef __cplusplus
}
#endif



#endif // _BIOS_MENU_H



