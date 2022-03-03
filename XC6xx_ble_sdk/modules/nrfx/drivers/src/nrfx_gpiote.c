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

#if NRFX_CHECK(NRFX_GPIOTE_ENABLED)

#include <nrfx_gpiote.h>
#include "nrf_bitmask.h"
#include <string.h>


#define NRFX_LOG_MODULE GPIOTE
#include <nrfx_log.h>

#if (GPIO_COUNT == 1)
#define MAX_PIN_NUMBER 32
#elif (GPIO_COUNT == 2)
#define MAX_PIN_NUMBER (32 + P1_PIN_NUM)
#else
#error "Not supported."
#endif

#define FORBIDDEN_HANDLER_ADDRESS ((nrfx_gpiote_evt_handler_t)UINT32_MAX)
#define PIN_NOT_USED              (-1)
#define PIN_USED                  (-2)
#define NO_CHANNELS               (-1)
#define POLARITY_FIELD_POS        (6)
#define POLARITY_FIELD_MASK       (0xC0)

/* Check if every pin can be encoded on provided number of bits. */
//NRFX_STATIC_ASSERT(MAX_PIN_NUMBER <= (1 << POLARITY_FIELD_POS));

/**
 * @brief Macro for converting task-event index to an address of an event register.
 *
 * Macro utilizes the fact that registers are grouped together in ascending order.
 */
#define TE_IDX_TO_EVENT_ADDR(idx)    (nrf_gpiote_events_t)((uint32_t)NRF_GPIOTE_EVENTS_IN_0 + \
                                                           (sizeof(uint32_t) * (idx)))

/**
 * @brief Macro for converting task-event index of OUT task to an address of a task register.
 *
 * Macro utilizes the fact that registers are grouped together in ascending order.
 */
#define TE_OUT_IDX_TO_TASK_ADDR(idx) (nrf_gpiote_tasks_t)((uint32_t)NRF_GPIOTE_TASKS_OUT_0 + \
                                                          (sizeof(uint32_t) * (idx)))

#if defined(GPIOTE_FEATURE_SET_PRESENT) || defined(__NRFX_DOXYGEN__)
/**
 * @brief Macro for converting task-event index of SET task to an address of a task register.
 *
 * Macro utilizes the fact that registers are grouped together in ascending order.
 */
#define TE_SET_IDX_TO_TASK_ADDR(idx) (nrf_gpiote_tasks_t)((uint32_t)NRF_GPIOTE_TASKS_SET_0 + \
                                                          (sizeof(uint32_t) * (idx)))

#endif // defined(GPIOTE_FEATURE_SET_PRESENT) || defined(__NRFX_DOXYGEN__)

#if defined(GPIOTE_FEATURE_CLR_PRESENT) || defined(__NRFX_DOXYGEN__)
/**
 * @brief Macro for converting task-event index of CLR task to an address of a task register.
 *
 * Macro utilizes the fact that registers are grouped together in ascending order.
 */
#define TE_CLR_IDX_TO_TASK_ADDR(idx) (nrf_gpiote_tasks_t)((uint32_t)NRF_GPIOTE_TASKS_CLR_0 + \
                                                          (sizeof(uint32_t) * (idx)))

#endif // defined(GPIOTE_FEATURE_CLR_PRESENT) || defined(__NRFX_DOXYGEN__)

/*lint -save -e571*/ /* Suppress "Warning 571: Suspicious cast" */
typedef struct
{
    nrfx_gpiote_evt_handler_t handlers[MAX_PIN_NUMBER];

    int8_t                    pin_assignments[MAX_PIN_NUMBER];
    int8_t                    port_handlers_pins[NRFX_GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS];
    uint8_t                   configured_pins[((MAX_PIN_NUMBER)+7) / 8];
    nrfx_drv_state_t          state;
	
		NRF_GPIOTE_Type           NRF_GPIOTE_STRUCT;
} gpiote_control_block_t;

static gpiote_control_block_t m_cb;

__STATIC_INLINE bool pin_in_use(uint32_t pin)
{
    return (m_cb.pin_assignments[pin] != PIN_NOT_USED);
}


__STATIC_INLINE bool pin_in_use_by_gpiote(uint32_t pin)
{
    return (m_cb.pin_assignments[pin] >= 0);
}


__STATIC_INLINE void pin_in_use_by_te_set(uint32_t                  pin,
                                          uint32_t                  channel_id,
                                          nrfx_gpiote_evt_handler_t handler,
                                          bool                      is_channel)
{
    m_cb.pin_assignments[pin] = channel_id;
    m_cb.handlers[channel_id] = handler;
   
}


__STATIC_INLINE void pin_in_use_set(uint32_t pin)
{
    m_cb.pin_assignments[pin] = PIN_USED;
}


__STATIC_INLINE void pin_in_use_clear(uint32_t pin)
{
    m_cb.pin_assignments[pin] = PIN_NOT_USED;
}


