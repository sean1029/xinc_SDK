/**************************************************************************
 * MODULE NAME:    lmp_ecc_256.c       
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    LMP Handling for ECC.
 * AUTHOR:         Gary Fleming
 * DATE:           02 Feb 2009
 *
 * SOURCE CONTROL: $Id: lmp_ecc_256.c,v 1.1 2013/06/05 13:08:00 garyf Exp $
 *
 *
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2009 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 * NOTES:
 * This module encompasses the protocol exchange to support Secure Simple Pairing
 * 
 * This module encompasses the protocol only and depends on the cryptographic functions
 * which are defined in lmp_ssp_engine.c and lmp_ecc.c
 *
 **************************************************************************/

#include "sys_config.h"
#include "lmp_acl_connection.h"
#include "lmp_config.h"
#include "lmp_timer.h"
#include "lmp_ecc.h"
#include "lmp_ecc_256.h"
#include "lmp_ssp.h" 
//#include "lmp_features.h"
#include <string.h>                     /* For memset */ 
    
#if 1//(PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_ECC256_SUPPORTED==1)

// Turn on Below for 'best' ARM optimisation of this file.
//#pragma Otime
//#pragma O2

 const u_int8 BasePoint_x_256[32] = {0x6b,0x17,0xd1,0xf2,0xe1,0x2c,0x42,0x47,0xf8,0xbc,0xe6,0xe5,0x63,0xa4,0x40,0xf2,0x77,0x03,0x7d,0x81,0x2d,0xeb,0x33,0xa0,0xf4,0xa1,0x39,0x45,0xd8,0x98,0xc2,0x96};
 const u_int8 BasePoint_y_256[32] = {0x4f,0xe3,0x42,0xe2,0xfe,0x1a,0x7f,0x9b,0x8e,0xe7,0xeb,0x4a,0x7c,0x0f,0x9e,0x16,0x2b,0xce,0x33,0x57,0x6b,0x31,0x5e,0xce,0xcb,0xb6,0x40,0x68,0x37,0xbf,0x51,0xf5};
  
 // maxSecretKey = r_265/2
 // r_256 =   0x1999999980000000199999999999999992E3D7295EB4DDF2972F634AFC632551
 // r_256/2 = 0x0CCCCCCCC00000000CCCCCCCCCCCCCCCC971EB94AF5A6EF94B97B1A57E3192A8 
   
 const u_int8 maxSecretKey_256[32] ={0x0C,0xCC,0xCC,0xCC,0xC0,0x00,0x00,0x00,0x0C,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xC9,0x71,0xEB,0x94,0xAF,0x5A,0x6E,0xF9,0x4B,0x97,0xB1,0xA5,0x7E,0x31,0x92,0xA8};
 

#ifdef _MSC_VER
#if (_MSC_VER==1200)
#define _MSVC_6
#endif
#endif

#ifdef _MSVC_6
 typedef __int64 u64;
#define _LL(x) x
#else
 typedef unsigned long long int u64;  
#define _LL(x) x##ll
#endif

 const bigHex256 bigHexP256 = {{ 0x00000000,0xFFFFFFFF,0x00000001,0x00000000,0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},
						      8,0};							
 const veryBigHex256 veryBigHexP256 = 
						    {{  0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
								0x00000000,0xFFFFFFFF,0x00000001,0x00000000,0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},
					            8,0};

#define MAX_OCTETS256  36
//#define BIT16 0
#define MAX_DIGITS_256  72 // ( (9+9) * 4 )
#define ELEMENTS_BIG_HEX256 9 // MAX_OCTETS/4
#define HIGHEST_INDEX_BIG_HEX256  8// (MAX_OCTETS/4 )- 1
#define ISBIGHEXEVEN256(x) (!((x.num[HIGHEST_INDEX_BIG_HEX256]) & 0x01))

#define GETMOSTSIGNIGICANTU32_256(tmpHexA)  tmpHexA->num[ELEMENTS_BIG_HEX256-tmpHexA->len]

static bigHex256 PrivateKey256;
static ECC_Point256 PublicKey256;
//static ECC_Point ResultPoint;
static u_int32 ECC_Point_Mul_Word256 = 0;
static ECC_Point256 LMecc_PointP256;
 ECC_Point256 LMecc_PointQ256;
static ECC_Point256 LMecc_PointR256;
static bigHex256 LMecc_Pk256;

void AddBigHex256(const bigHex256 *bigHexA,const bigHex256 *bigHexB, bigHex256 *BigHexResult);
void SubtractBigHex256( const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult);
void MultiplyBigHex256( const bigHex256 *bigHexA, const bigHex256 *bigHexB, veryBigHex256 *BigHexResult);
void bigHexInversion256( bigHex256* bigHexA, bigHex256* pResult);
void Add2SelfBigHex256( bigHex256 *bigHexA,const bigHex256 *bigHexB);

void SubtractBigHexMod256(const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult);

void SubtractFromSelfBigHexSign256( bigHex256 *bigHexA, const bigHex256 *bigHexB);
__INLINE__ void initBigNumber256(bigHex256 *BigHexResult);
// Not Used
//__INLINE__ void initVeryBigNumber(veryBigHex *BigHexResult);

__INLINE__ void setBigNumberLength256( bigHex256 *BigHexResult);
__INLINE__ void setVeryBigNumberLength256(veryBigHex256 *BigHexResult);

__INLINE__ void copyBigHex256(const bigHex256 *source,bigHex256 *destination);
__INLINE__ void copyVeryBigHex256(const veryBigHex256 *source,veryBigHex256 *destination);

__INLINE__ void shiftLeftOneArrayElement256(bigHex256 *input);

void MultiplyBigHexModP256(const bigHex256 *bigHexA,const bigHex256 *bigHexB, bigHex256 *BigHexResult);

__INLINE__ void divideByTwo256(bigHex256* A);

__INLINE__ int isbigHexEven256(const bigHex256* A);


