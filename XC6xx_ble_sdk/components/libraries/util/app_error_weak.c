/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "app_error.h"

#include "nrf_log.h"
//#include "nrf_log_ctrl.h"
#include "app_util_platform.h"
#include "nrf_strerror.h"

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
#include "nrf_sdm.h"
#endif

/*lint -save -e14 */
/**
 * Function is implemented as weak so that it can be overwritten by custom application error handler
 * when needed.
 */
__WEAK void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    __disable_irq();
 //   XINC_LOG_FINAL_FLUSH();

#ifndef DEBUG
  //  XINC_LOG_ERROR("Fatal error");
#else
    switch (id)
    {
#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
        case XINC_FAULT_ID_SD_ASSERT:
            XINC_LOG_ERROR("SOFTDEVICE: ASSERTION FAILED");
            break;
        case XINC_FAULT_ID_APP_MEMACC:
            XINC_LOG_ERROR("SOFTDEVICE: INVALID MEMORY ACCESS");
            break;
#endif
        case XINC_FAULT_ID_SDK_ASSERT:
        {
            assert_info_t * p_info = (assert_info_t *)info;
            XINC_LOG_ERROR("ASSERTION FAILED at %s:%u",
                          p_info->p_file_name,
                          p_info->line_num);
            break;
        }
        case XINC_FAULT_ID_SDK_ERROR:
        {
            error_info_t * p_info = (error_info_t *)info;
            XINC_LOG_ERROR("ERROR %u [%s] at %s:%u\r\nPC at: 0x%08x",
                          p_info->err_code,
                          nrf_strerror_get(p_info->err_code),
                          p_info->p_file_name,
                          p_info->line_num,
                          pc);
             XINC_LOG_ERROR("End of error report");
            break;
        }
        default:
            XINC_LOG_ERROR("UNKNOWN FAULT at 0x%08X", pc);
            break;
    }
#endif

 //   XINC_BREAKPOINT_COND;
    // On assert, the system can only recover with a reset.

#ifndef DEBUG
 //   XINC_LOG_WARNING("System reset");
//    NVIC_SystemReset();
#else
    app_error_save_and_stop(id, pc, info);
#endif // DEBUG
}
/*lint -restore */
