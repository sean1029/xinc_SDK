/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include <xincx.h>

#if XINCX_CHECK(XINCX_UARTE_ENABLED)

#if !XINCX_CHECK(XINCX_UARTE0_ENABLED) && !XINCX_CHECK(XINCX_UARTE1_ENABLED)
#error "No enabled UARTE instances. Check <xincx_config.h>."
#endif

#if !XINCX_CHECK(XINCX_DMAS_ENABLED)
#error "No enabled DMAS instances. Check <xincx_config.h>."
#endif

#include <xincx_uarte.h>
#include <xincx_dmas.h>
#include <hal/xinc_gpio.h>
#include "xinc_delay.h"
#define XINCX_LOG_MODULE UARTE
#include <xincx_log.h>


#define EVT_TO_STR(event) \
    (event == XINC_UART_EVENT_ERROR ? "XINC_UART_EVENT_ERROR" : \
                                     "UNKNOWN EVENT")

void uarte0_irq_dma_ch_handler(xincx_dmas_ch_evt_t const * p_event,
                                        void *                 p_context);
void uarte1_irq_dma_ch_handler(xincx_dmas_ch_evt_t const * p_event,
                                        void *                 p_context);

#define TX_COUNTER_ABORT_REQ_VALUE  UINT32_MAX

typedef struct
{
    void                        * p_context;
    xincx_uarte_event_handler_t  handler;
    xincx_dmas_t                * dmas;
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
    bool                        use_easy_dma;
    xinc_dma_ch_t               tx_dma_ch;
    xinc_dma_ch_t               rx_dma_ch;
    xincx_drv_state_t          state;
} uarte_control_block_t;
static uarte_control_block_t m_cb[XINCX_UARTE_ENABLED_COUNT];

static xincx_err_t apply_config(xincx_uarte_t        const * p_instance,
                         xincx_uarte_config_t const * p_config)
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
    if(p_config->parity == XINC_UART_PARITY_INCLUDED)//???????:
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


    //??data bits
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
    //??stop bits
    p_instance->p_reg->TCR &= ~(UART_UARTx_TCR_STOP_Msk);  
    p_instance->p_reg->TCR |= ((p_config->stop_bits << UART_UARTx_TCR_STOP_Pos) & UART_UARTx_TCR_STOP_Msk);

 //   printf("TCR1:%x\r\n",p_instance->p_reg->TCR);
    while(p_instance->p_reg->USR == 1)
    {
        xinc_delay_ms(10);
    }
    // FIFO ??
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

//static void interrupts_enable(xincx_uarte_t const * p_instance,
//                              uint8_t             interrupt_priority)
//{
//    printf("%s\r\n",__func__);

//      
//    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + p_instance->id));
//}

static void interrupts_disable(xincx_uarte_t const * p_instance)
{
    p_instance->p_reg->IER_DLH.IER = 0X00;
    XINCX_IRQ_DISABLE((IRQn_Type)(UART0_IRQn + p_instance->id));
}

//static void xincx_uarte_clk_init(xincx_uarte_t const * const  p_instance,
//                         xincx_uarte_config_t const * p_config)
//{
//    XINC_CPR_CTL_Type * p_cpr = (XINC_CPR_CTL_Type *)p_instance->p_cpr;

//    volatile uint32_t *Uart0_CLK_GRCTL_BaseAddr = (uint32_t*)&(p_cpr->UART0_CLK_GRCTL);
//    uint8_t ch = p_instance->id;
//    uint32_t	val;		
//    p_cpr->RSTCTL_SUBRST_SW =   ((CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Enable << CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Pos)|
//                                (CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET)) << ch;

