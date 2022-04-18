/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "bsp.h"
#include <stddef.h>
#include <stdio.h>
#include "xinchip_common.h"
#include "xinc.h"
#include "xinc_gpio.h"
#include "xinc_error.h"
#include "bsp_config.h"
#include "boards.h"

#ifndef BSP_SIMPLE
#include "app_timer.h"
#include "app_button.h"
#endif // BSP_SIMPLE

#if LEDS_NUMBER > 0 && !(defined BSP_SIMPLE)
static bsp_indication_t m_stable_state        = BSP_INDICATE_IDLE;
static bool             m_leds_clear          = false;
static uint32_t         m_indication_type     = 0;
static bool             m_alert_on            = false;
APP_TIMER_DEF(m_bsp_leds_tmr);
APP_TIMER_DEF(m_bsp_alert_tmr);
#endif // LEDS_NUMBER > 0 && !(defined BSP_SIMPLE)

#if BUTTONS_NUMBER > 0
#ifndef BSP_SIMPLE
static bsp_event_callback_t   m_registered_callback         = NULL;
static bsp_button_event_cfg_t m_events_list[BUTTONS_NUMBER] = {{BSP_EVENT_NOTHING, BSP_EVENT_NOTHING}};

		
#ifdef BSP_BUTTON_0
APP_TIMER_DEF(m_bsp_button_tmr);
#endif // BUTTON_0

#ifdef BSP_BUTTON_1
APP_TIMER_DEF(m_bsp_button_tmr1);
#endif // BUTTON_1

#ifdef BSP_BUTTON_2
APP_TIMER_DEF(m_bsp_button_tmr2);
#endif // BUTTON_2

#ifdef BSP_BUTTON_3
APP_TIMER_DEF(m_bsp_button_tmr3);
#endif // BUTTON_3

#ifdef BSP_BUTTON_4
APP_TIMER_DEF(m_bsp_button_tmr4);
#endif // BUTTON_4

#ifdef BSP_BUTTON_5
APP_TIMER_DEF(m_bsp_button_tmr5);
#endif // BUTTON_5

#ifdef BSP_BUTTON_6
APP_TIMER_DEF(m_bsp_button_tmr6);
#endif // BUTTON_6

#ifdef BSP_BUTTON_7
APP_TIMER_DEF(m_bsp_button_tmr7);
#endif // BUTTON_7

static void bsp_button_event_handler(uint8_t pin_no, uint8_t button_action);
#endif // BSP_SIMPLE

#ifndef BSP_SIMPLE
static const app_button_cfg_t app_buttons[BUTTONS_NUMBER] =
{
	
    #ifdef BSP_BUTTON_0
    {BSP_BUTTON_0, false, BUTTON_PULL, bsp_button_event_handler},
    #endif // BUTTON_0

    #ifdef BSP_BUTTON_1
    {BSP_BUTTON_1, false, BUTTON_PULL, bsp_button_event_handler},
    #endif // BUTTON_1

    #ifdef BSP_BUTTON_2
    {BSP_BUTTON_2, false, BUTTON_PULL, bsp_button_event_handler},
    #endif // BUTTON_2

    #ifdef BSP_BUTTON_3
    {BSP_BUTTON_3, false, BUTTON_PULL, bsp_button_event_handler},
    #endif // BUTTON_3

    #ifdef BSP_BUTTON_4
    {BSP_BUTTON_4, false, BUTTON_PULL, bsp_button_event_handler},
    #endif // BUTTON_4

    #ifdef BSP_BUTTON_5
    {BSP_BUTTON_5, false, BUTTON_PULL, bsp_button_event_handler},
    #endif // BUTTON_5

    #ifdef BSP_BUTTON_6
    {BSP_BUTTON_6, false, BUTTON_PULL, bsp_button_event_handler},
    #endif // BUTTON_6

    #ifdef BSP_BUTTON_7
    {BSP_BUTTON_7, false, BUTTON_PULL, bsp_button_event_handler},
    #endif // BUTTON_7

};
#endif // BSP_SIMPLE
#endif // BUTTONS_NUMBER > 0

