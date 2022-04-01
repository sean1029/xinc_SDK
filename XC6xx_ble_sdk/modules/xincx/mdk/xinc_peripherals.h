/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_PERIPHERALS_H__
#define XINC_PERIPHERALS_H__

/*lint ++flb "Enter library region */

#if defined(XINC6206_XXAA)
 #include "xinc620610_peripherals.h"
#elif defined (XINC628_XXAA)
  #include "xinc628_XXXX_peripherals.h"
#else
    #error "Device must be defined. See xinc.h."
#endif

/*lint --flb "Leave library region" */

#endif // XINC_PERIPHERALS_H__
