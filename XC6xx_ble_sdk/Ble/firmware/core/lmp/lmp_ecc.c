/**************************************************************************
 * MODULE NAME:    lmp_ecc.c       
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    LMP Handling for ECC.
 * AUTHOR:         Gary Fleming
 * DATE:           02 Feb 2009
 *
 * SOURCE CONTROL: $Id: lmp_ecc.c,v 1.20 2013/06/05 13:03:29 garyf Exp $
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
#include "lmp_ssp.h" 
//#include "lmp_features.h"
#include <string.h>                     /* For memset */ 
    
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)

// Turn on Below for 'best' ARM optimisation of this file.
//#pragma Otime
//#pragma O2

 const u_int8 BasePoint_x[24] = { 0x18,0x8d,0xa8,0x0e,0xb0,0x30,0x90,0xf6,0x7c,0xbf,0x20,0xeb,
						   0x43,0xa1,0x88,0x00,0xf4,0xff,0x0a,0xfd,0x82,0xff,0x10,0x12};

 const u_int8 BasePoint_y[24] = { 0x07,0x19,0x2b,0x95,0xff,0xc8,0xda,0x78,0x63,0x10,0x11,0xed, 
						   0x6b,0x24,0xcd,0xd5,0x73,0xf9,0x77,0xa1,0x1e,0x79,0x48,0x11};

 const u_int8 maxSecretKey[24] = { 0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                   0xcc,0xef,0x7c,0x1b,0x0a,0x35,0xe4,0xd8,0xda,0x69,0x14,0x18};

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

 const bigHex bigHexP = {{ 0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFE,0xFFFFFFFF,0xFFFFFFFF},
					6,0};

 const veryBigHex veryBigHexP = {{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
      0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFE,0xFFFFFFFF,0xFFFFFFFF},
					6,0};

//#define BIT16 0
#define MAX_DIGITS 56
#define ELEMENTS_BIG_HEX 7 // MAX_OCTETS/4
#define HIGHEST_INDEX_BIG_HEX  6// (MAX_OCTETS/4 )- 1
#define ISBIGHEXEVEN(x) (!((x.num[HIGHEST_INDEX_BIG_HEX]) & 0x01))

#define GETMOSTSIGNIGICANTU32(tmpHexA)  tmpHexA->num[ELEMENTS_BIG_HEX-tmpHexA->len]	

static bigHex PrivateKey;
static ECC_Point PublicKey;
//static ECC_Point ResultPoint;
static u_int32 ECC_Point_Mul_Word = 0;
static ECC_Point LMecc_PointP;
 ECC_Point LMecc_PointQ;
static ECC_Point LMecc_PointR;
static bigHex LMecc_Pk;

void AddBigHex(const bigHex *bigHexA,const bigHex *bigHexB, bigHex *BigHexResult);
void SubtractBigHex( const bigHex *bigHexA, const bigHex *bigHexB, bigHex *BigHexResult);
void MultiplyBigHex( const bigHex *bigHexA, const bigHex *bigHexB, veryBigHex *BigHexResult);
void bigHexInversion( bigHex* bigHexA, bigHex* pResult);
void Add2SelfBigHex( bigHex *bigHexA,const bigHex *bigHexB);
void SubtractFromSelfBigHexSign( bigHex *bigHexA, const bigHex *bigHexB);
__INLINE__ void initBigNumber(bigHex *BigHexResult);
// Not Used
//__INLINE__ void initVeryBigNumber(veryBigHex *BigHexResult);

__INLINE__ void setBigNumberLength( bigHex *BigHexResult);
__INLINE__ void setVeryBigNumberLength(veryBigHex *BigHexResult);

__INLINE__ void copyBigHex(const bigHex *source,bigHex *destination);
__INLINE__ void copyVeryBigHex(const veryBigHex *source,veryBigHex *destination);

__INLINE__ void shiftLeftOneArrayElement(bigHex *input);
void MultiplyBigHexModP(const bigHex *bigHexA,const bigHex *bigHexB, bigHex *BigHexResult);

__INLINE__ void divideByTwo(bigHex* A);

__INLINE__ int isbigHexEven(const bigHex* A);


__INLINE__ int isGreaterThan(const bigHex *bigHexA,const bigHex *bigHexB);
__INLINE__ int isGreaterThanOrEqual(const bigHex *bigHexA,const bigHex *bigHexB);
__INLINE__ int isVeryBigHexGreaterThanOrEqual(const veryBigHex *bigHexA,const veryBigHex *bigHexB);


