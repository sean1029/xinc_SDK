/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "app_error.h"

#include "xinc_log.h"
//#include "xinc_log_ctrl.h"
#include "app_util_platform.h"
#include "xinc_strerror.h"

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

#endif


#ifndef DEBUG
  // XINC_LOG_WARNING("System reset");
//    NVIC_SystemReset();
#else
    app_error_save_and_stop(id, pc, info);
#endif // DEBUG
}
/*lint -restore */