#if (BUTTONS_NUMBER > 0)
bool bsp_button_is_pressed(uint32_t button)
{
    if (button < BUTTONS_NUMBER)
    {
        return bsp_board_button_state_get(button);
    }
    else
    {
        //If button is not present always return false
        return false;
    }
}
#endif

#if (BUTTONS_NUMBER > 0) && !(defined BSP_SIMPLE)
/**@brief Function for handling button events.
 *
 * @param[in]   pin_no          The pin number of the button pressed.
 * @param[in]   button_action   Action button.
 */
static void bsp_button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    bsp_event_t        event  = BSP_EVENT_NOTHING;
    uint32_t           button = 0;
    uint32_t           err_code;
    static uint8_t     current_long_push_pin_no[BUTTONS_NUMBER];              /**< Pin number of a currently pushed button, that could become a long push if held long enough. */
    static bsp_event_t release_event_at_push[BUTTONS_NUMBER]; /**< Array of what the release event of each button was last time it was pushed, so that no release event is sent if the event was bound after the push of the button. */

    button = bsp_board_pin_to_button_idx(pin_no);
//		printf(" bsp_button :%d,button_action:%d\r\n",button,button_action);
    if (button < BUTTONS_NUMBER)
    {
        switch (button_action)
        {
            case APP_BUTTON_PUSH:
            {     
                event = m_events_list[button].push_event;
                if (m_events_list[button].long_push_event != BSP_EVENT_NOTHING)
                {						
                    switch(button)
                    {
                        case 0:
                        {
                            #ifdef BSP_BUTTON_0
                            err_code = app_timer_start(m_bsp_button_tmr, APP_TIMER_TICKS(BSP_LONG_PUSH_TIMEOUT_MS), (void*)&current_long_push_pin_no[button]);
                            #endif
                        }break;
                        
                        case 1:
                        {
                            #ifdef BSP_BUTTON_1
                            err_code = app_timer_start(m_bsp_button_tmr1, APP_TIMER_TICKS(BSP_LONG_PUSH_TIMEOUT_MS), (void*)&current_long_push_pin_no[button]);
                            #endif
                        }break;	
                        
                        case 2:
                        {
                            #ifdef BSP_BUTTON_2
                            err_code = app_timer_start(m_bsp_button_tmr2, APP_TIMER_TICKS(BSP_LONG_PUSH_TIMEOUT_MS), (void*)&current_long_push_pin_no[button]);
                            #endif
                        }break;	
                        
                        case 3:
                        {
                            #ifdef BSP_BUTTON_3
                            err_code = app_timer_start(m_bsp_button_tmr3, APP_TIMER_TICKS(BSP_LONG_PUSH_TIMEOUT_MS), (void*)&current_long_push_pin_no[button]);
                            #endif
                        }break;	
                        
                        case 4:
                        {
                            #ifdef BSP_BUTTON_4
                            err_code = app_timer_start(m_bsp_button_tmr4, APP_TIMER_TICKS(BSP_LONG_PUSH_TIMEOUT_MS), (void*)&current_long_push_pin_no[button]);
                            #endif
                        }break;	
                                            
                        case 5:
                        {
                            #ifdef BSP_BUTTON_5
                            err_code = app_timer_start(m_bsp_button_tmr5, APP_TIMER_TICKS(BSP_LONG_PUSH_TIMEOUT_MS), (void*)&current_long_push_pin_no[button]);
                            #endif
                        }break;	
                        
                        case 6:
                        {
                            #ifdef BSP_BUTTON_6
                            err_code = app_timer_start(m_bsp_button_tmr6, APP_TIMER_TICKS(BSP_LONG_PUSH_TIMEOUT_MS), (void*)&current_long_push_pin_no[button]);
                            #endif
                        }break;	
                        
                        case 7:
                        {
                            #ifdef BSP_BUTTON_7
                            err_code = app_timer_start(m_bsp_button_tmr7, APP_TIMER_TICKS(BSP_LONG_PUSH_TIMEOUT_MS), (void*)&current_long_push_pin_no[button]);
                            #endif
                        }break;	
                        
                        default:
                            break;
                    }

									 
                    if (err_code == XINC_SUCCESS)
                    {
                        current_long_push_pin_no[button] = pin_no;
                    }
                }

                release_event_at_push[button] = m_events_list[button].release_event;
            }break;

            case APP_BUTTON_RELEASE:
            {
                
                switch(button)
                {
                    case 0:
                    {		
                        #ifdef BSP_BUTTON_0
                        (void)app_timer_stop(m_bsp_button_tmr);
                        #endif
                    }break;
                    
                    case 1:
                    {
                        #ifdef BSP_BUTTON_1
                        (void)app_timer_stop(m_bsp_button_tmr1);
                        #endif
                    }break;	
                        
                    case 2:
                    {
                        #ifdef BSP_BUTTON_2
                        (void)app_timer_stop(m_bsp_button_tmr2);
                        #endif
                    }break;	
                    
                    case 3:
                    {
                        #ifdef BSP_BUTTON_3
                        (void)app_timer_stop(m_bsp_button_tmr3);
                        #endif
                    }break;	
                    
                    case 4:
                    {
                        #ifdef BSP_BUTTON_4
                        (void)app_timer_stop(m_bsp_button_tmr4);
                        #endif
                    }break;	
                    
                    case 5:
                    {
                        #ifdef BSP_BUTTON_5
                        (void)app_timer_stop(m_bsp_button_tmr5);
                        #endif
                    }break;	
                    
                    case 6:
                    {
                        #ifdef BSP_BUTTON_6
                        (void)app_timer_stop(m_bsp_button_tmr6);
                        #endif
                    }break;	
                        
                    case 7:
                    {
                        #ifdef BSP_BUTTON_7
                        (void)app_timer_stop(m_bsp_button_tmr7);
                        #endif
                    }break;	
                    
                    default:
                        break;
                }
                if (release_event_at_push[button] == m_events_list[button].release_event)
                {
                    event = m_events_list[button].release_event;
                }
            }break;

            case BSP_BUTTON_ACTION_LONG_PUSH:
            {
                 event = m_events_list[button].long_push_event;
            }break;
               
        }
    }

    if ((event != BSP_EVENT_NOTHING) && (m_registered_callback != NULL))
    {
        m_registered_callback(event);
    }
}

