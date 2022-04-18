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


#ifdef AUDIO_ADC_PRESENT
#define XINCX_AUDIO_ADC_CH_COUNT   AUDIO_ADC_CH_COUNT
#else
#define XINCX_AUDIO_ADC_CH_COUNT   1
#endif


/** @brief AUDIO_ADC driver instance data structure. */
typedef struct
{
    XINC_CDC_Type        *p_reg;  ///< Pointer to the structure with AUDIO ADC peripheral instance registers.
    XINC_CPR_CTL_Type    *p_cpr; 
    XINC_CPR_AO_CTL_Type *p_cprAO; 		///< Pointer to a structure with CPR registers.    
    uint8_t              id;
    uint8_t              drv_inst_idx; ///< Index of the driver instance. For internal use only.

} xincx_audio_adc_t;

/** @brief Macro for creating a PWM driver instance. */

#define XINCX_AUDIO_ADC_INSTANCE(Id)                                        \
{                                                                           \
    .p_reg          =   XINCX_CONCAT_2(XINC_AUDIO_ADC,Id) ,                  \
    .p_cpr          =   XINC_CPR,                                           \
    .p_cprAO        =   XINC_CPR_AO,                                           \
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
    .interrupt_priority         = XINCX_AUDIO_ADC_CONFIG_IRQ_PRIORITY,                       \
    .frequency                  = (xinc_audio_adc_freq_t)XINC_AUDIO_ADC_FREQ_16K,                    \
    .mic_p                      = XINC_GPIO_18,														\
    .mic_n                      = XINC_GPIO_0,														\
    .mic_bias                   = XINC_GPIO_19,	\
	.ch_config.adc_fifo_len     = 8,           \
	.ch_config.reg0.bits.adc_d_vol     = 0,           \
	.ch_config.reg0.bits.hpf_en     = 1,           \
	.ch_config.reg0.bits.hpf_bypass      = 0,           \
}


/** @brief AUDIO_ADC driver configuration structure. */
#define XINCX_AUDIO_ADC_PIN_NOT_USED  0xFF

typedef struct
{
    uint32_t                mic_p;                 ///< mic_p pin number.
    uint32_t                mic_n;                 ///< mic_n pin number.
    uint32_t                mic_bias;                 ///< mic_bias pin number.
    xinc_audio_adc_freq_t   frequency;           ///< audio adc frequency.
    uint8_t                 interrupt_priority;  ///< Interrupt priority.
    xinc_audio_adc_channel_config_t ch_config;
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
    volatile xinc_audio_adc_value_t * p_buffer; ///< Pointer to buffer with converted samples.
    uint32_t          size;     ///< Number of samples in the buffer.
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
                           xincx_audio_adc_evt_handler_t  event_handler,
                             void                     * p_context);

/**
 * @brief Function for uninitializing the AUDIO_ADC.
 *
 * This function stops all ongoing conversions and disables all channels.
 */
void xincx_audio_adc_uninit(xincx_audio_adc_t const * const p_instance);


void xincx_audio_adc_enable(xincx_audio_adc_t const * const p_instance);
                            
void xincx_audio_adc_disable(xincx_audio_adc_t const * const p_instance);

                            
void xincx_audio_adc_config_set(xincx_audio_adc_t const * const p_instance,                                
                                   xincx_audio_adc_config_t const * p_config);


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

