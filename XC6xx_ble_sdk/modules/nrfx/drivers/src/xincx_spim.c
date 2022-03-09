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

#include <nrfx.h>
#include "bsp_register_macro.h"
#include "bsp_clk.h"
#if NRFX_CHECK(XINCX_SPIM_ENABLED)

#if !(NRFX_CHECK(XINCX_SPIM0_ENABLED) || NRFX_CHECK(XINCX_SPIM1_ENABLED) || \
      NRFX_CHECK(XINCX_SPIM2_ENABLED) || NRFX_CHECK(XINCX_SPIM3_ENABLED))
#error "No enabled SPIM instances. Check <nrfx_config.h>."
#endif

#if NRFX_CHECK(XINCX_SPIM_EXTENDED_ENABLED) && !NRFX_CHECK(XINCX_SPIM3_ENABLED)
#error "Extended options are available only in SPIM3 on the nRF52840 SoC."
#endif

#include <xincx_spim.h>
#include <hal/nrf_gpio.h>

#define NRFX_LOG_MODULE SPIM
#include <nrfx_log.h>

#define SPIMX_LENGTH_VALIDATE(peripheral, drv_inst_idx, rx_len, tx_len) \
    (((drv_inst_idx) == NRFX_CONCAT_3(XINCX_, peripheral, _INST_IDX)) && \
     NRFX_EASYDMA_LENGTH_VALIDATE(peripheral, rx_len, tx_len))

#if NRFX_CHECK(XINCX_SPIM0_ENABLED)
#define SPIM0_LENGTH_VALIDATE(...)  SPIMX_LENGTH_VALIDATE(SPIM0, __VA_ARGS__)
#else
#define SPIM0_LENGTH_VALIDATE(...)  0
#endif

#if NRFX_CHECK(XINCX_SPIM1_ENABLED)
#define SPIM1_LENGTH_VALIDATE(...)  SPIMX_LENGTH_VALIDATE(SPIM1, __VA_ARGS__)
#else
#define SPIM1_LENGTH_VALIDATE(...)  0
#endif

#if NRFX_CHECK(XINCX_SPIM2_ENABLED)
#define SPIM2_LENGTH_VALIDATE(...)  SPIMX_LENGTH_VALIDATE(SPIM2, __VA_ARGS__)
#else
#define SPIM2_LENGTH_VALIDATE(...)  0
#endif

#if NRFX_CHECK(XINCX_SPIM3_ENABLED)
#define SPIM3_LENGTH_VALIDATE(...)  SPIMX_LENGTH_VALIDATE(SPIM3, __VA_ARGS__)
#else
#define SPIM3_LENGTH_VALIDATE(...)  0
#endif

#define SPIM_LENGTH_VALIDATE(drv_inst_idx, rx_len, tx_len)  \
    (SPIM0_LENGTH_VALIDATE(drv_inst_idx, rx_len, tx_len) || \
     SPIM1_LENGTH_VALIDATE(drv_inst_idx, rx_len, tx_len) || \
     SPIM2_LENGTH_VALIDATE(drv_inst_idx, rx_len, tx_len) || \
     SPIM3_LENGTH_VALIDATE(drv_inst_idx, rx_len, tx_len))

#if defined(NRF52840_XXAA) && (NRFX_CHECK(XINCX_SPIM3_ENABLED))
// Enable workaround for nRF52840 anomaly 195 (SPIM3 continues to draw current after disable).
#define USE_WORKAROUND_FOR_ANOMALY_195
#endif

