/***********************************************************************
 *
 * MODULE NAME:    sys_crc32.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    
 * MAINTAINER:     Ivan Griffin
 * CREATION DATE:  20 June 2000
 *
 * SOURCE CONTROL: $Id: sys_crc32.c,v 1.11 2004/07/07 14:22:24 namarad Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    20.Jun.2000 -   IG       - CRC32 generation
 *
 * ISSUES:
 *    for more info on CRCs, see
 *        ftp://ftp.rocksoft.com/clients/rocksoft/papers/crc_v3.txt
 *
 *    This algorithm is CRC-32, which is used in PKzip, Ethernet, FDDI etc.
 *    
 ***********************************************************************/

#include "sys_config.h"
#include "sys_types.h"
#include "sys_crc32.h"

static u_int32 _crc_precomputed_table[256];

/*
 * 32-bit generator polynomial is
 *
 * x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 
 *    + x^5 + x^4 + x^2 + x^1 + x^0
 */
#define SYScrc32_POLYNOMIAL   ((u_int32)0x04c11db7)
#define SYScrc32_INITIAL_SEED ((u_int32)0xffffffff)

/*
 * build up table by dividing all bytes by polynomial using 
 * polynomial arithmetic mod 2
 */
void SYScrc32_Generate_Table(void)
{
    u_int32 i, j;
    u_int32 accumulator;

    for (i = 0; i < 256; i++)
    {
        accumulator = (i << 24);

        for (j = 0; j < 8; j++)
        {
            /* test is the top bit set - if so, perform 'subtraction' */
            if (accumulator & ((u_int32)0x80000000))
            {
                accumulator = (accumulator << 1) ^ SYScrc32_POLYNOMIAL;
            }
            else
            {
                accumulator = (accumulator << 1);
            }
        }

        _crc_precomputed_table[i] = accumulator;
    }
}

/*
 * IMPORTANT - first accumulator seed must be 0xFFFFFFFF
 */
u_int32 SYScrc32_Update_CRC(char *data_blk_ptr, u_int32 data_blk_size)
{
    /*
     * algorithm is:
     *
     * while (!exhausted(augmented_msg))
     * {
     *     accumulator = (accumulator << 8) ^
     *         _crc_precomputed_table[(accumulator >> 24)
     *             ^ next_byte(augmented_msg)];
     * }
     */
    u_int32 accumulator = SYScrc32_INITIAL_SEED;

    while (data_blk_size--)
    {
        accumulator = (accumulator << 8) ^
            _crc_precomputed_table[((accumulator >> 24) & 0xff) ^ *data_blk_ptr];
        data_blk_ptr++;
    }

    accumulator ^= SYScrc32_INITIAL_SEED;

    return accumulator;
}
