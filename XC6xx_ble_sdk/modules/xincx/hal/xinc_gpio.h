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
#include "xinc_52840_bitfields.h"
#include "XC620610.h"
#include "bsp_gpio.h"
#ifdef __cplusplus
extern "C" {
#endif


#ifndef XINC_P0
#define XINC_P0 XINC_GPIO
#endif

#if (GPIO_COUNT == 1)
#define NUMBER_OF_PINS (P0_PIN_NUM)
#define GPIO_REG_LIST  {XINC_P0}
#else
#error "Not supported."
#endif

#if defined(XINC52820_XXAA)
#include <xinc_erratas.h>
#endif

/**
 * @defgroup xinc_gpio_hal GPIO HAL
 * @{
 * @ingroup xinc_gpio
 * @brief   Hardware access layer for managing the GPIO peripheral.
 */


/** @brief Macro for mapping port and pin numbers to values understandable for xinc_gpio functions. */
#define XINC_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))

#define XINC_GPIO_0				    XINC_GPIO_PIN_MAP(0, 0)
#define XINC_GPIO_1				    XINC_GPIO_PIN_MAP(0, 1)
#define XINC_GPIO_2				    XINC_GPIO_PIN_MAP(0, 2)
#define XINC_GPIO_3				    XINC_GPIO_PIN_MAP(0, 3)
#define XINC_GPIO_4				    XINC_GPIO_PIN_MAP(0, 4)
#define XINC_GPIO_5				    XINC_GPIO_PIN_MAP(0, 5)
#define XINC_GPIO_6				    XINC_GPIO_PIN_MAP(0, 6)
#define XINC_GPIO_7				    XINC_GPIO_PIN_MAP(0, 7)
#define XINC_GPIO_8				    XINC_GPIO_PIN_MAP(0, 8)
#define XINC_GPIO_9				    XINC_GPIO_PIN_MAP(0, 9)
#define XINC_GPIO_10				XINC_GPIO_PIN_MAP(0, 10)
#define XINC_GPIO_11				XINC_GPIO_PIN_MAP(0, 11)
#define XINC_GPIO_12				XINC_GPIO_PIN_MAP(0, 12)
#define XINC_GPIO_13				XINC_GPIO_PIN_MAP(0, 13)
#define XINC_GPIO_14				XINC_GPIO_PIN_MAP(0, 14)
#define XINC_GPIO_15				XINC_GPIO_PIN_MAP(0, 15)
#define XINC_GPIO_16				XINC_GPIO_PIN_MAP(0, 16)
#define XINC_GPIO_17				XINC_GPIO_PIN_MAP(0, 17)
#define XINC_GPIO_18				XINC_GPIO_PIN_MAP(0, 18)
#define XINC_GPIO_19				XINC_GPIO_PIN_MAP(0, 19)
#define XINC_GPIO_20				XINC_GPIO_PIN_MAP(0, 20)
#define XINC_GPIO_21				XINC_GPIO_PIN_MAP(0, 21)
#define XINC_GPIO_22				XINC_GPIO_PIN_MAP(0, 22)
#define XINC_GPIO_23				XINC_GPIO_PIN_MAP(0, 23)
#define XINC_GPIO_24				XINC_GPIO_PIN_MAP(0, 24)
#define XINC_GPIO_25				XINC_GPIO_PIN_MAP(0, 25)
#define XINC_GPIO_26				XINC_GPIO_PIN_MAP(0, 26)
#define XINC_GPIO_27				XINC_GPIO_PIN_MAP(0, 27)
#define XINC_GPIO_28				XINC_GPIO_PIN_MAP(0, 28)
#define XINC_GPIO_29				XINC_GPIO_PIN_MAP(0, 29)
#define XINC_GPIO_30				XINC_GPIO_PIN_MAP(0, 30)
#define XINC_GPIO_31				XINC_GPIO_PIN_MAP(0, 31)

#include "bsp_gpio.h"


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
    XINC_GPIO_PIN_INPUT_NOINT            = NOT_INT,   ///< Connect input No int.
    XINC_GPIO_PIN_INPUT_RIS_EDGE_INT     = RIS_EDGE_INT,   //
    XINC_GPIO_PIN_INPUT_FAIL_EDGE_INT    = FAIL_EDGE_INT,   //
    XINC_GPIO_PIN_INPUTEDGE_IN           = RIS_FAIL_EDGE_INT,   //
} xinc_gpio_pin_input_int_t;

