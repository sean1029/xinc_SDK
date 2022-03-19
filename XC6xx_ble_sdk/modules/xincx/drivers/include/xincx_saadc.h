/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_SAADC_H__
#define XINCX_SAADC_H__

#include <xincx.h>
#include <hal/xinc_saadc.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(XINCX_SAADC_API_V2)
#include "xinc_saadc_v2.h"
#else
/**
 * @defgroup xinc_saadc SAADC legacy driver
 * @{
 * @ingroup xinc_saadc
 * @brief   Successive Approximation Analog-to-Digital Converter (SAADC) peripheral legacy driver.
 */

/** @brief Value to be set as high limit to disable limit detection. */
#define XINCX_SAADC_LIMITH_DISABLED (2047)
/** @brief Value to be set as low limit to disable limit detection. */
#define XINCX_SAADC_LIMITL_DISABLED (-2048)

/** @brief SAADC driver instance data structure. */
typedef struct
{
    XINC_SAADC_Type     *p_reg;  ///< Pointer to the structure with PWM peripheral instance registers.
    XINC_CPR_CTL_Type   *p_cpr; 
    uint8_t             id;
    uint8_t             drv_inst_idx; ///< Index of the driver instance. For internal use only.

} xincx_saadc_t;

/** @brief Macro for creating a PWM driver instance. */

#define XINCX_SAADC_INSTANCE(Id)                                            \
{                                                                           \
    .p_reg          =   XINCX_CONCAT_2(XINC_SAADC,Id),                       \
    .p_cpr          =   XINC_CPR,                                           \
    .id 		    =   Id,                                                 \
    .drv_inst_idx   =   XINCX_CONCAT_3(XINCX_SAADC, Id, _INST_IDX),          \
};

#ifndef __XINCX_DOXYGEN__
enum {
#if XINCX_CHECK(XINCX_SAADC_ENABLED)
    XINCX_SAADC0_INST_IDX,
#endif
    XINCX_SAADC_ENABLED_COUNT
};
#endif

/** @brief Macro for setting @ref xinc_saadc_config_t to default settings. */

#define XINCX_SAADC_DEFAULT_CONFIG                                               \
{                                                                               \
    .interrupt_priority = XINCX_SAADC_CONFIG_IRQ_PRIORITY,                       \
    .refvol             = (xinc_saadc_refvol_t)XINC_SAADC_CHANNEL_REFVOL_2_47,      \
    .freq               = (xinc_saadc_freq_t)XINC_SAADC_FREQ_8M,                    \
    .waite_time         = 4,														\
}
/**
 * @brief Macro for setting @ref xinc_saadc_channel_config_t to default settings
 *        in single-ended mode.
 *
 * @param PIN_P Analog input.
 */


#define XINCX_SAADC_DEFAULT_CHANNEL_CONFIG                                         \
{                                                                                 \
    .mode               =  XINC_SAADC_MODE_SINGLE_ENDED,                              \
	.adc_fifo_len       = XINCX_SAADC_CONFIG_FIFO_LEN                             \
}

/** @brief SAADC driver configuration structure. */

typedef struct
{
    uint8_t                interrupt_priority; ///< Interrupt priority.
    xinc_saadc_refvol_t    refvol;  ///< Reference control value.
    xinc_saadc_freq_t      freq;     
    uint32_t            waite_time;  
} xincx_saadc_config_t;



/** @brief SAADC driver event types. */
typedef enum
{
    XINCX_SAADC_EVT_DONE,         ///< Event generated when the buffer is filled with samples.
    XINCX_SAADC_EVT_LIMIT,        ///< Event generated after one of the limits is reached.
    XINCX_SAADC_EVT_ERROR ///< Event generated when the error is happen.
} xincx_saadc_evt_type_t;

/** @brief SAADC driver done event data. */
typedef struct
{
    xinc_saadc_value_t * p_buffer; ///< Pointer to buffer with converted samples.
    uint16_t          size;     ///< Number of samples in the buffer.
    int16_t           adc_value;     ///< Number of samples in the buffer.
    uint8_t           channel;    ///< Channel on which the limit was detected.
} xincx_saadc_done_evt_t;


/** @brief SAADC driver event structure. */
typedef struct
{
    xincx_saadc_evt_type_t type; ///< Event type.
    union
    {
        xincx_saadc_done_evt_t  done;  ///< Data for @ref XINCX_SAADC_EVT_DONE event.
    } data;                           ///< Union to store event data.
} xincx_saadc_evt_t;

