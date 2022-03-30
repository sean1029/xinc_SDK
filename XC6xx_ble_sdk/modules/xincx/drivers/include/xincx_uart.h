/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINCX_UART_H__
#define XINCX_UART_H__

#include <xincx.h>
#include <hal/xinc_uart.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_uart UART driver
 * @{
 * @ingroup xinc_uart
 * @brief   UART peripheral driver.
 */

/** @brief Data structure of the UART driver instance. */
typedef struct
{
    XINC_UART_Type * p_reg;        ///< Pointer to a structure with UART registers.
    XINC_CPR_CTL_Type * p_cpr; 		///< Pointer to a structure with CPR registers.
    uint8_t         drv_inst_idx; ///< Index of the driver instance. For internal use only.
    uint8_t         id;
} xincx_uart_t;

#ifndef __XINCX_DOXYGEN__
enum {
    #if XINCX_CHECK(XINCX_UART0_ENABLED)
    XINCX_UART0_INST_IDX ,
    #endif
    #if XINCX_CHECK(XINCX_UART1_ENABLED)
    XINCX_UART1_INST_IDX ,
    #endif
    XINCX_UART_ENABLED_COUNT,

};
#endif


/** @brief Macro for creating a UART driver instance. */
#define XINCX_UART_INSTANCE(Id)                                     \
{                                                                   \
    .p_reg          = XINCX_CONCAT_2(XINC_UART, Id),                 \
    .p_cpr          = XINC_CPR,                                     \
    .drv_inst_idx   = XINCX_CONCAT_3(XINCX_UART, Id, _INST_IDX),     \
    .id             = Id,                                           \
}

/** @brief Types of UART driver events. */
typedef enum
{
    XINCX_UART_EVT_TX_DONE, ///< Requested TX transfer completed.
    XINCX_UART_EVT_RX_READY, ///< Requested RX transfer start.
    XINCX_UART_EVT_RX_DONE, ///< Requested RX transfer completed.
    XINCX_UART_EVT_ERROR,   ///< Error reported by UART peripheral.
} xincx_uart_evt_type_t;

/** @brief Structure for the UART configuration. */
typedef struct
{
    uint32_t            pseltxd;            ///< TXD pin number.
    uint32_t            pselrxd;            ///< RXD pin number.
    uint32_t            pselcts;            ///< CTS pin number.
    uint32_t            pselrts;            ///< RTS pin number.
    void *              p_context;          ///< Context passed to interrupt handler.
    xinc_uart_hwfc_t     hwfc;               ///< Flow control configuration.
    xinc_uart_data_bits_t data_bits;             ///< configuration data bits.
    xinc_uart_stop_bits_t stop_bits;
    xinc_uart_parity_t   parity;             ///< Parity configuration.
    xinc_uart_parity_type_t parity_type;     ///< Parity configuration type odd /even. 
    xinc_uart_baudrate_t baudrate;           ///< Baud rate.
    uint8_t             interrupt_priority; ///< Interrupt priority.
    bool                use_easy_dma;
} xincx_uart_config_t;



/** @brief UART default configuration. */
#define XINCX_UART_DEFAULT_CONFIG                                                  \
{                                                                                 \
    .pseltxd            = XINC_UART_PSEL_DISCONNECTED,                             \
    .pselrxd            = XINC_UART_PSEL_DISCONNECTED,                             \
    .pselcts            = XINC_UART_PSEL_DISCONNECTED,                             \
    .pselrts            = XINC_UART_PSEL_DISCONNECTED,                             \
    .p_context          = NULL,                                                   \
    .hwfc               = (xinc_uart_hwfc_t)XINCX_UART_DEFAULT_CONFIG_HWFC,         \
    .parity             = (xinc_uart_parity_t)XINCX_UART_DEFAULT_CONFIG_PARITY,     \
    .parity_type        = (xinc_uart_parity_type_t)XINCX_UART_DEFAULT_CONFIG_PARITY_TYPE,     \
    .data_bits          = (xinc_uart_data_bits_t) XINCX_UART_DEFAULT_CONFIG_DATA_BITS,\
    .stop_bits          = (xinc_uart_stop_bits_t) XINCX_UART_DEFAULT_CONFIG_STOP_BITS,\
    .baudrate           = (xinc_uart_baudrate_t)XINCX_UART_DEFAULT_CONFIG_BAUDRATE, \
    .interrupt_priority = XINCX_UART_DEFAULT_CONFIG_IRQ_PRIORITY,                  \
}

/** @brief Structure for the UART transfer completion event. */
typedef struct
{
    uint8_t * p_data; ///< Pointer to memory used for transfer.
    uint32_t  bytes;  ///< Number of bytes transfered.
} xincx_uart_xfer_evt_t;

/** @brief Structure for the UART error event. */
typedef struct
{
    xincx_uart_xfer_evt_t rxtx;       ///< Transfer details, including number of bytes transferred.
    uint32_t             error_mask; ///< Mask of error flags that generated the event.
} xincx_uart_error_evt_t;

/** @brief Structure for the UART event. */
typedef struct
{
    xincx_uart_evt_type_t type; ///< Event type.
    union
    {
        xincx_uart_xfer_evt_t  rxtx;  ///< Data provided for transfer completion events.
        xincx_uart_error_evt_t error; ///< Data provided for error event.
    } data;                          ///< Union to store event data.
} xincx_uart_event_t;

/**
 * @brief UART interrupt event handler.
 *
 * @param[in] p_event   Pointer to event structure. Event is allocated on the stack so it is available
 *                      only within the context of the event handler.
 * @param[in] p_context Context passed to the interrupt handler, set on initialization.
 */
typedef void (*xincx_uart_event_handler_t)(xincx_uart_event_t const * p_event,
                                          void *                    p_context);

/**
 * @brief Function for initializing the UART driver.
 *
 * This function configures and enables UART. After this function GPIO pins are controlled by UART.
 *
 * @param[in] p_instance    Pointer to the driver instance structure.
 * @param[in] p_config      Pointer to the structure with the initial configuration.
 * @param[in] event_handler Event handler provided by the user. If not provided, the driver works in
 *                          blocking mode.
 *
 * @retval XINCX_SUCCESS             Initialization is successful.
 * @retval XINCX_ERROR_INVALID_STATE The driver is already initialized.
 * @retval XINCX_ERROR_BUSY          Some other peripheral with the same
 *                                  instance ID is already in use. This is
 *                                  possible only if @ref xincx_prs module
 *                                  is enabled.
 */
xincx_err_t xincx_uart_init(xincx_uart_t const *        p_instance,
                          xincx_uart_config_t const * p_config,
                          xincx_uart_event_handler_t  event_handler);

/**
 * @brief Function for uninitializing the UART driver.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_uart_uninit(xincx_uart_t const * p_instance);


/**
 * @brief Function for sending data over UART.
 *
 * If an event handler was provided in xincx_uart_init() call, this function
 * returns immediately and the handler is called when the transfer is done.
 * Otherwise, the transfer is performed in blocking mode, that is this function
 * returns when the transfer is finished. Blocking mode is not using interrupt
 * so there is no context switching inside the function.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] p_data     Pointer to data.
 * @param[in] length     Number of bytes to send.
 *
 * @retval XINCX_SUCCESS         Initialization was successful.
 * @retval XINCX_ERROR_BUSY      Driver is already transferring.
 * @retval XINCX_ERROR_FORBIDDEN The transfer was aborted from a different context
 *                              (blocking mode only).
 */
xincx_err_t xincx_uart_tx(xincx_uart_t const * p_instance,
                        uint8_t const *     p_data,
                        size_t              length);

/**
 * @brief Function for checking if UART is currently transmitting.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 *
 * @retval true  The UART is transmitting.
 * @retval false The UART is not transmitting.
 */
bool xincx_uart_tx_in_progress(xincx_uart_t const * p_instance);

/**
 * @brief Function for aborting any ongoing transmission.
 * @note @ref XINCX_UART_EVT_TX_DONE event will be generated in non-blocking mode.
 *       It will contain number of bytes sent until the abort was called. The event
 *       handler will be called from the function context.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_uart_tx_abort(xincx_uart_t const * p_instance);

/**
 * @brief Function for receiving data over UART.
 *
 * If an event handler is provided in the xincx_uart_init() call, this function
 * returns immediately and the handler is called when the transfer is done.
 * Otherwise, the transfer is performed in blocking mode, that is this function
 * returns when the transfer is finished. Blocking mode is not using interrupt so
 * there is no context switching inside the function.
 * The receive buffer pointer is double-buffered in non-blocking mode. The secondary
 * buffer can be set immediately after starting the transfer and will be filled
 * when the primary buffer is full. The double-buffering feature allows
 * receiving data continuously.
 *
 * If this function is used without a previous call to @ref xincx_uart_rx_enable, the reception
 * will be stopped on error or when the supplied buffer fills up. In both cases,
 * RX FIFO gets disabled. This means that, in case of error, the bytes that follow are lost.
 * If this xincx_uart_rx() function is used with the previous call to @ref xincx_uart_rx_enable,
 * the reception is stopped in case of error, but FIFO is still ongoing. The receiver is still
 * working, so after handling the error, an immediate repeated call to this xincx_uart_rx()
 * function with fresh data buffer will re-establish reception. To disable the receiver,
 * you must call @ref xincx_uart_rx_disable explicitly.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] p_data     Pointer to data.
 * @param[in] length     Number of bytes to receive.
 *
 * @retval    XINCX_SUCCESS         Reception is complete (in case of blocking mode) or it is
 *                                 successfully started (in case of non-blocking mode).
 * @retval    XINCX_ERROR_BUSY      The driver is already receiving
 *                                 (and the secondary buffer has already been set
 *                                 in non-blocking mode).
 * @retval    XINCX_ERROR_FORBIDDEN The transfer was aborted from a different context
 *                                 (blocking mode only, also see @ref xincx_uart_rx_disable).
 * @retval    XINCX_ERROR_INTERNAL  The UART peripheral reported an error.
 */
xincx_err_t xincx_uart_rx(xincx_uart_t const * p_instance,
                        uint8_t *           p_data,
                        size_t              length);

/**
 * @brief Function for testing the receiver state in blocking mode.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 *
 * @retval true  The receiver has at least one byte of data to get.
 * @retval false The receiver is empty.
 */
bool xincx_uart_rx_ready(xincx_uart_t const * p_instance);

/**
 * @brief Function for enabling the receiver.
 *
 * UART has a 6-byte-long RX FIFO and it is used to store incoming data. If a user does not call the
 * UART receive function before the FIFO is filled, an overrun error will appear. The receiver must be
 * explicitly closed by the user @sa xincx_uart_rx_disable.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_uart_rx_enable(xincx_uart_t const * p_instance);

/**
 * @brief Function for disabling the receiver.
 *
 * This function must be called to close the receiver after it has been explicitly enabled by
 * @sa xincx_uart_rx_enable.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_uart_rx_disable(xincx_uart_t const * p_instance);

/**
 * @brief Function for aborting any ongoing reception.
 * @note @ref XINCX_UART_EVT_TX_DONE event will be generated in non-blocking mode.
 *       It will contain number of bytes received until the abort was called. The event
 *       handler will be called from the UART interrupt context.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_uart_rx_abort(xincx_uart_t const * p_instance);

/**
 * @brief Function for reading error source mask. Mask contains values from @ref xinc_uart_error_mask_t.
 * @note Function must be used in blocking mode only. In case of non-blocking mode, an error event is
 *       generated. Function clears error sources after reading.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 *
 * @return Mask of reported errors.
 */
uint32_t xincx_uart_errorsrc_get(xincx_uart_t const * p_instance);


/** @} */


void xincx_uart_0_irq_handler(void);

void xincx_uart_1_irq_handler(void);
#ifdef __cplusplus
}
#endif

#endif // XINCX_UART_H__