__INLINE__ int isGreaterThan256(const bigHex256 *bigHexA,const bigHex256 *bigHexB);
__INLINE__ int isGreaterThanOrEqual256(const bigHex256 *bigHexA,const bigHex256 *bigHexB);
__INLINE__ int isVeryBigHexGreaterThanOrEqual256(const veryBigHex256 *bigHexA,const veryBigHex256 *bigHexB);


void MultiplyByU32ModP256(const u_int32 N,bigHex256* result);
void MultiplyByU32ModPInv256(const u_int32 N,bigHex256* result);
void specialModP256(bigHex256 *tmpHexA);
void AddPdiv2_256(bigHex256 *bigHexA);
void AddP_256(bigHex256 *bigHexA);
// Galois Field Operands
void GF_Point_Addition256(const ECC_Point256* PointP,const ECC_Point256* PointQ, ECC_Point256* ResultPoint);
void LMecc_CB_ECC_Point_Multiplication_Complete256(t_lmp_link* p_link);
void ECC_Point_Multiplication_uint32_256(t_lmp_link* p_link);
void ECC_Point_Multiplication_uint8_256(t_lmp_link* p_link,boolean blocking);
void ECC_Point_Multiplication_uint8_non_blocking_256(t_lmp_link* p_link );

int LMecc_isValidSecretKey_256(u_int8* secretKey)
{
	// First check that the secret key is not all zeros. 
	//
	int i;

	for(i=0;i<32;i++)
	{
		if (secretKey[i] != 0)
			break;
	}

	if ((i==32) && (secretKey[31] == 0))
		return 0;

	for(i = 0; i < 32; i++)
	{
		if (secretKey[i] > maxSecretKey_256[i])
			return 0;
		else if (secretKey[i] < maxSecretKey_256[i])
			return 1;
	}
    return 1;
}  

/******************************************************************************
 * Section of functions for BASIC big number handling 
 *   These do not consider the Finite Field or ECC
 *
 *  Mod P is not performed 
 *  Sign is not considered -- all inputs and results  are assumed to be positive
 *
 *  AddBigHex            - This adds two 15*u16 positive numbers.
 *  AddVeryBigHex        - This adds two 30*u16 positive numbers.
 *
 *  SubtractBigHex       - This subtracts two 15*u16 positive numbers 
 *                         To ensure a +ive result we assume A >= B
 *  SubtractVeryBigHex   - This subtracts two 30*u16 positive numbers 
 *                         To ensure a +ive result we assume A >= B
 *  MultiplyBigHex       - This multiplies two 15*u16 positive numbers to produce
 *                         a 30*u16 positive result
 *                              
 ******************************************************************************/

//
//
// Inputs - Format of the inputs that they begin with the LEAST significant bytes in the 0 entry of the array.
//
void AddBigHex256( const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult)
{
	u64 tmp;
	u_int32 carry = 0;
	s_int32 i;
    const u_int32 *numA = bigHexA->num;
    const u_int32 *numB = bigHexB->num;
	u_int32 *result = BigHexResult->num;  

	for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
	{
		tmp = (u64)((u64)(numA[i]) + (u64)(numB[i]) + carry);
		if(tmp & _LL(0x100000000))
			carry = 0x01;
		else
			carry = 0x00;

		result[i] = (tmp & 0xFFFFFFFF);
	}
	setBigNumberLength256(BigHexResult);
}

// A = A + B
void Add2SelfBigHex256( bigHex256 *bigHexA,const bigHex256 *bigHexB)
{
	u64 tmp;
	u_int32 carry = 0;
	s_int32 i;
	u_int32 *numA = bigHexA->num;
	const u_int32 *numB = bigHexB->num;

	for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
	{
		tmp = (u64)((u64)(numA[i]) + (u64)(numB[i]) + carry);
		if(tmp & _LL(0x100000000))
			carry = 0x01;
		else
			carry = 0x00;

		numA[i] = (tmp & 0xFFFFFFFF);
	}
	setBigNumberLength256(bigHexA);
}

void SubtractBigHex256(const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult)
{
    // Due to the nature of the GF we have to assume both inputs are +ive.
	u_int32 borrow=0;
	s_int32 i;
	const u_int32* pBigNum = bigHexA->num;
	const u_int32* pSmallNum = bigHexB->num;
	u_int32* pResult = BigHexResult->num;

	for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
	{
 		if (((u64)((u64)(pSmallNum[i])+(u64)borrow)) > (pBigNum[i]))
		{
			pResult[i] = (((u64)(pBigNum[i] + _LL(0x100000000)))- (pSmallNum[i]+borrow)) & 0xFFFFFFFF;
			borrow = 0x01;
		}
		else
		{
			pResult[i] = ((pBigNum[i])- (pSmallNum[i]+borrow)) & 0xFFFFFFFF;
			borrow = 0x00;
		}
	}
    setBigNumberLength256(BigHexResult);
}

// A = A - B

void SubtractFromSelfBigHex256(bigHex256 *bigHexA,const bigHex256 *bigHexB)
{
    // Due to the nature of the GF we have to assume both inputs are +ive.
	u_int32 borrow=0;
	s_int32 i;
	u_int32* pBigNum = bigHexA->num;
	const u_int32* pSmallNum = bigHexB->num;

	for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
	{
 		if (((u64)((u64)(pSmallNum[i])+(u64)borrow)) > (pBigNum[i]))
		{
			pBigNum[i]  = (((u64)(pBigNum[i] + _LL(0x100000000)))- (pSmallNum[i]+borrow)) & 0xFFFFFFFF;
			borrow = 0x01;
		}
		else 
		{
			pBigNum[i] = ((pBigNum[i])- (pSmallNum[i]+borrow)) & 0xFFFFFFFF;
			borrow = 0x00;
		}
	}
    setBigNumberLength256(bigHexA);
}

/**********************************************************************************
 * This section extends of the basic functions provided previously to support 
 * sign. We concentrate on the Add and Subtract functions for both 15*u16 and 30*u16 
 * numbers.
 *
 * AddBigHexSign         - Adds two 15*u16 numbers considering the sign.
 * SubtractBigHexSign    - Subtracts two 15*u16 numbers considering the sign
 * AddVeryBigHexSign
 * SubtractVeryBigHexSign
 * MultiplyBigHexSign    - Multiplies two 15*u16 numbers considering the sign.
 **********************************************************************************/

