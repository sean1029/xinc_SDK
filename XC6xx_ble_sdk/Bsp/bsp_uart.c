
/*----------------------------------------------------------------------------------------------------
	INCLUDE HEADE FILES
----------------------------------------------------------------------------------------------------*/
#include 		"Platform.h"


#define		CPR_BASE						0x40000000
#define		UART0_BASE						0x40010000
#define		UART1_BASE						0x40011000

#define				CPR_UARTx_CLK_GRCTL(a)	        ((volatile unsigned *)(CPR_BASE + 0x30 + (a * 0x08)))
#define				CPR_UARTx_CLK_CTL(a)		    ((volatile unsigned *)(CPR_BASE + 0x34 + (a * 0x08)))
#define				UARTx_RBR(a)					((volatile unsigned *)(UART0_BASE + 0x00 + (a * 0x1000)))
#define				UARTx_THR(a)					((volatile unsigned *)(UART0_BASE + 0x00 + (a * 0x1000)))
#define				UARTx_DLL(a)					((volatile unsigned *)(UART0_BASE + 0x00 + (a * 0x1000)))
#define				UARTx_IER(a)					((volatile unsigned *)(UART0_BASE + 0x04 + (a * 0x1000)))
#define				UARTx_DLH(a)					((volatile unsigned *)(UART0_BASE + 0x04 + (a * 0x1000)))
#define				UARTx_IIR(a)					((volatile unsigned *)(UART0_BASE + 0x08 + (a * 0x1000)))
#define				UARTx_FCR(a)					((volatile unsigned *)(UART0_BASE + 0x08 + (a * 0x1000)))
#define				UARTx_TCR(a)					((volatile unsigned *)(UART0_BASE + 0x0c + (a * 0x1000)))
#define				UARTx_MCR(a)					((volatile unsigned *)(UART0_BASE + 0x10 + (a * 0x1000)))
#define				UARTx_TSR(a)					((volatile unsigned *)(UART0_BASE + 0x14 + (a * 0x1000)))
#define				UARTx_MSR(a)					((volatile unsigned *)(UART0_BASE + 0x18 + (a * 0x1000)))
#define				UARTx_USR(a)					((volatile unsigned *)(UART0_BASE + 0x7c + (a * 0x1000)))

#define		UART0_RBR						((volatile unsigned *)(UART0_BASE + 0x00))
#define		UART0_THR						((volatile unsigned *)(UART0_BASE + 0x00))
#define		UART0_DLL						((volatile unsigned *)(UART0_BASE + 0x00))
#define		UART0_IER						((volatile unsigned *)(UART0_BASE + 0x04))
#define		UART0_DLH						((volatile unsigned *)(UART0_BASE + 0x04))
#define		UART0_IIR						((volatile unsigned *)(UART0_BASE + 0x08))
#define		UART0_FCR						((volatile unsigned *)(UART0_BASE + 0x08))
#define		UART0_TCR						((volatile unsigned *)(UART0_BASE + 0x0c))
#define		UART0_MCR						((volatile unsigned *)(UART0_BASE + 0x10))
#define		UART0_TSR						((volatile unsigned *)(UART0_BASE + 0x14))
#define		UART0_MSR						((volatile unsigned *)(UART0_BASE + 0x18))
#define		UART0_USR						((volatile unsigned *)(UART0_BASE + 0x7c))
#define		UART1_RBR						((volatile unsigned *)(UART1_BASE + 0x00))
#define		UART1_THR						((volatile unsigned *)(UART1_BASE + 0x00))
#define		UART1_DLL						((volatile unsigned *)(UART1_BASE + 0x00))
#define		UART1_IER						((volatile unsigned *)(UART1_BASE + 0x04))
#define		UART1_DLH						((volatile unsigned *)(UART1_BASE + 0x04))
#define		UART1_IIR						((volatile unsigned *)(UART1_BASE + 0x08))
#define		UART1_FCR						((volatile unsigned *)(UART1_BASE + 0x08))
#define		UART1_TCR						((volatile unsigned *)(UART1_BASE + 0x0c))
#define		UART1_MCR						((volatile unsigned *)(UART1_BASE + 0x10))
#define		UART1_TSR						((volatile unsigned *)(UART1_BASE + 0x14))
#define		UART1_MSR						((volatile unsigned *)(UART1_BASE + 0x18))
#define		UART1_USR						((volatile unsigned *)(UART1_BASE + 0x7c))


