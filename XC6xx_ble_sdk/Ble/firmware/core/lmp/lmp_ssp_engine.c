
/**************************************************************************
 * MODULE NAME:    lmp_ssp_engine.c       
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    LMP Secure Simple Pairing Protocol Crypographic Functions
 * AUTHOR:         Gary Fleming
 * DATE:           02 Feb 2009
 *
 * SOURCE CONTROL: $Id: lmp_ssp_engine.c,v 1.8 2014/03/11 03:14:01 garyf Exp $
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

#include "lmp_features.h"
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
#include "lmp_ecc.h"
#include "lmp_ssp_engine.h"

//Initialize table of round constants
//(first 32 bits of the fractional parts of the cube roots of the first 64 primes 2..311):

const u_int32 K[64] = {
   0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

/* rotates the hard way */
#define ROL(x, y) ( (((unsigned long)(x)<<(unsigned long)((y)&31)) | (((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)
#define ROR(x, y) ( ((((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)((y)&31)) | ((unsigned long)(x)<<(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)
#define ROLc(x, y) ( (((unsigned long)(x)<<(unsigned long)((y)&31)) | (((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)
#define RORc(x, y) ( ((((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)((y)&31)) | ((unsigned long)(x)<<(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)

#define SHA_256_BlockSize_In_Words 16
#define SHA_256_BlockSize_In_Bytes 64

u_int8* HMAC(u_int8* Key,int Key_Len,u_int8 *message,int message_len);
u_int32 SHA_256(u_int8* Message,u_int32 MessageLength,u_int8* Hash);
u_int32 hashConcat(u_int8* Message1,u_int32 Message1_Len,u_int8* Message2, u_int32 Message2_Len,u_int8* newConcatBuffer);

u_int8 GenericHashOutput[32];

//using namespace System;
#if 0
void Test_HMAC_SHA_256(void);
#endif

int Test_SPP_Functions(void)
{
#if 0
   u_int8  Hash[32];

   u_int8 Key[15] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
   int Key_Len = 128;
   u_int8 U_1a[24] = {0x15,0x20,0x70,0x09,0x98,0x44,0x21,0xa6,0x58,0x6f,0x9f,0xc3,0xfe,0x7e,0x43,0x29,
	   0xd2,0x80,0x9e,0xa5,0x11,0x25,0xf8,0xed };

   u_int8 V_1a[24] = {0x35,0x6b,0x31,0x93,0x84,0x21,0xfb,0xbf,0x2f,0xb3,0x31,0xc8,0x9f,0xd5,0x88,0xa6,
	   0x93,0x67,0xe9,0xa8,0x33,0xf5,0x68,0x12 };

   u_int8 X_1a[24] = {0xd5,0xcb,0x84,0x54,0xd1,0x77,0x73,0x3e,0xff,0xff,0xb2,0xec,0x71,0x2b,0xae,0xab};

   u_int8 Y_1a[24] = {0xa6,0xe8,0xe7,0xcc,0x25,0xa7,0x5f,0x6e,0x21,0x65,0x83,0xf7,0xff,0x3d,0xc4,0xcf};

   u_int8 W_1a[24]  = {0xfb,0x3b,0xa2,0x01,0x2c,0x7e,0x62,0x46,0x6e,0x48,0x6e,0x22,0x92,0x90,0x17,0x5b,0x4a,0xfe,0xbc,0x13,0xfd,0xcc,0xee,0x46};

   u_int8 N1_1a[16] = {0xd5,0xcb,0x84,0x54,0xd1,0x77,0x73,0x3e,0xff,0xff,0xb2,0xec,0x71,0x2b,0xae,0xab};

   u_int8 N2_1a[16] = {0xa6,0xe8,0xe7,0xcc,0x25,0xa7,0x5f,0x6e,0x21,0x65,0x83,0xf7,0xff,0x3d,0xc4,0xcf};

   u_int8 KeyId_1a[4] = {0x62,0x74,0x6c,0x6b};

   u_int8 A1_1a[6] = {0x56,0x12,0x37,0x37,0xbf,0xce};

   u_int8 A2_1a[6] = {0xa7,0x13,0x70,0x2d,0xcf,0xc1};


   u_int8 R_1a[16] = {0x12,0xa3,0x34,0x3b,0xb4,0x53,0xbb,0x54,0x08,0xda,0x42,0xd2,0x0c,0x2d,0x0f,0xc8};

   u_int8 IOcap_1a[3] = {0x00,0x00,0x00};

   u_int8 Z[1] = {0};

   u_int8 message[56] = {'a','b','c','d', 'b','c','d','e', 'c','d','e','f',   'd','e','f','g',  'e','f','g','h',
			    		 'f','g','h','i', 'g','h','i','j', 'h','i','j','k',   'i','j','k','l',  'j','k','l','m',

                         'k','l','m','n', 'l','m','n','o', 'm','n','o','p',   'n','o','p','q'};

   u_int8 message1[3] = {'a','b','c'};

   int message1_len =   24;

   int message_len =  448;

   
   // test of hash concat 
  // hashConcat(temp1,(15*8),temp2,(7*8),tempResult);

   Test_HMAC_SHA_256();

   SHA_256(message, message_len,Hash); 

   SHA_256(message1,message1_len,Hash);
#endif
    return 0;
}