//    //??????????
//    val = p_cpr->LP_CTL;
//    val &= ~(CPR_LP_CTL_UART0_CLK_OFF_PROTECT_EN_Msk >> ch);
//    p_cpr->LP_CTL = val;
// 
//    
//    p_cpr->CTLAPBCLKEN_GRCTL = val = ((CPR_CTLAPBCLKEN_GRCTL_UART0_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_UART0_PCLK_EN_Pos) | 
//                                (CPR_CTLAPBCLKEN_GRCTL_UART0_PCLK_EN_Msk << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET)) << ch;
//    
//    Uart0_CLK_GRCTL_BaseAddr += (ch * 2UL); //????? UART_CLK_GRCTL ???

//    
//    // 0x110018;
//    *Uart0_CLK_GRCTL_BaseAddr = val = (((8UL << CPR_UART_CLK_GRCTL_UART_CLK_GR_Pos) | CPR_UART_CLK_GRCTL_UART_CLK_GR_WE)   |
//                              
//                                ((CPR_UART_CLK_GRCTL_UART_CLK_GR_UPD_Enable << CPR_UART_CLK_GRCTL_UART_CLK_GR_UPD_Pos) |
//                                (CPR_UART_CLK_GRCTL_UART_CLK_GR_UPD_Msk << CPR_UART_CLK_GRCTL_MASK_OFFSET)));

//    Uart0_CLK_GRCTL_BaseAddr++;  //????? UART_CLK_CTL ???

//    *Uart0_CLK_GRCTL_BaseAddr = p_config->baudrate>>4;

//}