/**
 * @brief Enumerator used for selecting the pin to be pulled down or up at the time of pin
 * configuration.
 */
typedef enum
{
    XINC_GPIO_PIN_NOPULL   = GPIO_Mode_Input_Float, ///<  Pin pull-up resistor disabled.
    XINC_GPIO_PIN_PULLDOWN = GPIO_Mode_Input_Down, ///<  Pin pull-down resistor enabled.
    XINC_GPIO_PIN_PULLUP   = GPIO_Mode_Input_Up,   ///<  Pin pull-up resistor enabled.
} xinc_gpio_pin_pull_t;


/** @brief Enumerator used for selecting output drive mode. */
typedef enum
{
    XINC_GPIO_PIN_GPIODx = GPIO_Dx, ///< 
    XINC_GPIO_PIN_UART0_TX = UART0_TX, ///
    XINC_GPIO_PIN_UART0_RX = UART0_RX, ///< 
    XINC_GPIO_PIN_UART0_CTS = UART0_CTS, //
    XINC_GPIO_PIN_UART0_RTS = UART0_RTS, //
    XINC_GPIO_PIN_I2C_SCL = I2C_SCL, //
    XINC_GPIO_PIN_I2C_SDA = I2C_SDA, //
    XINC_GPIO_PIN_UART1_RX = UART1_RX, //
    XINC_GPIO_PIN_UART1_TX = UART1_TX, //
    XINC_GPIO_PIN_SIM_IO = SIM_IO, //
    XINC_GPIO_PIN_SIM_RST = SIM_RST, //
    XINC_GPIO_PIN_SIM_CLK_OUT = SIM_CLK_OUT, //
    XINC_GPIO_PIN_PWM0 = PWM0, //
    XINC_GPIO_PIN_PWM1 = PWM1, //
    XINC_GPIO_PIN_SSI1_CLK = SSI1_CLK, //
    XINC_GPIO_PIN_SSI1_SSN = SSI1_SSN, //
    XINC_GPIO_PIN_SSI1_RX = SSI1_RX, //
    XINC_GPIO_PIN_SSI1_TX = SSI1_TX, //
    XINC_GPIO_PIN_PWM0_INV = PWM0_INV, //
    XINC_GPIO_PIN_PWM1_INV = PWM1_INV, // 
    //////////////////////////////////////////////////////

    XINC_GPIO_PIN_PWM2,
    XINC_GPIO_PIN_PWM3,
    XINC_GPIO_PIN_PWM4,
    XINC_GPIO_PIN_PWM5,
		
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
 * @param pull_config      State of the pin range pull resistor (no pull, pulled down, or pulled high).
 */
__STATIC_INLINE void xinc_gpio_range_cfg_input(uint32_t            pin_range_start,
                                              uint32_t            pin_range_end,
                                              xinc_gpio_pin_pull_t pull_config);

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
__STATIC_INLINE void xinc_gpio_cfg_output(uint32_t pin_number);

/**
 * @brief Function for configuring the given GPIO pin number as input, hiding inner details.
 *        This function can be used to configure a pin as simple input.
 *
 * @note  Sense capability on the pin is disabled and input is connected to buffer so that the GPIO->IN register is readable.
 *
 * @param pin_number  Specifies the pin number.
 * @param pull_config State of the pin range pull resistor (no pull, pulled down, or pulled high).
 */
__STATIC_INLINE void xinc_gpio_cfg_input(uint32_t pin_number, xinc_gpio_pin_pull_t pull_config);

/**
 * @brief Function for resetting pin configuration to its default state.
 *
 * @param pin_number Specifies the pin number.
 */
__STATIC_INLINE void xinc_gpio_cfg_default(uint32_t pin_number);


/**
 * @brief Function for setting the direction for a GPIO pin.
 *
 * @param pin_number Specifies the pin number for which to set the direction.
 * @param direction  Specifies the direction.
 */
__STATIC_INLINE void xinc_gpio_pin_dir_set(uint32_t pin_number, xinc_gpio_pin_dir_t direction);

/**
 * @brief Function for setting a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to be set.
 */
__STATIC_INLINE void xinc_gpio_pin_set(uint32_t pin_number);

/**
 * @brief Function for clearing a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to clear.
 */
__STATIC_INLINE void xinc_gpio_pin_clear(uint32_t pin_number);

/**
 * @brief Function for toggling a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to toggle.
 */
__STATIC_INLINE void xinc_gpio_pin_toggle(uint32_t pin_number);

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
__STATIC_INLINE void xinc_gpio_pin_write(uint32_t pin_number, uint32_t value);

/**
 * @brief Function for reading the input level of a GPIO pin.
 *
 * If the value returned by this function is to be valid, the pin's input buffer must be connected.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return 0 if the pin input level is low. Positive value if the pin is high.
 */
__STATIC_INLINE uint32_t xinc_gpio_pin_read(uint32_t pin_number);

/**
 * @brief Function for reading the output level of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return 0 if the pin output level is low. Positive value if pin output is high.
 */
__STATIC_INLINE uint32_t xinc_gpio_pin_out_read(uint32_t pin_number);

/**
 * @brief Function for reading the sense configuration of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return Sense configuration.
 */
//__STATIC_INLINE xinc_gpio_pin_sense_t xinc_gpio_pin_sense_get(uint32_t pin_number);

/**
 * @brief Function for reading the direction configuration of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return Direction configuration.
 */
__STATIC_INLINE xinc_gpio_pin_dir_t xinc_gpio_pin_dir_get(uint32_t pin_number);

/**
 * @brief Function for reading the status of GPIO pin input buffer.
 *
 * @param pin_number Pin number to be read.
 *
 * @retval Input buffer configuration.
 */
//__STATIC_INLINE xinc_gpio_pin_input_t xinc_gpio_pin_input_get(uint32_t pin_number);

/**
 * @brief Function for reading the pull configuration of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @retval Pull configuration.
 */
__STATIC_INLINE xinc_gpio_pin_pull_t xinc_gpio_pin_pull_get(uint32_t pin_number);

/**
 * @brief Function for setting output direction on the selected pins on the given port.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param out_mask Mask specifying the pins to set as output.
 */
__STATIC_INLINE void xinc_gpio_port_dir_output_set(XINC_GPIO_Type * p_reg, uint32_t out_mask);

/**
 * @brief Function for setting input direction on selected pins on a given port.
 *
 * @param p_reg   Pointer to the structure of registers of the peripheral.
 * @param in_mask Mask that specifies the pins to be set as input.
 */
__STATIC_INLINE void xinc_gpio_port_dir_input_set(XINC_GPIO_Type * p_reg, uint32_t in_mask);

/**
 * @brief Function for writing the direction configuration of the GPIO pins in the given port.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param dir_mask Mask that specifies the direction of pins. Bit set means that the given pin is configured as output.
 */
__STATIC_INLINE void xinc_gpio_port_dir_write(XINC_GPIO_Type * p_reg, uint32_t dir_mask);

/**
 * @brief Function for reading the direction configuration of a GPIO port.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Pin configuration of the current direction settings. Bit set means that the given pin is configured as output.
 */
__STATIC_INLINE uint32_t xinc_gpio_port_dir_read(XINC_GPIO_Type const * p_reg);

/**
 * @brief Function for reading the input signals of the GPIO pins on the given port.
 *
 * @param p_reg Pointer to the peripheral registers structure.
 *
 * @return Port input values.
 */
__STATIC_INLINE uint32_t xinc_gpio_port_in_read(XINC_GPIO_Type const * p_reg);

/**
 * @brief Function for reading the output signals of the GPIO pins on the given port.
 *
 * @param p_reg Pointer to the peripheral registers structure.
 *
 * @return Port output values.
 */
__STATIC_INLINE uint32_t xinc_gpio_port_out_read(XINC_GPIO_Type const * p_reg);

/**
 * @brief Function for writing the GPIO pins output on a given port.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param value Output port mask.
 */
__STATIC_INLINE void xinc_gpio_port_out_write(XINC_GPIO_Type * p_reg, uint32_t value);

/**
 * @brief Function for setting high level on selected the GPIO pins on the given port.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param set_mask Mask with pins to be set as logical high level.
 */
__STATIC_INLINE void xinc_gpio_port_out_set(XINC_GPIO_Type * p_reg, uint32_t set_mask);

/**
 * @brief Function for setting low level on selected the GPIO pins on the given port.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param clr_mask Mask with pins to be set as logical low level.
 */
__STATIC_INLINE void xinc_gpio_port_out_clear(XINC_GPIO_Type * p_reg, uint32_t clr_mask);

/**
 * @brief Function for reading pin state of multiple consecutive ports.
 *
 * @param start_port Index of the first port to read.
 * @param length     Number of ports to read.
 * @param p_masks    Pointer to output array where port states will be stored.
 */
__STATIC_INLINE void xinc_gpio_ports_read(uint32_t start_port, uint32_t length, uint32_t * p_masks);


/**
 * @brief Function for checking if provided pin is present on the MCU.
 *
 * @param[in] pin_number Number of the pin to be checked.
 *
 * @retval true  Pin is present.
 * @retval false Pin is not present.
 */
__STATIC_INLINE bool xinc_gpio_pin_present_check(uint32_t pin_number);


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
//    XINCX_ASSERT(xinc_gpio_pin_present_check(*p_pin));
#if (GPIO_COUNT == 1)
    return XINC_P0;
#else
    if (*p_pin < P0_PIN_NUM)
    {
        return XINC_P0;
    }
    else
    {
        *p_pin = *p_pin & 0x1F;
        return XINC_P1;
    }
#endif
}