// AddP is only invoked when the sign of A is -ive
// A is always less than P 
//
void AddP256( bigHex256 *bigHexA)
{
	bigHex256 BigHexResult;
	SubtractBigHex256(&bigHexP256, bigHexA,&BigHexResult);
	copyBigHex256(&BigHexResult,bigHexA);
	bigHexA->sign = 0;
}

void AddPdiv2_256( bigHex256 *bigHexA)
{
	//
	u_int32 *numA = bigHexA->num;
	const u_int32 *numB = bigHexP256.num;

	if (bigHexA->sign == 0)
	{
		u64 tmp;
		u_int32 carry;
		s_int32 i;

		carry = 0;

		for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
		{
			tmp = (u64)((u64)(numA[i]) + (u64)(numB[i]) + carry);
			if(tmp & _LL(0x100000000))
				carry = 0x01;
			else
				carry = 0x00;

			numA[i] = (tmp & 0xFFFFFFFF);
		}
	}
	else // if (bigHexA->sign == 1)
	{
		if ( isGreaterThan256(bigHexA, &bigHexP256))
		{

			// Due to the nature of the GF we have to assume both inputs are +ive.
			u_int32 borrow=0;
			s_int32 i;

			for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
			{
				if (((u64)((u64)(numB[i])+(u64)borrow)) > (numA[i]))
				{
					numA[i]  = (((u64)(numA[i] + _LL(0x100000000)))- (numB[i]+borrow)) & 0xFFFFFFFF;
					borrow = 0x01;
				}
				else 
				{
					numA[i] = ((numA[i])- (numB[i]+borrow)) & 0xFFFFFFFF;
					borrow = 0x00;
				}
			}
		}
		else
		{
			bigHex256 BigHexResult;

			SubtractBigHex256(&bigHexP256,bigHexA,&BigHexResult);
			copyBigHex256(&BigHexResult,bigHexA);
			bigHexA->sign = 0;
		}
	}
	// divide by 2
	// divideByTwo(bigHexA);
	{
		u_int32 rem=0;
		u64 u64val;
		u_int32 i;

		for(i=0; i < (ELEMENTS_BIG_HEX256); i++)
		{
			u64val = (u64)((u64)(numA[i]) + ((u64)rem * _LL(0x100000000)));

			numA[i] = (u_int32)(u64val>>1);
			rem = (u64)(u64val - ((u64val>>1)<<1));
		}

		// We need to re-calculate the length. 
		setBigNumberLength256(bigHexA);
	}
}

void SubtractFromSelfBigHexSign256( bigHex256 *bigHexA, const bigHex256 *bigHexB)
{
	// This function uses use the basis AddBigHex and SubtractBigHex to implement a full
	// Addition which considers sign and mod.
	// 
	if (bigHexA->sign == 0)
	{
		if (bigHexB->sign == 0)
		{
			if ( isGreaterThanOrEqual256(bigHexA,bigHexB))
			{
				SubtractFromSelfBigHex256(bigHexA, bigHexB);
			}
			else
			{
				bigHex256 BigHexResult;

				SubtractBigHex256(bigHexB, bigHexA,&BigHexResult);
				copyBigHex256(&BigHexResult,bigHexA);
				bigHexA->sign = 1;
			}
		}
		else 
		{
			// 3/  if A is + and B is -   C =   A+B  Mod
			Add2SelfBigHex256(bigHexA, bigHexB);
		}
	}
	else  // if (bigHexA->sign == 1)
	{
		if (bigHexB->sign == 0)
		{
			Add2SelfBigHex256(bigHexA, bigHexB);
		}
		else
		{
			if ( isGreaterThanOrEqual256(bigHexB,bigHexA))
			{
				bigHex256 BigHexResult;

				SubtractBigHex256(bigHexB, bigHexA,&BigHexResult);
				copyBigHex256(&BigHexResult,bigHexA);
				bigHexA->sign = 0;
			}
			else
			{
				SubtractFromSelfBigHex256(bigHexA, bigHexB);
			}
		}
	}
}
/*****************************************************************************
 * Following functions further extend on the basic functions to include Mod P.
 *
 * AddBigHexMod  -- This function takes A and B which can be signed and output 
 *                  a result C which is less than P.
 *                  It call AddBigHexSign and then perform Mod P on the output.
 *                  If the output is -ive it recursively add P until we get a +ive
 *                  number which is less than P.
 *                  If the output is +ive it recursively subtracts P until we get 
 *                  a +number which is less than P.
 * 
 * SubtractBigHexMod  -- This function takes A and B which can be signed and output 
 *                  a result C which is less than P.
 *                  It calls SubtractBigHexSign and then performs Mod P on the output.
 *                  If the output is -ive it recursively add P until we get a +ive
 *                  number which is less than P.
 *                  If the output is +ive it recursively subtracts P until we get 
 *                  a +number which is less than P.
 * 
 * MultiplyBigHexMod
 ****************************************************************************/
void SubtractBigHexMod256(const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult)
{
	if (bigHexA->sign == 0)
	{
		if (bigHexB->sign == 0)
		{
			if ( isGreaterThanOrEqual256(bigHexA,bigHexB))
			{
				SubtractBigHex256(bigHexA, bigHexB,BigHexResult);
				BigHexResult->sign = 0;
			}
			else
			{
				SubtractBigHex256(bigHexB, bigHexA,BigHexResult);
				BigHexResult->sign = 1;
			}
		}
		else 
		{
			// 3/  if A is + and B is -   C =   A+B  Mod
			AddBigHex256(bigHexA, bigHexB,BigHexResult);
			BigHexResult->sign = 0;
		}
	}
	else  // if (bigHexA->sign == 1)
	{
		if (bigHexB->sign == 0)
		{
			AddBigHex256(bigHexA, bigHexB,BigHexResult);
			BigHexResult->sign = 1;
		}
		else
		{
			if ( isGreaterThanOrEqual256(bigHexB,bigHexA))
			{
				SubtractBigHex256(bigHexB, bigHexA,BigHexResult);
				BigHexResult->sign = 0;
			}
			else
			{
				SubtractBigHex256(bigHexA, bigHexB,BigHexResult);
				BigHexResult->sign = 1;
			}
		}
	}

	if (BigHexResult->sign == 0x00)   // Positive Number
	{
		if(isGreaterThanOrEqual256(BigHexResult,&bigHexP256))
	    { 
			//SubtractFromSelfBigHexSign(BigHexResult,&bigHexP);
			SubtractFromSelfBigHex256(BigHexResult, &bigHexP256);
		}
	}	
	else
	{
		AddP256(BigHexResult);
	}
}