void F1_Invert(u_int8 *U,u_int8 *V, u_int8 *X, u_int8 *Z,u_int8* outPut)
{
	u_int8 U_inv[24];
	u_int8 V_inv[24];
	u_int8 X_inv[16];
	u_int8 output_inv[16];

	int i;

	for (i=0;i<24;i++)
	{
		U_inv[i] = U[23-i];
		V_inv[i] = V[23-i];	
	}

	for (i=0;i<16;i++)
	{
		X_inv[i] = X[15-i];
	}

	F1(U_inv,V_inv,X_inv,Z,output_inv);

	for (i=0;i<16;i++)
	{
		outPut[i] = output_inv[15-i];
	}
}

void F1(u_int8 *U,u_int8 *V, u_int8 *X, u_int8 *Z,u_int8* OutPut)
{
    u_int8 *tempPtr;
	int i;
	// U is 192 bits = 24 Bytes = 6 Words
	// V is 192 bits = 24 Bytes = 6 Words
	// X is 128 bits = 16 Bytes = 4 Words  -- The KEY
	// Z is   8 bits =  1 Byte
	//

	u_int8 IntMessage[48];
	u_int8 Message[49];

	hashConcat(U,(24*8),V,(24*8),IntMessage);
	hashConcat(IntMessage,(48*8),Z,(1*8),Message);

	tempPtr = HMAC(X,128,Message,(49*8));

#if 1
	for (i=0;i<16;i++)
		OutPut[i] = tempPtr[i];
#endif
}

void G(u_int8 *U,u_int8 *V, u_int8 *X, u_int8 *Y,u_int8* OutPut)
{
	int i;
#if 1
	u_int8 IntMessage1[48];
	u_int8 IntMessage2[64];
	u_int8 Message[80];
	u_int8 HashG[32];
#endif

	// init all arrays
	for (i=0;i<48;i++)
		IntMessage1[i]= 0;

	for (i=0;i<64;i++)
		IntMessage2[i]= 0;

	for (i=0;i<80;i++)
		Message[i]= 0;

	for (i=0;i<32;i++)
		HashG[i] = 0;

	// U is 192 bits = 24 Bytes = 6 Words
	// V is 192 bits = 24 Bytes = 6 Words
	// X is 128 bits = 16 Bytes = 4 Words
	// Y is 128 bits = 16 Bytes = 4 Words
	//
	// g(U, V, X, Y) = SHA-256 (U || V || X || Y) / 2e32

	hashConcat(U,(24*8),V,(24*8),IntMessage1);
	hashConcat(IntMessage1,48*8,X,(16*8),IntMessage2);
	hashConcat(IntMessage2,(64*8),Y,(16*8),Message);

    SHA_256(Message,(80*8),HashG);

	OutPut[0] = HashG[28];
	OutPut[1] = HashG[29];
	OutPut[2] = HashG[30];
	OutPut[3] = HashG[31];
}

