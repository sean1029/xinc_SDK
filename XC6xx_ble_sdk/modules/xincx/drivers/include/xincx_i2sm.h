/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_I2SMM_H__
#define XINCX_I2SMM_H__

#include <xincx.h>
#include <hal/xinc_i2sm.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_i2sm I2SM driver
 * @{
 * @ingroup xinc_i2sm
 * @brief   Inter-IC Sound (I2SM) peripheral driver.
 */


/**
 * @brief Structure for the I2SM master driver instance.
 */
typedef struct
{
    XINC_I2S_Type * p_i2sm;        ///< Pointer to a structure with I2C registers.
    XINC_CPR_CTL_Type * p_cpr;  	///< Pointer to a structure with CPR registers.
    uint8_t        drv_inst_idx; ///< Index of the driver instance. For internal use only.
    uint8_t        id; 
} xincx_i2sm_t;


/** @brief Macro for creating a I2C master driver instance. */
#define XINCX_I2SM_INSTANCE(Id)                               \
{                                                           \
    .p_i2sm        = XINCX_CONCAT_2(XINC_I2S, Id),             \
    .p_cpr            = XINC_CPR,	                        \
    .drv_inst_idx = XINCX_CONCAT_3(XINCX_I2SM, Id, _INST_IDX), \
    .id = Id 												\
}

#ifndef __XINCX_DOXYGEN__
enum {
#if XINCX_CHECK(XINCX_I2SM0_ENABLED)
    XINCX_I2SM0_INST_IDX,
#endif
    XINCX_I2SM_ENABLED_COUNT
};
#endif


/**
 * @brief This value can be provided instead of a pin number for the signals
 *        SDOUT, SDIN, and MCK to specify that a given signal is not used
 *        and therefore does not need to be connected to a pin.
 */
#define XINCX_I2SM_PIN_NOT_USED  0xFF

/** @brief I2SM driver configuration structure. */
typedef struct
{
    uint8_t sck_pin;      ///< SCK pin number.
    uint8_t lrck_pin;     ///< LRCK pin number.
    uint8_t mck_pin;      ///< MCK pin number.
                          /**< Optional. Use @ref XINCX_I2SM_PIN_NOT_USED
                           *   if this signal is not needed. */
    uint8_t sdout_pin;    ///< SDOUT pin number.
                          /**< Optional. Use @ref XINCX_I2SM_PIN_NOT_USED
                           *   if this signal is not needed. */
    uint8_t sdin_pin;     ///< SDIN pin number.
                          /**< Optional. Use @ref XINCX_I2SM_PIN_NOT_USED
                           *   if this signal is not needed. */
    uint8_t irq_priority; ///< Interrupt priority.

    xinc_i2sm_mode_t     mode;         ///< Mode of operation.
    xinc_i2sm_format_t   format;       ///< Frame format.
    xinc_i2sm_align_t    alignment;    ///< Alignment of sample within a frame.
    xinc_i2sm_swidth_t   sample_width; ///< Sample width.
    xinc_i2sm_channels_t channels;     ///< Enabled channels.
    xinc_i2sm_mck_t      mck_setup;    ///< Master clock setup.
    xinc_i2sm_ratio_t    ratio;        ///< MCK/LRCK ratio.
} xincx_i2sm_config_t;

/** @brief I2SM driver buffers structure. */
typedef struct
{
    uint32_t       * p_rx_buffer; ///< Pointer to the buffer for received data.
    uint32_t const * p_tx_buffer; ///< Pointer to the buffer with data to be sent.
} xincx_i2sm_buffers_t;

/** @brief I2SM driver default configuration. */
#define XINCX_I2SM_DEFAULT_CONFIG                                   \
{                                                                 \
    .sck_pin      = XINCX_I2S_CONFIG_SCK_PIN,                      \
    .lrck_pin     = XINCX_I2S_CONFIG_LRCK_PIN,                     \
    .mck_pin      = XINCX_I2S_CONFIG_MCK_PIN,                      \
    .sdout_pin    = XINCX_I2S_CONFIG_SDOUT_PIN,                    \
    .sdin_pin     = XINCX_I2S_CONFIG_SDIN_PIN,                     \
    .irq_priority = XINCX_I2S_CONFIG_IRQ_PRIORITY,                 \
    .mode         = (xinc_i2sm_mode_t)XINCX_I2S_CONFIG_MASTER,       \
    .format       = (xinc_i2sm_format_t)XINCX_I2S_CONFIG_FORMAT,     \
    .alignment    = (xinc_i2sm_align_t)XINCX_I2S_CONFIG_ALIGN,       \
    .sample_width = (xinc_i2sm_swidth_t)XINCX_I2S_CONFIG_SLOT_WIDTH,     \
    .channels     = (xinc_i2sm_channels_t)XINCX_I2S_CONFIG_CHANNELS, \
    .mck_setup    = (xinc_i2sm_mck_t)XINCX_I2S_CONFIG_MCK_SETUP,     \
    .ratio        = (xinc_i2sm_ratio_t)XINCX_I2S_CONFIG_RATIO,       \
}

//
#define XINCX_I2SM_STATUS_NEXT_BUFFERS_NEEDED  (1UL << 0)
    /**< The application must provide buffers that are to be used in the next
     *   part of the transfer. A call to @ref xincx_i2sm_next_buffers_set must
     *   be done before the currently used buffers are completely processed
     *   (that is, the time remaining for supplying the next buffers depends on
     *   the used size of the buffers). */

/**
 * @brief I2SM driver data handler type.
 *
 * A data handling function of this type must be specified during the initialization
 * of the driver. The driver will call this function when it finishes using
 * buffers passed to it by the application, and when it needs to be provided
 * with buffers for the next part of the transfer.
 *
 * @note The @c p_released pointer passed to this function is temporary and
 *       will be invalid after the function returns, hence it cannot be stored
 *       and used later. If needed, the pointed content (that is, buffers pointers)
 *       must be copied instead.
 *
 * @param[in] p_released  Pointer to a structure with pointers to buffers
 *                        passed previously to the driver that will no longer
 *                        be accessed by it (they can be now safely released or
 *                        used for another purpose, in particular for a next
 *                        part of the transfer).
 *                        This pointer will be NULL if the application did not
 *                        supply the buffers for the next part of the transfer
 *                        (via a call to @ref xincx_i2sm_next_buffers_set) since
 *                        the previous time the data handler signaled such need.
 *                        This means that data corruption occurred (the previous
 *                        buffers are used for the second time) and no buffers
 *                        can be released at the moment.
 *                        Both pointers in this structure are NULL when the
 *                        handler is called for the first time after a transfer
 *                        is started, because no data has been transferred yet
 *                        at this point. In all successive calls the pointers
 *                        specify what has been sent (TX) and what has been
 *                        received (RX) in the part of transfer that has just
 *                        been completed (provided that a given direction is
 *                        enabled, see @ref xincx_i2sm_start).
 * @param[in] status  Bit field describing the current status of the transfer.
 *                    It can be 0 or a combination of the following flags:
 *                    - @ref XINCX_I2SM_STATUS_NEXT_BUFFERS_NEEDED
 */
typedef void (* xincx_i2sm_data_handler_t)(xincx_i2sm_buffers_t const * p_released,
                                                uint32_t                   status,
                                                void *                 p_context);


/**
 * @brief Function for initializing the I2SM driver.
 *
 * @param[in] p_config Pointer to the structure with the initial configuration.
 * @param[in] handler  Data handler provided by the user. Must not be NULL.
 *
 * @retval XINCX_SUCCESS             Initialization was successful.
 * @retval XINCX_ERROR_INVALID_STATE The driver was already initialized.
 * @retval XINCX_ERROR_INVALID_PARAM The requested combination of configuration
 *                                  options is not allowed by the I2SM peripheral.
 */
xincx_err_t xincx_i2sm_init(xincx_i2sm_t const *        p_instance,
                            xincx_i2sm_config_t const * p_config,
                            xincx_i2sm_data_handler_t   handler,
                             void *                    p_context);

/** @brief Function for uninitializing the I2SM driver. */
void xincx_i2sm_uninit(xincx_i2sm_t const *        p_instance);

 
/**
 * @brief Function for setting the pointer to the receive buffer.
 *
 * @note The size of the buffer can be set only by calling
 *       @ref xinc_i2sm_transfer_set.
 *
 * @param[in] p_reg    Pointer to the structure of registers of the peripheral.
 * @param[in] p_buffer Pointer to the receive buffer.
 */
void xincx_i2sm_rx_buffer_set(xincx_i2sm_t const *        p_instance,uint16_t         size,
                                           uint32_t *     p_buffer);


/**
 * @brief Function for setting the pointer to the transmit buffer.
 *
 * @note The size of the buffer can be set only by calling
 *       @ref xinc_i2sm_transfer_set.
 *
 * @param[in] p_reg    Pointer to the structure of registers of the peripheral.
 * @param[in] p_buffer Pointer to the transmit buffer.
 */
void xincx_i2sm_tx_buffer_set(xincx_i2sm_t const *        p_instance,uint16_t         size,
                                           uint32_t const * p_buffer);


 /**
 * @brief Function for setting up the I2SM transfer.
 *
 * This function sets up the RX and TX buffers and enables reception or
 * transmission (or both) accordingly. If the transfer in a given direction is not
 * required, pass NULL instead of the pointer to the corresponding buffer.
 *
 * @param[in] p_reg       Pointer to the structure of registers of the peripheral.
 * @param[in] size        Size of the buffers (in 32-bit words).
 * @param[in] p_rx_buffer Pointer to the receive buffer.
 *                        Pass NULL to disable reception.
 * @param[in] p_tx_buffer Pointer to the transmit buffer.
 *                        Pass NULL to disable transmission.
 */
void xincx_i2sm_transfer_set(xincx_i2sm_t const *        p_instance,
                                          uint16_t         size,
                                          uint32_t *       p_rx_buffer,
                                          uint32_t const * p_tx_buffer);

/**
 * @brief Function for starting the continuous I2SM transfer.
 *
 * The I2SM data transfer can be performed in one of three modes: RX (reception)
 * only, TX (transmission) only, or in both directions simultaneously.
 * The mode is selected by specifying a proper buffer for a given direction
 * in the call to this function or by passing NULL instead if this direction
 * is to be disabled.
 *
 * The length of the buffer (which is a common value for RX and TX if both
 * directions are enabled) is specified in 32-bit words. One 32-bit memory
 * word can either contain four 8-bit samples, two 16-bit samples, or one
 * right-aligned 24-bit sample sign-extended to a 32-bit value.
 * For a detailed memory mapping for different supported configurations,
 * see the @linkProductSpecification52.
 *
 * @note Peripherals using EasyDMA (including I2SM) require the transfer buffers
 *       to be placed in the Data RAM region. If this condition is not met,
 *       this function will fail with the error code XINCX_ERROR_INVALID_ADDR.
 *
 * @param[in] p_initial_buffers Pointer to a structure specifying the buffers
 *                              to be used in the initial part of the transfer
 *                              (buffers for all consecutive parts are provided
 *                              through the data handler).
 * @param[in] buffer_size       Size of the buffers (in 32-bit words).
 *                              Must not be 0.
 * @param[in] flags             Transfer options (0 for default settings).
 *                              Currently, no additional flags are available.
 *
 * @retval XINCX_SUCCESS             The operation was successful.
 * @retval XINCX_ERROR_INVALID_STATE Transfer was already started or
 *                                  the driver has not been initialized.
 * @retval XINCX_ERROR_INVALID_ADDR  The provided buffers are not placed
 *                                  in the Data RAM region.
 */
xincx_err_t xincx_i2sm_start(xincx_i2sm_t const *        p_instance,
                            xincx_i2sm_buffers_t const * p_initial_buffers,
                          uint16_t                   buffer_size,
                          uint8_t                    flags);

/**
 * @brief Function for supplying the buffers to be used in the next part of
 *        the transfer.
 *
 * The application must call this function when the data handler receives
 * @ref XINCX_I2SM_STATUS_NEXT_BUFFERS_NEEDED in the @c status parameter.
 * The call can be done immediately from the data handler function or later,
 * but it has to be done before the I2SM peripheral finishes processing the
 * buffers supplied previously. Otherwise, data corruption will occur.
 *
 * @sa xincx_i2sm_data_handler_t
 *
 * @retval XINCX_SUCCESS             If the operation was successful.
 * @retval XINCX_ERROR_INVALID_STATE If the buffers were already supplied or
 *                                  the peripheral is currently being stopped.
 */
xincx_err_t xincx_i2sm_next_buffers_set(xincx_i2sm_t const *        p_instance,
                                        xincx_i2sm_buffers_t const * p_buffers);

/** @brief Function for stopping the I2SM transfer. */
void xincx_i2sm_stop(xincx_i2sm_t const *        p_instance);

/** @} */


void xincx_i2sm0_irq_handler(void);


#ifdef __cplusplus
}
#endif

#endif // XINCX_I2SM_H__

