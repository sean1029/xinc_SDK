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

#include <xincx.h>

#if XINCX_CHECK(XINCX_I2S_ENABLED)

#include <xincx_i2sm.h>
#include <hal/xinc_gpio.h>
#include <xincx_dmas.h>
#include "xinc_delay.h"
#define XINCX_LOG_MODULE I2SMM
#include <xincx_log.h>

#define EVT_TO_STR(event)                                         \
    (event == XINC_I2SM_EVENT_RXPTRUPD ? "XINC_I2SM_EVENT_RXPTRUPD" : \
    (event == XINC_I2SM_EVENT_TXPTRUPD ? "XINC_I2SM_EVENT_TXPTRUPD" : \
    (event == XINC_I2SM_EVENT_STOPPED  ? "XINC_I2SM_EVENT_STOPPED"  : \
                                       "UNKNOWN EVENT")))


// Control block - driver instance local data.
typedef struct
{
    xincx_i2sm_data_handler_t handler;
    void *                     context;
    xincx_drv_state_t        state;
    xinc_i2sm_event_t        event;

    bool use_rx         : 1;
    bool use_tx         : 1;
    bool rx_ready       : 1;
    bool tx_ready       : 1;
    bool buffers_needed : 1;
    bool buffers_reused : 1;

    uint16_t            buffer_size;
    xinc_dma_ch_t       tx_dma_ch;
    xinc_dma_ch_t       rx_dma_ch;
    xincx_i2sm_buffers_t  next_buffers;
    xincx_i2sm_buffers_t  current_buffers;
} i2sm_control_block_t;
static i2sm_control_block_t m_cb[XINCX_I2SM_ENABLED_COUNT];

static void xincx_i2sm_dma_irq_ch_handler(xincx_dmas_ch_evt_t const * p_event,
                                        void *                 p_context);

static void xincx_i2sm_clk_init(xincx_i2sm_t const *        p_instance,
                         xincx_i2sm_config_t const * p_config)
{
    XINC_CPR_CTL_Type * p_cpr = (XINC_CPR_CTL_Type *)p_instance->p_cpr;
    
    XINC_I2S_Type * p_i2sm = p_instance->p_i2sm;    
    
    uint32_t reg_val;

    //复位I2S模块
    p_cpr->RSTCTL_SUBRST_SW =   (CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Enable << CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Pos) |
                                (CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET);

    xinc_delay_ms(XINCX_RST_DELAY_MS);
    
    p_cpr->RSTCTL_SUBRST_SW =   (CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Disable << CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Pos) |
                                (CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET);

    
    p_cpr->CTLAPBCLKEN_GRCTL =  (CPR_CTLAPBCLKEN_GRCTL_I2C_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_I2C_PCLK_EN_Pos) |
                               (CPR_CTLAPBCLKEN_GRCTL_I2C_PCLK_EN_Msk << CPR_CTLAPBCLKEN_GRCTL_I2C_PCLK_EN_Pos);

    //打开I2S pclk and mclk   
    reg_val = p_cpr->I2S_CLK_CTL;
    reg_val |= (CPR_I2S_CLK_CTL_I2S_MCLK_EN_Enable << CPR_I2S_CLK_CTL_I2S_MCLK_EN_Pos) | (CPR_I2S_CLK_CTL_I2S_PCLK_EN_Enable << CPR_I2S_CLK_CTL_I2S_PCLK_EN_Pos);
    p_cpr->I2S_CLK_CTL = reg_val;
    
    reg_val = p_i2sm->CLOCK_GEN;
    
    if (p_config->mode == XINC_I2SM_MODE_MASTER)
    {
        reg_val |= I2S_I2S_CLOCK_GEN_MASTEREN_Msk; 
    }
    else
    {
        reg_val &= ~I2S_I2S_CLOCK_GEN_MASTEREN_Msk;
    }
    
    if(p_config->format == XINC_I2SM_FORMAT_I2S)
    {
        reg_val &= ~I2S_I2S_CLOCK_GEN_WS_FORMAT_Msk;
        reg_val |= (I2S_I2S_CLOCK_GEN_WS_FORMA_Standard << I2S_I2S_CLOCK_GEN_WS_FORMAT_Pos); 
        
        reg_val &= ~I2S_I2S_CLOCK_GEN_WS_POLARITY_I2S_Msk;
        reg_val |= (I2S_I2S_CLOCK_GEN_WS_POLARITY_I2S_0L_1R << I2S_I2S_CLOCK_GEN_WS_POLARITY_I2S_Pos);
        
        reg_val |= (I2S_I2S_CLOCK_GEN_WS_WIDTH_I2S_2_SCLK_cycle << I2S_I2S_CLOCK_GEN_WS_WIDTH_I2S_Pos);
    }
    else
    {
    
    
    }
    p_i2sm->CLOCK_GEN = reg_val ;
    
    reg_val = p_i2sm->CONFIG;
    
    reg_val &= ~I2S_I2S_CONFIG_SLOT_WIDTH_Msk;
    reg_val |= p_config->sample_width;
    

    p_i2sm->CONFIG = reg_val;
    printf("p_config->alignment:%d\r\n",p_config->alignment);
    printf("p_config->format:%d\r\n",p_config->format);
    
    printf("p_config->sample_width:%d\r\n",p_config->sample_width);
    printf("p_config->ratio:%d\r\n",p_config->ratio);
  

   


}

