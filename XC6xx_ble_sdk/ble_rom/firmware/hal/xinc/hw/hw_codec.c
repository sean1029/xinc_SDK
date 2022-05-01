/*
 * MODULE NAME:    hw_codec.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Code to drive codecs on prth_chimera
 * MAINTAINER:     Daire McNamara
 * DATE:           10 October 2001
 *
 * SOURCE CONTROL: $Id: hw_codec.c,v 1.21 2013/10/25 16:50:15 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2001-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    V0.9     10 October 2001 -   DMN       - Initial Revision
 *
 * SOURCE: 
 * ISSUES:
 * NOTES TO USERS:
 *    
 */
#include "sys_config.h"
#include "hc_const.h"
#include "hw_codec.h"
#include "hw_lc.h"
#include "lmp_sco_container.h"

static u_int16 _HWcodec_voice_setting;

/******************************************************************************
 *
 * FUNCTION: HWcodec_Initialise
 *
 *****************************************************************************/
void HWcodec_Initialise()
{
    _HWcodec_voice_setting =  (
        ( (LINEAR_IC)        << 8) |  
        /*( (CVSD_IC)        << 8) | */
        ( (SIGN_MAG_IDF)     << 6) |
        ( (SIXTEEN_BIT_ISS)  << 5) |
        ( (0)                << 2) |
        ( (CVSD_ACF)         << 0)  
        );
}
/******************************************************************************
 *
 * FUNCTION: HWcodec_Is_Valid_SCO_Conversion
 *
 * DESCRIPTION:
 *  Check if the platform (Tabasco/Codec combination) can
 *  support the proposed voice setting.
 *
 * RETURNS:
 *  0 if cannot support the proposed voice setting, 1 if it can.
 *
 *****************************************************************************/
boolean HWcodec_Is_Valid_SCO_Conversion(u_int16 new_voice_setting)
{
   boolean can_convert = 0; /* FALSE */	
   return can_convert;
}


/******************************************************************************
 *
 * FUNCTION: HWcodec_Get_VCI_Clk_Sel
 *
 * DESCRIPTION:
 *
 ******************************************************************************/
u_int8 HWcodec_Get_VCI_Clk_Sel(void)
{
    return 2;
}


/*******************************************************************************
 *
 * FUNCTION: HWcodec_Get_VCI_Clk_Sel_Map
 *
 * DESCRIPTION:
 *
 ******************************************************************************/
u_int8 HWcodec_Get_VCI_Clk_Sel_Map(void)
{
#if(BUILD_TYPE==UNIT_TEST_BUILD)
    return 0;
#else
    return 1;
#endif
}


/*******************************************************************************
 *
 * FUNCTION: HWcodec_Enable
 *
 * DESCRIPTION:
 *  Do anything special required to enable the codec.
 *
 ******************************************************************************/
void HWcodec_Enable(void)
{
}


/*******************************************************************************
 *
 * FUNCTION: HWcodec_Disable
 *
 * DESCRIPTION:
 *  Do anything special required to disable the codec.
 *
 ******************************************************************************/
void HWcodec_Disable(void)
{
}


/*******************************************************************************
 *
 * FUNCTION: HWcodec_Increase_Volume
 *
 * DESCRIPTION:
 *     Do anything required on the codec interface to increase
 *     the volume from the codec.
 *
 ******************************************************************************/
void HWcodec_Increase_Volume(void)
{
    /* Nothing to do on Chimera */
}


/*******************************************************************************
 *
 * FUNCTION: HWcodec_Decrease_Volume
 *
 * DESCRIPTION:
 *     Do anything required on the codec interface to decrease
 *     the volume from the codec.
 *
 ******************************************************************************/
void HWcodec_Decrease_Volume(void)
{
}

/*
 * Set up chimera's codec to expect either
 * A-law, u-law, or linear PCM.
 */
void HWcodec_Set_Voice_Setting(u_int16 new_voice_setting)
{
	_HWcodec_voice_setting = new_voice_setting;
}

u_int16 HWcodec_Get_Voice_Setting(void)
{
    return _HWcodec_voice_setting;
}