void F2(u_int8 *W,u_int8 *N1, u_int8 *N2, u_int8 *KeyId, u_int8 *A1, u_int8 *A2,u_int8* OutPut)
{
	// W is 192 bits = 24 Bytes = 6 Words
	// N1 is 128 bits = 16 Bytes = 4 Words
    // N2 is 128 bits = 16 Bytes = 4 Words
	// keyID is 32 bits = 4 Bytes = 1 Word
	// A1 is 48 bits = 6 Bytes
	// A2 is 48 bits = 6 Bytes 

	u_int8 IntMessage1[32];
	u_int8 IntMessage2[36];
	u_int8 IntMessage3[42];
	u_int8 Message[48];
	u_int8 *tempPtr;
	int i;

	hashConcat(N1,(16*8),N2,(16*8),IntMessage1);
	hashConcat(IntMessage1,(32*8),KeyId,(4*8),IntMessage2);
	hashConcat(IntMessage2,(36*8),A1,(6*8),IntMessage3);
	hashConcat(IntMessage3,(42*8),A2,(6*8),Message);

	tempPtr = HMAC(W,192,Message,(48*8));

#if 1
	for (i=0;i<16;i++)
		OutPut[i] = tempPtr[i];
#endif
}


void F3(u_int8 *W,u_int8 *N1,u_int8 *N2, u_int8 *R, u_int8 *IOcap, u_int8 *A1, u_int8 *A2, u_int8* OutPut)
{
	// W is 192 bits = 24 Bytes = 6 Words
	// N1 is 128 bits = 16 Bytes = 4 Words
    // N2 is 128 bits = 16 Bytes = 4 Words
	// R is  128 bits = 16 Bytes = 4 Words
	// IOcap is 24 bits = 4 Bytes = 1 Word
	// A1 is 48 bits = 6 Bytes
	// A2 is 48 bits = 6 Bytes 

	u_int8 IntMessage1[32];
	u_int8 IntMessage2[48];
	u_int8 IntMessage3[51];
	u_int8 IntMessage4[57];
	u_int8 Message[63];
	u_int8 *tempPtr;
	int i;

	hashConcat(N1,(16*8),N2,(16*8),IntMessage1);
	hashConcat(IntMessage1,(32*8),R,(16*8),IntMessage2);
	hashConcat(IntMessage2,(48*8),IOcap,(3*8),IntMessage3);
	hashConcat(IntMessage3,(51*8),A1,(6*8),IntMessage4);
	hashConcat(IntMessage4,(57*8),A2,(6*8),Message);

    tempPtr = HMAC(W,192,Message,(63*8));

#if 1
	for (i=0;i<16;i++)
		OutPut[i] = tempPtr[i];
#endif
}




