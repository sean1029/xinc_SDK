/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include <xincx.h>

#if XINCX_CHECK(XINCX_UART_ENABLED)

#if !XINCX_CHECK(XINCX_UART0_ENABLED) && !XINCX_CHECK(XINCX_UART1_ENABLED)
#error "No enabled UART instances. Check <xincx_config.h>."
#endif

#include <xincx_uart.h>
#include <hal/xinc_gpio.h>
#include "bsp_register_macro.h"
#include "bsp_clk.h"
#include "bsp_uart.h"
#define XINCX_LOG_MODULE UART
#include <xincx_log.h>

#define EVT_TO_STR(event) \
    (event == XINC_UART_EVENT_ERROR ? "XINC_UART_EVENT_ERROR" : \
                                     "UNKNOWN EVENT")


#define TX_COUNTER_ABORT_REQ_VALUE  UINT32_MAX

typedef struct
{
    void                    * p_context;
    xincx_uart_event_handler_t handler;
    uint8_t           const * p_tx_buffer;
    uint8_t                 * p_rx_buffer;
    uint8_t                 * p_rx_secondary_buffer;
    volatile size_t           tx_buffer_length;
    size_t                    rx_buffer_length;
    size_t                    rx_secondary_buffer_length;
    volatile size_t           tx_counter;
    volatile size_t           rx_counter;
    volatile bool             tx_abort;
    bool                      rx_enabled;
    xincx_drv_state_t          state;
} uart_control_block_t;
static uart_control_block_t m_cb[XINCX_UART_ENABLED_COUNT];

