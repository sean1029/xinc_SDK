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
#if NRFX_CHECK(XINCX_I2C_ENABLED)

#if !(NRFX_CHECK(XINCX_I2C0_ENABLED) || NRFX_CHECK(XINCX_I2C1_ENABLED))
#error "No enabled I2C instances. Check <nrfx_config.h>."
#endif

#include <xincx_i2c.h>
#include <hal/nrf_gpio.h>
#include "bsp_clk.h"
#define XINCX_LOG_MODULE I2C
#include <nrfx_log.h>

#define EVT_TO_STR(event)                                      \
    (event == XINCX_I2C_EVT_DONE         ? "EVT_DONE"         : \
    (event == XINCX_I2C_EVT_ADDRESS_NACK ? "EVT_ADDRESS_NACK" : \
    (event == XINCX_I2C_EVT_DATA_NACK    ? "EVT_DATA_NACK"    : \
    (event == XINCX_I2C_EVT_OVERRUN      ? "EVT_OVERRUN"      : \
    (event == XINCX_I2C_EVT_BUS_ERROR    ? "EVT_BUS_ERROR"    : \
                                          "UNKNOWN ERROR")))))

#define EVT_TO_STR_I2C(event)                                       \
    (event == XINC_I2C_EVENT_STOPPED   ? "XINC_I2C_EVENT_STOPPED"   : \
    (event == XINC_I2C_EVENT_RXDREADY  ? "XINC_I2C_EVENT_RXDREADY"  : \
    (event == XINC_I2C_EVENT_TXDSENT   ? "XINC_I2C_EVENT_TXDSENT"   : \
    (event == XINC_I2C_EVENT_ERROR     ? "XINC_I2C_EVENT_ERROR"     : \
    (event == XINC_I2C_EVENT_BB        ? "XINC_I2C_EVENT_BB"        : \
    (event == XINC_I2C_EVENT_SUSPENDED ? "XINC_I2C_EVENT_SUSPENDED" : \
                                        "UNKNOWN ERROR"))))))

#define TRANSFER_TO_STR(type)                   \
    (type == XINCX_I2C_XFER_TX   ? "XFER_TX"   : \
    (type == XINCX_I2C_XFER_RX   ? "XFER_RX"   : \
    (type == XINCX_I2C_XFER_TXRX ? "XFER_TXRX" : \
    (type == XINCX_I2C_XFER_TXTX ? "XFER_TXTX" : \
                                  "UNKNOWN TRANSFER TYPE"))))

#define I2C_PIN_INIT(_pin) 

#define I2C_FLAG_HAS_START(flags)           (flags & XINCX_I2C_FLAG_TX_HAS_START)
#define I2C_FLAG_NO_STOP(flags)             (flags & XINCX_I2C_FLAG_TX_NO_STOP)
#define I2C_FLAG_SUSPEND(flags)             (flags & XINCX_I2C_FLAG_SUSPEND)
#define I2C_FLAG_NO_HANDLER_IN_USE(flags)   (flags & XINCX_I2C_FLAG_NO_XFER_EVT_HANDLER)

#define HW_TIMEOUT      100000

/* I2C master driver suspend types. */
typedef enum
{
    I2C_NO_SUSPEND, //< Last transfer was not suspended.
    I2C_SUSPEND_TX, //< Last transfer was TX and was suspended.
    I2C_SUSPEND_RX  //< Last transfer was RX and was suspended.
} i2c_suspend_t;

// Control block - driver instance local data.
typedef struct
{
    xincx_i2c_evt_handler_t  handler;
    void *                  p_context;
    volatile uint32_t       int_mask;
    xincx_i2c_xfer_desc_t    xfer_desc;
    uint32_t                flags;
    uint8_t *               p_curr_buf;
    size_t                  curr_length;
    bool                    curr_tx_no_stop;
    i2c_suspend_t           prev_suspend;
    nrfx_drv_state_t        state;
    bool                    error;
    volatile bool           busy;
    bool                    repeated;
    size_t                  bytes_transferred;
		size_t                  bytes_transForRx;
    bool                    hold_bus_uninit;
} i2c_control_block_t;

static i2c_control_block_t m_cb[XINCX_I2C_ENABLED_COUNT];

