/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_FLASH_H__
#define XINC_FLASH_H__

#include <xincx.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "compiler_abstraction.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_nvmc_hal_deprecated NVMC HAL (deprecated)
 * @{
 * @ingroup xinc_nvmc
 * @brief   Hardware access layer (HAL) for managing the Non-Volatile Memory Controller (NVMC) peripheral.
 *
 * This driver allows writing to the non-volatile memory (NVM) regions
 * of the chip. To write to NVM, the controller must be powered
 * on and the relevant page must be erased.
 */


void xinc_flash_init(void);


void xinc_flash_read_bytes(uint32_t address, uint8_t * src, uint32_t num_bytes);
/**
 * @brief Erase a page in flash. This is required before writing to any address in the page.
 *
 * @param address Start address of the page.
 */
void xinc_flash_page_erase(uint32_t address);

/**
 * @brief Write a single byte to flash.
 *
 * The function reads the word containing the byte, and then rewrites the entire word.
 *
 * @param address Address to write to.
 * @param value   Value to write.
 */
void xinc_flash_write_byte(uint32_t address , uint8_t value);

/**
 * @brief Write a 32-bit word to flash.
 *
 * @param address Address to write to.
 * @param value   Value to write.
 */
void xinc_flash_write_word(uint32_t address, uint32_t value);

/**
 * @brief Write consecutive bytes to flash.
 *
 * @param address   Address to write to.
 * @param src       Pointer to data to copy from.
 * @param num_bytes Number of bytes in src to write.
 */
void xinc_flash_write_bytes(uint32_t  address, uint8_t * src, uint32_t num_bytes);

/**
 * @brief Write consecutive words to flash.
 *
 * @param address   Address to write to.
 * @param src       Pointer to data to copy from.
 * @param num_words Number of words in src to write.
 */
void xinc_flash_write_words(uint32_t address, const uint32_t * src, uint32_t num_words);

/** @} */

/**
 * @defgroup xinc_flash_hal FLASH HAL
 * @{
 * @ingroup xinc_flash
 * @brief   Hardware access layer (HAL) for managing the flash peripheral.
 */

/** @brief flash modes. */
typedef enum
{
    XINC_FLASH_MODE_READONLY      = 0UL,//< FLASH in read-only mode.
    XINC_FLASH_MODE_WRITE         = 1UL,//< FLASH in read and write mode.
    XINC_FLASH_MODE_ERASE         = 2UL,//< FLASH in read and erase mode.
 #if defined(NVMC_CONFIG_WEN_PEen)
    XINC_FLASH_MODE_PARTIAL_ERASE = 3UL, ///< NVMC in read and partial erase mode.
#endif
} xinc_flash_mode_t;


#if defined(FLASH_CONFIGNS_WEN_Msk) || defined(__NRFX_DOXYGEN__)
/** @brief Non-secure FLASH modes. */
typedef enum
{
    XINC_FLASH_NS_MODE_READONLY = 0UL, ///< Non-secure FLASH in read-only mode.
    XINC_FLASH_NS_MODE_WRITE    = 1UL, ///< Non-secure FLASH in read and write mode.
    XINC_FLASH_NS_MODE_ERASE    = 2UL, ///< Non-secure FLASH in read and erase mode.
} xinc_flash_ns_mode_t;
#endif

#if defined(XINC_FLASH_ICACHE_PRESENT)
/** @brief FLASH ICache configuration. */
typedef enum
{
    XINC_FLASH_ICACHE_DISABLE               = 0UL, ///< Instruction Cache disabled.
    XINC_FLASH_ICACHE_ENABLE                = 1UL,  ///< Instruction Cache enabled.
    XINC_FLASH_ICACHE_ENABLE_WITH_PROFILING = 1UL |2UL ///< Instruction Cache with cache profiling enabled.
                                               
} xinc_flash_icache_config_t;
#endif // defined(NRF_NVMC_ICACHE_PRESENT)

/**
 * @brief Function for checking if FLASH is ready to perform write or erase operation.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval true  FLASH can perform write or erase.
 * @retval false FLASH is busy and cannot perform next operation yet.
 */
__STATIC_INLINE bool xinc_flash_ready_check( uint8_t * p_reg);

#if defined(FLASH_READYNEXT_READYNEXT_Msk) || defined(__XINCX_DOXYGEN__)
/**
 * @brief Function for checking if FLASH is ready to accept the next write operation.
 *
 * NVM writing time can be reduced by using this function.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval true  NVMC can accept the next write. It will be buffered and will be taken
 *               into account as soon as the ongoing write operation is completed.
 * @retval false NVMC is busy and cannot accept the next write yet.
 */
__STATIC_INLINE bool xinc_flash_write_ready_check(XINC_FLASH_Type const * p_reg);
#endif // defined(FLASH_READYNEXT_READYNEXT_Msk) || defined(__XINCX_DOXYGEN__)

/**
 * @brief Function for setting the FLASH mode.
 *
 * Only activate erase and write modes when they are actively used.
 * If Instruction Cache (ICache) is present, enabling write or erase will
 * invalidate the cache and keep it invalidated.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mode  Desired operating mode for NVMC.
 */
__STATIC_INLINE void xinc_flash_mode_set(uint8_t * p_reg,
                                       xinc_flash_mode_t mode);

#if defined(FLASH_CONFIGNS_WEN_Msk) || defined(__XINCX_DOXYGEN__)
/**
 * @brief Function for setting the NVMC mode for non-secure Flash page operations.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mode  Desired operating mode for NVMC.
 */
__STATIC_INLINE void xinc_flash_nonsecure_mode_set(XINC_FLASH_Type *    p_reg,
                                                 xinc_flash_ns_mode_t mode);
#endif

/**
 * @brief Function for starting a single page erase in the Flash memory.
 *
 * The NVMC mode must be correctly configured with @ref xinc_nvmc_mode_set
 * before starting the erase operation.
 *
 * @param[in] p_reg     Pointer to the structure of registers of the peripheral.
 * @param[in] page_addr Address of the first word of the page to erase.
 */
__STATIC_INLINE void xinc_flash_page_erase_start(uint8_t * p_reg,
                                               uint32_t        page_addr);


/**
 * @brief Function for starting the erase of the whole FLASH
 *
 * This function purges all user code.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_flash_erase_all_start(uint8_t * p_reg);

#if defined(XINC_FLASH_PARTIAL_ERASE_PRESENT)
/**
 * @brief Function for configuring the page partial erase duration in milliseconds.
 *
 * @param[in] p_reg       Pointer to the structure of registers of the peripheral.
 * @param[in] duration_ms Page partial erase duration in milliseconds.
 */
__STATIC_INLINE void xinc_flash_partial_erase_duration_set(XINC_FLASH_Type * p_reg,
                                                         uint32_t        duration_ms);

/**
 * @brief Function for getting the current setting for the page partial erase duration.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval Interval duration setting in milliseconds.
 */
__STATIC_INLINE uint32_t xinc_flash_partial_erase_duration_get(XINC_FLASH_Type const * p_reg);

/**
 * @brief Function for starting a partial erase operation.
 *
 * It must be called successively until the page erase time is reached.
 *
 * @param[in] p_reg     Pointer to the structure of registers of the peripheral.
 * @param[in] page_addr Address of the first word of the page to erase.
 */
__STATIC_INLINE void xinc_flash_page_partial_erase_start(XINC_FLASH_Type * p_reg,
                                                       uint32_t        page_addr);
#endif // defined(XINC_NVMC_PARTIAL_ERASE_PRESENT)

#if defined(XINC_FLASH_ICACHE_PRESENT)
/**
 * @brief Function for applying the Instruction Cache (ICache) configuration.
 *
 * Enabling the cache can increase CPU performance and reduce power
 * consumption by reducing the number of wait cycles and the number
 * of flash accesses.
 *
 * @param[in] p_reg  Pointer to the structure of registers of the peripheral.
 * @param[in] config ICache configuration.
 */
__STATIC_INLINE void xinc_flash_icache_config_set(XINC_FLASH_Type *          p_reg,
                                                xinc_nvmc_icache_config_t config);

/**
 * @brief Function for checking if ICache is enabled.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval true  ICache enabled.
 * @retval false ICache disabled.
 */
__STATIC_INLINE bool xinc_flash_icache_enable_check(XINC_FLASH_Type const * p_reg);

/**
 * @brief Function for checking if the ICache profiling option is enabled.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval true  ICache profiling enabled.
 * @retval false ICache profiling disabled.
 */
__STATIC_INLINE bool xinc_flash_icache_profiling_enable_check(XINC_FLASH_Type const * p_reg);

/**
 * @brief Function for getting the number of ICache hits.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval Number of the ICache hits.
 */
__STATIC_INLINE uint32_t xinc_flash_icache_hit_get(XINC_FLASH_Type const * p_reg);

/**
 * @brief Function for getting the number of ICache misses.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval Number of the ICache misses.
 */
__STATIC_INLINE uint32_t xinc_flash_icache_miss_get(XINC_FLASH_Type const * p_reg);

/**
 * @brief Function for resetting the ICache hit and miss counters.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
 __STATIC_INLINE void xinc_flash_icache_hit_miss_reset(XINC_FLASH_Type * p_reg);
#endif // defined(XINC_FLASH_ICACHE_PRESENT)

#ifndef SUPPRESS_INLINE_IMPLEMENTATION

__STATIC_INLINE bool xinc_flash_ready_check(uint8_t * p_reg)
{
    return (bool) true ;//(p_reg->READY & NVMC_READY_READY_Msk);
}

#if defined(FLASH_READYNEXT_READYNEXT_Msk)
__STATIC_INLINE bool xinc_flash_write_ready_check(XINC_FLASH_Type const * p_reg)
{
    return (bool)
}
#endif

__STATIC_INLINE void xinc_flash_mode_set(uint8_t * p_reg,
                                       xinc_flash_mode_t mode)
{
   
}

#if defined(FLASH_CONFIGNS_WEN_Msk)
__STATIC_INLINE void xinc_flash_nonsecure_mode_set(XINC_FLASH_Type *    p_reg,
                                                 xinc_flash_ns_mode_t mode)
{
}
#endif

__STATIC_INLINE void xinc_flash_page_erase_start(uint8_t * p_reg,
                                               uint32_t        page_addr)
{

}


__STATIC_INLINE void xinc_flash_erase_all_start(uint8_t * p_reg)
{

}

#if defined(XINC_FLASH_PARTIAL_ERASE_PRESENT)
__STATIC_INLINE void xinc_flash_partial_erase_duration_set(XINC_FLASH_Type * p_reg,
                                                         uint32_t        duration_ms)
{

}

__STATIC_INLINE uint32_t xinc_flash_partial_erase_duration_get(XINC_FLASH_Type const * p_reg)
{
   
}

__STATIC_INLINE void xinc_flash_page_partial_erase_start(XINC_FLASH_Type * p_reg,
                                                       uint32_t        page_addr)
{

    xinc_flash_page_erase_start(p_reg, page_addr);

}
#endif // defined(XINC_FLASH_PARTIAL_ERASE_PRESENT)

#if defined(XINC_FLASH_ICACHE_PRESENT)
__STATIC_INLINE void xinc_flash_icache_config_set(XINC_FLASH_Type *          p_reg,
                                                xinc_flash_icache_config_t config)
{
  
}

__STATIC_INLINE bool xinc_flash_icache_enable_check(XINC_FLASH_Type const * p_reg)
{
    return (bool)true;
}

__STATIC_INLINE bool xinc_flash_icache_profiling_enable_check(XINC_FLASH_Type const * p_reg)
{
    return (bool)false;
}

__STATIC_INLINE uint32_t xinc_flash_icache_hit_get(XINC_FLASH_Type const * p_reg)
{
    return 0;
}

__STATIC_INLINE uint32_t xinc_flash_icache_miss_get(XINC_FLASH_Type const * p_reg)
{
    return 0;
}

__STATIC_INLINE void xinc_flash_icache_hit_miss_reset(XINC_FLASH_Type * p_reg)
{

}
#endif // defined(XINC_FLASH_ICACHE_PRESENT)

#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_FLASH_H__
