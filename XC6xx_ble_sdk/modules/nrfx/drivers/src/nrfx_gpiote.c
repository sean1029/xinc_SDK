/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
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
#define NO_HANDLERS_ID               (-1)
#define POLARITY_FIELD_POS        (6)
#define POLARITY_FIELD_MASK       (0xC0)


/*lint -save -e571*/ /* Suppress "Warning 571: Suspicious cast" */
typedef struct
{
    nrfx_gpiote_evt_handler_t handlers[MAX_PIN_NUMBER];
    int8_t                    pin_assignments[MAX_PIN_NUMBER];
    uint8_t                   configured_pins[((MAX_PIN_NUMBER)+7) / 8];
    nrfx_drv_state_t          state;
} gpiote_control_block_t;

static gpiote_control_block_t m_cb;

__STATIC_INLINE bool pin_in_use(uint32_t pin)
{
    return (m_cb.pin_assignments[pin] != PIN_NOT_USED);
}


__STATIC_INLINE bool pin_in_use_by_gpio_handler(uint32_t pin)
{
    return (m_cb.pin_assignments[pin] >= 0);
}


__STATIC_INLINE void pin_in_use_by_handler_set(uint32_t                  pin,
                                        uint32_t                  handler_id,
                                        nrfx_gpiote_evt_handler_t handler)
{
    m_cb.pin_assignments[pin] = handler_id;
    m_cb.handlers[handler_id] = handler;
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

__STATIC_INLINE int8_t pin_handler_id_get(uint32_t pin)
{
    return m_cb.pin_assignments[pin];
}


__STATIC_INLINE nrfx_gpiote_evt_handler_t pin_handler_get(uint32_t channel)
{
    return m_cb.handlers[channel];
}


static int8_t pin_handler_use_alloc(uint32_t pin, nrfx_gpiote_evt_handler_t handler)
{
    int8_t   handler_id = NO_HANDLERS_ID;
    uint32_t i;

    uint32_t start_idx = 0;
    uint32_t end_idx   = MAX_PIN_NUMBER;

    // critical section

    for (i = start_idx; i < end_idx; i++)
    {
        if (m_cb.handlers[i] == FORBIDDEN_HANDLER_ADDRESS)
        {
            pin_in_use_by_handler_set(pin, i, handler);
            handler_id = i;
            break;
        }
    }
    printf("pin_handler_use_alloc :%d\r\n",handler_id);
    // critical section
    return handler_id;
}


static void pin_handler_free(uint8_t handler_id)
{
    m_cb.handlers[handler_id] = FORBIDDEN_HANDLER_ADDRESS;
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
        if (nrf_gpio_pin_present_check(i))
        {
            pin_in_use_clear(i);
        }
        pin_handler_free(i);
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
           
            if (pin_in_use_by_gpio_handler(i))
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
    if (pin_in_use_by_gpio_handler(pin))
    {
        err_code = NRFX_ERROR_INVALID_STATE;
    }
    else
    {

        int8_t handler_id = pin_handler_use_alloc(pin, evt_handler);
        printf("evt_handler : %p \r\n ", evt_handler);
        if (handler_id != NO_HANDLERS_ID)
        {     
            nrf_gpio_cfg_input(pin, p_config->pull);
            printf("nrf_gpio_cfg_input : %d \r\n ", pin);

            pin_configured_set(pin);         
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
    NRFX_ASSERT(pin_in_use_by_gpio_handler(pin));

}


void nrfx_gpiote_in_event_disable(nrfx_gpiote_pin_t pin)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    NRFX_ASSERT(pin_in_use_by_gpio_handler(pin));
}


void nrfx_gpiote_in_uninit(nrfx_gpiote_pin_t pin)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    NRFX_ASSERT(pin_in_use_by_gpio_handler(pin));
    nrfx_gpiote_in_event_disable(pin);
   
    if (pin_configured_check(pin))
    {
        nrf_gpio_cfg_default(pin);
        pin_configured_clear(pin);
    }
    {
        pin_handler_free((uint8_t)pin_handler_id_get(pin));
    }
    pin_in_use_clear(pin);
}


bool nrfx_gpiote_in_is_set(nrfx_gpiote_pin_t pin)
{
    NRFX_ASSERT(nrf_gpio_pin_present_check(pin));
    return nrf_gpio_pin_read(pin) ? true : false;
}


void nrfx_gpiote_irq_handler(void)
{
    uint32_t            status     = 0;
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
                pin_handler_get((uint32_t)pin_handler_id_get(i));
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
