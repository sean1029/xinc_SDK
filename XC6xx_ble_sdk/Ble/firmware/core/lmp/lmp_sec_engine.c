/******************************************************************************
 *
 * MODULE NAME:    lmp_security_engine.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LMP Security Engine
 * MAINTAINER:     Daire McNamara
 * CREATION DATE:  18 February 2000
 *
 * SOURCE CONTROL: $Id: lmp_sec_engine.c,v 1.7 2004/07/07 14:22:09 namarad Exp $
 *
 * LICENSE:
 *   This source code is copyright (c) 2000-2004 Ceva Inc.
 *   All rights reserved.
 *    
 ****************************************************************************/

#include "sys_config.h"
#include "lmp_sec_engine.h"

#ifdef TEST_LMP_SEC_ENG
#include "test_sec_eng.h"
#endif

#ifdef LOG_LMP_SEC_ENG
#include "lmp_test_harn.h"

#endif

static void _Array_Xor(u_int8 size, u_int8* result, 
    u_int8* array1, u_int8* array2);
static void _Array_Copy(u_int8 size, u_int8* dest, const u_int8* src);
static void _Keyed_Hash(t_rand randinput, 
    t_link_key key, t_byte_bd_addr address, u_int32 addresslength, 
    t_link_key output);
static void _Mod2_Multiply_Kc(u_int32 *kcReg, u_int32 const *g2Reg, 
    u_int8 g2ByteLength, u_int32 *productReg);
static void _Mod2_Reduce_Kc(u_int32 *kcReg, const polynomial *g1);
static void _Byte_To_Reg_Array(u_int8 *byteArray, u_int32 *regArray);
static void _Reg_To_Byte_Array(u_int32 *regArray, u_int8 *byteArray);
static void _Safer_Plus_Encrypt(t_rand randinput, t_link_key inputkey, 
    t_link_key output, saferplustag version);


/* initialise tables for powers and logs of 45 */
  
const static u_int8 exparray[256] = 
{ 
 1,   45, 226, 147, 190,  69,  21, 174, 120,   3, 135, 164, 184,  56, 207,  63, 
 8,  103,   9, 148, 235,  38, 168, 107, 189,  24,  52,  27, 187, 191, 114, 247,
 64,  53,  72, 156,  81,  47,  59,  85, 227, 192, 159, 216, 211, 243, 141, 177,
255, 167,  62, 220, 134, 119, 215, 166,  17, 251, 244, 186, 146, 145, 100, 131, 
241,  51, 239, 218,  44, 181, 178,  43, 136, 209, 153, 203, 140, 132,  29,  20, 
129, 151, 113, 202,  95, 163, 139,  87,  60, 130, 196,  82,  92,  28, 232, 160, 
  4, 180, 133,  74, 246,  19,  84, 182, 223,  12,  26, 142, 222, 224,  57, 252, 
 32, 155,  36,  78, 169, 152, 158, 171, 242,  96, 208, 108, 234, 250, 199, 217,
  0, 212,  31, 110,  67, 188, 236,  83, 137, 254, 122,  93,  73, 201,  50, 194,
249, 154, 248, 109,  22, 219,  89, 150,  68, 233, 205, 230,  70,  66, 143,  10,
193, 204, 185, 101, 176, 210, 198, 172,  30,  65,  98,  41,  46,  14, 116,  80,
  2,  90, 195,  37, 123, 138,  42,  91, 240,   6,  13,  71, 111, 112, 157, 126,
 16, 206,  18,  39, 213,  76,  79, 214, 121,  48, 104,  54, 117, 125, 228, 237,
128, 106, 144,  55, 162,  94, 118, 170, 197, 127,  61, 175, 165, 229,  25,  97,
253,  77, 124, 183,  11, 238, 173,  75,  34, 245, 231, 115,  35,  33, 200,   5,
225, 102, 221, 179,  88, 105,  99,  86,  15, 161,  49, 149,  23,   7,  58,  40
  };

