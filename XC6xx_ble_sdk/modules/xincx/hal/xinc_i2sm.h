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
    XINC_I2SM_EVENT_RXPTRUPD = 0, ///< The RXD.PTR register has been copied to internal double buffers.
    XINC_I2SM_EVENT_TXPTRUPD = 1, ///< The TXD.PTR register has been copied to internal double buffers.
    XINC_I2SM_EVENT_STOPPED  = 2  ///< I2SM transfer stopped.
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
    XINC_I2SM_MCK_DISABLED  = 0,                                       ///< MCK disabled.
#if defined(I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV2) || defined(__XINCX_DOXYGEN__)
    // [conversion to 'int' needed to prevent compilers from complaining
    //  that the provided value (0x80000000UL) is out of range of "int"]
    XINC_I2SM_MCK_32MDIV2   = (int)I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV2, ///< 32 MHz / 2 = 16.0 MHz.
#endif
#if defined(I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV3) || defined(__XINCX_DOXYGEN__)
    XINC_I2SM_MCK_32MDIV3   = I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV3,      ///< 32 MHz / 3 = 10.6666667 MHz.
#endif
#if defined(I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV4) || defined(__XINCX_DOXYGEN__)
    XINC_I2SM_MCK_32MDIV4   = I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV4,      ///< 32 MHz / 4 = 8.0 MHz.
#endif
#if defined(I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV5) || defined(__XINCX_DOXYGEN__)
    XINC_I2SM_MCK_32MDIV5   = I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV5,      ///< 32 MHz / 5 = 6.4 MHz.
#endif
#if defined(I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV6) || defined(__XINCX_DOXYGEN__)
    XINC_I2SM_MCK_32MDIV6   = I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV6,      ///< 32 MHz / 6 = 5.3333333 MHz.
#endif
    XINC_I2SM_MCK_32MDIV8   = 8,      ///< 32 MHz / 8 = 4.0 MHz.
    XINC_I2SM_MCK_32MDIV10  = 9,     ///< 32 MHz / 10 = 3.2 MHz.
    XINC_I2SM_MCK_32MDIV11  = 10,     ///< 32 MHz / 11 = 2.9090909 MHz.
    XINC_I2SM_MCK_32MDIV15  = 11,     ///< 32 MHz / 15 = 2.1333333 MHz.
    XINC_I2SM_MCK_32MDIV16  = 12,     ///< 32 MHz / 16 = 2.0 MHz.
    XINC_I2SM_MCK_32MDIV21  = 13,     ///< 32 MHz / 21 = 1.5238095 MHz.
    XINC_I2SM_MCK_32MDIV23  = 14,     ///< 32 MHz / 23 = 1.3913043 MHz.
    XINC_I2SM_MCK_32MDIV30  = 15,     ///< 32 MHz / 30 = 1.0666667 MHz.
    XINC_I2SM_MCK_32MDIV31  = 16,     ///< 32 MHz / 31 = 1.0322581 MHz.
    XINC_I2SM_MCK_32MDIV32  = 17,     ///< 32 MHz / 32 = 1.0 MHz.
    XINC_I2SM_MCK_32MDIV42  = 18,     ///< 32 MHz / 42 = 0.7619048 MHz.
    XINC_I2SM_MCK_32MDIV63  = 19,     ///< 32 MHz / 63 = 0.5079365 MHz.
    XINC_I2SM_MCK_32MDIV125 = 20,//I2SM_CONFIG_MCKFREQ_MCKFREQ_32MDIV125     ///< 32 MHz / 125 = 0.256 MHz.
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
    XINC_I2SM_SWIDTH_8BIT  = 0,//I2SM_CONFIG_SWIDTH_SWIDTH_8Bit,  ///< 8 bit.
    XINC_I2SM_SWIDTH_16BIT = 1,//I2SM_CONFIG_SWIDTH_SWIDTH_16Bit, ///< 16 bit.
    XINC_I2SM_SWIDTH_24BIT = 2,//I2SM_CONFIG_SWIDTH_SWIDTH_24Bit  ///< 24 bit.
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
    XINC_I2SM_FORMAT_I2SM     = 0,//I2SM_CONFIG_FORMAT_FORMAT_I2SM,    ///< Original I2SM format.
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
__STATIC_INLINE bool xinc_i2sm_configure(XINC_I2S_Type *     p_reg,
                                       xinc_i2sm_mode_t     mode,
                                       xinc_i2sm_format_t   format,
                                       xinc_i2sm_align_t    alignment,
                                       xinc_i2sm_swidth_t   sample_width,
                                       xinc_i2sm_channels_t channels,
                                       xinc_i2sm_mck_t      mck_setup,
                                       xinc_i2sm_ratio_t    ratio);

/**
 * @brief Function for setting up the I2SM transfer.
 *
 * This function sets up the RX and TX buffers and enables reception or
 * transmission (or both) accordingly. If the transfer in a given direction is not
 * required, pass NULL instead of the pointer to the corresponding buffer.
 *
 * @param[in] p_reg       Pointer to the structure of registers of the peripheral.
 * @param[in] size        Size of the buffers (in 32-bit words).
 * @param[in] p_rx_buffer Pointer to the receive buffer.
 *                        Pass NULL to disable reception.
 * @param[in] p_tx_buffer Pointer to the transmit buffer.
 *                        Pass NULL to disable transmission.
 */