// From curren t usage we can safely assume both inputs are positive.

void SubtractBigHexUint32_256(const bigHex256 *bigHexA, const u_int32 numB, bigHex256 *BigHexResult)
{
	initBigNumber256(BigHexResult);

	if (bigHexA->num[HIGHEST_INDEX_BIG_HEX256] >= numB)
	{
		copyBigHex256(bigHexA,BigHexResult);
		BigHexResult->num[ELEMENTS_BIG_HEX256-1] = bigHexA->num[ELEMENTS_BIG_HEX256-1]-numB;
	}
	else
	{
		bigHex256 N;

		initBigNumber256(&N);

		N.len = 1;
		N.num[HIGHEST_INDEX_BIG_HEX256] = numB;
		SubtractBigHexMod256(bigHexA,&N,BigHexResult);
	}
}


void MultiplyBigHexModP256(const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult)
{
	veryBigHex256 tmpResult;
	{
		s_int32 k;
		const u_int32 *numA = bigHexA->num;
		const u_int32 *numB = bigHexB->num;
		u_int32 *result = tmpResult.num;

		memset(tmpResult.num,0,72);
		tmpResult.len = 0;
		tmpResult.sign = 0;
		//
		// Below Optimisations seem to degrade performance on Windows by a factor of 2.
		//
		for(k=(ELEMENTS_BIG_HEX256-1);k >= 0;k--)
		{
			u_int32 mcarry = 0;
			s_int32 j = 0;

			// The inner loop multiplies each byte of HexB by a single byte of 
			// HexA
			for(j=(ELEMENTS_BIG_HEX256-1);j >=0;j--)
			{
				u64 val;

				val = (((u64)(numA[k]) ) * ((u64)(numB[j]))) + result[j+k+1] + mcarry;
				result[j+k+1] = (val & 0xFFFFFFFF);
				mcarry = (u_int32)( val >> 32);
			}
		}
		setVeryBigNumberLength256(&tmpResult);
		tmpResult.sign = (bigHexA->sign != bigHexB->sign);
	}

	{
		u_int32 i=0;
		bigHex256 tmpResult2;

		while ((tmpResult.num[i] == 0x00) && (i<(ELEMENTS_BIG_HEX256)))
			i++;

		// Take the next 13 elements 
		// 
		tmpResult2.sign = tmpResult.sign;

		if (isVeryBigHexGreaterThanOrEqual256(&tmpResult,&veryBigHexP256))
		{
			while(tmpResult.num[i] == 0x00)
				i++;
		}
		memcpy(tmpResult2.num,tmpResult.num+i,36);
		setBigNumberLength256(&tmpResult2);
#if 1
		while((i+ELEMENTS_BIG_HEX256) < (MAX_OCTETS256/2))
		{
			specialModP256(&tmpResult2);
			shiftLeftOneArrayElement256(&tmpResult2);

			// Add the next byte from A in left_over;
			tmpResult2.num[HIGHEST_INDEX_BIG_HEX256] = tmpResult.num[i+ELEMENTS_BIG_HEX256];
			i++;
			setBigNumberLength256(&tmpResult2);
		}
#else
		{
			bigHex256 s1,s2,s3,s4,s5,s6,s7,s8,s9;

			u_int32 c15,c14,c13,c12,c11,c10,c9,c8,c7,c6,c5,c4,c3,c2,c1,c0;


			s1[

		}
#endif
		specialModP256(&tmpResult2);
		copyBigHex256(&tmpResult2,BigHexResult);
	}
}


void MultiplyBigHexByUint32_256(const bigHex256 *bigHexA, const u_int32 numB, bigHex256 *BigHexResult)
{
	s_int32 k;
	const u_int32 *numA = bigHexA->num;
	u_int32 *result = BigHexResult->num;
	u_int32 mcarry = 0;

	//
	// Below Optimisations seem to degrade performance on Windows by a factor of 2.
	//
	for(k=HIGHEST_INDEX_BIG_HEX256;k >= 0;k--)
	{
		u64 val;

		val = (((u64)(numA[k]) ) * ((u64)numB)) + mcarry;
		result[k] = (val & 0xFFFFFFFF);
		mcarry = (u_int32)( val >> 32);
	}
	setBigNumberLength256(BigHexResult);
	BigHexResult->sign = bigHexA->sign;
	specialModP256(BigHexResult);
}


__INLINE__ void shiftLeftOneArrayElement256(bigHex256 *input)
{
	memcpy(input->num,(input->num+1),32);
}

//
// This function adds N * Mod P to a number.
//  
// The left_over = tmpHexA % P

void specialModP256(bigHex256 *tmpHexA)
{
  // bigHex256 NModPInv;
	bigHex256 NModP;
   u_int32 SignificantU32;

   initBigNumber256(&NModP);
   if(((tmpHexA->sign == 0) && isGreaterThanOrEqual256(tmpHexA,&bigHexP256)) ||
	   (tmpHexA->sign == 1))
   {
	   // If I multiply the most significant u_int16 of A by the Mod P and then subtract from A 
	   // this is equivalent to an iteractive subtraction - but much faster.
	   if (tmpHexA->len > bigHexP256.len)
	   {
		   // approach
		   SignificantU32 = GETMOSTSIGNIGICANTU32_256(tmpHexA);
		   MultiplyByU32ModP256(SignificantU32,&NModP);
	       //tmpHexA->num[0] = 0x00;

		   if (tmpHexA->sign == 0)
		   {
			   SubtractFromSelfBigHexSign256(tmpHexA,&NModP);
		   }
		   else
		   {
			   Add2SelfBigHex256(tmpHexA,&NModP);
		   }
	   }


	   if (((tmpHexA->sign == 0) && isGreaterThanOrEqual256(tmpHexA,&bigHexP256)) ||
			(tmpHexA->sign == 1))
	   {
		 // It is implicit that the sign is negative. 
	     // so we can remove the check.
		 //
		   if (tmpHexA->sign == 1)
		   {
			   AddP256(tmpHexA);
		   }
		   else
		   {
			   // Can this be replaced by Adding the invert of P
			   SubtractFromSelfBigHex256(tmpHexA, &bigHexP256);
		   }
	   }
   }
}

void MultiplyByU32ModP256(const u_int32 N,bigHex256* result)
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// This is the one function which could be wrong for P256
	//
//	u64 tempVal;
//	u_int32 i;
//	tempVal = _LL(0x100000000) - N;

	u_int32 tempVal = (0xFFFFFFFF - N) + 1;

#if 1  // Trial of Slow Way
	result->num[0] = N - 1;
	result->num[1] = (u_int32)tempVal;
	result->num[2] = N ;
	result->num[3] = 0x00 ;
	result->num[4] = 0x00 ;
    result->num[5] = N-1;
    result->num[6] = 0xFFFFFFFF;
    result->num[7] = 0xFFFFFFFF;
	result->num[8] = (u_int32)tempVal;
#else
	for (i=0; i< N; i++)
		Add2SelfBigHex256( result , &bigHexP256);

#endif
 	result->sign = 0x00;
	if (result->num[0])
		result->len = 0x9;
	else 
		result->len = 0x8;
}