const static u_int8 logarray[256] =  
{
128,   0, 176,   9,  96, 239, 185, 253,  16,  18, 159, 228, 105, 186, 173, 248, 
192,  56, 194, 101,  79,   6, 148, 252,  25, 222, 106,  27,  93,  78, 168, 130, 
112, 237, 232, 236, 114, 179,  21, 195, 255, 171, 182,  71,  68,   1, 172,  37, 
201, 250, 142,  65,  26,  33, 203, 211,  13, 110, 254,  38,  88, 218,  50,  15, 
 32, 169, 157, 132, 152,   5, 156, 187,  34, 140,  99, 231, 197, 225, 115, 198, 
175,  36,  91, 135, 102,  39, 247,  87, 244, 150, 177, 183,  92, 139, 213,  84, 
121, 223, 170, 246,  62, 163, 241,  17, 202, 245, 209,  23, 123, 147, 131, 188, 
189,  82,  30, 235, 174, 204, 214,  53,   8, 200, 138, 180, 226, 205, 191, 217, 
208,  80,  89,  63,  77,  98,  52,  10,  72, 136, 181,  86,  76,  46, 107, 158, 
210,  61,  60,   3,  19, 251, 151,  81, 117,  74, 145, 113,  35, 190, 118,  42, 
 95, 249, 212,  85,  11, 220,  55,  49,  22, 116, 215, 119, 167, 230,   7, 219, 
164,  47,  70, 243,  97,  69, 103, 227,  12, 162,  59,  28, 133,  24,   4,  29, 
 41, 160, 143, 178,  90, 216, 166, 126, 238, 141,  83,  75, 161, 154, 193,  14, 
122,  73, 165,  44, 129, 196, 199,  54,  43, 127,  67, 149,  51, 242, 108, 104, 
109, 240,   2,  40, 206, 221, 155, 234,  94, 153, 124,  20, 134, 207, 229,  66, 
184,  64, 120,  45,  58, 233, 100,  31, 146, 144, 125,  57, 111, 224, 137,  48
  };

  /* Matrix used in SAFER+ Algorithm */
const static u_int8 matrix[LINK_KEY_SIZE*LINK_KEY_SIZE]=  
{
 2, 2, 1, 1, 16, 8, 2, 1, 4, 2, 4, 2, 1, 1, 4, 4,
 1, 1, 1, 1, 8, 4, 2, 1, 2, 1, 4, 2, 1, 1, 2, 2,
 1, 1, 4, 4, 2, 1, 4, 2, 4, 2, 16, 8, 2, 2, 1, 1, 
 1, 1, 2, 2, 2, 1, 2, 1, 4, 2, 8, 4, 1, 1, 1, 1,
 4, 4, 2, 1, 4, 2, 4, 2, 16, 8, 1, 1, 1, 1, 2, 2,
 2, 2, 2, 1, 2, 1, 4, 2, 8, 4, 1, 1, 1, 1, 1, 1, 
 1, 1, 4, 2, 4, 2, 16, 8, 2, 1, 2, 2, 4, 4, 1, 1,
 1, 1, 2, 1, 4, 2, 8, 4, 2, 1, 1, 1, 2, 2, 1, 1,
 2, 1, 16, 8, 1, 1, 2, 2, 1, 1, 4, 4, 4, 2, 4, 2,
 2, 1, 8, 4, 1, 1, 1, 1,  1, 1, 2, 2, 4, 2, 2, 1, 
 4, 2, 4, 2, 4, 4, 1, 1, 2, 2, 1, 1, 16, 8, 2, 1,
 2, 1, 4, 2, 2, 2, 1, 1, 1, 1, 1, 1, 8, 4, 2, 1,
 4, 2, 2, 2, 1, 1, 4, 4, 1, 1, 4, 2, 2, 1, 16, 8,
 4, 2, 1, 1, 1, 1, 2, 2, 1, 1, 2, 1, 2, 1, 8, 4,
16, 8, 1, 1, 2, 2, 1, 1, 4, 4, 2, 1, 4, 2, 4, 2,
 8, 4, 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 2, 1, 4, 2  };


/******************************************************************************
 * FUNCTION : - LM_SecEng_Authenticate_E1
 *
 * INPUTS : - randinput, key, bd_addr
 * OUTPUTS : - sres, aco
 *
 * DESCRIPTION : - Performs BT authentication, calculating SRES and ACO, where
 *  SRES = Hash(key, randinput, bd_addr, 6)[0,...,3]
 *  ACO  = Hash(key, randinput, bd_addr, 6)[4,...,15]
 *
 * SRES is the value compared in authentication.
 * ACO is used as an input in the Encryption Key routine - Generate_Key_E3()
 *
 *****************************************************************************/

