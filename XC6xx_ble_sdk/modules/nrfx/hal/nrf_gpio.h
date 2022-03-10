/**
 * Copyright (c) 2015 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef NRF_GPIO_H__
#define NRF_GPIO_H__

#include <nrfx.h>
#include "nrf_52840_bitfields.h"
#include "nrf52840.h"
#include "bsp_gpio.h"
#ifdef __cplusplus
extern "C" {
#endif


#ifndef NRF_P0
#define NRF_P0 NRF_GPIO
#endif

#if (GPIO_COUNT == 1)
#define NUMBER_OF_PINS (P0_PIN_NUM)
#define GPIO_REG_LIST  {NRF_P0}
#else
#error "Not supported."
#endif

#if defined(NRF52820_XXAA)
#include <nrf_erratas.h>
#endif

/**
 * @defgroup nrf_gpio_hal GPIO HAL
 * @{
 * @ingroup nrf_gpio
 * @brief   Hardware access layer for managing the GPIO peripheral.
 */


/** @brief Macro for mapping port and pin numbers to values understandable for nrf_gpio functions. */
#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))


#define XINC_GPIO_0				NRF_GPIO_PIN_MAP(0, 0)
#define XINC_GPIO_1				NRF_GPIO_PIN_MAP(0, 1)
#define XINC_GPIO_2				NRF_GPIO_PIN_MAP(0, 2)
#define XINC_GPIO_3				NRF_GPIO_PIN_MAP(0, 3)
#define XINC_GPIO_4				NRF_GPIO_PIN_MAP(0, 4)
#define XINC_GPIO_5				NRF_GPIO_PIN_MAP(0, 5)
#define XINC_GPIO_6				NRF_GPIO_PIN_MAP(0, 6)
#define XINC_GPIO_7				NRF_GPIO_PIN_MAP(0, 7)
#define XINC_GPIO_8				NRF_GPIO_PIN_MAP(0, 8)
#define XINC_GPIO_9				NRF_GPIO_PIN_MAP(0, 9)
#define XINC_GPIO_10				NRF_GPIO_PIN_MAP(0, 10)
#define XINC_GPIO_11				NRF_GPIO_PIN_MAP(0, 11)
#define XINC_GPIO_12				NRF_GPIO_PIN_MAP(0, 12)
#define XINC_GPIO_13				NRF_GPIO_PIN_MAP(0, 13)
#define XINC_GPIO_14				NRF_GPIO_PIN_MAP(0, 14)
#define XINC_GPIO_15				NRF_GPIO_PIN_MAP(0, 15)
#define XINC_GPIO_16				NRF_GPIO_PIN_MAP(0, 16)
#define XINC_GPIO_17				NRF_GPIO_PIN_MAP(0, 17)
#define XINC_GPIO_18				NRF_GPIO_PIN_MAP(0, 18)
#define XINC_GPIO_19				NRF_GPIO_PIN_MAP(0, 19)
#define XINC_GPIO_20				NRF_GPIO_PIN_MAP(0, 20)
#define XINC_GPIO_21				NRF_GPIO_PIN_MAP(0, 21)
#define XINC_GPIO_22				NRF_GPIO_PIN_MAP(0, 22)
#define XINC_GPIO_23				NRF_GPIO_PIN_MAP(0, 23)
#define XINC_GPIO_24				NRF_GPIO_PIN_MAP(0, 24)
#define XINC_GPIO_25				NRF_GPIO_PIN_MAP(0, 25)
#define XINC_GPIO_26				NRF_GPIO_PIN_MAP(0, 26)
#define XINC_GPIO_27				NRF_GPIO_PIN_MAP(0, 27)
#define XINC_GPIO_28				NRF_GPIO_PIN_MAP(0, 28)
#define XINC_GPIO_29				NRF_GPIO_PIN_MAP(0, 29)
#define XINC_GPIO_30				NRF_GPIO_PIN_MAP(0, 30)
#define XINC_GPIO_31				NRF_GPIO_PIN_MAP(0, 31)

