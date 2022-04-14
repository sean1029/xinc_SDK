/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_KBS_H__
#define XINCX_KBS_H__

#include <xincx.h>
#include <hal/xinc_kbs.h>

#ifdef __cplusplus
extern "C" {
#endif


#define FORBIDDEN_HANDLER_ADDRESS ((xincx_kbs_mtxkey_handler_t)UINT32_MAX)
#define PIN_NOT_USED              (-1)
#define PIN_USED                  (-2)
#define PIN_NO_FUN                 (-3)
#define NO_MTXKEY_ID               (-1)
#define NO_MTXKEY_ID               (-1)
#define POLARITY_FIELD_POS        (6)
#define POLARITY_FIELD_MASK       (0xC0)
#define MAX_MTXKEY_NUMBER          (KBS_ROW_PIN_COUNT * KBS_COL_PIN_COUNT)

#define KBS_MTXKEY_PUSH        1                               /**< Indicates that a button is pushed. */
#define KBS_MTXKEY_RELEASE     0                               /**< Indicates that a button is released. */
#define KBS_MTXKEY_LONG_PUSH (2)  

#define KBS_EVENT_NOTHING      0       


                          
typedef uint32_t   kbs_mtxkey_event_t;

typedef struct
{
    kbs_mtxkey_event_t push_event;      /**< The event to fire on regular button press. */
    kbs_mtxkey_event_t long_push_event; /**< The event to fire on long button press. */
    kbs_mtxkey_event_t release_event;   /**< The event to fire on button release. */
} kbs_mtxkey_event_cfg_t;


/**@brief Kbs Button event handler type. */
typedef void (*xincx_kbs_mtxkey_handler_t)(int16_t mtxkey_idx,uint16_t mtxkey_val,uint8_t row_pin,uint8_t col_pin, uint8_t button_action);

/**@brief Button configuration structure. */
typedef struct
{
    uint8_t              key_val;           /**< Pin set value. */
    uint8_t              row_pin;           /**< Pin to be used as a row button. */
    uint8_t              col_pin;           /**< Pin to be used as a col button. */
   // uint8_t              active_state;     /**< APP_BUTTON_ACTIVE_HIGH or APP_BUTTON_ACTIVE_LOW. */
    xincx_kbs_mtxkey_handler_t handler;   /**< Handler to be called when button is pushed. */
} xincx_kbs_mtxkey_cfg_t;

typedef	struct
{
    uint8_t prs_intval;//按键按下检测间隔
    uint8_t rls_intval;//释放按键检测间隔

    uint8_t dbc_intval;//debounce间隔

    uint16_t rprs_intval;//重复按键间隔
    uint8_t lprs_intval;//长按按键间隔
} xincx_kbs_intval_config_t;


/** @brief Macro for setting @ref xincx_kbs_intval_config_t to default settings. */

#define XINCX_KBS_INTVAL_DEFAULT_CONFIG                                 \
{                                                                       \
        .prs_intval = XINCX_KBS_CONFIG_PRS_INTVAL,                      \
        .rls_intval = XINCX_KBS_CONFIG_RLS_INTVAL,                      \
        .dbc_intval = XINCX_KBS_CONFIG_DBC_INTVAL,                      \
        .rprs_intval = XINCX_KBS_CONFIG_RPRS_INTVAL,                    \
        .lprs_intval = XINCX_KBS_CONFIG_LPRS_INTVAL                     \
}



/**@brief Function for initializing the kbs Buttons.
 *
 * @details This function will initialize the specified pins as buttons, and configure the Button
 *          Handler module as a kbs user 
 *
 * @note xincx_kbs_init() function must be called in order to enable the button .
 *
 * @param[in]  p_mtxkeys           Array of buttons to be used (NOTE: Must be static!).
 * @param[in]  mtxkey_count        Number of buttons.
 * @param[in]  detection_delay     Delay from a kbs event until a button is reported as pushed.
 *
 * @return   XINC_SUCCESS on success, otherwise an error code.
 */
xincx_err_t  xincx_kbs_init(xincx_kbs_mtxkey_cfg_t const *       p_mtxkeys,
                         uint8_t                        mtxkey_count,
                         xincx_kbs_intval_config_t  const *       p_config);


#ifdef __cplusplus
}
#endif


#endif // XINCX_KBS_H__