void LM_SecEng_Authenticate_E1(t_rand randinput, t_link_key key,
        const t_bd_addr* p_bd_addr, t_sres sres, t_aco aco)
{
    t_link_key output;
    t_byte_bd_addr byte_bd_addr;

    /*
     *Convert from type "t_bd_addr", which contains bit fields to straight
     *array of type "t_byte_bd_addr"
     */
    BDADDR_Get_Byte_Array_Ex(p_bd_addr, byte_bd_addr);

    _Keyed_Hash(randinput, key, byte_bd_addr, BYTE_BD_ADDR_SIZE, output);

    _Array_Copy(SRES_SIZE, sres, output);
    _Array_Copy(ACO_SIZE, aco, &(output[SRES_SIZE])); 
}

        
/******************************************************************************
 * FUNCTION : - LM_SecEng_Generate_Key_E21
 *
 * INPUTS : - randinput, bd_addr
 * OUTPUTS : - outputkey
 *
 * DESCRIPTION : - Generate link key used for authentication. This routine 
 *    applies to both unit and combination keys. The E21 function is 
 *    defined as follows:
 *    E21: outputkey = Ar'(X, Y)
 *    where:
 *    X = randinput[0..14]U(randinput[15]XOR6)
 *    Y = U bd_addr[i(mod 6)] for i=0..15
 *
 *****************************************************************************/

void LM_SecEng_Generate_Key_E21(t_rand randinput, const t_bd_addr* p_bd_addr, 
    t_link_key output_key)
{
    t_link_key key;
    t_rand input;
    t_byte_bd_addr byte_bd_addr;
    int i, k;

    /* Convert from type "t_bd_addr", which contains bit fields to 
       straight array - "t_byte_bd_addr" */
    BDADDR_Get_Byte_Array_Ex(p_bd_addr, byte_bd_addr);

    _Array_Copy(LINK_KEY_SIZE-1,key,randinput);
    key[15] = randinput[15] ^ 6;

    k = 0;
    for(i=0; i<LINK_KEY_SIZE; ++i)
    {
        input[i] = byte_bd_addr[k];
        if(k==BYTE_BD_ADDR_SIZE-1)
            k = 0;
        else
            k++;
    }

    _Safer_Plus_Encrypt(input, key, output_key, AR_MOD);
}


/*****************************************************************************
 * FUNCTION : - LM_SecEng_Generate_Key_E22
 *
 * INPUTS : - randinput, PIN, lengthPIN, bd_addr
 * OUTPUTS : - outputkey
 *
 * DESCRIPTION : - Generate link key used for authentication. This 
 *     routine applies to Kinit and Kmaster generation. E22 is defined 
 *     as follows:
 *     E22: outputkey = Ar'(X,Y) 
 *     where:
 *     X = U PIN'[i(mod L')] for i=0..15
 *     Y = randinput[0..14] U (randinput[15]XOR L')
 *     PIN' = PIN[0..lengthPIN-1] U bd_addr[0..min{5, 15-lengthPIN}], 
 *            lengthPIN < 16
 *          = PIN[0..lengthPIN-1], lengthPIN = 16
 *     L' = min{16, lengthPIN+6}
 *
 *****************************************************************************/
void LM_SecEng_Generate_Key_E22(const t_rand randinput, t_pin_code PIN,
    u_int8 lengthPIN, const t_bd_addr* p_bd_addr, t_link_key outputkey)
{
    t_byte_bd_addr byte_bd_addr;
    t_link_key inputkey;
    int i,k, lengthPINAug =0;
    t_rand tempRandInput;

    _Array_Copy(lengthPIN,inputkey,PIN);
    _Array_Copy(RANDOM_NUM_SIZE, tempRandInput, randinput);

    /* let lengthPIN = L' */
    if(lengthPIN+6 < 16)
        lengthPINAug = lengthPIN + 6;
    else
        lengthPINAug = 16;
    /* Augment PIN: If PIN is less than 16 bytes, append the bd_addr. 
       If the resulting  */
    /* length (lengthPINAug) is less than 16 byte, fill the remaining 
       bytes of inputkey[i] with inputkey[i%lengthPINAug] */
    if ( lengthPIN < LINK_KEY_SIZE )
    {
        BDADDR_Get_Byte_Array_Ex(p_bd_addr, byte_bd_addr);
        for(i=lengthPIN; i<lengthPINAug; ++i)
            inputkey[i] = byte_bd_addr[i-lengthPIN];
    }

    if( lengthPINAug < LINK_KEY_SIZE)
    {
        k = 0;
        for(i=0; i<LINK_KEY_SIZE; ++i)
        {
            inputkey[i] = inputkey[k];
            if(k==lengthPINAug-1)
                k = 0;
            else
                k++;
        }
    }

    tempRandInput[15] ^= (u_int8)lengthPINAug;
    _Safer_Plus_Encrypt(tempRandInput, inputkey, outputkey, AR_MOD);
}

/******************************************************************************
 * FUNCTION : - LM_SecEng_Generate_Key_E3
 *
 * INPUTS : - randinput, cof, key
 * OUTPUTS : - output
 *
 * DESCRIPTION : - Generate Encryption Key from current link key (key). 
 *     This routine also uses the keyed Hash routine:
 * 
 *  output = Hash(key, randinput, cof)
 * 
 ****************************************************************************/
