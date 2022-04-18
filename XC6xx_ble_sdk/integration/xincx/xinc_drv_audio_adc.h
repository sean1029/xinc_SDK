/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_DRV_AUDIO_ADC_H__
#define XINC_DRV_AUDIO_ADC_H__

#include <xincx.h>
#ifdef AUDIO_ADC_PRESENT
    #include <xincx_audio_adc.h>
    #include "xinc_gpio.h"
#else
    // Compilers (at least the smart ones) will remove the I2CM related code
    // (blocks starting with "if (XINC_DRV_I2C_USE_I2CM)") when it is not used,
    // but to perform the compilation they need the following definitions.
    #define xincx_audio_adc_init(...)                 0
    #define xincx_audio_adc_uninit(...)
    #define xincx_audio_adc_enable(...)
    #define xincx_audio_adcm_disable(...)
#endif



#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_drv_audio_adc AUDIO_ADC driver 
 * @{
 * @ingroup  xinc_audio_adc
 * @brief    Layer providing compatibility with the former API.
 */

/**
 * @brief Structure for the audio adc driver instance.
 */
typedef struct
{
    uint8_t inst_idx;
    #ifdef AUDIO_ADC_PRESENT
    xincx_audio_adc_t audio_adc;
    #endif
    bool    use_easy_dma;
} xinc_drv_audio_adc_t;

/** @brief Type definition for forwarding the new implementation. */
//typedef xincx_audio_adc_t        xinc_drv_audio_adc_t;
///** @brief Type definition for forwarding the new implementation. */
typedef xincx_audio_adc_config_t    xinc_drv_audio_adc_config_t;
/**
 * @brief Audio ADC clock frequency.
 */
typedef xinc_audio_adc_freq_t xinc_drv_audio_adc_frequency_t;
     
/**
 * @brief Structure for the AUDIO ADC driver instance configuration.
 */
typedef xincx_audio_adc_config_t xinc_drv_audio_adc_config_t;


/**
 * @brief Macro for creating a audio adc driver instance.
 */
#define XINC_DRV_AUDIO_ADC_INSTANCE(id)    XINC_DRV_AUDIO_ADC_INSTANCE_(id)
#define XINC_DRV_AUDIO_ADC_INSTANCE_(id)   XINC_DRV_AUDIO_ADC_INSTANCE_ ## id
#if XINCX_CHECK(XINCX_AUDIO_ADC0_ENABLED)
    #define XINC_DRV_AUDIO_ADC_INSTANCE_0 \
     { 0, XINCX_AUDIO_ADC_INSTANCE(0), false }
#endif


/**
 * @brief AUDIO_ADC driver instance default configuration.
 */
#define XINC_DRV_AUDIO_ADC_DEFAULT_CONFIG                                               \
{                                                                                \
    .frequency          = (xinc_drv_i2c_frequency_t)I2C_DEFAULT_CONFIG_FREQUENCY, \
    .mic_p              = XINC_GPIO_18,                                                    \
    .mic_n              = XINC_GPIO_0,                                                    \
    .mic_bias           = XINC_GPIO_19,                                             \
    .interrupt_priority = I2C_DEFAULT_CONFIG_IRQ_PRIORITY,                       \
}


/**
 * @brief Structure for a AUDIO ADC event.
 */

typedef xincx_audio_adc_evt_t xinc_drv_audio_adc_evt_t;

/**
 * @brief AUDIO ADC event handler prototype.
 */

typedef xincx_audio_adc_evt_handler_t xinc_drv_audio_adc_evt_handler_t  ;
/**
 * @brief Function for initializing the AUDIO ADC driver instance.
 *
 * @param[in] p_instance      Pointer to the driver instance structure.
 * @param[in] p_config        Initial configuration.
 * @param[in] event_handler   Event handler provided by the user. If NULL, blocking mode is enabled.
 * @param[in] p_context       Context passed to event handler.
 *
 * @retval XINC_SUCCESS             If initialization was successful.
 * @retval XINC_ERROR_INVALID_STATE If the driver is in invalid state.
 * @retval XINC_ERROR_BUSY          If some other peripheral with the same
 *                                 instance ID is already in use. This is
 *                                 possible only if PERIPHERAL_RESOURCE_SHARING_ENABLED
 *                                 is set to a value other than zero.
 */
ret_code_t xinc_drv_audio_adc_init(xinc_drv_audio_adc_t const *        p_instance,
                            xinc_drv_audio_adc_config_t const * p_config,
                            xinc_drv_audio_adc_evt_handler_t    event_handler,
                            void *                       p_context);

/**
 * @brief Function for uninitializing the AUDIO ADC instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
__STATIC_INLINE
void xinc_drv_audio_adc_uninit(xinc_drv_audio_adc_t const * p_instance);

/**
 * @brief Function for enabling the AUDIO ADC instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
__STATIC_INLINE
void xinc_drv_audio_adc_enable(xinc_drv_audio_adc_t const * p_instance);

/**
 * @brief Function for disabling the audio_adc instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
__STATIC_INLINE
void xinc_drv_audio_adc_disable(xinc_drv_audio_adc_t const * p_instance);






/**
 * @brief Function for checking the audio_adc driver state.
 *
 * @param[in] p_instance AUDIO ADC instance.
 *
 * @retval true  If the AUDIO ADC driver is currently busy performing a converted.
 * @retval false If the AUDIO ADC driver is ready for a new converted.
 */
__STATIC_INLINE
bool xinc_drv_audio_adc_is_busy(xinc_drv_audio_adc_t const * p_instance);




#ifndef SUPPRESS_INLINE_IMPLEMENTATION





__STATIC_INLINE
void xinc_drv_audio_adc_uninit(xinc_drv_audio_adc_t const * p_instance)
{
   xincx_audio_adc_uninit(&p_instance->audio_adc);
}

__STATIC_INLINE
void xinc_drv_audio_adc_enable(xinc_drv_audio_adc_t const * p_instance)
{
    xincx_audio_adc_enable(&p_instance->audio_adc);
}

__STATIC_INLINE
void xinc_drv_audio_adc_disable(xinc_drv_audio_adc_t const * p_instance)
{
    xincx_audio_adc_disable(&p_instance->audio_adc);
}

bool xinc_drv_audio_adc_is_busy(xinc_drv_audio_adc_t const * p_instance)
{
    return xincx_audio_adc_is_busy(&p_instance->audio_adc);
}

__STATIC_INLINE
xincx_err_t xinc_drv_audio_adc_buffer_convert(xinc_drv_audio_adc_t const * const p_instance,xinc_audio_adc_value_t * p_buffer, uint16_t size)
{
	return xincx_audio_adc_buffer_convert( &p_instance->audio_adc, p_buffer,  size);
}



#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_DRV_AUDIO_ADC_H__