static xincx_err_t configure_pins(xincx_i2sm_t const *        p_instance,
                                    xincx_i2sm_config_t const * p_config)
{
    uint32_t mck_pin,sck_pin,lrck_pin,sdout_pin, sdin_pin;

    xincx_err_t err_code = XINCX_SUCCESS;
    // Configure pins used by the peripheral:

    // - SCK and LRCK (required) - depending on the mode of operation these
    //   pins are configured as outputs (in Master mode) or inputs (in Slave
    //   mode).
    if (p_config->mode == XINC_I2SM_MODE_MASTER)
    {
//        xinc_gpio_cfg_output(p_config->sck_pin);
//        xinc_gpio_cfg_output(p_config->lrck_pin);
    }
    else
    {
//        xinc_gpio_cfg_input(p_config->sck_pin,  XINC_GPIO_PIN_NOPULL);
//        xinc_gpio_cfg_input(p_config->lrck_pin, XINC_GPIO_PIN_NOPULL);
    }

    // - MCK (optional) - always output,
    if (p_config->mck_pin != XINCX_I2SM_PIN_NOT_USED)
    {
        mck_pin = p_config->mck_pin;
       
    }
    else
    {
        mck_pin = XINC_I2SM_PIN_NOT_CONNECTED;
        
    }

     // - SCK  - always output,
    if (p_config->sck_pin != XINCX_I2SM_PIN_NOT_USED)
    {
        sck_pin = p_config->sck_pin;
        err_code = xinc_gpio_secfun_config(p_config->sck_pin,XINC_GPIO_PIN_I2S_SCLK);
       
    }
    else
    {
        sck_pin = XINC_I2SM_PIN_NOT_CONNECTED;
        err_code =XINCX_ERROR_FORBIDDEN; 
    }
    
     // - LRCK  - always output,
    if (p_config->lrck_pin != XINCX_I2SM_PIN_NOT_USED)
    {
        lrck_pin = p_config->lrck_pin;
        err_code = xinc_gpio_secfun_config(p_config->lrck_pin,XINC_GPIO_PIN_I2S_WS);
       
    }
    else
    {
        lrck_pin = XINC_I2SM_PIN_NOT_CONNECTED;
        err_code =XINCX_ERROR_FORBIDDEN;
    }
     
    
    // - SDOUT (optional) - always output,
    if (p_config->sdout_pin != XINCX_I2SM_PIN_NOT_USED)
    {
        sdout_pin = p_config->sdout_pin;
        err_code = xinc_gpio_secfun_config(p_config->sdout_pin,XINC_GPIO_PIN_I2S_DOUT);
    }
    else
    {
        sdout_pin = XINC_I2SM_PIN_NOT_CONNECTED;
        err_code =XINCX_ERROR_FORBIDDEN;
    }

    // - SDIN (optional) - always input.
    if (p_config->sdin_pin != XINCX_I2SM_PIN_NOT_USED)
    {
        sdin_pin = p_config->sdin_pin;
        err_code = xinc_gpio_secfun_config(p_config->sdin_pin,XINC_GPIO_PIN_I2S_DIN);

    }
    else
    {
        sdin_pin = XINC_I2SM_PIN_NOT_CONNECTED;
        err_code =XINCX_ERROR_FORBIDDEN;
    }
    
    (void) mck_pin;
    (void) sck_pin;
    (void) lrck_pin;
    (void) sdout_pin;
    (void) sdin_pin;

    return err_code;
}