u_int8* HMAC(u_int8* Key,int Key_Len,u_int8 *message,int message_len)
{
	// HMACk(m) = h((K|opad)||h((K|ipad)||m))
	// where h is a cryptographic hash function, K is a secret key padded to the right with extra zeros to the block size of a
	// the hash function.
	// 

	u_int8 opad[SHA_256_BlockSize_In_Bytes]; // Outer Pad
	u_int8 ipad[SHA_256_BlockSize_In_Bytes]; // Inner Pad
	u_int8 concatIpadAndMessage[SHA_256_BlockSize_In_Bytes*2];
	u_int8 concatOpadAndHash1[SHA_256_BlockSize_In_Bytes*2];
	u_int32 concatIpadAndMessageLength;
	u_int32 concatOpadAndHash1_len;
	u_int32 opad_len;
	u_int32 ipad_len;
//	u_int32* OutPut_Hash2=0;
	u_int8 Hash1[32];
	u_int8 Hash2[32];
	u_int32 OutPut_Hash1_Len;

	int i;

	for(i=0;i<SHA_256_BlockSize_In_Bytes;i++)
	{
 		opad[i] = 0x5c;
		ipad[i] = 0x36;
	}

	ipad_len = SHA_256_BlockSize_In_Bytes;
	opad_len = SHA_256_BlockSize_In_Bytes;


	for(i=0;i<(Key_Len/8);i++)
	{
		ipad[i] = ipad[i] ^ Key[i];
		opad[i] = opad[i] ^ Key[i];
	}

	concatIpadAndMessageLength = hashConcat(ipad,(ipad_len*8),message,message_len,concatIpadAndMessage);
	
	OutPut_Hash1_Len = SHA_256(concatIpadAndMessage,concatIpadAndMessageLength,Hash1);

	concatOpadAndHash1_len = hashConcat(opad,(opad_len*8),Hash1,OutPut_Hash1_Len,concatOpadAndHash1);

    SHA_256(concatOpadAndHash1,concatOpadAndHash1_len,Hash2);

	for (i=0;i<32;i++)
       GenericHashOutput[i] = Hash2[i];

	return GenericHashOutput; // OutPut_Hash2;

}


u_int32 hashConcat(u_int8* Message1,u_int32 Message1_Len,u_int8* Message2, u_int32 Message2_Len,u_int8* newConcatBuffer)
{

	u_int32 i;
	char* concatOutput;

	concatOutput = (char*) newConcatBuffer;

	for(i=0; i < (Message1_Len/8); i++)
	{
		newConcatBuffer[i] = Message1[i];
	}

	for (i=0; i < (Message2_Len/8);i++)
	{
		newConcatBuffer[i+(Message1_Len/8)] = Message2[i];
	}

	return (Message1_Len + Message2_Len);
}




// First test - lets assume Message < 512 bytes