/**@brief Handle events from button timer.
 *
 * @param[in]   p_context   parameter registered in timer start function.
 */
static void button_timer_handler(void * p_context)
{
    bsp_button_event_handler(*(uint8_t *)p_context, BSP_BUTTON_ACTION_LONG_PUSH);
}


#endif // (BUTTONS_NUMBER > 0) && !(defined BSP_SIMPLE)


#if LEDS_NUMBER > 0 && !(defined BSP_SIMPLE)
static void leds_off(void)
{
    if (m_alert_on)
    {
        uint32_t i;
        for (i = 0; i < LEDS_NUMBER; i++)
        {
            if (i != BSP_LED_ALERT)
            {
                bsp_board_led_off(i);
            }
        }
    }
    else
    {
        bsp_board_leds_off();
    }
}


/**@brief       Configure leds to indicate required state.
 * @param[in]   indicate   State to be indicated.
 */
static uint32_t bsp_led_indication(bsp_indication_t indicate)
{
    uint32_t err_code   = XINC_SUCCESS;
    uint32_t next_delay = 0;

    if (m_leds_clear)
    {
        m_leds_clear = false;
        leds_off();
    }

    switch (indicate)
    {
        case BSP_INDICATE_IDLE:
            leds_off();
            err_code       = app_timer_stop(m_bsp_leds_tmr);
            m_stable_state = indicate;
            break;

        case BSP_INDICATE_SCANNING:
        case BSP_INDICATE_ADVERTISING:
            // in advertising blink LED_0
            if (bsp_board_led_state_get(BSP_LED_INDICATE_INDICATE_ADVERTISING))
            {
                bsp_board_led_off(BSP_LED_INDICATE_INDICATE_ADVERTISING);
                next_delay = indicate ==
                             BSP_INDICATE_ADVERTISING ? ADVERTISING_LED_OFF_INTERVAL :
                             ADVERTISING_SLOW_LED_OFF_INTERVAL;
            }
            else
            {
                bsp_board_led_on(BSP_LED_INDICATE_INDICATE_ADVERTISING);
                next_delay = indicate ==
                             BSP_INDICATE_ADVERTISING ? ADVERTISING_LED_ON_INTERVAL :
                             ADVERTISING_SLOW_LED_ON_INTERVAL;
            }

            m_stable_state = indicate;
            err_code       = app_timer_start(m_bsp_leds_tmr, APP_TIMER_TICKS(next_delay), NULL);
            break;

        case BSP_INDICATE_ADVERTISING_WHITELIST:
            // in advertising quickly blink LED_0
            if (bsp_board_led_state_get(BSP_LED_INDICATE_ADVERTISING_WHITELIST))
            {
                bsp_board_led_off(BSP_LED_INDICATE_ADVERTISING_WHITELIST);
                next_delay = indicate ==
                             BSP_INDICATE_ADVERTISING_WHITELIST ?
                             ADVERTISING_WHITELIST_LED_OFF_INTERVAL :
                             ADVERTISING_SLOW_LED_OFF_INTERVAL;
            }
            else
            {
                bsp_board_led_on(BSP_LED_INDICATE_ADVERTISING_WHITELIST);
                next_delay = indicate ==
                             BSP_INDICATE_ADVERTISING_WHITELIST ?
                             ADVERTISING_WHITELIST_LED_ON_INTERVAL :
                             ADVERTISING_SLOW_LED_ON_INTERVAL;
            }
            m_stable_state = indicate;
            err_code       = app_timer_start(m_bsp_leds_tmr, APP_TIMER_TICKS(next_delay), NULL);
            break;

        case BSP_INDICATE_ADVERTISING_SLOW:
            // in advertising slowly blink LED_0
            if (bsp_board_led_state_get(BSP_LED_INDICATE_ADVERTISING_SLOW))
            {
                bsp_board_led_off(BSP_LED_INDICATE_ADVERTISING_SLOW);
                next_delay = indicate ==
                             BSP_INDICATE_ADVERTISING_SLOW ? ADVERTISING_SLOW_LED_OFF_INTERVAL :
                             ADVERTISING_SLOW_LED_OFF_INTERVAL;
            }
            else
            {
                bsp_board_led_on(BSP_LED_INDICATE_ADVERTISING_SLOW);
                next_delay = indicate ==
                             BSP_INDICATE_ADVERTISING_SLOW ? ADVERTISING_SLOW_LED_ON_INTERVAL :
                             ADVERTISING_SLOW_LED_ON_INTERVAL;
            }
            m_stable_state = indicate;
            err_code       = app_timer_start(m_bsp_leds_tmr, APP_TIMER_TICKS(next_delay), NULL);
            break;

        case BSP_INDICATE_ADVERTISING_DIRECTED:
            // in advertising very quickly blink LED_0
            if (bsp_board_led_state_get(BSP_LED_INDICATE_ADVERTISING_DIRECTED))
            {
                bsp_board_led_off(BSP_LED_INDICATE_ADVERTISING_DIRECTED);
                next_delay = indicate ==
                             BSP_INDICATE_ADVERTISING_DIRECTED ?
                             ADVERTISING_DIRECTED_LED_OFF_INTERVAL :
                             ADVERTISING_SLOW_LED_OFF_INTERVAL;
            }
            else
            {
                bsp_board_led_on(BSP_LED_INDICATE_ADVERTISING_DIRECTED);
                next_delay = indicate ==
                             BSP_INDICATE_ADVERTISING_DIRECTED ?
                             ADVERTISING_DIRECTED_LED_ON_INTERVAL :
                             ADVERTISING_SLOW_LED_ON_INTERVAL;
            }
            m_stable_state = indicate;
            err_code       = app_timer_start(m_bsp_leds_tmr, APP_TIMER_TICKS(next_delay), NULL);
            break;

        case BSP_INDICATE_BONDING:
            // in bonding fast blink LED_0
            bsp_board_led_invert(BSP_LED_INDICATE_BONDING);

            m_stable_state = indicate;
            err_code       =
                app_timer_start(m_bsp_leds_tmr, APP_TIMER_TICKS(BONDING_INTERVAL), NULL);
            break;

        case BSP_INDICATE_CONNECTED:
            bsp_board_led_on(BSP_LED_INDICATE_CONNECTED);
            m_stable_state = indicate;
            break;

        case BSP_INDICATE_SENT_OK:
            // when sending shortly invert LED_1
            m_leds_clear = true;
            bsp_board_led_invert(BSP_LED_INDICATE_SENT_OK);
            err_code = app_timer_start(m_bsp_leds_tmr, APP_TIMER_TICKS(SENT_OK_INTERVAL), NULL);
            break;

        case BSP_INDICATE_SEND_ERROR:
            // on receving error invert LED_1 for long time
            m_leds_clear = true;
            bsp_board_led_invert(BSP_LED_INDICATE_SEND_ERROR);
            err_code = app_timer_start(m_bsp_leds_tmr, APP_TIMER_TICKS(SEND_ERROR_INTERVAL), NULL);
            break;

        case BSP_INDICATE_RCV_OK:
            // when receving shortly invert LED_1
            m_leds_clear = true;
            bsp_board_led_invert(BSP_LED_INDICATE_RCV_OK);
            err_code = app_timer_start(m_bsp_leds_tmr, APP_TIMER_TICKS(RCV_OK_INTERVAL), NULL);
            break;

        case BSP_INDICATE_RCV_ERROR:
            // on receving error invert LED_1 for long time
            m_leds_clear = true;
            bsp_board_led_invert(BSP_LED_INDICATE_RCV_ERROR);
            err_code = app_timer_start(m_bsp_leds_tmr, APP_TIMER_TICKS(RCV_ERROR_INTERVAL), NULL);
            break;

        case BSP_INDICATE_FATAL_ERROR:
            // on fatal error turn on all leds
            bsp_board_leds_on();
            m_stable_state = indicate;
            break;

        case BSP_INDICATE_ALERT_0:
        case BSP_INDICATE_ALERT_1:
        case BSP_INDICATE_ALERT_2:
        case BSP_INDICATE_ALERT_3:
        case BSP_INDICATE_ALERT_OFF:
            err_code   = app_timer_stop(m_bsp_alert_tmr);
            next_delay = (uint32_t)BSP_INDICATE_ALERT_OFF - (uint32_t)indicate;

            // a little trick to find out that if it did not fall through ALERT_OFF
            if (next_delay && (err_code == XINC_SUCCESS))
            {
                if (next_delay > 1)
                {
                    err_code = app_timer_start(m_bsp_alert_tmr,
                                               APP_TIMER_TICKS(((uint16_t)next_delay * ALERT_INTERVAL)),
                                               NULL);
                }
                bsp_board_led_on(BSP_LED_ALERT);
                m_alert_on = true;
            }
            else
            {
                bsp_board_led_off(BSP_LED_ALERT);
                m_alert_on = false;

            }
            break;

        case BSP_INDICATE_USER_STATE_OFF:
            leds_off();
            m_stable_state = indicate;
            break;

        case BSP_INDICATE_USER_STATE_0:
            leds_off();
            bsp_board_led_on(BSP_LED_INDICATE_USER_LED1);
            m_stable_state = indicate;
            break;

        case BSP_INDICATE_USER_STATE_1:
            leds_off();
            bsp_board_led_on(BSP_LED_INDICATE_USER_LED2);
            m_stable_state = indicate;
            break;

        case BSP_INDICATE_USER_STATE_2:
            leds_off();
            bsp_board_led_on(BSP_LED_INDICATE_USER_LED1);
            bsp_board_led_on(BSP_LED_INDICATE_USER_LED2);
            m_stable_state = indicate;
            break;

        case BSP_INDICATE_USER_STATE_3:

        case BSP_INDICATE_USER_STATE_ON:
            bsp_board_leds_on();
            m_stable_state = indicate;
            break;

        default:
            break;
    }

    return err_code;
}


