/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_DRV_RTC_H__
#define XINC_DRV_RTC_H__

#include <xincx_rtc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_drv_rtc RTC driver - legacy layer
 * @{
 * @ingroup  xinc_rtc
 *
 * @brief    Layer providing compatibility with the former API.
 */

/** @brief Type definition for forwarding the new implementation. */
typedef xincx_rtc_t          xinc_drv_rtc_t;
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_rtc_config_t   xinc_drv_rtc_config_t;

/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_RTC_INSTANCE            XINCX_RTC_INSTANCE
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_RTC_DEFAULT_CONFIG      XINCX_RTC_DEFAULT_CONFIG
/** @brief Macro for forwarding the new implementation. */
#define RTC_US_TO_TICKS                 XINCX_RTC_US_TO_TICKS

/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_RTC_INT_COMPARE1        XINCX_RTC_INT_TIME1
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_RTC_INT_COMPARE2        XINCX_RTC_INT_TIME2
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_RTC_INT_COMPARE3        XINCX_RTC_INT_TIME3
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_RTC_INT_SEC             XINCX_RTC_INT_SEC
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_RTC_INT_MIN       		  XINCX_RTC_INT_MIN
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_RTC_INT_HOUR        		XINCX_RTC_INT_HOUR
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_RTC_INT_DAY        			XINCX_RTC_INT_DAT
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_int_type_t          xincx_rtc_int_type_t
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_handler_t           xincx_rtc_handler_t

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_init                xincx_rtc_init
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_uninit              xincx_rtc_uninit
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_enable              xincx_rtc_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_disable             xincx_rtc_disable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_sec_int_enable      xincx_rtc_sec_int_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_sec_int_disable     xincx_rtc_sec_int_disable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_min_int_enable      xincx_rtc_min_int_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_min_int_disable     xincx_rtc_min_int_disable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_hour_int_enable     xincx_rtc_hour_int_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_hour_int_disable    xincx_rtc_hour_int_disable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_day_int_enable     xincx_rtc_day_int_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_day_int_disable    xincx_rtc_day_int_disable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_hour_int_enable     xincx_rtc_hour_int_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_hour_int_disable    xincx_rtc_hour_int_disable

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_time1_int_enable     xincx_rtc_time1_int_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_time1_int_disable    xincx_rtc_time1_int_disable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_time2_int_enable     xincx_rtc_time2_int_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_time2_int_disable    xincx_rtc_time2_int_disable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_time3_int_enable     xincx_rtc_time3_int_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_time3_int_disable    xincx_rtc_time3_int_disable

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_time_set     					xincx_rtc_time_set
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_time_disable    			xincx_rtc_time_disable


/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_AOtime_set     				xincx_rtc_AOtime_set

#define xinc_drv_rtc_date_get     					xincx_rtc_date_get

#define xinc_drv_rtc_date_set     					xincx_rtc_date_set






/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_rtc_int_enable           xincx_rtc_int_enable



/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_DRV_RTC_H__
