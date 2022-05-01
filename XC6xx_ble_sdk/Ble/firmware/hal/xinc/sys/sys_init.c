/***********************************************************************
 *
 * MODULE NAME:    sys_init.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    hal specific init code
 * MAINTAINER:     Ivan Griffin
 * CREATION DATE:  21 September 2001
 *
 * SOURCE CONTROL: $Id: 
 *
 * LICENSE:
 *     This source code is copyright (c) 2001-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    21 Sept 2001 - RGB  - initial version
 *
 *    
 ***********************************************************************/

#include "sys_config.h"
#include "sys_types.h"

#include "sys_init.h"

#include "hw_leds.h"

#include "hw_lc.h"
#include "hw_radio.h"

#include "hw_register.h"
#include "hw_hab_defs.h"
#include "hw_habanero.h"
#include "patch_function_id.h"


void SYSinit_Initialise(void)
{
	
	if(PATCH_FUN[SYSINIT_INITIALISE_ID]){
         ((void (*)(void))PATCH_FUN[SYSINIT_INITIALISE_ID])();
         return ;
    }


    /* Setup the Tabasco Clock Divider (40MHz / 5 => 8MHz) */
    {
        mHWreg_Create_Cache_Register(HAB_RF_MUX_CLK_DIV_REG);
        mHWreg_Load_Cache_Register(HAB_RF_MUX_CLK_DIV_REG);
        mHWreg_Assign_Cache_Field(HAB_RF_MUX_CLK_DIV_REG, HAB_CLK_DIVIDER, 5);
        mHWreg_Store_Cache_Register(HAB_RF_MUX_CLK_DIV_REG);
    }
}

