/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_DRV_DMAS_H__
#define XINC_DRV_DMAS_H__
#define DMAS_PRESENT

#include <xincx.h>
#ifdef DMAS_PRESENT
    #include <xincx_dmas.h>
    
        #define XINC_DRV_DMAS_CREATE_DMAS(id)   _XINC_DRV_DMAS_CREATE_DMAS(id)
    #define _XINC_DRV_DMAS_CREATE_DMAS(id)  XINC_DRV_DMAS_CREATE_DMAS_##id
    #define XINC_DRV_DMAS_CREATE_DMAS_0  \
        .dmas = XINCX_DMAS_INSTANCE(0),
        
#else
    // Compilers (at least the smart ones) will remove the I2CM related code
    // (blocks starting with "if (XINC_DRV_I2C_USE_I2CM)") when it is not used,
    // but to perform the compilation they need the following definitions.
    #define xincx_dmas_init(...)                 0
    #define xincx_dmas_uninit(...)
    #define xincx_dmas_enable(...)
    #define xincx_dmas_disable(...)
    #define xincx_dmas_int_enable(...)                   0
    #define xincx_dmas_int_disable(...)                   0
#endif



typedef struct
{
    uint8_t inst_idx;
    uint8_t id;
#ifdef DMAS_PRESENT
    xincx_dmas_t dmas;
#endif
} xinc_drv_dmas_t;


/**
 * @brief Macro for creating an DMAS driver instance.
 */
#define XINC_DRV_DMAS_INSTANCE(Id) \
{                                 \
    .inst_idx = Id,               \
	.id = Id,											\
    XINC_DRV_DMAS_CREATE_DMAS(Id)  \
}


typedef struct
{

} xinc_drv_dmas_config_t;


/**
 * @brief Structure for a DMAS event.
 */
typedef struct
{
    uint8_t dma_ch;
} xinc_drv_dmas_evt_t;


/**
 * @brief Structure for a DMAS ch event.
 */
typedef struct
{
    uint8_t dma_ch;
} xinc_drv_dmas_ch_evt_t;

/**
 * @brief DMAS event handler prototype.
 */
typedef void (* xinc_drv_dmas_evt_handler_t)(xinc_drv_dmas_evt_t const * p_event,
                                           void *                    p_context);

typedef void (* xinc_drv_dmas_ch_evt_handler_t)(xinc_drv_dmas_ch_evt_t const * p_event,
                                           void *                    p_context);


ret_code_t xinc_drv_dmas_init(xinc_drv_dmas_t const *        p_instance,
                            xinc_drv_dmas_config_t const * p_config,
                            xinc_drv_dmas_evt_handler_t    event_handler,
                            void *                       p_context);

ret_code_t xinc_drv_dmas_ch_handler_register(xinc_drv_dmas_t const *p_instance ,uint8_t ch,xinc_drv_dmas_ch_evt_handler_t handler);

#endif //XINC_DRV_DMAS_H__
