/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_I2SMM_H__
#define XINC_I2SMM_H__

#include <xincx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_i2smm_hal I2SM HAL
 * @{
 * @ingroup xinc_i2smm
 * @brief   Hardware access layer for managing the Inter-IC Sound (I2SM) peripheral.
 */

/**
 * @brief This value can be provided as a parameter for the @ref xinc_i2sm_pins_set
 *        function call to specify that the given I2SM signal (SDOUT, SDIN, or MCK)
 *        shall not be connected to a physical pin.
 */
#define XINC_I2SM_PIN_NOT_CONNECTED  0xFFFFFFFF


/** @brief I2SM tasks. */
typedef enum
{
    XINC_I2SM_TASK_START = 0, ///< Starts continuous I2SM transfer. Also starts the MCK generator if this is enabled.
    XINC_I2SM_TASK_STOP  =  1  ///< Stops I2SM transfer. Also stops the MCK generator.
} xinc_i2sm_task_t;

/** @brief I2SM events. */
typedef enum
{
    XINC_I2SM_EVENT_NONE = 0,
    XINC_I2SM_EVENT_RXPTRUPD = 1, ///< The RXD.PTR register has been copied to internal double buffers.
    XINC_I2SM_EVENT_TXPTRUPD = 2, ///< The TXD.PTR register has been copied to internal double buffers.
    XINC_I2SM_EVENT_STOPPED  = 4  ///< I2SM transfer stopped.
} xinc_i2sm_event_t;

/** @brief I2SM interrupts. */
typedef enum
{
    XINC_I2SM_INT_RXPTRUPD_MASK = 0, ///< Interrupt on RXPTRUPD event.
    XINC_I2SM_INT_TXPTRUPD_MASK = 1, ///< Interrupt on TXPTRUPD event.
    XINC_I2SM_INT_STOPPED_MASK  = 2   ///< Interrupt on STOPPED event.
} xinc_i2sm_int_mask_t;

/** @brief I2SM modes of operation. */
typedef enum
{
    XINC_I2SM_MODE_MASTER = 0, ///< Master mode.
    XINC_I2SM_MODE_SLAVE  = 1   ///< Slave mode.
} xinc_i2sm_mode_t;

/** @brief I2SM master clock generator settings. */
typedef enum
{
    XINC_I2SM_MCK_SAME_PCLK  = 0,  ///< MCK same pclk.

    XINC_I2SM_MCK_16MDIV2   = 1,//(int)I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV2, ///< 16 MHz / 2 = 8.0 MHz.


    XINC_I2SM_MCK_32MDIV3   = 2,      ///< 16 MHz / 3 = 5.33333 MHz.
   XINC_I2SM_MCK_32MDIV4   = 3,      ///< 16 MHz / 4 = 8.0 MHz.


    XINC_I2SM_MCK_32MDIV5   = 4,      ///< 16 MHz / 5 = 6.4 MHz.


    XINC_I2SM_MCK_32MDIV6   = 5,      ///< 16 MHz / 6 = 5.3333333 MHz.

    XINC_I2SM_MCK_32MDIV8   = 7,      ///< 16 MHz / 8 = 2.0 MHz.
    XINC_I2SM_MCK_32MDIV10  = 9,     ///< 16 MHz / 10 = 1.6 MHz.
    XINC_I2SM_MCK_32MDIV11  = 10,     ///< 16 MHz / 11 = 1.454545 MHz.
    XINC_I2SM_MCK_32MDIV15  = 14,     ///< 16 MHz / 15 = 1.066667 MHz.
    XINC_I2SM_MCK_32MDIV16  = 15,     ///< 16 MHz / 16 = 1.0 MHz.
    XINC_I2SM_MCK_32MDIV21  = 20,     ///< 16 MHz / 21 = 0.761904 MHz.
    XINC_I2SM_MCK_32MDIV23  = 22,     ///< 16 MHz / 23 = 0.695652 MHz.
    XINC_I2SM_MCK_32MDIV30  = 29,     ///< 16 MHz / 30 = 0.533333 MHz.
    XINC_I2SM_MCK_32MDIV31  = 30,     ///< 16 MHz / 31 = 0.516129 MHz.
    XINC_I2SM_MCK_32MDIV32  = 31,     ///< 16 MHz / 32 = 0.5 MHz.
    XINC_I2SM_MCK_32MDIV42  = 41,     ///< 16 MHz / 42 = 0.380952 MHz.
    XINC_I2SM_MCK_32MDIV63  = 62,     ///< 16 MHz / 63 = 0.253968 MHz.
    XINC_I2SM_MCK_32MDIV125 = 124,//I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV125     ///< 16 MHz / 125 = 0.128 MHz.
} xinc_i2sm_mck_t;

/** @brief I2SM MCK/LRCK ratios. */
typedef enum
{
    XINC_I2SM_RATIO_32X  = 0,//I2SM_CONFIG_RATIO_RATIO_32X,  ///< LRCK = MCK / 32.
    XINC_I2SM_RATIO_48X  = 1,//I2SM_CONFIG_RATIO_RATIO_48X,  ///< LRCK = MCK / 48.
    XINC_I2SM_RATIO_64X  = 2,//I2SM_CONFIG_RATIO_RATIO_64X,  ///< LRCK = MCK / 64.
    XINC_I2SM_RATIO_96X  = 3,//I2SM_CONFIG_RATIO_RATIO_96X,  ///< LRCK = MCK / 96.
    XINC_I2SM_RATIO_128X = 4,//I2SM_CONFIG_RATIO_RATIO_128X, ///< LRCK = MCK / 128.
    XINC_I2SM_RATIO_192X = 5,//I2SM_CONFIG_RATIO_RATIO_192X, ///< LRCK = MCK / 192.
    XINC_I2SM_RATIO_256X = 6,//I2SM_CONFIG_RATIO_RATIO_256X, ///< LRCK = MCK / 256.
    XINC_I2SM_RATIO_384X = 7,//I2SM_CONFIG_RATIO_RATIO_384X, ///< LRCK = MCK / 384.
    XINC_I2SM_RATIO_512X = 8,//I2SM_CONFIG_RATIO_RATIO_512X  ///< LRCK = MCK / 512.
} xinc_i2sm_ratio_t;

/** @brief I2SM sample widths. */
typedef enum
{
   
    XINC_I2SM_SWIDTH_8BIT  = 1,//I2SM_CONFIG_SWIDTH_SWIDTH_8Bit,  ///< 8 bit.
    XINC_I2SM_SWIDTH_12BIT  = 2,//I2SM_CONFIG_SWIDTH_SWIDTH_12Bit,  ///< 12 bit.
    XINC_I2SM_SWIDTH_16BIT  = 3,//I2SM_CONFIG_SWIDTH_SWIDTH_16Bit,  ///< 16 bit.
    XINC_I2SM_SWIDTH_20BIT = 4,//I2SM_CONFIG_SWIDTH_SWIDTH_20Bit, ///< 20 bit.
    XINC_I2SM_SWIDTH_24BIT = 5,//I2SM_CONFIG_SWIDTH_SWIDTH_24Bit  ///< 24 bit.
    XINC_I2SM_SWIDTH_28BIT = 6,//I2SM_CONFIG_SWIDTH_SWIDTH_24Bit  ///< 28 bit.
    XINC_I2SM_SWIDTH_32BIT = 7,//I2SM_CONFIG_SWIDTH_SWIDTH_24Bit  ///< 32 bit.
} xinc_i2sm_swidth_t;

/** @brief I2SM alignments of sample within a frame. */
typedef enum
{
    XINC_I2SM_ALIGN_LEFT  = 0,//I2SM_CONFIG_ALIGN_ALIGN_Left, ///< Left-aligned.
    XINC_I2SM_ALIGN_RIGHT = 1,//I2SM_CONFIG_ALIGN_ALIGN_Right ///< Right-aligned.
} xinc_i2sm_align_t;

/** @brief I2SM frame formats. */
typedef enum
{
    XINC_I2SM_FORMAT_I2S     = 0,//I2SM_CONFIG_FORMAT_FORMAT_I2S,    ///< Original I2S format.
    XINC_I2SM_FORMAT_ALIGNED = 1,//I2SM_CONFIG_FORMAT_FORMAT_Aligned ///< Alternate (left-aligned or right-aligned) format.
} xinc_i2sm_format_t;

/** @brief I2SM enabled channels. */
typedef enum
{
    XINC_I2SM_CHANNELS_STEREO = 0,//I2SM_CONFIG_CHANNELS_CHANNELS_Stereo, ///< Stereo.
    XINC_I2SM_CHANNELS_LEFT   = 1,//I2SM_CONFIG_CHANNELS_CHANNELS_Left,   ///< Left only.
    XINC_I2SM_CHANNELS_RIGHT  = 2,//I2SM_CONFIG_CHANNELS_CHANNELS_Right   ///< Right only.
} xinc_i2sm_channels_t;


/**
 * @brief Function for enabling specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be enabled.
 */
__STATIC_INLINE void xinc_i2sm_int_enable(XINC_I2S_Type * p_reg, uint32_t mask);

/**
 * @brief Function for disabling specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be disabled.
 */
__STATIC_INLINE void xinc_i2sm_int_disable(XINC_I2S_Type * p_reg, uint32_t mask);

/**
 * @brief Function for retrieving the state of a given interrupt.
 *
 * @param[in] p_reg   Pointer to the structure of registers of the peripheral.
 * @param[in] i2sm_int Interrupt to be checked.
 *
 * @retval true  The interrupt is enabled.
 * @retval false The interrupt is not enabled.
 */
__STATIC_INLINE bool xinc_i2sm_int_enable_check(XINC_I2S_Type const * p_reg,
                                              xinc_i2sm_int_mask_t   i2sm_int);

/**
 * @brief Function for enabling the I2SM peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_i2sm_enable(XINC_I2S_Type * p_reg);

/**
 * @brief Function for disabling the I2SM peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_i2sm_disable(XINC_I2S_Type * p_reg);




/**
 * @brief Function for setting the I2SM peripheral configuration.
 *
 * @param[in] p_reg        Pointer to the structure of registers of the peripheral.
 * @param[in] mode         Mode of operation (master or slave).
 * @param[in] format       I2SM frame format.
 * @param[in] alignment    Alignment of sample within a frame.
 * @param[in] sample_width Sample width.
 * @param[in] channels     Enabled channels.
 * @param[in] mck_setup    Master clock generator setup.
 * @param[in] ratio        MCK/LRCK ratio.
 *
 * @retval true  The configuration has been set successfully.
 * @retval false The specified configuration is not allowed.
 */
__STATIC_INLINE bool xinc_i2sm_configure_check(XINC_I2S_Type *     p_reg,
                                       xinc_i2sm_mode_t     mode,
                                       xinc_i2sm_format_t   format,
                                       xinc_i2sm_align_t    alignment,
                                       xinc_i2sm_swidth_t   sample_width,
                                       xinc_i2sm_channels_t channels,
                                       xinc_i2sm_mck_t      mck_setup,
                                       xinc_i2sm_ratio_t    ratio);






#ifndef SUPPRESS_INLINE_IMPLEMENTATION




__STATIC_INLINE void xinc_i2sm_int_enable(XINC_I2S_Type * p_reg, uint32_t mask)
{
    p_reg->INT_EN |= mask;
}

__STATIC_INLINE void xinc_i2sm_int_disable(XINC_I2S_Type * p_reg, uint32_t mask)
{
    p_reg->INT_EN &= ~mask;
}

__STATIC_INLINE bool xinc_i2sm_int_enable_check(XINC_I2S_Type const * p_reg,
                                              xinc_i2sm_int_mask_t   i2sm_int)
{
    return (bool)(p_reg->INT_EN & i2sm_int);
}

__STATIC_INLINE void xinc_i2sm_enable(XINC_I2S_Type * p_reg)
{
    p_reg->TRANS_EN |= (I2S_I2S_TRANS_EN_I2S_ENABLE_Enable  << I2S_I2S_TRANS_EN_I2S_ENABLE_Pos);
}

__STATIC_INLINE void xinc_i2sm_disable(XINC_I2S_Type * p_reg)
{
    p_reg->TRANS_EN &= ~(I2S_I2S_TRANS_EN_I2S_ENABLE_Enable  << I2S_I2S_TRANS_EN_I2S_ENABLE_Pos);
}


__STATIC_INLINE bool xinc_i2sm_configure_check(XINC_I2S_Type *     p_reg,
                                       xinc_i2sm_mode_t     mode,
                                       xinc_i2sm_format_t   format,
                                       xinc_i2sm_align_t    alignment,
                                       xinc_i2sm_swidth_t   sample_width,
                                       xinc_i2sm_channels_t channels,
                                       xinc_i2sm_mck_t      mck_setup,
                                       xinc_i2sm_ratio_t    ratio)
{
    if (mode == XINC_I2SM_MODE_MASTER)
    {
        // The MCK/LRCK ratio must be a multiple of 2 * sample width.
        if (((sample_width == XINC_I2SM_SWIDTH_16BIT) &&
                 (ratio == XINC_I2SM_RATIO_48X))
            ||
            ((sample_width == XINC_I2SM_SWIDTH_24BIT) &&
                ((ratio == XINC_I2SM_RATIO_32X)  ||
                 (ratio == XINC_I2SM_RATIO_64X)  ||
                 (ratio == XINC_I2SM_RATIO_128X) ||
                 (ratio == XINC_I2SM_RATIO_256X) ||
                 (ratio == XINC_I2SM_RATIO_512X))))
        {
            return false;
        }
    }


    if (mck_setup == XINC_I2SM_MCK_SAME_PCLK)
    {

    }
    else
    {

    }

    return true;
}




#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_I2SM_H__
