/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include <xincx.h>
#include <xincx_config.h>
#if XINCX_CHECK(XINCX_UART_ENABLED)

#if defined(XC60XX_M0)

#if !XINCX_CHECK(XINCX_UART0_ENABLED) && !XINCX_CHECK(XINCX_UART1_ENABLED)
#error "No enabled UART instances. Check <xincx_config.h>."
#endif

#if XINCX_CHECK(XINCX_UART2_ENABLED)
#error "CHIP XC60XX_M0 Not has UART2. Check <xincx_config.h>."
#endif

#elif defined (XC66XX_M4)
#if !XINCX_CHECK(XINCX_UART0_ENABLED) && !XINCX_CHECK(XINCX_UART1_ENABLED) && !XINCX_CHECK(XINCX_UART2_ENABLED)
#error "No enabled UART instances. Check <xincx_config.h>."
#endif

#endif



#include <xincx_uart.h>
#include <hal/xinc_gpio.h>
#include "xinc_delay.h"
#define XINCX_LOG_MODULE UART
#include <xincx_log.h>
#include "xincx_dmas.h"

#define EVT_TO_STR(event) \
    (event == XINC_UART_EVENT_ERROR ? "XINC_UART_EVENT_ERROR" : \
                                     "UNKNOWN EVENT")


#define TX_COUNTER_ABORT_REQ_VALUE  UINT32_MAX

typedef struct
{
    void                        * p_context;
    xincx_uart_event_handler_t  handler;
    uint8_t           const     * p_tx_buffer;
    uint8_t                     * p_rx_buffer;
    uint8_t                     * p_rx_secondary_buffer;
    volatile size_t             tx_buffer_length;
    size_t                      rx_buffer_length;
    size_t                      rx_secondary_buffer_length;
    volatile size_t             tx_counter;
    volatile size_t             rx_counter;
    volatile bool               tx_abort;
    bool                        rx_enabled;
    uint8_t                     rx_fifo_read_max;
    uint8_t                     rx_fifo_ready_read;
    uint8_t                     tx_dma_ch;
    uint8_t                     rx_dma_ch;
    xincx_drv_state_t          state;
} uart_control_block_t;
static uart_control_block_t m_cb[XINCX_UART_ENABLED_COUNT];