void LM_SecEng_Generate_Key_E3(t_rand encr_rand,t_cof cof,
    t_link_key link_key,t_rand  encr_key)
{
    _Keyed_Hash(encr_rand, link_key, cof, COF_SIZE, encr_key);
}


/******************************************************************************
 * FUNCTION : - LM_SecEng_Generate_Kc_Prime
 *
 * INPUTS : - kc, kc_prime_len
 * OUTPUTS : - kc_prime
 *
 * DESCRIPTION : - Takes a 16 bit encryption key as produced from E3 and 
 *     generates the actual encryption key based on desired effective length 
 *     in bytes (1 -16). The key length is first reduced by a modulo 
 *     operation between kc and a polynomial (g1) of desired degree. The key 
 *     is multiplied by another polynomial (g2). The equation is:
 *     kc_prime = g2(kc mod g1), where 
 *         g1 and g2 are chosen based on kc_prime_len
 * 
 *****************************************************************************/
void LM_SecEng_Generate_Kc_Prime(u_int8 * kc, u_int8 kc_prime_len, 
    u_int8 *kc_prime)
{
    const static polynomial arrayG1Poly[15] =
    {
    { 0x001d, 8 },
    { 0x003f, 16},
    { 0x00db, 24},
    { 0x00af, 32},
    { 0x0039, 40},
    { 0x0291, 48},
    { 0x0095, 56},
    { 0x001b, 64},
    { 0x0609, 72},
    { 0x0215, 80},
    { 0x013b, 88},
    { 0x00dd, 96},
    { 0x049d, 104},
    { 0x014f, 112},
    { 0x00e7, 120}
    };

    static const u_int32 arrayG2Poly[37] = 
    {
    0x00e275a0, 0xabd218d4, 0xcf928b9b, 0xbf6cb08f,
    0x0001e3f6, 0x3d7659b3, 0x7f18c258, 0xcff6efef,
    0x000001be, 0xf66c6c3a, 0xb1030a5a, 0x1919808b,
    0x00000001, 0x6ab89969, 0xde17467f, 0xd3736ad9,
    0x01630632, 0x91da50ec, 0x55715247, 
    0x00002c93, 0x52aa6cc0, 0x54468311,
    0x000000b3, 0xf7fffce2, 0x79f3a073,
    0xa1ab815b, 0xc7ec8025,
    0x0002c980, 0x11d8b04d,
    0x0000058e, 0x24f9a4bb,
    0x0000000c, 0xa76024d7,
    0x1c9c26b9,
    0x0026d9e3,
    0x00004377,
    0x00000089,
    };

    static const u_int8 g2PolyByteLen[15] = {15, 15, 14, 13, 12, 10, 9, 
        8, 7, 6, 5, 4, 3, 2, 1};
    static const u_int8 g2PolyByteOffset[16] = {0, 4, 8, 12, 16, 19, 22, 
        25, 27, 29, 31, 33, 34, 35, 36, 37}; 

    u_int32 const *g2Ptr;
    u_int32 KcRegArray[MAX_KEY_REG_SIZE];
    u_int32 KcPrimeRegArray[MAX_KEY_REG_SIZE]; 

    /* If the encryption key length is 16, kc_prime = kc */
    if(MAX_KEY_SIZE == kc_prime_len)
    {
#ifdef TEST_LMP_SEC_ENG
        LM_Test_SecEng_Print_Len16_Polys();
#endif
        _Array_Copy(MAX_KEY_SIZE, kc_prime, kc);
    }
    else
    {
        _Byte_To_Reg_Array(kc, KcRegArray);
        _Mod2_Reduce_Kc(KcRegArray, &(arrayG1Poly[kc_prime_len-1]));
        /* Find the polynomial g2Ptr */
        g2Ptr = arrayG2Poly + g2PolyByteOffset[kc_prime_len-1];
#ifdef TEST_LMP_SEC_ENG
        LM_Test_SecEng_Print_G1(&(arrayG1Poly[kc_prime_len-1]));
        LM_Test_SecEng_Print_G2(g2Ptr, g2PolyByteLen[kc_prime_len-1]);
#endif
        _Mod2_Multiply_Kc(KcRegArray, g2Ptr, g2PolyByteLen[kc_prime_len-1], 
            KcPrimeRegArray);
        _Reg_To_Byte_Array(KcPrimeRegArray, kc_prime);
    }
}


