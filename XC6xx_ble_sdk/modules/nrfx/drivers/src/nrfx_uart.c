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

#if NRFX_CHECK(NRFX_UART_ENABLED)

#if !NRFX_CHECK(NRFX_UART0_ENABLED) && !NRFX_CHECK(NRFX_UART1_ENABLED)
#error "No enabled UART instances. Check <nrfx_config.h>."
#endif

#include <nrfx_uart.h>
#include <hal/nrf_gpio.h>
#include "bsp_register_macro.h"
#include "bsp_clk.h"
#include "bsp_uart.h"
#define NRFX_LOG_MODULE UART
#include <nrfx_log.h>

#define EVT_TO_STR(event) \
    (event == NRF_UART_EVENT_ERROR ? "NRF_UART_EVENT_ERROR" : \
                                     "UNKNOWN EVENT")


#define TX_COUNTER_ABORT_REQ_VALUE  UINT32_MAX

typedef struct
{
    void                    * p_context;
    nrfx_uart_event_handler_t handler;
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
    nrfx_drv_state_t          state;
} uart_control_block_t;
static uart_control_block_t m_cb[NRFX_UART_ENABLED_COUNT];

static void apply_config(nrfx_uart_t        const * p_instance,
                         nrfx_uart_config_t const * p_config)
{
		printf("%s\r\n",__func__);
	
		printf("pseltxd:%d\r\n",p_config->pseltxd);
    if (p_config->pseltxd != NRF_UART_PSEL_DISCONNECTED)
    {     
      
    }
		printf("pselrxd:%d\r\n",p_config->pselrxd);
    if (p_config->pselrxd != NRF_UART_PSEL_DISCONNECTED)
    {
      
    }
		printf("baudrate_set:%d\r\n",p_config->baudrate);
		
    nrf_uart_baudrate_set(p_instance->p_reg, p_config->baudrate);
		
		//  nrf_uart_configure
		p_instance->p_reg->TCR &= ~(0x01 << 3);
		p_instance->p_reg->MCR &= ~(0x01 << 5);
		p_instance->p_reg->TCR |= (0x03 << 0);
		p_instance->p_reg->IIR_FCR.FCR = 0XB7;
		
    nrf_uart_txrx_pins_set(p_instance->p_reg, p_config->pseltxd, p_config->pselrxd);
		
		
		gpio_fun_sel(p_config->pseltxd,UART1_TX);
		gpio_fun_sel(p_config->pselrxd,UART1_RX);
		
		
		
		
		
    if (p_config->hwfc == NRF_UART_HWFC_ENABLED)
    {
        if (p_config->pselcts != NRF_UART_PSEL_DISCONNECTED)
        {
          
        }
        if (p_config->pselrts != NRF_UART_PSEL_DISCONNECTED)
        {
         
        }
     
    }
}

static void interrupts_enable(nrfx_uart_t const * p_instance,
                              uint8_t             interrupt_priority)
{
	  printf("%s\r\n",__func__);
 
		p_instance->p_reg->IER_DLH.IER = 0X03;//open tx/rx interrupt
	
		printf("p_instance->p_reg:%p\r\n",p_instance->p_reg);
	
	//	printf("p_instance->p_reg.IER:%p\r\n",&p_instance->p_reg->IER_DLH.IER);
              
	
		NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + p_instance->id));
}

static void interrupts_disable(nrfx_uart_t const * p_instance)
{
   p_instance->p_reg->IER_DLH.IER = 0X00;
    NRFX_IRQ_DISABLE((IRQn_Type)(UART0_IRQn + p_instance->id));
}


