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

/* Define XINC51_SERIES for common use in nRF51 series devices. Only if not previously defined. */
#if defined (XINC51) ||\
    defined (XINC51422_XXAA) ||\
    defined (XINC51422_XXAB) ||\
    defined (XINC51422_XXAC) ||\
    defined (XINC51801_XXAB) ||\
    defined (XINC51802_XXAA) ||\
    defined (XINC51822_XXAA) ||\
    defined (XINC51822_XXAB) ||\
    defined (XINC51822_XXAC) ||\
    defined (XINC51824_XXAA)
    #ifndef XINC51_SERIES
        #define XINC51_SERIES
    #endif
    #ifndef XINC51
        #define XINC51
    #endif
#endif

/* Redefine "old" too-generic name XINC52 to XINC52832_XXAA to keep backwards compatibility. */
#if defined (XINC52)
    #ifndef XINC52832_XXAA
        #define XINC52832_XXAA
    #endif
#endif

/* Define XINC52_SERIES for common use in nRF52 series devices. Only if not previously defined. */
#if defined (XINC52805_XXAA) || defined (XINC52810_XXAA) || defined (XINC52811_XXAA) || defined (XINC52820_XXAA) || defined (XINC52832_XXAA) || defined (XINC52832_XXAB) || defined (XINC52833_XXAA) || defined (XINC52840_XXAA)
    #ifndef XINC52_SERIES
        #define XINC52_SERIES
    #endif
#endif

/* Define XINC53_SERIES for common use in nRF53 series devices. */
#if defined (XINC5340_XXAA_APPLICATION) || defined (XINC5340_XXAA_NETWORK)
    #ifndef XINC53_SERIES
        #define XINC53_SERIES
    #endif
#endif

/* Define XINC91_SERIES for common use in nRF91 series devices. */
#if defined (XINC9160_XXAA)
    #ifndef XINC91_SERIES    
        #define XINC91_SERIES
    #endif
#endif
   
/* Define coprocessor domains */
#if defined (XINC5340_XXAA_APPLICATION) || defined (XINC5340_XXAA_NETWORK)
    #ifndef XINC5340_XXAA
        #define XINC5340_XXAA
    #endif
#endif
#if defined (XINC5340_XXAA_APPLICATION)
    #ifndef XINC_APPLICATION
        #define XINC_APPLICATION
    #endif
#endif
#if defined (XINC5340_XXAA_NETWORK)
    #ifndef XINC_NETWORK
        #define XINC_NETWORK
    #endif
#endif

/* Apply compatibility macros for old nRF5340 macros */
#if defined(XINC5340_XXAA)
    #if defined (XINC_APPLICATION)
        #ifndef XINC5340_XXAA_APPLICATION
            #define XINC5340_XXAA_APPLICATION
        #endif
    #endif
    #if defined (XINC_NETWORK)
        #ifndef XINC5340_XXAA_NETWORK
            #define XINC5340_XXAA_NETWORK
        #endif
    #endif
#endif

/* Device selection for device includes. */
#if defined (XINC51)
    #include "xinc51.h"
    #include "xinc51_bitfields.h"
    #include "xinc51_deprecated.h"

#elif defined (XINC52805_XXAA)
    #include "xinc52805.h"
    #include "xinc52805_bitfields.h"
    #include "xinc51_to_xinc52810.h"
    #include "xinc52_to_xinc52810.h"
    #include "xinc52810_to_xinc52811.h"
#elif defined (XINC52810_XXAA)
    #include "xinc52810.h"
    #include "xinc52810_bitfields.h"
    #include "xinc51_to_xinc52810.h"
    #include "xinc52_to_xinc52810.h"
    #include "xinc52810_name_change.h"
#elif defined (XINC52811_XXAA)
    #include "xinc52811.h"
    #include "xinc52811_bitfields.h"
    #include "xinc51_to_xinc52810.h"
    #include "xinc52_to_xinc52810.h"
    #include "xinc52810_to_xinc52811.h"
#elif defined (XINC52820_XXAA)
    #include "xinc52820.h"
    #include "xinc52820_bitfields.h"
    #include "xinc51_to_xinc52.h"
    #include "xinc52_to_xinc52833.h"
    #include "xinc52833_to_xinc52820.h"
#elif defined (XINC52832_XXAA) || defined (XINC52832_XXAB)
    #include "xinc52.h"
    #include "xinc52_bitfields.h"
    #include "xinc51_to_xinc52.h"
    #include "xinc52_name_change.h"
#elif defined (XINC52833_XXAA)
    #include "xinc52833.h"
    #include "xinc52833_bitfields.h"
    #include "xinc52_to_xinc52833.h"
    #include "xinc51_to_xinc52.h"
#elif defined (XINC52840_XXAA)
    #include "XC620610.h"
    #include "xinc_52840_bitfields.h"
//    #include "xinc51_to_xinc52840.h"
//    #include "xinc52_to_xinc52840.h"

#elif defined (XINC5340_XXAA)
    #if defined(XINC_APPLICATION)
        #include "xinc5340_application.h"
        #include "xinc5340_application_bitfields.h"
    #elif defined (XINC_NETWORK)
        #include "xinc5340_network.h"
        #include "xinc5340_network_bitfields.h"
    #endif

#elif defined (XINC9160_XXAA)
    #include "xinc9160.h"
    #include "xinc9160_bitfields.h"
    #include "xinc9160_name_change.h"

#else
    ;//#error "Device must be defined. See xinc.h."
#endif /* XINC51, XINC52805_XXAA, XINC52810_XXAA, XINC52811_XXAA, XINC52820_XXAA, XINC52832_XXAA, XINC52832_XXAB, XINC52833_XXAA, XINC52840_XXAA, XINC5340_XXAA_APPLICATION, XINC5340_XXAA_NETWORK, XINC9160_XXAA */

#include "compiler_abstraction.h"

#endif /* XINC_H */

