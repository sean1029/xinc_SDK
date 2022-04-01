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

#if defined (XINC628_A) || defined (XINC628_B) ||  defined (XINC628_C) ||  defined (XINC628_D)

#ifndef XINC628_XXAA
        #define XINC628_XXAA
    #endif
#endif


/* Device selection for device includes. */
#if   defined (XINC6206_XXAA)
    #include "XC620610.h"
    #include "xinc_620610_bitfields.h"

#elif defined (XINC628_XXAA)
    #include "XC628_XXXX.h"
    #include "xinc628_XXXX_bitfields.h"

#else
    #error "Device must be defined. See xinc.h."
#endif /* */

#include "compiler_abstraction.h"

#endif /* XINC_H */