static xincx_err_t apply_config(xincx_uart_t        const * p_instance,
                         xincx_uart_config_t const * p_config)
{
    printf("%s\r\n",__func__);
    xincx_err_t err_code = XINCX_SUCCESS;
    printf("pseltxd:%d\r\n",p_config->pseltxd);
    if (p_config->pseltxd != XINC_UART_PSEL_DISCONNECTED)
    {     
        if(p_instance->id == 0)
        {
            err_code = xinc_gpio_secfun_config(p_config->pseltxd, XINC_GPIO_PIN_UART0_TX);
            printf("secfun_config0:%d\r\n",err_code);
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        if(p_instance->id == 1)
        {
            err_code = xinc_gpio_secfun_config(p_config->pseltxd, XINC_GPIO_PIN_UART1_TX);
            printf("secfun_config1:%d\r\n",err_code);
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
    }
    printf("pselrxd:%d\r\n",p_config->pselrxd);
    if (p_config->pselrxd != XINC_UART_PSEL_DISCONNECTED)
    {
        if(p_instance->id == 0)
        {
            err_code = xinc_gpio_secfun_config(p_config->pselrxd, XINC_GPIO_PIN_UART0_RX);
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        if(p_instance->id == 1)
        {
            err_code = xinc_gpio_secfun_config(p_config->pselrxd, XINC_GPIO_PIN_UART1_RX);
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        
    }
    printf("baudrate_set:%d\r\n",p_config->baudrate);

    xinc_uart_baudrate_set(p_instance->p_reg, p_config->baudrate);

    printf("parity:%d\r\n",p_config->parity);
    //  xinc_uart_configure
    if(p_config->parity == XINC_UART_PARITY_INCLUDED)//奇偶校验使能位：
    {
        p_instance->p_reg->TCR |= (UART_UARTx_TCR_PEN_Msk);
        p_instance->p_reg->TCR &= ~(UART_UARTx_TCR_EPS_Msk);            
        p_instance->p_reg->TCR |= p_config->parity_type;
        
    }else
    {
        p_instance->p_reg->TCR &= ~(UART_UARTx_TCR_PEN_Msk);
    }

    printf("hwfc:%d\r\n",p_config->hwfc);
    if(p_config->hwfc == XINC_UART_HWFC_ENABLED)
    {
        p_instance->p_reg->MCR |= (UART_UARTx_MCR_AFCE_Enable << UART_UARTx_MCR_AFCE_Pos);
    }else
    {
        p_instance->p_reg->MCR &= ~(UART_UARTx_MCR_AFCE_Msk);
    }


    printf("data bits:%d\r\n",p_config->data_bits);
    printf("stop bits:%d\r\n",p_config->stop_bits);
    //设置data bits
    p_instance->p_reg->TCR &= ~(UART_UARTx_TCR_CLS_Msk); 
    p_instance->p_reg->TCR |= ((p_config->data_bits << UART_UARTx_TCR_CLS_Pos) & UART_UARTx_TCR_CLS_Msk);

    //设置stop bits
    p_instance->p_reg->TCR &= ~(UART_UARTx_TCR_STOP_Msk);  
    p_instance->p_reg->TCR |= ((p_config->stop_bits << UART_UARTx_TCR_STOP_Pos) & UART_UARTx_TCR_STOP_Msk);

    printf("TCR:%x\r\n",p_instance->p_reg->TCR);
    // FIFO 控制
    //    p_instance->p_reg->IIR_FCR.FCR = 0XB7;
    p_instance->p_reg->IIR_FCR.FCR =    (UART_UARTx_FCR_RCVR_Trigger_FIFO_2_1 << UART_UARTx_FCR_RCVR_Trigger_Pos) |
                                    (UART_UARTx_FCR_TX_Empty_Trigger_FIFO_2_1 << UART_UARTx_FCR_TX_Empty_Trigger_Pos) |
                                    (UART_UARTx_FCR_XMIT_FIFO_Reset_Clear << UART_UARTx_FCR_XMIT_FIFO_Reset_Pos)|
                                    (UART_UARTx_FCR_RCVR_FIFO_Reset_Clear << UART_UARTx_FCR_RCVR_FIFO_Reset_Pos)|
                                    (UART_UARTx_FCR_FIFO_Enable_Enable << UART_UARTx_FCR_FIFO_Enable_Pos);//0XB7;
                                    
        
    printf("FCR:%x\r\n",p_instance->p_reg->IIR_FCR.FCR);



    if (p_config->hwfc == XINC_UART_HWFC_ENABLED)
    {
        if (p_config->pselcts != XINC_UART_PSEL_DISCONNECTED)
        {
            
        }

        if (p_config->pselrts != XINC_UART_PSEL_DISCONNECTED)
        {
            
        }

    }
    return err_code;
}

static void interrupts_enable(xincx_uart_t const * p_instance,
                              uint8_t             interrupt_priority)
{
    printf("%s\r\n",__func__);

    p_instance->p_reg->IER_DLH.IER = UART_UARTx_IER_ERDAI_Msk ;//| UART_UARTx_IER_ETHEI_Msk;// | UART_UARTx_IER_PTIME_Msk;//open tx/rx interrupt

    printf("p_instance->p_reg:%p\r\n",p_instance->p_reg);
            
    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + p_instance->id));
}

static void interrupts_disable(xincx_uart_t const * p_instance)
{
    p_instance->p_reg->IER_DLH.IER = 0X00;
    XINCX_IRQ_DISABLE((IRQn_Type)(UART0_IRQn + p_instance->id));
}


xincx_err_t xincx_uart_init(xincx_uart_t const *        p_instance,
                          xincx_uart_config_t const * p_config,
                          xincx_uart_event_handler_t  event_handler)
{
    XINCX_ASSERT(p_config);
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    xincx_err_t err_code = XINCX_SUCCESS;
    printf("%s,inst_idx:%d,state:%d\r\n",__func__,p_instance->drv_inst_idx,p_cb->state);
    // printf("XINC_UART_Type size :%d\r\n",sizeof(XINC_UART_Type));
    if (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                            __func__,
                            XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    XINC_CPR_CTL_Type * p_cpr = p_instance->p_cpr;
    uint8_t ch = p_instance->id;
    uint32_t	val;		


    p_cpr->RSTCTL_SUBRST_SW = (1<<(16+ch));
    p_cpr->RSTCTL_SUBRST_SW = ((1<<(16+ch))|(1<<ch));
    
    val = p_cpr->LP_CTL;
    val &= ~(1<<(1-ch));
    p_cpr->LP_CTL = val;
    
    val = (1<<(16+ch+4)) | (1<<(ch+4));
    p_cpr->CTLAPBCLKEN_GRCTL = val;
    
    if(ch == 0)
    {
        p_cpr->UART0_GRCTL = 0x110018;
        p_cpr->UART0_CLK_CTL = p_config->baudrate>>4;
    }
    else
    {   
        p_cpr->UART1_GRCTL = 0x110018;
        p_cpr->UART1_CLK_CTL = p_config->baudrate>>4;
    
    }
    
    err_code = apply_config(p_instance, p_config);

    if(err_code != XINCX_SUCCESS)
    {
        err_code = XINCX_ERROR_INVALID_PARAM;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                            __func__,
                            XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    p_cb->handler   = event_handler;
    p_cb->p_context = p_config->p_context;

    if (p_cb->handler)
    {
        interrupts_enable(p_instance, p_config->interrupt_priority);
    }

    xinc_uart_enable(p_instance->p_reg);
    p_cb->rx_buffer_length           = 0;
    p_cb->rx_secondary_buffer_length = 0;
    p_cb->rx_enabled                 = false;
    p_cb->tx_buffer_length           = 0;
    p_cb->state                      = XINCX_DRV_STATE_INITIALIZED;
    XINCX_LOG_WARNING("Function: %s, error code: %s.",
                        __func__,
                        XINCX_LOG_ERROR_STRING_GET(err_code));

    printf("Function: %s, error code: %s.\r\n",
                        __func__,
                        XINCX_LOG_ERROR_STRING_GET(err_code));
return err_code;
}

void xincx_uart_uninit(xincx_uart_t const * p_instance)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    xinc_uart_disable(p_instance->p_reg);

    if (p_cb->handler)
    {
        interrupts_disable(p_instance);
    }


    p_cb->state   = XINCX_DRV_STATE_UNINITIALIZED;
    p_cb->handler = NULL;
    XINCX_LOG_INFO("Instance uninitialized: %d.", p_instance->drv_inst_idx);
}

static void tx_byte(XINC_UART_Type * p_uart, uart_control_block_t * p_cb)
{
    uint8_t txd = p_cb->p_tx_buffer[p_cb->tx_counter];
    p_cb->tx_counter++;
    xinc_uart_txd_set(p_uart, txd);
}

static bool tx_blocking(XINC_UART_Type * p_uart, uart_control_block_t * p_cb)
{
    // Use a local variable to avoid undefined order of accessing two volatile variables
    // in one statement.
    size_t const tx_buffer_length = p_cb->tx_buffer_length;
    while (p_cb->tx_counter < tx_buffer_length)
    {
        // Wait until the transmitter is ready to accept a new byte.
        // Exit immediately if the transfer has been aborted.
        while (!xinc_uart_event_check(p_uart, XINC_UART_EVENT_TXDRDY))
        {
            if (p_cb->tx_abort)
            {
                return false;
            }
        }

        tx_byte(p_uart, p_cb);
    }

    return true;
}

xincx_err_t xincx_uart_tx(xincx_uart_t const * p_instance,
                        uint8_t const *     p_data,
                        size_t              length)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINCX_ASSERT(p_cb->state == XINCX_DRV_STATE_INITIALIZED);
    XINCX_ASSERT(p_data);
    XINCX_ASSERT(length > 0);

    xincx_err_t err_code;
	//  printf("xincx_uart_tx progress:%d\r\n",xincx_uart_tx_in_progress(p_instance));
    if (xincx_uart_tx_in_progress(p_instance))
    {
        err_code = XINCX_ERROR_BUSY;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    p_cb->tx_buffer_length = length;
    p_cb->p_tx_buffer      = p_data;
    p_cb->tx_counter       = 0;
    p_cb->tx_abort         = false;

    XINCX_LOG_INFO("Transfer tx_len: %d.", p_cb->tx_buffer_length);
    XINCX_LOG_DEBUG("Tx data:");
    XINCX_LOG_HEXDUMP_DEBUG(p_cb->p_tx_buffer,
                           p_cb->tx_buffer_length * sizeof(p_cb->p_tx_buffer[0]));

    err_code = XINCX_SUCCESS;

    tx_byte(p_instance->p_reg, p_cb);
    

    if (p_cb->handler == NULL)
    {
        if (!tx_blocking(p_instance->p_reg, p_cb))
        {
            // The transfer has been aborted.
            err_code = XINCX_ERROR_FORBIDDEN;
        }
        else
        {
            // Wait until the last byte is completely transmitted.
            while (!xinc_uart_event_check(p_instance->p_reg, XINC_UART_EVENT_TXDRDY))
            {}
           
        }
        p_cb->tx_buffer_length = 0;
    }else
    {
        p_instance->p_reg->IER_DLH.IER = UART_UARTx_IER_ERDAI_Msk | UART_UARTx_IER_ETHEI_Msk;;
    }

    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

bool xincx_uart_tx_in_progress(xincx_uart_t const * p_instance)
{
//    printf("tx_buffer_length:%d\r\n",m_cb[p_instance->drv_inst_idx].tx_buffer_length);
    return (m_cb[p_instance->drv_inst_idx].tx_buffer_length != 0);
}


static void rx_byte(XINC_UART_Type * p_uart, uart_control_block_t * p_cb)
{
    if (!p_cb->rx_buffer_length)
    {
        // Byte received when buffer is not set - data lost.
        (void) xinc_uart_rxd_get(p_uart);
        return;
    }
    p_cb->p_rx_buffer[p_cb->rx_counter] = xinc_uart_rxd_get(p_uart);
    p_cb->rx_counter++;
}

xincx_err_t xincx_uart_rx(xincx_uart_t const * p_instance,
                        uint8_t *           p_data,
                        size_t              length)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    XINCX_ASSERT(m_cb[p_instance->drv_inst_idx].state == XINCX_DRV_STATE_INITIALIZED);
    XINCX_ASSERT(p_data);
    XINCX_ASSERT(length > 0);

    xincx_err_t err_code;

    bool second_buffer = false;

    if (p_cb->handler)
    {
//        xinc_uart_int_disable(p_instance->p_reg, XINC_UART_INT_MASK_RXDRDY |
//                                                XINC_UART_INT_MASK_ERROR);
    }
    if (p_cb->rx_buffer_length != 0)
    {
        if (p_cb->rx_secondary_buffer_length != 0)
        {
            if (p_cb->handler)
            {
//                xinc_uart_int_enable(p_instance->p_reg, XINC_UART_INT_MASK_RXDRDY |
//                                                       XINC_UART_INT_MASK_ERROR);
            }
            err_code = XINCX_ERROR_BUSY;
            XINCX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             XINCX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
        second_buffer = true;
    }

    if (!second_buffer)
    {
        p_cb->rx_buffer_length = length;
        p_cb->p_rx_buffer      = p_data;
        p_cb->rx_counter       = 0;
        p_cb->rx_secondary_buffer_length = 0;
    }
    else
    {
        p_cb->p_rx_secondary_buffer = p_data;
        p_cb->rx_secondary_buffer_length = length;
    }

    XINCX_LOG_INFO("Transfer rx_len: %d.", length);
   

    if (p_cb->handler == NULL)
    {

        bool rxrdy;
        bool rxto;
        bool error;
        do
        {
            do
            {
                error = xinc_uart_event_check(p_instance->p_reg, XINC_UART_EVENT_ERROR);
                rxrdy = xinc_uart_event_check(p_instance->p_reg, XINC_UART_EVENT_RXDRDY);
                rxto  = xinc_uart_event_check(p_instance->p_reg, XINC_UART_EVENT_RXTO);
            } while ((!rxrdy) && (!rxto) && (!error));

            if (error || rxto)
            {
                break;
            }
            rx_byte(p_instance->p_reg, p_cb);
        } while (p_cb->rx_buffer_length > p_cb->rx_counter);

        p_cb->rx_buffer_length = 0;
        if (error)
        {
            err_code = XINCX_ERROR_INTERNAL;
            XINCX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             XINCX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }

        if (rxto)
        {
            err_code = XINCX_ERROR_FORBIDDEN;
            XINCX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             XINCX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }

    }
    else
    {
//        xinc_uart_int_enable(p_instance->p_reg, XINC_UART_INT_MASK_RXDRDY |
//                                               XINC_UART_INT_MASK_ERROR);
    }
    err_code = XINCX_SUCCESS;
    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

bool xincx_uart_rx_ready(xincx_uart_t const * p_instance)
{
    return xinc_uart_event_check(p_instance->p_reg, XINC_UART_EVENT_RXDRDY);
}

void xincx_uart_rx_enable(xincx_uart_t const * p_instance)
{
    if (!m_cb[p_instance->drv_inst_idx].rx_enabled)
    {
        m_cb[p_instance->drv_inst_idx].rx_enabled = true;
    }
}

void xincx_uart_rx_disable(xincx_uart_t const * p_instance)
{

    m_cb[p_instance->drv_inst_idx].rx_enabled = false;
}

uint32_t xincx_uart_errorsrc_get(xincx_uart_t const * p_instance)
{
//    xinc_uart_event_clear(p_instance->p_reg, XINC_UART_EVENT_ERROR);
    return xinc_uart_errorsrc_get_and_clear(p_instance->p_reg);
}

static void rx_done_event(uart_control_block_t * p_cb,
                          size_t                 bytes,
                          uint8_t *              p_data)
{
    xincx_uart_event_t event;

    event.type             = XINCX_UART_EVT_RX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = p_data;

    p_cb->handler(&event, p_cb->p_context);
}

static void tx_done_event(uart_control_block_t * p_cb,
                          size_t                 bytes)
{
    xincx_uart_event_t event;

    event.type             = XINCX_UART_EVT_TX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = (uint8_t *)p_cb->p_tx_buffer;

    p_cb->tx_buffer_length = 0;

    p_cb->handler(&event, p_cb->p_context);
}

void xincx_uart_tx_abort(xincx_uart_t const * p_instance)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    p_cb->tx_abort = true;
    if (p_cb->handler)
    {
        tx_done_event(p_cb, p_cb->tx_counter);
    }

    XINCX_LOG_INFO("TX transaction aborted.");
    printf("xincx_uart_tx_abort\r\n");
}

void xincx_uart_rx_abort(xincx_uart_t const * p_instance)
{
    xinc_uart_int_disable(p_instance->p_reg, XINC_UART_INT_MASK_RXDRDY |
                                        XINC_UART_INT_MASK_ERROR);

    XINCX_LOG_INFO("RX transaction aborted.");

    printf("xincx_uart_rx_abort\r\n");
}

static void uart_irq_handler(XINC_UART_Type *        p_uart,
                             uart_control_block_t * p_cb)
{

    uint32_t IIR,IER;
    IER = p_uart->IER_DLH.IER;
    IIR = p_uart->IIR_FCR.IIR;

  //  printf("IER:%x,IIR:%x\r\n",IER,IIR);
    if (((IIR & UART_UARTx_IIR_IID_Msk) == UART_UARTx_IIR_IID_ETSI))
    {
        xincx_uart_event_t event;

        XINCX_LOG_DEBUG("Event: %s.", EVT_TO_STR(XINC_UART_EVENT_ERROR));
//        xinc_uart_int_disable(p_uart, XINC_UART_INT_MASK_RXDRDY |
//                                     XINC_UART_INT_MASK_ERROR);
        if (!p_cb->rx_enabled)
        {
        }
        event.type                   = XINCX_UART_EVT_ERROR;
        event.data.error.error_mask  = xinc_uart_errorsrc_get_and_clear(p_uart);
        event.data.error.rxtx.bytes  = p_cb->rx_buffer_length;
        event.data.error.rxtx.p_data = p_cb->p_rx_buffer;

        // Abort transfer.
        p_cb->rx_buffer_length = 0;
        p_cb->rx_secondary_buffer_length = 0;

        p_cb->handler(&event,p_cb->p_context);
    }


    if(((IIR & UART_UARTx_IIR_IID_Msk) == UART_UARTx_IIR_IID_ERDAI )|| ( (IIR & UART_UARTx_IIR_IID_Msk) == UART_UARTx_IIR_IID_TO))
    {

        rx_byte(p_uart, p_cb);

        //  printf("rx_counter:%d,rx_buffer_length:%d\r\n",p_cb->rx_counter,p_cb->rx_buffer_length);
        if (p_cb->rx_buffer_length == p_cb->rx_counter)
        {
            if (p_cb->rx_secondary_buffer_length)
            {
                uint8_t * p_data     = p_cb->p_rx_buffer;
                size_t    rx_counter = p_cb->rx_counter;

                // Switch to secondary buffer.
                p_cb->rx_buffer_length = p_cb->rx_secondary_buffer_length;
                p_cb->p_rx_buffer = p_cb->p_rx_secondary_buffer;
                p_cb->rx_secondary_buffer_length = 0;
                p_cb->rx_counter = 0;
                rx_done_event(p_cb, rx_counter, p_data);
            }
            else
            {
                if (!p_cb->rx_enabled)
                {
                //                    
                }
//                xinc_uart_int_disable(p_uart, XINC_UART_INT_MASK_RXDRDY |
//                                    XINC_UART_INT_MASK_ERROR);
                p_cb->rx_buffer_length = 0;
                rx_done_event(p_cb, p_cb->rx_counter, p_cb->p_rx_buffer);
            }
        }

        if((IIR & UART_UARTx_IIR_IID_Msk) == UART_UARTx_IIR_IID_TO)
        {
        // RXTO event may be triggered as a result of abort call. In th
            if (p_cb->rx_enabled)
            {
            //									
            }
            if (p_cb->rx_buffer_length)
            {
                p_cb->rx_buffer_length = 0;
                rx_done_event(p_cb, p_cb->rx_counter, p_cb->p_rx_buffer);
            }
        }

    }

    if ((IIR & UART_UARTx_IIR_IID_Msk) == UART_UARTx_IIR_IID_ETHEI)
    {
        // Use a local variable to avoid undefined order of accessing two volatile variables
        // in one statement.
        size_t const tx_buffer_length = p_cb->tx_buffer_length;
        //	printf("tx:%d,%d\r\n",p_cb->tx_counter,tx_buffer_length);
     
         {
            while (p_cb->tx_counter < tx_buffer_length && !p_cb->tx_abort)
            {
                if(!xinc_uart_event_check(p_uart, XINC_UART_EVENT_TXDRDY))
                {
                    tx_byte(p_uart, p_cb);
                }
                else
                {
                  //  printf("break\r\n");
                    break;
                }
                
            }   
            if (p_cb->tx_buffer_length && (p_cb->tx_counter == tx_buffer_length))
            {
                p_uart->IER_DLH.IER = UART_UARTx_IER_ERDAI_Msk;
                tx_done_event(p_cb, p_cb->tx_buffer_length);
            }
            
        }
    }
    (void)IER;
}


#if XINCX_CHECK(XINCX_UART0_ENABLED)
void xincx_uart_0_irq_handler(void)
{
    uart_irq_handler(XINC_UART0, &m_cb[XINCX_UART0_INST_IDX]);
}
void UART0_Handler(void)
{
    xincx_uart_0_irq_handler();    
}
#endif

#if XINCX_CHECK(XINCX_UART1_ENABLED)
void xincx_uart_1_irq_handler(void)
{
    uart_irq_handler(XINC_UART1, &m_cb[XINCX_UART1_INST_IDX]);
}
void UART1_Handler(void)
{
    
	xincx_uart_1_irq_handler();
		
}
#endif

#endif // XINCX_CHECK(XINCX_UART_ENABLED)
