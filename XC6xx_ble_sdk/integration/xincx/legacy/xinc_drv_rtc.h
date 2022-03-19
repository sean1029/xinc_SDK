/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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