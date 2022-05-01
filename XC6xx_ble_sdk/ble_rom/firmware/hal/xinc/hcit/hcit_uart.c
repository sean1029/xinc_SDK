
#include "sys_config.h"


#if (BUILD_TYPE!=GENERIC_BUILD)

#include "sys_types.h"
#include "sys_features.h"
#include "sys_irq.h"

#include "tra_hcit.h"
#include "tra_uart.h"

#include "hcit_uart.h"
#include "hw_leds.h"
#include "xinc_m0.h"
#include "sys_debug_config.h"

//#define     UART_ERR_DEBUG          //-  debug uart err.
unsigned    int     uart_err_cnt = 0; 
unsigned    int     uart_err_busy = 0; 
unsigned    int     uart_err_default = 0; 

#ifdef      UART_ERR_DEBUG   
#define     error_while()   \
            do {            \
                            \
            }while(1)
#else
#define     error_while()   
#endif
     
#if (__DEBUG_UART_RDATA_MONITOR__ == 1)
#define     UART_MONITOR_BUFF_LENGTH        100
unsigned    char    Uart_monitor_buff[UART_MONITOR_BUFF_LENGTH] = {0};
volatile    unsigned    int     Uart_monitor_cnt = 0;
#endif

/*
 * End of transmit callback function pointer.
 */

static void (*hcit_hw_tx_fptr)(void);

/*
 * State of CTS (are we allowed to send ?)
 */

#if HCIT_CHIMERA_USE_CTSRTS == 1
static volatile u_int8 hcit_hw_can_send;
#endif

/*
 * Implements function pointer from tra_hcit.c, - when called, it registers
 * this HCI Transport system (UART)
 * This function returns a void pointer to the UART structure
 * In tra_hcit.c this is cast to the generic tra_hcit_info structure
 * (which must be the first member or sys_hcit_uart_info)
 */

/*
 * Reference the uart_info structure (comes from tra_uart.c)
 * It is referenced rather than defined here as its space reservation 
 * is in the domain of its parent module (tra_uart.c)
 */

extern struct tra_hcit_uart_info sys_hcit_uart_info;

#if (PRH_BS_DEV_PROCESS_UART_RX_DATA_IN_IRQ_CONTEXT == 1)
u_int8 rx_fifo_cache[40];           /* 460Kbits/sec = approx 37 bytes/slot.  Between 2 & 5 IRQ/slot. 
							        Therfore, 40 byte buffer should be more than sufficient given 
							        that we'll service the buffer at least twice a slot*/
u_int8 *FIQ_insert, *IRQ_process, *rx_fifo_cache_end, *rx_fifo_cache_start;
 
void _Insert_Rx_Char(u_int8 rx_char);
#endif
 

 void* TRAhcit_Register_UART_Driver(void)
{
    /*
     * Set up common members first, then UART specific ones.
     */

    sys_hcit_uart_info.common.type = TRA_HCIT_TYPE_UART;
    sys_hcit_uart_info.common.access_type = TRA_HCIT_FAST_IRQ;
    sys_hcit_uart_info.common.interrupt_handler = HCIT_hw_Interrupt_Handler;

    sys_hcit_uart_info.initialise = HCIT_hw_Initialise;
    sys_hcit_uart_info.shutdown = HCIT_hw_Shutdown;
#if TRA_HCIT_UART_POLLED_TX_SUPPORTED == 1
    sys_hcit_uart_info.poll_tx = HCIT_hw_Tx_Char_Polled;
#else
    sys_hcit_uart_info.transmit = HCIT_hw_Tx_Block;
#endif

#if (PRH_BS_DEV_SERVICE_TRANSPORT_DURING_SPINS_SUPPORTED==1)
    sys_hcit_uart_info.common.poll_rx = HCIT_hw_Empty_FIFO;
    sys_hcit_uart_info.common.suspend = 0;      /* no callback registered */
    sys_hcit_uart_info.common.resume = 0;       /* no callback registered */
#endif

    return (void*)&sys_hcit_uart_info;
}

