
#include 	"rfc.h"
#include    "sys_debug_config.h"
#include    "patch_function_id.h"

extern  void le_rf_init(void);
extern  void SER_WRITE(unsigned int regAddr,unsigned int regValue);
extern  unsigned int SER_READ(unsigned int regAddr,unsigned int *regValue);
extern  void    register_pka_callback(pka_t op);
extern  void    register_pkd_callback(pkd_t op);
extern signed  char rf_tx_power;

rf_freq_t const rf_rx_freq[40] =
{
	{0x96,  0x1},  //2401   
	{0x96,  0x3},  //2403   
	{0x96,  0x5},  //2405   
	{0x96,  0x7},  //2407   
	{0x96,  0x9},  //2409   
	{0x96,  0xB},  //2411   
	{0x96,  0xD},  //2413   
	{0x96,  0xF},  //2415   
	{0x97,  0x1},  //2417   
	{0x97,  0x3},  //2419   
	{0x97,  0x5},  //2421   
	{0x97,  0x7},  //2423   
	{0x97,  0x9},  //2425   
	{0x97,  0xB},  //2427   
	{0x97,  0xD},  //2429   
	{0x97,  0xF},  //2431   
	{0x98,  0x1},  //2433   
	{0x98,  0x3},  //2435   
	{0x98,  0x5},  //2437   
	{0x98,  0x7},  //2439   
	{0x98,  0x9},  //2441   
	{0x98,  0xB},  //2443   
	{0x98,  0xD},  //2445   
	{0x98,  0xF},  //2447   
	{0x99,  0x1},  //2449   
	{0x99,  0x3},  //2451   
	{0x99,  0x5},  //2453   
	{0x99,  0x7},  //2455   
	{0x99,  0x9},  //2457   
	{0x99,  0xB},  //2459   
	{0x99,  0xD},  //2461   
	{0x99,  0xF},  //2463   
	{0x9A,  0x1},  //2465   
	{0x9A,  0x3},  //2467   
	{0x9A,  0x5},  //2469   
	{0x9A,  0x7},  //2471   
	{0x9A,  0x9},  //2473   
	{0x9A,  0xB},  //2475   
	{0x9A,  0xD},  //2477   
	{0x9A,  0xF}   //2479    
};

rf_freq_t const rf_tx_freq[40]=
{
	{0x96,0x2}, //2402 
	{0x96,0x4}, //2404 
	{0x96,0x6}, //2406 
	{0x96,0x8}, //2408 
	{0x96,0xA}, //2410 
	{0x96,0xC}, //2412 
	{0x96,0xE}, //2414 
	{0x97,0x0}, //2416 
	{0x97,0x2}, //2418 
	{0x97,0x4}, //2420 
	{0x97,0x6}, //2422 
	{0x97,0x8}, //2424 
	{0x97,0xA}, //2426 
	{0x97,0xC}, //2428 
	{0x97,0xE}, //2430 
	{0x98,0x0}, //2432 
	{0x98,0x2}, //2434 
	{0x98,0x4}, //2436 
	{0x98,0x6}, //2438 
	{0x98,0x8}, //2440 
	{0x98,0xA}, //2442 
	{0x98,0xC}, //2444 
	{0x98,0xE}, //2446 
	{0x99,0x0}, //2448 
	{0x99,0x2}, //2450 
	{0x99,0x4}, //2452 
	{0x99,0x6}, //2454 
	{0x99,0x8}, //2456 
	{0x99,0xA}, //2458 
	{0x99,0xC}, //2460 
	{0x99,0xE}, //2462 
	{0x9A,0x0}, //2464 
	{0x9A,0x2}, //2466 
	{0x9A,0x4}, //2468 
	{0x9A,0x6}, //2470 
	{0x9A,0x8}, //2472 
	{0x9A,0xA}, //2474 
	{0x9A,0xC}, //2476 
	{0x9A,0xE}, //2478 
	{0x9B,0x0}  //2480 
};

#ifndef REDUCE_ROM
void    rf_pka_operation(void)
{
    SER_WRITE(REG_CTL_0x6c, REG_CTL_0x6c_v0);     
}

void    rf_pkd_operation(void)
{   
    SER_WRITE(REG_CTL_0x6c, REG_CTL_0x6c_v0);    
}
#endif

void	rf_init(void)
{
    //le_rf_init();
    ((void	(*)(void))PATCH_FUN[LE_RF_INIT])();
    
    //register_pka_callback(rf_pka_operation);
    //register_pkd_callback(rf_pkd_operation);
    
#if		(__DEBUG_TEST_RF_XINC == 1)
    extern  void xcrf_read_test(void);
    extern  void xcrf_rw_test(void);
    
    xcrf_read_test();
    
    //xcrf_rw_test();
	//while(1);
#endif	
}
