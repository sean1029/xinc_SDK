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

#ifndef XINC_I2C_H__
#define XINC_I2C_H__

#include <nrfx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrf_i2c_hal I2C HAL
 * @{
 * @ingroup nrf_i2c
 * @brief   Hardware access layer for managing the I2C peripheral.
 */

/** @brief I2C tasks. */
typedef enum
{
    XINC_I2C_TASK_STARTRX = 0,//offsetof(XINC_I2C_Type, TASKS_STARTRX), ///< Start I2C receive sequence.
    XINC_I2C_TASK_STARTTX = 1,//offsetof(XINC_I2C_Type, TASKS_STARTTX), ///< Start I2C transmit sequence.
    XINC_I2C_TASK_STOP    = 2,//offsetof(XINC_I2C_Type, TASKS_STOP),    ///< Stop I2C transaction.
    XINC_I2C_TASK_SUSPEND = 3,//offsetof(XINC_I2C_Type, TASKS_SUSPEND), ///< Suspend I2C transaction.
    XINC_I2C_TASK_RESUME  = 4,//offsetof(XINC_I2C_Type, TASKS_RESUME)   ///< Resume I2C transaction.
} nrf_i2c_task_t;

/** @brief i2c events. */
typedef enum
{
    XINC_I2C_EVENT_STOPPED   = 0,//offsetof(XINC_I2C_Type, EVENTS_STOPPED),  ///< I2C stopped.
    XINC_I2C_EVENT_RXDREADY  = 1,//offsetof(XINC_I2C_Type, EVENTS_RXDREADY), ///< I2C RXD byte received.
    XINC_I2C_EVENT_TXDSENT   = 2,//offsetof(XINC_I2C_Type, EVENTS_TXDSENT),  ///< I2C TXD byte sent.
    XINC_I2C_EVENT_ERROR     = 3,//offsetof(XINC_I2C_Type, EVENTS_ERROR),    ///< I2C error.
    XINC_I2C_EVENT_BB        = 4,//offsetof(XINC_I2C_Type, EVENTS_BB),       ///< I2C byte boundary, generated before each byte that is sent or received.
    XINC_I2C_EVENT_SUSPENDED = 5,//offsetof(XINC_I2C_Type, EVENTS_SUSPENDED) ///< I2C entered the suspended state.
} nrf_i2c_event_t;




/** @brief I2C master clock frequency. */
typedef enum
{
    XINC_I2C_FREQ_100K = I2C_FREQUENCY_FREQUENCY_K100, ///< 100 kbps.
    XINC_I2C_FREQ_400K = I2C_FREQUENCY_FREQUENCY_K400  ///< 400 kbps.
} xinc_i2c_frequency_t;


/**
 * @brief Function for activating the specified I2C task.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] task  Task to be activated.
 */
__STATIC_INLINE void xinc_i2c_task_trigger(XINC_I2C_Type * p_reg,
                                          nrf_i2c_task_t task);



/**
 * @brief Function for clearing the specified I2C event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event to clear.
 */
__STATIC_INLINE void xinc_i2c_event_clear(XINC_I2C_Type *  p_reg,
                                         nrf_i2c_event_t event);

/**
 * @brief Function for retrieving the state of the I2C event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event to be checked.
 *
 * @retval true  The event has been generated.
 * @retval false The event has not been generated.
 */
__STATIC_INLINE bool xinc_i2c_event_check(XINC_I2C_Type  * p_reg,
                                         nrf_i2c_event_t event);


/**
 * @brief Function for enabling the specified shortcuts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Shortcuts to be enabled.
 */
__STATIC_INLINE void xinc_i2c_shorts_enable(XINC_I2C_Type * p_reg,
                                           uint32_t       mask);

/**
 * @brief Function for disabling the specified shortcuts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Shortcuts to be disabled.
 */
__STATIC_INLINE void xinc_i2c_shorts_disable(XINC_I2C_Type * p_reg,
                                            uint32_t       mask);

/**
 * @brief Function for enabling the specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be enabled.
 */
__STATIC_INLINE void xinc_i2c_int_enable(XINC_I2C_Type * p_reg,
                                        uint32_t       mask);

/**
 * @brief Function for disabling the specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be disabled.
 */
__STATIC_INLINE void xinc_i2c_int_disable(XINC_I2C_Type * p_reg,
                                         uint32_t       mask);


/**
 * @brief Function for enabling the I2C peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_i2c_enable(XINC_I2C_Type * p_reg);

/**
 * @brief Function for disabling the I2C peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_i2c_disable(XINC_I2C_Type * p_reg);

/**
 * @brief Function for configuring I2C pins.
 *
 * @param[in] p_reg   Pointer to the structure of registers of the peripheral.
 * @param[in] scl_pin SCL pin number.
 * @param[in] sda_pin SDA pin number.
 */
__STATIC_INLINE void xinc_i2c_pins_set(XINC_I2C_Type * p_reg,
                                      uint32_t       scl_pin,
                                      uint32_t       sda_pin);

/**
 * @brief Function for retrieving the SCL pin number.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return SCL pin number.
 */
__STATIC_INLINE uint32_t xinc_i2c_scl_pin_get(XINC_I2C_Type * p_reg);

/**
 * @brief Function for retrieving the SDA pin number.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return SDA pin number.
 */
__STATIC_INLINE uint32_t xinc_i2c_sda_pin_get(XINC_I2C_Type * p_reg);

/**
 * @brief Function for setting the I2C master clock frequency.
 *
 * @param[in] p_reg     Pointer to the structure of registers of the peripheral.
 * @param[in] frequency I2C frequency.
 */
__STATIC_INLINE void xinc_i2c_frequency_set(XINC_I2C_Type *      p_reg,
                                           xinc_i2c_frequency_t frequency);

/**
 * @brief Function for checking the I2C error source.
 *
 * The error flags are cleared after reading.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Mask with error source flags.
 */
__STATIC_INLINE uint32_t xinc_i2c_errorsrc_get_and_clear(XINC_I2C_Type * p_reg);

/**
 * @brief Function for setting the address to be used in I2C transfers.
 *
 * @param[in] p_reg   Pointer to the structure of registers of the peripheral.
 * @param[in] address Address to be used in transfers.
 */
__STATIC_INLINE void xinc_i2c_address_set(XINC_I2C_Type * p_reg, uint8_t address);

/**
 * @brief Function for reading data received by I2C.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Received data.
 */
__STATIC_INLINE uint16_t xinc_i2c_rxd_get(XINC_I2C_Type * p_reg);

/**
 * @brief Function for writing data to be transmitted by I2C.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] data  Data to be transmitted.
 */
__STATIC_INLINE void xinc_i2c_txd_set(XINC_I2C_Type * p_reg, uint16_t data);

/**
 * @brief Function for setting the specified shortcuts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Shortcuts to be set.
 */
__STATIC_INLINE void xinc_i2c_shorts_set(XINC_I2C_Type * p_reg,
                                        uint32_t       mask);

#ifndef SUPPRESS_INLINE_IMPLEMENTATION

__STATIC_INLINE void xinc_i2c_task_trigger(XINC_I2C_Type * p_reg,
                                          nrf_i2c_task_t task)
{
    
}



__STATIC_INLINE void xinc_i2c_event_clear(XINC_I2C_Type  * p_reg,
                                         nrf_i2c_event_t event)
{

}

__STATIC_INLINE bool xinc_i2c_event_check(XINC_I2C_Type  * p_reg,
                                         nrf_i2c_event_t event)
{
    return 0;
}





__STATIC_INLINE void xinc_i2c_int_enable(XINC_I2C_Type * p_reg,
                                        uint32_t       mask)
{
    
}

__STATIC_INLINE void xinc_i2c_int_disable(XINC_I2C_Type * p_reg,
                                         uint32_t       mask)
{
    
}

__STATIC_INLINE void xinc_i2c_enable(XINC_I2C_Type * p_reg)
{
    
}

__STATIC_INLINE void xinc_i2c_disable(XINC_I2C_Type * p_reg)
{
   
}

__STATIC_INLINE void xinc_i2c_pins_set(XINC_I2C_Type * p_reg,
                                      uint32_t       scl_pin,
                                      uint32_t       sda_pin)
{

}



__STATIC_INLINE void xinc_i2c_frequency_set(XINC_I2C_Type *      p_reg,
                                           xinc_i2c_frequency_t frequency)
{
  
}

__STATIC_INLINE uint32_t xinc_i2c_errorsrc_get_and_clear(XINC_I2C_Type * p_reg)
{
    uint32_t error_source = 0;

    // [error flags are cleared by writing '1' on their position]
  //  p_reg->ERRORSRC = error_source;

    return error_source;
}

__STATIC_INLINE void xinc_i2c_address_set(XINC_I2C_Type * p_reg, uint8_t address)
{
   
}

__STATIC_INLINE uint16_t xinc_i2c_rxd_get(XINC_I2C_Type * p_reg)
{
		uint16_t data = p_reg->i2c_DATA_CMD;
	//	printf("txd_get:0x%04x\r\n",data);
    return data;
}

__STATIC_INLINE void xinc_i2c_txd_set(XINC_I2C_Type * p_reg, uint16_t data)
{
	//	printf("txd_set:0x%04x\r\n",data);
    p_reg->i2c_DATA_CMD = data;
}

__STATIC_INLINE void xinc_i2c_shorts_set(XINC_I2C_Type * p_reg,
                                        uint32_t       mask)
{
   
}

#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // NRF_I2C_H__