/*
 * HCIT_hw_Initialise
 *
 * paramters: baud_rate = baud rate
 * returns: void
 *
 * Initialises the specified baud rate, sets up interrupt driven FIFO mode 
 */
void HCIT_hw_Initialise(u_int32 baud_rate)
{
    /* 
     * Clear the fptr in case an MSR interrupt happens before the upper layer
     * has initialised the pointer via a call to Tx_Block. We can validly
     * check against NULL.
     */
	uint32_t	i;
    hcit_hw_tx_fptr = 0x0;
    
#if (PRH_BS_DEV_PROCESS_UART_RX_DATA_IN_IRQ_CONTEXT == 1)
	rx_fifo_cache_start = &rx_fifo_cache[0];
	rx_fifo_cache_end = &rx_fifo_cache[(sizeof(rx_fifo_cache)/sizeof(rx_fifo_cache[0]))-1];
    FIQ_insert = rx_fifo_cache_start;
	IRQ_process = rx_fifo_cache_start;
#endif

	//- CPR_RSTCTL_SUBRST_SW
	*((u_int32 volatile*)0x40000104) = 0x20000;
	for(i=0x100; i>0; i--);
	*((u_int32 volatile*)0x40000104) = 0x20002;	


    if(baud_rate == 921600UL)
    {
	    __write_hw_reg32(CPR_UART1_CLK_CTL, 0x90061a8);	
        __write_hw_reg32(CPR_UART1_CLK_GRCTL, 0x110018);
    }
    else if(baud_rate == 4000000UL)
    {
        __write_hw_reg32(CPR_UART1_CLK_CTL,0x20005);
        __write_hw_reg32(CPR_UART1_CLK_GRCTL, 0x110018);
    }
    else //if(baud_rate == 115200UL) /*default baud_rate == 115200bps.*/
    {   
		__write_hw_reg32(CPR_UART1_CLK_GRCTL, 0x110018);
		__write_hw_reg32(CPR_UART1_CLK_CTL, 0x480271);
	}
	
	__write_hw_reg32(UART1_IER,0x0);               /*disable interrupt*/
	
	__write_hw_reg32(UART1_TCR,0x80);	 
	__write_hw_reg32(UART1_DLL,1);
	__write_hw_reg32(UART1_DLH,0);
	__write_hw_reg32(UART1_TCR,0x03);
	
	__write_hw_reg32(UART1_FCR,0xB7);	
	
#if HCIT_CHIMERA_USE_CTSRTS == 1	
	//*UART1_MCR = 0x22;
	hcit_hw_can_send = 1;
#endif

	__write_hw_reg32(UART1_IER,0x05);              /*enable interrupt*/  
	NVIC_EnableIRQ(UART1_IRQn);	    

}

/*
 * HCIT_hw_Shutdown
 * 
 * paramters: void
 * returns: void
 *
 * This function shuts down UART operation by disabling Interrupts.
 */

void HCIT_hw_Shutdown(void)
{
    __write_hw_reg32(UART1_IER,0x0);               /*disable interrupt*/  
}


/*
 * HCIT_hw_Tx_Block
 *
 * parameters: buf (pointer to pointer to buffer - double pointer needed to alter the original pointer)
 *             length = pointer to length field (updated by amount transmitted)
 *             txcb = function to call on end of transmission interrupt
 * Returns: void
 *
 * Block transmission function. Takes pointer and changes it (adds) by amount transmitted.
 * Decrements length accordingly
 */

void HCIT_hw_Tx_Block(volatile u_int8 **buf, volatile u_int32 *length, void (*txcb)(void))
{
	unsigned int i;

#if HCIT_CHIMERA_USE_CTSRTS == 1
    if(hcit_hw_can_send)
#endif
    {
        u_int32 count = *length;

        hcit_hw_tx_fptr = txcb;

        if(count >= HCIT_HW_TX_FIFO_LENGTH)
            count = HCIT_HW_TX_FIFO_LENGTH;
   
        (*length)-= count;

		do
        {
            *UART1_THR = (u_int8)(*(*buf)++) ;	    
            count--;
        }
        while(count);

         /*
         * Turn on transmitter interrupts
         */
        *UART1_IER |= 0x2;
        
    }
}



