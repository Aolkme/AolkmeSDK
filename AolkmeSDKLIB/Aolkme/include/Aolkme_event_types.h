/**
 * @file Aolkme_event_types.h
 * @brief AolkmeSDK 标准事件类型定义
 */

#ifndef AOLKME_EVENT_TYPES_H
#define AOLKME_EVENT_TYPES_H

#include "Aolkme_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 系统级事件ID
 */
typedef enum {
    AOLKME_EVENT_SYSTEM_STARTUP        = 0x00010000, ///< 系统启动完成
    AOLKME_EVENT_SYSTEM_SHUTDOWN       = 0x00010001, ///< 系统关闭
    AOLKME_EVENT_SYSTEM_ERROR          = 0x00010002, ///< 系统错误
    AOLKME_EVENT_SYSTEM_WARNING        = 0x00010003, ///< 系统警告
    AOLKME_EVENT_SYSTEM_HEARTBEAT      = 0x00010004, ///< 系统心跳
    AOLKME_EVENT_SYSTEM_RESOURCE_LOW   = 0x00010005, ///< 系统资源不足
    AOLKME_EVENT_SYSTEM_MONITOR_REPORT = 0x00010006, ///< 系统监控报告
} E_AolkmeSystemEventId;

/**
 * @brief 网络事件ID
 */
typedef enum {
    AOLKME_EVENT_NETWORK_CONNECTED     = 0x00020000, ///< 网络连接成功
    AOLKME_EVENT_NETWORK_DISCONNECTED  = 0x00020001, ///< 网络断开
    AOLKME_EVENT_NETWORK_DATA_RECEIVED = 0x00020002, ///< 网络数据接收
} E_AolkmeNetworkEventId;

/**
 * @brief 传感器事件ID
 */
typedef enum {
    AOLKME_EVENT_SENSOR_DATA_READY     = 0x00030000, ///< 传感器数据就绪
    AOLKME_EVENT_SENSOR_ERROR          = 0x00030001, ///< 传感器错误
    AOLKME_EVENT_SENSOR_CALIBRATION    = 0x00030002, ///< 传感器校准
} E_AolkmeSensorEventId;

/**
 * @brief 电源管理事件ID
 */
typedef enum {
    AOLKME_EVENT_POWER_BATTERY_LOW     = 0x00040000, ///< 电池电量低
    AOLKME_EVENT_POWER_BATTERY_CRITICAL= 0x00040001, ///< 电池电量严重不足
    AOLKME_EVENT_POWER_CHARGING        = 0x00040002, ///< 充电中
    AOLKME_EVENT_POWER_CHARGED         = 0x00040003, ///< 充电完成
} E_AolkmePowerEventId;







typedef enum {
    EVENT_FLAG_NONE          = 0x00000000, ///< 无标志
    EVENT_FLAG_DYNAMIC_DATA  = 0x00000001, ///< 动态数据标志
    EVENT_FLAG_STATIC_DATA   = 0x00000002, ///< 静态数据标志
} E_AolkmeEventFlags;






/**
 * @brief 用户自定义事件起始ID
 */
#define AOLKME_EVENT_USER_BASE 0x80000000

#ifdef __cplusplus
}
#endif

#endif // AOLKME_EVENT_TYPES_H




