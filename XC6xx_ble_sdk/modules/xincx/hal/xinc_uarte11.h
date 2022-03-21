/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_UARTE_H__
#define XINC_UARTE_H__

#include <xincx.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XINC_UARTE_PSEL_DISCONNECTED 0xFFFFFFFF

/**
 * @defgroup xinc_uarte_hal UARTE HAL
 * @{
 * @ingroup xinc_uarte
 * @brief   Hardware access layer for managing the UARTE peripheral.
 */

/** @brief UARTE tasks. */
typedef enum
{
    XINC_UARTE_TASK_STARTRX   = 0,//offsetof(XINC_UARTE_Type, TASKS_STARTRX), ///< Start UART receiver.
    XINC_UARTE_TASK_STOPRX    = 0,////offsetof(XINC_UARTE_Type, TASKS_STOPRX),  ///< Stop UART receiver.
    XINC_UARTE_TASK_STARTTX   = 0,////offsetof(XINC_UARTE_Type, TASKS_STARTTX), ///< Start UART transmitter.
    XINC_UARTE_TASK_STOPTX    = 0,////offsetof(XINC_UARTE_Type, TASKS_STOPTX),  ///< Stop UART transmitter.
    XINC_UARTE_TASK_FLUSHRX   = 0,//,//offsetof(XINC_UARTE_Type, TASKS_FLUSHRX)  ///< Flush RX FIFO in RX buffer.
} xinc_uarte_task_t;

/** @brief UARTE events. */
typedef enum
{
    XINC_UARTE_EVENT_CTS       = 0,//offsetof(XINC_UARTE_Type, EVENTS_CTS),       ///< CTS is activated.
    XINC_UARTE_EVENT_NCTS      = 0,//offsetof(XINC_UARTE_Type, EVENTS_NCTS),      ///< CTS is deactivated.
    XINC_UARTE_EVENT_RXDRDY    = 0,//offsetof(XINC_UARTE_Type, EVENTS_RXDRDY),    ///< Data received in RXD (but potentially not yet transferred to Data RAM).
    XINC_UARTE_EVENT_ENDRX     = 0,//offsetof(XINC_UARTE_Type, EVENTS_ENDRX),     ///< Receive buffer is filled up.
    XINC_UARTE_EVENT_TXDRDY    = 0,//offsetof(XINC_UARTE_Type, EVENTS_TXDRDY),    ///< Data sent from TXD.
    XINC_UARTE_EVENT_ENDTX     = 0,//offsetof(XINC_UARTE_Type, EVENTS_ENDTX),     ///< Last TX byte transmitted.
    XINC_UARTE_EVENT_ERROR     = 0,//offsetof(XINC_UARTE_Type, EVENTS_ERROR),     ///< Error detected.
    XINC_UARTE_EVENT_RXTO      = 0,//offsetof(XINC_UARTE_Type, EVENTS_RXTO),      ///< Receiver timeout.
    XINC_UARTE_EVENT_RXSTARTED = 0,//offsetof(XINC_UARTE_Type, EVENTS_RXSTARTED), ///< Receiver has started.
    XINC_UARTE_EVENT_TXSTARTED = 0,//offsetof(XINC_UARTE_Type, EVENTS_TXSTARTED), ///< Transmitter has started.
    XINC_UARTE_EVENT_TXSTOPPED = 0,//offsetof(XINC_UARTE_Type, EVENTS_TXSTOPPED)  ///< Transmitted stopped.
} xinc_uarte_event_t;

/** @brief Types of UARTE shortcuts. */
typedef enum
{
    XINC_UARTE_SHORT_ENDRX_STARTRX = UARTE_SHORTS_ENDRX_STARTRX_Msk, ///< Shortcut between ENDRX event and STARTRX task.
    XINC_UARTE_SHORT_ENDRX_STOPRX  = UARTE_SHORTS_ENDRX_STOPRX_Msk   ///< Shortcut between ENDRX event and STOPRX task.
} xinc_uarte_short_t;