__INLINE__ void copyBigHex256(const bigHex256 *source,bigHex256 *destination)
{
	memcpy(destination->num,source->num,MAX_OCTETS256); //for P256 max_octets = 36

	destination->len = source->len;
	destination->sign = source->sign;
}

__INLINE__ void initBigNumber256(bigHex256 *BigHexResult)
{
	memset(BigHexResult->num,0,MAX_OCTETS256);

	BigHexResult->len = 0;
	BigHexResult->sign = 0;
}

__INLINE__ void setBigNumberLength256(bigHex256 *BigHexResult)
{
	int i;

	for(i=0;i<(ELEMENTS_BIG_HEX256);i++)
	{
		if(BigHexResult->num[i] != 0x00)
           break;
	}
	BigHexResult->len = (ELEMENTS_BIG_HEX256)-i;
}

__INLINE__ void setVeryBigNumberLength256(veryBigHex256 *BigHexResult)
{
	int i;

	for(i=0;i<(MAX_OCTETS256/2);i++)
	{
		if(BigHexResult->num[i] != 0x00)
           break;
	}
	BigHexResult->len = (MAX_OCTETS256/2)-i;
}

//
// if A > B return TRUE
//
  
__INLINE__ int isGreaterThan256(const bigHex256 *bigHexA,const bigHex256 *bigHexB)
{

   u_int32 i;
   u_int32 A_len = bigHexA->len;
   u_int32 B_len = bigHexB->len;

    if(A_len > B_len)
		return 1;
	else if(A_len < B_len)
		return 0;

	// Now we have two equal sized arrays which we need to compare.
    // . 
	for (i=((ELEMENTS_BIG_HEX256)-A_len); i <(ELEMENTS_BIG_HEX256);i++)
	{
		if (bigHexB->num[i] > bigHexA->num[i])
		{
			return 0;
			break;
		}
		else if (bigHexB->num[i] < bigHexA->num[i])
		{
			return 1;
			break;
		}
	}
	return 0;
}


__INLINE__ int isGreaterThanOrEqual256(const bigHex256 *bigHexA,const bigHex256 *bigHexB)
{
   u_int32 i;
   u_int32 A_len = bigHexA->len;
   u_int32 B_len = bigHexB->len;

    if(A_len > B_len)
		return 1;
	else if(A_len < B_len)
		return 0;

	// Now we have two equal sized arrays which we need to compare.
    // . 
    
	for (i=((ELEMENTS_BIG_HEX256)-A_len); i <(ELEMENTS_BIG_HEX256);i++)
	{
		if (bigHexB->num[i] > bigHexA->num[i])
		{
			return 0;
			break;
		}
		else if (bigHexB->num[i] < bigHexA->num[i])
		{
			return 1;
			break;
		}
	}
	return 1;
}


__INLINE__ int isVeryBigHexGreaterThanOrEqual256(const veryBigHex256 *bigHexA,const veryBigHex256 *bigHexB)
{
	int i;
	u_int32 A_len = bigHexA->len;
	u_int32 B_len = bigHexB->len;

    if(A_len > B_len)
		return 1;
	else if(A_len < B_len)
		return 0;

	// Now we have two equal sized arrays which we need to compare.
    // 
	for (i=((MAX_OCTETS256/2)-A_len); i <(MAX_OCTETS256/2);i++)
	{
		if (bigHexB->num[i] > bigHexA->num[i])
		{
			return 0;
			break;
		}
		else if (bigHexB->num[i] < bigHexA->num[i])
		{
			return 1;
			break;
		}
	}
	return 1;
}


/*__INLINE__*/ int notEqual256(const bigHex256 *bigHexA,const bigHex256 *bigHexB)
{
	u_int32 i;

	for(i=0;i<(ELEMENTS_BIG_HEX256);i++)
	{
		if(bigHexA->num[i] != bigHexB->num[i])
		{
			return 1;
		}
	}
	return 0;
}

//isLessThan
__INLINE__ int isLessThan256(const bigHex256 *bigHexA,const bigHex256 *bigHexB)
{
	u_int32 i;

	if(bigHexA->len < bigHexB->len)
		return 1;
	else if(bigHexA->len > bigHexB->len)
		return 0;

	// Now we have two equal sized arrays which we need to compare.
    // Start at the last entry is each array and compare. 

	for (i=bigHexA->len; i!=0;i--)
	{
		if (bigHexB->num[i] < bigHexA->num[i])
			return 0;
			break;
	}
	return 1;
}

