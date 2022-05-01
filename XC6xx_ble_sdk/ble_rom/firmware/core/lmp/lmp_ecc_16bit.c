
#include "lmp_features.h"
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
#include "lmp_ecc.h"

static bigHex bigHexP = {{ 0x0000,0x0000,0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFE,0xFFFF,0xFFFF,0xFFFF,0xFFFF},
					{12},{0}};

static veryBigHex veryBigHexP = {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
                    0x0000,0x0000,0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFE,0xFFFF,0xFFFF,0xFFFF,0xFFFF},
					{12},{0}};

//bigHex bigHexR = {{ 0x0000,0x0000,0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x99DE,0xF836,0x145B,0xC9B1,0xB4D2,0x2831},
//					{12},{0}};


static bigHex PrivateKey;
static ECC_Point PublicKey;
static ECC_Point ResultPoint;

void AddBigHex( bigHex *bigHexA, bigHex *bigHexB, bigHex *BigHexResult);
void SubtractBigHex( bigHex *bigHexA,  bigHex *bigHexB, bigHex *BigHexResult);
void MultiplyBigHex( bigHex *bigHexA,  bigHex *bigHexB, veryBigHex *BigHexResult);
void bigHexInversion(bigHex* bigHexA, bigHex* pResult);
  
void initBigNumber(bigHex *BigHexResult);
void initVeryBigNumber(veryBigHex *BigHexResult);

int getNumberLen(bigHex* bigHexA);
void setBigNumberLength( bigHex *BigHexResult);
void setVeryBigNumberLength(veryBigHex *BigHexResult);

void takeSignificantPart(bigHex *input,u_int8 start_offset,u_int8 end_offset,bigHex* result);

void copyBigHex(bigHex *source,bigHex *destination);
void copyVeryBigHex(veryBigHex *source,veryBigHex *destination);

void shiftLeftOneArrayElement(bigHex *input);
void MultiplyBigHexModP(bigHex *bigHexA,bigHex *bigHexB, bigHex *BigHexResult);

u_int32 GetMostSignificantU16(bigHex *tmpHexA);

void divideByTwo(bigHex* A);

int isbigHexEven(bigHex* A);


int isGreaterThan(bigHex *bigHexA,bigHex *bigHexB);
int isGreaterThanOrEqual(bigHex *bigHexA,bigHex *bigHexB);
int isVeryBigHexGreaterThanOrEqual(veryBigHex *bigHexA,veryBigHex *bigHexB);

void MultiplyByU16ModP(u_int32 N,bigHex* result);
void bigHexModP(bigHex* bigHexA,bigHex* pResult);
void veryBigHexModP(veryBigHex* tmpResult,bigHex* BigHexResult);
void specialModP(u_int32 *num_times,bigHex *left_over,bigHex *tmpHexA);
  
// Galois Field Operands
void GF_Point_Double(ECC_Point *PointP,ECC_Point *ResultPoint);
void GF_Point_Addition(ECC_Point* PointP,ECC_Point* PointQ, ECC_Point* ResultPoint);

void ECC_Point_Multiplication(bigHex* pk,ECC_Point* pPointP, ECC_Point* pPointQ);

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
void AddBigHex( bigHex *bigHexA, bigHex *bigHexB, bigHex *BigHexResult)
{
	u_int32 tmp;
	u_int32 carry;
	u_int8 i;

	carry = 0;

	initBigNumber(BigHexResult);

	for (i=(MAX_OCTETS/2);i > 0; i--)
	{
		tmp = (bigHexA->num[i-1] + bigHexB->num[i-1] + carry);
		if(tmp & 0x10000)
			carry = 0x01;
		else
			carry = 0x00;

		BigHexResult->num[i-1] = (tmp & 0xFFFF);
	}
	setBigNumberLength(BigHexResult);
	BigHexResult->sign = 0;
}


void SubtractBigHex(bigHex *bigHexA,bigHex *bigHexB, bigHex *BigHexResult)
{
    // Due to the nature of the GF we have to assume both inputs are +ive.

	u_int8 borrow=0;
	int i;
	bigHex* pBigNum;
	bigHex* pSmallNum;

	initBigNumber(BigHexResult);

    pBigNum = bigHexA;
	pSmallNum = bigHexB;

	for (i=(MAX_OCTETS/2);i > 0; i--)
	{
 		if ((pSmallNum->num[i-1]+borrow) > pBigNum->num[i-1])
		{
			BigHexResult->num[i-1] = ((pBigNum->num[i-1] + 0x10000)- (pSmallNum->num[i-1]+borrow)) & 0xFFFF;
			borrow = 0x01;
		}
		else
		{
			BigHexResult->num[i-1] = ((pBigNum->num[i-1])- (pSmallNum->num[i-1]+borrow)) & 0xFFFF;
			borrow = 0x00;
		}
	}
    setBigNumberLength(BigHexResult);
 	BigHexResult->sign = 0;
}