/** @brief UARTE interrupts. */
typedef enum
{
    XINC_UARTE_INT_CTS_MASK       = UARTE_INTENSET_CTS_Msk,       ///< Interrupt on CTS event.
    XINC_UARTE_INT_NCTS_MASK      = UARTE_INTENSET_NCTS_Msk,      ///< Interrupt on NCTS event.
    XINC_UARTE_INT_RXDRDY_MASK    = UARTE_INTENSET_RXDRDY_Msk,    ///< Interrupt on RXDRDY event.
    XINC_UARTE_INT_ENDRX_MASK     = UARTE_INTENSET_ENDRX_Msk,     ///< Interrupt on ENDRX event.
    XINC_UARTE_INT_TXDRDY_MASK    = UARTE_INTENSET_TXDRDY_Msk,    ///< Interrupt on TXDRDY event.
    XINC_UARTE_INT_ENDTX_MASK     = UARTE_INTENSET_ENDTX_Msk,     ///< Interrupt on ENDTX event.
    XINC_UARTE_INT_ERROR_MASK     = UARTE_INTENSET_ERROR_Msk,     ///< Interrupt on ERROR event.
    XINC_UARTE_INT_RXTO_MASK      = UARTE_INTENSET_RXTO_Msk,      ///< Interrupt on RXTO event.
    XINC_UARTE_INT_RXSTARTED_MASK = UARTE_INTENSET_RXSTARTED_Msk, ///< Interrupt on RXSTARTED event.
    XINC_UARTE_INT_TXSTARTED_MASK = UARTE_INTENSET_TXSTARTED_Msk, ///< Interrupt on TXSTARTED event.
    XINC_UARTE_INT_TXSTOPPED_MASK = UARTE_INTENSET_TXSTOPPED_Msk  ///< Interrupt on TXSTOPPED event.
} xinc_uarte_int_mask_t;

/** @brief Baudrates supported by UARTE. */
typedef enum
{
    XINC_UARTE_BAUDRATE_1200    = UARTE_BAUDRATE_BAUDRATE_Baud1200,   ///< 1200 baud.
    XINC_UARTE_BAUDRATE_2400    = UARTE_BAUDRATE_BAUDRATE_Baud2400,   ///< 2400 baud.
    XINC_UARTE_BAUDRATE_4800    = UARTE_BAUDRATE_BAUDRATE_Baud4800,   ///< 4800 baud.
    XINC_UARTE_BAUDRATE_9600    = UARTE_BAUDRATE_BAUDRATE_Baud9600,   ///< 9600 baud.
    XINC_UARTE_BAUDRATE_14400   = UARTE_BAUDRATE_BAUDRATE_Baud14400,  ///< 14400 baud.
    XINC_UARTE_BAUDRATE_19200   = UARTE_BAUDRATE_BAUDRATE_Baud19200,  ///< 19200 baud.
    XINC_UARTE_BAUDRATE_28800   = UARTE_BAUDRATE_BAUDRATE_Baud28800,  ///< 28800 baud.
    XINC_UARTE_BAUDRATE_31250   = UARTE_BAUDRATE_BAUDRATE_Baud31250,  ///< 31250 baud.
    XINC_UARTE_BAUDRATE_38400   = UARTE_BAUDRATE_BAUDRATE_Baud38400,  ///< 38400 baud.
    XINC_UARTE_BAUDRATE_56000   = UARTE_BAUDRATE_BAUDRATE_Baud56000,  ///< 56000 baud.
    XINC_UARTE_BAUDRATE_57600   = UARTE_BAUDRATE_BAUDRATE_Baud57600,  ///< 57600 baud.
    XINC_UARTE_BAUDRATE_76800   = UARTE_BAUDRATE_BAUDRATE_Baud76800,  ///< 76800 baud.
    XINC_UARTE_BAUDRATE_115200  = UARTE_BAUDRATE_BAUDRATE_Baud115200, ///< 115200 baud.
    XINC_UARTE_BAUDRATE_230400  = UARTE_BAUDRATE_BAUDRATE_Baud230400, ///< 230400 baud.
    XINC_UARTE_BAUDRATE_250000  = UARTE_BAUDRATE_BAUDRATE_Baud250000, ///< 250000 baud.
    XINC_UARTE_BAUDRATE_460800  = UARTE_BAUDRATE_BAUDRATE_Baud460800, ///< 460800 baud.
    XINC_UARTE_BAUDRATE_921600  = UARTE_BAUDRATE_BAUDRATE_Baud921600, ///< 921600 baud.
    XINC_UARTE_BAUDRATE_1000000 = UARTE_BAUDRATE_BAUDRATE_Baud1M      ///< 1000000 baud.
} xinc_uarte_baudrate_t;

