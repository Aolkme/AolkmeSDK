/**
 * @file Aolkme_typedef.h
 * @brief 内核
 * @author Aolkme
 * @
 * 
 */

// 头部引用格式




#ifndef AOLKME_TYPEDEF_H
#define AOLKME_TYPEDEF_H

//#pragma once


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Aolkme_error.h"
//#include "Aolkme_OSAL.h"



#ifdef __cplusplus
extern "C" {
#endif



#define AOLKME_Pi       (3.14159265358979323846f)













/**
 * @brief Type define double as olkme_f64_t.
 */
typedef double olkme_f64_t;

/**
 * @brief Type define float as olkme_f32_t.
 */
typedef float olkme_f32_t;

/**
 * @brief Type define uint32 as T_AolkmeReturnCode.
 * @details The type can be any value of ::AolkmeErrorCode.
 */
typedef uint32_t T_AolkmeReturnCode;












#ifdef __cplusplus
}
#endif


#endif // AOLKME_TYPEDEF_H


