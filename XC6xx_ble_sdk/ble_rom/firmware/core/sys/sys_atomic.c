/**************************************************************************
 * MODULE NAME:    sys_atomic.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Atomic operations
 * MAINTAINER:     John Sheehy
 * DATE:           
 *
 * SOURCE CONTROL: $Id: sys_atomic.c,v 1.3 2004/07/07 14:22:24 namarad Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 * 
 * 
 * 
 * SOURCE: 
 * ISSUES:
 **************************************************************************/

#include "sys_atomic.h"

#ifndef __USE_INLINES__
#include "sys_atomic_impl.h"
#else

/* Empty C files are illegal in ANSI C */
void __sys_atomic_dummy(void) {}

#endif
