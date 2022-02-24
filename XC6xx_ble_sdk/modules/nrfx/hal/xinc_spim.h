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

#ifndef XINC_SPIM_H__
#define XINC_SPIM_H__

#include <nrfx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_spim_hal SPIM HAL
 * @{
 * @ingroup xinc_spim
 * @brief   Hardware access layer for managing the SPIM peripheral.
 */

/**
 * @brief This value can be used as a parameter for the @ref xinc_spim_pins_set
 *        function to specify that a given SPI signal (SCK, MOSI, or MISO)
 *        shall not be connected to a physical pin.
 */
#define XINC_SPIM_PIN_NOT_CONNECTED  0xFFFFFFFF

#if defined(SPIM_DCXCNT_DCXCNT_Msk) || defined(__NRFX_DOXYGEN__)
/**
 * @brief This value specified in the DCX line configuration causes this line
 *        to be set low during whole transmission (all transmitted bytes are
 *        marked as command bytes). Any lower value causes the DCX line to be
 *        switched from low to high after this number of bytes is transmitted
 *        (all remaining bytes are marked as data bytes).
 */
#define XINC_SPIM_DCX_CNT_ALL_CMD 0xF
#endif

/** @brief Macro for checking if the hardware chip select function is available. */
#define XINC_SPIM_HW_CSN_PRESENT                        \
    (NRFX_CHECK(SPIM0_FEATURE_HARDWARE_CSN_PRESENT) || \
     NRFX_CHECK(SPIM1_FEATURE_HARDWARE_CSN_PRESENT) || \
     NRFX_CHECK(SPIM2_FEATURE_HARDWARE_CSN_PRESENT) || \
     NRFX_CHECK(SPIM3_FEATURE_HARDWARE_CSN_PRESENT))



/**
 * @brief SPIM shortcuts.
 */
typedef enum
{
    XINC_SPIM_SHORT_END_START_MASK = SPIM_SHORTS_END_START_Msk, ///< Shortcut between END event and START task.
    XINC_SPIM_ALL_SHORTS_MASK      = SPIM_SHORTS_END_START_Msk  ///< All SPIM shortcuts.
} xinc_spim_short_mask_t;

/** @brief SPIM interrupts. */
typedef enum
{
    XINC_SPIM_INT_STOPPED_MASK = SPIM_INTENSET_STOPPED_Msk,  ///< Interrupt on STOPPED event.
    XINC_SPIM_INT_ENDRX_MASK   = SPIM_INTENSET_ENDRX_Msk,    ///< Interrupt on ENDRX event.
    XINC_SPIM_INT_END_MASK     = SPIM_INTENSET_END_Msk,      ///< Interrupt on END event.
    XINC_SPIM_INT_ENDTX_MASK   = SPIM_INTENSET_ENDTX_Msk,    ///< Interrupt on ENDTX event.
    XINC_SPIM_INT_STARTED_MASK = SPIM_INTENSET_STARTED_Msk,  ///< Interrupt on STARTED event.
    XINC_SPIM_ALL_INTS_MASK    = SPIM_INTENSET_STOPPED_Msk |
                                SPIM_INTENSET_ENDRX_Msk   |
                                SPIM_INTENSET_END_Msk     |
                                SPIM_INTENSET_ENDTX_Msk   |
                                SPIM_INTENSET_STARTED_Msk   ///< All SPIM interrupts.
} xinc_spim_int_mask_t;

/** @brief SPI master data rates. */
typedef enum
{
    XINC_SPIM_FREQ_125K = SPIM_FREQUENCY_FREQUENCY_K125,    ///< 125 kbps.
    XINC_SPIM_FREQ_250K = SPIM_FREQUENCY_FREQUENCY_K250,    ///< 250 kbps.
    XINC_SPIM_FREQ_500K = SPIM_FREQUENCY_FREQUENCY_K500,    ///< 500 kbps.
    XINC_SPIM_FREQ_1M   = SPIM_FREQUENCY_FREQUENCY_M1,      ///< 1 Mbps.
    XINC_SPIM_FREQ_2M   = SPIM_FREQUENCY_FREQUENCY_M2,      ///< 2 Mbps.
    XINC_SPIM_FREQ_4M   = SPIM_FREQUENCY_FREQUENCY_M4,      ///< 4 Mbps.
    XINC_SPIM_FREQ_8M   = SPIM_FREQUENCY_FREQUENCY_M8, ///< 8 Mbps.
    XINC_SPIM_FREQ_16M  = SPIM_FREQUENCY_FREQUENCY_M16,     ///< 16 Mbps.


} xinc_spim_frequency_t;

/** @brief SPI modes. */
typedef enum
{
    XINC_SPIM_MODE_0, ///< SCK active high, sample on leading edge of clock.
    XINC_SPIM_MODE_1, ///< SCK active high, sample on trailing edge of clock.
    XINC_SPIM_MODE_2, ///< SCK active low, sample on leading edge of clock.
    XINC_SPIM_MODE_3  ///< SCK active low, sample on trailing edge of clock.
} xinc_spim_mode_t;

/** @brief SPI bit orders. */
typedef enum
{
    XINC_SPIM_BIT_ORDER_MSB_FIRST = SPIM_CONFIG_ORDER_MsbFirst, ///< Most significant bit shifted out first.
    XINC_SPIM_BIT_ORDER_LSB_FIRST = SPIM_CONFIG_ORDER_LsbFirst  ///< Least significant bit shifted out first.
} xinc_spim_bit_order_t;

#if (XINC_SPIM_HW_CSN_PRESENT) || defined(__NRFX_DOXYGEN__)
/** @brief SPI CSN pin polarity. */
typedef enum
{
    XINC_SPIM_CSN_POL_LOW  = SPIM_CSNPOL_CSNPOL_LOW, ///< Active low (idle state high).
    XINC_SPIM_CSN_POL_HIGH = SPIM_CSNPOL_CSNPOL_HIGH ///< Active high (idle state low).
} xinc_spim_csn_pol_t;
#endif // (XINC_SPIM_HW_CSN_PRESENT) || defined(__NRFX_DOXYGEN__)



/**
 * @brief Function for enabling the specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be enabled.
 */
__STATIC_INLINE void xinc_spim_int_enable(XINC_SPIM_Type * p_reg,
                                         uint32_t        mask);

/**
 * @brief Function for disabling the specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be disabled.
 */
__STATIC_INLINE void xinc_spim_int_disable(XINC_SPIM_Type * p_reg,
                                          uint32_t        mask);

/**
 * @brief Function for retrieving the state of a given interrupt.
 *
 * @param[in] p_reg    Pointer to the structure of registers of the peripheral.
 * @param[in] spim_int Interrupt to be checked.
 *
 * @retval true  The interrupt is enabled.
 * @retval false The interrupt is not enabled.
 */
__STATIC_INLINE bool xinc_spim_int_enable_check(XINC_SPIM_Type *     p_reg,
                                               xinc_spim_int_mask_t spim_int);

/**
 * @brief Function for enabling the SPIM peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_spim_enable(XINC_SPIM_Type * p_reg);

/**
 * @brief Function for disabling the SPIM peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_spim_disable(XINC_SPIM_Type * p_reg);




/**
 * @brief Function for setting the SPI master data rate.
 *
 * @param[in] p_reg     Pointer to the structure of registers of the peripheral.
 * @param[in] frequency SPI frequency.
 */
__STATIC_INLINE void xinc_spim_frequency_set(XINC_SPIM_Type *      p_reg,
                                            xinc_spim_frequency_t frequency);



/**
 * @brief Function for setting the SPI configuration.
 *
 * @param[in] p_reg         Pointer to the structure of registers of the peripheral.
 * @param[in] spi_mode      SPI mode.
 * @param[in] spi_bit_order SPI bit order.
 */
__STATIC_INLINE void xinc_spim_configure(XINC_SPIM_Type *      p_reg,
                                        xinc_spim_mode_t      spi_mode,
                                        xinc_spim_bit_order_t spi_bit_order);






#ifndef SUPPRESS_INLINE_IMPLEMENTATION




__STATIC_INLINE void xinc_spim_configure(XINC_SPIM_Type *      p_reg,
                                        xinc_spim_mode_t      spi_mode,
                                        xinc_spim_bit_order_t spi_bit_order)
{
    uint32_t config = (spi_bit_order == XINC_SPIM_BIT_ORDER_MSB_FIRST ?
        SPIM_CONFIG_ORDER_MsbFirst : SPIM_CONFIG_ORDER_LsbFirst);
	
		config = 0;
    switch (spi_mode)
    {
    default:
    case XINC_SPIM_MODE_0:
//        config |= (SPIM_CONFIG_CPOL_ActiveHigh << SPIM_CONFIG_CPOL_Pos) |
//                  (SPIM_CONFIG_CPHA_Leading    << SPIM_CONFIG_CPHA_Pos);
//		
		    config |= (0 << 7) |(0    << 6); 
                  
        break;

    case XINC_SPIM_MODE_1:
//        config |= (SPIM_CONFIG_CPOL_ActiveHigh << SPIM_CONFIG_CPOL_Pos) |
//                  (SPIM_CONFIG_CPHA_Trailing   << SPIM_CONFIG_CPHA_Pos);
		    config |= (0 << 7) |(1    << 6); 
        break;

    case XINC_SPIM_MODE_2:
//        config |= (SPIM_CONFIG_CPOL_ActiveLow  << SPIM_CONFIG_CPOL_Pos) |
//                  (SPIM_CONFIG_CPHA_Leading    << SPIM_CONFIG_CPHA_Pos);
		   config |= (1 << 7) |(0    << 6); 
        break;

    case XINC_SPIM_MODE_3:
//        config |= (SPIM_CONFIG_CPOL_ActiveLow  << SPIM_CONFIG_CPOL_Pos) |
//                  (SPIM_CONFIG_CPHA_Trailing   << SPIM_CONFIG_CPHA_Pos);
		config |= (1 << 7) |(1    << 6); 
        break;
    }
    p_reg->CTRL0 &= ~(0x03 << 6);
		p_reg->CTRL0 |= config;
}




#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_SPIM_H__
