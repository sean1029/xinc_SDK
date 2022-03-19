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
    #include "nrf51_peripherals.h"

#elif defined (XINC52805_XXAA)
    #include "nrf52805_peripherals.h"
#elif defined(XINC52810_XXAA)
    #include "nrf52810_peripherals.h"
#elif defined(XINC52811_XXAA)
    #include "nrf52811_peripherals.h"
#elif defined(XINC52820_XXAA)
    #include "nrf52820_peripherals.h"
#elif defined(XINC52832_XXAA) || defined(XINC52832_XXAB)
    #include "nrf52832_peripherals.h"
#elif defined (XINC52833_XXAA)
    #include "nrf52833_peripherals.h"
#elif defined(XINC52840_XXAA)
    #include "nrf52840_peripherals.h"

#elif defined (XINC5340_XXAA_APPLICATION)
    #include "nrf5340_application_peripherals.h"
#elif defined (XINC5340_XXAA_NETWORK)
    #include "nrf5340_network_peripherals.h"

#elif defined(XINC9160_XXAA)
    #include "nrf9160_peripherals.h"

#else
    #error "Device must be defined. See nrf.h."
#endif

/*lint --flb "Leave library region" */

#endif // XINC_PERIPHERALS_H__