void MultiplyByU32ModP(const u_int32 N,bigHex* result);
void MultiplyByU32ModPInv(const u_int32 N,bigHex* result);
void specialModP(bigHex *tmpHexA);
void AddPdiv2(bigHex *bigHexA);
void AddP(bigHex *bigHexA);
// Galois Field Operands
void GF_Point_Addition(const ECC_Point* PointP,const ECC_Point* PointQ, ECC_Point* ResultPoint);
void LMecc_CB_ECC_Point_Multiplication_Complete(t_lmp_link* p_link);
void ECC_Point_Multiplication_uint32(t_lmp_link* p_link);
void ECC_Point_Multiplication_uint8(t_lmp_link* p_link,boolean blocking);
void ECC_Point_Multiplication_uint8_non_blocking(t_lmp_link* p_link );
int LMecc_isValidSecretKey(u_int8* secretKey)
{
	// First check that the secret key is not all zeros. 
	//
	int i;

	for(i=0;i<24;i++)
	{
		if (secretKey[i] != 0)
			break;
	}

	if ((i==24) && (secretKey[23] == 0))
		return 0;

	for(i = 0; i < 24; i++)
	{
		if (secretKey[i] > maxSecretKey[i])
			return 0;
		else if (secretKey[i] < maxSecretKey[i])
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
void AddBigHex( const bigHex *bigHexA, const bigHex *bigHexB, bigHex *BigHexResult)
{
	u64 tmp;
	u_int32 carry = 0;
	s_int32 i;
    const u_int32 *numA = bigHexA->num;
    const u_int32 *numB = bigHexB->num;
	u_int32 *result = BigHexResult->num;  

	for (i=(ELEMENTS_BIG_HEX-1);i >= 0; i--)
	{
		tmp = (u64)((u64)(numA[i]) + (u64)(numB[i]) + carry);
		if(tmp & _LL(0x100000000))
			carry = 0x01;
		else
			carry = 0x00;

		result[i] = (tmp & 0xFFFFFFFF);
	}
	setBigNumberLength(BigHexResult);
}

// A = A + B
void Add2SelfBigHex( bigHex *bigHexA,const bigHex *bigHexB)
{
	u64 tmp;
	u_int32 carry = 0;
	s_int32 i;
	u_int32 *numA = bigHexA->num;
	const u_int32 *numB = bigHexB->num;

	for (i=(ELEMENTS_BIG_HEX-1);i >= 0; i--)
	{
		tmp = (u64)((u64)(numA[i]) + (u64)(numB[i]) + carry);
		if(tmp & _LL(0x100000000))
			carry = 0x01;
		else
			carry = 0x00;

		numA[i] = (tmp & 0xFFFFFFFF);
	}
	setBigNumberLength(bigHexA);
}

void SubtractBigHex(const bigHex *bigHexA, const bigHex *bigHexB, bigHex *BigHexResult)
{
    // Due to the nature of the GF we have to assume both inputs are +ive.
	u_int32 borrow=0;
	s_int32 i;
	const u_int32* pBigNum = bigHexA->num;
	const u_int32* pSmallNum = bigHexB->num;
	u_int32* pResult = BigHexResult->num;

	for (i=(ELEMENTS_BIG_HEX-1);i >= 0; i--)
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
    setBigNumberLength(BigHexResult);
}

// A = A - B

void SubtractFromSelfBigHex(bigHex *bigHexA,const bigHex *bigHexB)
{
    // Due to the nature of the GF we have to assume both inputs are +ive.
	u_int32 borrow=0;
	s_int32 i;
	u_int32* pBigNum = bigHexA->num;
	const u_int32* pSmallNum = bigHexB->num;

	for (i=(ELEMENTS_BIG_HEX-1);i >= 0; i--)
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
    setBigNumberLength(bigHexA);
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
void AddP( bigHex *bigHexA)
{
	bigHex BigHexResult;

	SubtractBigHex(&bigHexP, bigHexA,&BigHexResult);
	copyBigHex(&BigHexResult,bigHexA);
	bigHexA->sign = 0;
}

void AddPdiv2( bigHex *bigHexA)
{
	//
	u_int32 *numA = bigHexA->num;
	const u_int32 *numB = bigHexP.num;

	if (bigHexA->sign == 0)
	{
		u64 tmp;
		u_int32 carry;
		s_int32 i;

		carry = 0;

		for (i=(ELEMENTS_BIG_HEX-1);i >= 0; i--)
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
		if ( isGreaterThan(bigHexA, &bigHexP))
		{

			// Due to the nature of the GF we have to assume both inputs are +ive.
			u_int32 borrow=0;
			s_int32 i;

			for (i=(ELEMENTS_BIG_HEX-1);i >= 0; i--)
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
			bigHex BigHexResult;

			SubtractBigHex(&bigHexP,bigHexA,&BigHexResult);
			copyBigHex(&BigHexResult,bigHexA);
			bigHexA->sign = 0;
		}
	}
	// divide by 2
	// divideByTwo(bigHexA);
	{
		u_int32 rem=0;
		u64 u64val;
		u_int32 i;

		for(i=0; i < (ELEMENTS_BIG_HEX); i++)
		{
			u64val = (u64)((u64)(numA[i]) + ((u64)rem * _LL(0x100000000)));

			numA[i] = (u_int32)(u64val>>1);
			rem = (u64)(u64val - ((u64val>>1)<<1));
		}

		// We need to re-calculate the length. 
		setBigNumberLength(bigHexA);
	}
}
void SubtractFromSelfBigHexSign( bigHex *bigHexA, const bigHex *bigHexB)
{
	// This function uses use the basis AddBigHex and SubtractBigHex to implement a full
	// Addition which considers sign and mod.
	// 
	if (bigHexA->sign == 0)
	{
		if (bigHexB->sign == 0)
		{
			if ( isGreaterThanOrEqual(bigHexA,bigHexB))
			{
				SubtractFromSelfBigHex(bigHexA, bigHexB);
			}
			else
			{
				bigHex BigHexResult;

				SubtractBigHex(bigHexB, bigHexA,&BigHexResult);
				copyBigHex(&BigHexResult,bigHexA);
				bigHexA->sign = 1;
			}
		}
		else 
		{
			// 3/  if A is + and B is -   C =   A+B  Mod
			Add2SelfBigHex(bigHexA, bigHexB);
		}
	}
	else  // if (bigHexA->sign == 1)
	{
		if (bigHexB->sign == 0)
		{
			Add2SelfBigHex(bigHexA, bigHexB);
		}
		else
		{
			if ( isGreaterThanOrEqual(bigHexB,bigHexA))
			{
				bigHex BigHexResult;

				SubtractBigHex(bigHexB, bigHexA,&BigHexResult);
				copyBigHex(&BigHexResult,bigHexA);
				bigHexA->sign = 0;
			}
			else
			{
				SubtractFromSelfBigHex(bigHexA, bigHexB);
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
void SubtractBigHexMod(const bigHex *bigHexA, const bigHex *bigHexB, bigHex *BigHexResult)
{
	if (bigHexA->sign == 0)
	{
		if (bigHexB->sign == 0)
		{
			if ( isGreaterThanOrEqual(bigHexA,bigHexB))
			{
				SubtractBigHex(bigHexA, bigHexB,BigHexResult);
				BigHexResult->sign = 0;
			}
			else
			{
				SubtractBigHex(bigHexB, bigHexA,BigHexResult);
				BigHexResult->sign = 1;
			}
		}
		else 
		{
			// 3/  if A is + and B is -   C =   A+B  Mod
			AddBigHex(bigHexA, bigHexB,BigHexResult);
			BigHexResult->sign = 0;
		}
	}
	else  // if (bigHexA->sign == 1)
	{
		if (bigHexB->sign == 0)
		{
			AddBigHex(bigHexA, bigHexB,BigHexResult);
			BigHexResult->sign = 1;
		}
		else
		{
			if ( isGreaterThanOrEqual(bigHexB,bigHexA))
			{
				SubtractBigHex(bigHexB, bigHexA,BigHexResult);
				BigHexResult->sign = 0;
			}
			else
			{
				SubtractBigHex(bigHexA, bigHexB,BigHexResult);
				BigHexResult->sign = 1;
			}
		}
	}

	if (BigHexResult->sign == 0x00)   // Positive Number
	{
		if(isGreaterThanOrEqual(BigHexResult,&bigHexP))
	    { 
			//SubtractFromSelfBigHexSign(BigHexResult,&bigHexP);
			SubtractFromSelfBigHex(BigHexResult, &bigHexP);
		}
	}	
	else
	{
		AddP(BigHexResult);
	}
}

// From curren t usage we can safely assume both inputs are positive.

void SubtractBigHexUint32(const bigHex *bigHexA, const u_int32 numB, bigHex *BigHexResult)
{
	initBigNumber(BigHexResult);

	if (bigHexA->num[HIGHEST_INDEX_BIG_HEX] >= numB)
	{
		copyBigHex(bigHexA,BigHexResult);
		BigHexResult->num[ELEMENTS_BIG_HEX-1] = bigHexA->num[ELEMENTS_BIG_HEX-1]-numB;
	}
	else
	{
		bigHex N;

		initBigNumber(&N);

		N.len = 1;
		N.num[HIGHEST_INDEX_BIG_HEX] = numB;
		SubtractBigHexMod(bigHexA,&N,BigHexResult);
	}
}


void MultiplyBigHexModP(const bigHex *bigHexA, const bigHex *bigHexB, bigHex *BigHexResult)
{
	veryBigHex tmpResult;
	{
		s_int32 k;
		const u_int32 *numA = bigHexA->num;
		const u_int32 *numB = bigHexB->num;
		u_int32 *result = tmpResult.num;

		memset(tmpResult.num,0,56);
		tmpResult.len = 0;
		tmpResult.sign = 0;
		//
		// Below Optimisations seem to degrade performance on Windows by a factor of 2.
		//
		for(k=(ELEMENTS_BIG_HEX-1);k >= 0;k--)
		{
			u_int32 mcarry = 0;
			s_int32 j = 0;

			// The inner loop multiplies each byte of HexB by a single byte of 
			// HexA
			for(j=(ELEMENTS_BIG_HEX-1);j >=0;j--)
			{
				u64 val;

				val = (((u64)(numA[k]) ) * ((u64)(numB[j]))) + result[j+k+1] + mcarry;
				result[j+k+1] = (val & 0xFFFFFFFF);
				mcarry = (u_int32)( val >> 32);
			}
		}
		setVeryBigNumberLength(&tmpResult);
		tmpResult.sign = (bigHexA->sign != bigHexB->sign);
	}

	{
		u_int32 i=0;
		bigHex tmpResult2;

		while ((tmpResult.num[i] == 0x00) && (i<(ELEMENTS_BIG_HEX)))
			i++;

		// Take the next 13 elements 
		// 
		tmpResult2.sign = tmpResult.sign;

		if (isVeryBigHexGreaterThanOrEqual(&tmpResult,&veryBigHexP))
		{
			while(tmpResult.num[i] == 0x00)
				i++;
		}
		memcpy(tmpResult2.num,tmpResult.num+i,28);
		setBigNumberLength(&tmpResult2);

		while((i+ELEMENTS_BIG_HEX) < (MAX_OCTETS/2))
		{
			specialModP(&tmpResult2);
			shiftLeftOneArrayElement(&tmpResult2);

			// Add the next byte from A in left_over;
			tmpResult2.num[HIGHEST_INDEX_BIG_HEX] = tmpResult.num[i+ELEMENTS_BIG_HEX];
			i++;
			setBigNumberLength(&tmpResult2);
		}
		specialModP(&tmpResult2);
		copyBigHex(&tmpResult2,BigHexResult);
	}
}


void MultiplyBigHexByUint32(const bigHex *bigHexA, const u_int32 numB, bigHex *BigHexResult)
{
	s_int32 k;
	const u_int32 *numA = bigHexA->num;
	u_int32 *result = BigHexResult->num;
	u_int32 mcarry = 0;

	//
	// Below Optimisations seem to degrade performance on Windows by a factor of 2.
	//
	for(k=HIGHEST_INDEX_BIG_HEX;k >= 0;k--)
	{
		u64 val;

		val = (((u64)(numA[k]) ) * ((u64)numB)) + mcarry;
		result[k] = (val & 0xFFFFFFFF);
		mcarry = (u_int32)( val >> 32);
	}
	setBigNumberLength(BigHexResult);
	BigHexResult->sign = bigHexA->sign;
	specialModP(BigHexResult);
}


__INLINE__ void shiftLeftOneArrayElement(bigHex *input)
{
	memcpy(input->num,(input->num+1),24);
}

//
// This function adds N * Mod P to a number.
//  
// The left_over = tmpHexA % P

void specialModP(bigHex *tmpHexA)
{
   bigHex NModPinv;
   u_int32 SignificantU32;

   initBigNumber(&NModPinv);
   if(((tmpHexA->sign == 0) && isGreaterThanOrEqual(tmpHexA,&bigHexP)) ||
	   (tmpHexA->sign == 1))
   {
	   // If I multiply the most significant u_int16 of A by the Mod P and then subtract from A 
	   // this is equivalent to an iteractive subtraction - but much faster.
	   if (tmpHexA->len > bigHexP.len)
	   {
		   SignificantU32 = GETMOSTSIGNIGICANTU32(tmpHexA);
           MultiplyByU32ModPInv(SignificantU32,&NModPinv);
		   tmpHexA->num[0] = 0x00;

		   if (tmpHexA->sign == 0)
		   {
				Add2SelfBigHex(tmpHexA,&NModPinv);
		   }
		   else
		   {
				SubtractFromSelfBigHexSign(tmpHexA,&NModPinv);
		   }
	   }
	   if (((tmpHexA->sign == 0) && isGreaterThanOrEqual(tmpHexA,&bigHexP)) ||
			(tmpHexA->sign == 1))
	   {
		 // It is implicit that the sign is negative. 
	     // so we can remove the check.
		 //
		   if (tmpHexA->sign == 1)
		   {
			   AddP(tmpHexA);
		   }
		   else
		   {
			   // Can this be replaced by Adding the invert of P
			   SubtractFromSelfBigHex(tmpHexA, &bigHexP);
		   }
	   }
   }
}

void MultiplyByU32ModPInv(const u_int32 N,bigHex* result)
{
	u64 tempVal;

	tempVal = (u64)((N << 1) & _LL(0x1FFFFFFFE));

    result->num[6] = N;
	result->num[4] = N; 
	result->num[3] = (u_int32) ((tempVal>>32) & 0xF);

	result->sign = 0x00;
	if (result->num[3])
		result->len = 0x4;
	else 
		result->len = 0x3;
}

__INLINE__ void copyBigHex(const bigHex *source,bigHex *destination)
{
	memcpy(destination->num,source->num,28);

	destination->len = source->len;
	destination->sign = source->sign;
}

__INLINE__ void initBigNumber(bigHex *BigHexResult)
{
	memset(BigHexResult->num,0,28);

	BigHexResult->len = 0;
	BigHexResult->sign = 0;
}

__INLINE__ void setBigNumberLength(bigHex *BigHexResult)
{
	int i;

	for(i=0;i<(ELEMENTS_BIG_HEX);i++)
	{
		if(BigHexResult->num[i] != 0x00)
           break;
	}
	BigHexResult->len = (ELEMENTS_BIG_HEX)-i;
}

__INLINE__ void setVeryBigNumberLength(veryBigHex *BigHexResult)
{
	int i;

	for(i=0;i<(MAX_OCTETS/2);i++)
	{
		if(BigHexResult->num[i] != 0x00)
           break;
	}
	BigHexResult->len = (MAX_OCTETS/2)-i;
}

//
// if A > B return TRUE
//
  
__INLINE__ int isGreaterThan(const bigHex *bigHexA,const bigHex *bigHexB)
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
	for (i=((ELEMENTS_BIG_HEX)-A_len); i <(ELEMENTS_BIG_HEX);i++)
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


__INLINE__ int isGreaterThanOrEqual(const bigHex *bigHexA,const bigHex *bigHexB)
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
    
	for (i=((ELEMENTS_BIG_HEX)-A_len); i <(ELEMENTS_BIG_HEX);i++)
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


__INLINE__ int isVeryBigHexGreaterThanOrEqual(const veryBigHex *bigHexA,const veryBigHex *bigHexB)
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
	for (i=((MAX_OCTETS/2)-A_len); i <(MAX_OCTETS/2);i++)
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
__INLINE__ int notEqual(const bigHex *bigHexA,const bigHex *bigHexB)
{
	u_int32 i;

	for(i=0;i<(ELEMENTS_BIG_HEX);i++)
	{
		if(bigHexA->num[i] != bigHexB->num[i])
		{
			return 1;
		}
	}
	return 0;
}

//isLessThan
__INLINE__ int isLessThan(const bigHex *bigHexA,const bigHex *bigHexB)
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

void GF_Point_Addition(const ECC_Point* pPointP,const ECC_Point* pPointQ, ECC_Point* pResultPoint)
{
	// Gamma or S = ((y1-y2)/(x1-x2))
    // What is a  = -3 
	// x3 = (Gamma*Gamma) - x1 - x2
	// y3 = -y1 + Gamma*(x1 - x3)
	//see 115-EllipticCurveCryptography 
	 
	// Need to advance all GF operands so that they return a result in the field  -- Mod P
	bigHex gamma;
	bigHex tempResult;
	bigHex DiffX_coords;

	if (pPointP->y.len == 0)
	{
		if (pPointP->x.len == 0)
		{
			// Let the result be PointQ
			GF_Point_Copy(pPointQ,pResultPoint);
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
        GF_Point_Copy(pPointP,pResultPoint);
		return;
	}

	if (notEqual(&(pPointP->x),&(pPointQ->x)))
	{
		bigHex tempResult4;
		bigHex DiffY_coords;

		// Determine Gamma = S = (yP - yQ)/(xP - xQ)
		SubtractBigHexMod(&(pPointP->y),&(pPointQ->y),&DiffY_coords);
		SubtractBigHexMod(&(pPointP->x),&(pPointQ->x),&DiffX_coords);
		bigHexInversion(&DiffX_coords,&tempResult4);
		MultiplyBigHexModP(&tempResult4,&DiffY_coords,&gamma);
	}
	else
	{
		bigHex xP_squared;
		bigHex tempResult4;
		bigHex tempResult2;

		// S = (3xP^2 + A)/2yP     A = -3
		// xP^2
		// !!! Scope for Optimisation of the Function below into 
		// a variant specifically for squaring.

		MultiplyBigHexModP(&(pPointP->x),&(pPointP->x),&xP_squared);
		// 3 * xP^2
		MultiplyBigHexByUint32(&xP_squared,0x03,&tempResult);
        // 3xP^2 - 3
		SubtractBigHexUint32(&tempResult,0x3,&tempResult2);

		MultiplyBigHexByUint32(&(pPointP->y),0x02,&tempResult);
		bigHexInversion(&tempResult,&tempResult4);

		MultiplyBigHexModP(&tempResult4,&tempResult2,&gamma);
	}
	{
		bigHex GammaSquared;

		// Determine x3 = S^2 -xP -xQ
		MultiplyBigHexModP(&gamma,&gamma,&GammaSquared);
		SubtractBigHexMod(&GammaSquared,&pPointP->x,&tempResult);
		SubtractBigHexMod(&tempResult,&pPointQ->x,&pResultPoint->x);
	}
	// Determine y3 = S(xP - x3) - yP

	SubtractBigHexMod(&pPointP->x,&pResultPoint->x, &DiffX_coords);	
    MultiplyBigHexModP(&DiffX_coords,&gamma,&tempResult);
    SubtractBigHexMod(&tempResult,&pPointP->y,&pResultPoint->y);

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
 *   ?Require k=(km-1,km-2,?k0)2, km=1
 *   ?Compute Q=kP
 * ?Q=0, R=P
 * ?For i=0 to m-1
 *   ?If ki=1 then
 *      ?Q=Q+R
 *   ?End if
 *   ?R=2R
 * ?End for
 * ?Return Q
 ******************************************************************************************/
void ECC_Point_Multiplication(const bigHex* pk,const ECC_Point* pPointP,t_lmp_link* p_link,boolean blocking)
{
	// see ecc.pdf
	//

	int i;

	GF_Point_Copy(pPointP,&LMecc_PointP);
	GF_Point_Copy(pPointP,&LMecc_PointR);
	copyBigHex(pk,&LMecc_Pk);

	initBigNumber(&LMecc_PointQ.x);
	initBigNumber(&LMecc_PointQ.y);

	//ECC_Point_Mul_Word = ELEMENTS_BIG_HEX;
	ECC_Point_Mul_Word = ELEMENTS_BIG_HEX*4;
   // ECC_Point_Multiplication_uint32(p_link);

	if (blocking)
	{
		for(i = ECC_Point_Mul_Word; i > 0; i --)
			ECC_Point_Multiplication_uint8(p_link,blocking);
	}
	else
		ECC_Point_Multiplication_uint8(p_link,blocking);

}

void LMecc_CB_ECC_Point_Multiplication_Complete(t_lmp_link* p_link)
{
	u_int32 big_num_offset;
	u_int32 i,j=0;

	// Copy the secret_key to a bigHex format.
	big_num_offset = 1;

	if (p_link)
	{
		for(i=0;i<24;)
		{
			p_link->DHkey[i] =   ((LMecc_PointQ.x.num[j+big_num_offset] & 0xFF000000) >> 24);
			p_link->DHkey[i+1] = ((LMecc_PointQ.x.num[j+big_num_offset] & 0xFF0000) >> 16);
			p_link->DHkey[i+2] = ((LMecc_PointQ.x.num[j+big_num_offset] & 0xFF00) >> 8);
			p_link->DHkey[i+3] = ( LMecc_PointQ.x.num[j+big_num_offset] & 0xFF);

			i+=4;
			j++;
		}

		// Need to invoke logic in the SSP protocol in case this calculation complete comes after the 
		// User_Confirmation_Request_Reply.

		LMssp_CallBack_DH_Key_Complete(p_link);
	}
	else // p_link == 0  then local device.
	{
		for(i=0;i<24;)
		{
			g_LM_config_info.public_key_x[i] =   ((LMecc_PointQ.x.num[j+big_num_offset] & 0xFF000000) >> 24);
			g_LM_config_info.public_key_x[i+1] = ((LMecc_PointQ.x.num[j+big_num_offset] & 0xFF0000) >> 16);
			g_LM_config_info.public_key_x[i+2] = ((LMecc_PointQ.x.num[j+big_num_offset] & 0xFF00) >> 8);
			g_LM_config_info.public_key_x[i+3] = ( LMecc_PointQ.x.num[j+big_num_offset] & 0xFF);

			g_LM_config_info.public_key_y[i] =   ((LMecc_PointQ.y.num[j+big_num_offset] & 0xFF000000) >> 24);
			g_LM_config_info.public_key_y[i+1] = ((LMecc_PointQ.y.num[j+big_num_offset] & 0xFF0000) >> 16);
			g_LM_config_info.public_key_y[i+2] = ((LMecc_PointQ.y.num[j+big_num_offset] & 0xFF00) >> 8);
			g_LM_config_info.public_key_y[i+3] = ( LMecc_PointQ.y.num[j+big_num_offset] & 0xFF);

			i+=4;
			j++;
		}
#if 1 // LC FIX FOR SSP- Blocking
		g_LM_config_info.new_public_key_generation = 0x02; // Key Generation Complete
#endif
	}
}


void ECC_Point_Multiplication_uint32(t_lmp_link* p_link)
{
	ECC_Point tmpResultPoint;
	ECC_Point tmpResultPoint2;

	u_int32 j;
	u_int32 bitVal;

	for (j=0;j<32;j++)
	{
		bitVal = (LMecc_Pk.num[ECC_Point_Mul_Word-1] >> j) & 0x0001;

		if (bitVal)
		{
			// Q = Q + R
            GF_Point_Addition(&LMecc_PointQ,&LMecc_PointR,&tmpResultPoint);
			// Copy ResultPoint to PointQ
			GF_Point_Copy(&tmpResultPoint,&LMecc_PointQ);
		} 
		// Point Doubling
		// R = 2R
		GF_Point_Addition(&LMecc_PointR,&LMecc_PointR,&tmpResultPoint2);
		// Copy Result point to PointR 
		GF_Point_Copy(&tmpResultPoint2,&LMecc_PointR);
	}

	// We Set a Timer - for the next Multiplication of next u_int32

	if (ECC_Point_Mul_Word == 1)
	{
		// Call Back a function which will store the ECC calculation result and 
		// flag that the calculation is complete.

		LMecc_CB_ECC_Point_Multiplication_Complete(p_link);
	}
	else
	{ 
	    ECC_Point_Mul_Word--;
		LMtmr_Set_Timer(4,ECC_Point_Multiplication_uint32, p_link, 1);
	}

}

void ECC_Point_Multiplication_uint8(t_lmp_link* p_link,boolean blocking)
{
	ECC_Point tmpResultPoint;
	ECC_Point tmpResultPoint2;

	u_int32 j;
	u_int32 bitVal;
	u_int8 byte=0;
	u_int8 byteOffset;
    
	u_int32 wordVal;
		
	wordVal = LMecc_Pk.num[((ECC_Point_Mul_Word+3)/4)-1];
	byteOffset = (ECC_Point_Mul_Word-1)%4;

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
            GF_Point_Addition(&LMecc_PointQ,&LMecc_PointR,&tmpResultPoint);
			// Copy ResultPoint to PointQ
			GF_Point_Copy(&tmpResultPoint,&LMecc_PointQ);
		} 
		// Point Doubling
		// R = 2R
		GF_Point_Addition(&LMecc_PointR,&LMecc_PointR,&tmpResultPoint2);
		// Copy Result point to PointR 
		GF_Point_Copy(&tmpResultPoint2,&LMecc_PointR);
	}

	// We Set a Timer - for the next Multiplication of next u_int32

	if (ECC_Point_Mul_Word == 1)
	{
		// Call Back a function which will store the ECC calculation result and 
		// flag that the calculation is complete.

		LMecc_CB_ECC_Point_Multiplication_Complete(p_link);
	}
	else
	{ 
	    ECC_Point_Mul_Word--;

		if (!blocking)
			LMtmr_Set_Timer(4,ECC_Point_Multiplication_uint8_non_blocking, p_link, 1);
	}

}


void ECC_Point_Multiplication_uint8_non_blocking(t_lmp_link* p_link )
{
	ECC_Point_Multiplication_uint8(p_link,FALSE);
}

__INLINE__ void GF_Point_Copy(const ECC_Point *source,ECC_Point *destination)
{
	copyBigHex(&source->x,&destination->x);
	copyBigHex(&source->y,&destination->y);
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

void bigHexInversion( bigHex* bigHexA,bigHex* pResult)
{
	bigHex u;
	bigHex v;
	bigHex A;
	bigHex C;

	// Change the sign to positive
	bigHexA->sign = 0;

	// Step 1 
	// u <-- a, v <-- p, A <-- 1, C <-- 0.
	//
	initBigNumber(&A);
	initBigNumber(&C);

	copyBigHex(bigHexA,&u);
	copyBigHex(&bigHexP,&v);

	A.num[HIGHEST_INDEX_BIG_HEX] = 1;
	C.num[HIGHEST_INDEX_BIG_HEX] = 0;
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
		while(ISBIGHEXEVEN(u))
		{
			divideByTwo(&u);
			if (ISBIGHEXEVEN(A))
			{
				// A = A/2
                divideByTwo(&A);
			}
			else
			{
                AddPdiv2(&A);
			}
		}
		while(ISBIGHEXEVEN(v))
		{
			divideByTwo(&v);
			if (ISBIGHEXEVEN(C))
			{
                divideByTwo(&C);
			}
			else
			{
                AddPdiv2(&C);
			}
		}
		if (isGreaterThanOrEqual(&u,&v))
		{
			SubtractFromSelfBigHex(&u,&v);
			SubtractFromSelfBigHexSign(&A,&C);
		}
		else
		{
			SubtractFromSelfBigHex(&v,&u);
			SubtractFromSelfBigHexSign(&C,&A);
		}
	}
	// Step 3 :- Now perform Mod P
	// pResult = C % P 
	{
		// If the Mod P is greater than bigHexA then return with
		// C unmodified.

		if (C.sign == 0)
		{
			if (isGreaterThan(&bigHexP,&C))
			{
				copyBigHex(&C,pResult);
				return;
			}
			else // Positive Number thus subtract P iteratively.
			{
				specialModP(&C);
			}
		}
		else // Negative Number 
		{
			specialModP(&C);
		}
		copyBigHex(&C,pResult);
	}
}

/*******************************************************************
 * Funcion :- divideByTwo
 *
 * Parameters :- A - a bigHex pointer - which is the divided by two
 *               
 *******************************************************************/

void divideByTwo(bigHex* A)
{
    u_int32 rem=0;
	u64 u64val;
    u_int32 i;

	for(i=0; i < (ELEMENTS_BIG_HEX); i++)
	{
		u64val = (u64)((u64)(A->num[i]) + ((u64)rem * _LL(0x100000000)));

		A->num[i] = (u_int32)(u64val>>1);
        rem = (u64)(u64val - ((u64val>>1)<<1));
	}
	// We need to re-calculate the length. 
    setBigNumberLength(A);
}

void LMecc_Generate_ECC_Key(const u_int8* secret_key, const u_int8* public_key_x,
					   const u_int8* public_key_y,t_lmp_link* p_link,boolean blocking)
{
	u_int32 big_num_offset=1;
	u_int32 i;

	// Now Copy the Public Key and Secret Key coordinates to ECC point format.
	PrivateKey.num[0] = 0;
	PublicKey.x.num[0] = 0;
	PublicKey.y.num[0] = 0;

	for (i=0;i<24;)
	{
		PrivateKey.num[(i/4)+big_num_offset] = (u_int32)
		                                            (((*(secret_key+i    )) << 24) & 0xFF000000) +
			                                        (((*(secret_key+i+1  )) << 16) & 0x00FF0000) +
			                                        (((*(secret_key+i+2  )) <<  8) & 0x0000FF00) +
													(((*(secret_key+i+3  ))        & 0x000000FF));

		PublicKey.x.num[(i/4)+big_num_offset] = (u_int32)
		                                          ( (((*(public_key_x+i    )) << 24) & 0xFF000000) +
			                                        (((*(public_key_x+i+1  )) << 16) & 0x00FF0000) +
			                                        (((*(public_key_x+i+2  )) <<  8) & 0x0000FF00) +
													(((*(public_key_x+i+3  )))        & 0x000000FF));

		PublicKey.y.num[(i/4)+big_num_offset] = (u_int32)
		                                          ( (((*(public_key_y+i    )) << 24) & 0xFF000000) +
			                                        (((*(public_key_y+i+1  )) << 16) & 0x00FF0000) +
			                                        (((*(public_key_y+i+2  )) <<  8) & 0x0000FF00) +
													(((*(public_key_y+i+3  )))        & 0x000000FF));
		i += 4;
	}
	setBigNumberLength(&PrivateKey);
	setBigNumberLength(&PublicKey.x);
	setBigNumberLength(&PublicKey.y);
	PublicKey.x.sign = 0;
	PublicKey.y.sign = 0;

	ECC_Point_Multiplication(&PrivateKey,&PublicKey,p_link,blocking);
}

// For ARM build below lines return the system to default build optimisation
//#pragma Ospace
//#pragma O0

#else

#endif
