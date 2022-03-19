/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_H
#define XINC_H

/* MDK version */
#define MDK_MAJOR_VERSION   8 
#define MDK_MINOR_VERSION   35 
#define MDK_MICRO_VERSION   0 







   




/* Device selection for device includes. */
#if   defined (XINC6206_XXAA)
    #include "XC620610.h"
    #include "xinc_620610_bitfields.h"

#elif defined (XINC6201_XXAA) || defined (XINC6202_XXAA)


#else
    #error "Device must be defined. See xinc.h."
#endif /* XINC51, XINC52805_XXAA, XINC52810_XXAA, XINC52811_XXAA, XINC52820_XXAA, XINC52832_XXAA, XINC52832_XXAB, XINC52833_XXAA, XINC6206_XXAA, XINC5340_XXAA_APPLICATION, XINC5340_XXAA_NETWORK, XINC9160_XXAA */

#include "compiler_abstraction.h"

#endif /* XINC_H */