#include "bsp_gpio.h"
/** @brief Pin direction definitions. */
typedef enum
{
    NRF_GPIO_PIN_DIR_INPUT  = GPIO_PIN_CNF_DIR_Input, ///< Input.
    NRF_GPIO_PIN_DIR_OUTPUT = GPIO_PIN_CNF_DIR_Output ///< Output.
} nrf_gpio_pin_dir_t;

/** @brief Connection of input buffer. */
typedef enum
{
  NRF_GPIO_PIN_INPUT_NOINT    = NOT_INT,   ///< Connect input No int.
  NRF_GPIO_PIN_INPUT_RIS_EDGE_INT    = RIS_EDGE_INT,   //
	NRF_GPIO_PIN_INPUT_FAIL_EDGE_INT    = FAIL_EDGE_INT,   //
	NRF_GPIO_PIN_INPUTEDGE_INT    = RIS_FAIL_EDGE_INT,   //
} nrf_gpio_pin_input_int_t;

/**
 * @brief Enumerator used for selecting the pin to be pulled down or up at the time of pin
 * configuration.
 */
typedef enum
{
    NRF_GPIO_PIN_NOPULL   = GPIO_Mode_Input_Float, ///<  Pin pull-up resistor disabled.
    NRF_GPIO_PIN_PULLDOWN = GPIO_Mode_Input_Down, ///<  Pin pull-down resistor enabled.
    NRF_GPIO_PIN_PULLUP   = GPIO_Mode_Input_Up,   ///<  Pin pull-up resistor enabled.
} nrf_gpio_pin_pull_t;


/** @brief Enumerator used for selecting output drive mode. */
typedef enum
{
    NRF_GPIO_PIN_GPIODx = GPIO_Dx, ///< 
    NRF_GPIO_PIN_UART0_TX = UART0_TX, ///
    NRF_GPIO_PIN_UART0_RX = UART0_RX, ///< 
    NRF_GPIO_PIN_UART0_CTS = UART0_CTS, //
    NRF_GPIO_PIN_UART0_RTS = UART0_RTS, //
    NRF_GPIO_PIN_I2C_SCL = I2C_SCL, //
    NRF_GPIO_PIN_I2C_SDA = I2C_SDA, //
    NRF_GPIO_PIN_UART1_RX = UART1_RX, //
	  NRF_GPIO_PIN_UART1_TX = UART1_TX, //
		NRF_GPIO_PIN_SIM_IO = SIM_IO, //
		NRF_GPIO_PIN_SIM_RST = SIM_RST, //
		NRF_GPIO_PIN_SIM_CLK_OUT = SIM_CLK_OUT, //
		NRF_GPIO_PIN_PWM0 = PWM0, //
		NRF_GPIO_PIN_PWM1 = PWM1, //
		NRF_GPIO_PIN_SSI1_CLK = SSI1_CLK, //
		NRF_GPIO_PIN_SSI1_SSN = SSI1_SSN, //
		NRF_GPIO_PIN_SSI1_RX = SSI1_RX, //
		NRF_GPIO_PIN_SSI1_TX = SSI1_TX, //
		NRF_GPIO_PIN_PWM0_INV = PWM0_INV, //
		NRF_GPIO_PIN_PWM1_INV = PWM1_INV, // 
	//////////////////////////////////////////////////////
	
		NRF_GPIO_PIN_PWM2,
		NRF_GPIO_PIN_PWM3,
		NRF_GPIO_PIN_PWM4,
		NRF_GPIO_PIN_PWM5,
	
	
		
} nrf_gpio_pin_fun_sel_t;

/** @brief Enumerator used for selecting the pin to sense high or low level on the pin input. */
typedef enum
{
    NRF_GPIO_PIN_MUX0    = 0, ///<  Pin sense level disabled.
    NRF_GPIO_PIN_MUX1  = 1,      ///<  Pin sense low level.
    NRF_GPIO_PIN_MUX2 = 2,     ///<  Pin sense high level.
	  NRF_GPIO_PIN_MUX3 = 3
} nrf_gpio_pin_mux_t;