void GF_Point_Addition256(const ECC_Point256* pPointP,const ECC_Point256* pPointQ, ECC_Point256* pResultPoint)
{
	// Gamma or S = ((y1-y2)/(x1-x2))
    // What is a  = -3 
	// x3 = (Gamma*Gamma) - x1 - x2
	// y3 = -y1 + Gamma*(x1 - x3)
	//see 115-EllipticCurveCryptography 
	 
	// Need to advance all GF operands so that they return a result in the field  -- Mod P
	bigHex256 gamma;
	bigHex256 tempResult;
	bigHex256 DiffX_coords;

	if (pPointP->y.len == 0)
	{
		if (pPointP->x.len == 0)
		{
			// Let the result be PointQ
			GF_Point_Copy256(pPointQ,pResultPoint);
			return;
		}
		else if (pPointQ->y.len == 0)
		{
			return;
		}
	}
	else if ((pPointQ->y.len == 0) && (pPointQ->x.len == 0))
	{
		// Let the result be PointP
        GF_Point_Copy256(pPointP,pResultPoint);
		return;
	}

	if (notEqual256(&(pPointP->x),&(pPointQ->x)))
	{
		bigHex256 tempResult4;
		bigHex256 DiffY_coords;

		// Determine Gamma = S = (yP - yQ)/(xP - xQ)
		SubtractBigHexMod256(&(pPointP->y),&(pPointQ->y),&DiffY_coords);
		SubtractBigHexMod256(&(pPointP->x),&(pPointQ->x),&DiffX_coords);
		bigHexInversion256(&DiffX_coords,&tempResult4);
		MultiplyBigHexModP256(&tempResult4,&DiffY_coords,&gamma);
	}
	else
	{
		bigHex256 xP_squared;
		bigHex256 tempResult4;
		bigHex256 tempResult2;

		// S = (3xP^2 + A)/2yP     A = -3
		// xP^2
		// !!! Scope for Optimisation of the Function below into 
		// a variant specifically for squaring.

		MultiplyBigHexModP256(&(pPointP->x),&(pPointP->x),&xP_squared);
		// 3 * xP^2
		MultiplyBigHexByUint32_256(&xP_squared,0x03,&tempResult);
        // 3xP^2 - 3
		SubtractBigHexUint32_256(&tempResult,0x3,&tempResult2);

		MultiplyBigHexByUint32_256(&(pPointP->y),0x02,&tempResult);
		bigHexInversion256(&tempResult,&tempResult4);

		MultiplyBigHexModP256(&tempResult4,&tempResult2,&gamma);
	}
	{
		bigHex256 GammaSquared;

		// Determine x3 = S^2 -xP -xQ
		MultiplyBigHexModP256(&gamma,&gamma,&GammaSquared);
		SubtractBigHexMod256(&GammaSquared,&pPointP->x,&tempResult);
		SubtractBigHexMod256(&tempResult,&pPointQ->x,&pResultPoint->x);
	}
	// Determine y3 = S(xP - x3) - yP

	SubtractBigHexMod256(&pPointP->x,&pResultPoint->x, &DiffX_coords);
    MultiplyBigHexModP256(&DiffX_coords,&gamma,&tempResult);
    SubtractBigHexMod256(&tempResult,&pPointP->y,&pResultPoint->y);

}

//    Q = kP
//  
/********************************************************
 * Function :- ECC_Point_Multiplication
 *
 * Parameters :- pk       - a pointer to a bigHex which the point should be multiplied by. 
 *               pPointP  - pointer to the point which is to be multiplied by pk
 * 
 * Description 
 * This function performs ECC point multiplication. It uses the Scalar Multiplication
 * algorithm. Scalar multiplication works is way through the bits of a BigHex starting with the LSB.
 * For each bit (irrespective of Value) a point Doubling is performed, if the bit has a value = 1 then
 * a point addition is also performed. 
 *
 * Scalar Multiplication: LSB first
 *   • Require k=(km-1,km-2,…,k0)2, km=1
 *   • Compute Q=kP
 * – Q=0, R=P
 * – For i=0 to m-1
 *   • If ki=1 then
 *      – Q=Q+R
 *   • End if
 *   • R=2R
 * – End for
 * – Return Q
 ******************************************************************************************/
void ECC_Point_Multiplication256(const bigHex256* pk,const ECC_Point256* pPointP,t_lmp_link* p_link,boolean blocking)
{
	// see ecc.pdf
	//

	int i;

	GF_Point_Copy256(pPointP,&LMecc_PointP256);
	GF_Point_Copy256(pPointP,&LMecc_PointR256);
	copyBigHex256(pk,&LMecc_Pk256);

	initBigNumber256(&LMecc_PointQ256.x);
	initBigNumber256(&LMecc_PointQ256.y);

	//ECC_Point_Mul_Word = ELEMENTS_BIG_HEX;
	ECC_Point_Mul_Word256 = ELEMENTS_BIG_HEX256*4;
   // ECC_Point_Multiplication_uint32(p_link);

	if (blocking)
	{
		for(i = ECC_Point_Mul_Word256; i > 0; i --)
			ECC_Point_Multiplication_uint8_256(p_link,blocking);
	}
	else
		ECC_Point_Multiplication_uint8_256(p_link,blocking);

}