//
// This function multiplies two 15 byte numbers to provide a 30 byte number.
// Both A and B are +ive. 
// 
void MultiplyBigHex(bigHex *bigHexA,bigHex *bigHexB, veryBigHex *BigHexResult)
{
	u_int8 i;

	initVeryBigNumber(BigHexResult);

	for(i=(MAX_OCTETS/2);i > 0;i--)
	{
		u_int16 mcarry = 0;
		u_int8 j = 0;

        // The inner loop multiplies each byte of HexB by a single byte of 
		// HexA
		for(j=(MAX_OCTETS/2);j > 0;j--)
		{
			u_int32 val;

			val = (bigHexA->num[i-1] * bigHexB->num[j-1]) +
			BigHexResult->num[j+i-1] + mcarry;
			BigHexResult->num[j+i-1] = (val & 0x0000FFFF);

			mcarry = (u_int16)( val >> 16);
		}
	}
	setVeryBigNumberLength(BigHexResult);
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

void AddBigHexSign( bigHex *bigHexA, bigHex *bigHexB, bigHex *BigHexResult)
{
	//
	// This function uses use the basis AddBigHex and SubtractBigHex to implement a full
	// Addition which considers sign and mod.
	// 

	// if A is + and B is + the just Add and perform Mod  C = A+B
	// if A is + and B is - then
	//     if ( A > B)
	//        C = A - B  (sign +)
	//     else if (A < B)
	//        C = B - A  (sign -)
	// if A is - and B is - then just Add and Mod and change sign to -  C = -(A+B).
    // if A is - and B is + then 
	//     if ( A > B)
	//        C = A - B  (sign -)
	//     else if (A < B)
	//        C = B - A  (sign +)

	if ((bigHexA->sign == bigHexB->sign))
	{
		AddBigHex(bigHexA, bigHexB,BigHexResult);
		BigHexResult->sign = bigHexA->sign;
	}
	else if ((bigHexA->sign == 0) && (bigHexB->sign == 1))
	{
		if (isGreaterThanOrEqual(bigHexA,bigHexB))
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
	else if ((bigHexA->sign == 1) && (bigHexB->sign == 0))
	{
	//     if ( A > B)
	//        C = A - B  (sign -)
	//     else if (A < B)
	//        C = B - A  (sign +)

		if ( isGreaterThan(bigHexA,bigHexB))
		{
			SubtractBigHex(bigHexA, bigHexB,BigHexResult);
			BigHexResult->sign = 1;
		}
		else
		{
			SubtractBigHex(bigHexB, bigHexA,BigHexResult);
			BigHexResult->sign = 0;
		}
	}
}


void SubtractBigHexSign(bigHex *bigHexA, bigHex *bigHexB, bigHex *BigHexResult)
{
	// This function uses use the basis AddBigHex and SubtractBigHex to implement a full
	// Addition which considers sign and mod.
	// 

	//  C = A - B
	// 1/  if A is + and B is + then two options 
	//     if ( A > B)
	//        C = A - B  (sign +)
	//     else if (A < B)
	//        C = B - A  (sign -)
	// 2/  if A is - and B is +   C = -(A+B) Mod 
	// 3/  if A is + and B is -   C =   A+B  Mod
	// 4/  if A is - and B is -   C = -A + B = B - A
	//      if (B > A)
	//        C = B - A (sign +)
	//      else if ( B < A )
	//        C = A - B (sign -)
	//       
	// 

	if ((bigHexA->sign == 0) && (bigHexB->sign == 0))
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
	else if ((bigHexA->sign == 1) && (bigHexB->sign == 1))
	{
	//      if (B > A)
	//        C = B - A (sign +)
	//      else if ( B < A )
	//        C = A - B (sign -)
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
	else if ((bigHexA->sign == 0) && (bigHexB->sign == 1))
	{
	  // 3/  if A is + and B is -   C =   A+B  Mod
		AddBigHex(bigHexA, bigHexB,BigHexResult);
		BigHexResult->sign = 0;
	}
	else if ((bigHexA->sign == 1) && (bigHexB->sign == 0))
	{
		AddBigHex(bigHexA, bigHexB,BigHexResult);
		BigHexResult->sign = 1;
	}
}


void MultiplyBigHexSign(bigHex *bigHexA,bigHex *bigHexB, veryBigHex *BigHexResult)
{
	u_int8 sign;

	if (bigHexA->sign == bigHexB->sign)
	{
		// We Have a Positive number 
		sign = 0x00;
	}
	else
	{
		sign = 0x01;
	}

	MultiplyBigHex(bigHexA,bigHexB,BigHexResult);
	BigHexResult->sign = sign;
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

void AddBigHexMod( bigHex *bigHexA, bigHex *bigHexB, bigHex *BigHexResult)
{
	bigHex tmpResult;

	AddBigHexSign(bigHexA, bigHexB,BigHexResult);

	if (BigHexResult->sign == 0x00)   // Positive Number
	{
		while(isGreaterThanOrEqual(BigHexResult,&bigHexP))
	    {
			SubtractBigHexSign(BigHexResult,&bigHexP,&tmpResult);
			copyBigHex(&tmpResult,BigHexResult);
		}
	}	
	else
	{
		while(BigHexResult->sign == 0x01)
		{
			AddBigHexSign(BigHexResult,&bigHexP,&tmpResult);
			copyBigHex(&tmpResult,BigHexResult);
		}
	}
	setBigNumberLength(BigHexResult);
}


void SubtractBigHexMod(bigHex *bigHexA, bigHex *bigHexB, bigHex *BigHexResult)
{
	bigHex tmpResult;

	SubtractBigHexSign(bigHexA, bigHexB,BigHexResult);


	if (BigHexResult->sign == 0x00)   // Positive Number
	{
		while(isGreaterThanOrEqual(BigHexResult,&bigHexP))
	    {
			SubtractBigHexSign(BigHexResult,&bigHexP,&tmpResult);
			copyBigHex(&tmpResult,BigHexResult);
		}
	}	
	else
	{
		while(BigHexResult->sign == 0x01)
		{
			AddBigHexSign(BigHexResult,&bigHexP,&tmpResult);
			copyBigHex(&tmpResult,BigHexResult);
		}
	}

	setBigNumberLength(BigHexResult);
}

void MultiplyBigHexModP(bigHex *bigHexA,bigHex *bigHexB, bigHex *BigHexResult)
{
	veryBigHex tmpResult;

    MultiplyBigHexSign(bigHexA,bigHexB,&tmpResult);
	veryBigHexModP(&tmpResult,BigHexResult);
}

//****************************************************************
// MOD FUNCTIONS 
//  
//  veryBigHexModP
//***************************************************************
void veryBigHexModP(veryBigHex* tmpResult,bigHex* BigHexResult)
{
	bigHex left_over;

	int i=0;
	int k;
	bigHex tmpResult2;
    u_int8 sign;
	u_int32 times;

	sign = tmpResult->sign;

	while ((tmpResult->num[i] == 0x00) && (i<(MAX_OCTETS/2)))
		i++;

	// Take the next 13 elements 
	// 
	tmpResult2.sign = sign;

	if (isVeryBigHexGreaterThanOrEqual(tmpResult,&veryBigHexP))
	{
		while(tmpResult->num[i] == 0x00)
			i++;

		tmpResult2.num[0] = 0;
		tmpResult2.num[1] = 0;
		for(k=0;k<13;k++)
		{
			tmpResult2.num[k+2] = tmpResult->num[i+k];
		}
	}
	else
	{
		for(k=0;k<15;k++)
		{
			tmpResult2.num[k] = tmpResult->num[i+k];
		}
	}
	setBigNumberLength(&tmpResult2);

	// Now do interative subtract -  interative subtract P from tmpResult2
	// combine the left_over with the next byte from tmpResult and repeat. 

	while((i+k) < (MAX_OCTETS))
	{
		specialModP(&times,&left_over,&tmpResult2);
		shiftLeftOneArrayElement(&left_over);

		// Add the next byte from A in left_over;
		left_over.num[(MAX_OCTETS/2)-1] = tmpResult->num[i+k];
		k++;
		setBigNumberLength(&left_over);
		copyBigHex(&left_over,&tmpResult2);
	}
	specialModP(&times,&left_over,&tmpResult2);

	copyBigHex(&left_over,BigHexResult);
	// 2/ Copy left_over from the Mod to BigHexResult

	setBigNumberLength(BigHexResult);
	BigHexResult->sign = 0;
}

void shiftLeftOneArrayElement(bigHex *input)
{
	int i;
    for(i=0;i<(MAX_OCTETS/2);i++)
	{
		input->num[i] = input->num[i+1];
	}
}
//------------------------------------------------------------------------
// The purpose of this function is to extract from a bigHex a bigHex which
//
//  Big Number A
//     00 00 00 00 00 00 AA BB CC DD EE FF
//
//  Big Number B -- the divider
//     00 00 00 00 00 00 00 00 00 XX YY ZZ
//
//  Returns   if(00 AA BB CC >= 00 XX YY ZZ)
//                return 00 AA BB CC
//            else
//                return 00 AA BB CC DD
//    
// 
//-------------------------------------------------------------------------
void takeSignificantPart(bigHex *input,u_int8 num_bytes,u_int8 start_offset,bigHex* result)
{
	int j=0;
	int i=0;

	while(input->num[i] == 0)
		i++;
// this is the start of the number we want to copy
	for (j=0;j!=num_bytes;j++)
	{
		result->num[(MAX_OCTETS/2)-num_bytes+j] = input->num[i+j];
	}

	result->len = num_bytes;
}

//
// This function adds N * Mod P to a number.
//  

void specialModP(u_int32 *num_times,bigHex *left_over,bigHex *tmpHexA)
{
   bigHex NModP;
   u_int32 SignificantU16;
   copyBigHex(tmpHexA,left_over);

   if(((left_over->sign == 0) && isGreaterThanOrEqual(left_over,&bigHexP)) ||
	   (left_over->sign == 1))
   {
	   // If I multiply the most significant u_int16 of A by the Mod P and then subtract from A 
	   // this is equivalent to an iteractive subtraction - but much faster.
	   if (left_over->len > bigHexP.len)
	   {
		   SignificantU16 = GetMostSignificantU16(left_over);
		   MultiplyByU16ModP(SignificantU16,&NModP);
		   if (tmpHexA->sign == 0)
		   {
				SubtractBigHexSign(tmpHexA,&NModP,left_over);
		   }
		   else
		   {
				AddBigHexSign(tmpHexA,&NModP,left_over);
		   }
	   }
	   while (((left_over->sign == 0) && isGreaterThanOrEqual(left_over,&bigHexP)) ||
			(left_over->sign == 1))
	   {
		   // We an do iterative subtration. 
		   if (left_over->sign == 0)
		   {
			   SubtractBigHexSign(left_over,&bigHexP,tmpHexA);
		   }
		   else
		   {
			   AddBigHexSign(left_over,&bigHexP,tmpHexA);
		   }

			(*num_times)++;
			copyBigHex(tmpHexA,left_over);
	   }
   }
}

u_int32 GetMostSignificantU16(bigHex *tmpHexA)
{
	u_int8 i;
	for (i = 0; i< (MAX_OCTETS/2); i++)
	{
		if (tmpHexA->num[i] != 0x00)
			break;
	}
	return tmpHexA->num[i];
}

void MultiplyByU16ModP(u_int32 N,bigHex* result)
{
	bigHex NbigHex;
	veryBigHex tempResult;
	int i;

   	// Create a bigHex with the least significant part = N
	initBigNumber(&NbigHex);

	NbigHex.num[(MAX_OCTETS/2)-1] = N;
    setBigNumberLength(&NbigHex);
    MultiplyBigHex(&NbigHex,&bigHexP,&tempResult);

	for(i=0;i<(MAX_OCTETS/2);i++)
		result->num[i] = tempResult.num[i+15];

    setBigNumberLength(result);
	result->sign = 0;
}


void copyBigHex(bigHex *source,bigHex *destination)
{
	int i;
	for(i=0;i<(MAX_OCTETS/2);i++)
	{
		destination->num[i] = source->num[i];
	}
	destination->len = source->len;
	destination->sign = source->sign;
}

void initBigNumber(bigHex *BigHexResult)
{
	int i; 

	for(i=0;i<(MAX_OCTETS/2);i++)
		BigHexResult->num[i] = 0x00;

	BigHexResult->len = 0;
	BigHexResult->sign = 0;
}

void initVeryBigNumber(veryBigHex *BigHexResult)
{
	int i; 

	for(i=0;i<(MAX_OCTETS);i++)
		BigHexResult->num[i] = 0x00;

	BigHexResult->len = 0;
	BigHexResult->sign = 0;
}

void setBigNumberLength(bigHex *BigHexResult)
{
	int i;

	for(i=0;i < (MAX_OCTETS/2);i++)
	{
		if(BigHexResult->num[i] != 0x00)
           break;
	}
	BigHexResult->len = (MAX_OCTETS/2)-i;
}

void setVeryBigNumberLength(veryBigHex *BigHexResult)
{
	int i;

	for(i=0;i < (MAX_OCTETS);i++)
	{
		if(BigHexResult->num[i] != 0x00)
           break;
	}
	BigHexResult->len = (MAX_OCTETS)-i;
}

//
// if A > B return TRUE
//
  
int isGreaterThan(bigHex *bigHexA,bigHex *bigHexB)
{

int i;
   int A_index=0;
   int B_index=0;
   int A_len;
   int B_len;

   while(bigHexA->num[A_index] == 00)
		A_index++;

   while(bigHexB->num[B_index] == 00)
		B_index++;

   A_len = (MAX_OCTETS/2)-A_index;
   B_len = (MAX_OCTETS/2)-B_index;

    if(A_len > B_len)
		return 1;
	else if(A_len < B_len)
		return 0;

	// Now we have two equal sized arrays which we need to compare.
    // . 
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
	return 0;
}


int isGreaterThanOrEqual(bigHex *bigHexA,bigHex *bigHexB)
{
   int i;
   int A_index=0;
   int B_index=0;
   int A_len;
   int B_len;

   while(bigHexA->num[A_index] == 00)
		A_index++;

   while(bigHexB->num[B_index] == 00)
		B_index++;

   A_len = (MAX_OCTETS/2)-A_index;
   B_len = (MAX_OCTETS/2)-B_index;

    if(A_len > B_len)
		return 1;
	else if(A_len < B_len)
		return 0;

	// Now we have two equal sized arrays which we need to compare.
    // . 
    
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


int isVeryBigHexGreaterThanOrEqual(veryBigHex *bigHexA,veryBigHex *bigHexB)
{
   int i;
   int A_index=0;
   int B_index=0;
   int A_len;
   int B_len;

   while(bigHexA->num[A_index] == 00)
		A_index++;

   while(bigHexB->num[B_index] == 00)
		B_index++;

   A_len = (MAX_OCTETS)-A_index;
   B_len = (MAX_OCTETS)-B_index;

    if(A_len > B_len)
		return 1;
	else if(A_len < B_len)
		return 0;

	// Now we have two equal sized arrays which we need to compare.
    // . 
    
	for (i=((MAX_OCTETS)-A_len); i <(MAX_OCTETS);i++)
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
int notEqual(bigHex *bigHexA,bigHex *bigHexB)
{
	int i;

	for(i=0;i<(MAX_OCTETS/2);i++)
	{
		if(bigHexA->num[i] != bigHexB->num[i])
		{
			return 1;
		}
	}
	return 0;
}

//isLessThan
int isLessThan(bigHex *bigHexA,bigHex *bigHexB)
{
int i;
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

int getNumberLen(bigHex* bigHexA)
{
	int i=0;

	while(bigHexA->num[i] == 00)
	{
		i++;
	}
	return ((MAX_OCTETS/2)-i);
}


void GF_Point_Double(ECC_Point *pPointP,ECC_Point *pResultPoint)
{
	GF_Point_Addition(pPointP,pPointP,pResultPoint);
}

void GF_Point_Addition(ECC_Point* pPointP,ECC_Point* pPointQ, ECC_Point* pResultPoint)
{
	// Gamma or S = ((y1-y2)/(x1-x2))
    // What is a  = -3 
	// x3 = (Gamma*Gamma) - x1 - x2
	// y3 = -y1 + Gamma*(x1 - x3)
	//see 115-EllipticCurveCryptography 
	 
	// Need to advance all GF operands so that they return a result in the field  -- Mod P
	bigHex gamma;
	bigHex tempResult;
	bigHex tempResult2;
	bigHex tempResult4;
#if 0 //def DEBUG_P192
	bigHex tempResult5;
#endif
	bigHex GammaSquared;
	bigHex DiffY_coords;
	bigHex DiffX_coords;

    initBigNumber(&tempResult4);

	if ((pPointP->x.len == 0) && (pPointP->y.len == 0))
	{
		// Let the result be PointQ
        GF_Point_Copy(pPointQ,pResultPoint);
		return;
	}
	else if ((pPointQ->x.len == 0) && (pPointQ->y.len == 0))
	{
		// Let the result be PointP
        GF_Point_Copy(pPointP,pResultPoint);
		return;
	}

	if ((pPointQ->y.len == 0) || (pPointP->y.len == 0))
		return;
	if (notEqual(&(pPointP->x),&(pPointQ->x)))
	{
		// Determine Gamma = S = (yP - yQ)/(xP - xQ)
		SubtractBigHexMod(&(pPointP->y),&(pPointQ->y),&DiffY_coords);
		SubtractBigHexMod(&(pPointP->x),&(pPointQ->x),&DiffX_coords);
		bigHexInversion(&DiffX_coords,&tempResult4);

#if 0 //def DEBUG_P192
		MultiplyBigHexModP(&tempResult4,&DiffX_coords,&tempResult5);
		if ((tempResult5.num[0xe] != 1) || (tempResult5.len != 1) || (tempResult5.sign != 0))
		{
			// ERROR !! Should Exit of Generate HW Error Event
			return;
		}
#endif
		MultiplyBigHexModP(&tempResult4,&DiffY_coords,&gamma);
	}
	else
	{
		bigHex xP_squared;
		bigHex tempResult3;
		bigHex N;

		initBigNumber(&N);
		N.len = 1;

		// S = (3xP^2 + A)/2yP     A = -3
		// xP^2
		MultiplyBigHexModP(&(pPointP->x),&(pPointP->x),&xP_squared);
		// 3 * xP^2
		N.num[(MAX_OCTETS/2)-1] = 0x03;
		MultiplyBigHexModP(&xP_squared,&N,&tempResult);
        // 3xP^2 - 3
		SubtractBigHexMod(&tempResult,&N,&tempResult2);
		N.num[(MAX_OCTETS/2)-1] = 0x02;
		MultiplyBigHexModP(&(pPointP->y),&N,&tempResult3);
		bigHexInversion(&tempResult3,&tempResult4);
#if 0 //def DEBUG_P192
		MultiplyBigHexModP(&tempResult4,&tempResult3,&tempResult5);
		if ((tempResult5.num[0xe] != 1) || (tempResult5.len != 1) || (tempResult5.sign != 0))
		{
			// ERROR !! Should Exit of Generate HW Error Event
			return;
		}
#endif
		MultiplyBigHexModP(&tempResult4,&tempResult2,&gamma);
	}
    // Determine x3 = S^2 -xP -xQ
    MultiplyBigHexModP(&gamma,&gamma,&GammaSquared);
	SubtractBigHexMod(&GammaSquared,&pPointP->x,&tempResult);
	SubtractBigHexMod(&tempResult,&pPointQ->x,&pResultPoint->x);
    
	// Determine y3 = S(xP - x3) - yP

	SubtractBigHexMod(&pPointP->x,&pResultPoint->x, &DiffX_coords);	
    MultiplyBigHexModP(&DiffX_coords,&gamma,&tempResult);
    SubtractBigHexMod(&tempResult,&pPointP->y,&pResultPoint->y);

}
//    Q = kP
//   
void ECC_Point_Multiplication(bigHex* pk,ECC_Point* pPointP, ECC_Point* pPointQ)
{
	// see ecc.pdf
	//
	ECC_Point PointR;
	ECC_Point tmpResultPoint;
	ECC_Point tmpResultPoint2;

	//Scalar Multiplication: LSB first
    // • Require k=(km-1,km-2,…,k0)2, km=1
    // • Compute Q=kP
    //– Q=0, R=P
    // – For i=0 to m-1
    //   • If ki=1 then
    //      – Q=Q+R
    //   • End if
    //   • R=2R
    // – End for
    // – Return Q

	int i,j;
	u_int16 uint16Val;
	u_int8 bitVal;
	u_int16 rem ;
	int bitNum=0;
	
	initBigNumber(&pPointQ->x);
    initBigNumber(&pPointQ->y);

	// Copy PointP to R;
	GF_Point_Copy(pPointP,&PointR);

	for(i=(MAX_OCTETS/2);i>0;i--)
	{
		uint16Val = pk->num[i-1];
		for (j=0;j<16;j++)
		{
			bitNum++;
            rem = (uint16Val >> j);
			bitVal = (uint16Val >> j) & 0x0001;

			if (bitVal)
			{
				// Q = Q + R
                GF_Point_Addition(pPointQ,&PointR,&tmpResultPoint);
				// Copy ResultPoint to PointQ
				GF_Point_Copy(&tmpResultPoint,pPointQ);
			} 
			// R = 2R
			GF_Point_Double(&PointR,&tmpResultPoint2);
			// Copy Result point to PointR 
			GF_Point_Copy(&tmpResultPoint2,&PointR);
		}
	}
	// Q = Q + R
	// ResultPoint is in  PointQ

	GF_Point_Copy(&tmpResultPoint,pPointQ);
}

void GF_Point_Copy(ECC_Point *source,ECC_Point *destination)
{
	copyBigHex(&source->x,&destination->x);
	copyBigHex(&source->y,&destination->y);
}

void bigHexInversion(bigHex* bigHexA,bigHex* pResult)
{
	bigHex u;
	bigHex v;
	bigHex A;
	bigHex C;
	bigHex tmpResult;
	bigHex tmpBigResult;
	u_int8 sign;
	// Ref " Software Implementation of the NIST Elliptical " 
	//  Input : Prime P (bigHexP), a (bigHexA) E [1, p-1]
	//  Output : a^-1 mod P

	if (bigHexA->sign == 1)
	{
		sign = 1;
		bigHexA->sign = 0;
	}
	else
		sign = 0;

	// Step 1 
	// u <-- a, v <-- p, A <-- 1, C <-- 0.
	//
	initBigNumber(&A);
	initBigNumber(&C);

	copyBigHex(bigHexA,&u);
	copyBigHex(&bigHexP,&v);

	A.num[(MAX_OCTETS/2)-1] = 1;
	C.num[(MAX_OCTETS/2)-1] = 0;
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
		while(isbigHexEven(&u))
		{
			divideByTwo(&u);
			if (isbigHexEven(&A))
			{
				// A = A/2
                divideByTwo(&A);
			}
			else
			{
				// A = (A+P)/2
				AddBigHexSign(&A,&bigHexP,&tmpBigResult);
                divideByTwo(&tmpBigResult);
	            copyBigHex(&tmpBigResult,&A);

			}

		}
		while(isbigHexEven(&v))
		{
			divideByTwo(&v);
			if (isbigHexEven(&C))
			{
                divideByTwo(&C);
			}
			else
			{
				AddBigHexSign(&C,&bigHexP,&tmpBigResult);
                divideByTwo(&tmpBigResult);
	            copyBigHex(&tmpBigResult,&C);
			}
		}
		if (isGreaterThanOrEqual(&u,&v))
		{
			SubtractBigHexSign(&u,&v,&tmpResult);
		    copyBigHex(&tmpResult,&u);
			SubtractBigHexSign(&A,&C,&tmpBigResult);
	        copyBigHex(&tmpBigResult,&A);
		}
		else
		{
			SubtractBigHexSign(&v,&u,&tmpResult);
		    copyBigHex(&tmpResult,&v);
			SubtractBigHexSign(&C,&A,&tmpBigResult);
	        copyBigHex(&tmpBigResult,&C);
		}
	}
	bigHexModP(&C,pResult);
}

	
int isbigHexEven(bigHex* A)
{

	if ((A->num[(MAX_OCTETS/2)-1]) & 0x01)
		return 0;
	else
		return 1;
}


void divideByTwo(bigHex* A)
{
    u_int8 rem=0;
	u_int32 u32val;
    int i;

	for(i=0; i < (MAX_OCTETS/2); i++)
	{
		u32val = A->num[i] + (rem * 0x10000);

		A->num[i] = u32val/2;
        rem = u32val - ((u32val/2)*2);
	}
}

void bigHexModP(bigHex* bigHexA,bigHex* pResult)
{
	int i=0;
	int k;
	bigHex tmpResult2;
	bigHex left_over;
	u_int32 times;
	u_int8 sign;

	sign = bigHexA->sign;

	// If the Mod P is greater than bigHexA then return with bigHexA unmodified.

	if (isGreaterThan(&bigHexP,bigHexA) && (sign == 0))
	{
		copyBigHex(bigHexA,pResult);
		setBigNumberLength(pResult);
		return;
	}

	// Take the next 13 elements 
	// 
	initBigNumber(&tmpResult2);

	if (isGreaterThanOrEqual(bigHexA,&bigHexP))
	{
		while(bigHexA->num[i] == 0x00)
			i++;

		tmpResult2.num[0] = 0;
		tmpResult2.num[1] = 0;
		for(k=0;k<13;k++)
		{
			tmpResult2.num[k+2] = bigHexA->num[i+k];
		}
	}
	else
	{
		for(k=0;k<15;k++)
		{
			tmpResult2.num[k] = bigHexA->num[i+k];
		}
	}

	setBigNumberLength(&tmpResult2);

	// Now do interative subtract -  interative subtract P from tmpResult2
	// combine the left_over with the next byte from tmpResult and repeat. 

	if (sign == 0) // Positive Number thus subtract P iteratively.
	{
		while((i+k) < (MAX_OCTETS/2))
		{
			specialModP(&times,&left_over,&tmpResult2);
			shiftLeftOneArrayElement(&left_over);

			// Add the next byte from A in left_over;
			left_over.num[(MAX_OCTETS/2)-1] = bigHexA->num[i+k];
			k++;
			copyBigHex(&left_over,&tmpResult2);
		}
		specialModP(&times,&left_over,&tmpResult2);
	}
	else // Negative Number 
	{

		tmpResult2.sign = 1;
		while((i+k) < (MAX_OCTETS/2))
		{
			specialModP(&times,&left_over,&tmpResult2);
			shiftLeftOneArrayElement(&left_over);
			// Add the next byte from A to left_over;
			left_over.num[(MAX_OCTETS/2)-1] = bigHexA->num[i+k];
			k++;
			copyBigHex(&left_over,&tmpResult2);
		}
		specialModP(&times,&left_over,&tmpResult2);
	}
	copyBigHex(&left_over,pResult);
	setBigNumberLength(pResult);
}

void LMecc_Generate_ECC_Key(u_int8* secret_key,  u_int8* public_key_x,
					   u_int8* public_key_y,u_int8* result_x)
{
	u_int8 big_num_offset;
	u_int8 i,j=0;

	// Copy the secret_key to a bigHex format.
	big_num_offset = 3;

	PrivateKey.num[0] = 0;
	PrivateKey.num[1] = 0;
	PrivateKey.num[2] = 0;

	for (i=0;i<24;)
	{
		PrivateKey.num[(i/2)+big_num_offset] = (u_int16)((*(secret_key+i) << 8) & 0xFF00) +
													((*(secret_key+i+1)) & 0x00FF);
		i += 2;
	}

	setBigNumberLength(&PrivateKey);
	// Now Copy the Public Key coordinates to ECC point format.

	PublicKey.x.num[0] = 0;
	PublicKey.x.num[1] = 0;
	PublicKey.x.num[2] = 0;
	PublicKey.y.num[0] = 0;
	PublicKey.y.num[1] = 0;
	PublicKey.y.num[2] = 0;

	for (i=0;i<24;)
	{
		PublicKey.x.num[(i/2)+big_num_offset] = (u_int16)((*(public_key_x+i) << 8) & 0xFF00) +
													((*(public_key_x+i+1)) & 0x00FF);
		PublicKey.y.num[(i/2)+big_num_offset] = (u_int16)((*(public_key_y+i) << 8) & 0xFF00) +
													((*(public_key_y+i+1)) & 0x00FF);

		ResultPoint.x.num[(i/2)] = 0;
		ResultPoint.y.num[(i/2)] = 0;
		i += 2;


	}
	setBigNumberLength(&PublicKey.x);
	setBigNumberLength(&PublicKey.y);
	PublicKey.x.sign = 0;
	PublicKey.y.sign = 0;
	ResultPoint.x.len = 0;
	ResultPoint.y.len = 0;
	ResultPoint.x.sign = 0;
	ResultPoint.y.sign = 0;



	ECC_Point_Multiplication(&PrivateKey,&PublicKey,&ResultPoint);

    for(i=0;i<24;)
	{
		result_x[i] = ((ResultPoint.x.num[j+big_num_offset] & 0xFF00) >> 8);
        result_x[i+1] = (ResultPoint.x.num[j+big_num_offset] & 0xFF);
		i+=2;
		j++;
	}
}

#if 0
/* 
 Multiply two 32-bit numbers, V1 and V2, to produce 
 a 64 bit result in the HI/LO registers.  
 The algorithm is high-school math:

         A B
       x C D
       ------
       AD || BD
 AC || CB || 0

 where A and B are the high and low short words of V1, 
 C and D are the short words of V2, AD is the product of 
 A and D, and X || Y is (X << 16) + Y.
 Since the algorithm is programmed in C, we need to be 
 careful not to overflow. 
*/

static void long_multiply (reg_word v1, reg_word v2)
{
  register long a, b, c, d;
  register long x, y;

  a = (v1 >> 16) & 0xffff;
  b = v1 & 0xffff;
  c = (v2 >> 16) & 0xffff;
  d = v2 & 0xffff;

  LO = b * d;                   /* BD */
  x = a * d + c * b;            /* AD + CB */
  y = ((LO >> 16) & 0xffff) + x;

  LO = (LO & 0xffff) | ((y & 0xffff) << 16);
  HI = (y >> 16) & 0xffff;

  HI += a * c;                  /* AC */
}


The ARM7TDMI features a multi-cycle 32x32->64-bit multiplier. The ARM7TDMI multiplier employs 
an early termination algorithm that multiplies multiplicands in 8-bit stages. If at any point
all of the remaining bits of either multiplicand are zero in the case of positive numbers, 
or one in the case of negative numbers (thus, assuring the remaining product will be zero),
the multiply terminates. Each 8-bit multiply stage requires one instruction cycle to execute. 
Thus, an ARM7TDMI multiply instruction can take as little as two cycles (one to load the instruction
and one for the first-stage multiply) or as many as five cycles (one to load the instruction and four
for all four multiply stages).

Some multiply and multiply-add instructions, called long multiply instructions by ARM, store or add
the 64-bit product to two 32-bit general-purpose registers. Normal multiply and multiply-add instructions
store or add only the lower 32 bits of the product. When using normal multiply and multiply-add instructions,
programmers must be careful to scale input operands so that all significant output bits are contained in the
lower 32 bits of the result since no guard bits are available. The ARM7TDMI does not support saturation
arithmetic

#endif
#else


#endif