/** @brief Enumerator used for selecting the pin to sense high or low level on the pin input. */
typedef enum
{
		NRF_GPIO_PIN_DEBOUNCE_DISABLE = 0,
    NRF_GPIO_PIN_DEBOUNCE_ENABLE    = 1, ///<  Pin DEBOUNCE  enabled.
} nrf_gpio_pin_debounce_t;

/**
 * @brief Function for configuring the GPIO pin range as output pins with normal drive strength.
 *        This function can be used to configure pin range as simple output with gate driving GPIO_PIN_CNF_DRIVE_S0S1 (normal cases).
 *
 * @note For configuring only one pin as output, use @ref nrf_gpio_cfg_output.
 *       Sense capability on the pin is disabled and input is disconnected from the buffer as the pins are configured as output.
 *
 * @param pin_range_start  Specifies the start number (inclusive) in the range of pin numbers to be configured (allowed values 0-30).
 * @param pin_range_end    Specifies the end number (inclusive) in the range of pin numbers to be configured (allowed values 0-30).
 */
__STATIC_INLINE void nrf_gpio_range_cfg_output(uint32_t pin_range_start, uint32_t pin_range_end);

/**
 * @brief Function for configuring the GPIO pin range as input pins with given initial value set, hiding inner details.
 *        This function can be used to configure pin range as simple input.
 *
 * @note  For configuring only one pin as input, use @ref nrf_gpio_cfg_input.
 *        Sense capability on the pin is disabled and input is connected to buffer so that the GPIO->IN register is readable.
 *
 * @param pin_range_start  Specifies the start number (inclusive) in the range of pin numbers to be configured (allowed values 0-30).
 * @param pin_range_end    Specifies the end number (inclusive) in the range of pin numbers to be configured (allowed values 0-30).
 * @param pull_config      State of the pin range pull resistor (no pull, pulled down, or pulled high).
 */
__STATIC_INLINE void nrf_gpio_range_cfg_input(uint32_t            pin_range_start,
                                              uint32_t            pin_range_end,
                                              nrf_gpio_pin_pull_t pull_config);

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
__STATIC_INLINE void nrf_gpio_cfg(
    uint32_t             pin_number,
    nrf_gpio_pin_dir_t   dir,
    nrf_gpio_pin_input_int_t inter,
    nrf_gpio_pin_pull_t  pull,
    nrf_gpio_pin_fun_sel_t fun,
    nrf_gpio_pin_debounce_t debounce);


/**
 * @brief Function for configuring the given GPIO pin number as output, hiding inner details.
 *        This function can be used to configure a pin as simple output with gate driving GPIO_PIN_CNF_DRIVE_S0S1 (normal cases).
 *
 * @note  Sense capability on the pin is disabled and input is disconnected from the buffer as the pins are configured as output.
 *
 * @param pin_number Specifies the pin number.
 */
__STATIC_INLINE void nrf_gpio_cfg_output(uint32_t pin_number);

/**
 * @brief Function for configuring the given GPIO pin number as input, hiding inner details.
 *        This function can be used to configure a pin as simple input.
 *
 * @note  Sense capability on the pin is disabled and input is connected to buffer so that the GPIO->IN register is readable.
 *
 * @param pin_number  Specifies the pin number.
 * @param pull_config State of the pin range pull resistor (no pull, pulled down, or pulled high).
 */
__STATIC_INLINE void nrf_gpio_cfg_input(uint32_t pin_number, nrf_gpio_pin_pull_t pull_config);

/**
 * @brief Function for resetting pin configuration to its default state.
 *
 * @param pin_number Specifies the pin number.
 */
__STATIC_INLINE void nrf_gpio_cfg_default(uint32_t pin_number);

