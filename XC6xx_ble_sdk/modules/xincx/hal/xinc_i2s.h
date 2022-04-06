/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_I2S_H__
#define XINC_I2S_H__

#include <xincx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_i2s_hal I2S HAL
 * @{
 * @ingroup xinc_i2s
 * @brief   Hardware access layer for managing the I2S peripheral.
 */



/** @brief I2S master clock frequency. */
typedef enum
{
    XINC_I2S_FREQ_100K = 0, ///< 100 kbps.
    XINC_I2S_FREQ_400K = 1  ///< 400 kbps.
} xinc_i2s_frequency_t;

/** @brief I2C interrupts. */
typedef enum
{
    XINC_I2S_INT_EN_MASK_RX_UNDER    = 0,    /**< RX_UNDER interrupt. */
    XINC_I2S_INT_EN_MASK_RX_OVER   = 1,   /**< RX_OVER interrupt. */
    XINC_I2S_INT_EN_MASK_RX_FULL = 2, /**< _RX_FULL interrupt. */
    XINC_I2S_INT_EN_MASK_TX_OVER = 3, /**< TX_OVER interrupt. */
    XINC_I2S_INT_EN_MASK_TX_EMPTY  = 4,  /**< TX_EMPTY interrupt. */
    XINC_I2S_INT_EN_MASK_RD_REQ   = 5,    /**< RD_REQ interrupt. */
    XINC_I2S_INT_EN_MASK_TX_ABRT    = 6,    /**< TX_ABRT interrupt. */

    XINC_I2S_INT_EN_MASK_RX_DONE   = 7,   /**< RX_DONE interrupt. */
    XINC_I2S_INT_EN_MASK_ACTIVITY = 8, /**< ACTIVITY interrupt. */
    XINC_I2S_INT_EN_MASK_STOP_DET = 9, /**< STOP_DET interrupt. */
    XINC_I2S_INT_EN_MASK_START_DET  = 10,  /**< STOP_DET interrupt. */
    XINC_I2S_INT_EN_MASK_GEN_CALL   = 11,    /**< GEN_CALL interrupt. */
    XINC_I2S_INT_EN_MASK_ALL = 0xFFF,
    XINC_I2S_INT_DIS_MASK_ALL = 0,
} xinc_i2s_int_en_mask_t;

/** @brief I2C interrupts. */
typedef enum
{
    XINC_I2S_INT_RAW_MASK_RX_UNDER    = 0,    /**< RX_UNDER interrupt. */
    XINC_I2S_INT_RAW_MASK_RX_OVER   = 0,   /**< RX_OVER interrupt. */
    XINC_I2S_INT_RAW_MASK_RX_FULL = 0, /**< _RX_FULL interrupt. */
    XINC_I2S_INT_RAW_MASK_TX_OVER = 0, /**< TX_OVER interrupt. */
    XINC_I2S_INT_RAW_MASK_TX_EMPTY  = 0,  /**< TX_EMPTY interrupt. */
    XINC_I2S_INT_RAW_MASK_RD_REQ   = 0,    /**< RD_REQ interrupt. */
    XINC_I2S_INT_RAW_MASK_TX_ABRT    = 0,    /**< TX_ABRT interrupt. */

    XINC_I2S_INT_RAW_MASK_RX_DONE   = 0,   /**< RX_DONE interrupt. */
    XINC_I2S_INT_RAW_MASK_ACTIVITY = 0, /**< ACTIVITY interrupt. */
    XINC_I2S_INT_RAW_MASK_STOP_DET = 0, /**< STOP_DET interrupt. */
    XINC_I2S_INT_RAW_MASK_START_DET  = 0,  /**< STOP_DET interrupt. */
    XINC_I2S_INT_RAW_MASK_GEN_CALL   = 0    /**< GEN_CALL interrupt. */
} xinc_i2s_int_raw_status_mask_t;


/** @brief I2S interrupts. */
typedef enum
{
    XINC_I2S_STATUS_MASK_ACTIVITY    = I2C_STATUS_ACTIVITY_Msk,    /**< ACTIVITY Status. */
    XINC_I2S_STATUS_MASK_TFNF   = I2C_STATUS_TFNF_Msk,   /**< TX FIFO NOT full. */
    XINC_I2S_STATUS_MASK_TFE = I2C_STATUS_TFE_Msk,  /**< _t. */
    XINC_I2S_STATUS_MASK_RFNE = I2C_STATUS_RFNE_Msk, /**< . */
    XINC_I2S_STATUS_MASK_RFF  = I2C_STATUS_RFF_Msk,  /**< . */



} xinc_i2s_status_mask_t;

/**
 * @brief Function for enabling the specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be enabled.
 */
__STATIC_INLINE void xinc_i2s_int_enable(XINC_I2S_Type * p_reg,
                                        uint32_t       mask);

/**
 * @brief Function for disabling the specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be disabled.
 */
__STATIC_INLINE void xinc_i2s_int_disable(XINC_I2S_Type * p_reg,
                                         uint32_t       mask);


/**
 * @brief Function for enabling the I2C peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_i2s_enable(XINC_I2S_Type * p_reg);

/**
 * @brief Function for disabling the I2C peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_i2s_disable(XINC_I2S_Type * p_reg);




/**
 * @brief Function for reading data received by I2S.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Received data.
 */
__STATIC_INLINE uint16_t xinc_i2s_rxd_get(XINC_I2S_Type * p_reg);

/**
 * @brief Function for writing data to be transmitted by I2C.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] data  Data to be transmitted.
 */
__STATIC_INLINE void xinc_i2s_txd_set(XINC_I2S_Type * p_reg, uint16_t data);



#ifndef SUPPRESS_INLINE_IMPLEMENTATION

__STATIC_INLINE void xinc_i2s_int_clean(XINC_I2C_Type * p_reg,
                                        uint32_t       mask)
{

}


__STATIC_INLINE void xinc_i2s_int_enable(XINC_I2S_Type * p_reg,
                                        uint32_t       mask)
{
     
}

__STATIC_INLINE void xinc_i2s_int_disable(XINC_I2S_Type * p_reg,
                                         uint32_t       mask)
{

}

__STATIC_INLINE void xinc_i2s_enable(XINC_I2S_Type * p_reg)
{
 //   printf("xinc_i2c_enable:0x%p\r\n",&(p_reg->i2c_ENABLE));

}

__STATIC_INLINE void xinc_i2s_disable(XINC_I2S_Type * p_reg)
{

}



__STATIC_INLINE uint32_t xinc_i2s_errorsrc_get_and_clear(XINC_I2S_Type * p_reg)
{
    uint32_t error_source = 0;

    // [error flags are cleared by writing '1' on their position]
  //  p_reg->ERRORSRC = error_source;

    return error_source;
}



__STATIC_INLINE uint16_t xinc_i2s_rxd_get(XINC_I2S_Type * p_reg)
{
	uint16_t data = 0;
	//	printf("txd_get:0x%04x\r\n",data);
    return data;
}

__STATIC_INLINE void xinc_i2s_txd_set(XINC_I2S_Type * p_reg, uint16_t data)
{
	//printf("txd_set:0x%04x\r\n",data);
    
}


#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_I2S_H__
