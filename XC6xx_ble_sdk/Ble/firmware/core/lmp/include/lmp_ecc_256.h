#ifndef ECC_256_HEADER
#define ECC_256_HEADER
/******************************************************************************
 * MODULE NAME:    lmp_ecc.h
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:   
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:        
 *
 * SOURCE CONTROL: $ $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2009 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 ******************************************************************************/

#include "lmp_acl_connection.h"
#define u_int32 unsigned int
#define u_int16 unsigned short int
#define u_int8 unsigned char

#define MAX_OCTETS256 36
#define MAX_DIGITS_256  72 // ( (9+9) * 4 )

typedef struct bigHex256 
{
	u_int32 num[MAX_OCTETS256/4];
	u_int32  len;
	u_int32  sign;
} bigHex256;

typedef struct veryBigHex256 
{
	u_int32 num[MAX_OCTETS256/2];
	u_int32  len;
	u_int32  sign;
} veryBigHex256;

typedef struct ECC_Point256
{
	bigHex256 x;
	bigHex256 y;
} ECC_Point256;

void ECC_Point_Multiplication256(const bigHex256* pk,const ECC_Point256* pPointP,t_lmp_link* p_link,boolean blocking);
int notEqual256(const bigHex256 *bigHexA,const bigHex256 *bigHexB);
__INLINE__ void GF_Point_Copy256(const ECC_Point256 *source,ECC_Point256 *destination);


void LMecc_Generate_ECC_Key256(const u_int8* secret_key, const u_int8* public_key_x,
					   const u_int8* public_key_y,t_lmp_link* p_link,boolean blocking);

int LMecc_isValidSecretKey_256(u_int8* secretKey);
void ECC_Point_Multiplication_uint8_non_blocking256(t_lmp_link* p_link );

#endif