static nrfx_err_t i2c_process_error(uint32_t errorsrc)
{
    nrfx_err_t ret = NRFX_ERROR_INTERNAL;

//  //  if (errorsrc & NRF_I2C_ERROR_OVERRUN)
//    {
//        ret = NRFX_ERROR_DRV_I2C_ERR_OVERRUN;
//    }

//   // if (errorsrc & NRF_I2C_ERROR_ADDRESS_NACK)
//    {
//        ret = NRFX_ERROR_DRV_I2C_ERR_ANACK;
//    }

//  //  if (errorsrc & NRF_I2C_ERROR_DATA_NACK)
//    {
//        ret = NRFX_ERROR_DRV_I2C_ERR_DNACK;
//    }

    return ret;
}

static bool xfer_completeness_check(XINC_I2C_Type * p_i2c, i2c_control_block_t const * p_cb)
{
    // If the actual number of transferred bytes is not equal to what was requested,
    // but there was no error signaled by the peripheral, this means that something
    // unexpected, like a premature STOP condition, was received on the bus.
    // In such case the peripheral has to be disabled and re-enabled, so that its
    // internal state machine is reinitialized.

    if (p_cb->bytes_transferred != p_cb->curr_length)
    {
        xinc_i2c_disable(p_i2c);
        xinc_i2c_enable(p_i2c);
        return false;
    }
    else
    {
        return true;
    }
}

