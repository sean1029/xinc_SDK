/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_KBS_H__
#define XINC_KBS_H__

#include <xincx.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_kbs_hal KBS HAL
 * @{
 * @ingroup xinc_kbs
 * @brief   Hardware access layer for managing the KBS peripheral.
 */


/** @brief KBS interrupts. */
typedef enum
{
    XINC_I2C_INT_DIS_MASK_ALL = 0,
    XINC_KBS_INT_EN_PRS_INT             = KBS_MTXKEY_INT_PRS_INT_EN_Msk,    /**< PRS interrupt. */
    XINC_KBS_INT_EN_FIFO_UPDATE_INT     = KBS_MTXKEY_INT_FIFO_UPDATE_INT_EN_Msk,   /**< FIFO_UPDATE interrupt. */
    XINC_I2C_INT_EN_RPRS_INT            = KBS_MTXKEY_INT_RPRS_INT_EN_Msk, /**< RPRS interrupt. */
    XINC_I2C_INT_EN_LPRS_INT            = KBS_MTXKEY_INT_LPRS_INT_EN_Msk, /**< LPRS interrupt. */
    XINC_I2C_INT_EN_RLS_INT             = KBS_MTXKEY_INT_RLS_INT_EN_Msk,  /**< RLS interrupt. */
    XINC_I2C_INT_EN_MASK_ALL            = KBS_MTXKEY_INT_PRS_INT_EN_Msk  | KBS_MTXKEY_INT_FIFO_UPDATE_INT_EN_Msk |
                                          KBS_MTXKEY_INT_RPRS_INT_EN_Msk | KBS_MTXKEY_INT_LPRS_INT_EN_Msk |
                                          KBS_MTXKEY_INT_RLS_INT_EN_Msk,
} xinc_kbs_int_en_mask_t;


/** @brief KBS interrupts raw status. */
typedef enum
{
    XINC_I2C_INT_RAW_STATUS_PRS         = KBS_MTXKEY_INT_PRS_INT_RAW_Msk,    /**< PRS interrupt. */
    XINC_I2C_INT_RAW_STATUS_FIFO_UPDATE = KBS_MTXKEY_INT_FIFO_UPDATE_INT_RAW_Msk,   /**< FIFO_UPDATE interrupt. */
    XINC_I2C_INT_RAW_STATUS_RPRS        = KBS_MTXKEY_INT_RPRS_INT_RAW_Msk, /**< RPRS interrupt. */
    XINC_I2C_INT_RAW_STATUS_LPRS        = KBS_MTXKEY_INT_LPRS_INT_RAW_Msk, /**< LPRS interrupt. */
    XINC_I2C_INT_RAW_STATUS_RLS         = KBS_MTXKEY_INT_RLS_INT_RAW_Msk,  /**< RLS interrupt. */
} xinc_kbs_int_raw_status_t;

/** @brief KBS interrupts status. */
typedef enum
{
    XINC_I2C_INT_STATUS_PRS         = KBS_MTXKEY_INT_PRS_INT_Msk,    /**< PRS interrupt. */
    XINC_I2C_INT_STATUS_FIFO_UPDATE = KBS_MTXKEY_INT_FIFO_UPDATE_INT_Msk,   /**< FIFO_UPDATE interrupt. */
    XINC_I2C_INT_STATUS_RPRS        = KBS_MTXKEY_INT_RPRS_INT_Msk, /**< RPRS interrupt. */
    XINC_I2C_INT_STATUS_LPRS        = KBS_MTXKEY_INT_LPRS_INT_Msk, /**< LPRS interrupt. */
    XINC_I2C_INT_STATUS_RLS         = KBS_MTXKEY_INT_RLS_INT_Msk,  /**< RLS interrupt. */
} xinc_kbs_int_status_t;

/** @brief KBS FIFO status. */
typedef enum
{
    XINC_I2C_FIFO_STATUS_AL_EMPTY    = KBS_MTXKEY_FIFO_AL_EMPTY_Msk,    /**< FIFO 近空. */
    XINC_I2C_FIFO_STATUS_EMPTY       = KBS_MTXKEY_FIFO_EMPTY_Msk,   /**< FIFO 空. */
    XINC_I2C_FIFO_STATUS_AL_FULL     = KBS_MTXKEY_FIFO_AL_FULL_Msk,  /**< FIFO 近满. */
    XINC_I2C_FIFO_STATUS_FULL        = KBS_MTXKEY_FIFO_FULL_Msk,  /**< FIFO 满. */
    XINC_I2C_FIFO_STATUS_POP_ERROR   = KBS_MTXKEY_FIFO_POP_ERROR_Msk, /**< FIFO POP 出错状态 */
    XINC_I2C_FIFO_STATUS_PUSH_ERROR  = KBS_MTXKEY_FIFO_PUSH_ERROR_Msk,  /**< FIFO PUSH 出错状态 */
} xinc_kbs_fifo_status_t;


/**
 * @brief Function for enabling the specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be enabled.
 */
__STATIC_INLINE void xinc_kbs_int_enable(XINC_KBS_Type * p_reg,
                                        uint32_t       mask);

/**
 * @brief Function for disabling the specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be disabled.
 */
__STATIC_INLINE void xinc_kbs_int_disable(XINC_KBS_Type * p_reg,
                                         uint32_t       mask);



/**
 * @brief Function for enabling the specified kbs mask.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be enabled.
 */
__STATIC_INLINE void xinc_kbs_mask_set(XINC_KBS_Type * p_reg,
                                        uint32_t       mask);

/**
 * @brief Function for enabling the KBS peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_kbs_enable(XINC_KBS_Type * p_reg);

/**
 * @brief Function for disabling the KBS peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_kbs_disable(XINC_KBS_Type * p_reg);

/**
 * @brief Function for reading data received by KBS.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Received data.
 */
__STATIC_INLINE uint32_t xinc_kbs_fifo_get(XINC_KBS_Type * p_reg);


__STATIC_INLINE XINC_KBS_Type * xinc_kbs_decode(void)
{
    return XINC_KBS;

}


__STATIC_INLINE void xinc_kbs_int_enable(XINC_KBS_Type * p_reg,
                                        uint32_t       mask)
{
    p_reg->MTXKEY_INT_EN |= mask;
}


__STATIC_INLINE void xinc_kbs_int_disable(XINC_KBS_Type * p_reg,
                                         uint32_t       mask)
{
    p_reg->MTXKEY_INT_EN &= ~mask;
}

__STATIC_INLINE void xinc_kbs_mask_set(XINC_KBS_Type * p_reg,
                                        uint32_t       mask)
{
    p_reg->MASK = mask;
}

__STATIC_INLINE void xinc_kbs_interval_set(XINC_KBS_Type * p_reg,
                                        uint32_t       mask)
{
    p_reg->MASK = mask;
}

__STATIC_INLINE void xinc_kbs_enable(XINC_KBS_Type * p_reg)
{
    p_reg->CTL |= KBS_KBS_CTL_MTXKEY_EN_Msk;
}

__STATIC_INLINE void xinc_kbs_disable(XINC_KBS_Type * p_reg)
{
    p_reg->CTL &= ~KBS_KBS_CTL_MTXKEY_EN_Msk;
}

__STATIC_INLINE uint32_t xinc_kbs_fifo_get(XINC_KBS_Type * p_reg)
{
    return p_reg->MTXKEY_FIFO;
}

#ifdef __cplusplus
}
#endif


#endif //XINC_KBS_H__