// Control block - driver instance local data.
typedef struct
{
    xincx_spim_evt_handler_t handler;
    void *                  p_context;
    xincx_spim_evt_t         evt;  // Keep the struct that is ready for event handler. Less memcpy.
    nrfx_drv_state_t        state;
    volatile bool           transfer_in_progress;

#if NRFX_CHECK(XINCX_SPIM_EXTENDED_ENABLED)
    bool                    use_hw_ss;
#endif

    // [no need for 'volatile' attribute for the following members, as they
    //  are not concurrently used in IRQ handlers and main line code]
    bool            ss_active_high;
    uint8_t         ss_pin;
    uint8_t         miso_pin;
    uint8_t         orc;

#if NRFX_CHECK(XINCX_SPIM_NRF52_ANOMALY_109_WORKAROUND_ENABLED)
    size_t          tx_length;
    size_t          rx_length;
#endif
		uint8_t tx_dma_ch;
		uint8_t rx_dma_ch;
} spim_control_block_t;
static spim_control_block_t m_cb[XINCX_SPIM_ENABLED_COUNT];

#if NRFX_CHECK(XINCX_SPIM3_NRF52840_ANOMALY_198_WORKAROUND_ENABLED)

// Workaround for nRF52840 anomaly 198: SPIM3 transmit data might be corrupted.

static uint32_t m_anomaly_198_preserved_value;

static void anomaly_198_enable(uint8_t const * p_buffer, size_t buf_len)
{
    m_anomaly_198_preserved_value = *((volatile uint32_t *)0x40000E00);

    if (buf_len == 0)
    {
        return;
    }
    uint32_t buffer_end_addr = ((uint32_t)p_buffer) + buf_len;
    uint32_t block_addr      = ((uint32_t)p_buffer) & ~0x1FFF;
    uint32_t block_flag      = (1UL << ((block_addr >> 13) & 0xFFFF));
    uint32_t occupied_blocks = 0;

    if (block_addr >= 0x20010000)
    {
        occupied_blocks = (1UL << 8);
    }
    else
    {
        do {
            occupied_blocks |= block_flag;
            block_flag <<= 1;
            block_addr  += 0x2000;
        } while ((block_addr < buffer_end_addr) && (block_addr < 0x20012000));
    }

    *((volatile uint32_t *)0x40000E00) = occupied_blocks;
}

static void anomaly_198_disable(void)
{
    *((volatile uint32_t *)0x40000E00) = m_anomaly_198_preserved_value;
}
#endif // NRFX_CHECK(XINCX_SPIM3_NRF52840_ANOMALY_198_WORKAROUND_ENABLED)