/**@brief Handle events from leds timer.
 *
 * @note Timer handler does not support returning an error code.
 * Errors from bsp_led_indication() are not propagated.
 *
 * @param[in]   p_context   parameter registered in timer start function.
 */
static void leds_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

    if (m_indication_type & BSP_INIT_LEDS)
    {
        UNUSED_VARIABLE(bsp_led_indication(m_stable_state));
    }
}


/**@brief Handle events from alert timer.
 *
 * @param[in]   p_context   parameter registered in timer start function.
 */
static void alert_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    bsp_board_led_invert(BSP_LED_ALERT);
}
#endif // #if LEDS_NUMBER > 0 && !(defined BSP_SIMPLE)


/**@brief Configure indicators to required state.
 */
uint32_t bsp_indication_set(bsp_indication_t indicate)
{
    uint32_t err_code = XINC_SUCCESS;

#if LEDS_NUMBER > 0 && !(defined BSP_SIMPLE)

    if (m_indication_type & BSP_INIT_LEDS)
    {
        err_code = bsp_led_indication(indicate);
    }

#endif // LEDS_NUMBER > 0 && !(defined BSP_SIMPLE)
    return err_code;
}

#if (KBS_ROW_BUTTONS_NUMBER > 0) && (KBS_COL_BUTTONS_NUMBER > 0)
#include "xincx_kbs.h"
static bsp_event_callback_t   m_mtxkey_registered_callback         = NULL;

