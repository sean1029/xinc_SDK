/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_GPIO_H__
#define XINC_GPIO_H__

#include <xincx.h>
#include "xinc_bitmask.h"
//#include "XC620610.h"
#include "bsp_gpio.h"
#ifdef __cplusplus
extern "C" {
#endif


#ifndef XINC_GPIO0
#define XINC_GPIO0 XINC_GPIO0
#endif

#if (GPIO_COUNT == 1)
#define NUMBER_OF_PINS (P0_PIN_NUM)
#define GPIO_REG_LIST  {XINC_GPIO0}
#elif (GPIO_COUNT == 2)
#define NUMBER_OF_PINS (P0_PIN_NUM + P1_PIN_NUM)
#define GPIO_REG_LIST  {XINC_GPIO0}
#else
#error "Not supported."
#endif



/**
 * @defgroup xinc_gpio_hal GPIO HAL
 * @{
 * @ingroup xinc_gpio
 * @brief   Hardware access layer for managing the GPIO peripheral.
 */


/** @brief Macro for mapping port and pin numbers to values understandable for xinc_gpio functions. */
#define XINC_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))
#if (GPIO_COUNT == 1) || (GPIO_COUNT == 2)
//#define XINC_GPIO_0				    XINC_GPIO_PIN_MAP(0, 0)
//#define XINC_GPIO_1				    XINC_GPIO_PIN_MAP(0, 1)
//#define XINC_GPIO_2				    XINC_GPIO_PIN_MAP(0, 2)
//#define XINC_GPIO_3				    XINC_GPIO_PIN_MAP(0, 3)
//#define XINC_GPIO_4				    XINC_GPIO_PIN_MAP(0, 4)
//#define XINC_GPIO_5				    XINC_GPIO_PIN_MAP(0, 5)
//#define XINC_GPIO_6				    XINC_GPIO_PIN_MAP(0, 6)
//#define XINC_GPIO_7				    XINC_GPIO_PIN_MAP(0, 7)
//#define XINC_GPIO_8				    XINC_GPIO_PIN_MAP(0, 8)
//#define XINC_GPIO_9				    XINC_GPIO_PIN_MAP(0, 9)
//#define XINC_GPIO_10				XINC_GPIO_PIN_MAP(0, 10)
//#define XINC_GPIO_11				XINC_GPIO_PIN_MAP(0, 11)
//#define XINC_GPIO_12				XINC_GPIO_PIN_MAP(0, 12)
//#define XINC_GPIO_13				XINC_GPIO_PIN_MAP(0, 13)
//#define XINC_GPIO_14				XINC_GPIO_PIN_MAP(0, 14)
//#define XINC_GPIO_15				XINC_GPIO_PIN_MAP(0, 15)
//#define XINC_GPIO_16				XINC_GPIO_PIN_MAP(0, 16)
//#define XINC_GPIO_17				XINC_GPIO_PIN_MAP(0, 17)
//#define XINC_GPIO_18				XINC_GPIO_PIN_MAP(0, 18)
//#define XINC_GPIO_19				XINC_GPIO_PIN_MAP(0, 19)
//#define XINC_GPIO_20				XINC_GPIO_PIN_MAP(0, 20)
//#define XINC_GPIO_21				XINC_GPIO_PIN_MAP(0, 21)
//#define XINC_GPIO_22				XINC_GPIO_PIN_MAP(0, 22)
//#define XINC_GPIO_23				XINC_GPIO_PIN_MAP(0, 23)
//#define XINC_GPIO_24				XINC_GPIO_PIN_MAP(0, 24)
//#define XINC_GPIO_25				XINC_GPIO_PIN_MAP(0, 25)
//#define XINC_GPIO_26				XINC_GPIO_PIN_MAP(0, 26)
//#define XINC_GPIO_27				XINC_GPIO_PIN_MAP(0, 27)
//#define XINC_GPIO_28				XINC_GPIO_PIN_MAP(0, 28)
//#define XINC_GPIO_29				XINC_GPIO_PIN_MAP(0, 29)
//#define XINC_GPIO_30				XINC_GPIO_PIN_MAP(0, 30)
//#define XINC_GPIO_31				XINC_GPIO_PIN_MAP(0, 31)
#endif

#if (GPIO_COUNT == 2)
//#define XINC_GPIO_32				XINC_GPIO_PIN_MAP(0, 32)
//#define XINC_GPIO_33				XINC_GPIO_PIN_MAP(0, 33)
//#define XINC_GPIO_34				XINC_GPIO_PIN_MAP(0, 34)
//#define XINC_GPIO_35				XINC_GPIO_PIN_MAP(0, 35)
#endif

enum{
#if (GPIO_COUNT == 1) || (GPIO_COUNT == 2)
    XINC_GPIO_0			=	    XINC_GPIO_PIN_MAP(0, 0),
    XINC_GPIO_1			=	    XINC_GPIO_PIN_MAP(0, 1),
    XINC_GPIO_2			=	    XINC_GPIO_PIN_MAP(0, 2),
    XINC_GPIO_3			=	    XINC_GPIO_PIN_MAP(0, 3),
    XINC_GPIO_4			=	    XINC_GPIO_PIN_MAP(0, 4),
    XINC_GPIO_5			=	    XINC_GPIO_PIN_MAP(0, 5),
    XINC_GPIO_6			=	    XINC_GPIO_PIN_MAP(0, 6),
    XINC_GPIO_7			=	    XINC_GPIO_PIN_MAP(0, 7),
    XINC_GPIO_8			=	    XINC_GPIO_PIN_MAP(0, 8),
    XINC_GPIO_9			=	    XINC_GPIO_PIN_MAP(0, 9),
    XINC_GPIO_10		=		XINC_GPIO_PIN_MAP(0, 10),
    XINC_GPIO_11		=		XINC_GPIO_PIN_MAP(0, 11),
    XINC_GPIO_12		=		XINC_GPIO_PIN_MAP(0, 12),
    XINC_GPIO_13		=		XINC_GPIO_PIN_MAP(0, 13),
    XINC_GPIO_14		=		XINC_GPIO_PIN_MAP(0, 14),
    XINC_GPIO_15		=		XINC_GPIO_PIN_MAP(0, 15),
    XINC_GPIO_16		=		XINC_GPIO_PIN_MAP(0, 16),
    XINC_GPIO_17		=		XINC_GPIO_PIN_MAP(0, 17),
    XINC_GPIO_18		=		XINC_GPIO_PIN_MAP(0, 18),
    XINC_GPIO_19		=		XINC_GPIO_PIN_MAP(0, 19),
    XINC_GPIO_20		=		XINC_GPIO_PIN_MAP(0, 20),
    XINC_GPIO_21		=		XINC_GPIO_PIN_MAP(0, 21),
    XINC_GPIO_22		=		XINC_GPIO_PIN_MAP(0, 22),
    XINC_GPIO_23		=		XINC_GPIO_PIN_MAP(0, 23),
    XINC_GPIO_24		=		XINC_GPIO_PIN_MAP(0, 24),
    XINC_GPIO_25		=		XINC_GPIO_PIN_MAP(0, 25),
    XINC_GPIO_26		=		XINC_GPIO_PIN_MAP(0, 26),
    XINC_GPIO_27		=		XINC_GPIO_PIN_MAP(0, 27),
    XINC_GPIO_28		=		XINC_GPIO_PIN_MAP(0, 28),
    XINC_GPIO_29		=		XINC_GPIO_PIN_MAP(0, 29),
    XINC_GPIO_30		=		XINC_GPIO_PIN_MAP(0, 29),
    XINC_GPIO_31		=		XINC_GPIO_PIN_MAP(0, 31),
#endif

#if (GPIO_COUNT == 2)
    XINC_GPIO_32		=		XINC_GPIO_PIN_MAP(0, 32),
    XINC_GPIO_33		=		XINC_GPIO_PIN_MAP(0, 33),
    XINC_GPIO_34		=		XINC_GPIO_PIN_MAP(0, 34),
    XINC_GPIO_35		=		XINC_GPIO_PIN_MAP(0, 35),
#endif //

    XINC_GPIO_MAX 
};
typedef enum{
 XC_GPIO_Dx=0,//普通GPIO口功能
 XC_UART0_TX,
 XC_UART0_RX,
 XC_UART0_CTS,
 XC_UART0_RTS,
 XC_I2C_SCL,
 XC_I2C_SDA,
 XC_UART1_RX,
 XC_UART1_TX,
 XC_SIM_IO,
 XC_SIM_RST,
 XC_SIM_CLK_OUT,
 XC_PWM0,
 XC_PWM1,
 XC_SSI1_CLK,
 XC_SSI1_SSN,
 XC_SSI1_RX,
 XC_SSI1_TX,
 XC_PWM0_INV,  
 XC_PWM1_INV,  

}XC_GPIO_FUN_SEL_TypeDef;
typedef enum{
 XC_NOT_INT=0,
 XC_NA1_INT,
 XC_NA2_INT,
 XC_NA3_INT,
 XC_NA4_INT,
 XC_RIS_EDGE_INT,//上升沿中断
 XC_NA6_INT,
 XC_FAIL_EDGE_INT,//下降沿中断
 XC_NA8_INT,   
 XC_RIS_FAIL_EDGE_INT,//上升沿下降沿中断
    
}XC_GPIO_INT_TypeDef;

typedef enum{
	XC_GPIO_Mode_Input_Up=0, /*上拉输入*/
	XC_GPIO_Mode_Input_Down, /*下拉输入*/
	XC_GPIO_Mode_Input_Float,/*浮空输入*/
}XC_GPIO_InputMode_TypeDef;


 /** @brief Polarity for the GPIO channel. */
typedef enum
{
  XINC_GPIO_POLARITY_LOTOHI = 0,//,       ///<  Low to high.
  XINC_GPIO_POLARITY_HITOLO = 1,//,       ///<  High to low.
  XINC_GPIO_POLARITY_TOGGLE = 2,//        ///<  Toggle.
} xinc_gpio_polarity_t;

/** @brief Initial output value for the GPIO channel. */
typedef enum
{
  XINC_GPIO_INITIAL_VALUE_LOW  = GPIO_GPIO_PORT_DR_Low,//,       ///<  Low 
  XINC_GPIO_INITIAL_VALUE_HIGH = GPIO_GPIO_PORT_DR_High//       ///<  High.
} xinc_gpio_outinit_t;


/** @brief Pin direction definitions. */
typedef enum
{
    XINC_GPIO_PIN_DIR_INPUT  = GPIO_GPIO_PORT_DDR_Input, ///< Input.
    XINC_GPIO_PIN_DIR_OUTPUT = GPIO_GPIO_PORT_DDR_Output ///< Output.
} xinc_gpio_pin_dir_t;

/** @brief Connection of input buffer. */
typedef enum
{
    XINC_GPIO_PIN_INPUT_NOINT            = XC_NOT_INT,   ///< Connect input No int.
    XINC_GPIO_PIN_INPUT_RIS_EDGE_INT     = XC_RIS_EDGE_INT,   //
    XINC_GPIO_PIN_INPUT_FAIL_EDGE_INT    = XC_FAIL_EDGE_INT,   //
    XINC_GPIO_PIN_INPUTEDGE_IN           = XC_RIS_FAIL_EDGE_INT,   //
} xinc_gpio_pin_input_int_t;

/**
 * @brief Enumerator used for selecting the pin to be pulled down or up at the time of pin
 * configuration.
 */
typedef enum
{
    XINC_GPIO_PIN_NOPULL   = XC_GPIO_Mode_Input_Float, ///<  Pin pull-up resistor disabled.
    XINC_GPIO_PIN_PULLDOWN = XC_GPIO_Mode_Input_Down, ///<  Pin pull-down resistor enabled.
    XINC_GPIO_PIN_PULLUP   = XC_GPIO_Mode_Input_Up,   ///<  Pin pull-up resistor enabled.
} xinc_gpio_pin_pull_t;


/**
 * @brief Enumerator used for selecting the pin to be pulled down or up at the time of pin
 * configuration.
 */
typedef struct
{
    xinc_gpio_pin_pull_t pin_pulll;
    xinc_gpio_pin_input_int_t input_int;

} xinc_gpio_pin_input_cfg_t;

/** @brief Enumerator used for selecting output drive mode. */
typedef enum
{
    XINC_GPIO_PIN_GPIODx = XC_GPIO_Dx, ///< 
    XINC_GPIO_PIN_UART0_TX = XC_UART0_TX, ///
    XINC_GPIO_PIN_UART0_RX = XC_UART0_RX, ///< 
    XINC_GPIO_PIN_UART0_CTS = XC_UART0_CTS, //
    XINC_GPIO_PIN_UART0_RTS = XC_UART0_RTS, //
    XINC_GPIO_PIN_I2C_SCL = XC_I2C_SCL, //
    XINC_GPIO_PIN_I2C_SDA = XC_I2C_SDA, //
    XINC_GPIO_PIN_UART1_RX = XC_UART1_RX, //
    XINC_GPIO_PIN_UART1_TX = XC_UART1_TX, //
    XINC_GPIO_PIN_SIM_IO = XC_SIM_IO, //
    XINC_GPIO_PIN_SIM_RST = XC_SIM_RST, //
    XINC_GPIO_PIN_SIM_CLK_OUT = XC_SIM_CLK_OUT, //
    XINC_GPIO_PIN_PWM0 = XC_PWM0, //
    XINC_GPIO_PIN_PWM1 = XC_PWM1, //
    XINC_GPIO_PIN_SSI1_CLK = XC_SSI1_CLK, //
    XINC_GPIO_PIN_SSI1_SSN = XC_SSI1_SSN, //
    XINC_GPIO_PIN_SSI1_RX = XC_SSI1_RX, //
    XINC_GPIO_PIN_SSI1_TX = XC_SSI1_TX, //
    XINC_GPIO_PIN_PWM0_INV = XC_PWM0_INV, //
    XINC_GPIO_PIN_PWM1_INV = XC_PWM1_INV, // 
    XINC_GPIO_PIN_FUN_SEL_MAX,
    //////////////////////////////////////////////////////

    XINC_GPIO_PIN_PWM2,
    XINC_GPIO_PIN_PWM3,
    XINC_GPIO_PIN_PWM4,
    XINC_GPIO_PIN_PWM5,
    XINC_GPIO_PIN_SSI2_CLK,
    XINC_GPIO_PIN_SSI2_SSN,
    XINC_GPIO_PIN_SSI2_D0,
    XINC_GPIO_PIN_SSI2_D1,
    XINC_GPIO_PIN_SSI2_D2,
    XINC_GPIO_PIN_SSI2_D3,
    XINC_GPIO_PIN_UART2_TX , ///
    XINC_GPIO_PIN_UART2_RX , ///< 
    XINC_GPIO_PIN_AUDIO_MIC_P ,
    XINC_GPIO_PIN_AUDIO_MIC_N ,
    XINC_GPIO_PIN_AUDIO_MIC_BIAS ,
		
    XINC_GPIO_PIN_I2S_SCLK,
    XINC_GPIO_PIN_I2S_WS,
    XINC_GPIO_PIN_I2S_DIN,
    XINC_GPIO_PIN_I2S_DOUT,
} xinc_gpio_pin_fun_sel_t;

/** @brief Enumerator used for mux. */
typedef enum
{
    XINC_GPIO_PIN_MUX0   = 0, ///<  
    XINC_GPIO_PIN_MUX1   = 1,      ///< 
    XINC_GPIO_PIN_MUX2   = 2,     ///<  Pin
    XINC_GPIO_PIN_MUX3   = 3
} xinc_gpio_pin_mux_t;

/** @brief Enumerator used for selecting the pin to debounce on the pin input. */
typedef enum
{
    XINC_GPIO_PIN_DEBOUNCE_DISABLE   = 0,
    XINC_GPIO_PIN_DEBOUNCE_ENABLE    = 1, ///<  Pin DEBOUNCE  enabled.
} xinc_gpio_pin_debounce_t;

/**
 * @brief Function for configuring the GPIO pin range as output pins with normal drive strength.
 *        This function can be used to configure pin range as simple output with gate driving GPIO_PIN_CNF_DRIVE_S0S1 (normal cases).
 *
 * @note For configuring only one pin as output, use @ref xinc_gpio_cfg_output.
 *       Sense capability on the pin is disabled and input is disconnected from the buffer as the pins are configured as output.
 *
 * @param pin_range_start  Specifies the start number (inclusive) in the range of pin numbers to be configured (allowed values 0-30).
 * @param pin_range_end    Specifies the end number (inclusive) in the range of pin numbers to be configured (allowed values 0-30).
 */
__STATIC_INLINE void xinc_gpio_range_cfg_output(uint32_t pin_range_start, uint32_t pin_range_end);

/**
 * @brief Function for configuring the GPIO pin range as input pins with given initial value set, hiding inner details.
 *        This function can be used to configure pin range as simple input.
 *
 * @note  For configuring only one pin as input, use @ref xinc_gpio_cfg_input.
 *        Sense capability on the pin is disabled and input is connected to buffer so that the GPIO->IN register is readable.
 *
 * @param pin_range_start  Specifies the start number (inclusive) in the range of pin numbers to be configured (allowed values 0-30).
 * @param pin_range_end    Specifies the end number (inclusive) in the range of pin numbers to be configured (allowed values 0-30).
 * @param input_config      State of the pin range pull resistor (no pull, pulled down, or pulled high).
 */
__STATIC_INLINE void xinc_gpio_range_cfg_input(uint32_t            pin_range_start,
                                              uint32_t            pin_range_end,
                                              xinc_gpio_pin_input_cfg_t input_config);

/**
 * @brief Pin configuration function.
 *
 * The main pin configuration function.
 * This function allows to set any aspect in PIN_CNF register.
 *
 * @param pin_number Specifies the pin number.
 * @param dir        Pin direction.
 * @param input      Connect or disconnect the input buffer.
 * @param pull       Pull configuration.
 * @param drive      Drive configuration.
 * @param sense      Pin sensing mechanism.
 */
__STATIC_INLINE void xinc_gpio_cfg(
    uint32_t             pin_number,
    xinc_gpio_pin_dir_t   dir,
    xinc_gpio_pin_input_int_t inter,
    xinc_gpio_pin_pull_t  pull,
    xinc_gpio_pin_fun_sel_t fun,
    xinc_gpio_pin_debounce_t debounce);


/**
 * @brief Function for configuring the given GPIO pin number as output, hiding inner details.
 *        This function can be used to configure a pin as simple output with gate driving GPIO_PIN_CNF_DRIVE_S0S1 (normal cases).
 *
 * @note  Sense capability on the pin is disabled and input is disconnected from the buffer as the pins are configured as output.
 *
 * @param pin_number Specifies the pin number.
 */
__STATIC_INLINE void xinc_gpio_cfg_output(uint32_t pin_number);//o 

/**
 * @brief Function for configuring the given GPIO pin number as input, hiding inner details.
 *        This function can be used to configure a pin as simple input.
 *
 * @note  Sense capability on the pin is disabled and input is connected to buffer so that the GPIO->IN register is readable.
 *
 * @param pin_number  Specifies the pin number.
 * @param pull_config State of the pin range pull resistor (no pull, pulled down, or pulled high).
 */
__STATIC_INLINE void xinc_gpio_cfg_input(uint32_t pin_number, xinc_gpio_pin_input_cfg_t input_config);//o

/**
 * @brief Function for resetting pin configuration to its default state.
 *
 * @param pin_number Specifies the pin number.
 */
__STATIC_INLINE void xinc_gpio_cfg_default(uint32_t pin_number);//o


/**
 * @brief Function for setting the direction for a GPIO pin.
 *
 * @param pin_number Specifies the pin number for which to set the direction.
 * @param direction  Specifies the direction.
 */
__STATIC_INLINE void xinc_gpio_pin_dir_set(uint32_t pin_number, xinc_gpio_pin_dir_t direction);//o

/**
 * @brief Function for setting a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to be set.
 */
__STATIC_INLINE void xinc_gpio_pin_set(uint32_t pin_number);//o

/**
 * @brief Function for clearing a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to clear.
 */
__STATIC_INLINE void xinc_gpio_pin_clear(uint32_t pin_number);//o

/**
 * @brief Function for toggling a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to toggle.
 */
__STATIC_INLINE void xinc_gpio_pin_toggle(uint32_t pin_number);//o

/**
 * @brief Function for writing a value to a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to write.
 * @param value      Specifies the value to be written to the pin.
 * @arg 0 Clears the pin.
 * @arg >=1 Sets the pin.
 */
__STATIC_INLINE void xinc_gpio_pin_write(uint32_t pin_number, uint32_t value);//o

/**
 * @brief Function for reading the input level of a GPIO pin.
 *
 * If the value returned by this function is to be valid, the pin's input buffer must be connected.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return 0 if the pin input level is low. Positive value if the pin is high.
 */
__STATIC_INLINE uint32_t xinc_gpio_pin_read(uint32_t pin_number);//o

/**
 * @brief Function for reading the output level of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return 0 if the pin output level is low. Positive value if pin output is high.
 */
__STATIC_INLINE uint32_t xinc_gpio_pin_out_read(uint32_t pin_number);//o


/**
 * @brief Function for reading the direction configuration of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return Direction configuration.
 */
__STATIC_INLINE xinc_gpio_pin_dir_t xinc_gpio_pin_dir_get(uint32_t pin_number);//o



/**
 * @brief Function for reading the pull configuration of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @retval Pull configuration.
 */
__STATIC_INLINE xinc_gpio_pin_pull_t xinc_gpio_pin_pull_get(uint32_t pin_number);//o


__STATIC_INLINE void xinc_gpio_mux_ctl(uint32_t  pin_number,uint8_t mux);//o



__STATIC_INLINE void xinc_gpio_fun_sel(uint32_t pin_number,xinc_gpio_pin_fun_sel_t fun);//o


__STATIC_INLINE void xinc_gpio_inter_sel(uint32_t pin_number,xinc_gpio_pin_input_int_t inter);//o


__STATIC_INLINE void xinc_gpio_pull_sel(uint32_t pin_number,xinc_gpio_pin_pull_t pull);//o

/**
 * @brief Function for checking if provided pin is present on the MCU.
 *
 * @param[in] pin_number Number of the pin to be checked.
 *
 * @retval true  Pin is present.
 * @retval false Pin is not present.
 */
__STATIC_INLINE bool xinc_gpio_pin_present_check(uint32_t pin_number);//o


#ifndef SUPPRESS_INLINE_IMPLEMENTATION

/**
 * @brief Function for extracting port and the relative pin number from the absolute pin number.
 *
 * @param[in,out] p_pin Pointer to the absolute pin number overriden by the pin number that is relative to the port.
 *
 * @return Pointer to port register set.
 */
__STATIC_INLINE XINC_GPIO_Type * xinc_gpio_pin_port_decode(uint32_t * p_pin)
{
    XINCX_ASSERT(xinc_gpio_pin_present_check(*p_pin));

    return XINC_GPIO0;

}

__STATIC_INLINE XINC_CPR_CTL_Type * xinc_gpio_pin_cpr_decode(uint32_t * p_pin)
{
    XINCX_ASSERT(xinc_gpio_pin_present_check(*p_pin));

      return XINC_CPR;
}

__STATIC_INLINE XINC_CPR_AO_CTL_Type * xinc_gpio_pin_pull_decode(uint32_t * p_pin)
{
      XINCX_ASSERT(xinc_gpio_pin_present_check(*p_pin));

      return XINC_CPR_AO;
}

__STATIC_INLINE void xinc_gpio_range_cfg_output(uint32_t pin_range_start, uint32_t pin_range_end)
{
    /*lint -e{845} // A zero has been given as right argument to operator '|'" */
    for (; pin_range_start <= pin_range_end; pin_range_start++)
    {
        xinc_gpio_cfg_output(pin_range_start);
    }
}


__STATIC_INLINE void xinc_gpio_range_cfg_input(uint32_t            pin_range_start,
                                              uint32_t            pin_range_end,
                                              xinc_gpio_pin_input_cfg_t input_config)
{
    /*lint -e{845} // A zero has been given as right argument to operator '|'" */
    for (; pin_range_start <= pin_range_end; pin_range_start++)
    {
        xinc_gpio_cfg_input(pin_range_start, input_config);
    }
}
/* ---------------------------------------------------------------------------------------------------
- ��������: gpio_fun_inter
- ��������: gpio�ж�ģʽ���� 
- �������: gpio���0-28  �ж�����0���ж� 5�������ж� 7�½����ж�
-��inter=0  GPIO�ر��ж�:NOT_INT        ��                       
-��inter=5  GPIO�������ж�:RIS_EDGE_INT ��                        
-��inter=7  GPIO�½����ж�:FAIL_EDGE_INT��
-��inter=9  GPIO�������½����ж�:RIS_FAIL_EDGE_INT��
- ��������: 2022-03-18
----------------------------------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------------------------------
- ��������: xinc_gpio_mux_ctl
- ��������: pin�����ӿ���
- �������: mux
- ��������: 2022-3-18

- 32/16 PIN��
- GPIO0 : ��mux=0 ���ӵ�gpio_d[0] �� ��mux=1 NA��               ��mux=2 pwm2��       ��mux=3 NA��
- GPIO1 : ��mux=0 ���ӵ�gpio_d[1] �� ��mux=1 NA��               ��mux=2 pwm3��       ��mux=3 NA��
- GPIO2 : ��mux=0 ���ӵ�gpio_d[2] �� ��mux=1 NA��               ��mux=2 clk_12M_out����mux=3 NA��
- GPIO3 : ��mux=0 ���ӵ�gpio_d[3] �� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO4 : ��mux=0 ���ӵ�gpio_d[4] �� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO5 : ��mux=0 ���ӵ�gpio_d[5] �� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO6 : ��mux=0 ���ӵ�gpio_d[6] �� ��mux=1 NA��               ��mux=2 txen��       ��mux=3 NA��
- GPIO7 : ��mux=0 ���ӵ�gpio_d[7] �� ��mux=1 NA��               ��mux=2 rxen��       ��mux=3 NA��
- GPIO8 : ��mux=0 ���ӵ�gpio_d[8] �� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO9 : ��mux=0 ���ӵ�gpio_d[9] �� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO10: ��mux=0 ���ӵ�gpio_d[10]�� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO11: ��mux=0 ���ӵ�BOOT_CTL  �� ��mux=1 ���ӵ�gpio_d[11]�� ��mux=2 NA��         ��mux=3 NA��
- GPIO12: ��mux=0 ���ӵ�SWI�� SWCK�� ��mux=1 ���ӵ�gpio_d[12]�� ��mux=2 NA��         ��mux=3 pwm4��
- GPIO13: ��mux=0 ���ӵ�SWI�� SWD �� ��mux=1 ���ӵ�gpio_d[13]�� ��mux=2 NA��         ��mux=3 pwm5��
- GPIO14: ��mux=0 ���ӵ�gpio_d[14]�� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO18: ��mux=0 ���ӵ�gpio_d[18]�� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO19: ��mux=0 ���ӵ�gpio_d[19]�� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO20: ��mux=0 ���ӵ�gpio_d[20]�� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO21: ��mux=0 ���ӵ�gpio_d[21]�� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO22: ��mux=0 ���ӵ�gpio_d[22]�� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO23: ��mux=0 ���ӵ�gpio_d[23]�� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO24: ��mux=0 ���ӵ�gpio_d[24]�� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- GPIO25: ��mux=0 ���ӵ�gpio_d[25]�� ��mux=1 NA��               ��mux=2 NA��         ��mux=3 NA��
- ----------------------------------------------------------------------------------------------------*/


/* ---------------------------------------------------------------------------------------------------
- ��������: gpio_fun_sel
- ��������: gpio���ù�������
- �������: gpio���0-28 sel����
-��sel=0   ��ͨGPIO����:GPIO_Dx               ��
-��sel=1   ����0����:UART0_TX                 ��
-��sel=2   ����0����::UART0_RX                �� 
-��sel=3   ����0����:UART0_CTS                ��
-��sel=4   ����0����:UART0_RTS                ��
-��sel=5   I2Cʱ��:I2C_SCL                    �� 
-��sel=6   I2C����:I2C_SDA                    ��
-��sel=7   ����1����:UART1_RX                 ��
-��sel=8   ����1����:UART1_TX                 �� 
-��sel=9   SIM_IO                             ��   
-��sel=10  SIM_RST                            �� 
-��sel=11  SIM_CLK_OUT                        ��                  
-��sel=12  PWM0���:PWM0                      ��  
-��sel=13  PWM1���:PWM1                      ��   
-��sel=14  SPI1ʱ��:SSI1_CLK                  ��
-��sel=15  SPI1Ƭѡ:SSI1_SSN                  ��
-��sel=16  SPI1����:SSI1_RX                   �� 
-��sel=17  SPI1����:SSI1_TX                   ��
-��sel=18  PWM0�������:PWM0_INV(32/16PIN����)��
-��sel=19  PWM1�������:PWM1_INV(32/16PIN����)��
- ��������: 2019-12-03
----------------------------------------------------------------------------------------------------*/

//	#include "bsp_gpio.h"


__STATIC_INLINE void xinc_gpio_cfg(
    uint32_t             pin_number,
    xinc_gpio_pin_dir_t   dir,
    xinc_gpio_pin_input_int_t inter,
    xinc_gpio_pin_pull_t  pull,
    xinc_gpio_pin_fun_sel_t fun,
    xinc_gpio_pin_debounce_t debounce)
{

    XINC_GPIO_Type * preg = xinc_gpio_pin_port_decode(&pin_number);

//  gpio_fun_sel(pin_number,fun);
//    gpio_mux_ctl(pin_number,0);

        xinc_gpio_fun_sel(pin_number,fun);
    xinc_gpio_mux_ctl(pin_number,0);
    
    if(XINC_GPIO_PIN_DIR_INPUT == dir)
    {
        xinc_gpio_pin_dir_set(pin_number,XINC_GPIO_PIN_DIR_INPUT);
               xinc_gpio_pull_sel(pin_number,pull);
        xinc_gpio_inter_sel(pin_number,inter);
 
//        gpio_fun_inter(pin_number,inter);
//        gpio_direction_input(pin_number,pull);
        
    }else
    {
        xinc_gpio_pin_dir_set(pin_number,XINC_GPIO_PIN_DIR_OUTPUT);
       
        
      //  gpio_direction_output(pin_number);
       
    }

}


__STATIC_INLINE void xinc_gpio_cfg_output(uint32_t pin_number)//o
{
    xinc_gpio_cfg(
        pin_number,
        XINC_GPIO_PIN_DIR_OUTPUT,
        XINC_GPIO_PIN_INPUT_NOINT,
        XINC_GPIO_PIN_NOPULL,
        XINC_GPIO_PIN_GPIODx,
        XINC_GPIO_PIN_DEBOUNCE_DISABLE);
}


__STATIC_INLINE void xinc_gpio_cfg_input(uint32_t pin_number, xinc_gpio_pin_input_cfg_t input_config)//o
{
    xinc_gpio_cfg(
        pin_number,
        XINC_GPIO_PIN_DIR_INPUT,
        input_config.input_int,
        input_config.pin_pulll,
        XINC_GPIO_PIN_GPIODx,
        XINC_GPIO_PIN_DEBOUNCE_ENABLE);
}


__STATIC_INLINE void xinc_gpio_cfg_default(uint32_t pin_number)//O
{
    xinc_gpio_cfg(
        pin_number,
        XINC_GPIO_PIN_DIR_INPUT,
        XINC_GPIO_PIN_INPUT_NOINT,
        XINC_GPIO_PIN_NOPULL,
        XINC_GPIO_PIN_GPIODx,
        XINC_GPIO_PIN_DEBOUNCE_DISABLE);
}



__STATIC_INLINE void xinc_gpio_pin_dir_set(uint32_t pin_number, xinc_gpio_pin_dir_t direction)//o
{
    uint8_t reg_idx = pin_number >> 4UL;
    uint8_t pin_idx = pin_number & 0xFUL;

    uint32_t regVal;
    
    XINC_GPIO_Type * preg = xinc_gpio_pin_port_decode(&pin_number);
    regVal = preg->PORT_DDR[reg_idx];
    if (direction == XINC_GPIO_PIN_DIR_INPUT)
    {
        
        regVal |= (0x10000 << pin_idx);
    }
    else
    {   
        regVal |= (0x10001 << pin_idx);
    }
    
     preg->PORT_DDR[reg_idx] = regVal;
    
    
}


__STATIC_INLINE void xinc_gpio_mux_ctl(uint32_t  pin_number,uint8_t mux)//o
{
    uint8_t reg_idx = pin_number >> 4UL;
    uint8_t pin_idx = pin_number & 0xFUL;

    uint32_t regVal;
    
#if defined(XC60XX_M0)
    XINC_CPR_CTL_Type * preg = xinc_gpio_pin_cpr_decode(&pin_number);
    regVal = preg->CTL_MUXCTL1_2[reg_idx];
    regVal = (regVal & ~(0x03 << (pin_idx << 1))) | (mux << (pin_idx << 1));
    preg->CTL_MUXCTL1_2[reg_idx] = regVal;
#elif defined (XC66XX_M4)

    XINC_CPR_CTL_Type * preg = xinc_gpio_pin_cpr_decode(&pin_number);
    if(reg_idx < 2)
    {
        regVal = preg->CTL_MUXCTL1_2[reg_idx];
        regVal = (regVal & ~(0x03 << (pin_idx << 1))) | (mux << (pin_idx << 1));
        preg->CTL_MUXCTL1_2[reg_idx] = regVal;
    }
    else
    {
        regVal = preg->CTL_MUXCTL3[reg_idx - 2];
        regVal = (regVal & ~(0x03 << (pin_idx << 1))) | (mux << (pin_idx << 1));
        preg->CTL_MUXCTL3[reg_idx - 2] = regVal;
    }
    
#endif    

}

__STATIC_INLINE void xinc_gpio_fun_sel(uint32_t pin_number,xinc_gpio_pin_fun_sel_t fun)//o
{
    XINCX_ASSERT(fun < XINC_GPIO_PIN_FUN_SEL_MAX);
    uint8_t reg_idx = pin_number >> 2UL;
    uint8_t pin_idx = pin_number & 0x3UL;

    uint32_t regVal;
    
    XINC_CPR_CTL_Type * preg = xinc_gpio_pin_cpr_decode(&pin_number);

    regVal = preg->GPIO_FUN_SELx[reg_idx];
    regVal = (regVal & ~(0x1FUL << (pin_idx << 3UL))) | (fun << (pin_idx << 3UL));
    preg->GPIO_FUN_SELx[reg_idx] = regVal;

    
}


__STATIC_INLINE void xinc_gpio_pull_sel(uint32_t pin_number,xinc_gpio_pin_pull_t pull)
{
    XINCX_ASSERT(pull <= XINC_GPIO_PIN_PULLUP);

    uint8_t reg_idx;
    uint8_t pin_idx ;
    uint32_t regVal;
       

#if defined(XC60XX_M0)
    XINC_CPR_AO_CTL_Type * preg = xinc_gpio_pin_pull_decode(&pin_number);

    reg_idx = pin_number >> 4UL;
    
    if(pin_number <= 4)
    {
        reg_idx = 1;
    }
    pin_idx = pin_number & 0xFUL;
    
    regVal = preg->PE_CTRLx[reg_idx];

    if(XINC_GPIO_PIN_PULLUP == pull)
    {
        xinc_bitmask_bit_set((pin_idx << 1) + 1,&regVal);
        xinc_bitmask_bit_clear(pin_idx << 1,&regVal);
    }
    else if(XINC_GPIO_PIN_PULLDOWN == pull)
    {
        xinc_bitmask_bit_clear((pin_idx << 1) + 1,&regVal);
        xinc_bitmask_bit_set(pin_idx << 1,&regVal);
    }
    else
    {
        xinc_bitmask_bit_clear((pin_idx << 1) + 1,&regVal);
        xinc_bitmask_bit_clear((pin_idx << 1),&regVal);
    }
	preg->PE_CTRLx[reg_idx] = regVal;
		 
    
#elif defined (XC66XX_M4)

    XINC_CPR_AO_CTL_Type * preg = xinc_gpio_pin_pull_decode(&pin_number);

    reg_idx = pin_number >> 4UL;
    if(pin_number <= 4)
    {
        reg_idx = 1;
    }
    pin_idx = pin_number & 0xFUL;
    regVal = preg->PE_CTRLx[reg_idx];

    if(XINC_GPIO_PIN_PULLUP == pull)
    {
        xinc_bitmask_bit_set((pin_idx << 1) + 1,&regVal);
        xinc_bitmask_bit_clear(pin_idx << 1,&regVal);
    }
    else if(XINC_GPIO_PIN_PULLDOWN == pull)
    {
        xinc_bitmask_bit_clear((pin_idx << 1) + 1,&regVal);
        xinc_bitmask_bit_set(pin_idx << 1,&regVal);
    }
    else
    {
        xinc_bitmask_bit_clear((pin_idx << 1) + 1,&regVal);
        xinc_bitmask_bit_clear((pin_idx << 1),&regVal);
    }
    preg->PE_CTRLx[reg_idx] = regVal;
#endif   
    
}

__STATIC_INLINE void xinc_gpio_inter_sel(uint32_t pin_number,xinc_gpio_pin_input_int_t inter)//o
{

    XINCX_ASSERT(inter <= XINC_GPIO_PIN_INPUTEDGE_IN);
    uint8_t reg_idx = pin_number >> 2UL;
    uint8_t pin_idx = pin_number & 0x3UL;

    uint32_t regVal;
    
    XINC_GPIO_Type * preg =  xinc_gpio_pin_port_decode(&pin_number);
    
    regVal = preg->INTR_CTRL[reg_idx];
    
   //  GPIO_INTR_CTRLx[y*4+3:y*4] x = n / 4，y = n % 4
    
    regVal = (regVal  & ~(0xF0000UL) & ~(0xFUL << (pin_idx << 2UL))) | (inter << (pin_idx << 2UL)) | ((0x01UL << pin_idx) << 16UL);
 
    preg->INTR_CTRL[reg_idx] = regVal;

    
}



__STATIC_INLINE void xinc_gpio_pin_set(uint32_t pin_number)//o
{
    XINC_GPIO_Type* reg     = xinc_gpio_pin_port_decode(&pin_number);
    uint8_t         port    = pin_number >> 4UL;
    uint8_t         reg_idx = pin_number & 0xFUL;

    reg->PORT_DR[port]      = (((GPIO_GPIO_PORT_DR_High << GPIO_GPIO_PORT_DR_Pos) |
                              (GPIO_GPIO_PORT_DR_WE_Enable << GPIO_GPIO_PORT_DR_WE_Pos)) << reg_idx);

}


__STATIC_INLINE void xinc_gpio_pin_clear(uint32_t pin_number)//o
{
    XINC_GPIO_Type* reg     = xinc_gpio_pin_port_decode(&pin_number);
    uint8_t         port    = pin_number >> 4UL;
    uint8_t         reg_idx = pin_number & 0xFUL;

    reg->PORT_DR[port]      = (((GPIO_GPIO_PORT_DR_Low << GPIO_GPIO_PORT_DR_Pos) |
                              (GPIO_GPIO_PORT_DR_WE_Enable << GPIO_GPIO_PORT_DR_WE_Pos)) << reg_idx);
}


__STATIC_INLINE void xinc_gpio_pin_toggle(uint32_t pin_number)//o
{
    uint8_t val =  xinc_gpio_pin_out_read(pin_number);
    xinc_gpio_pin_write(pin_number,!val);
}


__STATIC_INLINE void xinc_gpio_pin_write(uint32_t pin_number, uint32_t value)//o
{
    if (value == 0)
    {
        xinc_gpio_pin_clear(pin_number);
    }
    else
    {
        xinc_gpio_pin_set(pin_number);
    }
}


__STATIC_INLINE uint32_t xinc_gpio_pin_read(uint32_t pin_number)//o
{
    XINC_GPIO_Type * reg = xinc_gpio_pin_port_decode(&pin_number);
    uint8_t port = pin_number >> 5UL;
    uint8_t pin_idx = pin_number & 0x1FUL;

    uint32_t value = reg->EXT_PORT[port];

    return (value & (0x01UL << pin_idx))? (GPIO_GPIO_EXT_PORT_DIN_High) : (GPIO_GPIO_EXT_PORT_DIN_Low);

}


__STATIC_INLINE uint32_t xinc_gpio_pin_out_read(uint32_t pin_number)//o
{
    XINC_GPIO_Type * reg = xinc_gpio_pin_port_decode(&pin_number);
    uint8_t port = pin_number >> 5;
    uint8_t pin_idx = pin_number & 0x1F;
    uint32_t value = reg->EXT_PORT[port];

    return (value & (0x01UL << pin_idx))? (GPIO_GPIO_EXT_PORT_DIN_High) : (GPIO_GPIO_EXT_PORT_DIN_Low);

}


__STATIC_INLINE bool xinc_gpio_pin_present_check(uint32_t pin_number)//o
{
    uint32_t port = pin_number >> 5;
    uint32_t mask = 0;

    switch (port)
    {
#ifdef P0_FEATURE_PINS_PRESENT
        case 0:
            mask = P0_FEATURE_PINS_PRESENT;

            break;
#endif
#ifdef P1_FEATURE_PINS_PRESENT
        case 1:
            mask = P1_FEATURE_PINS_PRESENT;
            break;
#endif
    }

    pin_number &= 0x1F;

    return (mask & (1UL << pin_number)) ? true : false;
}

#endif // SUPPRESS_INLINE_IMPLEMENTATION

ret_code_t xinc_gpio_secfun_config(uint32_t pin,xinc_gpio_pin_fun_sel_t fun);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_GPIO_H__