u_int32 SHA_256(u_int8* Message,u_int32 MessageLengthInBits,u_int8* Hash)
{
	u_int8 M[64];
	u_int32* M1;
	u_int32 S[8];
	u_int8* pMessage;
	u_int32 W[64];
	u_int32  t0, t1, t;
    u_int32 i;
	u_int32 h0,h1,h2,h3,h4,h5,h6,h7;
	u_int32 NumFullChunks;
	u_int32 RemainingBits;
	u_int8 SeperateChunkForLength;
	u_int32 N;

#define Ch(x,y,z)       (z ^ (x & (y ^ z)))
#define Maj(x,y,z)      (((x | y) & z) | (x & y)) 
#define S(x, n)         RORc((x),(n))
#define R(x, n)         (((x)&0xFFFFFFFFUL)>>(n))
#define Sigma0(x)       (S(x, 2) ^ S(x, 13) ^ S(x, 22))
#define Sigma1(x)       (S(x, 6) ^ S(x, 11) ^ S(x, 25))
#define Gamma0(x)       (S(x, 7) ^ S(x, 18) ^ R(x, 3))
#define Gamma1(x)       (S(x, 17) ^ S(x, 19) ^ R(x, 10))

#define RND(a,b,c,d,e,f,g,h,i)                         \
     t0 = h + Sigma1(e) + Ch(e, f, g) + K[i] + W[i];   \
     t1 = Sigma0(a) + Maj(a, b, c);                    \
     d += t0;                                          \
     h  = t0 + t1;

//Initialize variables
//(first 32 bits of the fractional parts of the square roots of the first 8 primes 2..19):

	h0 = 0x6a09e667;
	h1 = 0xbb67ae85;
	h2 = 0x3c6ef372;
	h3 = 0xa54ff53a;
	h4 = 0x510e527f;
	h5 = 0x9b05688c;
	h6 = 0x1f83d9ab;
	h7 = 0x5be0cd19;


// append the bit '1' to the message
// append k bits '0', where k is the minimum number >= 0 such that the resulting message
//    length (in bits) is congruent to 448 (mod 512)
// append length of message (before pre-processing), in bits, as 64-bit big-endian integer

	// Set up a pointer so we can access the message as a Byte array
	pMessage = (u_int8*)Message;

	// First Determine how many chunks N are going to be in the message.

	NumFullChunks = MessageLengthInBits/512;
	RemainingBits = MessageLengthInBits - (NumFullChunks*512);

	if (RemainingBits < 448)
	{
		// Need to append 1, Zero Pad and add the length
		NumFullChunks++;
		SeperateChunkForLength = 0;
	}
	else
	{
		// Need to add two chunks. One for the last bit of message and one for the length
		NumFullChunks += 2;
		SeperateChunkForLength = 1;
	}

	/*********************************************************************************
	 * STEP 1 - PreProcessing
	 *
	 * Padding the message. A 1 is added after the last bit of the message and the
	 * remainder of the block is padded with zeros. The length of the message is then
	 * inserted in the last 64 bits of the block.
	 *
	 * Actually we only need to do this for the last chunk of the message
	 *
	 *
	 *********************************************************************************/

	/*********************************************************************************
	 * STEP 2 - PreProcessing
	 *
	 * Break the message into N * 512 bit blocks. ( 16 * 32bit words)
	 *
	 * This may occur before Step 1 in our implementation.
	 *
	 *
	 *********************************************************************************/

	M1 = (u_int32*)M;

	N= 0;

	/*********************************************************************************/

    RemainingBits = MessageLengthInBits;
	// Start of loop through Chunks 

	while(NumFullChunks)
	{ 
		// Firstly is this the last chunk.
		if (((NumFullChunks == 2) && (SeperateChunkForLength == 1)) ||
			((NumFullChunks == 1) && (SeperateChunkForLength == 0)) )
		{
			// Need to Add 1 and Pad with Zero.
			// 
			for(i=0;i<(RemainingBits/8);i++)
			{
				//M[i] = pMessage[i];
				M[i] = pMessage[(N*64)+i];
			}
			M[(RemainingBits/8)] = 0x80;

			for(i=((RemainingBits/8)+1);i<64;i++)
			{
				M[i] = 0x00;
			}
			RemainingBits = 0;


			if((NumFullChunks == 1) && (SeperateChunkForLength == 0))
			{
		M[60] = ((MessageLengthInBits & 0xFF000000) >> 24);
		M[61] = ((MessageLengthInBits & 0xFF0000) >> 16);
		M[62] = ((MessageLengthInBits & 0xFF00) >> 8);
		M[63] =  (MessageLengthInBits & 0xFF);
			}

		}
		else if ((NumFullChunks == 1) && (SeperateChunkForLength == 1))
		{
			// Zero Pad and insert the length field
			for(i=0;i<64;i++)
			{
				M[i] = 0x00;
			}
		M[60] = ((MessageLengthInBits & 0xFF000000) >> 24);
		M[61] = ((MessageLengthInBits & 0xFF0000) >> 16);
		M[62] = ((MessageLengthInBits & 0xFF00) >> 8);
		M[63] =  (MessageLengthInBits & 0xFF);
				
		}
		else // Normal 512 bit chunk
		{
			for(i=0;i<64;i++)
			{
				M[i] = pMessage[(N*64)+i];
			}
			RemainingBits -= 512;
		}
        N++;
		NumFullChunks--;

		// Now the actual HASH function
		// copy the first 16 words into an array of 64 Words

		for(i=0;i<16;i++)
		{
            W[i] = ((M[i*4] << 24) & 0xFF000000) + ((M[(i*4)+1] << 16) & 0x00FF0000) + ((M[(i*4)+2] << 8) & 0x00FF00) + (M[(i*4)+3] & 0xFF);
		}
		//Extend the sixteen 32-bit words into sixty-four 32-bit words


		for (i = 16; i < 64; i++) 
		{
			W[i] = Gamma1(W[i - 2]) + W[i - 7] + Gamma0(W[i - 15]) + W[i - 16];
		}

		// compress

		S[0] = h0;
		S[1] = h1;
		S[2] = h2;
		S[3] = h3;
		S[4] = h4;
		S[5] = h5;
		S[6] = h6;
		S[7] = h7;

		for (i = 0; i < 64; ++i) 
		{
			RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],i);
			t = S[7];
			S[7] = S[6];
			S[6] = S[5];
			S[5] = S[4]; 
			S[4] = S[3];
			S[3] = S[2];
			S[2] = S[1];
			S[1] = S[0];
			S[0] = t;
		}

		h0 += S[0];
		h1 += S[1];
		h2 += S[2];
		h3 += S[3];
		h4 += S[4];
		h5 += S[5];
		h6 += S[6];
		h7 += S[7];

	}
	//Start of loop through Chunks 

	// Lets assume one block only so return here

	Hash[3] = h0 & 0x000000FF;
	Hash[2] = (h0 & 0x0000FF00) >> 8;
	Hash[1] = (h0 & 0x00FF0000) >> 16;
	Hash[0] = (h0 & 0xFF000000) >> 24;

	Hash[7] = h1 & 0x000000FF;
	Hash[6] = (h1 & 0x0000FF00) >> 8;
	Hash[5] = (h1 & 0x00FF0000) >> 16;
	Hash[4] = (h1 & 0xFF000000) >> 24;

	Hash[11] = h2 & 0x000000FF;
	Hash[10] = (h2 & 0x0000FF00) >> 8;
	Hash[9] = (h2 & 0x00FF0000) >> 16;
	Hash[8] = (h2 & 0xFF000000) >> 24;

	Hash[15] = h3 & 0x000000FF;
	Hash[14] = (h3 & 0x0000FF00) >> 8;
	Hash[13] = (h3 & 0x00FF0000) >> 16;
	Hash[12] = (h3 & 0xFF000000) >> 24;

	Hash[19] = h4 & 0x000000FF;
	Hash[18] = (h4 & 0x0000FF00) >> 8;
	Hash[17] = (h4 & 0x00FF0000) >> 16;
	Hash[16] = (h4 & 0xFF000000) >> 24;

	Hash[23] = h5 & 0x000000FF;
	Hash[22] = (h5 & 0x0000FF00) >> 8;
	Hash[21] = (h5 & 0x00FF0000) >> 16;
	Hash[20] = (h5 & 0xFF000000) >> 24;

	Hash[27] = h6 & 0x000000FF;
	Hash[26] = (h6 & 0x0000FF00) >> 8;
	Hash[25] = (h6 & 0x00FF0000) >> 16;
	Hash[24] = (h6 & 0xFF000000) >> 24;

	Hash[31] = h7 & 0x000000FF;
	Hash[30] = (h7 & 0x0000FF00) >> 8;
	Hash[29] = (h7 & 0x00FF0000) >> 16;
	Hash[28] = (h7 & 0xFF000000) >> 24;

	// Return length in Bits
	return 32*8;
}
#if 0
void Test_HMAC_SHA_256(void)
{

    // These are published test cases For HMAC.
	// In addition to the final output they also allow us to check the intermediate outputs

	u_int8 Key_TC1[32] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
		0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F };
					   //   0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
						//  0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F };

	u_int8 Data_TC1[34] = {'S','a','m','p','l','e',' ','m','e','s','s','a','g','e',' ','f','o','r',
		                   ' ','k','e','y','l','e','n','<','b','l','o','c','k','l','e','n'};
		
	HMAC(Key_TC1,(32*8),Data_TC1,(34*8));

}
#endif
#else


#endif
