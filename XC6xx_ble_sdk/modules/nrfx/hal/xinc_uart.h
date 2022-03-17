/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 *
 */

#ifndef XINC_UART_H__
#define XINC_UART_H__

#include <nrfx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_uart_hal UART HAL
 * @{
 * @ingroup xinc_uart
 * @brief   Hardware access layer for managing the UART peripheral.
 */

/** @brief Pin disconnected value. */
#define XINC_UART_PSEL_DISCONNECTED 0xFFFFFFFF


/** @brief UART events. */
typedef enum
{
    XINC_UART_EVENT_CTS    = UART_UARTx_IIR_IID_EMSI,  /**< Event from CTS line activation. */
    XINC_UART_EVENT_TXDRDY = UART_UARTx_IIR_IID_ETHEI, /**< Event from data sent from TXD. */
    XINC_UART_EVENT_RXDRDY = UART_UARTx_IIR_IID_ERDAI, /**< Event from data ready in RXD. */
    XINC_UART_EVENT_ERROR  = UART_UARTx_IIR_IID_ETSI, /**< Event from error detection. */
    XINC_UART_EVENT_BUSY   = UART_UARTx_IIR_IID_BUSY, /**< Event from BUSY. */
    XINC_UART_EVENT_RXTO   = UART_UARTx_IIR_IID_TO,   /**< Event from receiver timeout. */
} xinc_uart_event_t;

/** @brief UART interrupts. */
typedef enum
{
    XINC_UART_INT_MASK_CTS    = UART_UARTx_IER_EMSI_Msk,//,    /**< CTS line activation interrupt. */
    XINC_UART_INT_MASK_RXDRDY = UART_UARTx_IER_ERDAI_Msk,//, /**< Data ready in RXD interrupt. */
    XINC_UART_INT_MASK_TXDRDY = UART_UARTx_IER_ETHEI_Msk,//, /**< Data sent from TXD interrupt. */
    XINC_UART_INT_MASK_ERROR  = UART_UARTx_IER_ETSI_Msk,//,  /**< Error detection interrupt. */
    XINC_UART_INT_MASK_RXTO   = UART_UARTx_IER_ERDAI_Msk,//    /**< Receiver timeout interrupt. */
} xinc_uart_int_mask_t;

/** @brief Baudrates supported by UART. */
typedef enum
{
    XINC_UART_BAUDRATE_2400    = UART_BAUDRATE_BAUDRATE_Baud2400,   /**< 2400 baud. */
    XINC_UART_BAUDRATE_4800    = UART_BAUDRATE_BAUDRATE_Baud4800,   /**< 4800 baud. */
    XINC_UART_BAUDRATE_9600    = UART_BAUDRATE_BAUDRATE_Baud9600,   /**< 9600 baud. */
    XINC_UART_BAUDRATE_14400   = UART_BAUDRATE_BAUDRATE_Baud14400,  /**< 14400 baud. */
    XINC_UART_BAUDRATE_19200   = UART_BAUDRATE_BAUDRATE_Baud19200,  /**< 19200 baud. */

    XINC_UART_BAUDRATE_38400   = UART_BAUDRATE_BAUDRATE_Baud38400,  /**< 38400 baud. */

    XINC_UART_BAUDRATE_57600   = UART_BAUDRATE_BAUDRATE_Baud57600,  /**< 57600 baud. */

    XINC_UART_BAUDRATE_115200  = UART_BAUDRATE_BAUDRATE_Baud115200, /**< 115200 baud. */
    XINC_UART_BAUDRATE_230400  = UART_BAUDRATE_BAUDRATE_Baud230400, /**< 230400 baud. */

    XINC_UART_BAUDRATE_460800  = UART_BAUDRATE_BAUDRATE_Baud460800, /**< 460800 baud. */
    XINC_UART_BAUDRATE_921600  = UART_BAUDRATE_BAUDRATE_Baud921600, /**< 921600 baud. */
    XINC_UART_BAUDRATE_1000000 = UART_BAUDRATE_BAUDRATE_Baud1M,     /**< 1000000 baud. */
} xinc_uart_baudrate_t;

/** @brief Types of UART error masks. */
typedef enum
{
    XINC_UART_ERROR_OVERFIFO_MASK = UART_UARTx_TSR_OE_Msk,   /**< OverFIFO error. */
    XINC_UART_ERROR_PARITY_MASK  = UART_UARTx_TSR_PE_Msk,   /**< Parity error. */
    XINC_UART_ERROR_FRAMING_MASK = UART_UARTx_TSR_FE_Msk,   /**< Framing error. */
    XINC_UART_ERROR_BREAK_MASK   = UART_UARTx_TSR_BI_Msk   /**< Break error. */
} xinc_uart_error_mask_t;

/** @brief Types of UART parity modes. */
typedef enum
{
    XINC_UART_PARITY_EXCLUDED = UART_UARTx_TCR_PEN_Excluded << UART_UARTx_TCR_PEN_Pos, /**< Parity excluded. */
    XINC_UART_PARITY_INCLUDED = UART_UARTx_TCR_PENS_Included << UART_UARTx_TCR_PEN_Pos, /**< Parity included. */
} xinc_uart_parity_t;

/** @brief Types of UART parity type. */
typedef enum
{
    XINC_UART_PARITY_ODD  = UART_UARTx_TCR_EPS_Odd << UART_UARTx_TCR_EPS_Pos, /**< Parity excluded. */
    XINC_UART_PARITY_EVEN = UART_UARTx_TCR_EPS_Even << UART_UARTx_TCR_EPS_Pos, /**< Parity included. */
} xinc_uart_parity_type_t;

/** @brief Types of UART flow control modes. */
typedef enum
{
    XINC_UART_HWFC_DISABLED = UART_UARTx_MCR_AFCE_Disable, /**< Hardware flow control disabled. */
    XINC_UART_HWFC_ENABLED  = UART_UARTx_MCR_AFCE_Enable,  /**< Hardware flow control enabled. */
} xinc_uart_hwfc_t;

/** @brief Types of UART data bits */
typedef enum
{
    XINC_UART_DATA_5_BITS  = UART_UARTx_TCR_CLS_5bits, /**< Hardware data 5 bits. */
    XINC_UART_DATA_6_BITS  = UART_UARTx_TCR_CLS_6bits,  /**<  Hardware data 5 bits.. */
    XINC_UART_DATA_7_BITS  = UART_UARTx_TCR_CLS_7bits, /**< Hardware data 5 bits. */
    XINC_UART_DATA_8_BITS  = UART_UARTx_TCR_CLS_8bits,  /**<  Hardware data 5 bits.. */
} xinc_uart_data_bits_t;

/** @brief Types of UART stop bits */
typedef enum
{
    XINC_UART_STOP_1_BITS  = UART_UARTx_TCR_STOP_1bits, /**< Hardware stop 1 bits. */
    XINC_UART_STOP_1_5_BITS  = UART_UARTx_TCR_STOP_1_5bits,  /**<  Hardware stop 1.5 bits.. */
    XINC_UART_STOP_2_BITS  = UART_UARTx_TCR_STOP_2bits, /**< Hardware stop 2 bits. */
} xinc_uart_stop_bits_t;



/**
 * @brief Function for enabling the specified interrupt.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param mask  Mask of interrupts to be enabled.
 */
__STATIC_INLINE void xinc_uart_int_enable(XINC_UART_Type * p_reg, uint32_t mask);

/**
 * @brief Function for retrieving the state of a given interrupt.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param mask  Mask of interrupts to be checked.
 *
 * @retval true  The interrupt is enabled.
 * @retval false The interrupt is not enabled.
 */
__STATIC_INLINE bool xinc_uart_int_enable_check(XINC_UART_Type * p_reg, uint32_t mask);

/**
 * @brief Function for disabling the specified interrupts.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param mask  Mask of interrupts to be disabled.
 */
__STATIC_INLINE void xinc_uart_int_disable(XINC_UART_Type * p_reg, uint32_t mask);

/**
 * @brief Function for getting error source mask. Function is clearing error source flags after reading.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Mask with error source flags.
 */
__STATIC_INLINE uint32_t xinc_uart_errorsrc_get_and_clear(XINC_UART_Type * p_reg);

/**
 * @brief Function for enabling UART.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_uart_enable(XINC_UART_Type * p_reg);

/**
 * @brief Function for disabling UART.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_uart_disable(XINC_UART_Type * p_reg);


/**
 * @brief Function for reading RX data.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Received byte.
 */
__STATIC_INLINE uint8_t xinc_uart_rxd_get(XINC_UART_Type * p_reg);

/**
 * @brief Function for setting Tx data.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param txd   Byte.
 */
__STATIC_INLINE void xinc_uart_txd_set(XINC_UART_Type * p_reg, uint8_t txd);




/**
 * @brief Function for setting UART baud rate.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param baudrate Baud rate.
 */
__STATIC_INLINE void xinc_uart_baudrate_set(XINC_UART_Type * p_reg, xinc_uart_baudrate_t baudrate);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION



__STATIC_INLINE bool xinc_uart_event_check(XINC_UART_Type * p_reg, xinc_uart_event_t event)
{
    bool event_has = false;
    switch(event)
    {    
        case XINC_UART_EVENT_TXDRDY:
        {
                event_has = (bool)((p_reg->TSR & UART_UARTx_TSR_THRE_Msk));
            
        }break;
        
        case XINC_UART_EVENT_RXDRDY:
        {
                event_has = (bool)((p_reg->IIR_FCR.IIR & UART_UARTx_IIR_IID_Msk) == UART_UARTx_IIR_IID_ERDAI);
        }break;
        
        case XINC_UART_EVENT_ERROR:
        {
                event_has = (bool)((p_reg->IIR_FCR.IIR & UART_UARTx_IIR_IID_Msk)  == UART_UARTx_IIR_IID_ETSI);
        }break;
        
        case XINC_UART_EVENT_RXTO:
        {
                event_has = (bool)((p_reg->IIR_FCR.IIR & UART_UARTx_IIR_IID_Msk)  == UART_UARTx_IIR_IID_TO);
        }break;
        
        default:break;
    }
    return (bool)event_has;
}



__STATIC_INLINE void xinc_uart_int_enable(XINC_UART_Type * p_reg, uint32_t mask)//DLAB = 0 操作的才是这个寄存器
{
  // p_reg->IER_DLH.IER |= mask;
}

__STATIC_INLINE bool xinc_uart_int_enable_check(XINC_UART_Type * p_reg, uint32_t mask)
{
    return (bool)(p_reg->IER_DLH.IER & mask);
}

__STATIC_INLINE void xinc_uart_int_disable(XINC_UART_Type * p_reg, uint32_t mask)
{
  //  p_reg->IER_DLH.IER &= ~mask;
}

__STATIC_INLINE uint32_t xinc_uart_errorsrc_get_and_clear(XINC_UART_Type * p_reg)
{
    uint32_t errsrc_mask = 0;//p_reg->TSR;

    return errsrc_mask;
}

__STATIC_INLINE void xinc_uart_enable(XINC_UART_Type * p_reg)
{

}

__STATIC_INLINE void xinc_uart_disable(XINC_UART_Type * p_reg)
{

}



__STATIC_INLINE uint8_t xinc_uart_rxd_get(XINC_UART_Type * p_reg)
{
    return (p_reg->RBR_THR_DLL.RBR & UART_UARTx_RBR_RBR_Msk) >> UART_UARTx_RBR_RBR_Pos;
}

__STATIC_INLINE void xinc_uart_txd_set(XINC_UART_Type * p_reg, uint8_t txd)
{
   // printf("txd_set\r\n");
		p_reg->RBR_THR_DLL.THR = txd;//(txd << UART_UARTx_THR_THR_Pos) & UART_UARTx_THR_THR_Msk;
}


 
__STATIC_INLINE void xinc_uart_baudrate_set(XINC_UART_Type   * p_reg, xinc_uart_baudrate_t baudrate)
{
     p_reg->TCR |= (UART_UARTx_TCR_DLAB_DLLH_Enable << UART_UARTx_TCR_DLAB_Pos);
	 p_reg->RBR_THR_DLL.DLL = baudrate & 0x0f ;//& UART_UARTx_DLL_DLL_Msk;
	 p_reg->IER_DLH.DLH = 0;
	 p_reg->TCR &= ~(UART_UARTx_TCR_DLAB_DLLH_Enable << UART_UARTx_TCR_DLAB_Pos);
}
#endif //SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif //XINC_UART_H__