void LMecc_CB_ECC_Point_Multiplication_Complete256(t_lmp_link* p_link)
{
	u_int32 big_num_offset;
	u_int32 i,j=0;

	// Copy the secret_key to a bigHex format.
	big_num_offset = 1;

	if (p_link)
	{
		for(i=0;i<32;)
		{
			p_link->DHkey256[i] =   ((LMecc_PointQ256.x.num[j+big_num_offset] & 0xFF000000) >> 24);
			p_link->DHkey256[i+1] = ((LMecc_PointQ256.x.num[j+big_num_offset] & 0xFF0000) >> 16);
			p_link->DHkey256[i+2] = ((LMecc_PointQ256.x.num[j+big_num_offset] & 0xFF00) >> 8);
			p_link->DHkey256[i+3] = ( LMecc_PointQ256.x.num[j+big_num_offset] & 0xFF);

			i+=4;
			j++;
		}

		// Need to invoke logic in the SSP protocol in case this calculation complete comes after the 
		// User_Confirmation_Request_Reply.

		LMssp_CallBack_DH_Key_Complete256(p_link);
	}
	else // p_link == 0  then local device.
	{
		for(i=0;i<32;)
		{
			g_LM_config_info.public_key_x256[i] =   ((LMecc_PointQ256.x.num[j+big_num_offset] & 0xFF000000) >> 24);
			g_LM_config_info.public_key_x256[i+1] = ((LMecc_PointQ256.x.num[j+big_num_offset] & 0xFF0000) >> 16);
			g_LM_config_info.public_key_x256[i+2] = ((LMecc_PointQ256.x.num[j+big_num_offset] & 0xFF00) >> 8);
			g_LM_config_info.public_key_x256[i+3] = ( LMecc_PointQ256.x.num[j+big_num_offset] & 0xFF);

			g_LM_config_info.public_key_y256[i] =   ((LMecc_PointQ256.y.num[j+big_num_offset] & 0xFF000000) >> 24);
			g_LM_config_info.public_key_y256[i+1] = ((LMecc_PointQ256.y.num[j+big_num_offset] & 0xFF0000) >> 16);
			g_LM_config_info.public_key_y256[i+2] = ((LMecc_PointQ256.y.num[j+big_num_offset] & 0xFF00) >> 8);
			g_LM_config_info.public_key_y256[i+3] = ( LMecc_PointQ256.y.num[j+big_num_offset] & 0xFF);

			i+=4;
			j++;
		}
	}
}

void ECC_Point_Multiplication_uint32_256(t_lmp_link* p_link)
{
	ECC_Point256 tmpResultPoint;
	ECC_Point256 tmpResultPoint2;

	u_int32 j;
	u_int32 bitVal;

	for (j=0;j<32;j++)
	{
		bitVal = (LMecc_Pk256.num[ECC_Point_Mul_Word256-1] >> j) & 0x0001;

		if (bitVal)
		{
			// Q = Q + R
            GF_Point_Addition256(&LMecc_PointQ256,&LMecc_PointR256,&tmpResultPoint);
			// Copy ResultPoint to PointQ
			GF_Point_Copy256(&tmpResultPoint,&LMecc_PointQ256);
		} 
		// Point Doubling
		// R = 2R
		GF_Point_Addition256(&LMecc_PointR256,&LMecc_PointR256,&tmpResultPoint2);
		// Copy Result point to PointR 
		GF_Point_Copy256(&tmpResultPoint2,&LMecc_PointR256);
	}

	// We Set a Timer - for the next Multiplication of next u_int32

	if (ECC_Point_Mul_Word256 == 1)
	{
		// Call Back a function which will store the ECC calculation result and 
		// flag that the calculation is complete.

		LMecc_CB_ECC_Point_Multiplication_Complete256(p_link);
	}
	else
	{ 
	    ECC_Point_Mul_Word256--;
		LMtmr_Set_Timer(4,ECC_Point_Multiplication_uint32_256, p_link, 1);
	}

}

void ECC_Point_Multiplication_uint8_256(t_lmp_link* p_link,boolean blocking)
{
	ECC_Point256 tmpResultPoint;
	ECC_Point256 tmpResultPoint2;

	u_int32 j;
	u_int32 bitVal;
	u_int8 byte=0;
	u_int8 byteOffset;
    
	u_int32 wordVal;
		
	wordVal = LMecc_Pk256.num[((ECC_Point_Mul_Word256+3)/4)-1];
	byteOffset = (ECC_Point_Mul_Word256-1)%4;

	switch(byteOffset)
	{
	case 3:
		byte = wordVal & 0xFF;
		break;
	case 2:
		byte = (wordVal & 0xFF00)>>8;
		break;
	case 1:
		byte = (wordVal & 0xFF0000)>>16;
		break;
	case 0:
		byte = (wordVal & 0xFF000000)>>24;
		break;
	}

	for (j=0;j<8;j++)
	{
		bitVal = (byte >> j) & 0x0001;

		if (bitVal)
		{
			// Q = Q + R
            GF_Point_Addition256(&LMecc_PointQ256,&LMecc_PointR256,&tmpResultPoint);
			// Copy ResultPoint to PointQ
			GF_Point_Copy256(&tmpResultPoint,&LMecc_PointQ256);
		} 
		// Point Doubling
		// R = 2R
		GF_Point_Addition256(&LMecc_PointR256,&LMecc_PointR256,&tmpResultPoint2);
		// Copy Result point to PointR 
		GF_Point_Copy256(&tmpResultPoint2,&LMecc_PointR256);
	}

	// We Set a Timer - for the next Multiplication of next u_int32

	if (ECC_Point_Mul_Word256 == 1)
	{
		// Call Back a function which will store the ECC calculation result and 
		// flag that the calculation is complete.

		LMecc_CB_ECC_Point_Multiplication_Complete256(p_link);
	}
	else
	{ 
	    ECC_Point_Mul_Word256--;

		if (!blocking)
			LMtmr_Set_Timer(4,ECC_Point_Multiplication_uint8_non_blocking256, p_link, 1);
	}

}


void ECC_Point_Multiplication_uint8_non_blocking256(t_lmp_link* p_link )
{
	ECC_Point_Multiplication_uint8_256(p_link,FALSE);
}

