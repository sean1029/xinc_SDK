#ifndef ECC_HEADER
#define ECC_HEADER
#define u_int32 unsigned int
#define u_int16 unsigned short int
#define u_int8 unsigned char
#define MAX_DIGITS 60
#define MAX_OCTETS 30
typedef struct bigHex 
{
	u_int32 num[MAX_OCTETS/2];
	u_int8  len;
	u_int8  sign;
} bigHex;

typedef struct veryBigHex 
{
	u_int32 num[MAX_OCTETS];
	u_int8  len;
	u_int8  sign;
} veryBigHex;

typedef struct ECC_Point
{
	bigHex x;
	bigHex y;
} ECC_Point;


typedef struct ECC_Operands
{
	int multiply;
	int subtract;
	int add;
	int subtractBig;
	int addBig;
	int invert;
	int modP;
	int specialMod;
	int divideBig;
	int modBig;
	int bigHexModP;
} ECC_Operands;

void ECC_Point_Multiplication(bigHex* pk,ECC_Point* pPointP, ECC_Point* pPointQ);
int notEqual(bigHex *bigHexA,bigHex *bigHexB);
void GF_Point_Copy(ECC_Point *source,ECC_Point *destination);
void initBigNumber(bigHex *BigHexResult);
void initVeryBigNumber(veryBigHex *BigHexResult);

int getNumberLen(bigHex* bigHexA);
void setBigNumberLength( bigHex *BigHexResult);

void LMecc_Generate_ECC_Key(u_int8* secret_key,  u_int8* public_key_x,
					   u_int8* public_key_y,u_int8* result_x, boolean blocking);
#endif