/**
 * @brief Function for configuring the given GPIO pin number as a watcher. Only input is connected.
 *
 * @param pin_number Specifies the pin number.
 *
 */
__STATIC_INLINE void nrf_gpio_cfg_watcher(uint32_t pin_number);

/**
 * @brief Function for disconnecting input for the given GPIO.
 *
 * @param pin_number Specifies the pin number.
 */
__STATIC_INLINE void nrf_gpio_input_disconnect(uint32_t pin_number);

/**
 * @brief Function for configuring the given GPIO pin number as input, hiding inner details.
 *        This function can be used to configure pin range as simple input.
 *        Sense capability on the pin is configurable and input is connected to buffer so that the GPIO->IN register is readable.
 *
 * @param pin_number   Specifies the pin number.
 * @param pull_config  State of the pin pull resistor (no pull, pulled down, or pulled high).
 * @param sense_config Sense level of the pin (no sense, sense low, or sense high).
 */
//__STATIC_INLINE void nrf_gpio_cfg_sense_input(uint32_t             pin_number,
                                        //      nrf_gpio_pin_pull_t  pull_config,
                                         //     nrf_gpio_pin_sense_t sense_config);

/**
 * @brief Function for configuring sense level for the given GPIO.
 *
 * @param pin_number   Specifies the pin number.
 * @param sense_config Sense configuration.
 */
//__STATIC_INLINE void nrf_gpio_cfg_sense_set(uint32_t pin_number, nrf_gpio_pin_sense_t sense_config);

/**
 * @brief Function for setting the direction for a GPIO pin.
 *
 * @param pin_number Specifies the pin number for which to set the direction.
 * @param direction  Specifies the direction.
 */
__STATIC_INLINE void nrf_gpio_pin_dir_set(uint32_t pin_number, nrf_gpio_pin_dir_t direction);

/**
 * @brief Function for setting a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to be set.
 */
__STATIC_INLINE void nrf_gpio_pin_set(uint32_t pin_number);

/**
 * @brief Function for clearing a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to clear.
 */
__STATIC_INLINE void nrf_gpio_pin_clear(uint32_t pin_number);

/**
 * @brief Function for toggling a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to toggle.
 */
__STATIC_INLINE void nrf_gpio_pin_toggle(uint32_t pin_number);

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
__STATIC_INLINE void nrf_gpio_pin_write(uint32_t pin_number, uint32_t value);

/**
 * @brief Function for reading the input level of a GPIO pin.
 *
 * If the value returned by this function is to be valid, the pin's input buffer must be connected.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return 0 if the pin input level is low. Positive value if the pin is high.
 */
__STATIC_INLINE uint32_t nrf_gpio_pin_read(uint32_t pin_number);

/**
 * @brief Function for reading the output level of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return 0 if the pin output level is low. Positive value if pin output is high.
 */
__STATIC_INLINE uint32_t nrf_gpio_pin_out_read(uint32_t pin_number);

/**
 * @brief Function for reading the sense configuration of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return Sense configuration.
 */
//__STATIC_INLINE nrf_gpio_pin_sense_t nrf_gpio_pin_sense_get(uint32_t pin_number);

/**
 * @brief Function for reading the direction configuration of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return Direction configuration.
 */
__STATIC_INLINE nrf_gpio_pin_dir_t nrf_gpio_pin_dir_get(uint32_t pin_number);

/**
 * @brief Function for reading the status of GPIO pin input buffer.
 *
 * @param pin_number Pin number to be read.
 *
 * @retval Input buffer configuration.
 */
//__STATIC_INLINE nrf_gpio_pin_input_t nrf_gpio_pin_input_get(uint32_t pin_number);

/**
 * @brief Function for reading the pull configuration of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @retval Pull configuration.
 */
__STATIC_INLINE nrf_gpio_pin_pull_t nrf_gpio_pin_pull_get(uint32_t pin_number);

/**
 * @brief Function for setting output direction on the selected pins on the given port.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param out_mask Mask specifying the pins to set as output.
 */
