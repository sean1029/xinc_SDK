/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef NRFX_IRQS_H__
#define NRFX_IRQS_H__

#if defined(NRF51)
    #include <soc/nrfx_irqs_nrf51.h>
#elif defined(NRF52810_XXAA)
    #include <soc/nrfx_irqs_nrf52810.h>
#elif defined(NRF52811_XXAA)
    #include <soc/nrfx_irqs_nrf52811.h>
#elif defined(NRF52820_XXAA)
    #include <soc/nrfx_irqs_nrf52820.h>
#elif defined(NRF52832_XXAA) || defined (NRF52832_XXAB)
    #include <soc/nrfx_irqs_nrf52832.h>
#elif defined(NRF52833_XXAA)
    #include <soc/nrfx_irqs_nrf52833.h>
#elif defined(NRF52840_XXAA)
    #include <soc/nrfx_irqs_nrf52840.h>
#elif defined(NRF9160_XXAA)
    #include <soc/nrfx_irqs_nrf9160.h>
#else
    #error "Unknown device."
#endif

#endif // NRFX_IRQS_H__