/******************************************************************************
 * FUNCTION : - _Keyed_Hash
 *
 * INPUTS : - randinput, key, address, addresslength
 * OUTPUTS : - output
 *
 * DESCRIPTION : - Implements the Hash function, defined in 
 *     BT spec (pg. 171) as:
 *     Hash:{0,1}128 x {0,1}128 x {0,1}8*addresslength x 
 *         {addresslength=6 or 12} -> {0,1}128
 *     (key, randinput, address, addresslength) |-> A'r([key`],
 *         [E(address, addresslength)+(Ar(key, randinput) XOR randinput)])
 *
 *     where key' is offsetted from key
 *
 *     and where
 *     E:{0,1}8*addresslength x {addresslength=6 or 12} -> {0,1}128
 *         (X[0, ..., addresslength-1], addresslength) |-> 
 *             (X[i(mod adddresslength)] for i = 0..15)
 *                                                
 * When this function is called in E1(authentication function) the 
 * addresslength = 6 (address = BD_ADDR)
 * When this function is called in E3(Encryption Key gen.) the 
 * addresslength = 12 (address = COF)
 * 
 *****************************************************************************/
void _Keyed_Hash(t_rand randinput, t_link_key key, t_byte_bd_addr address, 
    u_int32 addresslength, t_link_key output)
{
    u_int32 i;
    t_link_key tempinput, keyoffset;

    _Safer_Plus_Encrypt(randinput, key, tempinput, AR_NORMAL);

    _Array_Xor(LINK_KEY_SIZE,tempinput,tempinput,randinput);

    for(i=0; i<LINK_KEY_SIZE; ++i)
        tempinput[i] += address[i%addresslength];

    keyoffset[0] = key[0] + 233;
    keyoffset[1] = key[1] ^ 229;
    keyoffset[2] = key[2] + 223;
    keyoffset[3] = key[3] ^ 193;
    keyoffset[4] = key[4] + 179;
    keyoffset[5] = key[5] ^ 167;
    keyoffset[6] = key[6] + 149;
    keyoffset[7] = key[7] ^ 131;

    keyoffset[8] = key[8] ^ 233;
    keyoffset[9] = key[9] + 229;
    keyoffset[10] = key[10] ^ 223;
    keyoffset[11] = key[11] + 193;
    keyoffset[12] = key[12] ^ 179;
    keyoffset[13] = key[13] + 167;
    keyoffset[14] = key[14] ^ 149;
    keyoffset[15] = key[15] + 131;

    _Safer_Plus_Encrypt(tempinput, keyoffset, output, AR_MOD);
}


/******************************************************************************
 * FUNCTION :- _Mod2_Multiply_Kc
 *
 * INPUTS :-  kcReg, pointer to 4 32-bit registers
 *            g2Reg, pointer to X 32-bit registers, where X = 1-4.
 *            g2ByteLength, length of g2 in bytes.
 *            
 * OUTPUTS :- productReg
 *
 * DESCRIPTION :- Using modulus-2 arithmethic, calculates the product 
 *     of kcReg and g2Reg and places the result in productReg. 
 *
 *****************************************************************************/
void _Mod2_Multiply_Kc(u_int32 *kcReg, u_int32 const *g2Reg, 
    u_int8 g2ByteLength, u_int32 *productReg)
{
    int i, regIndex, j;
    u_int32 bitMask = 1;
    u_int8 bitCount = 0, numG2Bits;
  
    /* Calculate the number of 32 bit registers in the G2 polynomial */
    regIndex = ((g2ByteLength-1)/4);
  
    /* Initialise the product register */
    for(j=0; j<4; j++)
        productReg[j] = 0;
 
    numG2Bits = g2ByteLength*8;
    i=0;
    /* Starting with the least significant 32-bit register, check each bit 
       of the g2 polynomial,
       if set to 1, xor the current value of Kc with the current product 
       value */
    do
    {
        if(g2Reg[regIndex] & bitMask)
        {
            for(j=0; j<4; j++)
                productReg[j] ^= kcReg[j]; 
        }
      
        /* Shift the Kc Register left 1 bit */
        for(j=0; j<3; j++)
            kcReg[j] = (kcReg[j]<<1) + (kcReg[j+1]>>31);
        kcReg[j] <<= 1;

        bitMask <<= 1;      
        bitCount++;
        i++;

        /* When one whole 32-bit register has been checked, reset the 
           bitmask and move to previous register */
        if(i==32)
        {
            bitMask = 1;
            regIndex--;
            i=0;
        }
    }while(bitCount<numG2Bits);
}