static void xincx_i2s_dma_ch_set(xincx_i2sm_t const * const  p_instance )
{
    i2sm_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
  
    p_cb->tx_dma_ch = DMAS_CH_SEND_I2S;
    p_cb->rx_dma_ch = DMAS_CH_RCV_I2S;
    xincx_dmas_ch_handler_register(p_cb->tx_dma_ch,xincx_i2sm_dma_irq_ch_handler); 
    xincx_dmas_ch_handler_register(p_cb->rx_dma_ch,xincx_i2sm_dma_irq_ch_handler); 
}


xincx_err_t xincx_i2sm_init(xincx_i2sm_t const *        p_instance,
                            xincx_i2sm_config_t const * p_config,
                            xincx_i2sm_data_handler_t   handler,
                             void *                    p_context)
{
    XINCX_ASSERT(p_config);
    XINCX_ASSERT(handler);

    xincx_err_t err_code;
    
    
    i2sm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
    
    XINC_I2S_Type * p_i2sm = p_instance->p_i2sm;

    if (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

  
    
    if (!xinc_i2sm_configure_check(p_i2sm,
                           p_config->mode,
                           p_config->format,
                           p_config->alignment,
                           p_config->sample_width,
                           p_config->channels,
                           p_config->mck_setup,
                           p_config->ratio))
    {
        err_code = XINCX_ERROR_INVALID_PARAM;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    
    xincx_i2sm_clk_init(p_instance,p_config);
    
    err_code = configure_pins(p_instance,p_config);
    
    if(err_code != XINCX_SUCCESS)
    {
        return err_code;
    }

    p_cb->handler = handler;
    
    xincx_i2s_dma_ch_set(p_instance);

    XINCX_IRQ_PRIORITY_SET(I2S_IRQn, p_config->irq_priority);
    XINCX_IRQ_ENABLE(I2S_IRQn);

    p_cb->state = XINCX_DRV_STATE_INITIALIZED;

    XINCX_LOG_INFO("Initialized.");
    return XINCX_SUCCESS;
}


void xincx_i2sm_uninit(xincx_i2sm_t const *        p_instance)
{
    i2sm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
    
    XINC_I2S_Type * p_i2sm = p_instance->p_i2sm;
    
    XINCX_ASSERT(p_cb->state != XINCX_DRV_STATE_UNINITIALIZED);

    xincx_i2sm_stop(p_instance);

    XINCX_IRQ_DISABLE(I2S_IRQn);


    p_cb->state = XINCX_DRV_STATE_UNINITIALIZED;
    XINCX_LOG_INFO("Uninitialized.");
}


void xinc_i2sm_rx_buffer_set(xincx_i2sm_t const *        p_instance,uint16_t         size,
                                           uint32_t * p_buffer)
{
    i2sm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];

    p_cb->event |= XINC_I2SM_EVENT_RXPTRUPD;
    //- RX Channel
    xincx_dmas_ch_set_t set;
    set.ch = p_cb->rx_dma_ch;
    set.src_addr = (uint32_t)&(p_instance->p_i2sm->RXFIFO_DATA);
    set.dst_addr = (uint32_t)p_buffer;
    set.ctl0 = size;
    set.ctl1 = ((2 << 8)|  0 | 0xfff << 16);
    xincx_dmas_ch_param_set(set);
    xincx_dmas_int_enable((0x01 << p_cb->rx_dma_ch) | (0x01 << (p_cb->rx_dma_ch + 16)));
    
    xincx_dmas_ch_enable(p_cb->rx_dma_ch);
}


void xinc_i2sm_tx_buffer_set(xincx_i2sm_t const *        p_instance,uint16_t         size,
                                           uint32_t const * p_buffer)
{
    i2sm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];

    p_cb->event |= XINC_I2SM_EVENT_TXPTRUPD;
    //- TX Channel
    xincx_dmas_ch_set_t set;

    set.ch = p_cb->tx_dma_ch;
    set.src_addr = (uint32_t)p_buffer;
    set.dst_addr = (uint32_t)&(p_instance->p_i2sm->TXFIFO_DATA);
    set.ctl0 = size;
    set.ctl1 = ((2 << 8) |  (0xfff << 16));
    xincx_dmas_ch_param_set(set);
    xincx_dmas_int_enable((0x01 << p_cb->tx_dma_ch) | (0x01 << (p_cb->tx_dma_ch + 16)));
    
     xincx_dmas_ch_enable(p_cb->rx_dma_ch);
}


