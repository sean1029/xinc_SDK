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

#if defined (XC6000) || \
    defined (XC6010) || \
    defined (XC6020) || \
    defined (XC6022) || \
    defined (XC6028) || \
    defined (XC6030) || \
    defined (XC003)

    #ifndef XC60XX_M0
        #define XC60XX_M0
    #endif
#endif



#if defined (XC6651) || \
    defined (XC6652) || \
    defined (XC6658) || \
    defined (XC6103)

    #ifndef XC66XX_M4
        #define XC66XX_M4
    #endif
#endif



/* Device selection for device includes. */
#if  defined (XC60XX_M0)
    #include "XC620610.h"
    #include "xinc_620610_bitfields.h"

#elif defined (XC66XX_M4)
    #include "XC628_XXXX.h"
    #include "xinc628_XXXX_bitfields.h"

#else
    #error "Device must be defined. See xinc.h."
#endif /* */

#include "compiler_abstraction.h"

#endif /* XINC_H */

