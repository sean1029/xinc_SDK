#ifndef _PARTHUS_LMP_REMOTE_INFO
#define _PARTHUS_LMP_REMOTE_INFO

/******************************************************************************
 * MODULE NAME:     lmp_remote_info.h
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:   
 * MAINTAINER:      Gary Fleming
 * CREATION DATE:        
 *
 * SOURCE CONTROL: $Id: lmp_remote_info.h,v 1.13 2004/07/07 14:22:19 namarad Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 ******************************************************************************/

#include "sys_types.h"
#include "lmp_acl_connection.h"

void LM_LkInfo_Pr__Initialise(void);

t_error LM_LkInfo_Pr_LM_Read_Remote_Supported_Features(t_lmp_link* p_link);
t_error LM_LkInfo_Pr_LM_Read_Remote_Version(t_lmp_link* p_link);
t_error LM_LkInfo_Pr_LM_Read_Remote_Name(t_pageParams* pageParams);

t_error LM_LkInfo_Pr_LMP_Features_Req(t_lmp_link* p_link,t_p_pdu);
t_error LM_LkInfo_Pr_LMP_Features_Res(t_lmp_link* p_link,t_p_pdu);
t_error LM_LkInfo_Pr_LMP_Version_Req(t_lmp_link* p_link,t_p_pdu);
t_error LM_LkInfo_Pr_LMP_Version_Res(t_lmp_link* p_link,t_p_pdu);
/*t_error LM_LkInfo_Pr_LMP_Name_Req(t_lmp_link* p_link,t_p_pdu);
t_error LM_LkInfo_Pr_LMP_Name_Res(t_lmp_link* p_link,t_p_pdu);*/

#define _LM_REMOTE_SUPORTED_FEATURES_CFM  0
#define _LM_REMOTE_VERSION_CFM   1
#define _LM_REMOTE_NAME_CFM    2

#endif