static xincx_err_t apply_config(xincx_uart_t        const * p_instance,
                         xincx_uart_config_t const * p_config)
{
    printf("%s\r\n",__func__);
    xincx_err_t err_code = XINCX_SUCCESS;
    printf("drv_inst_idx:%d\r\n",p_instance->drv_inst_idx);
    printf("p_reg:%p\r\n",p_instance->p_reg);
    
    
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
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
        else if(p_instance->id == 1)
        {
            err_code = xinc_gpio_secfun_config(p_config->pseltxd, XINC_GPIO_PIN_UART1_TX);
            printf("secfun_config1:%d\r\n",err_code);
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        #if defined (XC66XX_M4) && XINCX_CHECK(XINCX_UART2_ENABLED)
        else if(p_instance->id == 2UL)
        {
            err_code = xinc_gpio_secfun_config(p_config->pseltxd, XINC_GPIO_PIN_UART2_TX);
            printf("secfun_config2:%d\r\n",err_code);
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        #endif
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
        else if(p_instance->id == 1)
        {
            err_code = xinc_gpio_secfun_config(p_config->pselrxd, XINC_GPIO_PIN_UART1_RX);
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        #if defined (XC66XX_M4) && XINCX_CHECK(XINCX_UART2_ENABLED)
        else if(p_instance->id == 2UL)
        {
            err_code = xinc_gpio_secfun_config(p_config->pselrxd, XINC_GPIO_PIN_UART2_RX);
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        #endif
        
    }
    printf("baudrate_set:%d,busy:0x%p,%d\r\n",p_config->baudrate,&p_instance->p_reg->USR,p_instance->p_reg->USR);
    printf("parity:%d\r\n",p_config->parity);
    printf("data bits:%d\r\n",p_config->data_bits);
    printf("stop bits:%d\r\n",p_config->stop_bits);
    while(p_instance->p_reg->USR == 1)
    {
      //  printf("wait0\r\n");
        xinc_delay_ms(10);
    }
  
    xinc_uart_baudrate_set(p_instance->p_reg, p_config->baudrate);
    
    
    while(p_instance->p_reg->USR == 1)
    {
      //  printf("wait1\r\n");
        xinc_delay_ms(10);
    }
   
    //  xinc_uart_configure
    if(p_config->parity == XINC_UART_PARITY_INCLUDED)//????????????????????????
    {
        p_instance->p_reg->TCR |= (UART_UARTx_TCR_PEN_Msk);
        p_instance->p_reg->TCR &= ~(UART_UARTx_TCR_EPS_Msk);            
        p_instance->p_reg->TCR |= p_config->parity_type;
        
    }else
    {
        p_instance->p_reg->TCR &= ~(UART_UARTx_TCR_PEN_Msk);
    }

 //   printf("hwfc:%d\r\n",p_config->hwfc);
    while(p_instance->p_reg->USR == 1)
    {
        xinc_delay_ms(10);
    }
    if(p_config->hwfc == XINC_UART_HWFC_ENABLED)
    {
        p_instance->p_reg->MCR |= (UART_UARTx_MCR_AFCE_Enable << UART_UARTx_MCR_AFCE_Pos);
    }else
    {
        p_instance->p_reg->MCR &= ~(UART_UARTx_MCR_AFCE_Msk);
    }


    //??????data bits
    while(p_instance->p_reg->USR == 1)
    {
         xinc_delay_ms(10);
    }
    p_instance->p_reg->TCR &= ~(UART_UARTx_TCR_CLS_Msk); 
    p_instance->p_reg->TCR |= ((p_config->data_bits << UART_UARTx_TCR_CLS_Pos) & UART_UARTx_TCR_CLS_Msk);
   //  printf("TCR0:0x%p,0x%x\r\n",&p_instance->p_reg->TCR,p_instance->p_reg->TCR);
    while(p_instance->p_reg->USR == 1)
    {
         xinc_delay_ms(10);
    }
    //??????stop bits
    p_instance->p_reg->TCR &= ~(UART_UARTx_TCR_STOP_Msk);  
    p_instance->p_reg->TCR |= ((p_config->stop_bits << UART_UARTx_TCR_STOP_Pos) & UART_UARTx_TCR_STOP_Msk);

 //   printf("TCR1:%x\r\n",p_instance->p_reg->TCR);
    while(p_instance->p_reg->USR == 1)
    {
        xinc_delay_ms(10);
    }
    // FIFO ??????
    //    p_instance->p_reg->IIR_FCR.FCR = 0XB7;
    p_instance->p_reg->IIR_FCR.FCR =    (UART_UARTx_FCR_RCVR_Trigger_FIFO_2_1 << UART_UARTx_FCR_RCVR_Trigger_Pos) |
                                    (UART_UARTx_FCR_TX_Empty_Trigger_FIFO_2_1 << UART_UARTx_FCR_TX_Empty_Trigger_Pos) |
                                    (UART_UARTx_FCR_XMIT_FIFO_Reset_Clear << UART_UARTx_FCR_XMIT_FIFO_Reset_Pos)|
                                    (UART_UARTx_FCR_RCVR_FIFO_Reset_Clear << UART_UARTx_FCR_RCVR_FIFO_Reset_Pos)|
                                    (UART_UARTx_FCR_FIFO_Enable_Enable << UART_UARTx_FCR_FIFO_Enable_Pos);//0XB7;
          
    // ???????????????7????????????FIFO ?????????1/2??????????????? 16 /2 - 1
    p_cb->rx_fifo_read_max = 7;    
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
   // printf("%s\r\n",__func__);
    #if defined (XC60XX_M0) || defined (XC66XX_M4)
    if((p_instance->id == 0UL) || (p_instance->id == 1UL))
    {
        XINCX_IRQ_ENABLE((IRQn_Type)(UART0_IRQn + p_instance->id));
        printf("%s,id:%d\r\n",__func__,p_instance->id);
    }
    #endif
    #if defined (XC66XX_M4) && XINCX_CHECK(XINCX_UART2_ENABLED)
    if(p_instance->id == 2UL)
    {
        XINCX_IRQ_PRIORITY_SET((IRQn_Type)(UART2_IRQn),
        interrupt_priority);
        XINCX_IRQ_ENABLE((IRQn_Type)(UART2_IRQn));
    }
    #endif
    
}

static void interrupts_disable(xincx_uart_t const * p_instance)
{
    p_instance->p_reg->IER_DLH.IER = 0X00;
    #if defined (XC60XX_M0) || defined (XC66XX_M4)
    if((p_instance->id == 0UL) || (p_instance->id == 1UL))
    {
        XINCX_IRQ_DISABLE((IRQn_Type)(UART0_IRQn + p_instance->id));
    } 
    #endif
    #if defined (XC66XX_M4) && XINCX_CHECK(XINCX_UART2_ENABLED)
    if(p_instance->id == 2UL)
    {
        XINCX_IRQ_DISABLE((IRQn_Type)(UART2_IRQn));
    }
    #endif
}

static void xincx_uart_clk_init(xincx_uart_t const * const  p_instance,
                         xincx_uart_config_t const * p_config)
{
    XINC_CPR_CTL_Type * p_cpr = (XINC_CPR_CTL_Type *)p_instance->p_cpr;

    uint8_t ch = p_instance->id;
    uint32_t	val;
    #if XINCX_CHECK(XINCX_UART0_ENABLED)
    if(p_instance->id == 0UL)
    {
        p_cpr->RSTCTL_SUBRST_SW =   ((CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Enable << CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Pos)|
                                (CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET));

        p_cpr->RSTCTL_SUBRST_SW =   ((CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Disable << CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Pos)|
                                (CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET));
        
        //??????????????????????????????
        val = p_cpr->LP_CTL;
        val &= ~(CPR_LP_CTL_UART0_CLK_OFF_PROTECT_EN_Msk >> ch);
        p_cpr->LP_CTL = val;
     
        
        p_cpr->CTLAPBCLKEN_GRCTL = val = ((CPR_CTLAPBCLKEN_GRCTL_UART0_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_UART0_PCLK_EN_Pos) | 
                                    (CPR_CTLAPBCLKEN_GRCTL_UART0_PCLK_EN_Msk << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET));
        

         // 0x110018;
        p_cpr->UART0_CLK_GRCTL = (((8UL << CPR_UART_CLK_GRCTL_UART0_CLK_GR_Pos) | CPR_UART_CLK_GRCTL_UART0_CLK_GR_WE)   |
                                  
                                    ((CPR_UART_CLK_GRCTL_UART0_CLK_GR_UPD_Enable << CPR_UART_CLK_GRCTL_UART0_CLK_GR_UPD_Pos) |
                                    (CPR_UART_CLK_GRCTL_UART1_CLK_GR_UPD_WE)));
        
       
        p_cpr->UART0_CLK_CTL = p_config->baudrate>>4;

    }
    #endif
    #if XINCX_CHECK(XINCX_UART1_ENABLED)
    if(p_instance->id == 1UL)
    {
        p_cpr->RSTCTL_SUBRST_SW =   ((CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Enable << CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Pos)|
                                (CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET));

        p_cpr->RSTCTL_SUBRST_SW =   ((CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Disable << CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Pos)|
                                (CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET));
        
        //??????????????????????????????
        val = p_cpr->LP_CTL;
        val &= ~(CPR_LP_CTL_UART0_CLK_OFF_PROTECT_EN_Msk >> ch);
        p_cpr->LP_CTL = val;
     
        
        p_cpr->CTLAPBCLKEN_GRCTL = val = ((CPR_CTLAPBCLKEN_GRCTL_UART1_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_UART1_PCLK_EN_Pos) | 
                                    (CPR_CTLAPBCLKEN_GRCTL_UART1_PCLK_EN_Msk << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET));
        

         // 0x110018;
        p_cpr->UART1_CLK_GRCTL = (((8UL << CPR_UART_CLK_GRCTL_UART1_CLK_GR_Pos) | CPR_UART_CLK_GRCTL_UART1_CLK_GR_WE)   |
                                  
                                    ((CPR_UART_CLK_GRCTL_UART1_CLK_GR_UPD_Enable << CPR_UART_CLK_GRCTL_UART1_CLK_GR_UPD_Pos) |
                                        CPR_UART_CLK_GRCTL_UART1_CLK_GR_UPD_WE));
        
       
        p_cpr->UART1_CLK_CTL = p_config->baudrate>>4;

    }
    #endif
    
    #if defined (XC66XX_M4) && XINCX_CHECK(XINCX_UART2_ENABLED)
    if(p_instance->id == 2UL)
    {
        p_cpr->RSTCTL_SUBRST_SW =   ((CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Enable << CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Pos)|
                                (CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET));
        
        p_cpr->RSTCTL_SUBRST_SW =   ((CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Disable << CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Pos)|
                                (CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET));
        
        p_cpr->UART2_CLK_GRCTL  = (((8UL << CPR_UART_CLK_GRCTL_UART2_CLK_GR_Pos) | CPR_UART_CLK_GRCTL_UART2_CLK_GR_WE)   |
                                  
                                    (((CPR_UART_CLK_GRCTL_UART2_CLK_GR_UPD_Enable << CPR_UART_CLK_GRCTL_UART2_CLK_GR_UPD_Pos) |CPR_UART_CLK_GRCTL_UART2_CLK_GR_UPD_WE) |
                                           
                                        (CPR_UART_CLK_GRCTL_UART2_PCLK_EN_Enable << CPR_UART_CLK_GRCTL_UART2_PCLK_EN_Pos) | CPR_UART_CLK_GRCTL_UART2_PCLK_EN_WE));
        
        p_cpr->UART2_CLK_CTL = p_config->baudrate>>4;
    }
    #endif


}


xincx_err_t xincx_uart_init(xincx_uart_t const *        p_instance,
                          xincx_uart_config_t const * p_config,
                          xincx_uart_event_handler_t  event_handler)
{
    XINCX_ASSERT(p_config);
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    xincx_err_t err_code = XINCX_SUCCESS;
    
    printf("%s,id:%d,inst_idx:%d,state:%d\r\n",__func__,p_instance->id,p_instance->drv_inst_idx,p_cb->state);
    // printf("XINC_UART_Type size :%d\r\n",sizeof(XINC_UART_Type));
    if (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                            __func__,
                            XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    xincx_uart_clk_init(p_instance, p_config);
  
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
        
    printf("p_context: 0x%p\r\n",p_cb->p_context);
    xinc_uart_enable(p_instance->p_reg);
    p_cb->rx_buffer_length           = 0;
    p_cb->rx_secondary_buffer_length = 0;
    p_cb->rx_enabled                 = false;
    p_cb->tx_buffer_length           = 0;
    p_cb->state                      = XINCX_DRV_STATE_INITIALIZED;
    
    xinc_delay_ms(10);
    printf("handler: %p\r\n",p_cb->handler);
    if (p_cb->handler)
    {      
        interrupts_enable(p_instance, p_config->interrupt_priority);             
    }

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
        p_instance->p_reg->IER_DLH.IER = UART_UARTx_IER_ERDAI_Msk | UART_UARTx_IER_ETHEI_Msk | UART_UARTx_IER_PTIME_Msk;
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
  //  printf("rx:%c\r\n",p_cb->p_rx_buffer[p_cb->rx_counter]);
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
 //   printf("rx_buffer_length:%d, rx_secondary_buffer_length:%d\r\n",p_cb->rx_buffer_length,p_cb->rx_secondary_buffer_length);
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
     //   p_cb->rx_counter       = 0;
    }

    XINCX_LOG_INFO("Transfer rx_len: %d.", length);

    // printf("xincx_uart_rx:0x%p,0x%p\n",p_data,p_cb->p_rx_buffer);	

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
        p_instance->p_reg->IER_DLH.IER |= UART_UARTx_IER_ERDAI_Msk | UART_UARTx_IER_PTIME_Msk;//open rx interrupt	      
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

static void rx_ready_event(uart_control_block_t * p_cb,
                          size_t                 bytes,
                          uint8_t *              p_data)
{
//    printf("rx_ready_event:%p\r\n",p_cb->handler);
    xincx_uart_event_t event;

    event.type             = XINCX_UART_EVT_RX_READY;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = p_data;

    p_cb->handler(&event, p_cb->p_context);
}

static void rx_done_event(uart_control_block_t * p_cb,
                          size_t                 bytes,
                          uint8_t *              p_data)
{
//    printf("rx_done_event:%p\r\n",p_cb->handler);
    xincx_uart_event_t event;

    event.type             = XINCX_UART_EVT_RX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = p_data;
    p_cb->handler(&event, p_cb->p_context);
}


static void tx_done_event(uart_control_block_t * p_cb,
                          size_t                 bytes)
{
 //   printf("tx_done_event:%p\r\n",p_cb->handler);
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

  //  printf("xincx_uart_rx_abort\r\n");
}


static void uart_irq_handler(XINC_UART_Type *        p_uart,
                             uart_control_block_t * p_cb)
{

    uint32_t IIR,IER,TSR;
    IER = p_uart->IER_DLH.IER;
    IIR = p_uart->IIR_FCR.IIR;
    uint8_t RxBytes;

  //  printf("IER:%x,IIR:%x\r\n",IER,IIR);
    if (((IIR & UART_UARTx_IIR_IID_Msk) == UART_UARTx_IIR_IID_ETSI))
    {
       // xincx_uart_event_t event;

        XINCX_LOG_DEBUG("Event: %s.", EVT_TO_STR(XINC_UART_EVENT_ERROR));
 
    }


    if(((IIR & UART_UARTx_IIR_IID_Msk) == UART_UARTx_IIR_IID_ERDAI ))
    {
        TSR = p_uart->TSR;
        p_cb->rx_fifo_ready_read = 0UL;
        RxBytes = 0;
        while((TSR & UART_UARTx_TSR_DR_Msk) == UART_UARTx_TSR_DR_Msk )
        {
            rx_byte(p_uart, p_cb);
            RxBytes++;
            p_cb->rx_fifo_ready_read++;
            if(p_cb->rx_fifo_ready_read == p_cb->rx_fifo_read_max)
            {
                TSR = 0UL;
            }else
            {
                TSR = p_uart->TSR;
            }

            if(p_cb->rx_buffer_length == p_cb->rx_counter)
            {
            //    printf("rx0_counter:%d,rx_buffer_length:%d,RxBytesIdx:%d\r\n",p_cb->rx_counter,p_cb->rx_buffer_length,RxBytesIdx);      

                if (p_cb->rx_secondary_buffer_length)
                {
                    uint8_t * p_data     = p_cb->p_rx_buffer;
                    size_t    rx_counter = p_cb->rx_counter;

                    // Switch to secondary buffer.
                    p_cb->rx_buffer_length = p_cb->rx_secondary_buffer_length;
                    p_cb->p_rx_buffer = p_cb->p_rx_secondary_buffer;
                    p_cb->rx_secondary_buffer_length = 0;
                    p_cb->rx_counter = 0;
                     
                    rx_ready_event(p_cb, rx_counter, p_data);
                }
                else
                {
                    p_cb->rx_buffer_length = 0;

                    rx_ready_event(p_cb, p_cb->rx_counter, p_cb->p_rx_buffer);
                }              
            }              
        }
             
        
    }
    
    if((IIR & UART_UARTx_IIR_IID_Msk) == UART_UARTx_IIR_IID_TO)
    {
    // RXTO event may be triggered as a result of abort call. In th
        TSR = p_uart->TSR;
        RxBytes = 0;
        while((TSR & UART_UARTx_TSR_DR_Msk) == UART_UARTx_TSR_DR_Msk )
        {
             rx_byte(p_uart, p_cb);
                        
             TSR = p_uart->TSR;
             RxBytes++;

               
            if(p_cb->rx_buffer_length == p_cb->rx_counter)
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
                    rx_ready_event(p_cb, rx_counter, p_data);
                }
                else
                {
                    p_cb->rx_buffer_length = 0;
    
                    rx_ready_event(p_cb, p_cb->rx_counter, p_cb->p_rx_buffer);
                }              
            }                   
        }
       // printf("TO IER:%x,IIR:%x,rx_len:%d,p_cb->rx_counter:%d \r\n",IER,IIR,RxBytes,p_cb->rx_counter);   

   
        p_cb->rx_buffer_length = 0;
        rx_done_event(p_cb, p_cb->rx_counter, p_cb->p_rx_buffer);

    }

    if ((IIR & UART_UARTx_IIR_IID_Msk) == UART_UARTx_IIR_IID_ETHEI)
    {
        // Use a local variable to avoid undefined order of accessing two volatile variables
        // in one statement.
        size_t const tx_buffer_length = p_cb->tx_buffer_length;
        	
        TSR = p_uart->TSR;
        uint8_t fifo_full_flg;
        fifo_full_flg = ((TSR >> 5) & 0x01 );
        while (p_cb->tx_counter < tx_buffer_length && !p_cb->tx_abort && !fifo_full_flg)
        {
            tx_byte(p_uart, p_cb);
            TSR = p_uart->TSR;

            fifo_full_flg = ((TSR >> 5) & 0x01 );
        }
        
        if(p_cb->tx_counter == tx_buffer_length)
        {
          //  nrf_uart_event_clear(p_uart, NRF_UART_EVENT_TXDRDY);
            if (p_cb->tx_buffer_length)
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

#if XINCX_CHECK(XINCX_UART2_ENABLED)
void xincx_uart_2_irq_handler(void)
{
    uart_irq_handler(XINC_UART2, &m_cb[XINCX_UART2_INST_IDX]);
}
void UART2_Handler(void)
{
    
	xincx_uart_2_irq_handler();
		
}
#endif

#endif // XINCX_CHECK(XINCX_UART_ENABLED)