__STATIC_INLINE void nrf_gpio_port_dir_output_set(NRF_GPIO_Type * p_reg, uint32_t out_mask);

/**
 * @brief Function for setting input direction on selected pins on a given port.
 *
 * @param p_reg   Pointer to the structure of registers of the peripheral.
 * @param in_mask Mask that specifies the pins to be set as input.
 */
__STATIC_INLINE void nrf_gpio_port_dir_input_set(NRF_GPIO_Type * p_reg, uint32_t in_mask);

/**
 * @brief Function for writing the direction configuration of the GPIO pins in the given port.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param dir_mask Mask that specifies the direction of pins. Bit set means that the given pin is configured as output.
 */
__STATIC_INLINE void nrf_gpio_port_dir_write(NRF_GPIO_Type * p_reg, uint32_t dir_mask);

/**
 * @brief Function for reading the direction configuration of a GPIO port.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Pin configuration of the current direction settings. Bit set means that the given pin is configured as output.
 */
__STATIC_INLINE uint32_t nrf_gpio_port_dir_read(NRF_GPIO_Type const * p_reg);

/**
 * @brief Function for reading the input signals of the GPIO pins on the given port.
 *
 * @param p_reg Pointer to the peripheral registers structure.
 *
 * @return Port input values.
 */
__STATIC_INLINE uint32_t nrf_gpio_port_in_read(NRF_GPIO_Type const * p_reg);

/**
 * @brief Function for reading the output signals of the GPIO pins on the given port.
 *
 * @param p_reg Pointer to the peripheral registers structure.
 *
 * @return Port output values.
 */
__STATIC_INLINE uint32_t nrf_gpio_port_out_read(NRF_GPIO_Type const * p_reg);

/**
 * @brief Function for writing the GPIO pins output on a given port.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param value Output port mask.
 */
__STATIC_INLINE void nrf_gpio_port_out_write(NRF_GPIO_Type * p_reg, uint32_t value);

/**
 * @brief Function for setting high level on selected the GPIO pins on the given port.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param set_mask Mask with pins to be set as logical high level.
 */
__STATIC_INLINE void nrf_gpio_port_out_set(NRF_GPIO_Type * p_reg, uint32_t set_mask);

/**
 * @brief Function for setting low level on selected the GPIO pins on the given port.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param clr_mask Mask with pins to be set as logical low level.
 */
__STATIC_INLINE void nrf_gpio_port_out_clear(NRF_GPIO_Type * p_reg, uint32_t clr_mask);

/**
 * @brief Function for reading pin state of multiple consecutive ports.
 *
 * @param start_port Index of the first port to read.
 * @param length     Number of ports to read.
 * @param p_masks    Pointer to output array where port states will be stored.
 */
__STATIC_INLINE void nrf_gpio_ports_read(uint32_t start_port, uint32_t length, uint32_t * p_masks);


/**
 * @brief Function for checking if provided pin is present on the MCU.
 *
 * @param[in] pin_number Number of the pin to be checked.
 *
 * @retval true  Pin is present.
 * @retval false Pin is not present.
 */
__STATIC_INLINE bool nrf_gpio_pin_present_check(uint32_t pin_number);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION

/**
 * @brief Function for extracting port and the relative pin number from the absolute pin number.
 *
 * @param[in,out] p_pin Pointer to the absolute pin number overriden by the pin number that is relative to the port.
 *
 * @return Pointer to port register set.
 */
__STATIC_INLINE NRF_GPIO_Type * nrf_gpio_pin_port_decode(uint32_t * p_pin)
{
//    NRFX_ASSERT(nrf_gpio_pin_present_check(*p_pin));
#if (GPIO_COUNT == 1)
    return NRF_P0;
#else
    if (*p_pin < P0_PIN_NUM)
    {
        return NRF_P0;
    }
    else
    {
        *p_pin = *p_pin & 0x1F;
        return NRF_P1;
    }
#endif
}

