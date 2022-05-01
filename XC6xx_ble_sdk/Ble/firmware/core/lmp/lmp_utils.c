/**************************************************************************
 * MODULE NAME:    lmp_utils.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Link Manager Utility functions
 * AUTHOR:         Gary Fleming
 * DATE:           04-12-1999
 *
 * SOURCE CONTROL: $Id: lmp_utils.c,v 1.14 2004/07/07 14:22:10 namarad Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * DESCRIPTION
 *     This module contains some general utility functions used by all link
 *     manager module. 
 *        i.e.     Setting a Bd_Addr
 *                 Checking for Bd_Addr Match
 *
 **************************************************************************/
#include "sys_config.h"

#include "lmp_types.h"
#include "lmp_utils.h"
#ifndef REDUCE_ROM
/***************************************************************
 * FUNCTION :- LMutils_Set_Bd_Addr
 * DESCRIPTION 
 * Assigns the Bluetooth device address pointed to by "p_bda_src"
 * to the device addess pointed to by "p_bda_dest".
 **************************************************************/
void LMutils_Set_Bd_Addr(t_bd_addr *p_bda_dest, const t_bd_addr *p_bda_src)
{
    BDADDR_Copy(p_bda_dest, p_bda_src);
}
#endif
/***************************************************************
 * FUNCTION :- LMutils_Bd_Addr_Match
 * DESCRIPTION
 * Checks if two Bluetooth Device Addresses Match
 **************************************************************/
int LMutils_Bd_Addr_Match(const t_bd_addr *p_bd_addr_1, 
                          const t_bd_addr *p_bd_addr_2)
{
    return BDADDR_Is_Equal(p_bd_addr_1, p_bd_addr_2);
}

/***************************************************************
 * FUNCTION :- LMutils_Array_Copy
 * DESCRIPTION
 * Copies an array of u_int8s.
 **************************************************************/
void LMutils_Array_Copy(u_int8 ARRAY_SIZE, u_int8* arr_1, const u_int8* arr_2)
{
    int i=0;

    do
    {
        arr_1[i] = arr_2[i];
        i++;
    } while (i < ARRAY_SIZE);
}

/***************************************************************
 * FUNCTION :- LMutils_Get_Uint16
 * DESCRIPTION
 * Retrieves a Uint16 value from a position in a PDU pointed to
 * by "p_pdu".
 **************************************************************/
u_int16 LMutils_Get_Uint16(t_p_pdu p_pdu)
{
    return ((u_int16)p_pdu[0] + (u_int16)(p_pdu[1] << 8));
}
#ifndef REDUCE_ROM
/***************************************************************
 * FUNCTION :- LMutils_Get_Uint24
 * DESCRIPTION
 * Retrieves a Uint24 value from a position in a PDU pointed to
 * by "p_pdu". And returns it as a u_int32.
 **************************************************************/
u_int32 LMutils_Get_Uint24(u_int8* p_pdu)
{
    return (*p_pdu) + ((*(p_pdu+1)) << 8) + ((*(p_pdu+2)) <<16 );
}
#endif
/***************************************************************
 * FUNCTION :- LMutils_Get_Uint32
 * DESCRIPTION
 * Retrieves a Uint32 value from a position in a PDU pointed to
 * by "p_pdu". And returns it as a u_int32.
 **************************************************************/
u_int32 LMutils_Get_Uint32(u_int8* p_pdu)
{
    return (*p_pdu) + ((*(p_pdu+1)) << 8) + 
           ((*(p_pdu+2)) <<16) + ((*(p_pdu+3)) <<24);
}

/****************************************************************
 * FUNCTION : LMutils_Set_Uint8
 *
 * DESCRIPTION
 * Stores the u_int8 value to bytes in memory (little endian)
 *
 * Use the macro mLMutils_Set_Uint8
 ***************************************************************/


/****************************************************************
 * FUNCTION : LMutils_Set_Uint16
 *
 * DESCRIPTION
 * Stores the u_int16 value to bytes in memory (little endian)
 ***************************************************************/
void LMutils_Set_Uint16(t_p_pdu p_buffer, u_int16 value_16_bit)
{
   p_buffer[0] = (u_int8)(value_16_bit);
   p_buffer[1] = (u_int8)(value_16_bit >> 8);
}

/***************************************************************
 * FUNCTION :- LMutils_Set_Uint32
 * DESCRIPTION
 * Stores the u_int32 value to bytes in memory (little endian)
 **************************************************************/
void LMutils_Set_Uint32(t_p_pdu p_buffer, u_int32 value_32_bit)
{
    p_buffer[0]=(u_int8)value_32_bit;
    p_buffer[1]=(u_int8)(value_32_bit >> 8);
    p_buffer[2]=(u_int8)(value_32_bit >> 16);
    p_buffer[3]=(u_int8)(value_32_bit >> 24);
}
#ifndef REDUCE_ROM
/***************************************************************
 * FUNCTION :- LMutils_Alloc_u_int8_ID
 * DESCRIPTION
 * Allocate a identifier from the bit field pool representing
 * the identifiers 0..255
 *
 * Pool             An array of 8 u_int32s where each bit
 *                  if 1 indicates identifer is allocated.
 **************************************************************/
u_int8 LMutils_Alloc_u_int8_ID(u_int32 *Pool)
{
    u_int32  pool_word_entry;
    u_int8   offset;
    u_int8   pool_word;

    for(pool_word=0; pool_word <= 7; pool_word++)
    {
        offset = 0;
        while (offset < 32)
        {
            /*
             * Check if entry free by checking if pool entry bit is set
             * If entry is free Then Allocate and Exit Else try next Endif
             */          
            pool_word_entry = (1 << offset);
            if (Pool[pool_word] & pool_word_entry)
            {
                Pool[pool_word] &= ~pool_word_entry;
                return ((pool_word * 32) + offset);
            }
            else
            {
                offset++;
            }
        }
    }
    return 0;
}

/***************************************************************
 * FUNCTION :- LMutils_Free_u_int8_ID
 * DESCRIPTION
 * Releases an identifier to the bit field pool 
 * representing the identifiers 0..255.
 *
 * Pool             An array of 8 u_int32s where each bit
 *                  if 1 indicates identifer is allocated.
 * id               The identifier to be returned to the pool
 **************************************************************/
void LMutils_Free_u_int8_ID(u_int32 *pool, u_int8 id)
{
   u_int8 pool_word;
   u_int8 offset;
   u_int32 pool_word_entry;
   
   pool_word = (id >> 5);
   offset = id - (pool_word*32);
 
   pool_word_entry = (1 << offset);
   /*
    * Set pool entry bit to indicate available
    */
   pool[pool_word] |= pool_word_entry;
}
#endif