#include     <string.h>
#include    "platform.h"


QUEUE_Init_TypeDef COM_QUEUE;
/* ---------------------------------------------------------------------------------------------------
- 函数名称: queue_init
- 函数功能: 串口环形队列初始化
- 输入参数: 
- 创建日期: 2019-05-23
----------------------------------------------------------------------------------------------------*/
void queue_init(void)
{
    memset(&COM_QUEUE, 0, sizeof(COM_QUEUE));
}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: queue_empty
- 函数功能: 判断串口环形队列是否为空
- 输入参数: 
- 创建日期: 2019-05-23
----------------------------------------------------------------------------------------------------*/
uint8_t queue_empty(void)
{
    if(COM_QUEUE.head == COM_QUEUE.tail)
    {
        return (1);
    }
    else
    {
        return (0);
    }
}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: queue_read
- 函数功能: 读串口环形队列
- 输入参数: 
- 创建日期: 2019-05-23
----------------------------------------------------------------------------------------------------*/
uint8_t queue_read(void)
{
    uint8_t ucData = COM_QUEUE.data[COM_QUEUE.head++];

    COM_QUEUE.head %= QUEUE_SIZE;

    return ucData;
}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: queue_write
- 函数功能: 写� 串口环形队列
- 输入参数: 
- 创建日期: 2019-05-23
----------------------------------------------------------------------------------------------------*/
void queue_write(uint8_t value)
{
    COM_QUEUE.data[COM_QUEUE.tail++] = value;

    COM_QUEUE.tail %= QUEUE_SIZE;
}




/* ---------------------------------------------------------------------------------------------------
- 函数名称: config_uart1_pin
- 函数功能: UART1引脚配置
- 输入参数: 
- 创建日期: 2019-05-23
----------------------------------------------------------------------------------------------------*/
void config_uart1_pin()
{
    uint32_t	val;
	//配置GPIO18-UART1_TX    GPIO19-UART1_RX
	gpio_mux_ctl(18,0);gpio_fun_inter(18,0);gpio_fun_sel(18,UART1_TX);//UART1_TX 
    gpio_mux_ctl(19,0);gpio_fun_inter(19,0);gpio_fun_sel(19,UART1_RX);//UART1_RX 
	
}

/* ---------------------------------------------------------------------------------------------------
- 函数名称: xc_uart1_init
- 函数功能: uart1初始配置
- 输入参数: 
- 创建日期: 2019-05-23
----------------------------------------------------------------------------------------------------*/
extern  void xc_uart1_init(void)
{
    extern void at_init(void);
    config_uart1_pin();
	Init_uart(1, BAUD_115200);
	queue_init();
	at_init();
	Uart_Register_Callback(1, (uHandler_callback)queue_write);
}