/** @brief Types of UARTE error masks. */
typedef enum
{
    XINC_UARTE_ERROR_OVERRUN_MASK = UARTE_ERRORSRC_OVERRUN_Msk, ///< Overrun error.
    XINC_UARTE_ERROR_PARITY_MASK  = UARTE_ERRORSRC_PARITY_Msk,  ///< Parity error.
    XINC_UARTE_ERROR_FRAMING_MASK = UARTE_ERRORSRC_FRAMING_Msk, ///< Framing error.
    XINC_UARTE_ERROR_BREAK_MASK   = UARTE_ERRORSRC_BREAK_Msk    ///< Break error.
} xinc_uarte_error_mask_t;

/** @brief Types of UARTE parity modes. */
typedef enum
{
    XINC_UARTE_PARITY_EXCLUDED = UARTE_CONFIG_PARITY_Excluded << UARTE_CONFIG_PARITY_Pos, ///< Parity excluded.
    XINC_UARTE_PARITY_INCLUDED = UARTE_CONFIG_PARITY_Included << UARTE_CONFIG_PARITY_Pos  ///< Parity included.
} xinc_uarte_parity_t;

/** @brief Types of UARTE flow control modes. */
typedef enum
{
    XINC_UARTE_HWFC_DISABLED = UARTE_CONFIG_HWFC_Disabled << UARTE_CONFIG_HWFC_Pos, ///< Hardware flow control disabled.
    XINC_UARTE_HWFC_ENABLED  = UARTE_CONFIG_HWFC_Enabled  << UARTE_CONFIG_HWFC_Pos  ///< Hardware flow control enabled.
} xinc_uarte_hwfc_t;


/**
 * @brief Function for clearing the specified UARTE event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event to clear.
 */
__STATIC_INLINE void xinc_uarte_event_clear(XINC_UARTE_Type * p_reg, xinc_uarte_event_t event);

/**
 * @brief Function for retrieving the state of the UARTE event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event to be checked.
 *
 * @retval true  The event has been generated.
 * @retval false The event has not been generated.
 */
__STATIC_INLINE bool xinc_uarte_event_check(XINC_UARTE_Type * p_reg, xinc_uarte_event_t event);

/**
 * @brief Function for returning the address of the specified UARTE event register.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event The specified event.
 *
 * @return Address of specified event register.
 */
__STATIC_INLINE uint32_t xinc_uarte_event_address_get(XINC_UARTE_Type *  p_reg,
                                                     xinc_uarte_event_t event);

/**
 * @brief Function for enabling UARTE shortcuts.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param mask  Shortcuts to be enabled.
 */
__STATIC_INLINE void xinc_uarte_shorts_enable(XINC_UARTE_Type * p_reg, uint32_t mask);

/**
 * @brief Function for disabling UARTE shortcuts.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param mask  Shortcuts to be disabled.
 */
__STATIC_INLINE void xinc_uarte_shorts_disable(XINC_UARTE_Type * p_reg, uint32_t mask);