__STATIC_INLINE XINC_CPRA_AO_Type * nrf_gpio_pin_pull_decode(uint32_t * p_pin)
{
//    NRFX_ASSERT(nrf_gpio_pin_present_check(*p_pin));

      return NRF_CPR_AO;
}

__STATIC_INLINE void nrf_gpio_range_cfg_output(uint32_t pin_range_start, uint32_t pin_range_end)
{
    /*lint -e{845} // A zero has been given as right argument to operator '|'" */
    for (; pin_range_start <= pin_range_end; pin_range_start++)
    {
        nrf_gpio_cfg_output(pin_range_start);
    }
}


__STATIC_INLINE void nrf_gpio_range_cfg_input(uint32_t            pin_range_start,
                                              uint32_t            pin_range_end,
                                              nrf_gpio_pin_pull_t pull_config)
{
    /*lint -e{845} // A zero has been given as right argument to operator '|'" */
    for (; pin_range_start <= pin_range_end; pin_range_start++)
    {
        nrf_gpio_cfg_input(pin_range_start, pull_config);
    }
}


__STATIC_INLINE void nrf_gpio_cfg(
    uint32_t             pin_number,
    nrf_gpio_pin_dir_t   dir,
    nrf_gpio_pin_input_int_t inter,
    nrf_gpio_pin_pull_t  pull,
    nrf_gpio_pin_fun_sel_t fun,
    nrf_gpio_pin_debounce_t debounce)
{
    NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);
		XINC_CPRA_AO_Type * pull_reg = nrf_gpio_pin_pull_decode(&pin_number);
	
		uint32_t reg_value ;
		uint32_t tmp_num;
		tmp_num = pin_number;
		uint8_t reg_idx = 0;
	
		gpio_fun_sel(pin_number,fun);
		if(NRF_GPIO_PIN_DIR_INPUT == dir)
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
		
		
			if(NRF_GPIO_PIN_PULLUP == pull)
			{
					clrbit(reg_value,pin_number*2);
					setbit(reg_value,pin_number*2+1);
				
			}else if(NRF_GPIO_PIN_PULLDOWN == pull)
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
				reg_value = reg->DDR[1];
				reg_value |= (0x10000 << (tmp_num & 0x0F));
				reg->DDR[1] = reg_value;
				
				if(NRF_GPIO_PIN_DEBOUNCE_ENABLE == debounce )
				{
					reg_value = reg->DEBOUNCE[1];
					reg_value |= (0x10001 << (tmp_num & 0x0F));
					reg->DEBOUNCE[1] = reg_value;
				}

				
			}else
			{
				reg_value = reg->DDR[0];
				reg_value |= (0x10000 << (tmp_num & 0x0F));
				reg->DDR[0] = reg_value;
				if(NRF_GPIO_PIN_DEBOUNCE_ENABLE == debounce )
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
				reg_value = reg->DDR[1];
				reg_value |= (0x10001 << (tmp_num & 0x0F));
				reg->DDR[1] = reg_value;
				
			
				
			}else
			{
				reg_value = reg->DDR[0];
				reg_value |= (0x10001 << (tmp_num & 0x0F));
				reg->DDR[0] = reg_value;
					
			}
		}

//    reg->PIN_CNF[pin_number] = ((uint32_t)dir << GPIO_PIN_CNF_DIR_Pos)
//                               | ((uint32_t)input << GPIO_PIN_CNF_INPUT_Pos)
//                               | ((uint32_t)pull << GPIO_PIN_CNF_PULL_Pos)
//                               | ((uint32_t)drive << GPIO_PIN_CNF_DRIVE_Pos)
//                               | ((uint32_t)sense << GPIO_PIN_CNF_SENSE_Pos);
}


__STATIC_INLINE void nrf_gpio_cfg_output(uint32_t pin_number)
{
    nrf_gpio_cfg(
        pin_number,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_NOINT,
        NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_GPIODx,
        NRF_GPIO_PIN_DEBOUNCE_DISABLE);
}


