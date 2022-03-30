/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_DMAS_H__
#define XINCX_DMAS_H__

#include <xincx.h>
#include <hal/xinc_dmas.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_dmas DMAS driver
 * @{
 * @ingroup xincx_dmas
 * @brief   DMAS peripheral driver.
 */
/**
 * @brief Structure for the DMAS driver instance.
 */
typedef struct
{
    XINC_DMAS_Type * p_dmas;        ///< Pointer to a structure with DMAS registers.
    XINC_CPR_CTL_Type * p_cpr;  	///< Pointer to a structure with CPR registers.
    uint8_t        drv_inst_idx; ///< Index of the driver instance. For internal use only.
    uint8_t        id; 
} xincx_dmas_t;


/** @brief Macro for creating a DMAS driver instance. */
#define XINCX_DMAS_INSTANCE(Id)                               \
{                                                           \
    .p_dmas        = XINCX_CONCAT_2(XINC_DMAS, Id),             \
    .p_cpr            = XINC_CPR,	                        \
    .drv_inst_idx = XINCX_CONCAT_3(XINCX_DMAS, Id, _INST_IDX), \
    .id = Id 												\
}

#ifndef __XINCX_DOXYGEN__
enum {
#if 1//XINCX_CHECK(XINCX_DMAS_ENABLED)
    XINCX_DMAS0_INST_IDX,
#endif
    XINCX_DMAS_ENABLED_COUNT
};
#endif




/** @brief Structure for the configuration of the DMAS driver instance. */
typedef struct
{
  
} xincx_dmas_config_t;


/** @brief Structure for the configuration of the I2C master driver instance. */
typedef struct
{
    uint8_t  ch;
    uint32_t src_addr;
    uint32_t dst_addr;
    uint32_t ctl0;
    uint32_t ctl1;

} xincx_dmas_ch_set_t;


/** @brief Structure for a DMAS event. */
typedef struct
{
    uint8_t dmas_ch;
} xincx_dmas_evt_t;

/** @brief Structure for a DMAS ch event. */
typedef struct
{
    uint8_t dmas_ch;
    uint32_t ch_ca;
} xincx_dmas_ch_evt_t;

/** @brief DMAS event handler prototype. */
typedef void (* xincx_dmas_evt_handler_t)(xincx_dmas_evt_t const * p_event,
                                        void *                 p_context);

/** @brief DMAS event handler prototype. */
typedef void (* xincx_dmas_ch_evt_handler_t)(xincx_dmas_ch_evt_t const * p_event,
                                        void *                 p_context);

xincx_err_t xincx_dmas_init(xincx_dmas_config_t const * p_config,
                         xincx_dmas_evt_handler_t    event_handler,
                         void *                    p_context);
                         
bool xincx_dmas_is_init(void);
                         
xincx_err_t xincx_dmas_ch_param_set(xincx_dmas_ch_set_t set);

                         
xincx_err_t xincx_dmas_ch_enable(xinc_dma_ch_t ch);

xincx_err_t xincx_dmas_ch_disable(xinc_dma_ch_t ch);

uint32_t xincx_dmas_stat_get(void);

xincx_err_t xincx_dmas_int_enable(uint32_t mask);

xincx_err_t xincx_dmas_int_disable(uint32_t mask);

uint32_t xincx_dmas_int_stat_get(void);

uint32_t xincx_dmas_int_raw_stat_get(void);

xincx_err_t xincx_dmas_int_sta_clear(uint32_t clr_bits);

xincx_err_t xincx_dmas_ch_ca_get(uint8_t ch,uint32_t *ch_ca);

xincx_err_t xincx_dmas_low_power_ctl(uint32_t ctl);
                         
xincx_err_t xincx_dmas_ch_handler_register(uint8_t ch,xincx_dmas_ch_evt_handler_t handler);

#endif //XINCX_DMAS_H__
