/**
 * @file Aolkme_core.h
 * @brief 内核Core
 * @author Aolkme
 * @
 * 
 */

#ifndef AOLKME_CORE_H
#define AOLKME_CORE_H

//#pragma once



#ifdef __cplusplus
extern "C" {
#endif

#include "Aolkme_platform.h"


/**
 * @brief User information structure.
 * 
 */
typedef struct 
{
    char appName[32];           /*!< 应用名称*/
    char appId[16];             /*!< ID*/
    char appKey[32];            /*!< 密钥*/
    char appVersion[16];        /*!< 版本号*/
}T_AolkmeUserInfo;


/** 
 * @brief Initialize the Aolkme SDK core in blocking mode.
 * 
 */
T_AolkmeReturnCode Aolkme_Core_Init(T_AolkmeUserInfo *userInfo);



/**
 * @brief 
 * 
 * @return T_AolkmeReturnCode 
 */
T_AolkmeReturnCode Aolkme_Core_Application_Start(void);





/** 
 * @brief DeInitialize the Aolkme SDK core in blocking mode.
 */
T_AolkmeReturnCode Aolkme_Core_DeInit(void);



#ifdef __cplusplus
}
#endif

#endif



