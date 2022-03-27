/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "sdk_common.h"
#if XINC_MODULE_ENABLED(APP_UART) && XINC_MODULE_ENABLED(APP_FIFO)
#include "app_uart.h"
#include "app_fifo.h"
#include "xinc_drv_uart.h"
#include "xinc_assert.h"

static xinc_drv_uart_t app_uart_inst = XINC_DRV_UART_INSTANCE(APP_UART_DRIVER_INSTANCE);

static __INLINE uint32_t fifo_length(app_fifo_t * const fifo)
{
  uint32_t tmp = fifo->read_pos;
  return fifo->write_pos - tmp;
}

#define FIFO_LENGTH(F) fifo_length(&F)              /**< Macro to calculate length of a FIFO. */


static app_uart_event_handler_t   m_event_handler;            /**< Event handler function. */
#define TRX_BUFF_SIZE   64UL
static uint8_t tx_buffer[TRX_BUFF_SIZE];
static uint8_t rx_buffer[TRX_BUFF_SIZE];
static uint8_t rx_buffsize = TRX_BUFF_SIZE;
static bool m_rx_ovf;

static app_fifo_t                  m_rx_fifo;                               /**< RX FIFO buffer for storing data received on the UART until the application fetches them using app_uart_get(). */
static app_fifo_t                  m_tx_fifo;                               /**< TX FIFO buffer for storing data to be transmitted on the UART when TXD is ready. Data is put to the buffer on using app_uart_put(). */

static void uart_event_handler(xinc_drv_uart_event_t * p_event, void* p_context)
{
    app_uart_evt_t app_uart_event;
    uint32_t err_code;
    uint32_t rxbytes;
    uint32_t len8;
    app_uart_event.evt_type = APP_UART_DATA_READY;
    switch (p_event->type)
    {
        
        case XINC_DRV_UART_EVT_RX_DONE:
            app_uart_event.evt_type = APP_UART_DATA_DONE;
        case XINC_DRV_UART_EVT_RX_READY:
            // If 0, then this is a RXTO event with no new bytes.
            

            // Write received byte to FIFO.
           // err_code = app_fifo_put(&m_rx_fifo, p_event->data.rxtx.p_data[0]);
            rxbytes = p_event->data.rxtx.bytes;
         //   printf("rxbytes:%d\r\n",rxbytes);
            
            err_code = app_fifo_write(&m_rx_fifo,p_event->data.rxtx.p_data,&rxbytes);
            
           // printf("rxbytes write:%d\r\n",rxbytes);
            
            if (err_code != XINC_SUCCESS)
            {
                app_uart_event.evt_type          = APP_UART_FIFO_ERROR;
                app_uart_event.data.error_code   = err_code;
                m_event_handler(&app_uart_event);
            }
            // Notify that there are data available.
            else if (FIFO_LENGTH(m_rx_fifo) != 0)
            {
              // app_uart_event.evt_type = APP_UART_DATA_READY;APP_UART_DATA_DONE
                m_event_handler(&app_uart_event);
            }
            
            // Start new RX if size in buffer.
            if ((FIFO_LENGTH(m_rx_fifo) <= m_rx_fifo.buf_size_mask) || (app_uart_event.evt_type == APP_UART_DATA_DONE))
            {
              //  printf("drv_uart_rx2:%d\r\n",rx_buffsize);
                (void)xinc_drv_uart_rx(&app_uart_inst, rx_buffer, rx_buffsize);
            }
            else
            {
                // Overflow in RX FIFO.
                m_rx_ovf = true;
            }

            break;
  

        case XINC_DRV_UART_EVT_ERROR:
            app_uart_event.evt_type                 = APP_UART_COMMUNICATION_ERROR;
            app_uart_event.data.error_communication = p_event->data.error.error_mask;
            (void)xinc_drv_uart_rx(&app_uart_inst, rx_buffer, rx_buffsize);
            m_event_handler(&app_uart_event);
            break;

        case XINC_DRV_UART_EVT_TX_DONE:
						
            // Get next byte from FIFO.
            len8 = 255;
          //  if (app_fifo_get(&m_tx_fifo, tx_buffer) == XINC_SUCCESS)
            if (app_fifo_read(&m_tx_fifo, tx_buffer,&len8) == XINC_SUCCESS)
            {
//								printf("TX_DONE 0\r\n");
                (void)xinc_drv_uart_tx(&app_uart_inst, tx_buffer, len8);
            }
            else
            {
				//printf("TX_DONE 1\r\n");
                // Last byte from FIFO transmitted, notify the application.
                app_uart_event.evt_type = APP_UART_TX_EMPTY;
                m_event_handler(&app_uart_event);
            }
            break;

        default:
            break;
    }
}


