/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_LOG_H__
#define XINCX_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(XINCX_LOG_MODULE)
#define XINC_LOG_MODULE_NAME     XINCX_LOG_MODULE

#define XINCX_CONFIG_ENTRY(x)    CONCAT_3(XINCX_, XINCX_LOG_MODULE, x)

#if XINCX_CHECK(XINCX_CONFIG_ENTRY(_CONFIG_LOG_ENABLED))
#define XINC_LOG_LEVEL           XINCX_CONFIG_ENTRY(_CONFIG_LOG_LEVEL)
#define XINC_LOG_INFO_COLOR      XINCX_CONFIG_ENTRY(_CONFIG_INFO_COLOR)
#define XINC_LOG_DEBUG_COLOR     XINCX_CONFIG_ENTRY(_CONFIG_DEBUG_COLOR)
#else
#define XINC_LOG_LEVEL           0
#endif
#endif // defined(XINCX_LOG_MODULE)

#include <xinc_log.h>

#if defined(XINCX_LOG_MODULE)
XINC_LOG_MODULE_REGISTER();
#endif

#define TEST_MACRO_INFO(...)  XINC_LOG_INFO(__VA_ARGS__)
/**
 * @defgroup xincx_log xincx_log.h
 * @{
 * @ingroup xincx
 *
 * @brief This file contains macros that should be implemented according to
 *        the needs of the host environment into which @em xincx is integrated.
 */

/**
 * @brief Macro for logging a message with the severity level ERROR.
 */
#define XINCX_LOG_ERROR(...)     XINC_LOG_ERROR(__VA_ARGS__)

/**
 * @brief Macro for logging a message with the severity level WARNING.
 */
#define XINCX_LOG_WARNING(...)   XINC_LOG_WARNING(__VA_ARGS__)

/**
 * @brief Macro for logging a message with the severity level INFO.
 */
#define XINCX_LOG_INFO(...)      TEST_MACRO_INFO(__VA_ARGS__)

/**
 * @brief Macro for logging a message with the severity level DEBUG.
 */
#define XINCX_LOG_DEBUG(...)     XINC_LOG_DEBUG(__VA_ARGS__)


/**
 * @brief Macro for logging a memory dump with the severity level ERROR.
 *
 * @param[in] p_memory  Pointer to the memory region to be dumped.
 * @param[in] length    Length of the memory region in bytes.
 */
#define XINCX_LOG_HEXDUMP_ERROR(p_memory, length) \
    XINC_LOG_HEXDUMP_ERROR(p_memory, length)

/**
 * @brief Macro for logging a memory dump with the severity level WARNING.
 *
 * @param[in] p_memory  Pointer to the memory region to be dumped.
 * @param[in] length    Length of the memory region in bytes.
 */
#define XINCX_LOG_HEXDUMP_WARNING(p_memory, length) \
    XINC_LOG_HEXDUMP_WARNING(p_memory, length)

/**
 * @brief Macro for logging a memory dump with the severity level INFO.
 *
 * @param[in] p_memory  Pointer to the memory region to be dumped.
 * @param[in] length    Length of the memory region in bytes.
 */
#define XINCX_LOG_HEXDUMP_INFO(p_memory, length) \
    XINC_LOG_HEXDUMP_INFO(p_memory, length)

/**
 * @brief Macro for logging a memory dump with the severity level DEBUG.
 *
 * @param[in] p_memory  Pointer to the memory region to be dumped.
 * @param[in] length    Length of the memory region in bytes.
 */
#define XINCX_LOG_HEXDUMP_DEBUG(p_memory, length) \
    XINC_LOG_HEXDUMP_DEBUG(p_memory, length)


/**
 * @brief Macro for getting the textual representation of a given error code.
 *
 * @param[in] error_code  Error code.
 *
 * @return String containing the textual representation of the error code.
 */
#define XINCX_LOG_ERROR_STRING_GET(error_code) \
    XINC_LOG_ERROR_STRING_GET(error_code)

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINCX_LOG_H__
