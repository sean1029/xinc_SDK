/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_I2S_H__
#define XINCX_I2S_H__

#include <xincx.h>
#include <hal/xinc_i2s.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_i2s I2S driver
 * @{
 * @ingroup xinc_i2s
 * @brief   Two Wire Interface master (I2S) peripheral driver.
 */

/**
 * @brief Structure for the I2S master driver instance.
 */
typedef struct
{
    XINC_I2S_Type * p_i2s;        ///< Pointer to a structure with I2S registers.
    XINC_CPR_CTL_Type * p_cpr;  	///< Pointer to a structure with CPR registers.
    uint8_t        drv_inst_idx; ///< Index of the driver instance. For internal use only.
    uint8_t        id; 
} xincx_i2s_t;

/** @brief Macro for creating a I2C master driver instance. */
#define XINC_I2S_INSTANCE(Id)                               \
{                                                           \
    .p_i2s        = XINCX_CONCAT_2(XINC_I2C, Id),             \
    .p_cpr            = XINC_CPR,	                        \
    .drv_inst_idx = XINCX_CONCAT_3(XINCX_I2C, Id, _INST_IDX), \
    .id = Id 												\
}

#ifndef __XINCX_DOXYGEN__
enum {
#if XINCX_CHECK(XINCX_I2S0_ENABLED)
    XINCX_I2S0_INST_IDX,
#endif
    XINCX_I2S_ENABLED_COUNT
};
#endif

/** @brief Structure for the configuration of the I2S master driver instance. */
typedef struct
{
    uint32_t            scl;                ///< SCL pin number.
    uint32_t            sda;                ///< SDA pin number.
    xinc_i2s_frequency_t frequency;          ///< I2S frequency.
    uint8_t             interrupt_priority; ///< Interrupt priority.
    bool                hold_bus_uninit;    ///< Hold pull up state on GPIO pins after uninit.
} xincx_i2s_config_t;

/** @brief The default configuration of the I2S master driver instance. */
#define XINCX_I2S_DEFAULT_CONFIG                                                   \
{                                                                                 \
    .frequency          = (xinc_i2s_frequency_t)XINCX_I2S_DEFAULT_CONFIG_FREQUENCY, \
    .scl                = 31,                                                     \
    .sda                = 31,                                                     \
    .interrupt_priority = XINCX_I2S_DEFAULT_CONFIG_IRQ_PRIORITY,                   \
    .hold_bus_uninit    = XINCX_I2S_DEFAULT_CONFIG_HOLD_BUS_UNINIT,                \
}

/** @brief Flag indicating that the interrupt after each transfer will be suppressed, and the event handler will not be called. */
#define XINCX_I2S_FLAG_NO_XFER_EVT_HANDLER (1UL << 2)
/** @brief Flag indicating that the TX transfer will not end with a stop condition. */
#define XINCX_I2S_FLAG_TX_NO_STOP          (1UL << 5)

#define XINCX_I2S_FLAG_TX_HAS_START        (1UL << 7)
/** @brief Flag indicating that the transfer will be suspended. */
#define XINCX_I2S_FLAG_SUSPEND             (1UL << 6)


/** @brief I2S master driver event types. */
typedef enum
{
    XINCX_I2S_EVT_DONE,         ///< Transfer completed event.
    XINCX_I2S_EVT_ADDRESS_NACK, ///< Error event: NACK received after sending the address.
    XINCX_I2S_EVT_DATA_NACK,    ///< Error event: NACK received after sending a data byte.
    XINCX_I2S_EVT_OVERRUN,      ///< Error event: The unread data is replaced by new data.
    XINCX_I2S_EVT_BUS_ERROR     ///< Error event: An unexpected transition occurred on the bus.
} xincx_i2s_evt_type_t;

/** @brief I2S master driver transfer types. */
typedef enum
{
    XINCX_I2S_XFER_TX,   ///< TX transfer.
    XINCX_I2S_XFER_RX,   ///< RX transfer.
    XINCX_I2S_XFER_TXRX, ///< TX transfer followed by RX transfer with repeated start.
    XINCX_I2S_XFER_TXTX  ///< TX transfer followed by TX transfer with repeated start.
} xincx_i2s_xfer_type_t;

/** @brief Structure for a I2S transfer descriptor. */
typedef struct
{
    xincx_i2s_xfer_type_t    type;             ///< Type of transfer.
    uint8_t                 address;          ///< Slave address.
    size_t                  primary_length;   ///< Number of bytes transferred.
    size_t                  secondary_length; ///< Number of bytes transferred.
    uint8_t *               p_primary_buf;    ///< Pointer to transferred data.
    uint8_t *               p_secondary_buf;  ///< Pointer to transferred data.
} xincx_i2s_xfer_desc_t;


/** @brief Macro for setting the TX transfer descriptor. */
#define XINCX_I2S_XFER_DESC_TX(addr, p_data, length) \
{                                                   \
    .type             = XINCX_I2S_XFER_TX,           \
    .address          = (addr),                     \
    .primary_length   = (length),                   \
    .secondary_length = 0,                          \
    .p_primary_buf    = (p_data),                   \
    .p_secondary_buf  = NULL,                       \
}

/** @brief Macro for setting the RX transfer descriptor. */
#define XINCX_I2S_XFER_DESC_RX(addr, p_data, length) \
{                                                   \
    .type             = XINCX_I2S_XFER_RX,           \
    .address          = (addr),                     \
    .primary_length   = (length),                   \
    .secondary_length = 0,                          \
    .p_primary_buf    = (p_data),                   \
    .p_secondary_buf  = NULL,                       \
}

/** @brief Macro for setting the TX-RX transfer descriptor. */
#define XINCX_I2S_XFER_DESC_TXRX(addr, p_tx, tx_len, p_rx, rx_len) \
{                                                                 \
    .type             = XINCX_I2S_XFER_TXRX,                       \
    .address          = (addr),                                   \
    .primary_length   = (tx_len),                                 \
    .secondary_length = (rx_len),                                 \
    .p_primary_buf    = (p_tx),                                   \
    .p_secondary_buf  = (p_rx),                                   \
}

/** @brief Macro for setting the TX-TX transfer descriptor. */
#define XINCX_I2S_XFER_DESC_TXTX(addr, p_tx, tx_len, p_tx2, tx_len2) \
{                                                                   \
    .type             = XINCX_I2S_XFER_TXTX,                         \
    .address          = (addr),                                     \
    .primary_length   = (tx_len),                                   \
    .secondary_length = (tx_len2),                                  \
    .p_primary_buf    = (p_tx),                                     \
    .p_secondary_buf  = (p_tx2),                                    \
}

/** @brief Structure for a I2S event. */
typedef struct
{
    xincx_i2s_evt_type_t  type;      ///< Event type.
    xincx_i2s_xfer_desc_t xfer_desc; ///< Transfer details.
} xincx_i2s_evt_t;

/** @brief I2C event handler prototype. */
typedef void (* xincx_i2s_evt_handler_t)(xincx_i2s_evt_t const * p_event,
                                        void *                 p_context);

/**
 * @brief Function for initializing the I2C driver instance.
 *
 * @param[in] p_instance    Pointer to the driver instance structure.
 * @param[in] p_config      Pointer to the structure with the initial configuration.
 * @param[in] event_handler Event handler provided by the user. If NULL, blocking mode is enabled.
 * @param[in] p_context     Context passed to event handler.
 *
 * @retval XINCX_SUCCESS             Initialization is successful.
 * @retval XINCX_ERROR_INVALID_STATE The driver is in invalid state.
 * @retval XINCX_ERROR_BUSY          Some other peripheral with the same
 *                                  instance ID is already in use. This is
 *                                  possible only if @ref xincx_prs module
 *                                  is enabled.
 */
xincx_err_t xincx_i2s_init(xincx_i2s_t const *        p_instance,
                         xincx_i2s_config_t const * p_config,
                         xincx_i2s_evt_handler_t    event_handler,
                         void *                    p_context);

/**
 * @brief Function for uninitializing the I2C instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_i2s_uninit(xincx_i2s_t const * p_instance);

/**
 * @brief Function for enabling the I2S instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_i2s_enable(xincx_i2s_t const * p_instance);

/**
 * @brief Function for disabling the I2S instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_i2s_disable(xincx_i2s_t const * p_instance);

/**
 * @brief Function for sending data to a I2C slave.
 *
 * The transmission will be stopped when an error occurs. If a transfer is ongoing,
 * the function returns the error code @ref XINCX_ERROR_BUSY.
 *
 * @note This function is deprecated. Use @ref xincx_i2c_xfer instead.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] address    Address of a specific slave device (only 7 LSB).
 * @param[in] p_data     Pointer to a transmit buffer.
 * @param[in] length     Number of bytes to send.
 * @param[in] no_stop    If set, the stop condition is not generated on the bus
 *                       after the transfer has completed successfully (allowing
 *                       for a repeated start in the next transfer).
 *
 * @retval XINCX_SUCCESS                 The procedure is successful.
 * @retval XINCX_ERROR_BUSY              The driver is not ready for a new transfer.
 * @retval XINCX_ERROR_INTERNAL          An error is detected by hardware.
 * @retval XINCX_ERROR_INVALID_STATE     RX transaction is suspended on bus.
 * @retval XINCX_ERROR_DRV_I2C_ERR_ANACK Negative acknowledgement (NACK) is received after sending
 *                                      the address in polling mode.
 * @retval XINCX_ERROR_DRV_I2C_ERR_DNACK Negative acknowledgement (NACK) is received after sending
 *                                      a data byte in polling mode.
 */
xincx_err_t xincx_i2s_tx(xincx_i2s_t const * p_instance,
                       uint8_t            address,
                       uint8_t const *    p_data,
                       size_t             length,
                       bool               no_stop);

/**
 * @brief Function for reading data from a I2C slave.
 *
 * The transmission will be stopped when an error occurs. If a transfer is ongoing,
 * the function returns the error code @ref XINCX_ERROR_BUSY.
 *
 * @note This function is deprecated. Use @ref xincx_i2c_xfer instead.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] address    Address of a specific slave device (only 7 LSB).
 * @param[in] p_data     Pointer to a receive buffer.
 * @param[in] length     Number of bytes to be received.
 *
 * @retval XINCX_SUCCESS                   The procedure is successful.
 * @retval XINCX_ERROR_BUSY                The driver is not ready for a new transfer.
 * @retval XINCX_ERROR_INTERNAL            An error is detected by hardware.
 * @retval XINCX_ERROR_INVALID_STATE       TX transaction is suspended on bus.
 * @retval XINCX_ERROR_DRV_I2C_ERR_OVERRUN The unread data is replaced by new data.
 * @retval XINCX_ERROR_DRV_I2C_ERR_ANACK   Negative acknowledgement (NACK) is received after sending
 *                                        the address in polling mode.
 * @retval XINCX_ERROR_DRV_I2C_ERR_DNACK   Negative acknowledgement (NACK) is received after sending
 *                                        a data byte in polling mode.
 */
xincx_err_t xincx_i2s_rx(xincx_i2s_t const * p_instance,
                       uint8_t            address,
                       uint8_t *          p_data,
                       size_t             length);


/**
 * @brief Function for performing a I2C transfer.
 *
 * The following transfer types can be configured (@ref xincx_i2c_xfer_desc_t::type):
 * - @ref XINCX_I2C_XFER_TXRX - Write operation followed by a read operation (without STOP condition in between).
 * - @ref XINCX_I2C_XFER_TXTX - Write operation followed by a write operation (without STOP condition in between).
 * - @ref XINCX_I2C_XFER_TX - Write operation (with or without STOP condition).
 * - @ref XINCX_I2C_XFER_RX - Read operation  (with STOP condition).
 *
 * @note TX-RX and TX-TX transfers are supported only in non-blocking mode.
 *
 * Additional options are provided using the flags parameter:
 * - @ref XINCX_I2C_FLAG_NO_XFER_EVT_HANDLER - No user event handler after transfer completion. In most cases, this also means no interrupt at the end of the transfer.
 * - @ref XINCX_I2C_FLAG_TX_NO_STOP - No stop condition after TX transfer.
 * - @ref XINCX_I2C_FLAG_SUSPEND - Transfer will be suspended. This allows for combining multiple transfers into one transaction.
 *                                Only transactions with the same direction can be combined. To finish the transaction, call the function without this flag.
 *
 * @note
 * Some flag combinations are invalid:
 * - @ref XINCX_I2C_FLAG_TX_NO_STOP with @ref xincx_i2c_xfer_desc_t::type different than @ref XINCX_I2C_XFER_TX
 *
 * @param[in] p_instance  Pointer to the driver instance structure.
 * @param[in] p_xfer_desc Pointer to the transfer descriptor.
 * @param[in] flags       Transfer options (0 for default settings).
 *
 * @retval XINCX_SUCCESS                   The procedure is successful.
 * @retval XINCX_ERROR_BUSY                The driver is not ready for a new transfer.
 * @retval XINCX_ERROR_NOT_SUPPORTED       The provided parameters are not supported.
 * @retval XINCX_ERROR_INTERNAL            An unexpected transition occurred on the bus.
 * @retval XINCX_ERROR_INVALID_STATE       Other direction of transaction is suspended on the bus.
 * @retval XINCX_ERROR_DRV_I2C_ERR_OVERRUN The unread data is replaced by new data (TXRX and RX)
 * @retval XINCX_ERROR_DRV_I2C_ERR_ANACK   Negative acknowledgement (NACK) is received after sending
 *                                        the address in polling mode.
 * @retval XINCX_ERROR_DRV_I2C_ERR_DNACK   Negative acknowledgement (NACK) is received after sending
 *                                        a data byte in polling mode.
 */
xincx_err_t xincx_i2s_xfer(xincx_i2s_t           const * p_instance,
                         xincx_i2s_xfer_desc_t const * p_xfer_desc,
                         uint32_t                     flags);

/**
 * @brief Function for checking the I2C driver state.
 *
 * @param[in] p_instance I2C instance.
 *
 * @retval true  The I2C driver is currently busy performing a transfer.
 * @retval false The I2C driver is ready for a new transfer.
 */
bool xincx_i2s_is_busy(xincx_i2s_t const * p_instance);



/** @} */


void xincx_i2s_0_irq_handler(void);


#ifdef __cplusplus
}
#endif

#endif // XINCX_I2S_H__
