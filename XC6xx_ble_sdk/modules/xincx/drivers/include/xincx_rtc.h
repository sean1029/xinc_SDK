/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_RTC_H__
#define XINCX_RTC_H__


#include <xincx_config.h>
#include <drivers/xincx_common.h>
#include <xincx_glue.h>
#include <drivers/xincx_errors.h>
#include <hal/xinc_rtc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_rtc RTC driver
 * @{
 * @ingroup xinc_rtc
 * @brief   Real Timer Counter (RTC) peripheral driver.
 */

/** @brief Macro for converting microseconds into ticks. */
#define XINCX_RTC_US_TO_TICKS(us,freq) (((us) * (freq)) / 1000000U)

/** @brief RTC driver interrupt types. */
typedef enum
{
    XINCX_RTC_INT_TIME1 = 0, /**< Interrupt from TIME1 event. */
    XINCX_RTC_INT_TIME2 = 1, /**< Interrupt from TIME2 event. */
    XINCX_RTC_INT_TIME3 = 2, /**< Interrupt from TIME3 event. */
    XINCX_RTC_INT_SEC = 3, /**< Interrupt from SEC event. */
    XINCX_RTC_INT_MIN     = 4, /**< Interrupt from MIN event. */
    XINCX_RTC_INT_HOUR = 5,  /**< Interrupt from HOUR event. */
    XINCX_RTC_INT_DAY = 6,  /**< Interrupt from DAY event. */
    XINCX_RTC_INT_AOTIME = 7
} xincx_rtc_int_type_t;

/** @brief RTC driver interrupt types. */
enum
{
    XINCX_RTC_WEEK_MATCH_SUNDAY = (0x01 << 0), /**< Interrupt from TIME1 event. */
    XINCX_RTC_WEEK_MATCH_MONDAY = (0x01 << 1), /**< Interrupt from TIME2 event. */
    XINCX_RTC_WEEK_MATCH_TUESDAY = (0x01 << 2), /**< Interrupt from TIME3 event. */
    XINCX_RTC_WEEK_MATCH_WEDNESDAY = (0x01 << 3), /**< Interrupt from SEC event. */
    XINCX_RTC_WEEK_MATCH_THURSDAY     = (0x01 << 4), /**< Interrupt from MIN event. */
    XINCX_RTC_WEEK_MATCH_FRIDAY = (0x01 << 5),  /**< Interrupt from HOUR event. */
    XINCX_RTC_WEEK_MATCH_SATURDAY = (0x01 << 6)  /**< Interrupt from DAY event. */
};

/** @brief RTC driver interrupt types. */
typedef enum
{
    XINCX_RTC_MATCH_TIME_1 = (0x01 << 5), /**< MATCH_TIME_1 ch. */
    XINCX_RTC_MATCH_TIME_2 = (0x01 << 4), /**< MATCH_TIME_2 ch */
    XINCX_RTC_MATCH_TIME_3 = (0x01 << 6), /**< MATCH_TIME_3 ch */
}xincx_rtc_match_timer_ch_t;

/** @brief RTC driver instance structure. */
typedef struct
{
    XINC_RTC_Type  * p_reg;            /**< Pointer to instance register set. */
    XINC_CPR_CTL_Type * p_cpr; 		///< Pointer to a structure with CPR registers.
    XINC_CPR_AO_CTL_Type *p_cprAO; 		///< Pointer to a structure with CPR registers.    
    uint8_t         instance_id;      /**< Index of the driver instance. For internal use only. */
    uint8_t         id; /**< Number of capture/compare channels. */
} xincx_rtc_t;



/** @brief Macro for creating an RTC driver instance. */
#define XINCX_RTC_INSTANCE(Id)                                  \
{                                                               \
    .p_reg          = XINCX_CONCAT_2(XINC_RTC, Id),              \
    .p_cpr          = XINC_CPR,                                 \
    .p_cprAO        = XINC_CPR_AO,                              \
    .instance_id    = XINCX_CONCAT_3(XINCX_RTC, Id, _INST_IDX),  \
    .id             = Id,                                       \
}

#ifndef __XINCX_DOXYGEN__
enum {
    #if XINCX_CHECK(XINCX_RTC0_ENABLED)
    XINCX_RTC0_INST_IDX,
    #endif
    XINCX_RTC_ENABLED_COUNT
};
#endif


/** @brief RTC driver instance configuration structure. */
typedef struct
{
    xinc_rtc_time_t date;
    uint8_t   hour_limit;            /**< hour limit. */
    uint8_t   min_limit;             /**< minute limit. */
    uint8_t   sec_limit;             /**< second limit. */
    uint16_t  freq;                  /**< freq. */
    uint8_t   interrupt_priority;    /**< Interrupt priority. */
    xinc_rtc_type_t   type;           /**< Rtc type. */
    bool      reliable;              /**< Reliable mode flag. */
	
} xincx_rtc_config_t;

/** @brief RTC driver instance configuration structure. */
typedef union xincx_rtc_match_config
{
    uint32_t    value;
    struct time
    {
        uint32_t  sec:6;                    /**< second. */
        uint32_t  hour:6;                  /**< hour. */
        uint32_t  min:5;                   /**< minute. */
        uint32_t  week:7;                  /**< week. */
    }times;

} xincx_rtc_match_config_t;


/** @brief RTC instance default configuration. */
#define XINCX_RTC_DEFAULT_CONFIG                                \
{                                                               \
    .date.day           = 0,                                    \
    .date.hour          = 0,                                    \
    .date.min           = 0,                                    \
    .date.sec           = 0,                                    \
    .date.week          = 0,                                    \
    .hour_limit         = 24,                                   \
    .min_limit          = 60,                                   \
    .sec_limit          = 60,                                   \
    .freq               = 32716,                                \
    .interrupt_priority = 0,                                    \
    .reliable           = 0,                                    \
}


/** @brief RTC driver instance handler type. */
typedef void (*xincx_rtc_handler_t)(xincx_rtc_int_type_t int_type);


/**
 * @brief Function for initializing the RTC driver instance.
 *
 * After initialization, the instance is in power off state.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] p_config   Pointer to the structure with the initial configuration.
 * @param[in] handler    Event handler provided by the user.
 *                       Must not be NULL.
 *
 * @retval XINCX_SUCCESS             Successfully initialized.
 * @retval XINCX_ERROR_INVALID_STATE The instance is already initialized.
 */
xincx_err_t xincx_rtc_init(xincx_rtc_t const * const  p_instance,
                         xincx_rtc_config_t const * p_config,
                         xincx_rtc_handler_t        handler);

/**
 * @brief Function for uninitializing the RTC driver instance.
 *
 * After uninitialization, the instance is in idle state. The hardware should return to the state
 * before initialization.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_uninit(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for enabling the RTC driver instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_enable(xincx_rtc_t const * const p_instance);	

/**
 * @brief Function for disabling the RTC driver instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_disable(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for setting a compare channel.
 *
 * The function powers on the instance if the instance was in power off state.
 *
 * The driver is not entering a critical section when configuring RTC, which means that it can be
 * preempted for a certain amount of time. When the driver was preempted and the value to be set
 * is short in time, there is a risk that the driver sets a compare value that is
 * behind. In this case, if the reliable mode is enabled for the specified instance,
 * the risk is handled.
 * However, to detect if the requested value is behind, this mode makes the following assumptions:
 *  -  The maximum preemption time in ticks (8-bit value) is known and is less than 7.7 ms
 *   (for prescaler = 0, RTC frequency 32 kHz).
 *  -  The requested absolute compare value is not bigger than (0x00FFFFFF)-tick_latency. It is
 *   the user's responsibility to ensure this.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] channel    One of the channels of the instance.
 * @param[in] val        Absolute value to be set in the compare register.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 *
 * @retval XINCX_SUCCESS       The procedure is successful.
 * @retval XINCX_ERROR_TIMEOUT The compare is not set because the request value is behind the
 *                            current counter value. This error can only be reported
 *                            if the reliable mode is enabled.
 */
xincx_err_t xincx_rtc_time_set(xincx_rtc_t const * const p_instance,
                           xincx_rtc_match_timer_ch_t                 channel,
                           xincx_rtc_match_config_t  config ,
                           bool                     enable_irq);

/**
 * @brief Function for disabling a channel.
 *
 * This function disables channel  events and channel interrupts.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] channel    One of the channels of the instance.
 *
 * @retval XINCX_SUCCESS       The procedure is successful.
 * @retval XINCX_ERROR_TIMEOUT Interrupt is pending on the requested channel.
 */
xincx_err_t xincx_rtc_time_disable(xincx_rtc_t const * const p_instance, xincx_rtc_match_timer_ch_t channel);
						 
/**
 * @brief Function for enabling the sec event.
 *
 * This function enables the sec event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_sec_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the sec event.
 *
 * This function disables the sec event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_sec_int_disable(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for enabling the min event.
 *
 * This function enables the min event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_min_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the min event.
 *
 * This function disables the min event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_min_int_disable(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for enabling the hour event.
 *
 * This function enables the hour event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_hour_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the hour event.
 *
 * This function disables the hour event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_hour_int_disable(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for enabling the day event.
 *
 * This function enables the day event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_day_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the day event.
 *
 * This function disables the day event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_day_int_disable(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for enabling the Match Time 1 event.
 *
 * This function enables the Match Time 1 event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_time1_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the Match Time 1 event.
 *
 * This function disables the Match Time 1 event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_time1_int_disable(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for enabling the Match Time 2 event.
 *
 * This function enables the Match Time 2 event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_time2_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the Match Time 2 event.
 *
 * This function disables the Match Time 2 event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_time2_int_disable(xincx_rtc_t const * const p_instance);


/**
 * @brief Function for enabling the Match Time 3 event.
 *
 * This function enables the Match Time 3 event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_time3_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the Match Time 3 event.
 *
 * This function disables the Match Time 3 event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_time3_int_disable(xincx_rtc_t const * const p_instance);


void xincx_rtc_date_set(xincx_rtc_t const * const p_instance,                         
                           xinc_rtc_time_t  date);

void xincx_rtc_date_get(xincx_rtc_t const * const p_instance,                         
                           xinc_rtc_time_t  *date);



void xincx_rtc_AOtime_set(xincx_rtc_t const * const p_instance,                         
                           uint32_t  tick);
#endif // XINCX_H__
