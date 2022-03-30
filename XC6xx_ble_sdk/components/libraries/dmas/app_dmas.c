/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "sdk_common.h"
#if XINC_MODULE_ENABLED(APP_DMAS)
#include "app_dmas.h"
#include "xinc_drv_dmas.h"

static xinc_drv_dmas_t m_dmas = XINC_DRV_DMAS_INSTANCE(0);

static xinc_drv_dmas_config_t config;
uint32_t app_dmas_init(void)
{
    ret_code_t ret_code = XINC_SUCCESS;
    ret_code = xinc_drv_dmas_init(&m_dmas,&config,NULL,NULL);
    
    return ret_code;
}


uint32_t app_dmas_ch_handler_register(uint8_t ch,xinc_drv_dmas_ch_evt_handler_t handler)
{
    ret_code_t ret_code = XINC_SUCCESS;

    xinc_drv_dmas_ch_handler_register(&m_dmas,ch,handler);
    
    return ret_code;
}



#endif //