/******************************************************************************
 * FUNCTION :- _Mod2_Reduce_Kc
 *
 * INPUTS :-  kcReg, pointer to 4 32-bit registers
 *            g1, pointer to a polynomial
 *
 * DESCRIPTION: - Reduces the length of kcReg by a modulo operation 
 *     between kcReg and the polynomial g1. 
 *
 * ALGORITHM :-     Shift left g1 until degree(g1') == degree(kc) 
 *                  currBit = MSB(kc)
 *                  DO
 *                     IF(currBit(Kc)==1)
 *                          kc = kc ^ g1
 *                     currBit(Kc) = 0
 *                     Shift g1 1 bit right.
 *                     currBit = next bit to the right(kc).
 *                     degree(kc) = degree(kc)-1
 *                  WHILE( degree(kc) >= degree(g1)), where degree(g1) 
 *                      is the original degree of the polynomial g1.
 *
 ****************************************************************************/
void _Mod2_Reduce_Kc(u_int32 *kcReg, const polynomial *g1)
{
    int regIndexG1;            /* Index of Kc register to be xor'ed 
                                  with g1 register */
    u_int8 regIndexBitChk=0;   /* Index of Kc register being tested for 1's */
    u_int8 kcDegree = MAX_DEGREE_KC; /* Initial degree of Kc */
    u_int32 bitMask;                 /* Bit Mask used to test for 1's in Kc */
    u_int8 bitPos = 0;
    u_int16 g1UpperHalf=0, g1LowerHalf=0; /* Two 16-bit registers used to 
                                             store value of g1 to be xor'ed 
                                             with Kc */
    int g1Offset = 0;          /* Position of G1 value in relation to its 
                                  original position - left = +ive, 
                                  right = -ive. */

    /* Initialise bit mask to bit 31 */
    bitMask = ((u_int32)1)<<31;

    /* Initial index of Kc reg to be xor'ed with the g1 value*/
    if( g1->degree >= 16)
        regIndexG1 = ((g1->degree)-16)/32;
    else
        regIndexG1 = -1;

    /* Get bit position of MSB(g1 value) */
    bitPos = g1->degree;
    while(bitPos >= 32)
        bitPos -= 32;

    /* Shift g1 value left until MSB(g1 value) is 31 and get initial g1Offset */
    g1LowerHalf = g1->value;
    while(bitPos < 31)
    {
        g1UpperHalf = ((g1UpperHalf<<1) + (g1LowerHalf>>15)) & 0xFFFF;
        g1LowerHalf = (g1LowerHalf<<1) & 0xFFFF; 
        g1Offset++;

        if(g1Offset==16)
        {
            g1LowerHalf = g1UpperHalf;
            g1UpperHalf = 0;
        }
      
        bitPos++;
 
    }
    if(g1Offset>15)
        g1Offset-=32;
  
    while(kcDegree >= g1->degree)
    {
        if(kcReg[regIndexBitChk] & bitMask) 
        { 
            kcReg[regIndexBitChk]&= ~bitMask; 
            if (g1Offset <= 0 )
            {
                if(regIndexG1>=0)
                    kcReg[regIndexG1] ^= g1UpperHalf;
                if(regIndexG1<MAX_KEY_REG_SIZE-1)
                    kcReg[regIndexG1+1] ^= (g1LowerHalf<<16);
            }
            else
            {
                kcReg[regIndexG1] ^= g1LowerHalf;
                kcReg[regIndexG1] ^= (g1UpperHalf<<16);
            }
        }
        /* Shift g1 register 1 bit right */
        g1LowerHalf = (g1LowerHalf>>1) + ((g1UpperHalf&0x1)<<15);
        g1UpperHalf >>= 1;
        bitMask >>= 1;
        g1Offset--;
     
        /* After this, g1 value (g1UpperHalf) is exactly aligned with 
           lower 16 bits of kc{regIndexG1]
           and g1LowerHalf(=0) is exactly aligned with upper 16 bits 
           of kc[regIndex+1] */
        if(g1Offset==0)
        {
            g1UpperHalf = g1LowerHalf;
            g1LowerHalf = 0;
        }
      
        /* After this g1 value (g1UpperHalf) is exactly aligned with 
           upper 16 bits of kc[regIndexG1]
           and g1LowerHalf(=0) is exactly aligned with lower 16 bits 
           of kc[regIndex], g1Offset is set back to 15 */
        if(g1Offset== -16)
        {
            g1UpperHalf = g1LowerHalf;
            g1LowerHalf = 0;
            g1Offset = 15;
            regIndexG1++;
        }

        /* When a full register of Kc has been checked, increment 
           regIndexBitChk and reset the bitmask */
        if(0 == kcDegree%32)
        {
            regIndexBitChk++;
            bitMask = ((u_int32)1)<<31;
        }

        kcDegree--;
    }
}