static void xincx_uarte_clk_init(xincx_uarte_t const * const  p_instance,
                         xincx_uarte_config_t const * p_config)
{
    XINC_CPR_CTL_Type * p_cpr = (XINC_CPR_CTL_Type *)p_instance->p_cpr;

    uint8_t ch = p_instance->id;
    uint32_t	val;
    #if XINCX_CHECK(XINCX_UARTE0_ENABLED)
    if(p_instance->id == 0UL)
    {
        p_cpr->RSTCTL_SUBRST_SW =   ((CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Enable << CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Pos)|
                                (CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET));

        p_cpr->RSTCTL_SUBRST_SW =   ((CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Disable << CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Pos)|
                        (CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET));
        //不使能其时钟保护功能
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
    #if XINCX_CHECK(XINCX_UARTE1_ENABLED)
    if(p_instance->id == 1UL)
    {
        p_cpr->RSTCTL_SUBRST_SW =   ((CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Enable << CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Pos)|
                                (CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET));
        
        p_cpr->RSTCTL_SUBRST_SW =   ((CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Disable << CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Pos)|
                                (CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET));


        //不使能其时钟保护功能
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
    


}

static void xincx_uarte_dma_ch_set(xincx_uarte_t const *p_instance)
{
    uint8_t id = p_instance->id;
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    switch(id)
    {
        case 0:
        {
            p_cb->tx_dma_ch = DMAS_CH_SEND_UART0;
            p_cb->rx_dma_ch = DMAS_CH_RCV_UART0;
            #if XINCX_CHECK(XINCX_UARTE0_ENABLED)
            xincx_dmas_ch_handler_register(p_cb->rx_dma_ch,uarte0_irq_dma_ch_handler);
            xincx_dmas_ch_handler_register(p_cb->tx_dma_ch,uarte0_irq_dma_ch_handler);
            #endif //
        }break;
        
        case 1:
        {
            p_cb->tx_dma_ch = DMAS_CH_SEND_UART1;
            p_cb->rx_dma_ch = DMAS_CH_RCV_UART1;
            #if XINCX_CHECK(XINCX_UARTE1_ENABLED)
            xincx_dmas_ch_handler_register(p_cb->rx_dma_ch,uarte1_irq_dma_ch_handler);
            xincx_dmas_ch_handler_register(p_cb->tx_dma_ch,uarte1_irq_dma_ch_handler);
            #endif //
        }break;
    }

}

xincx_err_t xincx_uarte_init(xincx_uarte_t const *        p_instance,
                          xincx_uarte_config_t const * p_config,
                          xincx_uarte_event_handler_t  event_handler)
{
    XINCX_ASSERT(p_config);
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
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

    if(p_instance->id > 1UL)
    {
         err_code = XINC_ERROR_NOT_SUPPORTED;
         XINCX_LOG_WARNING("Function: %s, error code: %s.",
                            __func__,
                            XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    xincx_uarte_clk_init(p_instance, p_config);
  
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
   
    xincx_uarte_dma_ch_set(p_instance); 
    
  
    xinc_uart_enable(p_instance->p_reg);
    p_cb->rx_buffer_length           = 0;
    p_cb->rx_secondary_buffer_length = 0;
    p_cb->rx_enabled                 = false;
    p_cb->tx_buffer_length           = 0;
    p_cb->state                      = XINCX_DRV_STATE_INITIALIZED;
    
    xinc_delay_ms(10);
    if (p_cb->handler)
    {
        
    }

    XINCX_LOG_WARNING("Function: %s, error code: %s.",
                        __func__,
                        XINCX_LOG_ERROR_STRING_GET(err_code));

    printf("Function: %s, error code: %s.\r\n",
                        __func__,
                        XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

void xincx_uarte_uninit(xincx_uarte_t const * p_instance)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    xinc_uart_disable(p_instance->p_reg);

    if (p_cb->handler)
    {
        interrupts_disable(p_instance);
    }


    p_cb->state   = XINCX_DRV_STATE_UNINITIALIZED;
    p_cb->handler = NULL;
    XINCX_LOG_INFO("Instance uninitialized: %d.", p_instance->drv_inst_idx);
}


xincx_err_t xincx_uarte_tx(xincx_uarte_t const * p_instance,
                        uint8_t const *     p_data,
                        size_t              length)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINCX_ASSERT(p_cb->state == XINCX_DRV_STATE_INITIALIZED);
    XINCX_ASSERT(p_data);
    XINCX_ASSERT(length > 0);

    xincx_err_t err_code;
    
    if(true == p_cb->use_easy_dma)
    {
        if (!xincx_is_in_ram(p_data))
        {
            err_code = XINCX_ERROR_INVALID_ADDR;
            XINCX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             XINCX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
    }
	//  printf("xincx_uart_tx progress:%d\r\n",xincx_uart_tx_in_progress(p_instance));
    if (xincx_uarte_tx_in_progress(p_instance))
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
    
  
                //- TX Channel
    xincx_dmas_ch_set_t set;

    set.ch = p_cb->tx_dma_ch;
    set.src_addr = (uint32_t)p_cb->p_tx_buffer;
    set.dst_addr = (uint32_t)&(p_instance->p_reg->RBR_THR_DLL.THR);
    set.ctl0 = p_cb->tx_buffer_length;
    set.ctl1 = (2 << 8);
    xincx_dmas_ch_param_set(set);


  //  printf("tx start addr:0x%x,len:%d\r\n",(uint32_t)p_cb->p_tx_buffer,p_cb->tx_buffer_length);
    
    xincx_dmas_int_enable(0x01 << p_cb->tx_dma_ch);
    
    p_instance->p_reg->IER_DLH.IER = UART_UARTx_IER_ERDAI_Msk | UART_UARTx_IER_ETHEI_Msk | UART_UARTx_IER_PTIME_Msk;
    
    xincx_dmas_ch_enable(p_cb->tx_dma_ch);

    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

bool xincx_uarte_tx_in_progress(xincx_uarte_t const * p_instance)
{
//    printf("tx_buffer_length:%d\r\n",m_cb[p_instance->drv_inst_idx].tx_buffer_length);
    return (m_cb[p_instance->drv_inst_idx].tx_buffer_length != 0);
}


//static void rx_byte(XINC_UART_Type * p_uart, uarte_control_block_t * p_cb)
//{
//    if (!p_cb->rx_buffer_length)
//    {
//        // Byte received when buffer is not set - data lost.
//        (void) xinc_uart_rxd_get(p_uart);
//        return;
//    }
//    p_cb->p_rx_buffer[p_cb->rx_counter] = xinc_uart_rxd_get(p_uart);
//    p_cb->rx_counter++;
//}

xincx_err_t xincx_uarte_rx(xincx_uarte_t const * p_instance,
                        uint8_t *           p_data,
                        size_t              length)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    XINCX_ASSERT(m_cb[p_instance->drv_inst_idx].state == XINCX_DRV_STATE_INITIALIZED);
    XINCX_ASSERT(p_data);
    XINCX_ASSERT(length > 0);

    xincx_err_t err_code;

     // EasyDMA requires that transfer buffers are placed in DataRAM,
    // signal error if the are not.
  
    if (!xincx_is_in_ram(p_data))
    {
        err_code = XINCX_ERROR_INVALID_ADDR;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    

    
    bool second_buffer = false;

 //   printf("rx_buffer_length:%d, rx_secondary_buffer_length:%d\r\n",p_cb->rx_buffer_length,p_cb->rx_secondary_buffer_length);
    if (p_cb->rx_buffer_length != 0)
    {
        if (p_cb->rx_secondary_buffer_length != 0)
        {
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
    

            //- RX Channel
    xincx_dmas_ch_set_t set;

    set.ch = p_cb->rx_dma_ch;
    set.src_addr = (uint32_t)&(p_instance->p_reg->RBR_THR_DLL.RBR);
    set.dst_addr = (uint32_t)p_data;
    set.ctl0 = length;
    set.ctl1 = ((2 << 8)|  0 | 0xfff << 16);
    xincx_dmas_ch_param_set(set);
 
    xincx_dmas_int_enable(0x01 << p_cb->rx_dma_ch | 0x01 << (p_cb->rx_dma_ch + 16));
 
    
//    printf("xincx_uarte_rx:0x%p,0x%p\n",p_data,p_cb->p_rx_buffer);	
  
    p_instance->p_reg->IER_DLH.IER |= UART_UARTx_IER_ERDAI_Msk | UART_UARTx_IER_PTIME_Msk;//open rx interrupt
    xincx_dmas_ch_enable(p_cb->rx_dma_ch);

  
    
    err_code = XINCX_SUCCESS;
    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

bool xincx_uarte_rx_ready(xincx_uarte_t const * p_instance)
{
    return xinc_uart_event_check(p_instance->p_reg, XINC_UART_EVENT_RXDRDY);
}

void xincx_uarte_rx_enable(xincx_uarte_t const * p_instance)
{
    if (!m_cb[p_instance->drv_inst_idx].rx_enabled)
    {
        m_cb[p_instance->drv_inst_idx].rx_enabled = true;
    }
}

void xincx_uarte_rx_disable(xincx_uarte_t const * p_instance)
{

    m_cb[p_instance->drv_inst_idx].rx_enabled = false;
}

uint32_t xincx_uarte_errorsrc_get(xincx_uarte_t const * p_instance)
{
//    xinc_uart_event_clear(p_instance->p_reg, XINC_UART_EVENT_ERROR);
    return xinc_uart_errorsrc_get_and_clear(p_instance->p_reg);
}

static void rx_ready_event(uarte_control_block_t * p_cb,
                          size_t                 bytes,
                          uint8_t *              p_data)
{
    xincx_uarte_event_t event;

    event.type             = XINCX_UARTE_EVT_RX_READY;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = p_data;

    p_cb->handler(&event, p_cb->p_context);
}

static void rx_done_event(uarte_control_block_t * p_cb,
                          size_t                 bytes,
                          uint8_t *              p_data)
{
    xincx_uarte_event_t event;

    event.type             = XINCX_UARTE_EVT_RX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = p_data;

    p_cb->handler(&event, p_cb->p_context);
}


static void tx_done_event(uarte_control_block_t * p_cb,
                          size_t                 bytes)
{
    xincx_uarte_event_t event;

    event.type             = XINCX_UARTE_EVT_TX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = (uint8_t *)p_cb->p_tx_buffer;

    p_cb->tx_buffer_length = 0;

    p_cb->handler(&event, p_cb->p_context);
}

void xincx_uarte_tx_abort(xincx_uarte_t const * p_instance)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    p_cb->tx_abort = true;
    if (p_cb->handler)
    {
        tx_done_event(p_cb, p_cb->tx_counter);
    }

    XINCX_LOG_INFO("TX transaction aborted.");
    printf("xincx_uart_tx_abort\r\n");
}

void xincx_uarte_rx_abort(xincx_uarte_t const * p_instance)
{
    xinc_uart_int_disable(p_instance->p_reg, XINC_UART_INT_MASK_RXDRDY |
                                        XINC_UART_INT_MASK_ERROR);

    XINCX_LOG_INFO("RX transaction aborted.");

    printf("xincx_uart_rx_abort\r\n");
}

static void uarte_irq_dma_tx_handler(XINC_UART_Type *        p_uart,
                             uarte_control_block_t * p_cb)
{
    
    uint32_t TX_ADDR;
    size_t const tx_buffer_length = p_cb->tx_buffer_length;
        	
    xincx_dmas_ch_ca_get(p_cb->tx_dma_ch,&TX_ADDR);
    
    p_cb->tx_counter = TX_ADDR - (uint32_t)p_cb->p_tx_buffer;
  //  printf("tx addr:0x%x,start:0x%x,tx_len:%d,\r\n",TX_ADDR,(uint32_t)p_cb->p_tx_buffer,p_cb->tx_counter);
    if (tx_buffer_length)
    {
        
        tx_done_event(p_cb, tx_buffer_length);
    }
    
 

     
}
static void uarte_irq_dma_rx_handler(XINC_UART_Type *        p_uart,
                             uarte_control_block_t * p_cb)
{
    uint32_t IIR,iWK,RD_ADDR;
    uint32_t rx_len;

    xincx_dmas_ch_ca_get(p_cb->rx_dma_ch,&RD_ADDR);
    
  //  iWK = 
    
    rx_len = RD_ADDR - (uint32_t)p_cb->p_rx_buffer;
 //   printf("rx_len:%d\r\n",rx_len);
    IIR = p_uart->IIR_FCR.IIR;

    p_cb->rx_buffer_length = 0;

    if((iWK == 0x200) && (IIR == 0xc1))
    {
        rx_done_event(p_cb, rx_len, p_cb->p_rx_buffer);
    }else{
        rx_ready_event(p_cb, rx_len, p_cb->p_rx_buffer);
    }
     
}


#if XINCX_CHECK(XINCX_UARTE0_ENABLED)
void uarte0_irq_dma_ch_handler(xincx_dmas_ch_evt_t const * p_event,
                                        void *                 p_context)
{
    uarte_control_block_t * p_cb = &m_cb[XINCX_UARTE0_INST_IDX];
    if(p_event->dmas_ch == p_cb->rx_dma_ch)
    {
        uarte_irq_dma_rx_handler(XINC_UART1, &m_cb[XINCX_UARTE0_INST_IDX]);
    }else if(p_event->dmas_ch == p_cb->tx_dma_ch)
    {
        uarte_irq_dma_tx_handler(XINC_UART1, &m_cb[XINCX_UARTE0_INST_IDX]);
    }
    
}
#endif

#if XINCX_CHECK(XINCX_UARTE1_ENABLED)
void uarte1_irq_dma_ch_handler(xincx_dmas_ch_evt_t const * p_event,
                                        void *                 p_context)
{   
    uarte_control_block_t * p_cb = &m_cb[XINCX_UARTE1_INST_IDX];
    if(p_event->dmas_ch == p_cb->rx_dma_ch)
    {
        uarte_irq_dma_rx_handler(XINC_UART1, &m_cb[XINCX_UARTE1_INST_IDX]);
    }else if(p_event->dmas_ch == p_cb->tx_dma_ch)
    {
        uarte_irq_dma_tx_handler(XINC_UART1, &m_cb[XINCX_UARTE1_INST_IDX]);
    }
}
#endif


#endif // XINCX_CHECK(XINCX_UART_ENABLED)
