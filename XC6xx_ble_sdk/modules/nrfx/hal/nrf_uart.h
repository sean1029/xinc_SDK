/**
 * Copyright (c) 2015 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef NRF_UART_H__
#define NRF_UART_H__

#include <nrfx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrf_uart_hal UART HAL
 * @{
 * @ingroup nrf_uart
 * @brief   Hardware access layer for managing the UART peripheral.
 */

/** @brief Pin disconnected value. */
#define NRF_UART_PSEL_DISCONNECTED 0xFFFFFFFF


/** @brief UART events. */
typedef enum
{
    NRF_UART_EVENT_CTS    = 0,//offsetof(NRF_UART_Type, EVENTS_CTS),   /**< Event from CTS line activation. */
    NRF_UART_EVENT_NCTS   = 1,//offsetof(NRF_UART_Type, EVENTS_NCTS),  /**< Event from CTS line deactivation. */
    NRF_UART_EVENT_RXDRDY = 2,//offsetof(NRF_UART_Type, EVENTS_RXDRDY),/**< Event from data ready in RXD. */
    NRF_UART_EVENT_TXDRDY = 3,//offsetof(NRF_UART_Type, EVENTS_TXDRDY),/**< Event from data sent from TXD. */
    NRF_UART_EVENT_ERROR  = 4,//offsetof(NRF_UART_Type, EVENTS_ERROR), /**< Event from error detection. */
    NRF_UART_EVENT_RXTO   = 5,//offsetof(NRF_UART_Type, EVENTS_RXTO)   /**< Event from receiver timeout. */
} nrf_uart_event_t;

/** @brief UART interrupts. */
typedef enum
{
    NRF_UART_INT_MASK_CTS    = UART_INTENCLR_CTS_Msk,    /**< CTS line activation interrupt. */
    NRF_UART_INT_MASK_NCTS   = UART_INTENCLR_NCTS_Msk,   /**< CTS line deactivation interrupt. */
    NRF_UART_INT_MASK_RXDRDY = UART_INTENCLR_RXDRDY_Msk, /**< Data ready in RXD interrupt. */
    NRF_UART_INT_MASK_TXDRDY = UART_INTENCLR_TXDRDY_Msk, /**< Data sent from TXD interrupt. */
    NRF_UART_INT_MASK_ERROR  = UART_INTENCLR_ERROR_Msk,  /**< Error detection interrupt. */
    NRF_UART_INT_MASK_RXTO   = UART_INTENCLR_RXTO_Msk    /**< Receiver timeout interrupt. */
} nrf_uart_int_mask_t;

/** @brief Baudrates supported by UART. */
typedef enum
{
    NRF_UART_BAUDRATE_2400    = UART_BAUDRATE_BAUDRATE_Baud2400,   /**< 2400 baud. */
    NRF_UART_BAUDRATE_4800    = UART_BAUDRATE_BAUDRATE_Baud4800,   /**< 4800 baud. */
    NRF_UART_BAUDRATE_9600    = UART_BAUDRATE_BAUDRATE_Baud9600,   /**< 9600 baud. */
    NRF_UART_BAUDRATE_14400   = UART_BAUDRATE_BAUDRATE_Baud14400,  /**< 14400 baud. */
    NRF_UART_BAUDRATE_19200   = UART_BAUDRATE_BAUDRATE_Baud19200,  /**< 19200 baud. */

    NRF_UART_BAUDRATE_38400   = UART_BAUDRATE_BAUDRATE_Baud38400,  /**< 38400 baud. */

    NRF_UART_BAUDRATE_57600   = UART_BAUDRATE_BAUDRATE_Baud57600,  /**< 57600 baud. */

    NRF_UART_BAUDRATE_115200  = UART_BAUDRATE_BAUDRATE_Baud115200, /**< 115200 baud. */
    NRF_UART_BAUDRATE_230400  = UART_BAUDRATE_BAUDRATE_Baud230400, /**< 230400 baud. */

    NRF_UART_BAUDRATE_460800  = UART_BAUDRATE_BAUDRATE_Baud460800, /**< 460800 baud. */
    NRF_UART_BAUDRATE_921600  = UART_BAUDRATE_BAUDRATE_Baud921600, /**< 921600 baud. */
    NRF_UART_BAUDRATE_1000000 = UART_BAUDRATE_BAUDRATE_Baud1M,     /**< 1000000 baud. */
} nrf_uart_baudrate_t;

/** @brief Types of UART error masks. */
typedef enum
{
    NRF_UART_ERROR_OVERRUN_MASK = UART_ERRORSRC_OVERRUN_Msk,   /**< Overrun error. */
    NRF_UART_ERROR_PARITY_MASK  = UART_ERRORSRC_PARITY_Msk,    /**< Parity error. */
    NRF_UART_ERROR_FRAMING_MASK = UART_ERRORSRC_FRAMING_Msk,   /**< Framing error. */
    NRF_UART_ERROR_BREAK_MASK   = UART_ERRORSRC_BREAK_Msk,     /**< Break error. */
} nrf_uart_error_mask_t;

/** @brief Types of UART parity modes. */
typedef enum
{
    NRF_UART_PARITY_EXCLUDED = UART_CONFIG_PARITY_Excluded << UART_CONFIG_PARITY_Pos, /**< Parity excluded. */
    NRF_UART_PARITY_INCLUDED = UART_CONFIG_PARITY_Included << UART_CONFIG_PARITY_Pos, /**< Parity included. */
} nrf_uart_parity_t;