/**
 * @brief SAADC driver event handler.
 *
 * @param[in] p_event Pointer to an SAADC driver event. The event structure is allocated on
 *                    the stack, so it is valid only within the context of the event handler.
 */
typedef void (* xincx_saadc_event_handler_t)(xincx_saadc_evt_t const * p_event);

/**
 * @brief Function for initializing the SAADC.
 *
 * @param[in] p_config      Pointer to the structure with initial configuration.
 * @param[in] event_handler Event handler provided by the user.
 *                          Must not be NULL.
 *
 * @retval XINCX_SUCCESS             Initialization was successful.
 * @retval XINCX_ERROR_INVALID_STATE The driver is already initialized.
 */
xincx_err_t xincx_saadc_init(xincx_saadc_t const * const p_instance,
                            xincx_saadc_config_t const * p_config,
                           xincx_saadc_event_handler_t  event_handler);

/**
 * @brief Function for uninitializing the SAADC.
 *
 * This function stops all ongoing conversions and disables all channels.
 */
void xincx_saadc_uninit(xincx_saadc_t const * const p_instance);



void xincx_saadc_config_set(xincx_saadc_t const * const p_instance,                                
                                   xincx_saadc_config_t const * p_config);
/**
 * @brief Function for initializing an SAADC channel.
 *
 * This function configures and enables the channel.
 *
 * @param[in] channel  Channel index.
 * @param[in] p_config Pointer to the structure with the initial configuration.
 *
 * @retval XINCX_SUCCESS             Initialization was successful.
 * @retval XINCX_ERROR_INVALID_STATE The SAADC was not initialized.
 * @retval XINCX_ERROR_NO_MEM        The specified channel was already allocated.
 */
xincx_err_t xincx_saadc_channel_init(xincx_saadc_t const * const p_instance,
                                    uint8_t     channel,
                                   xinc_saadc_channel_config_t const * const p_config);

/**
 * @brief Function for uninitializing an SAADC channel.
 *
 * @param[in] channel Channel index.
 *
 * @retval XINCX_SUCCESS    Uninitialization was successful.
 * @retval XINCX_ERROR_BUSY The SAADC is busy.
 */
xincx_err_t xincx_saadc_channel_uninit(xincx_saadc_t const * const p_instance,uint8_t channel);

/**
 * @brief Function for starting the SAADC sampling.
 *
 * @retval XINCX_SUCCESS             The SAADC sampling was triggered.
 * @retval XINCX_ERROR_INVALID_STATE The SAADC is in idle state.
 */
xincx_err_t xincx_saadc_sample(xincx_saadc_t const * const p_instance,uint8_t channel);

/**
 * @brief Blocking function for executing a single SAADC conversion.
 *
 * This function selects the desired input, starts a single conversion,
 * waits for it to finish, and returns the result.
 *
 * The function fails if the SAADC is busy.
 *
 * @param[in]  channel Channel.
 * @param[out] p_value Pointer to the location where the result is to be placed.
 *
 * @retval XINCX_SUCCESS    The conversion was successful.
 * @retval XINCX_ERROR_BUSY The SAADC driver is busy.
 */
xincx_err_t xincx_saadc_sample_convert(xincx_saadc_t const * const p_instance,uint8_t channel, xinc_saadc_value_t * p_value);

/**
 * @brief Function for issuing conversion of data to the buffer.
 *
 * This function is non-blocking. The application is notified about filling the buffer by the event
 * handler. Conversion will be done on all enabled channels. If the SAADC is in idle state, the
 * function will set up EasyDMA for the conversion. The SAADC will be ready for sampling and wait
 * for the SAMPLE task. It can be triggered manually by the @ref xinc_saadc_sample function
 * or by PPI using the @ref XINC_SAADC_TASK_SAMPLE task. If one buffer is already set and the
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
xincx_err_t xincx_saadc_buffer_convert(xincx_saadc_t const * const p_instance,xinc_saadc_value_t * buffer, uint16_t size);


/**
 * @brief Function for retrieving the SAADC state.
 *
 * @retval true  The SAADC is busy.
 * @retval false The SAADC is ready.
 */
bool xincx_saadc_is_busy(xincx_saadc_t const * const p_instance);



/** @} */
#endif // defined(XINCX_SAADC_API_V2)


#ifdef __cplusplus
}
#endif

#endif // XINCX_SAADC_H__