//bsp_button_event_cfg_t
static kbs_mtxkey_event_cfg_t m_mtxkey_events_list[MAX_MTXKEY_NUMBER] = {{BSP_EVENT_NOTHING, BSP_EVENT_NOTHING}};


static void bsp_mtxkey_event_handler(int16_t mtxkey_idx,uint16_t key_val,uint8_t row_pin,uint8_t col_pin,uint8_t button_action);

static xincx_kbs_mtxkey_cfg_t mtxkeys[] =
{
  [2] = {12,KBS_ROW_BUTTON_2, KBS_COL_BUTTON_1, bsp_mtxkey_event_handler},//BUTTON_PULLDOWN
  [1] = {10,KBS_ROW_BUTTON_1, KBS_COL_BUTTON_1, bsp_mtxkey_event_handler},
  [3] = {11,KBS_ROW_BUTTON_1, KBS_COL_BUTTON_2, bsp_mtxkey_event_handler},

  
  [0] = {13,KBS_ROW_BUTTON_2, KBS_COL_BUTTON_2, bsp_mtxkey_event_handler},

};
            
static void bsp_mtxkey_event_handler(int16_t mtxkey_idx,uint16_t key_val,uint8_t row_pin,uint8_t col_pin,uint8_t button_action)
{
  //  printf("%s\n,mtxkey_idx:%d,key_val:%d,row_pin:%d,col_pin:%d,button_action:%d\r\n",__func__,mtxkey_idx,key_val,row_pin,col_pin,button_action);

    bsp_event_t        event  = BSP_EVENT_NOTHING;
    switch (button_action)
    {
        case BSP_BUTTON_ACTION_PUSH:
        {     
            event = (bsp_event_t)m_mtxkey_events_list[mtxkey_idx].push_event;
        }break;

        case BSP_BUTTON_ACTION_RELEASE:
        {     
            event = (bsp_event_t)m_mtxkey_events_list[mtxkey_idx].release_event;
        }break;

        case BSP_BUTTON_ACTION_LONG_PUSH:
        {     
            event = (bsp_event_t)m_mtxkey_events_list[mtxkey_idx].long_push_event;
        }break;

    }
            
    if ((event != BSP_EVENT_NOTHING) && (m_mtxkey_registered_callback != NULL))
    {
        m_mtxkey_registered_callback(event);
    }
}