/**
 * @brief Function for enabling UARTE interrupts.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param mask  Mask of interrupts to be enabled.
 */
__STATIC_INLINE void xinc_uarte_int_enable(XINC_UARTE_Type * p_reg, uint32_t mask);

/**
 * @brief Function for retrieving the state of the specified interrupt.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param mask  Mask of interrupts to be checked.
 *
 * @retval true  The interrupt is enabled.
 * @retval false The interrupt is not enabled.
 */
__STATIC_INLINE bool xinc_uarte_int_enable_check(XINC_UARTE_Type * p_reg, xinc_uarte_int_mask_t mask);

/**
 * @brief Function for disabling the specified interrupts.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param mask  Mask of interrupts to be disabled.
 */
__STATIC_INLINE void xinc_uarte_int_disable(XINC_UARTE_Type * p_reg, uint32_t mask);

#if defined(DPPI_PRESENT) || defined(__XINCX_DOXYGEN__)
/**
 * @brief Function for setting the subscribe configuration for a given
 *        UARTE task.
 *
 * @param[in] p_reg   Pointer to the structure of registers of the peripheral.
 * @param[in] task    Task for which to set the configuration.
 * @param[in] channel Channel through which to subscribe events.
 */
__STATIC_INLINE void xinc_uarte_subscribe_set(XINC_UARTE_Type * p_reg,
                                             xinc_uarte_task_t task,
                                             uint8_t          channel);

/**
 * @brief Function for clearing the subscribe configuration for a given
 *        UARTE task.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] task  Task for which to clear the configuration.
 */
__STATIC_INLINE void xinc_uarte_subscribe_clear(XINC_UARTE_Type * p_reg,
                                               xinc_uarte_task_t task);

/**
 * @brief Function for setting the publish configuration for a given
 *        UARTE event.
 *
 * @param[in] p_reg   Pointer to the structure of registers of the peripheral.
 * @param[in] event   Event for which to set the configuration.
 * @param[in] channel Channel through which to publish the event.
 */
__STATIC_INLINE void xinc_uarte_publish_set(XINC_UARTE_Type *  p_reg,
                                           xinc_uarte_event_t event,
                                           uint8_t           channel);

/**
 * @brief Function for clearing the publish configuration for a given
 *        UARTE event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event for which to clear the configuration.
 */
__STATIC_INLINE void xinc_uarte_publish_clear(XINC_UARTE_Type *  p_reg,
                                             xinc_uarte_event_t event);
#endif // defined(DPPI_PRESENT) || defined(__XINCX_DOXYGEN__)

/**
 * @brief Function for getting error source mask. Function is clearing error source flags after reading.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Mask with error source flags.
 */
__STATIC_INLINE uint32_t xinc_uarte_errorsrc_get_and_clear(XINC_UARTE_Type * p_reg);

/**
 * @brief Function for enabling UARTE.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_uarte_enable(XINC_UARTE_Type * p_reg);

/**
 * @brief Function for disabling UARTE.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_uarte_disable(XINC_UARTE_Type * p_reg);

/**
 * @brief Function for configuring TX/RX pins.
 *
 * @param p_reg   Pointer to the structure of registers of the peripheral.
 * @param pseltxd TXD pin number.
 * @param pselrxd RXD pin number.
 */
__STATIC_INLINE void xinc_uarte_txrx_pins_set(XINC_UARTE_Type * p_reg,
                                             uint32_t         pseltxd,
                                             uint32_t         pselrxd);

/**
 * @brief Function for disconnecting TX/RX pins.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_uarte_txrx_pins_disconnect(XINC_UARTE_Type * p_reg);

/**
 * @brief Function for getting TX pin.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return TX pin number.
 */
__STATIC_INLINE uint32_t xinc_uarte_tx_pin_get(XINC_UARTE_Type * p_reg);

/**
 * @brief Function for getting RX pin.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return RX pin number.
 */
__STATIC_INLINE uint32_t xinc_uarte_rx_pin_get(XINC_UARTE_Type * p_reg);

