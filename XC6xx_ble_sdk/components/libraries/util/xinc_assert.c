/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "xinc_assert.h"
#include "app_error.h"
#include "xinchip_common.h"

__WEAK void assert_xinc_callback(uint16_t line_num, const uint8_t * file_name)
{
    assert_info_t assert_info =
    {
        .line_num    = line_num,
        .p_file_name = file_name,
    };
    volatile bool loop = true;
    printf("assert file:%s,line num:%d\r\n",file_name,line_num);
    
    while (loop);
   // app_error_fault_handler(NRF_FAULT_ID_SDK_ASSERT, 0, (uint32_t)(&assert_info));

    UNUSED_VARIABLE(assert_info);
}
