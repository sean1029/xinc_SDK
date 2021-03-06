/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_SPIM_H__
#define XINCX_SPIM_H__

#include <xincx.h>
#include <hal/xinc_spim.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_spim SPIM driver
 * @{
 * @ingroup xinc_spim
 * @brief   Serial Peripheral Interface Master with EasyDMA (SPIM) driver.
 */

/** @brief Data structure of the Serial Peripheral Interface Master with EasyDMA (SPIM) driver instance. */
typedef struct
{
    XINC_SPIM_Type * p_reg;        ///< Pointer to a structure with SPIM registers.
    XINC_CPR_CTL_Type * p_cpr; 		///< Pointer to a structure with CPR registers.
    uint8_t         drv_inst_idx; ///< Index of the driver instance. For internal use only.
    uint8_t         id;
} xincx_spim_t;

#ifndef __XINCX_DOXYGEN__
enum {
#if XINCX_CHECK(XINCX_SPIM0_ENABLED)
    XINCX_SPIM0_INST_IDX ,
#endif
#if XINCX_CHECK(XINCX_SPIM1_ENABLED)
    XINCX_SPIM1_INST_IDX ,
#endif
#if XINCX_CHECK(XINCX_SPIM2_ENABLED)
    XINCX_SPIM2_INST_IDX ,
#endif
    XINCX_SPIM_ENABLED_COUNT
};
#endif

/** @brief Macro for creating an instance of the SPIM driver. */
#define XINCX_SPIM_INSTANCE(Id)                                         \
{                                                                       \
    .p_reg          = XINCX_CONCAT_2(XINC_SPIM, Id),                     \
    .p_cpr          = XINC_CPR,                                         \
    .drv_inst_idx   = XINCX_CONCAT_3(XINCX_SPIM, Id, _INST_IDX),         \
    .id             = Id,                                               \
}

/**
 * @brief This value can be provided instead of a pin number for signals MOSI,
 *        MISO, and Slave Select to specify that the given signal is not used and
 *        therefore does not need to be connected to a pin.
 */
#define XINCX_SPIM_PIN_NOT_USED  0xFF

/** @brief Configuration structure of the SPIM driver instance. */
typedef struct
{
    uint8_t sck_pin;      ///< SCK pin number.
    uint8_t mosi_pin;     ///< MOSI pin number (optional).
                            /**< Set to @ref XINCX_SPIM_PIN_NOT_USED
                             *   if this signal is not needed. */
    uint8_t miso_pin;     ///< MISO pin number (optional).
                            /**< Set to @ref XINCX_SPIM_PIN_NOT_USED
                             *   if this signal is not needed. */
    uint8_t ss_pin;       ///< Slave Select pin number (optional).
                            /**< Set to @ref XINCX_SPIM_PIN_NOT_USED
                             *   if this signal is not needed. */
#if defined (XC66XX_M4) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
    uint8_t d2_pin; 
    uint8_t d3_pin;
#endif
    
    bool ss_active_high;  ///< Polarity of the Slave Select pin during transmission.
    uint8_t irq_priority; ///< Interrupt priority.
    uint8_t orc;          ///< Overrun character.
                            /**< This character is used when all bytes from the TX buffer are sent,
                                 but the transfer continues due to RX. */
    xinc_spim_frequency_t frequency; ///< SPIM frequency.
    xinc_spim_mode_t      mode;      ///< SPIM mode.
    xinc_spim_bit_order_t bit_order; ///< SPIM bit order.

} xincx_spim_config_t;


#define XINCX_SPIM_DEFAULT_EXTENDED_CONFIG

#if defined (XC66XX_M4) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
#define XINC_DRV_SPIM_DEFAULT_D2_D3_CONFIG         .d2_pin = XINCX_SPIM_PIN_NOT_USED,\
                                                  .d3_pin = XINCX_SPIM_PIN_NOT_USED,
#else
#define XINC_DRV_SPIM_DEFAULT_D2_D3_CONFIG       
#endif


/** @brief The default configuration of the SPIM master instance. */
#define XINCX_SPIM_DEFAULT_CONFIG                             \
{                                                            \
    .sck_pin        = XINCX_SPIM_PIN_NOT_USED,                \
    .mosi_pin       = XINCX_SPIM_PIN_NOT_USED,                \
    .miso_pin       = XINCX_SPIM_PIN_NOT_USED,                \
    .ss_pin         = XINCX_SPIM_PIN_NOT_USED,                \
     XINC_DRV_SPIM_DEFAULT_D2_D3_CONFIG                       \
    .ss_active_high = false,                                 \
    .irq_priority   = 0,                                    \
    .orc            = 0xFF,                                  \
    .frequency      = XINC_SPIM_FREQ_4M,                      \
    .mode           = XINC_SPIM_MODE_0,                       \
    .bit_order      = XINC_SPIM_BIT_ORDER_MSB_FIRST,          \
    XINCX_SPIM_DEFAULT_EXTENDED_CONFIG                        \
}

/** @brief Flag indicating that TX buffer address will be incremented after transfer. */
#define XINCX_SPIM_FLAG_TX_POSTINC          (1UL << 0)
/** @brief Flag indicating that RX buffer address will be incremented after transfer. */
#define XINCX_SPIM_FLAG_RX_POSTINC          (1UL << 1)
/** @brief Flag indicating that the interrupt after each transfer will be suppressed, and the event handler will not be called. */
#define XINCX_SPIM_FLAG_NO_XFER_EVT_HANDLER (1UL << 2)
/** @brief Flag indicating that the transfer will be set up, but not started. */
#define XINCX_SPIM_FLAG_HOLD_XFER           (1UL << 3)
/** @brief Flag indicating that the transfer will be executed multiple times. */
#define XINCX_SPIM_FLAG_REPEATED_XFER       (1UL << 4)

/** @brief Single transfer descriptor structure. */
typedef struct
{
    uint8_t  * p_tx_buffer; ///< Pointer to TX buffer.
    size_t          tx_length;   ///< TX buffer length.
    uint8_t       * p_rx_buffer; ///< Pointer to RX buffer.
    size_t          rx_length;   ///< RX buffer length.
} xincx_spim_xfer_desc_t;

/**
 * @brief Macro for setting up single transfer descriptor.
 *
 * This macro is for internal use only.
 */
#define XINCX_SPIM_SINGLE_XFER(p_tx, tx_len, p_rx, rx_len)  \
{                                                           \
    .p_tx_buffer = (uint8_t *)(p_tx),                       \
    .tx_length = (tx_len),                                  \
    .p_rx_buffer = (p_rx),                                  \
    .rx_length = (rx_len),                                  \
}

/** @brief Macro for setting the duplex TX RX transfer. */
#define XINCX_SPIM_XFER_TRX(p_tx_buf, tx_length, p_rx_buf, rx_length) \
        XINCX_SPIM_SINGLE_XFER(p_tx_buf, tx_length, p_rx_buf, rx_length)

/** @brief Macro for setting the TX transfer. */
#define XINCX_SPIM_XFER_TX(p_buf, length) \
        XINCX_SPIM_SINGLE_XFER(p_buf, length, NULL, 0)

/** @brief Macro for setting the RX transfer. */
#define XINCX_SPIM_XFER_RX(p_buf, length) \
        XINCX_SPIM_SINGLE_XFER(NULL, 0, p_buf, length)

/**
 * @brief SPIM master driver event types, passed to the handler routine provided
 *        during initialization.
 */
typedef enum
{
    XINCX_SPIM_EVENT_DONE, ///< Transfer done.
} xincx_spim_evt_type_t;

/** @brief SPIM event description with transmission details. */
typedef struct
{
    xincx_spim_evt_type_t  type;      ///< Event type.
    xincx_spim_xfer_desc_t xfer_desc; ///< Transfer details.
} xincx_spim_evt_t;

/** @brief SPIM driver event handler type. */
typedef void (* xincx_spim_evt_handler_t)(xincx_spim_evt_t const * p_event,
                                         void *                  p_context);

/**
 * @brief Function for initializing the SPIM driver instance.
 *
 * This function configures and enables the specified peripheral.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] p_config   Pointer to the structure with the initial configuration.
 * @param[in] handler    Event handler provided by the user. If NULL, transfers
 *                       will be performed in blocking mode.
 * @param[in] p_context  Context passed to event handler.
 *
 * @retval XINCX_SUCCESS             Initialization was successful.
 * @retval XINCX_ERROR_INVALID_STATE The driver was already initialized.
 * @retval XINCX_ERROR_BUSY          Some other peripheral with the same
 *                                  instance ID is already in use. This is
 *                                  possible only if @ref xincx_prs module
 *                                  is enabled.
 * @retval XINCX_ERROR_NOT_SUPPORTED Requested configuration is not supported
 *                                  by the SPIM instance.
 */
xincx_err_t xincx_spim_init(xincx_spim_t const * const  p_instance,
                          xincx_spim_config_t const * p_config,
                          xincx_spim_evt_handler_t    handler,
                          void *                     p_context);

/**
 * @brief Function for uninitializing the SPIM driver instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_spim_uninit(xincx_spim_t const * const p_instance);

/**
 * @brief Function for starting the SPIM data transfer.
 *
 * Additional options are provided using the @c flags parameter:
 *
 * - @ref XINCX_SPIM_FLAG_TX_POSTINC and @ref XINCX_SPIM_FLAG_RX_POSTINC -
 *   Post-incrementation of buffer addresses.
 * - @ref XINCX_SPIM_FLAG_HOLD_XFER - Driver is not starting the transfer. Use this
 *   flag if the transfer is triggered externally by PPI. Use
 *   @ref xincx_spim_start_task_get to get the address of the start task.
 * - @ref XINCX_SPIM_FLAG_NO_XFER_EVT_HANDLER - No user event handler after transfer
 *   completion. This also means no interrupt at the end of the transfer.
 *   If @ref XINCX_SPIM_FLAG_NO_XFER_EVT_HANDLER is used, the driver does not set the instance into
 *   busy state, so you must ensure that the next transfers are set up when SPIM is not active.
 *   @ref xincx_spim_end_event_get function can be used to detect end of transfer. Option can be used
 *   together with @ref XINCX_SPIM_FLAG_REPEATED_XFER to prepare a sequence of SPI transfers
 *   without interruptions.
 * - @ref XINCX_SPIM_FLAG_REPEATED_XFER - Prepare for repeated transfers. You can set
 *   up a number of transfers that will be triggered externally (for example by PPI). An example is
 *   a TXRX transfer with the options @ref XINCX_SPIM_FLAG_RX_POSTINC,
 *   @ref XINCX_SPIM_FLAG_NO_XFER_EVT_HANDLER, and @ref XINCX_SPIM_FLAG_REPEATED_XFER. After the
 *   transfer is set up, a set of transfers can be triggered by PPI that will read, for example,
 *   the same register of an external component and put it into a RAM buffer without any interrupts.
 *   @ref xincx_spim_end_event_get can be used to get the address of the END event, which can be
 *   used to count the number of transfers. If @ref XINCX_SPIM_FLAG_REPEATED_XFER is used,
 *   the driver does not set the instance into busy state, so you must ensure that the next
 *   transfers are set up when SPIM is not active.
 *
 * @note Peripherals using EasyDMA (including SPIM) require the transfer buffers
 *       to be placed in the Data RAM region. If this condition is not met,
 *       this function will fail with the error code XINCX_ERROR_INVALID_ADDR.
 *
 * @param p_instance  Pointer to the driver instance structure.
 * @param p_xfer_desc Pointer to the transfer descriptor.
 * @param flags       Transfer options (0 for default settings).
 *
 * @retval XINCX_SUCCESS             The procedure is successful.
 * @retval XINCX_ERROR_BUSY          The driver is not ready for a new transfer.
 * @retval XINCX_ERROR_NOT_SUPPORTED The provided parameters are not supported.
 * @retval XINCX_ERROR_INVALID_ADDR  The provided buffers are not placed in the Data
 *                                  RAM region.
 */
xincx_err_t xincx_spim_xfer(xincx_spim_t const * const     p_instance,
                          xincx_spim_xfer_desc_t * p_xfer_desc,
                          uint32_t                      flags);

#if XINCX_CHECK(XINCX_SPIM_EXTENDED_ENABLED) || defined(__XINCX_DOXYGEN__)
/**
 * @brief Function for starting the SPIM data transfer with DCX control.
 *
 * See @ref xincx_spim_xfer for description of additional options of transfer
 * provided by the @c flags parameter.
 *
 * @note Peripherals that use EasyDMA (including SPIM) require the transfer buffers
 *       to be placed in the Data RAM region. If this condition is not met,
 *       this function will fail with the error code XINCX_ERROR_INVALID_ADDR.
 *
 * @param p_instance  Pointer to the driver instance structure.
 * @param p_xfer_desc Pointer to the transfer descriptor.
 * @param flags       Transfer options (0 for default settings).
 * @param cmd_length  Length of the command bytes preceding the data
 *                    bytes. The DCX line will be low during transmission
 *                    of command bytes and high during transmission of data bytes.
 *                    Maximum value available for dividing the transmitted bytes
 *                    into command bytes and data bytes is @ref XINC_SPIM_DCX_CNT_ALL_CMD - 1.
 *                    The @ref XINC_SPIM_DCX_CNT_ALL_CMD value passed as the
 *                    @c cmd_length parameter causes all transmitted bytes
 *                    to be marked as command bytes.
 *
 * @retval XINCX_SUCCESS             The procedure is successful.
 * @retval XINCX_ERROR_BUSY          The driver is not ready for a new transfer.
 * @retval XINCX_ERROR_NOT_SUPPORTED The provided parameters are not supported.
 * @retval XINCX_ERROR_INVALID_ADDR  The provided buffers are not placed in the Data
 *                                  RAM region.
 */
xincx_err_t xincx_spim_xfer_dcx(xincx_spim_t const * const     p_instance,
                              xincx_spim_xfer_desc_t  * p_xfer_desc,
                              uint32_t                      flags,
                              uint8_t                       cmd_length);
#endif

/**
 * @brief Function for returning the address of a SPIM start task.
 *
 * This function is to be used if @ref xincx_spim_xfer was called with the flag @ref XINCX_SPIM_FLAG_HOLD_XFER.
 * In that case, the transfer is not started by the driver, but it must be started externally by PPI.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 *
 * @return Start task address.
 */
uint32_t xincx_spim_start_task_get(xincx_spim_t const * p_instance);

/**
 * @brief Function for returning the address of a END SPIM event.
 *
 * The END event can be used to detect the end of a transfer
 * if the @ref XINCX_SPIM_FLAG_NO_XFER_EVT_HANDLER option is used.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 *
 * @return END event address.
 */
uint32_t xincx_spim_end_event_get(xincx_spim_t const * p_instance);

/**
 * @brief Function for aborting ongoing transfer.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_spim_abort(xincx_spim_t const * p_instance);

/** @} */


void xincx_spim_0_irq_handler(void);
void xincx_spim_1_irq_handler(void);
void xincx_spim_2_irq_handler(void);
void xincx_spim_3_irq_handler(void);


#ifdef __cplusplus
}
#endif

#endif // XINCX_SPIM_H__