#define CPR_RSTCTL_CTLAPB_SW         ((volatile unsigned *)(CPR_BASE + 0x100))
#define CPR_RSTCTL_SUBRST_SW         ((volatile unsigned *)(CPR_BASE + 0x104))
#define CPR_RSTCTL_M0RST_SW          ((volatile unsigned *)(CPR_BASE + 0x108))
#define CPR_RSTCTL_M0RST_MASK        ((volatile unsigned *)(CPR_BASE + 0x10c))
#define CPR_RSTCTL_LOCKUP_STATE      ((volatile unsigned *)(CPR_BASE + 0x110))
#define CPR_RSTCTL_WDTRST_MASK       ((volatile unsigned *)(CPR_BASE + 0x114))
#define CPR_LP_CTL                   ((volatile unsigned *)(CPR_BASE + 0x118))
#define CPR_CTLAPBCLKEN_GRCTL        ((volatile unsigned *)(CPR_BASE + 0x070))

uHandler_callback	uHandler_Callback[2] = {(uHandler_callback)0, (uHandler_callback)0};

/* ---------------------------------------------------------------------------------------------------
- ????????: Init_uart
- ????????: UART??????
- ????????: ????????????(0:uart0 1:uart1); ??????
- ????????: 2016-05-26
----------------------------------------------------------------------------------------------------*/

	
void	Init_uart(uint32_t	ch , uint32_t	baud)
{
			uint32_t	val;
	
	
			
			__write_hw_reg32(CPR_RSTCTL_SUBRST_SW , (1<<(16+ch)));	//????uart????
			__write_hw_reg32(CPR_RSTCTL_SUBRST_SW , ((1<<(16+ch))|(1<<ch)));//??????uart????		
			
			__read_hw_reg32(CPR_LP_CTL , val);
			val &= ~(1<<(1-ch));
			__write_hw_reg32(CPR_LP_CTL , val);						//????uartx????????
			
			val = (1<<(16+ch+4)) | (1<<(ch+4));
			__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL , val); 			//????uartx pclk
			__write_hw_reg32(CPR_UARTx_CLK_GRCTL(ch), 0x110018);
			//__write_hw_reg32(CPR_UARTx_CLK_CTL(ch), 0x480271);
            __write_hw_reg32(CPR_UARTx_CLK_CTL(ch), baud>>4);	
	
	
    
			__write_hw_reg32(UARTx_TCR(ch) , 0x80);                	//????DLAB??
			//__write_hw_reg32(UARTx_DLL(ch) , baud);
            __write_hw_reg32(UARTx_DLL(ch) , baud&0x0F);
			__write_hw_reg32(UARTx_DLH(ch) , 0);                



			__write_hw_reg32(UARTx_TCR(ch) , 0x03);                	//????DLAB??,8??BIT??
			__write_hw_reg32(UARTx_FCR(ch) , 0xb7);                	//????FIFO??????????, ??????FIFO????
			
            __write_hw_reg32(UARTx_IER(ch) , 0x01);                	//????????????
			val = UART0_IRQn + ch;
			NVIC_EnableIRQ((IRQn_Type)val);
            #if 0
            if(ch==1) Uart_Send_String(1,"\nuart1 dma!\n");
            else      Uart_Send_String(0,"\nuart0 dma!\n");
            #endif
						
						Uart_Send_Char(ch,'0');
						Uart_Send_Char(ch,'1');
						Uart_Send_Char(ch,'2');
						Uart_Send_Char(ch,'3');
						Uart_Send_Char(ch,'4');
						Uart_Send_Char(ch,'5');
						Uart_Send_Char(ch,'6');
						Uart_Send_Char(ch,'7');
						Uart_Send_Char(ch,'8');

}
/* ---------------------------------------------------------------------------------------------------
- ????????: Uart_Register_Callback
- ????????: ??uart????????????????
- ????????: ??????;????????
- ????????: 2016-05-26
----------------------------------------------------------------------------------------------------*/
void	Uart_Register_Callback(uint32_t	ch, uHandler_callback callback)
{
			uHandler_Callback[ch] = callback;
}
/* ---------------------------------------------------------------------------------------------------
- ????????: UART0_Handler
- ????????: UART0????????????
- ????????: ??
- ????????: 2016-05-26
----------------------------------------------------------------------------------------------------*/