/** @brief Types of UART flow control modes. */
typedef enum
{
    NRF_UART_HWFC_DISABLED = UART_CONFIG_HWFC_Disabled, /**< Hardware flow control disabled. */
    NRF_UART_HWFC_ENABLED  = UART_CONFIG_HWFC_Enabled,  /**< Hardware flow control enabled. */
} nrf_uart_hwfc_t;




/**
 * @brief Function for enabling the specified interrupt.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param mask  Mask of interrupts to be enabled.
 */
__STATIC_INLINE void nrf_uart_int_enable(NRF_UART_Type * p_reg, uint32_t mask);

/**
 * @brief Function for retrieving the state of a given interrupt.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param mask  Mask of interrupts to be checked.
 *
 * @retval true  The interrupt is enabled.
 * @retval false The interrupt is not enabled.
 */
__STATIC_INLINE bool nrf_uart_int_enable_check(NRF_UART_Type * p_reg, uint32_t mask);

/**
 * @brief Function for disabling the specified interrupts.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param mask  Mask of interrupts to be disabled.
 */
__STATIC_INLINE void nrf_uart_int_disable(NRF_UART_Type * p_reg, uint32_t mask);

/**
 * @brief Function for getting error source mask. Function is clearing error source flags after reading.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Mask with error source flags.
 */
__STATIC_INLINE uint32_t nrf_uart_errorsrc_get_and_clear(NRF_UART_Type * p_reg);

/**
 * @brief Function for enabling UART.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void nrf_uart_enable(NRF_UART_Type * p_reg);

/**
 * @brief Function for disabling UART.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void nrf_uart_disable(NRF_UART_Type * p_reg);


/**
 * @brief Function for reading RX data.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Received byte.
 */
__STATIC_INLINE uint8_t nrf_uart_rxd_get(NRF_UART_Type * p_reg);

/**
 * @brief Function for setting Tx data.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param txd   Byte.
 */
__STATIC_INLINE void nrf_uart_txd_set(NRF_UART_Type * p_reg, uint8_t txd);




/**
 * @brief Function for setting UART baud rate.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param baudrate Baud rate.
 */
__STATIC_INLINE void nrf_uart_baudrate_set(NRF_UART_Type * p_reg, nrf_uart_baudrate_t baudrate);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION

__STATIC_INLINE void nrf_uart_event_clear(NRF_UART_Type * p_reg, nrf_uart_event_t event)
{

}

__STATIC_INLINE bool nrf_uart_event_check(NRF_UART_Type * p_reg, nrf_uart_event_t event)
{
		bool event_has = false;
		switch(event)
		{
			case NRF_UART_EVENT_RXDRDY:
			{
				 event_has = (bool)(p_reg->TSR & 0X01);
			}break;
			
			case NRF_UART_EVENT_ERROR:
			{
				 event_has = (bool)(p_reg->IIR_FCR.IIR  & 0X06);
			}break;
			
			case NRF_UART_EVENT_RXTO:
			{
				 event_has = (bool)(p_reg->IIR_FCR.IIR  & 0X0C);
			}break;
			
			default:break;
		}
    return (bool)event_has;
}



__STATIC_INLINE void nrf_uart_int_enable(NRF_UART_Type * p_reg, uint32_t mask)
{
   
}

__STATIC_INLINE bool nrf_uart_int_enable_check(NRF_UART_Type * p_reg, uint32_t mask)
{
    return (bool)0;
}

__STATIC_INLINE void nrf_uart_int_disable(NRF_UART_Type * p_reg, uint32_t mask)
{

}

__STATIC_INLINE uint32_t nrf_uart_errorsrc_get_and_clear(NRF_UART_Type * p_reg)
{
    uint32_t errsrc_mask = 0;

    return errsrc_mask;
}

__STATIC_INLINE void nrf_uart_enable(NRF_UART_Type * p_reg)
{

}

__STATIC_INLINE void nrf_uart_disable(NRF_UART_Type * p_reg)
{

}

__STATIC_INLINE void nrf_uart_txrx_pins_set(NRF_UART_Type * p_reg, uint32_t pseltxd, uint32_t pselrxd)
{
//#if defined(UART_PSEL_RXD_CONNECT_Pos)
//    p_reg->PSEL.RXD = pselrxd;
//#else
//    p_reg->PSELRXD = pselrxd;
//#endif
//#if defined(UART_PSEL_TXD_CONNECT_Pos)
//    p_reg->PSEL.TXD = pseltxd;
//#else
//    p_reg->PSELTXD = pseltxd;
//#endif
}


__STATIC_INLINE uint8_t nrf_uart_rxd_get(NRF_UART_Type * p_reg)
{
    return p_reg->RBR_THR_DLL.RBR;
}

__STATIC_INLINE void nrf_uart_txd_set(NRF_UART_Type * p_reg, uint8_t txd)
{
		p_reg->RBR_THR_DLL.THR = txd;
}


 
__STATIC_INLINE void nrf_uart_baudrate_set(NRF_UART_Type   * p_reg, nrf_uart_baudrate_t baudrate)
{
   p_reg->TCR |= (0x01 << 7);
	 p_reg->RBR_THR_DLL.DLL = baudrate & 0x0f;
	 p_reg->IER_DLH.DLH = 0;
	 p_reg->TCR &= ~(0x01 << 7);
}
#endif //SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif //NRF_UART_H__