/*
 * HCIT_hw_Interrupt_Handler
 *
 * paramters: void
 * returns: void
 *
 * Handle interrupts for UART
 */
void HCIT_hw_Interrupt_Handler(void)
{
    u_int8 status;
    u_int8 ch; 
    
    /*
     * Level triggered interrupt, no need to clear any ICR condition
     */

    status = *UART1_IIR & 0xF;                /* Not interested in upper 7 bits */

    switch(status)
    {
    /*========================
       流控状态改变中断
    ========================*/
#if HCIT_CHIMERA_USE_CTSRTS == 1
    case IIR_INT0:
        ch = *UART1_MSR;

        /* Got a modem status interrupt, check CTS */

        if(ch & UART_MSR_DCTS)
        {
            if(ch & UART_MSR_CTS)                             /* If CTS goes high */
            {
                hcit_hw_can_send = 1; 
                //HWled6_On();
                /*
                 * If polled mode is in use, then we do not need to call the transmit
                 * callback, as polled transmit spins while it cannot send (based on the
                 * can send variable above). All that is necessary is that the can_send
                 * variable is set to one, and polled transmission should automatically
                 * continue after this interrupt.
                 */          
             
#if TRA_HCIT_UART_POLLED_TX_SUPPORTED == 0

                 if(hcit_hw_tx_fptr)
                    (*hcit_hw_tx_fptr)();
#endif
            }
            else 
            {
                //HWled6_Off();
                hcit_hw_can_send = 0;    
            }
        }
        break;
#endif

    /*==========================
       错误中断
    ==========================*/
    case IIR_INT6:                    /* This only fires with break or an error */
        ch = *UART1_TSR;              /* Clear interrupt by reading LSR */
        if(ch & UART_TSR_FE)
            TRAhcit_UART_Indicate_Error(TRA_HCIT_UART_FRAMING_ERROR);
        if(ch & UART_TSR_OE)
		    TRAhcit_UART_Indicate_Error(TRA_HCIT_UART_OVERRUN_ERROR);
		//error_while();
		uart_err_cnt++;
        break;

    /*==========================
      超时中断&接收数据有效中断
    ==========================*/        
	case IIR_INT12:               
    case IIR_INT4:
        while((*UART1_TSR) & UART_TSR_DR) /* Read all characters out of the fifo */
#if (PRH_BS_DEV_PROCESS_UART_RX_DATA_IN_IRQ_CONTEXT ==0)
        {
            ch = *UART1_RBR;
        #if (__DEBUG_UART_RDATA_MONITOR__ == 1)  
            Uart_monitor_buff[Uart_monitor_cnt++] = ch;
            if(Uart_monitor_cnt>=UART_MONITOR_BUFF_LENGTH) Uart_monitor_cnt = 0;
        #endif
            TRAhcit_UART_Rx_Char(ch);  
            
        }
#else
		_Insert_Rx_Char(*UART1_RBR);
#endif
        break;
        
    /*==========================
       发送保持寄存器THR空中断
    ==========================*/  
    case IIR_INT2:

        /* Turn off tx interrupts again */
        
        *UART1_IER &= ~0x2;       /* !! */
        
        (*hcit_hw_tx_fptr)(); 
        break;


    /*==========================
       忙检测中断
    ==========================*/
    case    IIR_INT7:
        status = *UART1_USR;
        error_while();
        uart_err_busy++;
        break;
        
     /*==========================
         没有中断//case IIR_INT1:
     ==========================*/ 
     default:
        error_while();
        uart_err_default++; 
        break;
      
    }
    
}


void HCIT_hw_Empty_FIFO(void)
{
    while((*UART1_TSR) & UART_TSR_DR)        /* Read all characters out of the fifo */
        TRAhcit_UART_Rx_Char(*UART1_RBR);  
}

#endif

