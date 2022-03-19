/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef NRF_DRV_I2C_H__
#define NRF_DRV_I2C_H__

#include <xincx.h>
#ifdef I2CM_PRESENT
    #include <xincx_i2cm.h>
#else
    // Compilers (at least the smart ones) will remove the I2CM related code
    // (blocks starting with "if (NRF_DRV_I2C_USE_I2CM)") when it is not used,
    // but to perform the compilation they need the following definitions.
    #define xincx_i2cm_init(...)                 0
    #define xincx_i2cm_uninit(...)
    #define xincx_i2cm_enable(...)
    #define xincx_i2cm_disable(...)
    #define xincx_i2cm_tx(...)                   0
    #define xincx_i2cm_rx(...)                   0
    #define xincx_i2cm_is_busy(...)              0
#endif

#ifdef I2C_PRESENT
    #include <xincx_i2c.h>
#else
    // Compilers (at least the smart ones) will remove the I2C related code
    // (blocks starting with "if (NRF_DRV_I2C_USE_I2C)") when it is not used,
    // but to perform the compilation they need the following definitions.
    #define xincx_i2c_init(...)                  0
    #define xincx_i2c_uninit(...)
    #define xincx_i2c_enable(...)
    #define xincx_i2c_disable(...)
    #define xincx_i2c_tx(...)                    0
    #define xincx_i2c_rx(...)                    0
    #define xincx_i2c_is_busy(...)               0

    // This part is for old modules that use directly I2C HAL definitions
    // (to make them compilable for chips that have only I2CM).
    #define NRF_I2C_ERROR_ADDRESS_NACK  NRF_I2CM_ERROR_ADDRESS_NACK
    #define NRF_I2C_ERROR_DATA_NACK     NRF_I2CM_ERROR_DATA_NACK
    #define NRF_I2C_FREQ_100K           NRF_I2CM_FREQ_100K
    #define NRF_I2C_FREQ_250K           NRF_I2CM_FREQ_250K
    #define NRF_I2C_FREQ_400K           NRF_I2CM_FREQ_400K
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_drv_i2c I2C driver - legacy layer
 * @{
 * @ingroup  nrf_i2c
 * @brief    Layer providing compatibility with the former API.
 */

/**
 * @brief Structure for the I2C master driver instance.
 */
typedef struct
{
    uint8_t inst_idx;
    union
    {
#ifdef I2CM_PRESENT
        xincx_i2cm_t i2cm;
#endif
#ifdef I2C_PRESENT
        xincx_i2c_t  i2c;
#endif
    } u;
    bool    use_easy_dma;
} xinc_drv_i2c_t;

/**
 * @brief Macro for creating a I2C master driver instance.
 */
#define XINC_DRV_I2C_INSTANCE(id)    XINC_DRV_I2C_INSTANCE_(id)
#define XINC_DRV_I2C_INSTANCE_(id)   XINC_DRV_I2C_INSTANCE_ ## id
#if XINCX_CHECK(XINCX_I2CM0_ENABLED)
    #define XINC_DRV_I2C_INSTANCE_0 \
        { 0, { .i2cm = XINCX_I2CM_INSTANCE(0) }, true }
#elif XINCX_CHECK(XINCX_I2C0_ENABLED)
    #define XINC_DRV_I2C_INSTANCE_0 \
        { 0, { .i2c = XINC_I2C_INSTANCE(0) }, false }
#endif

/**
 * @brief I2C master clock frequency.
 */
typedef enum
{
    NRF_DRV_I2C_FREQ_100K = XINC_I2C_FREQ_100K , ///< 100 kbps.
    NRF_DRV_I2C_FREQ_400K = XINC_I2C_FREQ_400K   ///< 400 kbps.
} xinc_drv_i2c_frequency_t;

/**
 * @brief Structure for the I2C master driver instance configuration.
 */
typedef struct
{
    uint32_t                scl;                 ///< SCL pin number.
    uint32_t                sda;                 ///< SDA pin number.
    xinc_drv_i2c_frequency_t frequency;           ///< I2C frequency.
    uint8_t                 interrupt_priority;  ///< Interrupt priority.
    bool                    clear_bus_init;      ///< Clear bus during init.
    bool                    hold_bus_uninit;     ///< Hold pull up state on gpio pins after uninit.
} xinc_drv_i2c_config_t;

/**
 * @brief I2C master driver instance default configuration.
 */
#define NRF_DRV_I2C_DEFAULT_CONFIG                                               \
{                                                                                \
    .frequency          = (xinc_drv_i2c_frequency_t)I2C_DEFAULT_CONFIG_FREQUENCY, \
    .scl                = 31,                                                    \
    .sda                = 31,                                                    \
    .interrupt_priority = I2C_DEFAULT_CONFIG_IRQ_PRIORITY,                       \
    .clear_bus_init     = I2C_DEFAULT_CONFIG_CLR_BUS_INIT,                       \
    .hold_bus_uninit    = I2C_DEFAULT_CONFIG_HOLD_BUS_UNINIT,                    \
}

#define NRF_DRV_I2C_FLAG_TX_POSTINC          (1UL << 0) /**< TX buffer address incremented after transfer. */
#define NRF_DRV_I2C_FLAG_RX_POSTINC          (1UL << 1) /**< RX buffer address incremented after transfer. */
#define NRF_DRV_I2C_FLAG_NO_XFER_EVT_HANDLER (1UL << 2) /**< Interrupt after each transfer is suppressed, and the event handler is not called. */
#define NRF_DRV_I2C_FLAG_HOLD_XFER           (1UL << 3) /**< Set up the transfer but do not start it. */
#define NRF_DRV_I2C_FLAG_REPEATED_XFER       (1UL << 4) /**< Flag indicating that the transfer will be executed multiple times. */
#define NRF_DRV_I2C_FLAG_TX_NO_STOP          (1UL << 5) /**< Flag indicating that the TX transfer will not end with a stop condition. */

/**
 * @brief I2C master driver event types.
 */
typedef enum
{
    NRF_DRV_I2C_EVT_DONE,         ///< Transfer completed event.
    NRF_DRV_I2C_EVT_ADDRESS_NACK, ///< Error event: NACK received after sending the address.
    NRF_DRV_I2C_EVT_DATA_NACK     ///< Error event: NACK received after sending a data byte.
} xinc_drv_i2c_evt_type_t;

/**
 * @brief I2C master driver transfer types.
 */
typedef enum
{
    NRF_DRV_I2C_XFER_TX,          ///< TX transfer.
    NRF_DRV_I2C_XFER_RX,          ///< RX transfer.
    NRF_DRV_I2C_XFER_TXRX,        ///< TX transfer followed by RX transfer with repeated start.
    NRF_DRV_I2C_XFER_TXTX         ///< TX transfer followed by TX transfer with repeated start.
} xinc_drv_i2c_xfer_type_t;

/**
 * @brief Structure for a I2C transfer descriptor.
 */
typedef struct
{
    xinc_drv_i2c_xfer_type_t type;             ///< Type of transfer.
    uint8_t                 address;          ///< Slave address.
    uint8_t                 primary_length;   ///< Number of bytes transferred.
    uint8_t                 secondary_length; ///< Number of bytes transferred.
    uint8_t *               p_primary_buf;    ///< Pointer to transferred data.
    uint8_t *               p_secondary_buf;  ///< Pointer to transferred data.
} xinc_drv_i2c_xfer_desc_t;


/**@brief Macro for setting the TX transfer descriptor. */
#define NRF_DRV_I2C_XFER_DESC_TX(addr, p_data, length)                 \
    {                                                                  \
        .type = NRF_DRV_I2C_XFER_TX,                                   \
        .address = addr,                                               \
        .primary_length = length,                                      \
        .p_primary_buf  = p_data,                                      \
    }

/**@brief Macro for setting the RX transfer descriptor. */
#define NRF_DRV_I2C_XFER_DESC_RX(addr, p_data, length)                 \
    {                                                                  \
        .type = NRF_DRV_I2C_XFER_RX,                                   \
        .address = addr,                                               \
        .primary_length = length,                                      \
        .p_primary_buf  = p_data,                                      \
    }

/**@brief Macro for setting the TXRX transfer descriptor. */
#define NRF_DRV_I2C_XFER_DESC_TXRX(addr, p_tx, tx_len, p_rx, rx_len)   \
    {                                                                  \
        .type = NRF_DRV_I2C_XFER_TXRX,                                 \
        .address = addr,                                               \
        .primary_length   = tx_len,                                    \
        .secondary_length = rx_len,                                    \
        .p_primary_buf    = p_tx,                                      \
        .p_secondary_buf  = p_rx,                                      \
    }

/**@brief Macro for setting the TXTX transfer descriptor. */
#define NRF_DRV_I2C_XFER_DESC_TXTX(addr, p_tx, tx_len, p_tx2, tx_len2) \
    {                                                                  \
        .type = NRF_DRV_I2C_XFER_TXTX,                                 \
        .address = addr,                                               \
        .primary_length   = tx_len,                                    \
        .secondary_length = tx_len2,                                   \
        .p_primary_buf    = p_tx,                                      \
        .p_secondary_buf  = p_tx2,                                     \
    }

/**
 * @brief Structure for a I2C event.
 */
typedef struct
{
    xinc_drv_i2c_evt_type_t  type;      ///< Event type.
    xinc_drv_i2c_xfer_desc_t xfer_desc; ///< Transfer details.
} xinc_drv_i2c_evt_t;

/**
 * @brief I2C event handler prototype.
 */
typedef void (* xinc_drv_i2c_evt_handler_t)(xinc_drv_i2c_evt_t const * p_event,
                                           void *                    p_context);

/**
 * @brief Function for initializing the I2C driver instance.
 *
 * @param[in] p_instance      Pointer to the driver instance structure.
 * @param[in] p_config        Initial configuration.
 * @param[in] event_handler   Event handler provided by the user. If NULL, blocking mode is enabled.
 * @param[in] p_context       Context passed to event handler.
 *
 * @retval NRF_SUCCESS             If initialization was successful.
 * @retval NRF_ERROR_INVALID_STATE If the driver is in invalid state.
 * @retval NRF_ERROR_BUSY          If some other peripheral with the same
 *                                 instance ID is already in use. This is
 *                                 possible only if PERIPHERAL_RESOURCE_SHARING_ENABLED
 *                                 is set to a value other than zero.
 */
ret_code_t xinc_drv_i2c_init(xinc_drv_i2c_t const *        p_instance,
                            xinc_drv_i2c_config_t const * p_config,
                            xinc_drv_i2c_evt_handler_t    event_handler,
                            void *                       p_context);

/**
 * @brief Function for uninitializing the I2C instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
__STATIC_INLINE
void xinc_drv_i2c_uninit(xinc_drv_i2c_t const * p_instance);

/**
 * @brief Function for enabling the I2C instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
__STATIC_INLINE
void xinc_drv_i2c_enable(xinc_drv_i2c_t const * p_instance);

/**
 * @brief Function for disabling the I2C instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
__STATIC_INLINE
void xinc_drv_i2c_disable(xinc_drv_i2c_t const * p_instance);

/**
 * @brief Function for sending data to a I2C slave.
 *
 * The transmission will be stopped when an error occurs. If a transfer is ongoing,
 * the function returns the error code @ref NRF_ERROR_BUSY.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] address    Address of a specific slave device (only 7 LSB).
 * @param[in] p_data     Pointer to a transmit buffer.
 * @param[in] length     Number of bytes to send.
 * @param[in] no_stop    If set, the stop condition is not generated on the bus
 *                       after the transfer has completed successfully (allowing
 *                       for a repeated start in the next transfer).
 *
 * @retval NRF_SUCCESS                  If the procedure was successful.
 * @retval NRF_ERROR_BUSY               If the driver is not ready for a new transfer.
 * @retval NRF_ERROR_INTERNAL           If an error was detected by hardware.
 * @retval NRF_ERROR_INVALID_ADDR       If the EasyDMA is used and memory adress in not in RAM.
 * @retval NRF_ERROR_DRV_I2C_ERR_ANACK  If NACK received after sending the address in polling mode.
 * @retval NRF_ERROR_DRV_I2C_ERR_DNACK  If NACK received after sending a data byte in polling mode.
 */
__STATIC_INLINE
ret_code_t xinc_drv_i2c_tx(xinc_drv_i2c_t const * p_instance,
                          uint8_t               address,
                          uint8_t const *       p_data,
                          uint16_t               length,
                          bool                  no_stop);

/**
 * @brief Function for reading data from a I2C slave.
 *
 * The transmission will be stopped when an error occurs. If a transfer is ongoing,
 * the function returns the error code @ref NRF_ERROR_BUSY.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] address    Address of a specific slave device (only 7 LSB).
 * @param[in] p_data     Pointer to a receive buffer.
 * @param[in] length     Number of bytes to be received.
 *
 * @retval NRF_SUCCESS                    If the procedure was successful.
 * @retval NRF_ERROR_BUSY                 If the driver is not ready for a new transfer.
 * @retval NRF_ERROR_INTERNAL             If an error was detected by hardware.
 * @retval NRF_ERROR_DRV_I2C_ERR_OVERRUN  If the unread data was replaced by new data
 * @retval NRF_ERROR_DRV_I2C_ERR_ANACK    If NACK received after sending the address in polling mode.
 * @retval NRF_ERROR_DRV_I2C_ERR_DNACK    If NACK received after sending a data byte in polling mode.
 */
__STATIC_INLINE
ret_code_t xinc_drv_i2c_rx(xinc_drv_i2c_t const * p_instance,
                          uint8_t               address,
                          uint8_t *             p_data,
                          uint16_t               length);

/**
 * @brief Function for preparing a I2C transfer.
 *
 * The following transfer types can be configured (@ref xinc_drv_i2c_xfer_desc_t::type):
 * - @ref NRF_DRV_I2C_XFER_TXRX<span></span>: Write operation followed by a read operation (without STOP condition in between).
 * - @ref NRF_DRV_I2C_XFER_TXTX<span></span>: Write operation followed by a write operation (without STOP condition in between).
 * - @ref NRF_DRV_I2C_XFER_TX<span></span>:   Write operation (with or without STOP condition).
 * - @ref NRF_DRV_I2C_XFER_RX<span></span>:   Read operation  (with STOP condition).
 *
 * Additional options are provided using the flags parameter:
 * - @ref NRF_DRV_I2C_FLAG_TX_POSTINC and @ref NRF_DRV_I2C_FLAG_RX_POSTINC<span></span>: Post-incrementation of buffer addresses. Supported only by I2CM.
 * - @ref NRF_DRV_I2C_FLAG_NO_XFER_EVT_HANDLER<span></span>: No user event handler after transfer completion. In most cases, this also means no interrupt at the end of the transfer.
 * - @ref NRF_DRV_I2C_FLAG_HOLD_XFER<span></span>: Driver is not starting the transfer. Use this flag if the transfer is triggered externally by PPI. Supported only by I2CM.
 *   Use @ref xinc_drv_i2c_start_task_get to get the address of the start task.
 * - @ref NRF_DRV_I2C_FLAG_REPEATED_XFER<span></span>: Prepare for repeated transfers. You can set up a number of transfers that will be triggered externally (for example by PPI).
 *   An example is a TXRX transfer with the options @ref NRF_DRV_I2C_FLAG_RX_POSTINC, @ref NRF_DRV_I2C_FLAG_NO_XFER_EVT_HANDLER, and @ref NRF_DRV_I2C_FLAG_REPEATED_XFER.
 *   After the transfer is set up, a set of transfers can be triggered by PPI that will read, for example, the same register of an
 *   external component and put it into a RAM buffer without any interrupts. @ref xinc_drv_i2c_stopped_event_get can be used to get the
 *   address of the STOPPED event, which can be used to count the number of transfers. If @ref NRF_DRV_I2C_FLAG_REPEATED_XFER is used,
 *   the driver does not set the driver instance into busy state, so you must ensure that the next transfers are set up
 *   when I2CM is not active. Supported only by I2CM.
 * - @ref NRF_DRV_I2C_FLAG_TX_NO_STOP<span></span>: No stop condition after TX transfer.
 *
 * @note
 * Some flag combinations are invalid:
 * - @ref NRF_DRV_I2C_FLAG_TX_NO_STOP with @ref xinc_drv_i2c_xfer_desc_t::type different than @ref NRF_DRV_I2C_XFER_TX
 * - @ref NRF_DRV_I2C_FLAG_REPEATED_XFER with @ref xinc_drv_i2c_xfer_desc_t::type set to @ref NRF_DRV_I2C_XFER_TXTX
 *
 * If @ref xinc_drv_i2c_xfer_desc_t::type is set to @ref NRF_DRV_I2C_XFER_TX and the @ref NRF_DRV_I2C_FLAG_TX_NO_STOP and @ref NRF_DRV_I2C_FLAG_REPEATED_XFER
 * flags are set, two tasks must be used to trigger a transfer: TASKS_RESUME followed by TASKS_STARTTX. If no stop condition is generated,
 * I2CM is in SUSPENDED state. Therefore, it must be resumed before the transfer can be started.
 *
 * @note
 * This function should be used only if the instance is configured to work in non-blocking mode. If the function is used in blocking mode, the driver asserts.
 * @note If you are using this function with I2C, the only supported flag is @ref NRF_DRV_I2C_FLAG_TX_NO_STOP. All other flags require I2CM.
  *
 * @param[in] p_instance        Pointer to the driver instance structure.
 * @param[in] p_xfer_desc       Pointer to the transfer descriptor.
 * @param[in] flags             Transfer options (0 for default settings).
 *
 * @retval NRF_SUCCESS                    If the procedure was successful.
 * @retval NRF_ERROR_BUSY                 If the driver is not ready for a new transfer.
 * @retval NRF_ERROR_NOT_SUPPORTED        If the provided parameters are not supported.
 * @retval NRF_ERROR_INTERNAL             If an error was detected by hardware.
 * @retval NRF_ERROR_INVALID_ADDR         If the EasyDMA is used and memory adress in not in RAM
 * @retval NRF_ERROR_DRV_I2C_ERR_OVERRUN  If the unread data was replaced by new data (TXRX and RX)
 * @retval NRF_ERROR_DRV_I2C_ERR_ANACK    If NACK received after sending the address.
 * @retval NRF_ERROR_DRV_I2C_ERR_DNACK    If NACK received after sending a data byte.
 */
__STATIC_INLINE
ret_code_t xinc_drv_i2c_xfer(xinc_drv_i2c_t           const * p_instance,
                            xinc_drv_i2c_xfer_desc_t const * p_xfer_desc,
                            uint32_t                        flags);

/**
 * @brief Function for checking the I2C driver state.
 *
 * @param[in] p_instance I2C instance.
 *
 * @retval true  If the I2C driver is currently busy performing a transfer.
 * @retval false If the I2C driver is ready for a new transfer.
 */
__STATIC_INLINE
bool xinc_drv_i2c_is_busy(xinc_drv_i2c_t const * p_instance);




#ifndef SUPPRESS_INLINE_IMPLEMENTATION

#if defined(I2C_PRESENT) && !defined(I2CM_PRESENT)
#define NRF_DRV_I2C_WITH_I2C
#elif !defined(I2C_PRESENT) && defined(I2CM_PRESENT)
#define NRF_DRV_I2C_WITH_I2CM
#else
#if (XINCX_CHECK(I2C0_ENABLED) && !XINCX_CHECK(I2C0_USE_EASY_DMA))
    #define NRF_DRV_I2C_WITH_I2C
#endif
#endif

#if defined(NRF_DRV_I2C_WITH_I2CM) && defined(NRF_DRV_I2C_WITH_I2C)
    #define NRF_DRV_I2C_USE_I2CM  (p_instance->use_easy_dma)
#elif defined(NRF_DRV_I2C_WITH_I2CM)
    #define NRF_DRV_I2C_USE_I2CM  true
#else
    #define NRF_DRV_I2C_USE_I2CM  false
#endif
#define NRF_DRV_I2C_USE_I2C  (!NRF_DRV_I2C_USE_I2CM)

__STATIC_INLINE
void xinc_drv_i2c_uninit(xinc_drv_i2c_t const * p_instance)
{
    if (NRF_DRV_I2C_USE_I2CM)
    {
        xincx_i2cm_uninit(&p_instance->u.i2cm);
    }
    else if (NRF_DRV_I2C_USE_I2C)
    {
        xincx_i2c_uninit(&p_instance->u.i2c);
    }
}

__STATIC_INLINE
void xinc_drv_i2c_enable(xinc_drv_i2c_t const * p_instance)
{
    if (NRF_DRV_I2C_USE_I2CM)
    {
        xincx_i2cm_enable(&p_instance->u.i2cm);
    }
    else if (NRF_DRV_I2C_USE_I2C)
    {
        xincx_i2c_enable(&p_instance->u.i2c);
    }
}

__STATIC_INLINE
void xinc_drv_i2c_disable(xinc_drv_i2c_t const * p_instance)
{
    if (NRF_DRV_I2C_USE_I2CM)
    {
        xincx_i2cm_disable(&p_instance->u.i2cm);
    }
    else if (NRF_DRV_I2C_USE_I2C)
    {
        xincx_i2c_disable(&p_instance->u.i2c);
    }
}

__STATIC_INLINE
ret_code_t xinc_drv_i2c_tx(xinc_drv_i2c_t const * p_instance,
                          uint8_t               address,
                          uint8_t const *       p_data,
                          uint16_t               length,
                          bool                  no_stop)
{
    ret_code_t result = 0;
    if (NRF_DRV_I2C_USE_I2CM)
    {
        result = xincx_i2cm_tx(&p_instance->u.i2cm,
                                address, p_data, length, no_stop);
    }
    else if (NRF_DRV_I2C_USE_I2C)
    {
        result = xincx_i2c_tx(&p_instance->u.i2c,
                               address, p_data, length, no_stop);
    }
		
		
    return result;
}

__STATIC_INLINE
ret_code_t xinc_drv_i2c_rx(xinc_drv_i2c_t const * p_instance,
                          uint8_t               address,
                          uint8_t *             p_data,
                          uint16_t               length)
{
    ret_code_t result = 0;
    if (NRF_DRV_I2C_USE_I2CM)
    {
        result = xincx_i2cm_rx(&p_instance->u.i2cm,
                                address, p_data, length);
    }
    else if (NRF_DRV_I2C_USE_I2C)
    {
        result = xincx_i2c_rx(&p_instance->u.i2c,
                               address, p_data, length);
    }
    return result;
}

__STATIC_INLINE
ret_code_t xinc_drv_i2c_xfer(xinc_drv_i2c_t           const * p_instance,
                            xinc_drv_i2c_xfer_desc_t const * p_xfer_desc,
                            uint32_t                        flags)
{
    ret_code_t result = 0;
    if (NRF_DRV_I2C_USE_I2CM)
    {
    #ifdef I2CM_PRESENT
        xincx_i2cm_xfer_desc_t const i2cm_xfer_desc =
        {
            .type             = (xincx_i2cm_xfer_type_t)p_xfer_desc->type,
            .address          = p_xfer_desc->address,
            .primary_length   = p_xfer_desc->primary_length,
            .secondary_length = p_xfer_desc->secondary_length,
            .p_primary_buf    = p_xfer_desc->p_primary_buf,
            .p_secondary_buf  = p_xfer_desc->p_secondary_buf,
        };
        result = xincx_i2cm_xfer(&p_instance->u.i2cm, &i2cm_xfer_desc, flags);
    #endif
    }
    else if (NRF_DRV_I2C_USE_I2C)
    {
    #ifdef I2C_PRESENT
        xincx_i2c_xfer_desc_t const i2c_xfer_desc =
        {
            .type             = (xincx_i2c_xfer_type_t)p_xfer_desc->type,
            .address          = p_xfer_desc->address,
            .primary_length   = p_xfer_desc->primary_length,
            .secondary_length = p_xfer_desc->secondary_length,
            .p_primary_buf    = p_xfer_desc->p_primary_buf,
            .p_secondary_buf  = p_xfer_desc->p_secondary_buf,
        };
        result = xincx_i2c_xfer(&p_instance->u.i2c, &i2c_xfer_desc, flags);
    #endif
    }
    return result;
}




#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // NRF_DRV_I2C_H__