nrfx_err_t xincx_i2c_init(xincx_i2c_t const *        p_instance,
                         xincx_i2c_config_t const * p_config,
                         xincx_i2c_evt_handler_t    event_handler,
                         void *                    p_context)
{
    NRFX_ASSERT(p_config);
    NRFX_ASSERT(p_config->scl != p_config->sda);
    i2c_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
    nrfx_err_t err_code;
		printf("%s\r\n",__func__);
    if (p_cb->state != NRFX_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRFX_ERROR_INVALID_STATE;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    p_cb->handler         = event_handler;
    p_cb->p_context       = p_context;
    p_cb->int_mask        = 0;
    p_cb->prev_suspend    = I2C_NO_SUSPEND;
    p_cb->repeated        = false;
    p_cb->busy            = false;
    p_cb->hold_bus_uninit = p_config->hold_bus_uninit;

    /* To secure correct signal levels on the pins used by the I2C
       master when the system is in OFF mode, and when the I2C master is
       disabled, these pins must be configured in the GPIO peripheral.
    */
		xc_i2c_clk_init();
    XINC_I2C_Type * p_i2c = p_instance->p_i2c;
		
		p_i2c->i2c_ENABLE = 0;
		p_i2c->i2c_RX_TL = 0;
		p_i2c->i2c_TX_TL = 0;
		p_i2c->i2c_INTR_EN = 0;
 //   xinc_i2c_pins_set(p_i2c, p_config->scl, p_config->sda);
    gpio_fun_sel(p_config->scl,I2C_SCL);
    gpio_fun_sel(p_config->sda,I2C_SDA);
		
   // xinc_i2c_frequency_set(p_i2c,(xinc_i2c_frequency_t)p_config->frequency);
    if(p_config->frequency == XINC_I2C_FREQ_100K)
    {
        p_i2c->i2c_CON = (p_config->frequency >> 16) & 0xFF;
        p_i2c->i2c_SS_SCL_HCNT = (p_config->frequency >> 8) & 0xFF;
        p_i2c->i2c_SS_SCL_LCNT = (p_config->frequency >> 0) & 0xFF;
        printf("speed config 100k\r\n");
    }
    else if(p_config->frequency == XINC_I2C_FREQ_400K)
    {
        p_i2c->i2c_CON = (p_config->frequency >> 16) & 0xFF;
        p_i2c->i2c_FS_SCL_HCNT = (p_config->frequency >> 8) & 0xFF;
        p_i2c->i2c_FS_SCL_LCNT = (p_config->frequency >> 0) & 0xFF;
        printf("speed config 400k\r\n");
    }
    else
    {
        err_code = NRFX_ERROR_INVALID_PARAM;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                        __func__,
                        NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    printf("speed config:0x%02x,0x%02x,0x%02x\r\n",(p_config->frequency >> 16) & 0xFF,(p_config->frequency >> 8) & 0xFF,(p_config->frequency >> 0) & 0xFF);
		
		

    if (p_cb->handler)
    {
      //  p_i2c->i2c_INTR_EN = 0x01 << 2;
       // NRFX_IRQ_ENABLE(I2C_IRQn);
			
		//	p_config->   XINCX_I2C_FLAG_NO_XFER_EVT_HANDLER
    }

    p_cb->state = NRFX_DRV_STATE_INITIALIZED;

    err_code = NRFX_SUCCESS;
    NRFX_LOG_INFO("Function: %s, error code: %s.", __func__, NRFX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

void xincx_i2c_uninit(xincx_i2c_t const * p_instance)
{
    i2c_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    NRFX_ASSERT(p_cb->state != NRFX_DRV_STATE_UNINITIALIZED);

    if (p_cb->handler)
    {
        NRFX_IRQ_DISABLE(I2C_IRQn);
    }
    xincx_i2c_disable(p_instance);

#if NRFX_CHECK(NRFX_PRS_ENABLED)
    nrfx_prs_release(p_instance->p_i2c);
#endif

    if (!p_cb->hold_bus_uninit)
    {
      //  nrf_gpio_cfg_default(xinc_i2c_scl_pin_get(p_instance->p_i2c));
       // nrf_gpio_cfg_default(xinc_i2c_sda_pin_get(p_instance->p_i2c));
    }

    p_cb->state = NRFX_DRV_STATE_UNINITIALIZED;
    NRFX_LOG_INFO("Instance uninitialized: %d.", p_instance->drv_inst_idx);
}

void xincx_i2c_enable(xincx_i2c_t const * p_instance)
{
    i2c_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    NRFX_ASSERT(p_cb->state == NRFX_DRV_STATE_INITIALIZED);

    XINC_I2C_Type * p_i2c = p_instance->p_i2c;
	
   // xinc_i2c_enable(p_i2c);
		p_i2c->i2c_ENABLE = 1;
	
    p_cb->state = NRFX_DRV_STATE_POWERED_ON;
    NRFX_LOG_INFO("Instance enabled: %d.", p_instance->drv_inst_idx);
}

void xincx_i2c_disable(xincx_i2c_t const * p_instance)
{
    i2c_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    NRFX_ASSERT(p_cb->state != NRFX_DRV_STATE_UNINITIALIZED);

    XINC_I2C_Type * p_i2c = p_instance->p_i2c;
//    xinc_i2c_int_disable(p_i2c, NRF_I2C_ALL_INTS_MASK);
  //  xinc_i2c_disable(p_i2c);
		p_i2c->i2c_ENABLE = 0;
    p_cb->state = NRFX_DRV_STATE_INITIALIZED;
    NRFX_LOG_INFO("Instance disabled: %d.", p_instance->drv_inst_idx);
}

static bool i2c_send_byte(XINC_I2C_Type          * p_i2c,
                          i2c_control_block_t   * p_cb)
{
		
	//	printf("%s,bytes_transferred:%d,curr_length:%d\r\n",__func__,p_cb->bytes_transferred,p_cb->curr_length);
//		printf("i2c_STATUS:0x%x\r\n",p_i2c->i2c_STATUS);
		
		
		if (p_cb->bytes_transferred  == p_cb->curr_length)
		{
				return false;
		}
		if((p_i2c->i2c_STATUS & 0x02) != 0x02)
		{
				return true;	
		}
		
		uint16_t data_cmd =  I2C_FLAG_HAS_START(p_cb->flags )? (0x01 << 10) : 0x0;
    if ((p_cb->bytes_transferred  +  1) < p_cb->curr_length)
    {
        xinc_i2c_txd_set(p_i2c, p_cb->p_curr_buf[p_cb->bytes_transferred] | data_cmd);
    }
    else
    {
        if (p_cb->curr_tx_no_stop)
        {
           // xinc_i2c_task_trigger(p_i2c, NRF_I2C_TASK_SUSPEND);
						xinc_i2c_txd_set(p_i2c, p_cb->p_curr_buf[p_cb->bytes_transferred] | data_cmd);
            return false;
        }
        else if(I2C_FLAG_SUSPEND(p_cb->flags))
        {
          //  xinc_i2c_task_trigger(p_i2c, NRF_I2C_TASK_SUSPEND);
					  xinc_i2c_txd_set(p_i2c, p_cb->p_curr_buf[p_cb->bytes_transferred] | data_cmd);
            p_cb->prev_suspend = I2C_SUSPEND_TX;
            return false;
        }
        else
        {
         //   xinc_i2c_task_trigger(p_i2c, NRF_I2C_TASK_STOP);
					 xinc_i2c_txd_set(p_i2c, p_cb->p_curr_buf[p_cb->bytes_transferred] | (0x1 << 9) | data_cmd);
					 return false;
        }
    }
    return true;
}

static bool i2c_receive_byte(XINC_I2C_Type         * p_i2c,
                             i2c_control_block_t  * p_cb)
{
	

		uint16_t data_cmd =  (0x01 << 8);
	
		if(p_cb->bytes_transForRx)
		{
			if((p_i2c->i2c_STATUS & 0x08) == 0x08)
			{
						p_cb->p_curr_buf[p_cb->bytes_transferred] = xinc_i2c_rxd_get(p_i2c) & 0xFF;
						++(p_cb->bytes_transferred);	
			}
		}
			
		if (p_cb->bytes_transForRx  + 1 < p_cb->curr_length)
    {					
					if((p_i2c->i2c_STATUS & 0x02) != 0x02)
					{
					//	printf("txd_set :0x%x fail\r\n ",data_cmd);
						return true;
					}

					xinc_i2c_txd_set(p_i2c, data_cmd);
					p_cb->bytes_transForRx++;
								     
    }
		else
		{
				if ((p_cb->bytes_transferred == p_cb->curr_length - 2) && (!I2C_FLAG_SUSPEND(p_cb->flags)))
        {
					
//            xinc_i2c_shorts_set(p_i2c, NRF_I2C_SHORT_BB_STOP_MASK);
						
						if((p_i2c->i2c_STATUS & 0x02) != 0x02)
						{
							return true;
						}
						data_cmd |= (0x01 << 9);
						xinc_i2c_txd_set(p_i2c, data_cmd);
						while((p_i2c->i2c_STATUS & 0x08) != 0x08)
						{
							__nop();
						}
						p_cb->p_curr_buf[p_cb->bytes_transferred] = xinc_i2c_rxd_get(p_i2c) & 0xFF;
						++(p_cb->bytes_transferred);
						while((p_i2c->i2c_STATUS & 0x08) != 0x08)
						{
							__nop();
						}
						p_cb->p_curr_buf[p_cb->bytes_transferred] = xinc_i2c_rxd_get(p_i2c) & 0xFF;
						++(p_cb->bytes_transferred);
						p_cb->bytes_transForRx = 0;
						return false;
        }
				else if ((p_cb->bytes_transferred == p_cb->curr_length - 1) && (!I2C_FLAG_SUSPEND(p_cb->flags)))
        {
					
         //   xinc_i2c_shorts_set(p_i2c, NRF_I2C_SHORT_BB_STOP_MASK);
						
						if((p_i2c->i2c_STATUS & 0x02) != 0x02)
						{
							return true;
						}
						data_cmd |= (0x01 << 9);
						xinc_i2c_txd_set(p_i2c, data_cmd);
						while((p_i2c->i2c_STATUS & 0x08) != 0x08)
						{
							__nop();
						}
						p_cb->p_curr_buf[p_cb->bytes_transferred] = xinc_i2c_rxd_get(p_i2c) & 0xFF;
						++(p_cb->bytes_transferred);
						p_cb->bytes_transForRx = 0;
						return false;
        }
		
		}
    return true;
}

static bool i2c_transfer(XINC_I2C_Type           * p_i2c,
                         i2c_control_block_t    * p_cb)
{
    bool stopped = false;
	//	printf("%s\r\n",__func__);
    if (xinc_i2c_event_check(p_i2c, XINC_I2C_EVENT_STOPPED))
    {
        xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_STOPPED);
        NRFX_LOG_DEBUG("I2C: Event: %s.", EVT_TO_STR_I2C(XINC_I2C_EVENT_STOPPED));

        // Delay handling of STOPPED event till the end of events processing procedure.
        // If penultimate byte is received and function gets interrupted for long enough
        // after enabling BB_STOP shortcut, RXDREADY for last byte as well as STOPPED
        // may be active at the same time. Therefore RXREADY has to be processed before STOPPED to
        // acquire last byte before finishing transmission by returning 'false'.
        stopped = true;
    }

    if (p_cb->error)
    {
        xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_ERROR);
        xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_TXDSENT);
        xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_RXDREADY);
    }
    else if (xinc_i2c_event_check(p_i2c, XINC_I2C_EVENT_ERROR))
    {
        xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_ERROR);
        NRFX_LOG_DEBUG("I2C: Event: %s.", EVT_TO_STR_I2C(XINC_I2C_EVENT_ERROR));
        xinc_i2c_task_trigger(p_i2c, XINC_I2C_TASK_STOP);
        p_cb->error = true;
    }
    else
    {
       // if (xinc_i2c_event_check(p_i2c, XINC_I2C_EVENT_TXDSENT))
				if (p_cb->xfer_desc.type == XINCX_I2C_XFER_TX)
        {
            xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_TXDSENT);
            ++(p_cb->bytes_transferred);
            NRFX_LOG_DEBUG("I2C: Event: %s.", EVT_TO_STR_I2C(XINC_I2C_EVENT_TXDSENT));
            if (xinc_i2c_event_check(p_i2c, XINC_I2C_EVENT_ERROR))
            {
                xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_ERROR);
                NRFX_LOG_DEBUG("I2C: Event: %s.", EVT_TO_STR_I2C(XINC_I2C_EVENT_ERROR));
                xinc_i2c_task_trigger(p_i2c, XINC_I2C_TASK_STOP);
                p_cb->error = true;
            }
            else
            {
                if (!i2c_send_byte(p_i2c, p_cb))
                {
										stopped = true;
                    return false;
                }
            }
        }
       // else if (xinc_i2c_event_check(p_i2c, XINC_I2C_EVENT_RXDREADY))
				else if (p_cb->xfer_desc.type == XINCX_I2C_XFER_RX)
        {
            xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_RXDREADY);
            NRFX_LOG_DEBUG("I2C: Event: %s.", EVT_TO_STR_I2C(XINC_I2C_EVENT_RXDREADY));
            if (xinc_i2c_event_check(p_i2c, XINC_I2C_EVENT_ERROR))
            {
                NRFX_LOG_DEBUG("I2C: Event: %s.", EVT_TO_STR_I2C(XINC_I2C_EVENT_ERROR));
                xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_ERROR);
                xinc_i2c_task_trigger(p_i2c, XINC_I2C_TASK_STOP);
                p_cb->error = true;
            }
            else
            {
                if (!i2c_receive_byte(p_i2c, p_cb))
                {
									  stopped = true;
                    return false;
                }else{
							//		printf("contiu receice byte\r\n");
								}
            }
        }
    }

    if (stopped)
    {
        p_cb->prev_suspend = I2C_NO_SUSPEND;
        if (!p_cb->error)
        {
            p_cb->error = !xfer_completeness_check(p_i2c, p_cb);
        }
        return false;
    }

    return true;
}