nrfx_err_t xincx_spim_init(xincx_spim_t  const * const p_instance,
                          xincx_spim_config_t const * p_config,
                          xincx_spim_evt_handler_t    handler,
                          void                     * p_context)
{
    NRFX_ASSERT(p_config);
    spim_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    nrfx_err_t err_code = NRFX_SUCCESS;

    if (p_cb->state != NRFX_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRFX_ERROR_INVALID_STATE;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

//		 if (p_instance->id != 1)
//    {
//        err_code = NRFX_ERROR_INVALID_PARAM;
//        NRFX_LOG_WARNING("Function: %s, error code: %s.",
//                         __func__,
//                         NRFX_LOG_ERROR_STRING_GET(err_code));
//        return err_code;
//    }

    XINC_SPIM_Type * p_spim = (XINC_SPIM_Type *)p_instance->p_reg;


    p_cb->handler = handler;
    p_cb->p_context = p_context;

    uint32_t mosi_pin;
    uint32_t miso_pin;
    // Configure pins used by the peripheral:
    // - SCK - output with initial value corresponding with the SPI mode used:
    //   0 - for modes 0 and 1 (CPOL = 0), 1 - for modes 2 and 3 (CPOL = 1);
    //   according to the reference manual guidelines this pin and its input
    //   buffer must always be connected for the SPI to work.

    // - MOSI (optional) - output with initial value 0,
		if(p_instance->id != 0)
		{
			if (p_config->mosi_pin != XINCX_SPIM_PIN_NOT_USED)
			{
					mosi_pin = p_config->mosi_pin;
					err_code = xinc_gpio_fun_config(mosi_pin,NRF_GPIO_PIN_SSI1_TX);
				
					if(err_code != NRFX_SUCCESS)
					{
						return err_code;
					}
			}
			else
			{
					mosi_pin = XINC_SPIM_PIN_NOT_CONNECTED;
					err_code = NRFX_ERROR_INVALID_PARAM;
					NRFX_LOG_WARNING("Function: %s, error code: %s.",
													 __func__,
													 NRFX_LOG_ERROR_STRING_GET(err_code));
					return err_code;
			}
			// - MISO (optional) - input,
			if (p_config->miso_pin != XINCX_SPIM_PIN_NOT_USED)
			{
					miso_pin = p_config->miso_pin;
					err_code = xinc_gpio_fun_config(miso_pin,NRF_GPIO_PIN_SSI1_RX);
				
					if(err_code != NRFX_SUCCESS)
					{
						return err_code;
					}
			}
			else
			{
					miso_pin = XINC_SPIM_PIN_NOT_CONNECTED;
					err_code = NRFX_ERROR_INVALID_PARAM;
					NRFX_LOG_WARNING("Function: %s, error code: %s.",
													 __func__,
													 NRFX_LOG_ERROR_STRING_GET(err_code));
					return err_code;
			}
			
			if (p_config->ss_pin != XINCX_SPIM_PIN_NOT_USED)
			{
		 
					p_cb->ss_active_high = p_config->ss_active_high;
					err_code = xinc_gpio_fun_config(p_config->ss_pin,NRF_GPIO_PIN_SSI1_SSN);
				
					if(err_code != NRFX_SUCCESS)
					{
						return err_code;
					}
			}
			
			if (p_config->sck_pin != XINCX_SPIM_PIN_NOT_USED)
			{
					err_code = xinc_gpio_fun_config(p_config->sck_pin,NRF_GPIO_PIN_SSI1_CLK);
				
					if(err_code != NRFX_SUCCESS)
					{
						return err_code;
					}
			}
		}
		miso_pin = p_config->miso_pin;
		mosi_pin = p_config->mosi_pin;
    p_cb->miso_pin = p_config->miso_pin;
    // - Slave Select (optional) - output with initial value 1 (inactive).

    // 'p_cb->ss_pin' variable is used during transfers to check if SS pin should be toggled,
    // so this field needs to be initialized even if the pin is not used.
    p_cb->ss_pin = p_config->ss_pin;
		
		printf("miso_pin:%d\n",miso_pin); 
		printf("mosi_pin:%d\n",mosi_pin); 
		printf("sck_pin:%d\n",p_config->sck_pin);
		printf("ss_pin:%d\n",p_config->ss_pin);		



		//	xc_spi_clk_init(ch);
				// __write_hw_reg32(CPR_SPIx_MCLK_CTL(ch), 0x110010);//1��Ƶ			//- spi(x)_mclk = 32Mhz(When TXCO=32Mhz).
    //__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL , (0x1000100<<ch)); 	//- ��spi(x) pclk.
//    __read_hw_reg32(CPR_SSI_CTRL, val);
//    val |= (ch==0)? 0x01: 0x30;
//    __write_hw_reg32(CPR_SSI_CTRL, val); 
		xinc_spim_disable(p_spim);
		uint32_t    val;
		uint8_t ch = p_instance->id;
		volatile uint32_t *Spim0CtlBaseAddr = (uint32_t*)&(p_instance->p_cpr->SSI0_MCLK_CTL);
		Spim0CtlBaseAddr+=ch;
		
		*Spim0CtlBaseAddr = 0x110010;//1��Ƶ			//- spi(x)_mclk = 32Mhz(When TXCO=32Mhz).
		p_instance->p_cpr->CTLAPBCLKEN_GRCTL = (0x1000100 << ch);
			
		val = p_instance->p_cpr->SSI_CTRL;
		val |= (ch==0)? 0x01: 0x30;
		p_instance->p_cpr->SSI_CTRL = val;
		
		// DMA 通道和外设使用是固定绑定关系的，因此此处不能做修改，对应中断处理里也就固定其检测值
		if(ch == 0)
		{
			p_cb->rx_dma_ch = 10;
			p_cb->tx_dma_ch = 2;
		}
		
		if(ch == 1)
		{
			p_cb->rx_dma_ch = 11;
			p_cb->tx_dma_ch = 3;
		}
		

		printf("p_spim:%p\n",p_spim); 
	//	__write_hw_reg32(SSIx_CTRL0(ch) , 0x07);					/* 8bit SPI data */
		p_spim->CTRL0 = 0x07;
		printf("CTRL0:%p,0x%x\n",&(p_spim->CTRL0),p_spim->CTRL0); 
		printf("xincx_spim_init ch :%d,mode:%d\n",ch,p_config->mode); 
		
		
    xinc_spim_configure(p_spim, p_config->mode, p_config->bit_order);
		
	
	//	p_spim->EN = 0x00;
		xinc_spim_disable(p_spim);
		printf("EN:%p\n",&(p_spim->EN)); 
		
		
  //  __write_hw_reg32(SSIx_SE(ch), 0x01);
	 p_spim->SE = 0x01;
	 printf("SE:%p\n",&(p_spim->SE)); 
	 
   // __write_hw_reg32(SSIx_BAUD(ch), p_config->frequency);						//- spix_mclk ��Ƶ.
	//p_spim->BAUD = p_config->frequency;
	xinc_spim_frequency_set(p_spim,p_config->frequency);
	
	 printf("BAUD:%p\n",&(p_spim->BAUD)); 
		
  //  __write_hw_reg32(SSIx_RXFTL(ch), 0x00);
  //  __write_hw_reg32(SSIx_TXFTL(ch), 0x00);
	p_spim->RXFLT = 0x00;
	p_spim->TXFLT = 0x00;
	printf("RXFLT:%p\n",&(p_spim->RXFLT)); 
	printf("TXFLT:%p\n",&(p_spim->TXFLT)); 
	printf("frequency:%d\n",p_config->frequency); 


    if (p_cb->handler)
    {
		//	p_spim->IE = 0x01;
			xinc_spim_int_enable(p_spim,0x01);
			NVIC_EnableIRQ(SPI0_IRQn + ch);
    }

    p_cb->transfer_in_progress = false;
    p_cb->state = NRFX_DRV_STATE_INITIALIZED;

    err_code = NRFX_SUCCESS;
    NRFX_LOG_INFO("Function: %s, error code: %s.", __func__, NRFX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

void xincx_spim_uninit(xincx_spim_t const * const p_instance)
{
    spim_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    NRFX_ASSERT(p_cb->state != NRFX_DRV_STATE_UNINITIALIZED);

    if (p_cb->handler)
    {
			
			NVIC_DisableIRQ(SPI0_IRQn + p_instance->drv_inst_idx);
    }

    XINC_SPIM_Type * p_spim = (XINC_SPIM_Type *)p_instance->p_reg;
    if (p_cb->handler)
    {
        if (p_cb->transfer_in_progress)
        {
            // Ensure that SPI is not performing any transfer.
          //  while (!xinc_spim_event_check(p_spim, XINC_SPIM_EVENT_STOPPED))
            {}
            p_cb->transfer_in_progress = false;
        }
    }

    if (p_cb->miso_pin != XINCX_SPIM_PIN_NOT_USED)
    {
      //  nrf_gpio_cfg_default(p_cb->miso_pin);
    }
  //  p_spim->IE = 0x00;
		xinc_spim_int_disable(p_spim,0x0);

#ifdef USE_WORKAROUND_FOR_ANOMALY_195
    if (p_spim == XINC_SPIM3)
    {
        *(volatile uint32_t *)0x4002F004 = 1;
    }
#endif

    p_cb->state = NRFX_DRV_STATE_UNINITIALIZED;
}


static void finish_transfer(spim_control_block_t * p_cb)
{
    // If Slave Select signal is used, this is the time to deactivate it.
    if (p_cb->ss_pin != XINCX_SPIM_PIN_NOT_USED)
    {
#if NRFX_CHECK(XINCX_SPIM_EXTENDED_ENABLED)
        if (!p_cb->use_hw_ss)
#endif
        {
            if (p_cb->ss_active_high)
            {
             //   nrf_gpio_pin_clear(p_cb->ss_pin);
            }
            else
            {
             //   nrf_gpio_pin_set(p_cb->ss_pin);
            }
        }
    }

    // By clearing this flag before calling the handler we allow subsequent
    // transfers to be started directly from the handler function.
    p_cb->transfer_in_progress = false;

    p_cb->evt.type = XINCX_SPIM_EVENT_DONE;
    p_cb->handler(&p_cb->evt, p_cb->p_context);
}

__STATIC_INLINE void spim_int_enable(XINC_SPIM_Type * p_spim, bool enable)
{
    if (!enable)
    {
      xinc_spim_int_disable(p_spim, 0x0);
		//	p_spim->IE = 0X00;
    }
    else
    {
        xinc_spim_int_enable(p_spim, 0x01);
		//	p_spim->IE = 0X01;

    }
}


static nrfx_err_t spim_xfer(XINC_SPIM_Type               * p_spim,
                            spim_control_block_t        * p_cb,
                            xincx_spim_xfer_desc_t  * p_xfer_desc,
                            uint32_t                      flags)
{
    nrfx_err_t err_code;
		uint8_t tmp_buf;
    // EasyDMA requires that transfer buffers are placed in Data RAM region;
    // signal error if they are not.
    if ((p_xfer_desc->p_tx_buffer != NULL && !nrfx_is_in_ram(p_xfer_desc->p_tx_buffer)) ||
        (p_xfer_desc->p_rx_buffer != NULL && !nrfx_is_in_ram(p_xfer_desc->p_rx_buffer)))
    {
        p_cb->transfer_in_progress = false;
        err_code = NRFX_ERROR_INVALID_ADDR;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         NRFX_LOG_ERROR_STRING_GET(err_code));
				printf("spim_xfer error:%d\r\n",err_code);
        return err_code;
    }
	
		
		 


 //   xinc_spim_tx_buffer_set(p_spim, p_xfer_desc->p_tx_buffer, p_xfer_desc->tx_length);
		
		
//		__write_hw_reg32(SSI1_EN , 0x00);
//    __write_hw_reg32(SSI1_DMAS , 0x03);
//    __write_hw_reg32(SSI1_DMATDL, 0x4);          //-
//    __write_hw_reg32(SSI1_DMARDL, 0x4);          //- 1/4 FIFO
		
	//	p_spim->EN = 0x00;
		xinc_spim_disable(p_spim);
		p_spim->DMAS = 0x03;
		p_spim->DMATDL = 0x04;
		p_spim->DMARDL = 0x04;
		
		
		
				//- TX Channel
		__write_hw_reg32(DMAS_CHx_SAR(p_cb->tx_dma_ch) , (uint32_t)p_xfer_desc->p_tx_buffer);
    __write_hw_reg32(DMAS_CHx_DAR(p_cb->tx_dma_ch) , (uint32_t)&(p_spim->DATA));
    __write_hw_reg32(DMAS_CHx_CTL1(p_cb->tx_dma_ch) ,((2 << 8)|  0));
    __write_hw_reg32(DMAS_CHx_CTL0(p_cb->tx_dma_ch) ,p_xfer_desc->tx_length);//

		
		
		
   // xinc_spim_rx_buffer_set(p_spim, p_xfer_desc->p_rx_buffer, p_xfer_desc->rx_length);
			//- RX Channel
		__write_hw_reg32(DMAS_CHx_SAR(p_cb->rx_dma_ch) , (uint32_t)&(p_spim->DATA));//
    __write_hw_reg32(DMAS_CHx_DAR(p_cb->rx_dma_ch) , (uint32_t)p_xfer_desc->p_rx_buffer);
    __write_hw_reg32(DMAS_CHx_CTL1(p_cb->rx_dma_ch) ,((2 << 8)|  0));
    __write_hw_reg32(DMAS_CHx_CTL0(p_cb->rx_dma_ch) ,p_xfer_desc->rx_length);
	

#if NRFX_CHECK(XINCX_SPIM3_NRF52840_ANOMALY_198_WORKAROUND_ENABLED)
    if (p_spim == XINC_SPIM3)
    {
        anomaly_198_enable(p_xfer_desc->p_tx_buffer, p_xfer_desc->tx_length);
    }
#endif



    if (!p_cb->handler)
    {
//        while (!xinc_spim_event_check(p_spim, XINC_SPIM_EVENT_END)){}

        if (p_cb->ss_pin != XINCX_SPIM_PIN_NOT_USED)
        {
#if NRFX_CHECK(XINCX_SPIM_EXTENDED_ENABLED)
            if (!p_cb->use_hw_ss)
#endif
            {
                if (p_cb->ss_active_high)
                {
                  //  nrf_gpio_pin_clear(p_cb->ss_pin);
                }
                else
                {
                  //  nrf_gpio_pin_set(p_cb->ss_pin);
                }
            }
        }
    }
    else
    {
      //  spim_int_enable(p_spim, !(flags & XINCX_SPIM_FLAG_NO_XFER_EVT_HANDLER));

			  __write_hw_reg32(DMAS_EN , p_cb->rx_dma_ch);
			  __write_hw_reg32(DMAS_EN , p_cb->tx_dma_ch);
			
			//	__write_hw_reg32(SSI1_EN , 0x01);
			//  p_spim->EN = 0x01;
			xinc_spim_enable(p_spim);
			
    }
    err_code = NRFX_SUCCESS;
    NRFX_LOG_INFO("Function: %s, error code: %s.", __func__, NRFX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

nrfx_err_t xincx_spim_xfer(xincx_spim_t     const * const p_instance,
                          xincx_spim_xfer_desc_t  * p_xfer_desc,
                          uint32_t                      flags)
{
    spim_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    NRFX_ASSERT(p_cb->state != NRFX_DRV_STATE_UNINITIALIZED);
    NRFX_ASSERT(p_xfer_desc->p_tx_buffer != NULL || p_xfer_desc->tx_length == 0);
    NRFX_ASSERT(p_xfer_desc->p_rx_buffer != NULL || p_xfer_desc->rx_length == 0);
    NRFX_ASSERT(SPIM_LENGTH_VALIDATE(p_instance->drv_inst_idx,
                                     p_xfer_desc->rx_length,
                                     p_xfer_desc->tx_length));

    nrfx_err_t err_code = NRFX_SUCCESS;

    if (p_cb->transfer_in_progress)
    {
        err_code = NRFX_ERROR_BUSY;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    else
    {
        if (p_cb->handler && !(flags & (XINCX_SPIM_FLAG_REPEATED_XFER |
                                        XINCX_SPIM_FLAG_NO_XFER_EVT_HANDLER)))
        {
            p_cb->transfer_in_progress = true;
        }
    }

    p_cb->evt.xfer_desc = *p_xfer_desc;

    if (p_cb->ss_pin != XINCX_SPIM_PIN_NOT_USED)
    {
#if NRFX_CHECK(XINCX_SPIM_EXTENDED_ENABLED)
        if (!p_cb->use_hw_ss)
#endif
        {
            if (p_cb->ss_active_high)
            {
              //  nrf_gpio_pin_set(p_cb->ss_pin);
            }
            else
            {
              //  nrf_gpio_pin_clear(p_cb->ss_pin);
            }
        }
    }

    return spim_xfer(p_instance->p_reg, p_cb,  p_xfer_desc, flags);
}

void xincx_spim_abort(xincx_spim_t const * p_instance)
{
    spim_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    NRFX_ASSERT(p_cb->state != NRFX_DRV_STATE_UNINITIALIZED);

    p_cb->transfer_in_progress = false;
}




static void irq_handler(XINC_SPIM_Type * p_spim, spim_control_block_t * p_cb)
{


  //  if (xinc_spim_event_check(p_spim, XINC_SPIM_EVENT_END))
    {

     //   xinc_spim_event_clear(p_spim, XINC_SPIM_EVENT_END);
        NRFX_ASSERT(p_cb->handler);
        NRFX_LOG_DEBUG("Event: XINC_SPIM_EVENT_END.");
			//	printf("finish_transfer %p\n",finish_transfer); 
        finish_transfer(p_cb);
			//	p_spim->EN = 0;
    }
}

#if NRFX_CHECK(XINCX_SPIM0_ENABLED)
void xincx_spim_0_irq_handler(void)
{
    irq_handler(XINC_SPIM0, &m_cb[XINCX_SPIM0_INST_IDX]);
}
#endif

#if NRFX_CHECK(XINCX_SPIM1_ENABLED)
void xincx_spim_1_irq_handler(void)
{
    irq_handler(XINC_SPIM1, &m_cb[XINCX_SPIM1_INST_IDX]);
}
#endif

void SPI0_Handler()
{
	uint32_t iWK;
	uint32_t iWK1;
	uint32_t STS;
	__read_hw_reg32(SSIx_IS(0) , iWK);
	__read_hw_reg32(SSIx_RIS(0) , iWK1);
	__read_hw_reg32(SSIx_STS(0) , STS);
	
	if(iWK && (STS & 0x4))
	{
			do	{
    	__read_hw_reg32(DMAS_INT_RAW , iWK);
    }while((iWK&0x404) != 0x404);
				
		__write_hw_reg32(DMAS_INT_RAW, 0x404);
    __write_hw_reg32(DMAS_CLR , 10);
    __write_hw_reg32(DMAS_CLR , 2);
		__write_hw_reg32(SSI0_EN, 0x00);
	  GPIO_OUTPUT_LOW(5);
	 GPIO_OUTPUT_HIGH(5);
	 GPIO_OUTPUT_LOW(5);
	//		irq_handler(XINC_SPIM0, &m_cb[XINCX_SPIM0_INST_IDX]);
		xincx_spim_0_irq_handler();
	}
	
}

void SPI1_Handler()
{
	uint32_t iWK;
	uint32_t iWK1;
	uint32_t STS;
	__read_hw_reg32(SSIx_IS(1) , iWK);
	__read_hw_reg32(SSIx_RIS(1) , iWK1);
	__read_hw_reg32(SSIx_STS(1) , STS);
//	printf("SPI1_Handler iWK :%x,iWK1:%x,STS:%x\n",iWK,iWK1,STS);  
	if(iWK && (STS & 0x4))
	{
			do	{
    	__read_hw_reg32(DMAS_INT_RAW , iWK);
    }while((iWK&0x808) != 0x808);		
		__write_hw_reg32(DMAS_INT_RAW, 0x808);
    __write_hw_reg32(DMAS_CLR , 11);
    __write_hw_reg32(DMAS_CLR , 3);
		__write_hw_reg32(SSI1_EN, 0x00);
	  GPIO_OUTPUT_LOW(5);
	  GPIO_OUTPUT_HIGH(5);
	  GPIO_OUTPUT_LOW(5);
		xincx_spim_1_irq_handler();
	}
	
}


#endif // NRFX_CHECK(XINCX_SPIM_ENABLED)