__STATIC_INLINE void pin_configured_set(uint32_t pin)
{
    nrf_bitmask_bit_set(pin, m_cb.configured_pins);
}

__STATIC_INLINE void pin_configured_clear(uint32_t pin)
{
    nrf_bitmask_bit_clear(pin, m_cb.configured_pins);
}

__STATIC_INLINE bool pin_configured_check(uint32_t pin)
{
    return 0 != nrf_bitmask_bit_is_set(pin, m_cb.configured_pins);
}

__STATIC_INLINE int8_t channel_port_get(uint32_t pin)
{
    return m_cb.pin_assignments[pin];
}


__STATIC_INLINE nrfx_gpiote_evt_handler_t channel_handler_get(uint32_t channel)
{
    return m_cb.handlers[channel];
}


static int8_t channel_port_alloc(uint32_t pin, nrfx_gpiote_evt_handler_t handler, bool channel)
{
    int8_t   channel_id = NO_CHANNELS;
    uint32_t i;

    uint32_t start_idx = 0;//channel ? 0 : GPIOTE_CH_NUM;
    uint32_t end_idx   = MAX_PIN_NUMBER;
      //  channel ? GPIOTE_CH_NUM : (GPIOTE_CH_NUM + NRFX_GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS);

    // critical section

    for (i = start_idx; i < end_idx; i++)
    {
        if (m_cb.handlers[i] == FORBIDDEN_HANDLER_ADDRESS)
        {
            pin_in_use_by_te_set(pin, i, handler, channel);
            channel_id = i;
            break;
        }
    }
		printf("channel_port_alloc :%d\r\n",channel_id);
    // critical section
    return channel_id;
}


static void channel_free(uint8_t channel_id)
{
    m_cb.handlers[channel_id] = FORBIDDEN_HANDLER_ADDRESS;
}


nrfx_err_t nrfx_gpiote_init(void)
{
    nrfx_err_t err_code;

    if (m_cb.state != NRFX_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRFX_ERROR_INVALID_STATE;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
		printf("Function 0: %s\r\n ", __func__);
    uint8_t i;

    for (i = 0; i < MAX_PIN_NUMBER; i++)
    {
      //  if (nrf_gpio_pin_present_check(i))
        {
            pin_in_use_clear(i);
        }
				channel_free(i);
    }
 
    memset(m_cb.configured_pins, 0, sizeof(m_cb.configured_pins));

		NVIC_EnableIRQ(GPIO_IRQn);
		NVIC_EnableIRQ(PendSV_IRQn);

    m_cb.state = NRFX_DRV_STATE_INITIALIZED;

    err_code = NRFX_SUCCESS;
    NRFX_LOG_INFO("Function: %s, error code: %s.", __func__, NRFX_LOG_ERROR_STRING_GET(err_code));
		printf("Function 1: %s, error:%x \r\n ", __func__,err_code);
    return err_code;
}


bool nrfx_gpiote_is_init(void)
{
    return (m_cb.state != NRFX_DRV_STATE_UNINITIALIZED) ? true : false;
}


void nrfx_gpiote_uninit(void)
{
    NRFX_ASSERT(m_cb.state != NRFX_DRV_STATE_UNINITIALIZED);

    uint32_t i;

    for (i = 0; i < MAX_PIN_NUMBER; i++)
    {   
        if (nrf_gpio_pin_present_check(i))
        {
           
            if (pin_in_use_by_gpiote(i))
            {
                /* Disable gpiote_in is having the same effect on out pin as gpiote_out_uninit on
                 * so it can be called on all pins used by GPIOTE.
                 */
                nrfx_gpiote_in_uninit(i);
            }
        }
    }
    m_cb.state = NRFX_DRV_STATE_UNINITIALIZED;
    NRFX_LOG_INFO("Uninitialized.");
}


nrfx_err_t nrfx_gpiote_out_init(nrfx_gpiote_pin_t                pin,
                                nrfx_gpiote_out_config_t const * p_config)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    NRFX_ASSERT(m_cb.state == NRFX_DRV_STATE_INITIALIZED);
    NRFX_ASSERT(p_config);

    nrfx_err_t err_code = NRFX_SUCCESS;

    if (pin_in_use(pin))
    {
        err_code = NRFX_ERROR_INVALID_STATE;
    }
    else
    {
           
        pin_in_use_set(pin);
        
        if (err_code == NRFX_SUCCESS)
        {
					  nrf_gpio_cfg_output(pin);
					  pin_configured_set(pin);
            if (p_config->init_state == NRF_GPIOTE_INITIAL_VALUE_HIGH)
            {
                nrf_gpio_pin_set(pin);
            }
            else
            {
                nrf_gpio_pin_clear(pin);
            }     
           
        }
    }

    NRFX_LOG_INFO("Function: %s, error code: %s.", __func__, NRFX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}


void nrfx_gpiote_out_uninit(nrfx_gpiote_pin_t pin)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    NRFX_ASSERT(pin_in_use(pin));

    pin_in_use_clear(pin);

    if (pin_configured_check(pin))
    {
        nrf_gpio_cfg_default(pin);
        pin_configured_clear(pin);
    }
}


