/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_PWM_H__
#define XINCX_PWM_H__

#include <xincx.h>
#include <hal/xinc_pwm.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_pwm PWM driver
 * @{
 * @ingroup xinc_pwm
 * @brief   Pulse Width Modulation (PWM) peripheral driver.
 */

/** @brief PWM driver instance data structure. */
typedef struct
{
    XINC_PWM_Type       *p_reg;  ///< Pointer to the structure with PWM peripheral instance registers.
    XINC_CPR_CTL_Type   *p_cpr; 
    uint8_t             id;
    uint8_t             drv_inst_idx; ///< Index of the driver instance. For internal use only.
    uint8_t             output_pin; ///< Pin numbers for individual output ./**< Use @ref XINCX_PWM_PIN_NOT_USED */
    uint8_t             output_inv_pin;					///< Pin numbers for individual output(optional )
} xincx_pwm_t;


/** @brief Macro for creating a PWM driver instance. */
#define XINCX_PWM_INSTANCE(Id)                               \
{                                                           \
    .p_reg  = XINCX_CONCAT_2(XINC_PWM, Id),             \
    .p_cpr            = XINC_CPR,											\
    .id 		= Id,																				\
    .drv_inst_idx = XINCX_CONCAT_3(XINCX_PWM, Id, _INST_IDX), \
    .output_pin  = XINCX_CONCAT_3(XINCX_PWM_DEFAULT_CONFIG_OUT,Id,_PIN),                   \
    .output_inv_pin  = XINCX_CONCAT_3(XINCX_PWM_DEFAULT_CONFIG_OUT,Id,_INV_PIN),                   \
}

#ifndef __XINCX_DOXYGEN__
enum {
#if XINCX_CHECK(XINCX_PWM0_ENABLED)
    XINCX_PWM0_INST_IDX,
#endif
#if XINCX_CHECK(XINCX_PWM1_ENABLED)
    XINCX_PWM1_INST_IDX,
#endif
#if XINCX_CHECK(XINCX_PWM2_ENABLED)
    XINCX_PWM2_INST_IDX,
#endif
#if XINCX_CHECK(XINCX_PWM3_ENABLED)
    XINCX_PWM3_INST_IDX,
#endif
#if XINCX_CHECK(XINCX_PWM4_ENABLED)
    XINCX_PWM4_INST_IDX,
#endif
#if XINCX_CHECK(XINCX_PWM5_ENABLED)
    XINCX_PWM5_INST_IDX,
#endif
    XINCX_PWM_ENABLED_COUNT
};
#endif

/**
 * @brief This value can be provided instead of a pin number for any channel
 *        to specify that its output is not used and therefore does not need
 *        to be connected to a pin.
 */
#define XINCX_PWM_PIN_NOT_USED    0xFF

/** @brief This value can be added to a pin number to invert its polarity (set idle state = 1). */
#define XINCX_PWM_PIN_INVERTED    0x80

/** @brief PWM driver configuration structure. */
typedef struct
{
																		
	xinc_pwm_clk_src_t clk_src;          ///< Bit width.
	xinc_pwm_ref_clk_t  ref_clk;   ///< Base clock frequency.

	uint32_t            frequency;
	uint16_t           	duty_cycle;
	uint8_t           	inv_delay;  
    xinc_pwm_acc_mode_t mode;
	bool           			start;
	bool           			inv_enable;
		
} xincx_pwm_config_t;

/** @brief PWM driver default configuration. */
#define XINCX_PWM_DEFAULT_CONFIG                                             \
{                                                                           \
	.ref_clk   = (xinc_pwm_ref_clk_t)XINCX_PWM_DEFAULT_CONFIG_REF_CLOCK,      \
	.clk_src   = 		(xinc_pwm_clk_src_t)XINCX_PWM_DEFAULT_CONFIG_CLK_SRC,      \
	.frequency       =  1000,                                                     \
	.duty_cycle   =  50,   	                                                 \
}



/** @brief PWM driver event handler type. */
typedef void (* xincx_pwm_handler_t)();

/**
 * @brief Function for initializing the PWM driver.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] p_config   Pointer to the structure with the initial configuration.
 * @param[in] handler    Event handler provided by the user. If NULL is passed
 *                       instead, event notifications are not done and PWM
 *                       interrupts are disabled.
 *
 * @retval XINCX_SUCCESS             Initialization was successful.
 * @retval XINCX_ERROR_INVALID_STATE The driver was already initialized.
 */
xincx_err_t xincx_pwm_init(xincx_pwm_t const * const  p_instance,
                         xincx_pwm_config_t const * p_config,
                         xincx_pwm_handler_t        handler);

/**
 * @brief Function for uninitializing the PWM driver.
 *
 * If any sequence playback is in progress, it is stopped immediately.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_pwm_uninit(xincx_pwm_t const * const p_instance);


bool xincx_pwm_start(xincx_pwm_t const * const p_instance);
bool xincx_pwm_stop(xincx_pwm_t const * const p_instance);


xincx_err_t xincx_pwm_freq_duty_cycl_update(xincx_pwm_t const * const p_instance,uint32_t new_freq,uint16_t new_duty);

xincx_err_t xincx_pwm_duty_cycle_update(xincx_pwm_t const * const p_instance,uint16_t new_duty);

xincx_err_t xincx_pwm_freq_update(xincx_pwm_t const * const p_instance,uint32_t new_freq);

bool xincx_pwm_freq_valid_range_check(uint8_t clk_src,uint8_t ref_clk, uint32_t set_freq);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION



#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */



#ifdef __cplusplus
}
#endif

#endif // XINCX_PWM_H__
