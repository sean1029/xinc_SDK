/**********************************************************************
 *
 * MODULE NAME:    lmp_config.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Link Manager Configuration. 
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:  18 February 2000
 *
 * SOURCE CONTROL: $Id: lmp_config.c,v 1.114 2014/03/11 03:14:00 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 ***********************************************************************/

#include "sys_config.h"

#include "lmp_types.h"
#include "lmp_const.h"
#include "lmp_config.h"
#include "lmp_config_impl.h"
#include "lmp_encode_pdu.h"
#include "lmp_acl_connection.h"
#include "lmp_utils.h"
#include "lmp_config.h"
#include "lmp_timer.h"
#include "lmp_scan.h"
#include "lmp_inquiry.h"
#include "lmp_link_power_control.h"
#include "lc_interface.h"
#include "uslc_chan_ctrl.h"

#include "hc_event_gen.h"
#include "hc_const.h"

#if (PRH_BS_CFG_SYS_LE_DUAL_MODE==1)
#include "le_scan.h"
#include "le_const.h"
#include "le_config.h"
#include "le_link_layer.h"
#endif

#ifdef LMP_DEBUG
#include <stdio.h>
#endif
#include "global_val.h"
//t_LMconfig g_LM_config_info;
//t_role g_LM_test_device_role;
//u_int32 g_LM_device_state = 0;
/***********************************************************************
 *  FUNCTION : LMconfig_LM_Read_Local_Name
 *
 *  DESCRIPTION :
 *  Sets up the command complete event parameters for returning
 *  the UTF string for the local device name.
 *
 *  Note the command complete event will pad the string
 *  if less than 248 with null characters (implicit termination)
 **********************************************************************/
void LMconfig_LM_Read_Local_Name(t_cmd_complete_event* p_event)
{

    g_LM_config_info.device_name.length = sizeof(PRH_BS_CFG_LOCAL_NAME);
    g_LM_config_info.device_name.p_utf_char = (u_int8*) PRH_BS_CFG_LOCAL_NAME;   
    

    p_event->p_u_int8 = g_LM_config_info.device_name.p_utf_char;
    p_event->number = g_LM_config_info.device_name.length;
}