#endif

uint32_t bsp_init(uint32_t type, bsp_event_callback_t callback)
{
    uint32_t err_code = XINC_SUCCESS;

#if LEDS_NUMBER > 0 && !(defined BSP_SIMPLE)
    m_indication_type     = type;
#endif // LEDS_NUMBER > 0 && !(defined BSP_SIMPLE)

#if (BUTTONS_NUMBER > 0) && !(defined BSP_SIMPLE)
    m_registered_callback = callback;

    // BSP will support buttons and generate events
    if (type & BSP_INIT_BUTTONS)
    {
//        uint32_t num;

//        err_code = bsp_event_to_button_action_assign(0, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_LED1_ON ));
// 
//        err_code = bsp_event_to_button_action_assign(0, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED1_OFF));
//        
//        err_code = bsp_event_to_button_action_assign(1, BSP_BUTTON_ACTION_LONG_PUSH, (bsp_event_t)(BSP_EVENT_LED2_ON ));
// 
//        err_code = bsp_event_to_button_action_assign(1, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED2_OFF));
//        

        if (err_code == XINC_SUCCESS)
        {
            err_code = app_button_init((app_button_cfg_t *)app_buttons,
                                       BUTTONS_NUMBER,
                                       APP_TIMER_TICKS(10));
        }

        if (err_code == XINC_SUCCESS)
        {
            err_code = app_button_enable();
        }
#ifdef BSP_BUTTON_0
        if (err_code == XINC_SUCCESS)
        {
            err_code = app_timer_create(&m_bsp_button_tmr,
                                        APP_TIMER_MODE_SINGLE_SHOT,
                                        button_timer_handler);
        }
#endif
				
#ifdef BSP_BUTTON_1
        if (err_code == XINC_SUCCESS)
        {
            err_code = app_timer_create(&m_bsp_button_tmr1,
                                        APP_TIMER_MODE_SINGLE_SHOT,
                                        button_timer_handler);
        }
#endif
				
#ifdef BSP_BUTTON_2
        if (err_code == XINC_SUCCESS)
        {
            err_code = app_timer_create(&m_bsp_button_tmr2,
            APP_TIMER_MODE_SINGLE_SHOT,
            button_timer_handler);
        }
#endif
				
#ifdef BSP_BUTTON_3
        if (err_code == XINC_SUCCESS)
        {
            err_code = app_timer_create(&m_bsp_button_tmr3,
                                        APP_TIMER_MODE_SINGLE_SHOT,
                                        button_timer_handler);
        }
#endif
    }
#elif (BUTTONS_NUMBER > 0) && (defined BSP_SIMPLE)
    bsp_board_init(type);
#endif // (BUTTONS_NUMBER > 0) && !(defined BSP_SIMPLE)
    
#if KBS_ROW_BUTTONS_NUMBER && KBS_COL_BUTTONS_NUMBER
    if (type & BSP_INIT_MTXKEY)
    {
        m_mtxkey_registered_callback = callback;
        
        //err_code = xincx_kbs_init(mtxkeys, ARRAY_SIZE(mtxkeys), NULL);    
    }
#endif

#if LEDS_NUMBER > 0 && !(defined BSP_SIMPLE)
    if (type & BSP_INIT_LEDS)
    {
        //handle LEDs only. Buttons are already handled.
        err_code = bsp_board_init(BSP_INIT_LEDS);

        // timers module must be already initialized!
        if (err_code == XINC_SUCCESS)
        {
            err_code =
                app_timer_create(&m_bsp_leds_tmr, APP_TIMER_MODE_SINGLE_SHOT, leds_timer_handler);
        }

        if (err_code == XINC_SUCCESS)
        {
            err_code =
                    app_timer_create(&m_bsp_alert_tmr, APP_TIMER_MODE_REPEATED, alert_timer_handler);
        }
    }
#endif // LEDS_NUMBER > 0 && !(defined BSP_SIMPLE)

    return err_code;
}