void nrfx_gpiote_out_set(nrfx_gpiote_pin_t pin)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    NRFX_ASSERT(pin_in_use(pin));


    nrf_gpio_pin_set(pin);
}


void nrfx_gpiote_out_clear(nrfx_gpiote_pin_t pin)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    NRFX_ASSERT(pin_in_use(pin));


    nrf_gpio_pin_clear(pin);
}


void nrfx_gpiote_out_toggle(nrfx_gpiote_pin_t pin)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    NRFX_ASSERT(pin_in_use(pin));


    nrf_gpio_pin_toggle(pin);
}


nrfx_err_t nrfx_gpiote_in_init(nrfx_gpiote_pin_t               pin,
                               nrfx_gpiote_in_config_t const * p_config,
                               nrfx_gpiote_evt_handler_t       evt_handler)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    nrfx_err_t err_code = NRFX_SUCCESS;

    {
        int8_t channel = channel_port_alloc(pin, evt_handler, p_config->hi_accuracy);
			   printf("evt_handler : %p \r\n ", evt_handler);
        if (channel != NO_CHANNELS)
        {
            
            {              
                {
                    nrf_gpio_cfg_input(pin, p_config->pull);
										printf("nrf_gpio_cfg_input : %d \r\n ", pin);
                }
                pin_configured_set(pin);
            }

        }
        else
        {
            err_code = NRFX_ERROR_NO_MEM;
        }
    }

    NRFX_LOG_INFO("Function: %s, error code: %s.", __func__, NRFX_LOG_ERROR_STRING_GET(err_code));
		printf("Function: %s, error code: %x.\r\n", __func__, (err_code));
    return err_code;
}

void nrfx_gpiote_in_event_enable(nrfx_gpiote_pin_t pin, bool int_enable)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    NRFX_ASSERT(pin_in_use_by_gpiote(pin));

}


void nrfx_gpiote_in_event_disable(nrfx_gpiote_pin_t pin)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    NRFX_ASSERT(pin_in_use_by_gpiote(pin));
}


void nrfx_gpiote_in_uninit(nrfx_gpiote_pin_t pin)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    NRFX_ASSERT(pin_in_use_by_gpiote(pin));
    nrfx_gpiote_in_event_disable(pin);
   
    if (pin_configured_check(pin))
    {
        nrf_gpio_cfg_default(pin);
        pin_configured_clear(pin);
    }
    {
        channel_free((uint8_t)channel_port_get(pin));
    }
    pin_in_use_clear(pin);
}


bool nrfx_gpiote_in_is_set(nrfx_gpiote_pin_t pin)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    return nrf_gpio_pin_read(pin) ? true : false;
}




//static bool input_read_and_check(uint32_t * input, uint32_t * pins_to_check)
//{
//    bool process_inputs_again;
//    uint32_t new_input[GPIO_COUNT];

//    nrf_gpio_ports_read(0, GPIO_COUNT, new_input);

//    process_inputs_again = false;
//    for (uint32_t port_idx = 0; port_idx < GPIO_COUNT; port_idx++)
//    {
//        /* Execute XOR to find out which inputs have changed. */
//        uint32_t input_diff = input[port_idx] ^ new_input[port_idx];
//        input[port_idx] = new_input[port_idx];
//        if (input_diff)
//        {
//            /* If any differences among inputs were found, mark those pins
//             * to be processed again. */
//            pins_to_check[port_idx] = input_diff;
//            process_inputs_again = true;
//        }
//        else
//        {
//            pins_to_check[port_idx] = 0;
//        }
//    }
//    return process_inputs_again;
//}


void nrfx_gpiote_irq_handler(void)
{
    uint32_t status            = 0;
		uint32_t            i;
		uint32_t            mask  = (uint32_t)0x01;
		
		NRF_GPIO_Type * gpio_regs[GPIO_COUNT] = GPIO_REG_LIST;
		
		status = gpio_regs[0]->INTR_STATUS_C0;
		
		gpio_regs[0]->INTR_CLR0 = status;
		

    /* collect status of all GPIOTE pin events. Processing is done once all are collected and cleared.*/
		    /* Process pin events. */
		if (status)
		{
			for (i = 0; i < MAX_PIN_NUMBER; i++)
			{
        if (mask & status)
        {
				//	printf("nrfx_gpiote_irq_handler:%x,i:%d\n",status,i);
             nrfx_gpiote_evt_handler_t handler =
                        channel_handler_get((uint32_t)channel_port_get(i));
					if (handler)
					{
							handler(i, NRF_GPIOTE_POLARITY_LOTOHI);
					}
            status |= mask;
        }
        mask <<= 1;
        /* Incrementing to next event, utilizing the fact that events are grouped together
         * in ascending order. */
			}
		}
		
}


/*lint -restore*/
#endif // NRFX_CHECK(NRFX_GPIOTE_ENABLED)