static nrfx_err_t i2c_tx_start_transfer(XINC_I2C_Type        * p_i2c,
                                        i2c_control_block_t * p_cb)
{
    nrfx_err_t ret_code = NRFX_SUCCESS;
    volatile int32_t hw_timeout;
		bool contiu_send_flag = true;

    hw_timeout = HW_TIMEOUT;
	//	printf("%s\r\n",__func__);
    xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_STOPPED);
    xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_ERROR);
    xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_TXDSENT);
    xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_RXDREADY);
    xinc_i2c_shorts_set(p_i2c, 0);

    p_cb->bytes_transferred = 0;
		p_cb->bytes_transForRx = 0;
    p_cb->error             = false;
		p_cb->flags &= ~(XINCX_I2C_FLAG_TX_HAS_START);
	

    // In case I2C is suspended resume its operation.
    xinc_i2c_task_trigger(p_i2c, XINC_I2C_TASK_RESUME);

    if (p_cb->prev_suspend != I2C_SUSPEND_TX)
    {
      //  xinc_i2c_task_trigger(p_i2c, NRF_I2C_TASK_STARTTX);
				p_cb->flags |= (XINCX_I2C_FLAG_TX_HAS_START);
    }
		p_i2c->i2c_ENABLE = 0x0;	
		p_i2c->i2c_TAR = p_cb->xfer_desc.address;
		p_i2c->i2c_ENABLE = 0x1;	
    contiu_send_flag = i2c_send_byte(p_i2c, p_cb);
		
		p_cb->flags &= ~(XINCX_I2C_FLAG_TX_HAS_START);

    if (p_cb->handler)
    {
//        p_cb->int_mask = NRF_I2C_INT_STOPPED_MASK   |
//                         NRF_I2C_INT_ERROR_MASK     |
//                         NRF_I2C_INT_TXDSENT_MASK   |
//                         NRF_I2C_INT_RXDREADY_MASK;

//        xinc_i2c_int_enable(p_i2c, p_cb->int_mask);
			p_i2c->i2c_INTR_EN = (0x01 << 4);
			NRFX_IRQ_ENABLE(I2C_IRQn);
			p_i2c->i2c_ENABLE = 1;
			
    }
    else
    {
				if(contiu_send_flag)
				{
					while ((hw_timeout > 0) &&
               i2c_transfer(p_i2c, p_cb))
					{
							hw_timeout--;
					}
				}
        
				
        if (p_cb->error)
        {
            uint32_t errorsrc =  xinc_i2c_errorsrc_get_and_clear(p_i2c);

            if (errorsrc)
            {
                ret_code = i2c_process_error(errorsrc);
            }
            else
            {
                ret_code = NRFX_ERROR_INTERNAL;
            }
        }

        if (hw_timeout <= 0)
        {
            xinc_i2c_disable(p_i2c);
            xinc_i2c_enable(p_i2c);
            ret_code = NRFX_ERROR_INTERNAL;
        }

    }
    return ret_code;
}