nrfx_err_t nrfx_uart_init(nrfx_uart_t const *        p_instance,
                          nrfx_uart_config_t const * p_config,
                          nrfx_uart_event_handler_t  event_handler)
{
    NRFX_ASSERT(p_config);
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    nrfx_err_t err_code = NRFX_SUCCESS;
	printf("%s,inst_idx:%d,state:%d\r\n",__func__,p_instance->drv_inst_idx,p_cb->state);
	// printf("NRF_UART_Type size :%d\r\n",sizeof(NRF_UART_Type));
    if (p_cb->state != NRFX_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRFX_ERROR_INVALID_STATE;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
	
		xc_uart_clk_init(p_instance->id,p_config->baudrate);
		

    apply_config(p_instance, p_config);

    p_cb->handler   = event_handler;
    p_cb->p_context = p_config->p_context;

    if (p_cb->handler)
    {
        interrupts_enable(p_instance, p_config->interrupt_priority);
    }

    nrf_uart_enable(p_instance->p_reg);
    p_cb->rx_buffer_length           = 0;
    p_cb->rx_secondary_buffer_length = 0;
    p_cb->rx_enabled                 = false;
    p_cb->tx_buffer_length           = 0;
    p_cb->state                      = NRFX_DRV_STATE_INITIALIZED;
    NRFX_LOG_WARNING("Function: %s, error code: %s.",
                     __func__,
                     NRFX_LOG_ERROR_STRING_GET(err_code));
		
		printf("Function: %s, error code: %s.\r\n",
                     __func__,
                     NRFX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

void nrfx_uart_uninit(nrfx_uart_t const * p_instance)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    nrf_uart_disable(p_instance->p_reg);

    if (p_cb->handler)
    {
        interrupts_disable(p_instance);
    }


    p_cb->state   = NRFX_DRV_STATE_UNINITIALIZED;
    p_cb->handler = NULL;
    NRFX_LOG_INFO("Instance uninitialized: %d.", p_instance->drv_inst_idx);
}

static void tx_byte(NRF_UART_Type * p_uart, uart_control_block_t * p_cb)
{
//    nrf_uart_event_clear(p_uart, NRF_UART_EVENT_TXDRDY);
    uint8_t txd = p_cb->p_tx_buffer[p_cb->tx_counter];
    p_cb->tx_counter++;
    nrf_uart_txd_set(p_uart, txd);
}

static bool tx_blocking(NRF_UART_Type * p_uart, uart_control_block_t * p_cb)
{
    // Use a local variable to avoid undefined order of accessing two volatile variables
    // in one statement.
    size_t const tx_buffer_length = p_cb->tx_buffer_length;
    while (p_cb->tx_counter < tx_buffer_length)
    {
        // Wait until the transmitter is ready to accept a new byte.
        // Exit immediately if the transfer has been aborted.
//        while (!nrf_uart_event_check(p_uart, NRF_UART_EVENT_TXDRDY))
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

nrfx_err_t nrfx_uart_tx(nrfx_uart_t const * p_instance,
                        uint8_t const *     p_data,
                        size_t              length)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    NRFX_ASSERT(p_cb->state == NRFX_DRV_STATE_INITIALIZED);
    NRFX_ASSERT(p_data);
    NRFX_ASSERT(length > 0);

    nrfx_err_t err_code;
//	  printf("nrfx_uart_tx progress:%d\r\n",nrfx_uart_tx_in_progress(p_instance));
    if (nrfx_uart_tx_in_progress(p_instance))
    {
        err_code = NRFX_ERROR_BUSY;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    p_cb->tx_buffer_length = length;
    p_cb->p_tx_buffer      = p_data;
    p_cb->tx_counter       = 0;
    p_cb->tx_abort         = false;

    NRFX_LOG_INFO("Transfer tx_len: %d.", p_cb->tx_buffer_length);
    NRFX_LOG_DEBUG("Tx data:");
    NRFX_LOG_HEXDUMP_DEBUG(p_cb->p_tx_buffer,
                           p_cb->tx_buffer_length * sizeof(p_cb->p_tx_buffer[0]));

    err_code = NRFX_SUCCESS;

//    nrf_uart_event_clear(p_instance->p_reg, NRF_UART_EVENT_TXDRDY);
   
    tx_byte(p_instance->p_reg, p_cb);

    if (p_cb->handler == NULL)
    {
        if (!tx_blocking(p_instance->p_reg, p_cb))
        {
            // The transfer has been aborted.
            err_code = NRFX_ERROR_FORBIDDEN;
        }
        else
        {
            // Wait until the last byte is completely transmitted.
//            while (!nrf_uart_event_check(p_instance->p_reg, NRF_UART_EVENT_TXDRDY))
            {}
           
        }
        p_cb->tx_buffer_length = 0;
    }

    NRFX_LOG_INFO("Function: %s, error code: %s.", __func__, NRFX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

bool nrfx_uart_tx_in_progress(nrfx_uart_t const * p_instance)
{
    return (m_cb[p_instance->drv_inst_idx].tx_buffer_length != 0);
}


static void rx_byte(NRF_UART_Type * p_uart, uart_control_block_t * p_cb)
{
    if (!p_cb->rx_buffer_length)
    {
        // Byte received when buffer is not set - data lost.
        (void) nrf_uart_rxd_get(p_uart);
        return;
    }
    p_cb->p_rx_buffer[p_cb->rx_counter] = nrf_uart_rxd_get(p_uart);
    p_cb->rx_counter++;
}

nrfx_err_t nrfx_uart_rx(nrfx_uart_t const * p_instance,
                        uint8_t *           p_data,
                        size_t              length)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    NRFX_ASSERT(m_cb[p_instance->drv_inst_idx].state == NRFX_DRV_STATE_INITIALIZED);
    NRFX_ASSERT(p_data);
    NRFX_ASSERT(length > 0);

    nrfx_err_t err_code;

    bool second_buffer = false;

    if (p_cb->handler)
    {
        nrf_uart_int_disable(p_instance->p_reg, NRF_UART_INT_MASK_RXDRDY |
                                                NRF_UART_INT_MASK_ERROR);
    }
    if (p_cb->rx_buffer_length != 0)
    {
        if (p_cb->rx_secondary_buffer_length != 0)
        {
            if (p_cb->handler)
            {
//                nrf_uart_int_enable(p_instance->p_reg, NRF_UART_INT_MASK_RXDRDY |
//                                                       NRF_UART_INT_MASK_ERROR);
            }
            err_code = NRFX_ERROR_BUSY;
            NRFX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             NRFX_LOG_ERROR_STRING_GET(err_code));
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

    NRFX_LOG_INFO("Transfer rx_len: %d.", length);
   

    if (p_cb->handler == NULL)
    {
//        nrf_uart_event_clear(p_instance->p_reg, NRF_UART_EVENT_RXTO);

        bool rxrdy;
        bool rxto;
        bool error;
        do
        {
            do
            {
                error = nrf_uart_event_check(p_instance->p_reg, NRF_UART_EVENT_ERROR);
                rxrdy = nrf_uart_event_check(p_instance->p_reg, NRF_UART_EVENT_RXDRDY);
                rxto  = nrf_uart_event_check(p_instance->p_reg, NRF_UART_EVENT_RXTO);
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
            err_code = NRFX_ERROR_INTERNAL;
            NRFX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             NRFX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }

        if (rxto)
        {
            err_code = NRFX_ERROR_FORBIDDEN;
            NRFX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             NRFX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }

    }
    else
    {
//        nrf_uart_int_enable(p_instance->p_reg, NRF_UART_INT_MASK_RXDRDY |
//                                               NRF_UART_INT_MASK_ERROR);
    }
    err_code = NRFX_SUCCESS;
    NRFX_LOG_INFO("Function: %s, error code: %s.", __func__, NRFX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

bool nrfx_uart_rx_ready(nrfx_uart_t const * p_instance)
{
    return nrf_uart_event_check(p_instance->p_reg, NRF_UART_EVENT_RXDRDY);
}

void nrfx_uart_rx_enable(nrfx_uart_t const * p_instance)
{
    if (!m_cb[p_instance->drv_inst_idx].rx_enabled)
    {
        m_cb[p_instance->drv_inst_idx].rx_enabled = true;
    }
}

void nrfx_uart_rx_disable(nrfx_uart_t const * p_instance)
{

    m_cb[p_instance->drv_inst_idx].rx_enabled = false;
}

uint32_t nrfx_uart_errorsrc_get(nrfx_uart_t const * p_instance)
{
//    nrf_uart_event_clear(p_instance->p_reg, NRF_UART_EVENT_ERROR);
    return nrf_uart_errorsrc_get_and_clear(p_instance->p_reg);
}

static void rx_done_event(uart_control_block_t * p_cb,
                          size_t                 bytes,
                          uint8_t *              p_data)
{
    nrfx_uart_event_t event;

    event.type             = NRFX_UART_EVT_RX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = p_data;

    p_cb->handler(&event, p_cb->p_context);
}

static void tx_done_event(uart_control_block_t * p_cb,
                          size_t                 bytes)
{
    nrfx_uart_event_t event;

    event.type             = NRFX_UART_EVT_TX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = (uint8_t *)p_cb->p_tx_buffer;

    p_cb->tx_buffer_length = 0;

    p_cb->handler(&event, p_cb->p_context);
}

void nrfx_uart_tx_abort(nrfx_uart_t const * p_instance)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    p_cb->tx_abort = true;
    if (p_cb->handler)
    {
        tx_done_event(p_cb, p_cb->tx_counter);
    }

    NRFX_LOG_INFO("TX transaction aborted.");
}

void nrfx_uart_rx_abort(nrfx_uart_t const * p_instance)
{
    nrf_uart_int_disable(p_instance->p_reg, NRF_UART_INT_MASK_RXDRDY |
                                            NRF_UART_INT_MASK_ERROR);

    NRFX_LOG_INFO("RX transaction aborted.");
	
//		printf("nrfx_uart_rx_abort\r\n");
}

static void uart_irq_handler(NRF_UART_Type *        p_uart,
                             uart_control_block_t * p_cb)
{
	
		uint32_t IIR,IER;
		IER = p_uart->IER_DLH.IER;
		IIR = p_uart->IIR_FCR.IIR;
	//	printf("IER:%x,IIR:%x\r\n",IER,IIR);
//    if (nrf_uart_int_enable_check(p_uart, NRF_UART_INT_MASK_ERROR))
//    {
//        nrfx_uart_event_t event;
////        nrf_uart_event_clear(p_uart, NRF_UART_EVENT_ERROR);
//        NRFX_LOG_DEBUG("Event: %s.", EVT_TO_STR(NRF_UART_EVENT_ERROR));
//        nrf_uart_int_disable(p_uart, NRF_UART_INT_MASK_RXDRDY |
//                                     NRF_UART_INT_MASK_ERROR);
//        if (!p_cb->rx_enabled)
//        {
////            nrf_uart_task_trigger(p_uart, NRF_UART_TASK_STOPRX);
//        }
//        event.type                   = NRFX_UART_EVT_ERROR;
//        event.data.error.error_mask  = nrf_uart_errorsrc_get_and_clear(p_uart);
//        event.data.error.rxtx.bytes  = p_cb->rx_buffer_length;
//        event.data.error.rxtx.p_data = p_cb->p_rx_buffer;

//        // Abort transfer.
//        p_cb->rx_buffer_length = 0;
//        p_cb->rx_secondary_buffer_length = 0;

//        p_cb->handler(&event,p_cb->p_context);
//    }
//    else 
			
		if ((IER & 0X01) && ( ( (IIR & 0xf) == 0x04 )|| ( (IIR & 0xf) == 0x0c) ))
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
                nrf_uart_int_disable(p_uart, NRF_UART_INT_MASK_RXDRDY |
                                             NRF_UART_INT_MASK_ERROR);
                p_cb->rx_buffer_length = 0;
                rx_done_event(p_cb, p_cb->rx_counter, p_cb->p_rx_buffer);
            }
        }
				
				if((IIR & 0xf) == 0x0c)
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

  //  if (nrf_uart_event_check(p_uart, NRF_UART_EVENT_TXDRDY))
		if ((IER & 0X02) && ((IIR & 0x0f) == 0x02))
    {
        // Use a local variable to avoid undefined order of accessing two volatile variables
        // in one statement.
        size_t const tx_buffer_length = p_cb->tx_buffer_length;
		//	printf("tx:%d,%d\r\n",p_cb->tx_counter,tx_buffer_length);
        if (p_cb->tx_counter < tx_buffer_length && !p_cb->tx_abort)
        {
            tx_byte(p_uart, p_cb);
        }
        else
        {
//            nrf_uart_event_clear(p_uart, NRF_UART_EVENT_TXDRDY);
            if (p_cb->tx_buffer_length)
            {
                tx_done_event(p_cb, p_cb->tx_buffer_length);
            }
        }
    }

}


#if NRFX_CHECK(NRFX_UART0_ENABLED)
void nrfx_uart_0_irq_handler(void)
{
    uart_irq_handler(NRF_UART0, &m_cb[NRFX_UART0_INST_IDX]);
}
void	UART0_Handler(void)
{
	nrfx_uart_0_irq_handler();
		
}
#endif

#if NRFX_CHECK(NRFX_UART1_ENABLED)
void nrfx_uart_1_irq_handler(void)
{
    uart_irq_handler(NRF_UART1, &m_cb[NRFX_UART1_INST_IDX]);
}
void	UART1_Handler(void)
{
	nrfx_uart_1_irq_handler();
		
}
#endif

#endif // NRFX_CHECK(NRFX_UART_ENABLED)
