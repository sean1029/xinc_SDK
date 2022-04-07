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
   
#elif defined(XC60XX_M0)
    #include <soc/xincx_irqs_xinc6206.h>
#else
    #error "Unknown device."
#endif

#endif // XINCX_IRQS_H__
