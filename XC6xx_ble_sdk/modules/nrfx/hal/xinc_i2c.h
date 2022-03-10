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



/** @brief I2C master clock frequency. */
typedef enum
{
    XINC_I2C_FREQ_100K = I2C_FREQUENCY_FREQUENCY_K100, ///< 100 kbps.
    XINC_I2C_FREQ_400K = I2C_FREQUENCY_FREQUENCY_K400  ///< 400 kbps.
} xinc_i2c_frequency_t;

/** @brief I2C interrupts. */
typedef enum
{
    XINC_I2C_INT_EN_MASK_RX_UNDER    = I2C_INTR_EN_RX_UNDER_Msk,    /**< RX_UNDER interrupt. */
    XINC_I2C_INT_EN_MASK_RX_OVER   = I2C_INTR_EN_RX_OVER_Msk,   /**< RX_OVER interrupt. */
    XINC_I2C_INT_EN_MASK_RX_FULL = I2C_INTR_EN_RX_FULL_Msk, /**< _RX_FULL interrupt. */
    XINC_I2C_INT_EN_MASK_TX_OVER = I2C_INTR_EN_TX_OVER_Msk, /**< TX_OVER interrupt. */
    XINC_I2C_INT_EN_MASK_TX_EMPTY  = I2C_INTR_EN_TX_EMPTY_Msk,  /**< TX_EMPTY interrupt. */
    XINC_I2C_INT_EN_MASK_RD_REQ   = I2C_INTR_EN_RD_REQ_Msk,    /**< RD_REQ interrupt. */
    XINC_I2C_INT_EN_MASK_TX_ABRT    = I2C_INTR_EN_TX_ABRT_Msk,    /**< TX_ABRT interrupt. */

    XINC_I2C_INT_EN_MASK_RX_DONE   = I2C_INTR_EN_RX_DONE_Msk,   /**< RX_DONE interrupt. */
    XINC_I2C_INT_EN_MASK_ACTIVITY = I2C_INTR_EN_ACTIVITY_Msk, /**< ACTIVITY interrupt. */
    XINC_I2C_INT_EN_MASK_STOP_DET = I2C_INTR_EN_STOP_DET_Msk, /**< STOP_DET interrupt. */
    XINC_I2C_INT_EN_MASK_START_DET  = I2C_INTR_EN_STOP_DET_Msk,  /**< STOP_DET interrupt. */
    XINC_I2C_INT_EN_MASK_GEN_CALL   = I2C_INTR_EN_GEN_CALL_Msk,    /**< GEN_CALL interrupt. */
    XINC_I2C_INT_EN_MASK_ALL = 0xFFF,
    XINC_I2C_INT_DIS_MASK_ALL = 0,
} xinc_i2c_int_en_mask_t;

/** @brief I2C interrupts. */
typedef enum
{
    XINC_I2C_INT_RAW_MASK_RX_UNDER    = I2C_RAW_INTR_STAT_RX_UNDER_Msk,    /**< RX_UNDER interrupt. */
    XINC_I2C_INT_RAW_MASK_RX_OVER   = I2C_RAW_INTR_STAT_RX_OVER_Msk,   /**< RX_OVER interrupt. */
    XINC_I2C_INT_RAW_MASK_RX_FULL = I2C_RAW_INTR_STAT_RX_FULL_Msk, /**< _RX_FULL interrupt. */
    XINC_I2C_INT_RAW_MASK_TX_OVER = I2C_RAW_INTR_STAT_TX_OVER_Msk, /**< TX_OVER interrupt. */
    XINC_I2C_INT_RAW_MASK_TX_EMPTY  = I2C_RAW_INTR_STAT_TX_EMPTY_Msk,  /**< TX_EMPTY interrupt. */
    XINC_I2C_INT_RAW_MASK_RD_REQ   = I2C_RAW_INTR_STAT_RD_REQ_Msk,    /**< RD_REQ interrupt. */
    XINC_I2C_INT_RAW_MASK_TX_ABRT    = I2C_RAW_INTR_STAT_TX_ABRT_Msk,    /**< TX_ABRT interrupt. */

    XINC_I2C_INT_RAW_MASK_RX_DONE   = I2C_RAW_INTR_STAT_RX_DONE_Msk,   /**< RX_DONE interrupt. */
    XINC_I2C_INT_RAW_MASK_ACTIVITY = I2C_RAW_INTR_STAT_ACTIVITY_Msk, /**< ACTIVITY interrupt. */
    XINC_I2C_INT_RAW_MASK_STOP_DET = I2C_RAW_INTR_STAT_STOP_DET_Msk, /**< STOP_DET interrupt. */
    XINC_I2C_INT_RAW_MASK_START_DET  = I2C_RAW_INTR_STAT_STOP_DET_Msk,  /**< STOP_DET interrupt. */
    XINC_I2C_INT_RAW_MASK_GEN_CALL   = I2C_RAW_INTR_STAT_GEN_CALL_Msk    /**< GEN_CALL interrupt. */
} xinc_i2c_int_raw_status_mask_t;


