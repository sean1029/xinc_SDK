#include    "sys_debug_config.h"


#if		(__DEBUG_TEST_RF_XINC == 1)

extern  void SER_WRITE(unsigned int regAddr,unsigned int regValue);
extern  unsigned int SER_READ(unsigned int regAddr,unsigned int *regValue);

unsigned int regValue[768]= {0};


//把所有regValue的值都读出来，和excel对比
void xcrf_read_test(void)
{
	unsigned int regAddr;

    for(regAddr=0;regAddr<=767;regAddr++)
    {
			SER_READ(regAddr, &regValue[regAddr]);
    }
}


//看一下regValud是否写进去了
void xcrf_rw_test(void)
{
	unsigned int regAddr;

	regAddr=0x15c/4;
	SER_READ(regAddr, &regValue[0]);
    /*
    SER_READ(0x160, &regValue[3]);
    SER_READ(0x158, &regValue[4]);
    SER_READ(0x154, &regValue[5]);
    SER_READ(0x150, &regValue[6]);
    */
	//2400, 1001 0110 0000 0000
	SER_WRITE(regAddr, 0x9600);
	SER_READ(regAddr, &regValue[1]);
	//{2401, 1001 0110 0001 0000
	SER_WRITE(regAddr, 0x9610);
	SER_READ(regAddr, &regValue[2]);
    
    //test
    SER_WRITE(0x16c/4, 0x6900);
	SER_READ(0x16c/4, &regValue[7]);
}

#endif
