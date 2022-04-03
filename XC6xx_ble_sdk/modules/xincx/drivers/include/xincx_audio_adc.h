/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_AUDIO_ADC_H__
#define XINCX_AUDIO_ADC_H__

#include <xincx.h>
#include <hal/xinc_audio_adc.h>

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @defgroup xinc_audio_adc AUDIO_ADC legacy driver
 * @{
 * @ingroup xinc_audio_adc
 * @brief   Successive Approximation Analog-to-Digital Converter (AUDIO_ADC) peripheral legacy driver.
 */

/** @brief Value to be set as high limit to disable limit detection. */
#define XINCX_AUDIO_ADC_LIMITH_DISABLED (2047)
/** @brief Value to be set as low limit to disable limit detection. */
#define XINCX_AUDIO_ADC_LIMITL_DISABLED (-2048)

/** @brief AUDIO_ADC driver instance data structure. */
typedef struct
{
    XINC_CDC_Type       *p_reg;  ///< Pointer to the structure with AUDIO ADC peripheral instance registers.
    XINC_CPR_CTL_Type   *p_cpr; 
    uint8_t             id;
    uint8_t             drv_inst_idx; ///< Index of the driver instance. For internal use only.

} xincx_audio_adc_t;

/** @brief Macro for creating a PWM driver instance. */

#define XINCX_AUDIO_ADC_INSTANCE(Id)                                        \
{                                                                           \
    .p_reg          =   XINCX_CONCAT_2(XINC_AUDIO_ADC,Id) ,                  \
    .p_cpr          =   XINC_CPR,                                           \
    .id 		    =   Id,                                                 \
    .drv_inst_idx   =   XINCX_CONCAT_3(XINCX_AUDIO_ADC, Id, _INST_IDX),     \
}

#ifndef __XINCX_DOXYGEN__
enum {
#if XINCX_CHECK(XINCX_AUDIO_ADC_ENABLED)
    XINCX_AUDIO_ADC0_INST_IDX,
#endif
    XINCX_AUDIO_ADC_ENABLED_COUNT
};
#endif

/** @brief Macro for setting @ref xinc_audio_adc_config_t to default settings. */

#define XINCX_AUDIO_ADC_DEFAULT_CONFIG                                               \
{                                                                               \
    .interrupt_priority = XINCX_AUDIO_ADC_CONFIG_IRQ_PRIORITY,                       \
    .refvol             = (xinc_audio_adc_refvol_t)XINC_AUDIO_ADC_CHANNEL_REFVOL_2_47,      \
    .freq               = (xinc_audio_adc_freq_t)XINC_AUDIO_ADC_FREQ_8M,                    \
    .waite_time         = 4,														\
}
/**
 * @brief Macro for setting @ref xinc_audio_adc_channel_config_t to default settings
 *        in single-ended mode.
 *
 * @param PIN_P Analog input.
 */


#define XINCX_AUDIO_ADC_DEFAULT_CHANNEL_CONFIG                                         \
{                                                                                 \
    .mode               =  XINC_AUDIO_ADC_MODE_SINGLE_ENDED,                              \
	.adc_fifo_len       = XINCX_AUDIO_ADC_CONFIG_FIFO_LEN                             \
}

/** @brief AUDIO_ADC driver configuration structure. */

typedef struct
{
    uint8_t                interrupt_priority; ///< Interrupt priority.
    xinc_audio_adc_refvol_t    refvol;  ///< Reference control value.
    xinc_audio_adc_freq_t      freq;     
    uint32_t            waite_time;  
} xincx_audio_adc_config_t;



/** @brief AUDIO_ADC driver event types. */
typedef enum
{
    XINCX_AUDIO_ADC_EVT_DONE,         ///< Event generated when the buffer is filled with samples.
    XINCX_AUDIO_ADC_EVT_LIMIT,        ///< Event generated after one of the limits is reached.
    XINCX_AUDIO_ADC_EVT_ERROR ///< Event generated when the error is happen.
} xincx_audio_adc_evt_type_t;

/** @brief AUDIO_ADC driver done event data. */
typedef struct
{
    xinc_audio_adc_value_t * p_buffer; ///< Pointer to buffer with converted samples.
    uint16_t          size;     ///< Number of samples in the buffer.
    int16_t           adc_value;     ///< Number of samples in the buffer.
    uint8_t           channel;    ///< Channel on which the limit was detected.
} xincx_audio_adc_done_evt_t;


/** @brief AUDIO_ADC driver event structure. */
typedef struct
{
    xincx_audio_adc_evt_type_t type; ///< Event type.
    union
    {
        xincx_audio_adc_done_evt_t  done;  ///< Data for @ref XINCX_AUDIO_ADC_EVT_DONE event.
    } data;                           ///< Union to store event data.
} xincx_audio_adc_evt_t;

/**
 * @brief AUDIO_ADC driver event handler.
 *
 * @param[in] p_event Pointer to an AUDIO_ADC driver event. The event structure is allocated on
 *                    the stack, so it is valid only within the context of the event handler.
 */
typedef void (* xincx_audio_adc_evt_handler_t)(xincx_audio_adc_evt_t const * p_event,void *p_context);

/**
 * @brief Function for initializing the AUDIO_ADC.
 *
 * @param[in] p_config      Pointer to the structure with initial configuration.
 * @param[in] event_handler Event handler provided by the user.
 *                          Must not be NULL.
 *
 * @retval XINCX_SUCCESS             Initialization was successful.
 * @retval XINCX_ERROR_INVALID_STATE The driver is already initialized.
 */
xincx_err_t xincx_audio_adc_init(xincx_audio_adc_t const * const p_instance,
                            xincx_audio_adc_config_t const * p_config,
                           xincx_audio_adc_evt_handler_t  event_handler);

/**
 * @brief Function for uninitializing the AUDIO_ADC.
 *
 * This function stops all ongoing conversions and disables all channels.
 */
void xincx_audio_adc_uninit(xincx_audio_adc_t const * const p_instance);



void xincx_audio_adc_config_set(xincx_audio_adc_t const * const p_instance,                                
                                   xincx_audio_adc_config_t const * p_config);
/**
 * @brief Function for initializing an AUDIO_ADC channel.
 *
 * This function configures and enables the channel.
 *
 * @param[in] channel  Channel index.
 * @param[in] p_config Pointer to the structure with the initial configuration.
 *
 * @retval XINCX_SUCCESS             Initialization was successful.
 * @retval XINCX_ERROR_INVALID_STATE The AUDIO_ADC was not initialized.
 * @retval XINCX_ERROR_NO_MEM        The specified channel was already allocated.
 */
xincx_err_t xincx_audio_adc_channel_init(xincx_audio_adc_t const * const p_instance,
                                    uint8_t     channel,
                                   xinc_audio_adc_channel_config_t const * const p_config);

/**
 * @brief Function for uninitializing an AUDIO_ADC channel.
 *
 * @param[in] channel Channel index.
 *
 * @retval XINCX_SUCCESS    Uninitialization was successful.
 * @retval XINCX_ERROR_BUSY The AUDIO_ADC is busy.
 */
xincx_err_t xincx_audio_adc_channel_uninit(xincx_audio_adc_t const * const p_instance,uint8_t channel);

/**
 * @brief Function for starting the AUDIO_ADC sampling.
 *
 * @retval XINCX_SUCCESS             The AUDIO_ADC sampling was triggered.
 * @retval XINCX_ERROR_INVALID_STATE The AUDIO_ADC is in idle state.
 */
xincx_err_t xincx_audio_adc_sample(xincx_audio_adc_t const * const p_instance,uint8_t channel);

/**
 * @brief Blocking function for executing a single AUDIO_ADC conversion.
 *
 * This function selects the desired input, starts a single conversion,
 * waits for it to finish, and returns the result.
 *
 * The function fails if the AUDIO_ADC is busy.
 *
 * @param[in]  channel Channel.
 * @param[out] p_value Pointer to the location where the result is to be placed.
 *
 * @retval XINCX_SUCCESS    The conversion was successful.
 * @retval XINCX_ERROR_BUSY The AUDIO_ADC driver is busy.
 */
xincx_err_t xincx_audio_adc_sample_convert(xincx_audio_adc_t const * const p_instance,uint8_t channel, xinc_audio_adc_value_t * p_value);

/**
 * @brief Function for issuing conversion of data to the buffer.
 *
 * This function is non-blocking. The application is notified about filling the buffer by the event
 * handler. Conversion will be done on all enabled channels. If the AUDIO_ADC is in idle state, the
 * function will set up EasyDMA for the conversion. The AUDIO_ADC will be ready for sampling and wait
 * for the SAMPLE task. It can be triggered manually by the @ref xinc_audio_adc_sample function
 * or by PPI using the @ref XINC_AUDIO_ADC_TASK_SAMPLE task. If one buffer is already set and the
 * conversion is ongoing, calling this function will result in queuing the given buffer.
 * The driver will start filling the issued buffer when the first one is completed.
 * If the function is called again before the first buffer is filled or calibration
 * is in progress, it will return with error.
 *
 * @param[in] buffer Result buffer.
 * @param[in] size   Buffer size in words.
 *
 * @retval XINCX_SUCCESS    The conversion was successful.
 * @retval XINCX_ERROR_BUSY The driver already has two buffers set or the calibration is in progress.
 */
xincx_err_t xincx_audio_adc_buffer_convert(xincx_audio_adc_t const * const p_instance,xinc_audio_adc_value_t * buffer, uint16_t size);


/**
 * @brief Function for retrieving the AUDIO_ADC state.
 *
 * @retval true  The AUDIO_ADC is busy.
 * @retval false The AUDIO_ADC is ready.
 */
bool xincx_audio_adc_is_busy(xincx_audio_adc_t const * const p_instance);



#ifdef __cplusplus
}
#endif

#endif // XINCX_AUDIO_ADC_H__

