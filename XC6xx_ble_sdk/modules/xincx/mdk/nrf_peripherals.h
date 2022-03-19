/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef NRF_PERIPHERALS_H__
#define NRF_PERIPHERALS_H__

/*lint ++flb "Enter library region */

#if defined(NRF51)
    #include "nrf51_peripherals.h"

#elif defined (NRF52805_XXAA)
    #include "nrf52805_peripherals.h"
#elif defined(NRF52810_XXAA)
    #include "nrf52810_peripherals.h"
#elif defined(NRF52811_XXAA)
    #include "nrf52811_peripherals.h"
#elif defined(NRF52820_XXAA)
    #include "nrf52820_peripherals.h"
#elif defined(NRF52832_XXAA) || defined(NRF52832_XXAB)
    #include "nrf52832_peripherals.h"
#elif defined (NRF52833_XXAA)
    #include "nrf52833_peripherals.h"
#elif defined(NRF52840_XXAA)
    #include "nrf52840_peripherals.h"

#elif defined (NRF5340_XXAA_APPLICATION)
    #include "nrf5340_application_peripherals.h"
#elif defined (NRF5340_XXAA_NETWORK)
    #include "nrf5340_network_peripherals.h"

#elif defined(NRF9160_XXAA)
    #include "nrf9160_peripherals.h"

#else
    #error "Device must be defined. See nrf.h."
#endif

/*lint --flb "Leave library region" */

#endif // NRF_PERIPHERALS_H__