/**
 * @brief Function for getting RTS pin.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return RTS pin number.
 */
__STATIC_INLINE uint32_t xinc_uarte_rts_pin_get(XINC_UARTE_Type * p_reg);

/**
 * @brief Function for getting CTS pin.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return CTS pin number.
 */
__STATIC_INLINE uint32_t xinc_uarte_cts_pin_get(XINC_UARTE_Type * p_reg);

/**
 * @brief Function for configuring flow control pins.
 *
 * @param p_reg   Pointer to the structure of registers of the peripheral.
 * @param pselrts RTS pin number.
 * @param pselcts CTS pin number.
 */
__STATIC_INLINE void xinc_uarte_hwfc_pins_set(XINC_UARTE_Type * p_reg,
                                             uint32_t         pselrts,
                                             uint32_t         pselcts);

/**
 * @brief Function for disconnecting flow control pins.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_uarte_hwfc_pins_disconnect(XINC_UARTE_Type * p_reg);

/**
 * @brief Function for starting an UARTE task.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param task  Task.
 */
__STATIC_INLINE void xinc_uarte_task_trigger(XINC_UARTE_Type * p_reg, xinc_uarte_task_t task);

/**
 * @brief Function for returning the address of the specified task register.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param task  Task.
 *
 * @return Task address.
 */
__STATIC_INLINE uint32_t xinc_uarte_task_address_get(XINC_UARTE_Type * p_reg, xinc_uarte_task_t task);

/**
 * @brief Function for configuring UARTE.
 *
 * @param p_reg  Pointer to the structure of registers of the peripheral.
 * @param hwfc   Hardware flow control. Enabled if true.
 * @param parity Parity. Included if true.
 */
__STATIC_INLINE void xinc_uarte_configure(XINC_UARTE_Type   * p_reg,
                                         xinc_uarte_parity_t parity,
                                         xinc_uarte_hwfc_t   hwfc);

/**
 * @brief Function for setting UARTE baud rate.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param baudrate Baud rate.
 */
__STATIC_INLINE void xinc_uarte_baudrate_set(XINC_UARTE_Type   * p_reg, xinc_uarte_baudrate_t baudrate);

/**
 * @brief Function for setting the transmit buffer.
 *
 * @param[in] p_reg    Pointer to the structure of registers of the peripheral.
 * @param[in] p_buffer Pointer to the buffer with data to send.
 * @param[in] length   Maximum number of data bytes to transmit.
 */
__STATIC_INLINE void xinc_uarte_tx_buffer_set(XINC_UARTE_Type * p_reg,
                                             uint8_t  const * p_buffer,
                                             size_t           length);

/**
 * @brief Function for getting number of bytes transmitted in the last transaction.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval Amount of bytes transmitted.
 */
__STATIC_INLINE uint32_t xinc_uarte_tx_amount_get(XINC_UARTE_Type * p_reg);

/**
 * @brief Function for setting the receive buffer.
 *
 * @param[in] p_reg    Pointer to the structure of registers of the peripheral.
 * @param[in] p_buffer Pointer to the buffer for received data.
 * @param[in] length   Maximum number of data bytes to receive.
 */
__STATIC_INLINE void xinc_uarte_rx_buffer_set(XINC_UARTE_Type * p_reg,
                                             uint8_t *        p_buffer,
                                             size_t           length);

/**
 * @brief Function for getting number of bytes received in the last transaction.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval Amount of bytes received.
 */
__STATIC_INLINE uint32_t xinc_uarte_rx_amount_get(XINC_UARTE_Type * p_reg);

#ifndef SUPPRESS_INLINE_IMPLEMENTATION
__STATIC_INLINE void xinc_uarte_event_clear(XINC_UARTE_Type * p_reg, xinc_uarte_event_t event)
{
    *((volatile uint32_t *)((uint8_t *)p_reg + (uint32_t)event)) = 0x0UL;
#if __CORTEX_M == 0x04
    volatile uint32_t dummy = *((volatile uint32_t *)((uint8_t *)p_reg + (uint32_t)event));
    (void)dummy;
#endif
}