/** @brief I2C interrupts. */
typedef enum
{
    XINC_I2C_STATUS_MASK_ACTIVITY    = I2C_STATUS_ACTIVITY_Msk,    /**< ACTIVITY Status. */
    XINC_I2C_STATUS_MASK_TFNF   = I2C_STATUS_TFNF_Msk,   /**< TX FIFO NOT full. */
    XINC_I2C_STATUS_MASK_TFE = I2C_STATUS_TFE_Msk,  /**< _t. */
    XINC_I2C_STATUS_MASK_RFNE = I2C_STATUS_RFNE_Msk, /**< . */
    XINC_I2C_STATUS_MASK_RFF  = I2C_STATUS_RFF_Msk,  /**< . */



} xinc_i2c_status_mask_t;

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



#ifndef SUPPRESS_INLINE_IMPLEMENTATION

__STATIC_INLINE void xinc_i2c_int_clean(XINC_I2C_Type * p_reg,
                                        uint32_t       mask)
{
    uint32_t reg = p_reg->i2c_CLR_INTR;
}


__STATIC_INLINE void xinc_i2c_int_enable(XINC_I2C_Type * p_reg,
                                        uint32_t       mask)
{
     p_reg->i2c_INTR_EN = mask;
}

__STATIC_INLINE void xinc_i2c_int_disable(XINC_I2C_Type * p_reg,
                                         uint32_t       mask)
{
    p_reg->i2c_INTR_EN = mask;
}

__STATIC_INLINE void xinc_i2c_enable(XINC_I2C_Type * p_reg)
{
 //   printf("xinc_i2c_enable:0x%p\r\n",&(p_reg->i2c_ENABLE));
    p_reg->i2c_ENABLE = I2C_ENABLE_EN_Enable & I2C_ENABLE_EN_Msk;
}

__STATIC_INLINE void xinc_i2c_disable(XINC_I2C_Type * p_reg)
{
   p_reg->i2c_ENABLE = I2C_ENABLE_EN_Disable & I2C_ENABLE_EN_Msk;;
}



__STATIC_INLINE uint32_t xinc_i2c_errorsrc_get_and_clear(XINC_I2C_Type * p_reg)
{
    uint32_t error_source = 0;

    // [error flags are cleared by writing '1' on their position]
  //  p_reg->ERRORSRC = error_source;

    return error_source;
}



__STATIC_INLINE uint16_t xinc_i2c_rxd_get(XINC_I2C_Type * p_reg)
{
	uint16_t data = p_reg->i2c_DATA_CMD;
	//	printf("txd_get:0x%04x\r\n",data);
    return data;
}

__STATIC_INLINE void xinc_i2c_txd_set(XINC_I2C_Type * p_reg, uint16_t data)
{
	//printf("txd_set:0x%04x\r\n",data);
    p_reg->i2c_DATA_CMD = data;
}


#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // NRF_I2C_H__