uint32_t app_uart_init(const app_uart_comm_params_t * p_comm_params,
                             app_uart_buffers_t *     p_buffers,
                             app_uart_event_handler_t event_handler,
                             app_irq_priority_t       irq_priority)
{
    uint32_t err_code;

    m_event_handler = event_handler;

    if (p_buffers == NULL)
    {
        return XINC_ERROR_INVALID_PARAM;
    }
    printf("app_uart_init baud:%d \r\n",p_comm_params->baud_rate);
    // Configure buffer RX buffer.
    err_code = app_fifo_init(&m_rx_fifo, p_buffers->rx_buf, p_buffers->rx_buf_size);
    VERIFY_SUCCESS(err_code);

    // Configure buffer TX buffer.
    err_code = app_fifo_init(&m_tx_fifo, p_buffers->tx_buf, p_buffers->tx_buf_size);
    VERIFY_SUCCESS(err_code);

    xinc_drv_uart_config_t config = XINC_DRV_UART_DEFAULT_CONFIG;
    config.baudrate = (xinc_uart_baudrate_t)p_comm_params->baud_rate;
    config.hwfc = (p_comm_params->flow_control == APP_UART_FLOW_CONTROL_DISABLED) ?
            XINC_UART_HWFC_DISABLED : XINC_UART_HWFC_ENABLED;
    config.interrupt_priority = irq_priority;
    config.parity = p_comm_params->use_parity ? XINC_UART_PARITY_INCLUDED : XINC_UART_PARITY_EXCLUDED;
    config.pselcts = p_comm_params->cts_pin_no;
    config.pselrts = p_comm_params->rts_pin_no;
    config.pselrxd = p_comm_params->rx_pin_no;
    config.pseltxd = p_comm_params->tx_pin_no;
    
    config.data_bits = p_comm_params->data_bits;
    config.stop_bits = p_comm_params->stop_bits;
    
    printf("config baud:%d \r\n",config.baudrate);

    err_code = xinc_drv_uart_init(&app_uart_inst, &config, uart_event_handler);
    VERIFY_SUCCESS(err_code);
    m_rx_ovf = false;

    // Turn on receiver if RX pin is connected
    if (p_comm_params->rx_pin_no != UART_PIN_DISCONNECTED)
    {
        return xinc_drv_uart_rx(&app_uart_inst, rx_buffer,rx_buffsize);
    }
    else
    {
        return XINC_SUCCESS;
    }
}


uint32_t app_uart_flush(void)
{
    uint32_t err_code;

    err_code = app_fifo_flush(&m_rx_fifo);
    VERIFY_SUCCESS(err_code);

    err_code = app_fifo_flush(&m_tx_fifo);
    VERIFY_SUCCESS(err_code);

    return XINC_SUCCESS;
}


uint32_t app_uart_get(uint8_t * p_byte)
{
    ASSERT(p_byte);
    bool rx_ovf = m_rx_ovf;

    ret_code_t err_code =  app_fifo_get(&m_rx_fifo, p_byte);

    // If FIFO was full new request to receive one byte was not scheduled. Must be done here.
    if (rx_ovf)
    {
        m_rx_ovf = false;
        uint32_t uart_err_code = xinc_drv_uart_rx(&app_uart_inst, rx_buffer, rx_buffsize);

        // RX resume should never fail.
        APP_ERROR_CHECK(uart_err_code);
    }

    return err_code;
}


uint32_t app_uart_put(uint8_t byte)
{
    uint32_t err_code;
    err_code = app_fifo_put(&m_tx_fifo, byte++);
    if (err_code == XINC_SUCCESS)
    {
        // The new byte has been added to FIFO. It will be picked up from there
        // (in 'uart_event_handler') when all preceding bytes are transmitted.
        // But if UART is not transmitting anything at the moment, we must start
        // a new transmission here.
        if (!xinc_drv_uart_tx_in_progress(&app_uart_inst))
        {
            // This operation should be almost always successful, since we've
            // just added a byte to FIFO, but if some bigger delay occurred
            // (some heavy interrupt handler routine has been executed) since
            // that time, FIFO might be empty already.
            uint32_t len8 = 255;
           // if (app_fifo_get(&m_tx_fifo, tx_buffer) == XINC_SUCCESS)
            if (app_fifo_read(&m_tx_fifo, tx_buffer,&len8) == XINC_SUCCESS)               
            {
                err_code = xinc_drv_uart_tx(&app_uart_inst, tx_buffer, len8);
            }
        }
    }
    return err_code;
}

uint32_t app_uart_gets(uint8_t * p_bytes,uint8_t* len)
{
    ASSERT(p_bytes);
    ASSERT(len);
    
    uint32_t rlen = *len;

    ret_code_t err_code =  app_fifo_read(&m_rx_fifo, p_bytes,&rlen);

    *len = rlen;
    return err_code;
}

uint32_t app_uart_puts(uint8_t* bytes,uint8_t len)
{
    uint32_t err_code;
    uint32_t write_len = len;
    /* CHECK available_count on fifo */
    err_code = app_fifo_write(&m_tx_fifo, NULL,&write_len);
    if(err_code == XINC_SUCCESS)
    {
        if(write_len >= len)
        {
            write_len = len;
            err_code = app_fifo_write(&m_tx_fifo, bytes,&write_len);
            if (err_code == XINC_SUCCESS)
            {
                // The new byte has been added to FIFO. It will be picked up from there
                // (in 'uart_event_handler') when all preceding bytes are transmitted.
                // But if UART is not transmitting anything at the moment, we must start
                // a new transmission here.
                if (!xinc_drv_uart_tx_in_progress(&app_uart_inst))
                {
                    // This operation should be almost always successful, since we've
                    // just added a byte to FIFO, but if some bigger delay occurred
                    // (some heavy interrupt handler routine has been executed) since
                    // that time, FIFO might be empty already.
                    uint32_t len8 = 255;
                    // if (app_fifo_get(&m_tx_fifo, tx_buffer) == XINC_SUCCESS)
                    if (app_fifo_read(&m_tx_fifo, tx_buffer,&len8) == XINC_SUCCESS)               
                    {
                        err_code = xinc_drv_uart_tx(&app_uart_inst, tx_buffer, len8);
                    }
                }
            }
        }
        else
        {
            err_code = XINC_ERROR_DATA_SIZE;
            return err_code;
        }       
    }
    
   
    return err_code;
}


uint32_t app_uart_close(void)
{
    xinc_drv_uart_uninit(&app_uart_inst);
    return XINC_SUCCESS;
}
#endif //XINC_MODULE_ENABLED(APP_UART)