__STATIC_INLINE bool xinc_uarte_event_check(XINC_UARTE_Type * p_reg, xinc_uarte_event_t event)
{
    return (bool)*(volatile uint32_t *)((uint8_t *)p_reg + (uint32_t)event);
}

__STATIC_INLINE uint32_t xinc_uarte_event_address_get(XINC_UARTE_Type  * p_reg,
                                                    xinc_uarte_event_t  event)
{
    return (uint32_t)((uint8_t *)p_reg + (uint32_t)event);
}

__STATIC_INLINE void xinc_uarte_shorts_enable(XINC_UARTE_Type * p_reg, uint32_t mask)
{
    p_reg->SHORTS |= mask;
}

__STATIC_INLINE void xinc_uarte_shorts_disable(XINC_UARTE_Type * p_reg, uint32_t mask)
{
    p_reg->SHORTS &= ~(mask);
}

__STATIC_INLINE void xinc_uarte_int_enable(XINC_UARTE_Type * p_reg, uint32_t mask)
{
    p_reg->INTENSET = mask;
}

__STATIC_INLINE bool xinc_uarte_int_enable_check(XINC_UARTE_Type * p_reg, xinc_uarte_int_mask_t mask)
{
    return (bool)(p_reg->INTENSET & mask);
}

__STATIC_INLINE void xinc_uarte_int_disable(XINC_UARTE_Type * p_reg, uint32_t mask)
{
    p_reg->INTENCLR = mask;
}

#if defined(DPPI_PRESENT)
__STATIC_INLINE void xinc_uarte_subscribe_set(XINC_UARTE_Type * p_reg,
                                             xinc_uarte_task_t task,
                                             uint8_t          channel)
{
    *((volatile uint32_t *) ((uint8_t *) p_reg + (uint32_t) task + 0x80uL)) =
            ((uint32_t)channel | UARTE_SUBSCRIBE_STARTRX_EN_Msk);
}

__STATIC_INLINE void xinc_uarte_subscribe_clear(XINC_UARTE_Type * p_reg,
                                               xinc_uarte_task_t task)
{
    *((volatile uint32_t *) ((uint8_t *) p_reg + (uint32_t) task + 0x80uL)) = 0;
}

__STATIC_INLINE void xinc_uarte_publish_set(XINC_UARTE_Type *  p_reg,
                                           xinc_uarte_event_t event,
                                           uint8_t           channel)
{
    *((volatile uint32_t *) ((uint8_t *) p_reg + (uint32_t) event + 0x80uL)) =
            ((uint32_t)channel | UARTE_PUBLISH_CTS_EN_Msk);
}

__STATIC_INLINE void xinc_uarte_publish_clear(XINC_UARTE_Type *  p_reg,
                                             xinc_uarte_event_t event)
{
    *((volatile uint32_t *) ((uint8_t *) p_reg + (uint32_t) event + 0x80uL)) = 0;
}
#endif // defined(DPPI_PRESENT)

__STATIC_INLINE uint32_t xinc_uarte_errorsrc_get_and_clear(XINC_UARTE_Type * p_reg)
{
    uint32_t errsrc_mask = p_reg->ERRORSRC;
    p_reg->ERRORSRC = errsrc_mask;
    return errsrc_mask;
}

__STATIC_INLINE void xinc_uarte_enable(XINC_UARTE_Type * p_reg)
{
    p_reg->ENABLE = UARTE_ENABLE_ENABLE_Enabled;
}

__STATIC_INLINE void xinc_uarte_disable(XINC_UARTE_Type * p_reg)
{
    p_reg->ENABLE = UARTE_ENABLE_ENABLE_Disabled;
}