/* ---------------------------------------------------------------------------------------------------
- ????????: UART1_Handler
- ????????: UART1????????????
- ????????: ??
- ????????: 2016-05-26
----------------------------------------------------------------------------------------------------*/

uint8_t rx_buff[20];

/* ---------------------------------------------------------------------------------------------------
- ????????: Uart_Send_Char
- ????????: Uart????????????
- ????????: ??????; ????????????
- ????????: 2016-05-26
----------------------------------------------------------------------------------------------------*/
void	Uart_Send_Char(uint32_t	ch, uint8_t	c)
{
		unsigned	int	status;
		for(; ;)
		{			
				__read_hw_reg32(UARTx_TSR(ch), status);
				status &= 0x20;
				if(status == 0x20) break;
		}
		__write_hw_reg32(UARTx_THR(ch), c);
}

/* ---------------------------------------------------------------------------------------------------
- ????????: Uart_Send_String
- ????????: Uart??????????????
- ????????: ??????; ??????????????
- ????????: 2016-05-26
----------------------------------------------------------------------------------------------------*/
void	Uart_Send_String(uint32_t	ch, uint8_t	*s)
{
		while(*s != '\0')				
		{	
				Uart_Send_Char(ch, *s++);
		}
}

/* ---------------------------------------------------------------------------------------------------
- ????????: Uart_Send_Buf
- ????????: Uart??????????????length??uint8_t??????????
- ????????: ??????; ??????????????????????
- ????????: 2019-04-02
- ??    ??????????
----------------------------------------------------------------------------------------------------*/
void	Uart_Send_Buf(uint32_t	ch, uint8_t	*s,uint32_t length)
{
		while(length--)				
		{	
				Uart_Send_Char(ch, *s++);
		}
}

/* ---------------------------------------------------------------------------------------------------
- ????????: Uart_DMA_Send
- ????????: Uart??DMA??????????????????length??uint8_t??????????  --????DMA??????????SHRAM0??0x10010000 ~ 0x1001FFFF??
- ????????: ?????? ??????????????????????
- ????????: 2019-04-02
- ??    ??????????
----------------------------------------------------------------------------------------------------*/
static uint8_t __attribute__((aligned(4))) uart_tx_buf[256]={0};
void	Uart_DMA_Send(uint8_t   ch,uint8_t	*s,uint32_t length)
{
	    for(int i=0;i<length;i++)
            uart_tx_buf[i]=s[i];    
		uint32_t    iWK = 0;
		//- TX Channel
		__write_hw_reg32(DMAS_CHx_SAR(ch) , (uint32_t)uart_tx_buf);
		__write_hw_reg32(DMAS_CHx_DAR(ch) , (ch*0x1000+0x40010000));//??????????UART_THR
		__write_hw_reg32(DMAS_CHx_CTL1(ch) ,((2 << 8)));//??????????2
		__write_hw_reg32(DMAS_CHx_CTL0(ch) ,length);
		__write_hw_reg32(DMAS_EN , ch);

		do	{
			__read_hw_reg32(DMAS_INT_RAW , iWK);
		}while((iWK&(1+ch)) != (1+ch));

		__write_hw_reg32(DMAS_INT_RAW, (1+ch));
		__write_hw_reg32(DMAS_CLR ,ch);


}