/******************************************************************************
 * FUNCTION : - Safer_Plus_Encrypt
 *
 * INPUTS : - randinput, inputkey, output, saferplustag
 * OUTPUTS : - output
 *
 * DESCRIPTION : - Implements the SAFER+ algorithm which produces a 128 bit 
 *     result (ouput) from a 128 bit random input (randinput) and a 128 bit 
 *     key (key). This function corresponds to the Ar function described in 
 *     the BT Spec. (see pg. 173/174). It also implements a modified version 
 *     of Ar, called Ar' - determined by the 'saferplustag' input parameter.
 *
 *****************************************************************************/
void _Safer_Plus_Encrypt(t_rand randinput, t_link_key inputkey, 
    t_link_key output, saferplustag version)
{
    u_int8 keysched[(LINK_KEY_SIZE+1)];
    u_int8 roundkeys[(LINK_KEY_SIZE*((2*NO_ROUNDS)+1))];
    u_int8 matrixtest[LINK_KEY_SIZE];
    int i, p, m;
    int round;
    u_int8 *roundkeyindex, *p_keysched, *p_keysched_end;

    /* KEY SCHEDULING - Generation of Round Keys K1, K2,..., B17 */

    _Array_Copy(LINK_KEY_SIZE,keysched,inputkey);

    p_keysched_end = keysched + LINK_KEY_SIZE;
    *p_keysched_end = 0;
    /*
     * sum octets bit-by-bit modulo two
     */
    p_keysched = keysched;
    roundkeyindex = roundkeys;
    while (p_keysched < p_keysched_end)
    {
        *p_keysched_end ^= *p_keysched;
        *roundkeyindex++ = *p_keysched++;
    }

    /* Rotate each byte left by 3 bits */
    for(p=0; p<(2*NO_ROUNDS); p++)
    {
        u_int key_sched_index;
        u_int round_key_index;
        p_keysched = keysched;
        while (p_keysched <= p_keysched_end)
        {
            *p_keysched = (*p_keysched<<3) | (*p_keysched>>5);
            ++p_keysched;
        }
        /*
         * For Round 1, select bytes 1,2..16:Round 2, 
         * select bytes 2,3..16,0....and so on
         */
        key_sched_index = p;
        round_key_index = (p*LINK_KEY_SIZE)+LINK_KEY_SIZE - 1;
        for(i=0; i<LINK_KEY_SIZE; ++i)
        {
            /*
             * key_sched_index = (p+i+1)%(LINK_KEY_SIZE+1)
             * Since NO_ROUNDS is 8, i < 16, => (p+i+1) <= 31,
             * hence mod 17 can be replaced by substraction 
             * (while - future proofs)
             * round_key_index = (p*LINK_KEY_SIZE)+LINK_KEY_SIZE+i
             */
            ++key_sched_index;
            ++round_key_index;
            while( key_sched_index > LINK_KEY_SIZE)   /* % 17 */
                key_sched_index -= (LINK_KEY_SIZE+1);
            /*
             * The early exist below is redundant since if
             * (p*LINK_KEY_SIZE)+LINK_KEY_SIZE+i > 
             *     (LINK_KEY_SIZE*((2*NO_ROUNDS)+1) -1)
             * => MAX_P*LKS + LKS + MAX_I > LKS * (MAX_P+2) - 1
             * => LKS + MAX_I > 2*LKS -1
             * => LKS + LKS-1 > 2*LKS -1 which is always false
             *
             * if( round_key_index > (LINK_KEY_SIZE*((2*NO_ROUNDS)+1) -1) )
             *    break;
             */
            roundkeys[round_key_index] =
                keysched[ key_sched_index ] +
                (exparray[exparray[(17*(p+2)+i+1) & 0xff]]);
        }
    }

    _Array_Copy(LINK_KEY_SIZE,output,randinput);

    /* ROUND COMPUTATION */

    for(round = 1; round <= NO_ROUNDS; round++)
    {
        roundkeyindex = &(roundkeys[((2*round)-2)*LINK_KEY_SIZE]);

#ifdef TEST_LMP_SEC_ENG
        LM_Test_SecEng_Print_NumRound(round);
        LM_Test_SecEng_Print_Bytes(output, LINK_KEY_SIZE);
#endif

        /*  Modified version of Ar, called Ar' adds the input of 
            round 1 to the input round 3 */
        if((3 == round)&&(AR_MOD == version))
        {
            i=0;
            do
            {
                output[i] ^= randinput[i];
                i++;
                output[i] += randinput[i];
                i++;
                output[i] += randinput[i];
                i++;
                output[i] ^= randinput[i];
                i++;
            } while (i!=16);

#ifdef TEST_LMP_SEC_ENG
            LM_Test_SecEng_Print_String("added ->:");
            LM_Test_SecEng_Print_Bytes(output, LINK_KEY_SIZE);
#endif
        }

#ifdef TEST_LMP_SEC_ENG
        LM_Test_SecEng_Print_NumKey(2*round-1);
        LM_Test_SecEng_Print_Bytes(roundkeyindex, LINK_KEY_SIZE);
        LM_Test_SecEng_Print_NumKey(2*round);
        LM_Test_SecEng_Print_Bytes(roundkeyindex+LINK_KEY_SIZE, LINK_KEY_SIZE);
#endif

        i=0;
        do
        {
            output[i] = exparray[output[i]^roundkeyindex[i]] 
                + roundkeyindex[LINK_KEY_SIZE+i];
            i++;
            output[i] = logarray[(output[i]+roundkeyindex[i]) & 0xff] 
                ^ roundkeyindex[LINK_KEY_SIZE+i];
            i++;
            output[i] = logarray[(output[i]+roundkeyindex[i]) & 0xff] 
                ^ roundkeyindex[LINK_KEY_SIZE+i];
            i++;
            output[i] = exparray[output[i]^roundkeyindex[i]] 
                + roundkeyindex[LINK_KEY_SIZE+i];
            i++;
        } while (i !=16);

        /* Alternatively, use Matrix M defined in Nomination Paper */
        /* Test use of M for the first round */
    
        for(i=0; i<LINK_KEY_SIZE; ++i)
            matrixtest[i] = 0;

        for(i=0; i<LINK_KEY_SIZE; ++i)
        {
            for(m=0; m<LINK_KEY_SIZE; m++)
                matrixtest[i] += output[m]*matrix[(m*LINK_KEY_SIZE)+i];
        }

        _Array_Copy(LINK_KEY_SIZE,output,matrixtest);
    }

    /* After last round - encrypt with RoundKey 17 */
    roundkeyindex = roundkeys + (LINK_KEY_SIZE * 16);

#ifdef TEST_LMP_SEC_ENG
    LM_Test_SecEng_Print_NumKey(17);
    LM_Test_SecEng_Print_Bytes(roundkeyindex, LINK_KEY_SIZE);
#endif

    i=0;
    do
    {
        output[i] ^= roundkeyindex[i];
        i++;
        output[i] += roundkeyindex[i];
        i++;
        output[i] += roundkeyindex[i];
        i++;
        output[i] ^= roundkeyindex[i];
        i++;
    } while (i!=16);
}