__STATIC_INLINE void xinc_uarte_txrx_pins_set(XINC_UARTE_Type * p_reg, uint32_t pseltxd, uint32_t pselrxd)
{
    p_reg->PSEL.TXD = pseltxd;
    p_reg->PSEL.RXD = pselrxd;
}

__STATIC_INLINE void xinc_uarte_txrx_pins_disconnect(XINC_UARTE_Type * p_reg)
{
    xinc_uarte_txrx_pins_set(p_reg, XINC_UARTE_PSEL_DISCONNECTED, XINC_UARTE_PSEL_DISCONNECTED);
}

__STATIC_INLINE uint32_t xinc_uarte_tx_pin_get(XINC_UARTE_Type * p_reg)
{
    return p_reg->PSEL.TXD;
}

__STATIC_INLINE uint32_t xinc_uarte_rx_pin_get(XINC_UARTE_Type * p_reg)
{
    return p_reg->PSEL.RXD;
}

__STATIC_INLINE uint32_t xinc_uarte_rts_pin_get(XINC_UARTE_Type * p_reg)
{
    return p_reg->PSEL.RTS;
}

__STATIC_INLINE uint32_t xinc_uarte_cts_pin_get(XINC_UARTE_Type * p_reg)
{
    return p_reg->PSEL.CTS;
}

__STATIC_INLINE void xinc_uarte_hwfc_pins_set(XINC_UARTE_Type * p_reg, uint32_t pselrts, uint32_t pselcts)
{
    p_reg->PSEL.RTS = pselrts;
    p_reg->PSEL.CTS = pselcts;
}

__STATIC_INLINE void xinc_uarte_hwfc_pins_disconnect(XINC_UARTE_Type * p_reg)
{
    xinc_uarte_hwfc_pins_set(p_reg, XINC_UARTE_PSEL_DISCONNECTED, XINC_UARTE_PSEL_DISCONNECTED);
}

__STATIC_INLINE void xinc_uarte_task_trigger(XINC_UARTE_Type * p_reg, xinc_uarte_task_t task)
{
    *((volatile uint32_t *)((uint8_t *)p_reg + (uint32_t)task)) = 0x1UL;
}

__STATIC_INLINE uint32_t xinc_uarte_task_address_get(XINC_UARTE_Type * p_reg, xinc_uarte_task_t task)
{
    return (uint32_t)p_reg + (uint32_t)task;
}

__STATIC_INLINE void xinc_uarte_configure(XINC_UARTE_Type   * p_reg,
                                         xinc_uarte_parity_t parity,
                                         xinc_uarte_hwfc_t   hwfc)
{
    p_reg->CONFIG = (uint32_t)parity | (uint32_t)hwfc;
}

__STATIC_INLINE void xinc_uarte_baudrate_set(XINC_UARTE_Type   * p_reg, xinc_uarte_baudrate_t baudrate)
{
    p_reg->BAUDRATE = baudrate;
}

__STATIC_INLINE void xinc_uarte_tx_buffer_set(XINC_UARTE_Type * p_reg,
                                             uint8_t  const * p_buffer,
                                             size_t           length)
{
    p_reg->TXD.PTR    = (uint32_t)p_buffer;
    p_reg->TXD.MAXCNT = length;
}

__STATIC_INLINE uint32_t xinc_uarte_tx_amount_get(XINC_UARTE_Type * p_reg)
{
    return p_reg->TXD.AMOUNT;
}

__STATIC_INLINE void xinc_uarte_rx_buffer_set(XINC_UARTE_Type * p_reg,
                                             uint8_t *        p_buffer,
                                             size_t           length)
{
    p_reg->RXD.PTR    = (uint32_t)p_buffer;
    p_reg->RXD.MAXCNT = length;
}

__STATIC_INLINE uint32_t xinc_uarte_rx_amount_get(XINC_UARTE_Type * p_reg)
{
    return p_reg->RXD.AMOUNT;
}
#endif //SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif //XINC_UARTE_H__