void xincx_i2sm_transfer_set(xincx_i2sm_t const *        p_instance ,
                                          uint16_t         size,
                                          uint32_t *       p_buffer_rx,
                                          uint32_t const * p_buffer_tx)
{
    XINC_I2S_Type *   p_i2sm = p_instance->p_i2sm;
    
    i2sm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
      

    if(p_buffer_rx != NULL)
    {             
        p_i2sm->DMA_CONFIG &= ~I2S_I2S_DMA_CONFIG_DMA_RXLVL_Msk;
        p_i2sm->DMA_CONFIG |= (0x8UL << I2S_I2S_DMA_CONFIG_DMA_RXLVL_Pos); 
        p_i2sm->DMA_CONFIG |= (I2S_I2S_DMA_CONFIG_DMA_RX_EN_Enable << I2S_I2S_DMA_CONFIG_DMA_RX_EN_Pos);
        xinc_i2sm_rx_buffer_set(p_instance,size,p_buffer_rx);
    }
    
    if(p_buffer_tx != NULL)      
    {
        p_i2sm->DMA_CONFIG &= ~I2S_I2S_DMA_CONFIG_DMA_TXLVL_Msk;
        p_i2sm->DMA_CONFIG |= (0x8UL << I2S_I2S_DMA_CONFIG_DMA_TXLVL_Pos);
        p_i2sm->DMA_CONFIG |= (I2S_I2S_DMA_CONFIG_DMA_RX_EN_Enable << I2S_I2S_DMA_CONFIG_DMA_RX_EN_Pos);
        xinc_i2sm_tx_buffer_set(p_instance,size,p_buffer_tx);
    }
}