__STATIC_INLINE void nrf_gpio_cfg_input(uint32_t pin_number, nrf_gpio_pin_pull_t pull_config)
{
    nrf_gpio_cfg(
        pin_number,
        NRF_GPIO_PIN_DIR_INPUT,
        NRF_GPIO_PIN_INPUT_RIS_EDGE_INT,
        pull_config,
        NRF_GPIO_PIN_GPIODx,
        NRF_GPIO_PIN_DEBOUNCE_ENABLE);
}


__STATIC_INLINE void nrf_gpio_cfg_default(uint32_t pin_number)
{
    nrf_gpio_cfg(
        pin_number,
        NRF_GPIO_PIN_DIR_INPUT,
        NRF_GPIO_PIN_INPUT_NOINT,
        NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_GPIODx,
        NRF_GPIO_PIN_DEBOUNCE_DISABLE);
}


__STATIC_INLINE void nrf_gpio_cfg_watcher(uint32_t pin_number)
{
    NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);
    /*lint -e{845} // A zero has been given as right argument to operator '|'" */
  //  uint32_t cnf = reg->PIN_CNF[pin_number] & ~GPIO_PIN_CNF_INPUT_Msk;

   // reg->PIN_CNF[pin_number] = cnf | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos);
}


__STATIC_INLINE void nrf_gpio_input_disconnect(uint32_t pin_number)
{
    NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);
    /*lint -e{845} // A zero has been given as right argument to operator '|'" */
 //   uint32_t cnf = reg->PIN_CNF[pin_number] & ~GPIO_PIN_CNF_INPUT_Msk;

  //  reg->PIN_CNF[pin_number] = cnf | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos);
}


__STATIC_INLINE void nrf_gpio_cfg_sense_input(uint32_t             pin_number,
                                              nrf_gpio_pin_pull_t  pull_config,
                                              nrf_gpio_pin_debounce_t debounce_config)
{
    nrf_gpio_cfg(
        pin_number,
        NRF_GPIO_PIN_DIR_INPUT,
        NRF_GPIO_PIN_INPUT_NOINT,
        pull_config,
        NRF_GPIO_PIN_GPIODx,
        debounce_config);
}


__STATIC_INLINE void nrf_gpio_cfg_sense_set(uint32_t pin_number, nrf_gpio_pin_debounce_t debounce_config)
{
    NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);

}


__STATIC_INLINE void nrf_gpio_pin_dir_set(uint32_t pin_number, nrf_gpio_pin_dir_t direction)
{
    if (direction == NRF_GPIO_PIN_DIR_INPUT)
    {
        nrf_gpio_cfg(
            pin_number,
            NRF_GPIO_PIN_DIR_INPUT,
            NRF_GPIO_PIN_INPUT_NOINT,
            NRF_GPIO_PIN_NOPULL,
            NRF_GPIO_PIN_GPIODx,
            NRF_GPIO_PIN_DEBOUNCE_ENABLE);
    }
    else
    {
         nrf_gpio_cfg(
            pin_number,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_NOINT,
            NRF_GPIO_PIN_NOPULL,
            NRF_GPIO_PIN_GPIODx,
            NRF_GPIO_PIN_DEBOUNCE_DISABLE);
    }
}