static nrfx_err_t i2c_rx_start_transfer(XINC_I2C_Type        * p_i2c,
                                        i2c_control_block_t * p_cb)
{
    nrfx_err_t ret_code = NRFX_SUCCESS;
    volatile int32_t hw_timeout;

    hw_timeout = HW_TIMEOUT;

    xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_STOPPED);
    xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_ERROR);
    xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_TXDSENT);
    xinc_i2c_event_clear(p_i2c, XINC_I2C_EVENT_RXDREADY);

    p_cb->bytes_transferred = 0;
		p_cb->bytes_transForRx = 0;
    p_cb->error             = false;
		p_i2c->i2c_ENABLE = 0;

    if ((p_cb->curr_length == 1) && (!I2C_FLAG_SUSPEND(p_cb->flags)))
    {
       // xinc_i2c_shorts_set(p_i2c, NRF_I2C_SHORT_BB_STOP_MASK);
    }
    else
    {
      //  xinc_i2c_shorts_set(p_i2c, NRF_I2C_SHORT_BB_SUSPEND_MASK);
    }
    // In case I2C is suspended resume its operation.
  //  xinc_i2c_task_trigger(p_i2c, XINC_I2C_TASK_RESUME);

    if (p_cb->prev_suspend != I2C_SUSPEND_RX)
    {
      //  xinc_i2c_task_trigger(p_i2c, XINC_I2C_TASK_STARTRX);
    }
	
    if (p_cb->handler)
    {
//        p_cb->int_mask = NRF_I2C_INT_STOPPED_MASK   |
//                        NRF_I2C_INT_ERROR_MASK     |
//                        NRF_I2C_INT_TXDSENT_MASK   |
//                        NRF_I2C_INT_RXDREADY_MASK;
    //    xinc_i2c_int_enable(p_i2c, p_cb->int_mask);
			p_i2c->i2c_INTR_EN = (0x01 << 2) | (0x01 << 4);
			NRFX_IRQ_ENABLE(I2C_IRQn);
			p_i2c->i2c_ENABLE = 1;
    }
    else
    {
        while ((hw_timeout > 0) &&
               i2c_transfer(p_i2c, p_cb))
        {
               hw_timeout--;
        }
        if (p_cb->error)
        {
            uint32_t errorsrc =  xinc_i2c_errorsrc_get_and_clear(p_i2c);

            if (errorsrc)
            {
                ret_code = i2c_process_error(errorsrc);
            }
            else
            {
                ret_code = NRFX_ERROR_INTERNAL;
            }
        }
        if (hw_timeout <= 0)
        {
           // xinc_i2c_disable(p_i2c);
						p_i2c->i2c_ENABLE = 0;
          //  xinc_i2c_enable(p_i2c);
						p_i2c->i2c_ENABLE = 1;
            ret_code = NRFX_ERROR_INTERNAL;
        }
    }
    return ret_code;
}