xincx_err_t xincx_i2sm_start(xincx_i2sm_t const *        p_instance,
                            xincx_i2sm_buffers_t const * p_initial_buffers,
                          uint16_t                   buffer_size,
                          uint8_t                    flags)
{
    i2sm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
    
    XINC_I2S_Type * p_i2sm = p_instance->p_i2sm;
    
    XINCX_ASSERT(p_initial_buffers != NULL);
    XINCX_ASSERT(p_initial_buffers->p_rx_buffer != NULL ||
                p_initial_buffers->p_tx_buffer != NULL);
    XINCX_ASSERT((p_initial_buffers->p_rx_buffer == NULL) ||
                (xincx_is_in_ram(p_initial_buffers->p_rx_buffer) &&
                 xincx_is_word_aligned(p_initial_buffers->p_rx_buffer)));
    XINCX_ASSERT((p_initial_buffers->p_tx_buffer == NULL) ||
                (xincx_is_in_ram(p_initial_buffers->p_tx_buffer) &&
                 xincx_is_word_aligned(p_initial_buffers->p_tx_buffer)));
    XINCX_ASSERT(buffer_size != 0);
    (void)(flags);

    xincx_err_t err_code;

    if (p_cb->state != XINCX_DRV_STATE_INITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    if (((p_initial_buffers->p_rx_buffer != NULL)
         && !xincx_is_in_ram(p_initial_buffers->p_rx_buffer))
        ||
        ((p_initial_buffers->p_tx_buffer != NULL)
         && !xincx_is_in_ram(p_initial_buffers->p_tx_buffer)))
    {
        err_code = XINCX_ERROR_INVALID_ADDR;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    p_cb->use_rx         = (p_initial_buffers->p_rx_buffer != NULL);
    p_cb->use_tx         = (p_initial_buffers->p_tx_buffer != NULL);
    p_cb->rx_ready       = false;
    p_cb->tx_ready       = false;
    p_cb->buffers_needed = false;
    p_cb->buffer_size    = buffer_size;

    // Set the provided initial buffers as next, they will become the current
    // ones after the IRQ handler is called for the first time, what will occur
    // right after the START task is triggered.
    p_cb->next_buffers = *p_initial_buffers;
    p_cb->current_buffers.p_rx_buffer = NULL;
    p_cb->current_buffers.p_tx_buffer = NULL;

    p_cb->event = XINC_I2SM_EVENT_NONE;

    xincx_i2sm_transfer_set(p_instance,
                         p_cb->buffer_size,
                         p_cb->next_buffers.p_rx_buffer,
                         p_cb->next_buffers.p_tx_buffer);

    xinc_i2sm_enable(p_i2sm);

    p_cb->state = XINCX_DRV_STATE_POWERED_ON;


    xinc_i2sm_int_enable(p_i2sm, (p_cb->use_rx ? XINC_I2SM_INT_RXPTRUPD_MASK : 0) |
                                (p_cb->use_tx ? XINC_I2SM_INT_TXPTRUPD_MASK : 0) |
                                XINC_I2SM_INT_STOPPED_MASK);

    XINCX_LOG_INFO("Started.");
    return XINCX_SUCCESS;
}


xincx_err_t xincx_i2sm_next_buffers_set(xincx_i2sm_t const *        p_instance,
                                        xincx_i2sm_buffers_t const * p_buffers)
{
    i2sm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
    
    XINC_I2S_Type * p_i2sm = p_instance->p_i2sm;
    
    XINCX_ASSERT(p_cb->state == XINCX_DRV_STATE_POWERED_ON);
    XINCX_ASSERT(p_buffers);
    XINCX_ASSERT((p_buffers->p_rx_buffer == NULL) ||
                (xincx_is_in_ram(p_buffers->p_rx_buffer) &&
                 xincx_is_word_aligned(p_buffers->p_rx_buffer)));
    XINCX_ASSERT((p_buffers->p_tx_buffer == NULL) ||
                (xincx_is_in_ram(p_buffers->p_tx_buffer) &&
                 xincx_is_word_aligned(p_buffers->p_tx_buffer)));

    xincx_err_t err_code;

    if (!p_cb->buffers_needed)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    if (((p_buffers->p_rx_buffer != NULL)
         && !xincx_is_in_ram(p_buffers->p_rx_buffer))
        ||
        ((p_buffers->p_tx_buffer != NULL)
         && !xincx_is_in_ram(p_buffers->p_tx_buffer)))
    {
        err_code = XINCX_ERROR_INVALID_ADDR;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    if (p_cb->use_tx)
    {
        XINCX_ASSERT(p_buffers->p_tx_buffer != NULL);
        xinc_i2sm_tx_buffer_set(p_instance,p_cb->buffer_size, p_buffers->p_tx_buffer);
    }
    if (p_cb->use_rx)
    {
        XINCX_ASSERT(p_buffers->p_rx_buffer != NULL);
        xinc_i2sm_rx_buffer_set(p_instance,p_cb->buffer_size, p_buffers->p_rx_buffer);
    }

    p_cb->next_buffers   = *p_buffers;
    p_cb->buffers_needed = false;

    return XINCX_SUCCESS;
}


void xincx_i2sm_stop(xincx_i2sm_t const *        p_instance)
{
    i2sm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
    
    XINC_I2S_Type * p_i2sm = p_instance->p_i2sm;
    
    p_cb->event = XINC_I2SM_EVENT_STOPPED;
    XINCX_ASSERT(p_cb->state != XINCX_DRV_STATE_UNINITIALIZED);

    p_cb->buffers_needed = false;

    // First disable interrupts, then trigger the STOP task, so no spurious
    // RXPTRUPD and TXPTRUPD events (see nRF52 anomaly 55) are processed.
    // xinc_i2sm_int_disable(p_i2sm, XINC_I2SM_INT_RXPTRUPD_MASK |
    //                              XINC_I2SM_INT_TXPTRUPD_MASK);


}

static void i2s_irq_handler(XINC_I2S_Type * p_i2sm, i2sm_control_block_t * p_cb)
{
    
    if (p_cb->event & XINC_I2SM_EVENT_TXPTRUPD)
    {
        p_cb->event &= ~XINC_I2SM_EVENT_TXPTRUPD;
        
        p_cb->tx_ready = true;
        if (p_cb->use_tx && p_cb->buffers_needed)
        {
            p_cb->buffers_reused = true;
        }
    }
  
    if (p_cb->event & XINC_I2SM_EVENT_RXPTRUPD)
    {
        p_cb->event &= ~XINC_I2SM_EVENT_RXPTRUPD;
        
        p_cb->rx_ready = true;
        if ( p_cb->use_rx && p_cb->buffers_needed)
        {
             p_cb->buffers_reused = true;
        }
    }

    if (p_cb->event & XINC_I2SM_EVENT_STOPPED)
    {
        p_cb->event &= ~XINC_I2SM_EVENT_STOPPED;
        
      //  xinc_i2sm_int_disable(p_i2sm, XINC_I2SM_INT_STOPPED_MASK);
        xinc_i2sm_disable(p_i2sm);

        // When stopped, release all buffers, including these scheduled for
        // the next transfer.
        p_cb->handler(&p_cb->current_buffers, 0,p_cb->context);
        p_cb->handler(&p_cb->next_buffers, 0,p_cb->context);

        p_cb->state = XINCX_DRV_STATE_INITIALIZED;
        XINCX_LOG_INFO("Stopped.");
    }
    else
    {
        // Check if the requested transfer has been completed:
        // - full-duplex mode
        if ((p_cb->use_tx && p_cb->use_rx && p_cb->tx_ready && p_cb->rx_ready) ||
            // - TX only mode
            (!p_cb->use_rx && p_cb->tx_ready) ||
            // - RX only mode
            (!p_cb->use_tx && p_cb->rx_ready))
        {
            p_cb->tx_ready = false;
            p_cb->rx_ready = false;

            // If the application did not supply the buffers for the next
            // part of the transfer until this moment, the current buffers
            // cannot be released, since the I2SM peripheral already started
            // using them. Signal this situation to the application by
            // passing NULL instead of the structure with released buffers.
            if (p_cb->buffers_reused)
            {
                p_cb->buffers_reused = false;
                // This will most likely be set at this point. However, there is
                // a small time window between TXPTRUPD and RXPTRUPD events,
                // and it is theoretically possible that next buffers will be
                // set in this window, so to be sure this flag is set to true,
                // set it explicitly.
                p_cb->buffers_needed = true;
                p_cb->handler(NULL,
                             XINCX_I2SM_STATUS_NEXT_BUFFERS_NEEDED,p_cb->context);
            }
            else
            {
                // Buffers that have been used by the I2SM peripheral (current)
                // are now released and will be returned to the application,
                // and the ones scheduled to be used as next become the current
                // ones.
                xincx_i2sm_buffers_t released_buffers = p_cb->current_buffers;
                p_cb->current_buffers = p_cb->next_buffers;
                p_cb->next_buffers.p_rx_buffer = NULL;
                p_cb->next_buffers.p_tx_buffer = NULL;
                p_cb->buffers_needed = true;
                p_cb->handler(&released_buffers,
                             XINCX_I2SM_STATUS_NEXT_BUFFERS_NEEDED,p_cb->context);
            }

        }
    }
}    



void xincx_i2sm0_irq_handler(void)
{
    i2s_irq_handler(XINC_I2S0,&m_cb[XINCX_I2SM0_INST_IDX]);
}

static void xincx_i2sm_dma_irq_ch_handler(xincx_dmas_ch_evt_t const * p_event,
                                        void *                 p_context)
{
    i2sm_control_block_t * p_cb = &m_cb[XINCX_I2SM0_INST_IDX];
    if(p_event->dmas_ch == p_cb->rx_dma_ch)
    {
        i2s_irq_handler(XINC_I2S0,&m_cb[XINCX_I2SM0_INST_IDX]);
    }else if(p_event->dmas_ch == p_cb->tx_dma_ch)
    {
        i2s_irq_handler(XINC_I2S0,&m_cb[XINCX_I2SM0_INST_IDX]);
    }

}


#endif // XINCX_CHECK(XINCX_I2SMM_ENABLED)
