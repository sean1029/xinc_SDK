/*
 * MODULE NAME:    bt_addr_sync.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    ADDR_Syncword Generation
 * MAINTAINER:     John Nelson
 * DATE:           1 May 1999
 *
 * SOURCE CONTROL: $Id: bt_addr_sync.c,v 1.9 2004/07/07 14:20:55 namarad Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    V1.0     1 June 1999 -   jn       - Initial Version V0.9
 *
 * SOURCE: Based on V1_4 Design
 * ISSUES:
 *    Reimplement as macros.
 * NOTES TO USERS:
 *    
 */

#include "sys_config.h"

#include "sys_types.h"
#include "hw_register.h"
#include "bt_addr_sync.h"
#include "patch_function_id.h"


/*
 * BTaddr_Build_Sync_Word()
 * In:   bd_addr:lap  24bit
 * Out:  SyncWord     64bit  high and low 32 bit words
 *                    b0  of SyncWord corresponds to b0  of *pSyncLo
 *                    b63 of SyncWord corresponds to b31 of *pSyncHi
 *
 * Source: V0.8 p123/127
 */
void BTaddr_Build_Sync_Word(const t_lap a_lap, u_int32* pSyncHi, u_int32* pSyncLo)
{
	
	if(PATCH_FUN[BTADDR_BUILD_SYNC_WORD_ID]){
         ((void (*)(const t_lap a_lap, u_int32* pSyncHi, u_int32* pSyncLo))PATCH_FUN[BTADDR_BUILD_SYNC_WORD_ID])(a_lap, pSyncHi, pSyncLo);
         return ;
    }

   u_int32 g_BCHgeneratorHi = ((u_int32) SYNC_BCHgenHi), /*Faster if in register     */
           g_BCHgeneratorLo = ((u_int32) SYNC_BCHgenLo);
   const u_int32 p_PNsequenceHi = ((u_int32) SYNC_PNseqHi),
                 p_PNsequenceLo = ((u_int32) SYNC_PNseqLo);
   const u_int32 B_BitsA23_0 = 0x2C000000U, B_BitsA23_1 = 0x13000000U;

   u_int32 x_infoBits;
   u_int32 x_encodeBits;
   u_int32 regHi, regLo;
   u_int32 count;

/* V0.8 p126                                                                */
/* Step 1   Format the 30 information bits to encode  LAP + Barker Sequence */
/*          x(D) = a(D) +D^24 B_A23(D)                                      */

   if (a_lap&BIT23)                       /* Toggle bits 5:0                */
      x_infoBits = a_lap | B_BitsA23_1;
   else
      x_infoBits = a_lap | B_BitsA23_0;

/* Step 2   Add (Exor) the information covering part of the PN sequence     */
/*          x~(D) = x(D) + p34 + p35D + ... + p63D^29                       */

   x_encodeBits =  (x_infoBits ^ (p_PNsequenceHi>>2)) << 2;

/* Step 3   Generate parity bits of BCH(64,30)                              */
/*          c~(D) = D^34 x~(D) mod g(D)                                     */

/* Initialise                                                               */
   regHi = x_encodeBits;
   regLo = 0;

/* Generate remainder      64 bit dividend, 35 bit divisor                  */
   count = 0;
   while (++count <= 64 - 35 + 1)      /* Reduce/Shift the remaining 30 bits*/
      {
      if (regHi&BIT31)                 /* Reduce                            */
         {
         regHi ^= g_BCHgeneratorHi;
         regLo ^= g_BCHgeneratorLo;
         }
      regHi = (regHi<<1) + (regLo>>31);/*Shift full register                */
      regLo <<= 1;
      }

/* Step 4   Create the BCH codeword                                         */
/*          s~(D) = D^34 x~(D) + c~(D)    Note: Exor Addition               */
/* Step 5   Add the PN Sequence                                             */
/*          s(D) = s(D) + p(D)            Note: Exor Addition               */

   *pSyncLo = ((regHi<<2)  ^ (regLo>>30))    ^ p_PNsequenceLo;
   *pSyncHi = ((regHi>>30) ^ (x_encodeBits)) ^ p_PNsequenceHi;

/* Hardware ? */
/* Step 6   Prepend and append the (DC-free preamble and trailer            */
/*          y(D = F_c0(D) + D^4 s(D) + D^68 F_a23(D)                        */

}
