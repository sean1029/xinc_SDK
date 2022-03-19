/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef NRF_H
#define NRF_H

/* MDK version */
#define MDK_MAJOR_VERSION   8 
#define MDK_MINOR_VERSION   35 
#define MDK_MICRO_VERSION   0 

/* Define NRF51_SERIES for common use in nRF51 series devices. Only if not previously defined. */
#if defined (NRF51) ||\
    defined (NRF51422_XXAA) ||\
    defined (NRF51422_XXAB) ||\
    defined (NRF51422_XXAC) ||\
    defined (NRF51801_XXAB) ||\
    defined (NRF51802_XXAA) ||\
    defined (NRF51822_XXAA) ||\
    defined (NRF51822_XXAB) ||\
    defined (NRF51822_XXAC) ||\
    defined (NRF51824_XXAA)
    #ifndef NRF51_SERIES
        #define NRF51_SERIES
    #endif
    #ifndef NRF51
        #define NRF51
    #endif
#endif

/* Redefine "old" too-generic name NRF52 to NRF52832_XXAA to keep backwards compatibility. */
#if defined (NRF52)
    #ifndef NRF52832_XXAA
        #define NRF52832_XXAA
    #endif
#endif

/* Define NRF52_SERIES for common use in nRF52 series devices. Only if not previously defined. */
#if defined (NRF52805_XXAA) || defined (NRF52810_XXAA) || defined (NRF52811_XXAA) || defined (NRF52820_XXAA) || defined (NRF52832_XXAA) || defined (NRF52832_XXAB) || defined (NRF52833_XXAA) || defined (NRF52840_XXAA)
    #ifndef NRF52_SERIES
        #define NRF52_SERIES
    #endif
#endif

/* Define NRF53_SERIES for common use in nRF53 series devices. */
#if defined (NRF5340_XXAA_APPLICATION) || defined (NRF5340_XXAA_NETWORK)
    #ifndef NRF53_SERIES
        #define NRF53_SERIES
    #endif
#endif

/* Define NRF91_SERIES for common use in nRF91 series devices. */
#if defined (NRF9160_XXAA)
    #ifndef NRF91_SERIES    
        #define NRF91_SERIES
    #endif
#endif
   
/* Define coprocessor domains */
#if defined (NRF5340_XXAA_APPLICATION) || defined (NRF5340_XXAA_NETWORK)
    #ifndef NRF5340_XXAA
        #define NRF5340_XXAA
    #endif
#endif
#if defined (NRF5340_XXAA_APPLICATION)
    #ifndef NRF_APPLICATION
        #define NRF_APPLICATION
    #endif
#endif
#if defined (NRF5340_XXAA_NETWORK)
    #ifndef NRF_NETWORK
        #define NRF_NETWORK
    #endif
#endif

/* Apply compatibility macros for old nRF5340 macros */
#if defined(NRF5340_XXAA)
    #if defined (NRF_APPLICATION)
        #ifndef NRF5340_XXAA_APPLICATION
            #define NRF5340_XXAA_APPLICATION
        #endif
    #endif
    #if defined (NRF_NETWORK)
        #ifndef NRF5340_XXAA_NETWORK
            #define NRF5340_XXAA_NETWORK
        #endif
    #endif
#endif

/* Device selection for device includes. */
#if defined (NRF51)
    #include "nrf51.h"
    #include "nrf51_bitfields.h"
    #include "nrf51_deprecated.h"

#elif defined (NRF52805_XXAA)
    #include "nrf52805.h"
    #include "nrf52805_bitfields.h"
    #include "nrf51_to_nrf52810.h"
    #include "nrf52_to_nrf52810.h"
    #include "nrf52810_to_nrf52811.h"
#elif defined (NRF52810_XXAA)
    #include "nrf52810.h"
    #include "nrf52810_bitfields.h"
    #include "nrf51_to_nrf52810.h"
    #include "nrf52_to_nrf52810.h"
    #include "nrf52810_name_change.h"
#elif defined (NRF52811_XXAA)
    #include "nrf52811.h"
    #include "nrf52811_bitfields.h"
    #include "nrf51_to_nrf52810.h"
    #include "nrf52_to_nrf52810.h"
    #include "nrf52810_to_nrf52811.h"
#elif defined (NRF52820_XXAA)
    #include "nrf52820.h"
    #include "nrf52820_bitfields.h"
    #include "nrf51_to_nrf52.h"
    #include "nrf52_to_nrf52833.h"
    #include "nrf52833_to_nrf52820.h"
#elif defined (NRF52832_XXAA) || defined (NRF52832_XXAB)
    #include "nrf52.h"
    #include "nrf52_bitfields.h"
    #include "nrf51_to_nrf52.h"
    #include "nrf52_name_change.h"
#elif defined (NRF52833_XXAA)
    #include "nrf52833.h"
    #include "nrf52833_bitfields.h"
    #include "nrf52_to_nrf52833.h"
    #include "nrf51_to_nrf52.h"
#elif defined (NRF52840_XXAA)
    #include "XC620610.h"
    #include "nrf_52840_bitfields.h"
//    #include "nrf51_to_nrf52840.h"
//    #include "nrf52_to_nrf52840.h"

#elif defined (NRF5340_XXAA)
    #if defined(NRF_APPLICATION)
        #include "nrf5340_application.h"
        #include "nrf5340_application_bitfields.h"
    #elif defined (NRF_NETWORK)
        #include "nrf5340_network.h"
        #include "nrf5340_network_bitfields.h"
    #endif

#elif defined (NRF9160_XXAA)
    #include "nrf9160.h"
    #include "nrf9160_bitfields.h"
    #include "nrf9160_name_change.h"

#else
    ;//#error "Device must be defined. See nrf.h."
#endif /* NRF51, NRF52805_XXAA, NRF52810_XXAA, NRF52811_XXAA, NRF52820_XXAA, NRF52832_XXAA, NRF52832_XXAB, NRF52833_XXAA, NRF52840_XXAA, NRF5340_XXAA_APPLICATION, NRF5340_XXAA_NETWORK, NRF9160_XXAA */

#include "compiler_abstraction.h"

#endif /* NRF_H */