__STATIC_INLINE XINC_CPRA_AO_Type * xinc_gpio_pin_pull_decode(uint32_t * p_pin)
{
//    XINCX_ASSERT(xinc_gpio_pin_present_check(*p_pin));

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
                                              xinc_gpio_pin_pull_t pull_config)
{
    /*lint -e{845} // A zero has been given as right argument to operator '|'" */
    for (; pin_range_start <= pin_range_end; pin_range_start++)
    {
        xinc_gpio_cfg_input(pin_range_start, pull_config);
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
__STATIC_INLINE void xinc_gpio_fun_inter(uint32_t  pin_number,xinc_gpio_pin_input_int_t inter)
{



}
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
__STATIC_INLINE void xinc_gpio_mux_ctl(uint32_t  pin_number,uint8_t mux)
{



}


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
__STATIC_INLINE void xinc_gpio_fun_sel(uint32_t pin_number,xinc_gpio_pin_fun_sel_t fun)
{

}
	
__STATIC_INLINE void xinc_gpio_dir_cfg(uint32_t pin_number,xinc_gpio_pin_dir_t dir)
{

}


__STATIC_INLINE void xinc_gpio_cfg(
    uint32_t             pin_number,
    xinc_gpio_pin_dir_t   dir,
    xinc_gpio_pin_input_int_t inter,
    xinc_gpio_pin_pull_t  pull,
    xinc_gpio_pin_fun_sel_t fun,
    xinc_gpio_pin_debounce_t debounce)
{
    XINC_GPIO_Type * reg = xinc_gpio_pin_port_decode(&pin_number);
    XINC_CPRA_AO_Type * pull_reg = xinc_gpio_pin_pull_decode(&pin_number);

    uint32_t reg_value ;
    uint32_t tmp_num;
    tmp_num = pin_number;
    uint8_t reg_idx = 0;

    gpio_fun_sel(pin_number,fun);
    if(XINC_GPIO_PIN_DIR_INPUT == dir)
    {
        gpio_fun_inter(pin_number,inter);
        if(/*(pin_number >= 0 &&) */pin_number <= 4)
        {
            reg_value = pull_reg->PE_CTRL2 ;
            reg_idx = 2;
        }
        else if(pin_number >= 5 && pin_number <= 15)
        {
            reg_value = pull_reg->PU_CTRL1 ;
            reg_idx = 1;
            if(pin_number >= 11 && pin_number <= 13)
            {
                pin_number = pin_number + 2;
                reg_value = pull_reg->PE_CTRL2;
                reg_idx = 2;
            }
        }
        else if(pin_number >= 16 && pin_number <= 28)
        {
            reg_value = pull_reg->PE_CTRL2;
            reg_idx = 2;
            pin_number = pin_number % 16;
        }

		
        if(XINC_GPIO_PIN_PULLUP == pull)
        {
                clrbit(reg_value,pin_number*2);
                setbit(reg_value,pin_number*2+1);
            
        }else if(XINC_GPIO_PIN_PULLDOWN == pull)
        {
            setbit(reg_value,pin_number*2);
            clrbit(reg_value,pin_number*2+1);
        }
        else
        {
                clrbit(reg_value,pin_number*2);
                clrbit(reg_value,pin_number*2+1);
        }
        if(reg_idx == 1)
        {
            pull_reg->PU_CTRL1 = reg_value;
        }else
        {
            pull_reg->PE_CTRL2 = reg_value;
        }
		
        if(tmp_num >= 16)
        {
            reg_value = reg->PORT_DDR[1];
            reg_value |= (0x10000 << (tmp_num & 0x0F));
            reg->PORT_DDR[1] = reg_value;
            
            if(XINC_GPIO_PIN_DEBOUNCE_ENABLE == debounce )
            {
                reg_value = reg->DEBOUNCE[1];
                reg_value |= (0x10001 << (tmp_num & 0x0F));
                reg->DEBOUNCE[1] = reg_value;
            }

            
        }else
        {
            reg_value = reg->PORT_DDR[0];
            reg_value |= (0x10000 << (tmp_num & 0x0F));
            reg->PORT_DDR[0] = reg_value;
            if(XINC_GPIO_PIN_DEBOUNCE_ENABLE == debounce )
            {
                reg_value = reg->DEBOUNCE[0];
                reg_value |= (0x10001 << (tmp_num & 0x0F));
            //	reg->DEBOUNCE[0] = reg_value;
            }		
        }
    }else
    {
        if(tmp_num >= 16)
        {
            reg_value = reg->PORT_DDR[1];
            reg_value |= (0x10001 << (tmp_num & 0x0F));
            reg->PORT_DDR[1] = reg_value;      
        }else
        {
            reg_value = reg->PORT_DDR[0];
            reg_value |= (0x10001 << (tmp_num & 0x0F));
            reg->PORT_DDR[0] = reg_value;
                
        }
    }

}


__STATIC_INLINE void xinc_gpio_cfg_output(uint32_t pin_number)
{
    xinc_gpio_cfg(
        pin_number,
        XINC_GPIO_PIN_DIR_OUTPUT,
        XINC_GPIO_PIN_INPUT_NOINT,
        XINC_GPIO_PIN_NOPULL,
        XINC_GPIO_PIN_GPIODx,
        XINC_GPIO_PIN_DEBOUNCE_DISABLE);
}


__STATIC_INLINE void xinc_gpio_cfg_input(uint32_t pin_number, xinc_gpio_pin_pull_t pull_config)
{
    xinc_gpio_cfg(
        pin_number,
        XINC_GPIO_PIN_DIR_INPUT,
        XINC_GPIO_PIN_INPUT_RIS_EDGE_INT,
        pull_config,
        XINC_GPIO_PIN_GPIODx,
        XINC_GPIO_PIN_DEBOUNCE_ENABLE);
}


__STATIC_INLINE void xinc_gpio_cfg_default(uint32_t pin_number)
{
    xinc_gpio_cfg(
        pin_number,
        XINC_GPIO_PIN_DIR_INPUT,
        XINC_GPIO_PIN_INPUT_NOINT,
        XINC_GPIO_PIN_NOPULL,
        XINC_GPIO_PIN_GPIODx,
        XINC_GPIO_PIN_DEBOUNCE_DISABLE);
}




__STATIC_INLINE void xinc_gpio_cfg_sense_set(uint32_t pin_number, xinc_gpio_pin_debounce_t debounce_config)
{
    XINC_GPIO_Type * reg = xinc_gpio_pin_port_decode(&pin_number);

}


__STATIC_INLINE void xinc_gpio_pin_dir_set(uint32_t pin_number, xinc_gpio_pin_dir_t direction)
{
    if (direction == XINC_GPIO_PIN_DIR_INPUT)
    {
        xinc_gpio_cfg(
            pin_number,
            XINC_GPIO_PIN_DIR_INPUT,
            XINC_GPIO_PIN_INPUT_NOINT,
            XINC_GPIO_PIN_NOPULL,
            XINC_GPIO_PIN_GPIODx,
            XINC_GPIO_PIN_DEBOUNCE_ENABLE);
    }
    else
    {
         xinc_gpio_cfg(
            pin_number,
            XINC_GPIO_PIN_DIR_OUTPUT,
            XINC_GPIO_PIN_INPUT_NOINT,
            XINC_GPIO_PIN_NOPULL,
            XINC_GPIO_PIN_GPIODx,
            XINC_GPIO_PIN_DEBOUNCE_DISABLE);
    }
}


__STATIC_INLINE void xinc_gpio_pin_set(uint32_t pin_number)
{
    XINC_GPIO_Type * reg = xinc_gpio_pin_port_decode(&pin_number);
		uint8_t port = pin_number >> 4UL;
    uint8_t reg_idx = pin_number & 0xFUL;

    reg->PORT_DR[port] = (((GPIO_GPIO_PORT_DR_High << GPIO_GPIO_PORT_DR_Pos) |
													(GPIO_GPIO_PORT_DR_WE_Enable << GPIO_GPIO_PORT_DR_WE_Pos)) << reg_idx);

}


__STATIC_INLINE void xinc_gpio_pin_clear(uint32_t pin_number)
{
    XINC_GPIO_Type * reg = xinc_gpio_pin_port_decode(&pin_number);
    uint8_t port = pin_number >> 4UL;
    uint8_t reg_idx = pin_number & 0xFUL;

		reg->PORT_DR[port] = (((GPIO_GPIO_PORT_DR_Low << GPIO_GPIO_PORT_DR_Pos) |
													(GPIO_GPIO_PORT_DR_WE_Enable << GPIO_GPIO_PORT_DR_WE_Pos)) << reg_idx);
}


__STATIC_INLINE void xinc_gpio_pin_toggle(uint32_t pin_number)
{
    XINC_GPIO_Type * reg        = xinc_gpio_pin_port_decode(&pin_number);
	
		uint8_t port = pin_number >> 4UL;
    uint8_t reg_idx = pin_number & 0xFUL;

    reg->PORT_DR[port] = (((GPIO_GPIO_PORT_DR_High << GPIO_GPIO_PORT_DR_Pos) |
													(GPIO_GPIO_PORT_DR_WE_Enable << GPIO_GPIO_PORT_DR_WE_Pos)) << reg_idx);
	
    reg->PORT_DR[port] = (((GPIO_GPIO_PORT_DR_Low << GPIO_GPIO_PORT_DR_Pos) |
													(GPIO_GPIO_PORT_DR_WE_Enable << GPIO_GPIO_PORT_DR_WE_Pos)) << reg_idx);
	

}


__STATIC_INLINE void xinc_gpio_pin_write(uint32_t pin_number, uint32_t value)
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


__STATIC_INLINE uint32_t xinc_gpio_pin_read(uint32_t pin_number)
{
    XINC_GPIO_Type * reg = xinc_gpio_pin_port_decode(&pin_number);
		uint8_t port = pin_number >> 5;
		uint8_t pin_idx = pin_number & 0x1F;

    uint32_t value = reg->EXT_PORT[port];

    return (value & (0x01UL << pin_idx))? (GPIO_GPIO_EXT_PORT_DIN_High) : (GPIO_GPIO_EXT_PORT_DIN_Low);

}


__STATIC_INLINE uint32_t xinc_gpio_pin_out_read(uint32_t pin_number)
{
    XINC_GPIO_Type * reg = xinc_gpio_pin_port_decode(&pin_number);
		uint8_t port = pin_number >> 5;
		uint8_t pin_idx = pin_number & 0x1F;
    uint32_t value = reg->EXT_PORT[port];

    return (value & (0x01UL << pin_idx))? (GPIO_GPIO_EXT_PORT_DIN_High) : (GPIO_GPIO_EXT_PORT_DIN_Low);

}



__STATIC_INLINE void xinc_gpio_ports_read(uint32_t start_port, uint32_t length, uint32_t * p_masks)
{
    XINC_GPIO_Type * gpio_regs[GPIO_COUNT] = GPIO_REG_LIST;

 //   XINCX_ASSERT(start_port + length <= GPIO_COUNT);
    uint32_t i;
	

    for (i = start_port; i < (start_port + length); )
    {
			uint8_t port = i >> 5;
			*p_masks = gpio_regs[0]->EXT_PORT[port];
      p_masks++;
			i+= 32;
    }
}


__STATIC_INLINE bool xinc_gpio_pin_present_check(uint32_t pin_number)
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
