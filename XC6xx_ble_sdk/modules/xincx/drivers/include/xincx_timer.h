/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_TIMER_H__
#define XINCX_TIMER_H__

#include <xincx.h>
#include <hal/xinc_timer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_timer Timer driver
 * @{
 * @ingroup xinc_timer
 * @brief   TIMER peripheral driver.
 */

/**
 * @brief Timer driver instance data structure.
 */
typedef struct
{
    XINC_TIMER_Type * p_reg;            ///< Pointer to the structure with TIMER peripheral instance registers.
    XINC_TIMER_GLOBAL_Type * p_Greg;
    XINC_CPR_CTL_Type * p_cpr;  
    uint8_t          id;      					///< Index of the driver instance. For internal use only.
    uint8_t          instance_idx;      ///< Index of the driver instance. For internal use only.

} xincx_timer_t;
			
/** @brief Macro for creating a timer driver instance. */
#define XINCX_TIMER_INSTANCE(Id)                                            \
{                                                                           \
    .p_reg          =   XINCX_CONCAT_2(XINC_TIMER, Id),                      \
    .p_Greg         =   XINC_TIMER_GLOBAL,                                  \
    .p_cpr          =   XINC_CPR,                                           \
    .id             =   Id,                                                 \
    .instance_idx   =   XINCX_CONCAT_3(XINCX_TIMER, Id, _INST_IDX),          \
}
          
#ifndef __XINCX_DOXYGEN__
enum {
#if XINCX_CHECK(XINCX_TIMER0_ENABLED)
    XINCX_TIMER0_INST_IDX ,
#endif
#if XINCX_CHECK(XINCX_TIMER1_ENABLED)
    XINCX_TIMER1_INST_IDX,
#endif
#if XINCX_CHECK(XINCX_TIMER2_ENABLED)
    XINCX_TIMER2_INST_IDX ,
#endif
#if XINCX_CHECK(XINCX_TIMER3_ENABLED)
    XINCX_TIMER3_INST_IDX ,
#endif
    XINCX_TIMER_ENABLED_COUNT
};
#endif

/** @brief The configuration structure of the timer driver instance. */
typedef struct
{
    xinc_timer_ref_clk_t    ref_clk;          ///< ref clk.
    xinc_timer_mode_t       mode;               ///< Mode of operation.
    xinc_timer_clk_src_t    clk_src;          ///< Bit width.
    uint8_t                 interrupt_priority; ///< Interrupt priority.
    void *                  p_context;          ///< Context passed to interrupt handler.
} xincx_timer_config_t;

/** @brief Timer driver instance default configuration. */
#define XINCX_TIMER_DEFAULT_CONFIG                                                              \
{                                                                                               \
    .ref_clk            =   (xinc_timer_ref_clk_t)XINCX_TIMER_DEFAULT_CONFIG_FREQUENCY,         \
    .mode               =   (xinc_timer_mode_t)XINCX_TIMER_DEFAULT_CONFIG_MODE,                 \
    .clk_src            =   (xinc_timer_clk_src_t)XINCX_TIMER_DEFAULT_CONFIG_CLK_SRC,           \
    .interrupt_priority =   XINCX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,                            \
    .p_context          =   NULL                                                                \
}

/**
 * @brief Timer driver event handler type.
 *
 * @param[in] event_type Timer event.
 * @param[in] p_context  General purpose parameter set during initialization of
 *                       the timer. This parameter can be used to pass
 *                       additional information to the handler function, for
 *                       example, the timer ID.
 */
typedef void (* xincx_timer_event_handler_t)(xinc_timer_int_event_t event_type,uint8_t channel ,
                                            void            * p_context);

/**
 * @brief Function for initializing the timer.
 *
 * @param[in] p_instance          Pointer to the driver instance structure.
 * @param[in] p_config            Pointer to the structure with the initial configuration.
 * @param[in] timer_event_handler Event handler provided by the user.
 *                                Must not be NULL.
 *
 * @retval XINCX_SUCCESS             Initialization was successful.
 * @retval XINCX_ERROR_INVALID_STATE The instance is already initialized.
 */
xincx_err_t xincx_timer_init(xincx_timer_t const * const  p_instance,
                           xincx_timer_config_t const * p_config,
                           xincx_timer_event_handler_t  timer_event_handler);

/**
 * @brief Function for uninitializing the timer.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_timer_uninit(xincx_timer_t const * const p_instance);

/**
 * @brief Function for turning on the timer.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_timer_enable(xincx_timer_t const * const p_instance);

/**
 * @brief Function for turning off the timer.
 *
 * The timer will allow to enter the lowest possible SYSTEM_ON state
 * only after this function is called.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_timer_disable(xincx_timer_t const * const p_instance);

/**
 * @brief Function for checking the timer state.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 *
 * @retval true  Timer is enabled.
 * @retval false Timer is not enabled.
 */
bool xincx_timer_is_enabled(xincx_timer_t const * const p_instance);



/**
 * @brief Function for setting the timer channel in compare mode.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] cc_value   Compare value.
 * @param[in] xinc_timer_mode_t                       mode.
 */
void xincx_timer_compare(xincx_timer_t const * const p_instance,
                        uint32_t                   cc_value,
                        xinc_timer_mode_t                       mode,bool enable);

uint32_t xincx_timer_cnt_get(xincx_timer_t const * const p_instance);

/**
 * @brief Function for converting time in microseconds to timer ticks.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] time_us    Time in microseconds.
 *
 * @return Number of ticks.
 */
__STATIC_INLINE uint32_t xincx_timer_us_to_ticks(xincx_timer_t const * const p_instance,
                                                uint32_t                   time_us);

/**
 * @brief Function for converting time in milliseconds to timer ticks.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] time_ms    Time in milliseconds.
 *
 * @return Number of ticks.
 */
__STATIC_INLINE uint32_t xincx_timer_ms_to_ticks(xincx_timer_t const * const p_instance,
                                                uint32_t                   time_ms);

/**
 * @brief Function for enabling timer compare interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] channel    Compare channel.
 */
void xincx_timer_compare_int_enable(xincx_timer_t const * const p_instance,
                                   uint32_t                   channel);

/**
 * @brief Function for disabling timer compare interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] channel    Compare channel.
 */
void xincx_timer_compare_int_disable(xincx_timer_t const * const p_instance,
                                    uint32_t                   channel);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION


__STATIC_INLINE uint32_t xincx_timer_us_to_ticks(xincx_timer_t const * const p_instance,
                                                uint32_t                   timer_us)
{
    return xinc_timer_us_to_ticks(timer_us, xinc_timer_clk_div_get(p_instance->p_cpr,p_instance->id));
}

__STATIC_INLINE uint32_t xincx_timer_ms_to_ticks(xincx_timer_t const * const p_instance,
                                                uint32_t                   timer_ms)
{
    return xinc_timer_ms_to_ticks(timer_ms,xinc_timer_clk_div_get(p_instance->p_cpr,p_instance->id));
}

#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */


void xincx_timer_0_irq_handler(void);
void xincx_timer_1_irq_handler(void);
void xincx_timer_2_irq_handler(void);
void xincx_timer_3_irq_handler(void);


#ifdef __cplusplus
}
#endif //__XINCX_DOXYGEN_

#endif // XINCX_TIMER_H__

