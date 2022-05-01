/***********************************************************************
 *
 * MODULE NAME:    sys_hal_config.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Customer specific configuration
 * MAINTAINER:     John Nelson
 * DATE:           15 June 2001
 *
 * SOURCE CONTROL: $Id: sys_hal_config.c,v 1.8 2011/01/19 17:35:32 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    15 June 2001 -   JN       - first version extracted from sys_config
 *
 *
 * This module is a customer specific module per hal that allows the 
 * customer to configure the stack through the sys_config interface.
 *
 * The customer can:
 * 1. Initialise the BD_ADDR
 * 2. Initialise the device class
 * 2. Override the default features supported
 * 3. Override the ACL/SCO buffer allocations
 * 4. Override default major hardware register initialisations
 * 5. Override the default unit key
 * 6. Override the default random number seed.
 ***********************************************************************/

#include "sys_config.h"
#include "sys_hal_features.h"
extern t_SYS_Config g_sys_config;


extern u_int8 bd_addr[6] ;
/*****************************************************************************
 * SYShal_config_Initialise
 *
 * Setup/override the major system configuration structure g_sys_config.
 *
 * Customer modifiable.
 *
 *****************************************************************************/
void SYShal_config_Initialise(void)
{
    int i = 0;

    for(i = 0; i < 6; i++)
    {
        g_sys_config.local_device_address.bytes[i] = bd_addr[i];
    }
}

