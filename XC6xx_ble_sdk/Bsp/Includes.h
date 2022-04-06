#ifndef 	__INCLUDES_H
#define 	__INCLUDES_H

#include "xinc.h"
#if defined(XC60XX_M0)
  #include    "xinc_m0.h"
#elif defined (XC66XX_M4)
  #include    "xinc_m4.h"
#else
    #error "Device must be defined. See xinc.h."
#endif

#endif

