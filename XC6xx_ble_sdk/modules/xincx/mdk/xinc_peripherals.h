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

#if defined(XINC51)
    #include "xinc51_peripherals.h"

#elif defined (XINC52805_XXAA)
    #include "xinc52805_peripherals.h"
#elif defined(XINC52810_XXAA)
    #include "xinc52810_peripherals.h"
#elif defined(XINC52811_XXAA)
    #include "xinc52811_peripherals.h"
#elif defined(XINC52820_XXAA)
    #include "xinc52820_peripherals.h"
#elif defined(XINC52832_XXAA) || defined(XINC52832_XXAB)
    #include "xinc52832_peripherals.h"
#elif defined (XINC52833_XXAA)
    #include "xinc52833_peripherals.h"
#elif defined(XINC52840_XXAA)
    #include "xinc52840_peripherals.h"

#elif defined (XINC5340_XXAA_APPLICATION)
    #include "xinc5340_application_peripherals.h"
#elif defined (XINC5340_XXAA_NETWORK)
    #include "xinc5340_network_peripherals.h"

#elif defined(XINC9160_XXAA)
    #include "xinc9160_peripherals.h"

#else
    #error "Device must be defined. See xinc.h."
#endif

/*lint --flb "Leave library region" */

#endif // XINC_PERIPHERALS_H__
