/*
 * MODULE NAME:    hw_lc_impl.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Implementation for HW accessor Inlines when inline is turned off
 * MAINTAINER:     Ivan Griffin
 * DATE:           20 July 1999
 *
 * SOURCE CONTROL: $Id: hw_le_lc_impl.c,v 1.1 2012/09/28 11:43:29 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    V1.0     20 July 1999 -   IG       - 
 *
 * SOURCE: 
 * ISSUES:
 * NOTES TO USERS:
 *    
 */

#include "sys_config.h"

#if 1// ndef __USE_INLINES__

#include "hw_le_lc.h"
#include "hw_le_lc_impl.h"

#else

/*
 * Completely empty source files are illegal in ANSI C
 */
void __hw_le_lc_dummy(void)  { }

#endif