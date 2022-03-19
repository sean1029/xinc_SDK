/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_IRQS_H__
#define XINCX_IRQS_H__

#if defined(XINC51)
    #include <soc/xincx_irqs_nrf51.h>
#elif defined(XINC52810_XXAA)
    #include <soc/xincx_irqs_nrf52810.h>
#elif defined(XINC52811_XXAA)
    #include <soc/xincx_irqs_nrf52811.h>
#elif defined(XINC52820_XXAA)
    #include <soc/xincx_irqs_nrf52820.h>
#elif defined(XINC52832_XXAA) || defined (XINC52832_XXAB)
    #include <soc/xincx_irqs_nrf52832.h>
#elif defined(XINC52833_XXAA)
    #include <soc/xincx_irqs_nrf52833.h>
#elif defined(XINC52840_XXAA)
    #include <soc/xincx_irqs_nrf52840.h>
#elif defined(XINC9160_XXAA)
    #include <soc/xincx_irqs_nrf9160.h>
#else
    #error "Unknown device."
#endif

#endif // XINCX_IRQS_H__
