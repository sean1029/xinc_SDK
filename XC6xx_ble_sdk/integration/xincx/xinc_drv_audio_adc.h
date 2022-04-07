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
//typedef xincx_saadc_config_t xinc_drv_saadc_config_t;

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
 * @brief I2C master clock frequency.
 */
typedef enum
{
    XINC_DRV_I2C_FREQ_100K = 0 , ///< 100 kbps.
    XINC_DRV_I2C_FREQ_400K = 1   ///< 400 kbps.
} xinc_drv_audio_adc_frequency_t;

/**
 * @brief Structure for the AUDIO ADC driver instance configuration.
 */
typedef struct
{
    uint32_t                scl;                 ///< SCL pin number.
    uint32_t                sda;                 ///< SDA pin number.
    xinc_drv_audio_adc_frequency_t frequency;           ///< I2C frequency.
    uint8_t                 interrupt_priority;  ///< Interrupt priority.
    bool                    clear_bus_init;      ///< Clear bus during init.
    bool                    hold_bus_uninit;     ///< Hold pull up state on gpio pins after uninit.
} xinc_drv_audio_adc_config_t;

/**
 * @brief AUDIO_ADC driver instance default configuration.
 */
#define XINC_DRV_AUDIO_ADC_DEFAULT_CONFIG                                               \
{                                                                                \
    .frequency          = (xinc_drv_i2c_frequency_t)I2C_DEFAULT_CONFIG_FREQUENCY, \
    .scl                = 31,                                                    \
    .sda                = 31,                                                    \
    .interrupt_priority = I2C_DEFAULT_CONFIG_IRQ_PRIORITY,                       \
    .clear_bus_init     = I2C_DEFAULT_CONFIG_CLR_BUS_INIT,                       \
    .hold_bus_uninit    = I2C_DEFAULT_CONFIG_HOLD_BUS_UNINIT,                    \
}

/**
 * @brief I2C master driver event types.
 */
typedef enum
{
    XINC_DRV_AUDIO_ADC_EVT_DONE,         ///< Transfer completed event.
    XINC_DRV_AUDIO_ADC_EVT_ADDRESS_NACK, ///< Error event: NACK received after sending the address.
    XINC_DRV_AUDIO_ADC_EVT_DATA_NACK     ///< Error event: NACK received after sending a data byte.
} xinc_drv_audio_adc_evt_type_t;







/**
 * @brief Structure for a AUDIO ADC event.
 */
typedef struct
{

} xinc_drv_audio_adc_evt_t;

/**
 * @brief I2C event handler prototype.
 */
typedef void (* xinc_drv_audio_adc_evt_handler_t)(xinc_drv_audio_adc_evt_t const * p_event,
                                           void *                    p_context);

/**
 * @brief Function for initializing the I2C driver instance.
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
 * @brief Function for uninitializing the I2C instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
__STATIC_INLINE
void xinc_drv_audio_adc_uninit(xinc_drv_audio_adc_t const * p_instance);

/**
 * @brief Function for enabling the I2C instance.
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



__STATIC_INLINE
ret_code_t xinc_drv_audio_adc_rx(xinc_drv_audio_adc_t const * p_instance,
                          uint8_t               address,
                          uint8_t *             p_data,
                          uint16_t               length);


/**
 * @brief Function for checking the audio_adc driver state.
 *
 * @param[in] p_instance I2C instance.
 *
 * @retval true  If the I2C driver is currently busy performing a transfer.
 * @retval false If the I2C driver is ready for a new transfer.
 */
__STATIC_INLINE
bool xinc_drv_audio_adc_is_busy(xinc_drv_audio_adc_t const * p_instance);




#ifndef SUPPRESS_INLINE_IMPLEMENTATION





__STATIC_INLINE
void xinc_drv_i2c_uninit(xinc_drv_audio_adc_t const * p_instance)
{
   
}

__STATIC_INLINE
void xinc_drv_audio_adc_enable(xinc_drv_audio_adc_t const * p_instance)
{
    
}

__STATIC_INLINE
void xinc_drv_audio_adc_disable(xinc_drv_audio_adc_t const * p_instance)
{

}



#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_DRV_I2C_H__
