#ifndef _CEVA_TRA_CODEC_H_
#define _CEVA_TRA_CODEC_H_
/*****************************************************************************
 * MODULE NAME:    tra_codec.h
 * PROJECT CODE:   Bluetooth
 * DESCRIPTION:    Codec FIFO Transport Interface
 * MAINTAINER:     Tom Kerwick
 * CREATION DATE:  27 Feb 2013
 *
 * SOURCE CONTROL: $Id: tra_codec.h,v 1.3 2013/10/08 16:45:32 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2013 Ceva Inc
 *     All rights reserved.
 *
 *****************************************************************************/

#if (PRH_BS_CFG_SYS_ESCO_VIA_VCI_SUPPORTED==1)

#define TRA_CODEC_VCI_TX_FIFO_SIZE 40 /* 40 samples (8-bit or 16-bit) */
#define TRA_CODEC_VCI_RX_FIFO_SIZE 40 /* 40 samples (8-bit or 16-bit) */

void TRAcodec_Initialise(void);
void TRAcodec_Enable(void);
void TRAcodec_Disable(void);
t_error TRAcodec_Dispatch_Pending_SCO(void);

#else

#define TRAcodec_Initialise()
#define TRAcodec_Enable()
#define TRAcodec_Disable()
#define TRAcodec_Dispatch_Pending_SCO()

#endif /* PRH_BS_CFG_SYS_ESCO_VIA_VCI_SUPPORTED */

#endif /* _CEVA_TRA_CODEC_H_ */