__STATIC_INLINE nrfx_err_t i2c_xfer(XINC_I2C_Type               * p_i2c,
                                    i2c_control_block_t        * p_cb,
                                    xincx_i2c_xfer_desc_t const * p_xfer_desc,
                                    uint32_t                     flags)
{

    nrfx_err_t err_code = NRFX_SUCCESS;

    if ((p_cb->prev_suspend == I2C_SUSPEND_TX) && (p_xfer_desc->type == XINCX_I2C_XFER_RX))
    {
        /* RX is invalid after TX suspend */
        return NRFX_ERROR_INVALID_STATE;
    }
    else if ((p_cb->prev_suspend == I2C_SUSPEND_RX) && (p_xfer_desc->type != XINCX_I2C_XFER_RX))
    {
        /* TX, TXRX and TXTX are invalid after RX suspend */
        return NRFX_ERROR_INVALID_STATE;
    }

    /* Block I2C interrupts to ensure that function is not interrupted by I2C interrupt. */
  //  xinc_i2c_int_disable(p_i2c, NRF_I2C_ALL_INTS_MASK);
		uint32_t reg;
		p_i2c->i2c_INTR_EN = 0;
		reg = p_i2c->i2c_CLR_INTR;

    if (p_cb->busy)
    {
        xinc_i2c_int_enable(p_i2c, p_cb->int_mask);
        err_code = NRFX_ERROR_BUSY;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    else
    {
			  p_cb->busy = (I2C_FLAG_NO_HANDLER_IN_USE(flags)) ? false : true;
    }

    p_cb->flags       = flags;
    p_cb->xfer_desc   = *p_xfer_desc;
    p_cb->curr_length = p_xfer_desc->primary_length;
    p_cb->p_curr_buf  = p_xfer_desc->p_primary_buf;

    if (p_xfer_desc->type != XINCX_I2C_XFER_RX)
    {
        p_cb->curr_tx_no_stop = ((p_xfer_desc->type == XINCX_I2C_XFER_TX) &&
                                 !(flags & XINCX_I2C_FLAG_TX_NO_STOP)) ? false : true;

        err_code = i2c_tx_start_transfer(p_i2c, p_cb);
    }
    else
    {
        err_code = i2c_rx_start_transfer(p_i2c, p_cb);
    }
    if (p_cb->handler == NULL)
    {
        p_cb->busy = false;
    }
    return err_code;
}

bool xincx_i2c_is_busy(xincx_i2c_t const * p_instance)
{
    i2c_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    return p_cb->busy;
}

nrfx_err_t xincx_i2c_xfer(xincx_i2c_t           const * p_instance,
                         xincx_i2c_xfer_desc_t const * p_xfer_desc,
                         uint32_t                     flags)
{

    nrfx_err_t err_code = NRFX_SUCCESS;
    i2c_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    // TXRX and TXTX transfers are supported only in non-blocking mode.
    NRFX_ASSERT( !((p_cb->handler == NULL) && (p_xfer_desc->type == XINCX_I2C_XFER_TXRX)));
    NRFX_ASSERT( !((p_cb->handler == NULL) && (p_xfer_desc->type == XINCX_I2C_XFER_TXTX)));

    NRFX_LOG_INFO("Transfer type: %s.", TRANSFER_TO_STR(p_xfer_desc->type));
    NRFX_LOG_INFO("Transfer buffers length: primary: %d, secondary: %d.",
                  p_xfer_desc->primary_length,
                  p_xfer_desc->secondary_length);
    NRFX_LOG_DEBUG("Primary buffer data:");
    NRFX_LOG_HEXDUMP_DEBUG(p_xfer_desc->p_primary_buf,
                           p_xfer_desc->primary_length * sizeof(p_xfer_desc->p_primary_buf[0]));
    NRFX_LOG_DEBUG("Secondary buffer data:");
    NRFX_LOG_HEXDUMP_DEBUG(p_xfer_desc->p_secondary_buf,
                           p_xfer_desc->secondary_length * sizeof(p_xfer_desc->p_secondary_buf[0]));

    err_code = i2c_xfer((XINC_I2C_Type  *)p_instance->p_i2c, p_cb, p_xfer_desc, flags);
    NRFX_LOG_WARNING("Function: %s, error code: %s.",
                     __func__,
                     NRFX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

nrfx_err_t xincx_i2c_tx(xincx_i2c_t const * p_instance,
                       uint8_t            address,
                       uint8_t    const * p_data,
                       size_t             length,
                       bool               no_stop)
{
    xincx_i2c_xfer_desc_t xfer = XINCX_I2C_XFER_DESC_TX(address, (uint8_t*)p_data, length);
    return xincx_i2c_xfer(p_instance, &xfer, no_stop ? XINCX_I2C_FLAG_TX_NO_STOP : 0);
}

nrfx_err_t xincx_i2c_rx(xincx_i2c_t const * p_instance,
                       uint8_t            address,
                       uint8_t *          p_data,
                       size_t             length)
{
    xincx_i2c_xfer_desc_t xfer = XINCX_I2C_XFER_DESC_RX(address, p_data, length);
    return xincx_i2c_xfer(p_instance, &xfer, 0);
}

size_t xincx_i2c_data_count_get(xincx_i2c_t const * const p_instance)
{
    return m_cb[p_instance->drv_inst_idx].bytes_transferred;
}


static void i2c_irq_handler(XINC_I2C_Type * p_i2c, i2c_control_block_t * p_cb)
{
    NRFX_ASSERT(p_cb->handler);

		uint8_t val;
    if (i2c_transfer(p_i2c, p_cb))
    {
        return;
    }
		if(p_i2c->i2c_TXFLR)
		{
			return;
		}
		p_i2c->i2c_ENABLE = 0;
		val = p_i2c->i2c_CLR_INTR;
		p_i2c->i2c_INTR_EN = 0;
		NRFX_IRQ_DISABLE(I2C_IRQn);
    if (!p_cb->error &&
        ((p_cb->xfer_desc.type == XINCX_I2C_XFER_TXRX) ||
         (p_cb->xfer_desc.type == XINCX_I2C_XFER_TXTX)) &&
        p_cb->p_curr_buf == p_cb->xfer_desc.p_primary_buf)
    {
        p_cb->p_curr_buf      = p_cb->xfer_desc.p_secondary_buf;
        p_cb->curr_length     = p_cb->xfer_desc.secondary_length;
        p_cb->curr_tx_no_stop = (p_cb->flags & XINCX_I2C_FLAG_TX_NO_STOP);
        p_cb->prev_suspend    = I2C_NO_SUSPEND;

        if (p_cb->xfer_desc.type == XINCX_I2C_XFER_TXTX)
        {
            (void)i2c_tx_start_transfer(p_i2c, p_cb);
        }
        else
        {
            (void)i2c_rx_start_transfer(p_i2c, p_cb);
        }
    }
    else
    {
        xincx_i2c_evt_t event;
        event.xfer_desc = p_cb->xfer_desc;

        if (p_cb->error)
        {
          //  uint32_t errorsrc = xinc_i2c_errorsrc_get_and_clear(p_i2c);
						uint32_t errorsrc = (p_i2c->i2c_INTR_STAT);
            
        }
        else
        {
            event.type = XINCX_I2C_EVT_DONE;
            NRFX_LOG_DEBUG("Event: %s.", EVT_TO_STR(XINCX_I2C_EVT_DONE));
        }

        p_cb->busy = false;

        if (!(I2C_FLAG_NO_HANDLER_IN_USE(p_cb->flags)) || p_cb->error)
        {
            p_cb->handler(&event, p_cb->p_context);
					
        }
    }

}


#if NRFX_CHECK(XINCX_I2C0_ENABLED)
void 	I2C_Handler()
{
//	printf("I2C_Handler\r\n");
	xincx_i2c_0_irq_handler();

}
void xincx_i2c_0_irq_handler(void)
{
    i2c_irq_handler(XINC_I2C0, &m_cb[XINCX_I2C0_INST_IDX]);
}
#endif

#if NRFX_CHECK(XINCX_I2C1_ENABLED)
void xincx_i2c_1_irq_handler(void)
{
    i2c_irq_handler(NRF_I2C1, &m_cb[XINCX_I2C1_INST_IDX]);
}
#endif

#endif // NRFX_CHECK(XINCX_I2C_ENABLED)