__INLINE__ void GF_Point_Copy256(const ECC_Point256 *source,ECC_Point256 *destination)
{
	copyBigHex256(&source->x,&destination->x);
	copyBigHex256(&source->y,&destination->y);
}
/**************************************************************
 *  Function :- bigHexInversion
 *
 *  Parameters :-  bigHexA - a pointer to bigHex - the input
 *                 pResult - a pointer to bigHex - the output
 *
 *  Description 
 *  This function performs the inversion of a bigHex number. The output multiplied
 *  by the input will result in '1', as the output is Moded by P.
 *
 *            ( bigHexA * pResult ) % P = 1.
 *
 * Ref " Software Implementation of the NIST Elliptical " 
 *  Input : Prime P (bigHexP), a (bigHexA) E [1, p-1]
 *  Output : a^-1 mod P
 ************************************************************************/

void bigHexInversion256( bigHex256* bigHexA,bigHex256* pResult)
{
	bigHex256 u;
	bigHex256 v;
	bigHex256 A;
	bigHex256 C;

	// Change the sign to positive
	bigHexA->sign = 0;

	// Step 1 
	// u <-- a, v <-- p, A <-- 1, C <-- 0.
	//
	initBigNumber256(&A);
	initBigNumber256(&C);

	copyBigHex256(bigHexA,&u);
	copyBigHex256(&bigHexP256,&v);

	A.num[HIGHEST_INDEX_BIG_HEX256] = 1;
	C.num[HIGHEST_INDEX_BIG_HEX256] = 0;
	A.len = 1;
	C.len = 1;

    //
	// Step 2.
    //   While u != 0 do
	//   2.1  While u is even do :
	//           u <-- u/2. If A is even then A <-- A/2; else A <-- (A+P)/2
	//   2.2  While v is even do :
	//           v <-- v/2. If C is even then C <-- C/2; else C <-- (C+P)/2
	//   2.3  If u >= v then : u <-- u - v, A <-- A - C; else v <-- v - u, C <-- C - A
	//
	while(u.len!=0)
	{
		while(ISBIGHEXEVEN256(u))
		{
			divideByTwo256(&u);
			if (ISBIGHEXEVEN256(A))
			{
				// A = A/2
                divideByTwo256(&A);
			}
			else
			{
                AddPdiv2_256(&A);
			}
		}
		while(ISBIGHEXEVEN256(v))
		{
			divideByTwo256(&v);
			if (ISBIGHEXEVEN256(C))
			{
                divideByTwo256(&C);
			}
			else
			{
                AddPdiv2_256(&C);
			}
		}
		if (isGreaterThanOrEqual256(&u,&v))
		{
			SubtractFromSelfBigHex256(&u,&v);
			SubtractFromSelfBigHexSign256(&A,&C);
		}
		else
		{
			SubtractFromSelfBigHex256(&v,&u);
			SubtractFromSelfBigHexSign256(&C,&A);
		}
	}
	// Step 3 :- Now perform Mod P
	// pResult = C % P 
	{
		// If the Mod P is greater than bigHexA then return with
		// C unmodified.

		if (C.sign == 0)
		{
			if (isGreaterThan256(&bigHexP256,&C))
			{
				copyBigHex256(&C,pResult);
				return;
			}
			else // Positive Number thus subtract P iteratively.
			{
				specialModP256(&C);
			}
		}
		else // Negative Number 
		{
			specialModP256(&C);
		}
		copyBigHex256(&C,pResult);
	}
}

/*******************************************************************
 * Funcion :- divideByTwo
 *
 * Parameters :- A - a bigHex pointer - which is the divided by two
 *               
 *******************************************************************/

void divideByTwo256(bigHex256* A)
{
    u_int32 rem=0;
	u64 u64val;
    u_int32 i;

	for(i=0; i < (ELEMENTS_BIG_HEX256); i++)
	{
		u64val = (u64)((u64)(A->num[i]) + ((u64)rem * _LL(0x100000000)));

		A->num[i] = (u_int32)(u64val>>1);
        rem = (u64)(u64val - ((u64val>>1)<<1));
	}
	// We need to re-calculate the length. 
    setBigNumberLength256(A);
}

void LMecc_Generate_ECC_Key256(const u_int8* secret_key, const u_int8* public_key_x,
					   const u_int8* public_key_y,t_lmp_link* p_link,boolean blocking)
{
	u_int32 big_num_offset=1;
	u_int32 i;

	// Now Copy the Public Key and Secret Key coordinates to ECC point format.
	PrivateKey256.num[0] = 0;
	PublicKey256.x.num[0] = 0;
	PublicKey256.y.num[0] = 0;

	for (i=0;i<32;)
	{
		PrivateKey256.num[(i/4)+big_num_offset] = (u_int32)
		                                            (((*(secret_key+i    )) << 24) & 0xFF000000) +
			                                        (((*(secret_key+i+1  )) << 16) & 0x00FF0000) +
			                                        (((*(secret_key+i+2  )) <<  8) & 0x0000FF00) +
													(((*(secret_key+i+3  ))        & 0x000000FF));

		PublicKey256.x.num[(i/4)+big_num_offset] = (u_int32)
		                                          ( (((*(public_key_x+i    )) << 24) & 0xFF000000) +
			                                        (((*(public_key_x+i+1  )) << 16) & 0x00FF0000) +
			                                        (((*(public_key_x+i+2  )) <<  8) & 0x0000FF00) +
													(((*(public_key_x+i+3  )))        & 0x000000FF));

		PublicKey256.y.num[(i/4)+big_num_offset] = (u_int32)
		                                          ( (((*(public_key_y+i    )) << 24) & 0xFF000000) +
			                                        (((*(public_key_y+i+1  )) << 16) & 0x00FF0000) +
			                                        (((*(public_key_y+i+2  )) <<  8) & 0x0000FF00) +
													(((*(public_key_y+i+3  )))        & 0x000000FF));
		i += 4;
	}
	setBigNumberLength256(&PrivateKey256);
	setBigNumberLength256(&PublicKey256.x);
	setBigNumberLength256(&PublicKey256.y);
	PublicKey256.x.sign = 0;
	PublicKey256.y.sign = 0;

	ECC_Point_Multiplication256(&PrivateKey256,&PublicKey256,p_link,blocking);
}

// For ARM build below lines return the system to default build optimisation
//#pragma Ospace
//#pragma O0

#else

#endif
