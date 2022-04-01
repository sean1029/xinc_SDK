#ifndef 	__INCLUDES_H
#define 	__INCLUDES_H

#include "xinc.h"
#if defined(XINC6206_XXAA)
  #include    "xinc_m0.h"
#elif defined (XINC628_XXAA)
  #include    "xinc_m4.h"
#else
    #error "Device must be defined. See xinc.h."
#endif

#endif

