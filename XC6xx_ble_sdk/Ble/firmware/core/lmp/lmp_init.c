/***********************************************************************
 *
 * MODULE NAME:    lmp_init.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:  8 December 1999
 *
 * SOURCE CONTROL: $Id: lmp_init.c,v 1.41 2011/10/28 16:03:39 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    8.Dec.1999 -   GF       - initial version
 *
 * ISSUES:
 *    
 ***********************************************************************/

#include "sys_config.h"

/*
 *  LMP Initialisation 
 *
 */

#include "lmp_acl_connection.h"
#include "lmp_acl_container.h"
#include "lmp_sec_core.h"
#include "lmp_ch.h"
#include "lmp_con_filter.h"
#include "lmp_sco_container.h"
#include "lmp_init.h"
#include "lmp_link_key_db.h"
#include "lmp_link_policy.h"
#include "lmp_scan.h"
#include "lmp_inquiry.h"
#include "lmp_timer.h"
#include "lmp_config.h"
#include "lmp_link_policy_protocol.h"
#include "lmp_afh.h"
#include "lmp_link_qos.h"
#include "lmp_ssp.h"
#include "patch_function_id.h"

/******************************************************************
 *  FUNCTION :- LM_Initialise
 *
 *  DESCRIPTION :-
 *  Initialises all of the LMP components.
 *
 ******************************************************************/
void LM_Initialise(void)
{
	if(PATCH_FUN[LM_INITIALISE_ID]){
         ((void (*)(void))PATCH_FUN[LM_INITIALISE_ID])();
 
         return ;
    }

#ifndef BLUETOOTH_MODE_LE_ONLY
    /*
     * Ensure that all callbacks are reset to defaults
     */
    LC_Reset_All_Callbacks();

    /* 
     * Initialise all the LM Modules 
     */

    LMaclctr_Initialise();       /* Initialise the LM Link container            */

    LMtmr_Initialise();          /* Initialise the LM timers                    */
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY
    LMconfig_Initialise();       /* Initialise the configuration of the LM      */

    LMscan_Initialise();         /* Initialise the LM Scan Object               */

    LMfltr_Initialise();         /* Initialise the Inquiry & Connection filters */
#endif

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    LMscoctr_Initialise();       /* Initialise the SCO Container                */
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY
    LMpol_Initialise();          /* Initialise the LM Link Policy               */
#endif
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
    LMpolicy_Initialise();       /* Initialise the LM Link Policy Protocol      */
#endif

#ifndef BLUETOOTH_MODE_LE_ONLY
    LMkeydb_Initialise();        /* Initialise the Link Key Database            */

#if (PRH_BS_CFG_SYS_COMBINATION_KEY_DEVICE_SUPPORTED==1)
    LMsec_core_Initialise(COMBINATION_KEY_DEVICE); /* Init Device Key Type      */
#else
    LMsec_core_Initialise(UNIT_KEY_DEVICE); /* Init Device Key Type      */
#endif
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY
    LMconnection_Initialise();   /* Initialise the LM Connection Handler        */

    LMinq_Initialise();          /* Initialise the LM Inquiry Object            */
#endif
#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
    LMqos_Initialise();
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY
    LMafh_Initialise();
#endif
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
	{extern boolean epc_enabled;
	epc_enabled = TRUE;}
#endif

}