#ifndef BSP_SIMPLE
/**@brief Assign specific event to button.
 */
uint32_t bsp_event_to_button_action_assign(uint32_t button, bsp_button_action_t action, bsp_event_t event)
{
    uint32_t err_code = XINC_SUCCESS;

#if BUTTONS_NUMBER > 0
    if (button < BUTTONS_NUMBER)
    {
        if (event == BSP_EVENT_DEFAULT)
        {
            // Setting default action: BSP_EVENT_KEY_x for PUSH actions, BSP_EVENT_NOTHING for RELEASE and LONG_PUSH actions.
            event = (action == BSP_BUTTON_ACTION_PUSH) ? (bsp_event_t)(BSP_EVENT_KEY_0 + button) : BSP_EVENT_NOTHING;
        }
        switch (action)
        {
            case BSP_BUTTON_ACTION_PUSH:
                m_events_list[button].push_event = event;
                break;
            case BSP_BUTTON_ACTION_LONG_PUSH:
                m_events_list[button].long_push_event = event;
                break;
            case BSP_BUTTON_ACTION_RELEASE:
                m_events_list[button].release_event = event;
                break;
            default:
                err_code = XINC_ERROR_INVALID_PARAM;
                break;
        }
    }
    else
    {
        err_code = XINC_ERROR_INVALID_PARAM;
    }
#else
    err_code = XINC_ERROR_INVALID_PARAM;
#endif // BUTTONS_NUMBER > 0

    return err_code;
}

