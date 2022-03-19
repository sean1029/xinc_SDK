/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_UARTE_H__
#define XINCX_UARTE_H__

#include <xincx.h>
#include <hal/nrf_uarte.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_uarte UARTE driver
 * @{
 * @ingroup nrf_uarte
 * @brief   UARTE peripheral driver.
 */

/** @brief Structure for the UARTE driver instance. */
typedef struct
{
    XINC_UARTE_Type * p_reg;        ///< Pointer to a structure with UARTE registers.
    uint8_t          drv_inst_idx; ///< Index of the driver instance. For internal use only.
} xincx_uarte_t;

#ifndef __XINCX_DOXYGEN__
enum {
#if XINCX_CHECK(XINCX_UARTE0_ENABLED)
    XINCX_UARTE0_INST_IDX,
#endif
#if XINCX_CHECK(XINCX_UARTE1_ENABLED)
    XINCX_UARTE1_INST_IDX,
#endif
#if XINCX_CHECK(XINCX_UARTE2_ENABLED)
    XINCX_UARTE2_INST_IDX,
#endif
#if XINCX_CHECK(XINCX_UARTE3_ENABLED)
    XINCX_UARTE3_INST_IDX,
#endif
    XINCX_UARTE_ENABLED_COUNT
};
#endif

/** @brief Macro for creating a UARTE driver instance. */
#define XINCX_UARTE_INSTANCE(id)                               \
{                                                             \
    .p_reg        = XINCX_CONCAT_2(XINC_UARTE, id),             \
    .drv_inst_idx = XINCX_CONCAT_3(XINCX_UARTE, id, _INST_IDX), \
}

/** @brief Types of UARTE driver events. */
typedef enum
{
    XINCX_UARTE_EVT_TX_DONE, ///< Requested TX transfer completed.
    XINCX_UARTE_EVT_RX_DONE, ///< Requested RX transfer completed.
    XINCX_UARTE_EVT_ERROR,   ///< Error reported by UART peripheral.
} xincx_uarte_evt_type_t;

/** @brief Structure for the UARTE configuration. */
typedef struct
{
    uint32_t             pseltxd;            ///< TXD pin number.
    uint32_t             pselrxd;            ///< RXD pin number.
    uint32_t             pselcts;            ///< CTS pin number.
    uint32_t             pselrts;            ///< RTS pin number.
    void *               p_context;          ///< Context passed to interrupt handler.
    nrf_uarte_hwfc_t     hwfc;               ///< Flow control configuration.
    nrf_uarte_parity_t   parity;             ///< Parity configuration.
    nrf_uarte_baudrate_t baudrate;           ///< Baud rate.
    uint8_t              interrupt_priority; ///< Interrupt priority.
} xincx_uarte_config_t;

/** @brief UARTE default configuration. */
#define XINCX_UARTE_DEFAULT_CONFIG                                                   \
{                                                                                   \
    .pseltxd            = XINC_UARTE_PSEL_DISCONNECTED,                              \
    .pselrxd            = XINC_UARTE_PSEL_DISCONNECTED,                              \
    .pselcts            = XINC_UARTE_PSEL_DISCONNECTED,                              \
    .pselrts            = XINC_UARTE_PSEL_DISCONNECTED,                              \
    .p_context          = NULL,                                                     \
    .hwfc               = (nrf_uarte_hwfc_t)XINCX_UARTE_DEFAULT_CONFIG_HWFC,         \
    .parity             = (nrf_uarte_parity_t)XINCX_UARTE_DEFAULT_CONFIG_PARITY,     \
    .baudrate           = (nrf_uarte_baudrate_t)XINCX_UARTE_DEFAULT_CONFIG_BAUDRATE, \
    .interrupt_priority = XINCX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY,                   \
}

/** @brief Structure for the UARTE transfer completion event. */
typedef struct
{
    uint8_t * p_data; ///< Pointer to memory used for transfer.
    size_t    bytes;  ///< Number of bytes transfered.
} xincx_uarte_xfer_evt_t;

/** @brief Structure for UARTE error event. */
typedef struct
{
    xincx_uarte_xfer_evt_t rxtx;       ///< Transfer details, including number of bytes transferred.
    uint32_t              error_mask; ///< Mask of error flags that generated the event.
} xincx_uarte_error_evt_t;

/** @brief Structure for UARTE event. */
typedef struct
{
    xincx_uarte_evt_type_t type; ///< Event type.
    union
    {
        xincx_uarte_xfer_evt_t  rxtx;  ///< Data provided for transfer completion events.
        xincx_uarte_error_evt_t error; ///< Data provided for error event.
    } data;                           ///< Union to store event data.
} xincx_uarte_event_t;

/**
 * @brief UARTE interrupt event handler.
 *
 * @param[in] p_event   Pointer to event structure. Event is allocated on the stack so it is available
 *                      only within the context of the event handler.
 * @param[in] p_context Context passed to the interrupt handler, set on initialization.
 */
typedef void (*xincx_uarte_event_handler_t)(xincx_uarte_event_t const * p_event,
                                           void *                     p_context);

/**
 * @brief Function for initializing the UARTE driver.
 *
 * This function configures and enables UARTE. After this function GPIO pins are controlled by UARTE.
 *
 * @param[in] p_instance    Pointer to the driver instance structure.
 * @param[in] p_config      Pointer to the structure with the initial configuration.
 * @param[in] event_handler Event handler provided by the user. If not provided driver works in
 *                          blocking mode.
 *
 * @retval XINCX_SUCCESS             Initialization was successful.
 * @retval XINCX_ERROR_INVALID_STATE Driver is already initialized.
 * @retval XINCX_ERROR_BUSY          Some other peripheral with the same
 *                                  instance ID is already in use. This is
 *                                  possible only if @ref xincx_prs module
 *                                  is enabled.
 */
xincx_err_t xincx_uarte_init(xincx_uarte_t const *        p_instance,
                           xincx_uarte_config_t const * p_config,
                           xincx_uarte_event_handler_t  event_handler);

/**
 * @brief Function for uninitializing the UARTE driver.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_uarte_uninit(xincx_uarte_t const * p_instance);

/**
 * @brief Function for getting the address of the specified UARTE task.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] task       Task.
 *
 * @return Task address.
 */
__STATIC_INLINE uint32_t xincx_uarte_task_address_get(xincx_uarte_t const * p_instance,
                                                     nrf_uarte_task_t     task);

/**
 * @brief Function for getting the address of the specified UARTE event.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] event      Event.
 *
 * @return Event address.
 */
__STATIC_INLINE uint32_t xincx_uarte_event_address_get(xincx_uarte_t const * p_instance,
                                                      nrf_uarte_event_t    event);

/**
 * @brief Function for sending data over UARTE.
 *
 * If an event handler is provided in xincx_uarte_init() call, this function
 * returns immediately and the handler is called when the transfer is done.
 * Otherwise, the transfer is performed in blocking mode, that is this function
 * returns when the transfer is finished. Blocking mode is not using interrupt
 * so there is no context switching inside the function.
 *
 * @note Peripherals using EasyDMA (including UARTE) require the transfer buffers
 *       to be placed in the Data RAM region. If this condition is not met,
 *       this function will fail with the error code XINCX_ERROR_INVALID_ADDR.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] p_data     Pointer to data.
 * @param[in] length     Number of bytes to send. Maximum possible length is
 *                       dependent on the used SoC (see the MAXCNT register
 *                       description in the Product Specification). The driver
 *                       checks it with assertion.
 *
 * @retval XINCX_SUCCESS            Initialization was successful.
 * @retval XINCX_ERROR_BUSY         Driver is already transferring.
 * @retval XINCX_ERROR_FORBIDDEN    The transfer was aborted from a different context
 *                                 (blocking mode only).
 * @retval XINCX_ERROR_INVALID_ADDR p_data does not point to RAM buffer.
 */
xincx_err_t xincx_uarte_tx(xincx_uarte_t const * p_instance,
                         uint8_t const *      p_data,
                         size_t               length);

/**
 * @brief Function for checking if UARTE is currently transmitting.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 *
 * @retval true  The UARTE is transmitting.
 * @retval false The UARTE is not transmitting.
 */
bool xincx_uarte_tx_in_progress(xincx_uarte_t const * p_instance);

/**
 * @brief Function for aborting any ongoing transmission.
 * @note @ref XINCX_UARTE_EVT_TX_DONE event will be generated in non-blocking mode.
 *       It will contain number of bytes sent until the abort was called. The event
 *       handler will be called from the UARTE interrupt context.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_uarte_tx_abort(xincx_uarte_t const * p_instance);

/**
 * @brief Function for receiving data over UARTE.
 *
 * If an event handler is provided in the xincx_uarte_init() call, this function
 * returns immediately and the handler is called when the transfer is done.
 * Otherwise, the transfer is performed in blocking mode, that is this function
 * returns when the transfer is finished. Blocking mode is not using interrupt so
 * there is no context switching inside the function.
 * The receive buffer pointer is double-buffered in non-blocking mode. The secondary
 * buffer can be set immediately after starting the transfer and will be filled
 * when the primary buffer is full. The double-buffering feature allows
 * receiving data continuously.
 *
 * @note Peripherals using EasyDMA (including UARTE) require the transfer buffers
 *       to be placed in the Data RAM region. If this condition is not met,
 *       this function fails with the error code XINCX_ERROR_INVALID_ADDR.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] p_data     Pointer to data.
 * @param[in] length     Number of bytes to receive. Maximum possible length is
 *                       dependent on the used SoC (see the MAXCNT register
 *                       description in the Product Specification). The driver
 *                       checks it with assertion.
 *
 * @retval XINCX_SUCCESS            Initialization is successful.
 * @retval XINCX_ERROR_BUSY         The driver is already receiving
 *                                 (and the secondary buffer has already been set
 *                                 in non-blocking mode).
 * @retval XINCX_ERROR_FORBIDDEN    The transfer is aborted from a different context
 *                                 (blocking mode only).
 * @retval XINCX_ERROR_INTERNAL     The UARTE peripheral reports an error.
 * @retval XINCX_ERROR_INVALID_ADDR p_data does not point to RAM buffer.
 */
xincx_err_t xincx_uarte_rx(xincx_uarte_t const * p_instance,
                         uint8_t *            p_data,
                         size_t               length);



/**
 * @brief Function for testing the receiver state in blocking mode.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 *
 * @retval true  The receiver has at least one byte of data to get.
 * @retval false The receiver is empty.
 */
bool xincx_uarte_rx_ready(xincx_uarte_t const * p_instance);

/**
 * @brief Function for aborting any ongoing reception.
 * @note @ref XINCX_UARTE_EVT_RX_DONE event will be generated in non-blocking mode.
 *       It will contain number of bytes received until the abort was called. The event
 *       handler will be called from the UARTE interrupt context.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_uarte_rx_abort(xincx_uarte_t const * p_instance);

/**
 * @brief Function for reading error source mask. Mask contains values from @ref nrf_uarte_error_mask_t.
 * @note Function must be used in the blocking mode only. In case of non-blocking mode, an error event is
 *       generated. Function clears error sources after reading.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 *
 * @return Mask of reported errors.
 */
uint32_t xincx_uarte_errorsrc_get(xincx_uarte_t const * p_instance);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION
__STATIC_INLINE uint32_t xincx_uarte_task_address_get(xincx_uarte_t const * p_instance,
                                                     nrf_uarte_task_t     task)
{
    return nrf_uarte_task_address_get(p_instance->p_reg, task);
}

__STATIC_INLINE uint32_t xincx_uarte_event_address_get(xincx_uarte_t const * p_instance,
                                                      nrf_uarte_event_t    event)
{
    return nrf_uarte_event_address_get(p_instance->p_reg, event);
}
#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */


void xincx_uarte_0_irq_handler(void);
void xincx_uarte_1_irq_handler(void);
void xincx_uarte_2_irq_handler(void);
void xincx_uarte_3_irq_handler(void);


#ifdef __cplusplus
}
#endif

#endif // XINCX_UARTE_H__