__STATIC_INLINE void xinc_i2sm_transfer_set(XINC_I2S_Type *   p_reg,
                                          uint16_t         size,
                                          uint32_t *       p_rx_buffer,
                                          uint32_t const * p_tx_buffer);

/**
 * @brief Function for setting the pointer to the receive buffer.
 *
 * @note The size of the buffer can be set only by calling
 *       @ref xinc_i2sm_transfer_set.
 *
 * @param[in] p_reg    Pointer to the structure of registers of the peripheral.
 * @param[in] p_buffer Pointer to the receive buffer.
 */
__STATIC_INLINE void xinc_i2sm_rx_buffer_set(XINC_I2S_Type * p_reg,
                                           uint32_t *     p_buffer);

/**
 * @brief Function for getting the pointer to the receive buffer.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Pointer to the receive buffer.
 */
__STATIC_INLINE uint32_t * xinc_i2sm_rx_buffer_get(XINC_I2S_Type const * p_reg);

/**
 * @brief Function for setting the pointer to the transmit buffer.
 *
 * @note The size of the buffer can be set only by calling
 *       @ref xinc_i2sm_transfer_set.
 *
 * @param[in] p_reg    Pointer to the structure of registers of the peripheral.
 * @param[in] p_buffer Pointer to the transmit buffer.
 */
__STATIC_INLINE void xinc_i2sm_tx_buffer_set(XINC_I2S_Type *   p_reg,
                                           uint32_t const * p_buffer);

/**
 * @brief Function for getting the pointer to the transmit buffer.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Pointer to the transmit buffer.
 */
__STATIC_INLINE uint32_t * xinc_i2sm_tx_buffer_get(XINC_I2S_Type const * p_reg);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION




__STATIC_INLINE void xinc_i2sm_int_enable(XINC_I2S_Type * p_reg, uint32_t mask)
{
   // p_reg->INTENSET = mask;
}

__STATIC_INLINE void xinc_i2sm_int_disable(XINC_I2S_Type * p_reg, uint32_t mask)
{
  //  p_reg->INTENCLR = mask;
}

__STATIC_INLINE bool xinc_i2sm_int_enable_check(XINC_I2S_Type const * p_reg,
                                              xinc_i2sm_int_mask_t   i2sm_int)
{
    return (bool)0;//(p_reg->INTENSET & i2sm_int);
}

__STATIC_INLINE void xinc_i2sm_enable(XINC_I2S_Type * p_reg)
{
   // p_reg->ENABLE = (I2SM_ENABLE_ENABLE_Enabled << I2SM_ENABLE_ENABLE_Pos);
}

__STATIC_INLINE void xinc_i2sm_disable(XINC_I2S_Type * p_reg)
{
    //p_reg->ENABLE = (I2SM_ENABLE_ENABLE_Disabled << I2SM_ENABLE_ENABLE_Pos);
}


__STATIC_INLINE bool xinc_i2sm_configure(XINC_I2S_Type *     p_reg,
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

//    p_reg->CONFIG.MODE     = mode;
//    p_reg->CONFIG.FORMAT   = format;
//    p_reg->CONFIG.ALIGN    = alignment;
//    p_reg->CONFIG.SWIDTH   = sample_width;
//    p_reg->CONFIG.CHANNELS = channels;
//    p_reg->CONFIG.RATIO    = ratio;

    if (mck_setup == XINC_I2SM_MCK_DISABLED)
    {
//        p_reg->CONFIG.MCKEN =
//            (I2SM_CONFIG_MCKEN_MCKEN_Disabled << I2SM_CONFIG_MCKEN_MCKEN_Pos);
    }
    else
    {
//        p_reg->CONFIG.MCKFREQ = mck_setup;
//        p_reg->CONFIG.MCKEN =
//            (I2SM_CONFIG_MCKEN_MCKEN_Enabled << I2SM_CONFIG_MCKEN_MCKEN_Pos);
    }

    return true;
}

__STATIC_INLINE void xinc_i2sm_transfer_set(XINC_I2S_Type *   p_reg,
                                          uint16_t         size,
                                          uint32_t *       p_buffer_rx,
                                          uint32_t const * p_buffer_tx)
{
   // p_reg->RXTXD.MAXCNT = size;

//    xinc_i2sm_rx_buffer_set(p_reg, p_buffer_rx);
//    p_reg->CONFIG.RXEN = (p_buffer_rx != NULL) ? 1 : 0;

//    xinc_i2sm_tx_buffer_set(p_reg, p_buffer_tx);
//    p_reg->CONFIG.TXEN = (p_buffer_tx != NULL) ? 1 : 0;
}

__STATIC_INLINE void xinc_i2sm_rx_buffer_set(XINC_I2S_Type * p_reg,
                                           uint32_t * p_buffer)
{
    //p_reg->RXD.PTR = (uint32_t)p_buffer;
}

__STATIC_INLINE uint32_t * xinc_i2sm_rx_buffer_get(XINC_I2S_Type const * p_reg)
{
    return 0;//(uint32_t *)(p_reg->RXD.PTR);
}

__STATIC_INLINE void xinc_i2sm_tx_buffer_set(XINC_I2S_Type * p_reg,
                                           uint32_t const * p_buffer)
{
   // p_reg->TXD.PTR = (uint32_t)p_buffer;
}

__STATIC_INLINE uint32_t * xinc_i2sm_tx_buffer_get(XINC_I2S_Type const * p_reg)
{
    return 0;//(uint32_t *)(p_reg->TXD.PTR);
}

#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_I2SM_H__