/**@brief Assign specific event to mtxkey.
 */
uint32_t bsp_event_to_mtxkey_action_assign(uint32_t mtxkey_idx, bsp_button_action_t action, bsp_event_t event)
{
    uint32_t err_code = XINC_SUCCESS;

#if KBS_ROW_BUTTONS_NUMBER > 0 && KBS_COL_BUTTONS_NUMBER > 0
    if (mtxkey_idx < (KBS_ROW_BUTTONS_NUMBER * KBS_COL_BUTTONS_NUMBER))
    {
        if (event == BSP_EVENT_DEFAULT)
        {
            // Setting default action: BSP_EVENT_KEY_x for PUSH actions, BSP_EVENT_NOTHING for RELEASE and LONG_PUSH actions.
            event = (action == BSP_BUTTON_ACTION_PUSH) ? (bsp_event_t)(BSP_EVENT_KEY_0 + mtxkey_idx) : BSP_EVENT_NOTHING;
        }
        switch (action)
        {
            case BSP_BUTTON_ACTION_PUSH:
                m_mtxkey_events_list[mtxkey_idx].push_event = event;
                break;
            case BSP_BUTTON_ACTION_LONG_PUSH:
                m_mtxkey_events_list[mtxkey_idx].long_push_event = event;
                break;
            case BSP_BUTTON_ACTION_RELEASE:
                m_mtxkey_events_list[mtxkey_idx].release_event = event;
                break;
            default:
                err_code = XINC_ERROR_INVALID_PARAM;
                break;
        }
    }
    else
    {
        err_code = XINC_ERROR_INVALID_PARAM;
    }
#else
    err_code = XINC_ERROR_INVALID_PARAM;
#endif // BUTTONS_NUMBER > 0

    return err_code;
}


#endif // BSP_SIMPLE


uint32_t bsp_buttons_enable()
{
#if (BUTTONS_NUMBER > 0) && !defined(BSP_SIMPLE)
    return app_button_enable();
#else
    return XINC_ERROR_NOT_SUPPORTED;
#endif
}

uint32_t bsp_buttons_disable()
{
#if (BUTTONS_NUMBER > 0) && !defined(BSP_SIMPLE)
    return app_button_disable();
#else
    return XINC_ERROR_NOT_SUPPORTED;
#endif
}
static uint32_t wakeup_button_cfg(uint32_t button_idx, bool enable)
{
#if !defined(BSP_SIMPLE)
    if (button_idx <  BUTTONS_NUMBER)
    {

        return XINC_SUCCESS;
    }
#else
    UNUSED_PARAMETER(button_idx);
    UNUSED_PARAMETER(enable);
#endif
    return XINC_ERROR_NOT_SUPPORTED;

}
uint32_t bsp_wakeup_button_enable(uint32_t button_idx)
{
    return wakeup_button_cfg(button_idx, true);
}

uint32_t bsp_wakeup_button_disable(uint32_t button_idx)
{
    return wakeup_button_cfg(button_idx, false);
}