#include "bsp_gpio.h"
__STATIC_INLINE ret_code_t xinc_gpio_fun_config(uint32_t pin,nrf_gpio_pin_fun_sel_t fun)
{
		ret_code_t err_code = NRF_SUCCESS; 
		if(pin > XINC_GPIO_31)
		{
			err_code = NRF_ERROR_INVALID_PARAM;
			return err_code;
		}
		if((NRF_GPIO_PIN_PWM2 == fun) && (XINC_GPIO_0 == pin))
		{
			gpio_mux_ctl(pin,2);		
			return err_code;
				
		}
		if((NRF_GPIO_PIN_PWM3 == fun) && (XINC_GPIO_1 == pin))
		{
			gpio_mux_ctl(pin,2);		
			return err_code;
								
		}
		if((NRF_GPIO_PIN_PWM4 == fun) && (XINC_GPIO_12 == pin))
		{
			
			gpio_mux_ctl(pin,3);		
			return err_code;
				
		}
		if((NRF_GPIO_PIN_PWM5 == fun) && (XINC_GPIO_13 == pin))
		{
						
			gpio_mux_ctl(pin,13);		
			return err_code;
		}
		
		switch(pin)
		{
			case XINC_GPIO_11:
			case XINC_GPIO_12:
			case XINC_GPIO_13:	
			{
				gpio_mux_ctl(pin,1);
				if(fun > NRF_GPIO_PIN_PWM1_INV)
				{
					err_code = NRFX_ERROR_INVALID_PARAM;
					return err_code;
				}
				gpio_fun_sel(pin,fun);
				
			}break;
			
			default:
			{
				
				gpio_mux_ctl(pin,0);
				if(fun > NRF_GPIO_PIN_PWM1_INV)
				{
					err_code = NRFX_ERROR_INVALID_PARAM;
					return err_code;
				}
				gpio_fun_sel(pin,fun);
			}
			break;	
		
		}	
	return err_code;

}

__STATIC_INLINE void nrf_gpio_pin_set(uint32_t pin_number)
{
    NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);
	  uint8_t reg_idx = pin_number >>4;
	
		reg->DR[reg_idx] = 0x00010001 << (pin_number & 0xF);

//    nrf_gpio_port_out_set(reg, 1UL << pin_number);
}


__STATIC_INLINE void nrf_gpio_pin_clear(uint32_t pin_number)
{
    NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);

		uint8_t reg_idx = pin_number >>4;
	
		reg->DR[reg_idx] = 0x00010000 << (pin_number & 0xF);
	
  //  nrf_gpio_port_out_clear(reg, 1UL << pin_number);
	
}


__STATIC_INLINE void nrf_gpio_pin_toggle(uint32_t pin_number)
{
    NRF_GPIO_Type * reg        = nrf_gpio_pin_port_decode(&pin_number);
		uint8_t reg_dix = pin_number >>4;

		reg->DR[reg_dix] = 0x10001 << (pin_number & 0x0f);
	  reg->DR[reg_dix] = 0x10000 << (pin_number & 0x0f);
	

}


__STATIC_INLINE void nrf_gpio_pin_write(uint32_t pin_number, uint32_t value)
{
    if (value == 0)
    {
        nrf_gpio_pin_clear(pin_number);
    }
    else
    {
        nrf_gpio_pin_set(pin_number);
    }
}


__STATIC_INLINE uint32_t nrf_gpio_pin_read(uint32_t pin_number)
{
    NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);
	
	  uint32_t value = reg->EXT_PORT0;
	
		return (value & (0x01L << (pin_number & 0x1F)))? (1) : (0);

}


__STATIC_INLINE uint32_t nrf_gpio_pin_out_read(uint32_t pin_number)
{
    NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);
	
		uint32_t value = reg->EXT_PORT0;

		return (value & (0x01L << (pin_number & 0x1F)))? (1) : (0);

}



__STATIC_INLINE void nrf_gpio_ports_read(uint32_t start_port, uint32_t length, uint32_t * p_masks)
{
    NRF_GPIO_Type * gpio_regs[GPIO_COUNT] = GPIO_REG_LIST;

 //   NRFX_ASSERT(start_port + length <= GPIO_COUNT);
    uint32_t i;

    for (i = start_port; i < (start_port + length); i++)
    {
      //  *p_masks = nrf_gpio_port_in_read(gpio_regs[i]);
				*p_masks = gpio_regs[0]->EXT_PORT0;
        p_masks++;
    }
}


__STATIC_INLINE bool nrf_gpio_pin_present_check(uint32_t pin_number)
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

/** @} */

#ifdef __cplusplus
}
#endif

#endif // NRF_GPIO_H__