/******************************************************************************
 * FUNCTION: _Array_Xor
 *
 ******************************************************************************/
void _Array_Xor(u_int8 size, u_int8* result, u_int8* array1, u_int8* array2)
{
    int i=0;

    do
    {
        result[i] = array1[i] ^ array2[i];
        i++;
    } while (i<size);
}


/******************************************************************************
 * FUNCTION: _Array_Copy
 *
 ******************************************************************************/
void _Array_Copy(u_int8 size, u_int8* dest, const u_int8* src)
{
    int i=0;

    do
    {
        dest[i] = src[i];
        i++;
    } while (i<size);
}


/******************************************************************************
 * FUNCTION: _Byte_To_Reg_Array
 *
 ******************************************************************************/
void _Byte_To_Reg_Array(u_int8 *byteArray, u_int32 *regArray)
{
    int i, j;

#ifndef KC_NOT_REVERSED
    for(j=0; j<=3; j++)
    {
        regArray[j] = 0;
        for(i=0; i<4; i++)
        {
            regArray[j] += (byteArray[(3-j)*4+i])<<(i*8);
        }
    }
#else
    for(j=3; j>=0; j--)
    {
        regArray[j] = 0;
        for(i=3; i>=0; i--)
        {
            regArray[j] += (byteArray[(4*j)+3-i])<<(i*8); 
        }
    }
#endif
}


/******************************************************************************
 * FUNCTION: _Reg_To_Byte_Array
 *
 ******************************************************************************/
void _Reg_To_Byte_Array(u_int32 *regArray, u_int8 *byteArray)
{

    int i, j;
  
#ifndef KC_NOT_REVERSED
    for(j=3; j>=0; j--)
    {
        for(i=3; i>=0; i--)
        {
            byteArray[15-(4*j)-i] = (regArray[j]>>((3-i)*8))&0xFF; 
        }
    }
#else
    for(j=0; j<4; j++)
    {
        for(i=0; i<4; i++)
        {
            byteArray[(4*j)+i] = (regArray[j]>>((3-i)*8))&0xFF;
        }
    }
#endif
}

