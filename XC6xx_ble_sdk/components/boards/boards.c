/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "boards.h"

#include <stdint.h>
#include <stdbool.h>

#if LEDS_NUMBER > 0
static const uint8_t m_board_led_list[LEDS_NUMBER] = LEDS_LIST;
#endif

#if BUTTONS_NUMBER > 0
static const uint8_t m_board_btn_list[BUTTONS_NUMBER] = BUTTONS_LIST;
#endif

#if LEDS_NUMBER > 0
bool bsp_board_led_state_get(uint32_t led_idx)
{
    ASSERT(led_idx < LEDS_NUMBER);
    bool pin_set = xinc_gpio_pin_out_read(m_board_led_list[led_idx]) ? true : false;
    return (pin_set == (LEDS_ACTIVE_STATE ? true : false));
}

void bsp_board_led_on(uint32_t led_idx)
{
        ASSERT(led_idx < LEDS_NUMBER);
        xinc_gpio_pin_write(m_board_led_list[led_idx], LEDS_ACTIVE_STATE ? 1 : 0);
}

void bsp_board_led_off(uint32_t led_idx)
{
    ASSERT(led_idx < LEDS_NUMBER);
    xinc_gpio_pin_write(m_board_led_list[led_idx], LEDS_ACTIVE_STATE ? 0 : 1);
}

void bsp_board_leds_off(void)
{
    uint32_t i;
    for (i = 0; i < LEDS_NUMBER; ++i)
    {
        bsp_board_led_off(i);
    }
}

void bsp_board_leds_on(void)
{
    uint32_t i;
    for (i = 0; i < LEDS_NUMBER; ++i)
    {
        bsp_board_led_on(i);
    }
}

void bsp_board_led_invert(uint32_t led_idx)
{
    ASSERT(led_idx < LEDS_NUMBER);
    xinc_gpio_pin_toggle(m_board_led_list[led_idx]);
}


static xincx_err_t bsp_board_leds_init(void)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    
    if (!xinc_drv_gpio_is_init())
    {
        err_code = xinc_drv_gpio_init();
        if(err_code != XINCX_SUCCESS)
        {
            return err_code;
        }
    }

    uint32_t i;
    
    xincx_gpio_out_config_t config;
    config.init_state = XINC_GPIO_INITIAL_VALUE_LOW;
    
    for (i = 0; i < LEDS_NUMBER; ++i)
    {
        err_code = xincx_gpio_out_init(m_board_led_list[i],&config);
        if(err_code != XINCX_SUCCESS)
        {
            return err_code;
        }
        
    }
    bsp_board_leds_off();
    
    return err_code;
}

uint32_t bsp_board_led_idx_to_pin(uint32_t led_idx)
{
    ASSERT(led_idx < LEDS_NUMBER);
    return m_board_led_list[led_idx];
}

uint32_t bsp_board_pin_to_led_idx(uint32_t pin_number)
{
    uint32_t ret = 0xFFFFFFFF;
    uint32_t i;
    for (i = 0; i < LEDS_NUMBER; ++i)
    {
        if (m_board_led_list[i] == pin_number)
        {
            ret = i;
            break;
        }
    }
    return ret;
}
#endif //LEDS_NUMBER > 0

#if BUTTONS_NUMBER > 0
bool bsp_board_button_state_get(uint32_t button_idx)
{
    ASSERT(button_idx < BUTTONS_NUMBER);
    bool pin_set = xinc_gpio_pin_read(m_board_btn_list[button_idx]) ? true : false;
    return (pin_set == (BUTTONS_ACTIVE_STATE ? true : false));
}

static xincx_err_t bsp_board_buttons_init(void)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    if (!xinc_drv_gpio_is_init())
    {
        err_code = xinc_drv_gpio_init();
        if(err_code != XINCX_SUCCESS)
        {
            return err_code;
        }
    }
    uint32_t i;
    xincx_gpio_in_config_t config;
    config.input_config.pin_pulll = XINC_GPIO_PIN_PULLUP;
    config.input_config.input_int = XINC_GPIO_PIN_INPUT_NOINT;
    for (i = 0; i < BUTTONS_NUMBER; ++i)
    {
       // xinc_gpio_cfg_input(m_board_btn_list[i], cfg);
        
        err_code =  xincx_gpio_in_init(m_board_btn_list[i],&config,NULL);

    }
    return err_code;
}

uint32_t bsp_board_pin_to_button_idx(uint32_t pin_number)
{
    uint32_t i;
    uint32_t ret = 0xFFFFFFFF;
    for (i = 0; i < BUTTONS_NUMBER; ++i)
    {
        if (m_board_btn_list[i] == pin_number)
        {
            ret = i;
            break;
        }
    }
    return ret;
}

uint32_t bsp_board_button_idx_to_pin(uint32_t button_idx)
{
    ASSERT(button_idx < BUTTONS_NUMBER);
    return m_board_btn_list[button_idx];
}
#endif //BUTTONS_NUMBER > 0


xincx_err_t bsp_board_init(uint32_t init_flags)
{
    xincx_err_t err_code;

    #if LEDS_NUMBER > 0
    if (init_flags & BSP_INIT_LEDS)
    {
       err_code  = bsp_board_leds_init();
    }
    #endif //LEDS_NUMBER > 0
    
    #if BUTTONS_NUMBER > 0
    if (init_flags & BSP_INIT_BUTTONS)
    {
        bsp_board_buttons_init();
    }
    #endif //BUTTONS_NUMBER > 0
    
    return err_code;

}
