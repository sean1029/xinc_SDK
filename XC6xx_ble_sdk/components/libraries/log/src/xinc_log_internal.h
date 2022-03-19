/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_LOG_INTERNAL_H__
#define XINC_LOG_INTERNAL_H__
#include "sdk_common.h"
#include "xinc.h"
#include "xinc_error.h"
#include "app_util.h"
#include <stdint.h>
#include <stdbool.h>
#include "xinc_log_instance.h"
#include "xinc_log_types.h"

#ifndef XINC_LOG_ERROR_COLOR
    #define XINC_LOG_ERROR_COLOR XINC_LOG_COLOR_DEFAULT
#endif

#ifndef XINC_LOG_WARNING_COLOR
    #define XINC_LOG_WARNING_COLOR XINC_LOG_COLOR_DEFAULT
#endif

#ifndef XINC_LOG_INFO_COLOR
    #define XINC_LOG_INFO_COLOR XINC_LOG_COLOR_DEFAULT
#endif

#ifndef XINC_LOG_DEBUG_COLOR
    #define XINC_LOG_DEBUG_COLOR XINC_LOG_COLOR_DEFAULT
#endif


#ifndef XINC_LOG_COLOR_DEFAULT
#define XINC_LOG_COLOR_DEFAULT 0
#endif

#ifndef XINC_LOG_DEFAULT_LEVEL
#define XINC_LOG_DEFAULT_LEVEL 0
#endif

#ifndef XINC_LOG_USES_COLORS
#define XINC_LOG_USES_COLORS       0
#endif

#ifndef XINC_LOG_USES_TIMESTAMP
#define XINC_LOG_USES_TIMESTAMP    0
#endif

#ifndef XINC_LOG_FILTERS_ENABLED
#define XINC_LOG_FILTERS_ENABLED   0
#endif

#ifndef XINC_LOG_MODULE_NAME
    #define XINC_LOG_MODULE_NAME app
#endif

#define XINC_LOG_LEVEL_BITS         3
#define XINC_LOG_LEVEL_MASK         ((1UL << XINC_LOG_LEVEL_BITS) - 1)
#define XINC_LOG_MODULE_ID_BITS     16
#define XINC_LOG_MODULE_ID_POS      16


#define XINC_LOG_MAX_NUM_OF_ARGS         6


#if XINC_LOG_FILTERS_ENABLED && XINC_LOG_ENABLED
    #define XINC_LOG_FILTER              XINC_LOG_ITEM_DATA_DYNAMIC(XINC_LOG_MODULE_NAME).filter
    #define XINC_LOG_INST_FILTER(p_inst) (p_inst)->filter
#else
    #undef XINC_LOG_FILTER
    #define XINC_LOG_FILTER              XINC_LOG_SEVERITY_DEBUG
    #define XINC_LOG_INST_FILTER(p_inst) XINC_LOG_SEVERITY_DEBUG
#endif

/**
 * @brief Macro for calculating module id based on address and section start address
 */
#define XINC_LOG_MODULE_ID_GET_CONST(addr) (((uint32_t)(addr) -                                     \
                                   (uint32_t)XINC_SECTION_START_ADDR(log_const_data)) /             \
                                    sizeof(xinc_log_module_const_data_t))
/**
 * @brief Macro for calculating module id based on address and section start address
 */
#define XINC_LOG_MODULE_ID_GET_DYNAMIC(addr) (((uint32_t)(addr) -                                   \
                                   (uint32_t)XINC_SECTION_START_ADDR(log_dynamic_data)) /           \
                                    sizeof(xinc_log_module_dynamic_data_t))


#if XINC_LOG_ENABLED
#define XINC_LOG_MODULE_ID        XINC_LOG_MODULE_ID_GET_CONST(&XINC_LOG_ITEM_DATA_CONST(XINC_LOG_MODULE_NAME))
#if XINC_LOG_FILTERS_ENABLED
#define XINC_LOG_INST_ID(p_inst)  XINC_LOG_MODULE_ID_GET_DYNAMIC(p_inst)
#else
#define XINC_LOG_INST_ID(p_inst)  XINC_LOG_MODULE_ID
#endif
#else
#define XINC_LOG_MODULE_ID       0
#define XINC_LOG_INST_ID(p_inst) 0
#endif


#define LOG_INTERNAL_X(N, ...)          CONCAT_2(LOG_INTERNAL_, N) (__VA_ARGS__)
#define LOG_INTERNAL(type, ...) LOG_INTERNAL_X(NUM_VA_ARGS_LESS_1( \
                                                           __VA_ARGS__), type, __VA_ARGS__)
#if XINC_LOG_ENABLED
#define XINC_LOG_INTERNAL_LOG_PUSH(_str) xinc_log_push(_str)
#define LOG_INTERNAL_0(type, str) \
    xinc_log_frontend_std_0(type, str)
#define LOG_INTERNAL_1(type, str, arg0) \
    /*lint -save -e571*/xinc_log_frontend_std_1(type, str, (uint32_t)(arg0))/*lint -restore*/
#define LOG_INTERNAL_2(type, str, arg0, arg1) \
    /*lint -save -e571*/xinc_log_frontend_std_2(type, str, (uint32_t)(arg0), \
            (uint32_t)(arg1))/*lint -restore*/
#define LOG_INTERNAL_3(type, str, arg0, arg1, arg2) \
    /*lint -save -e571*/xinc_log_frontend_std_3(type, str, (uint32_t)(arg0), \
            (uint32_t)(arg1), (uint32_t)(arg2))/*lint -restore*/
#define LOG_INTERNAL_4(type, str, arg0, arg1, arg2, arg3) \
    /*lint -save -e571*/xinc_log_frontend_std_4(type, str, (uint32_t)(arg0), \
            (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)(arg3))/*lint -restore*/
#define LOG_INTERNAL_5(type, str, arg0, arg1, arg2, arg3, arg4) \
    /*lint -save -e571*/xinc_log_frontend_std_5(type, str, (uint32_t)(arg0), \
            (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)(arg3), (uint32_t)(arg4))/*lint -restore*/
#define LOG_INTERNAL_6(type, str, arg0, arg1, arg2, arg3, arg4, arg5) \
    /*lint -save -e571*/xinc_log_frontend_std_6(type, str, (uint32_t)(arg0), \
            (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)(arg3), (uint32_t)(arg4), (uint32_t)(arg5))/*lint -restore*/


#else //XINC_LOG_ENABLED
#define XINC_LOG_INTERNAL_LOG_PUSH(_str) (void)(_str)
#define LOG_INTERNAL_0(_type, _str) \
               (void)(_type); (void)(_str)
#define LOG_INTERNAL_1(_type, _str, _arg0) \
               (void)(_type); (void)(_str); (void)(_arg0)
#define LOG_INTERNAL_2(_type, _str, _arg0, _arg1) \
               (void)(_type); (void)(_str); (void)(_arg0); (void)(_arg1)
#define LOG_INTERNAL_3(_type, _str, _arg0, _arg1, _arg2) \
               (void)(_type); (void)(_str); (void)(_arg0); (void)(_arg1); (void)(_arg2)
#define LOG_INTERNAL_4(_type, _str, _arg0, _arg1, _arg2, _arg3) \
               (void)(_type); (void)(_str); (void)(_arg0); (void)(_arg1); (void)(_arg2); (void)(_arg3)
#define LOG_INTERNAL_5(_type, _str, _arg0, _arg1, _arg2, _arg3, _arg4) \
               (void)(_type); (void)(_str); (void)(_arg0); (void)(_arg1); (void)(_arg2); (void)(_arg3); (void)(_arg4)
#define LOG_INTERNAL_6(_type, _str, _arg0, _arg1, _arg2, _arg3, _arg4, _arg5) \
               (void)(_type); (void)(_str); (void)(_arg0); (void)(_arg1); (void)(_arg2); (void)(_arg3); (void)(_arg4); (void)(_arg5)
#endif //XINC_LOG_ENABLED

#define LOG_SEVERITY_MOD_ID(severity) ((severity) | XINC_LOG_MODULE_ID << XINC_LOG_MODULE_ID_POS)
#define LOG_SEVERITY_INST_ID(severity,p_inst) ((severity) | XINC_LOG_INST_ID(p_inst) << XINC_LOG_MODULE_ID_POS)

#if XINC_LOG_ENABLED
#define LOG_HEXDUMP(_severity, _p_data, _length) \
            xinc_log_frontend_hexdump((_severity), (_p_data), (_length))
#else
#define LOG_HEXDUMP(_severity, _p_data, _length) \
             (void)(_severity); (void)(_p_data); (void)_length
#endif

#define XINC_LOG_INTERNAL_INST(level, level_id, p_inst, ...)                              \
    if (XINC_LOG_ENABLED && (XINC_LOG_LEVEL >= level) &&                                   \
        (level <= XINC_LOG_DEFAULT_LEVEL))                                                \
    {                                                                                    \
        if (XINC_LOG_INST_FILTER(p_inst) >= level)                                        \
        {                                                                                \
            LOG_INTERNAL(LOG_SEVERITY_INST_ID(level_id, p_inst), __VA_ARGS__);           \
        }                                                                                \
    }

#define XINC_LOG_INTERNAL_MODULE(level, level_id, ...)                                    \
    if (XINC_LOG_ENABLED && (XINC_LOG_LEVEL >= level) &&                                   \
        (level <= XINC_LOG_DEFAULT_LEVEL))                                                \
    {                                                                                    \
        if (XINC_LOG_FILTER >= level)                                                     \
        {                                                                                \
            LOG_INTERNAL(LOG_SEVERITY_MOD_ID(level_id), __VA_ARGS__);                    \
        }                                                                                \
    }

#define XINC_LOG_INTERNAL_HEXDUMP_INST(level, level_id, p_inst, p_data, len)        \
    if (XINC_LOG_ENABLED && (XINC_LOG_LEVEL >= level) &&                             \
        (level <= XINC_LOG_DEFAULT_LEVEL))                                          \
    {                                                                              \
        if (XINC_LOG_INST_FILTER(p_inst) >= level)                                  \
        {                                                                          \
            LOG_HEXDUMP(LOG_SEVERITY_INST_ID(level_id, p_inst),                    \
                                     (p_data), (len));                             \
        }                                                                          \
    }

#define XINC_LOG_INTERNAL_HEXDUMP_MODULE(level, level_id, p_data, len)              \
    if (XINC_LOG_ENABLED && (XINC_LOG_LEVEL >= level) &&                             \
        (level <= XINC_LOG_DEFAULT_LEVEL))                                          \
    {                                                                              \
        if (XINC_LOG_FILTER >= level)                                               \
        {                                                                          \
            LOG_HEXDUMP(LOG_SEVERITY_MOD_ID(level_id),                             \
                                     (p_data), (len));                             \
        }                                                                          \
    }																																							\

#define XINC_LOG_INTERNAL_INST_ERROR(p_inst, ...) \
                XINC_LOG_INTERNAL_INST(XINC_LOG_SEVERITY_ERROR, XINC_LOG_SEVERITY_ERROR, p_inst, __VA_ARGS__)

#define XINC_LOG_INTERNAL_ERROR(...) \
                XINC_LOG_INTERNAL_MODULE(XINC_LOG_SEVERITY_ERROR, XINC_LOG_SEVERITY_ERROR,__VA_ARGS__)

#define XINC_LOG_INTERNAL_HEXDUMP_INST_ERROR(p_inst, p_data, len) \
        XINC_LOG_INTERNAL_HEXDUMP_INST(XINC_LOG_SEVERITY_ERROR, XINC_LOG_SEVERITY_ERROR, p_inst, p_data, len)

#define XINC_LOG_INTERNAL_HEXDUMP_ERROR(p_data, len) \
        XINC_LOG_INTERNAL_HEXDUMP_MODULE(XINC_LOG_SEVERITY_ERROR, XINC_LOG_SEVERITY_ERROR, p_data, len)

#define XINC_LOG_INTERNAL_INST_WARNING(p_inst, ...) \
            XINC_LOG_INTERNAL_INST(XINC_LOG_SEVERITY_WARNING, XINC_LOG_SEVERITY_WARNING, p_inst, __VA_ARGS__)

#define XINC_LOG_INTERNAL_WARNING(...) \
            XINC_LOG_INTERNAL_MODULE(XINC_LOG_SEVERITY_WARNING, XINC_LOG_SEVERITY_WARNING,__VA_ARGS__)

#define XINC_LOG_INTERNAL_HEXDUMP_INST_WARNING(p_inst, p_data, len) \
        XINC_LOG_INTERNAL_HEXDUMP_INST(XINC_LOG_SEVERITY_WARNING, XINC_LOG_SEVERITY_WARNING, p_inst, p_data, len)

#define XINC_LOG_INTERNAL_HEXDUMP_WARNING(p_data, len) \
        XINC_LOG_INTERNAL_HEXDUMP_MODULE(XINC_LOG_SEVERITY_WARNING, XINC_LOG_SEVERITY_WARNING, p_data, len)

#define XINC_LOG_INTERNAL_INST_INFO(p_inst, ...) \
        XINC_LOG_INTERNAL_INST(XINC_LOG_SEVERITY_INFO, XINC_LOG_SEVERITY_INFO, p_inst, __VA_ARGS__)

#define XINC_LOG_INTERNAL_INFO(...) \
        XINC_LOG_INTERNAL_MODULE(XINC_LOG_SEVERITY_INFO, XINC_LOG_SEVERITY_INFO, __VA_ARGS__)

#define XINC_LOG_INTERNAL_HEXDUMP_INST_INFO(p_inst, p_data, len) \
        XINC_LOG_INTERNAL_HEXDUMP_INST(XINC_LOG_SEVERITY_INFO, XINC_LOG_SEVERITY_INFO, p_inst, p_data, len)

#define XINC_LOG_INTERNAL_HEXDUMP_INFO(p_data, len) \
        XINC_LOG_INTERNAL_HEXDUMP_MODULE(XINC_LOG_SEVERITY_INFO, XINC_LOG_SEVERITY_INFO, p_data, len)

#define XINC_LOG_INTERNAL_RAW_INFO(...) \
        XINC_LOG_INTERNAL_MODULE(XINC_LOG_SEVERITY_INFO, XINC_LOG_SEVERITY_INFO_RAW, __VA_ARGS__)

#define XINC_LOG_INTERNAL_RAW_HEXDUMP_INFO(p_data, len) \
        XINC_LOG_INTERNAL_HEXDUMP_MODULE(XINC_LOG_SEVERITY_INFO, XINC_LOG_SEVERITY_INFO_RAW, p_data, len)

#define XINC_LOG_INTERNAL_INST_DEBUG(p_inst, ...) \
        XINC_LOG_INTERNAL_INST(XINC_LOG_SEVERITY_DEBUG, XINC_LOG_SEVERITY_DEBUG, p_inst, __VA_ARGS__)

#define XINC_LOG_INTERNAL_DEBUG(...) \
        XINC_LOG_INTERNAL_MODULE(XINC_LOG_SEVERITY_DEBUG, XINC_LOG_SEVERITY_DEBUG, __VA_ARGS__)

#define XINC_LOG_INTERNAL_HEXDUMP_INST_DEBUG(p_inst, p_data, len) \
        XINC_LOG_INTERNAL_HEXDUMP_INST(XINC_LOG_SEVERITY_DEBUG, XINC_LOG_SEVERITY_DEBUG, p_inst, p_data, len)

#define XINC_LOG_INTERNAL_HEXDUMP_DEBUG(p_data, len) \
        XINC_LOG_INTERNAL_HEXDUMP_MODULE(XINC_LOG_SEVERITY_DEBUG, XINC_LOG_SEVERITY_DEBUG, p_data, len)


#if XINC_LOG_ENABLED

#ifdef UNIT_TEST
#define COMPILED_LOG_LEVEL 4
#else
#define COMPILED_LOG_LEVEL XINC_LOG_LEVEL
#endif


#define XINC_LOG_INTERNAL_MODULE_REGISTER() \
                   XINC_LOG_INTERNAL_ITEM_REGISTER(XINC_LOG_MODULE_NAME,                 \
                                                  STRINGIFY(XINC_LOG_MODULE_NAME),      \
                                                  XINC_LOG_INFO_COLOR,                  \
                                                  XINC_LOG_DEBUG_COLOR,                 \
                                                  XINC_LOG_INITIAL_LEVEL,               \
                                                  COMPILED_LOG_LEVEL)

#else
#define XINC_LOG_INTERNAL_MODULE_REGISTER() /*lint -save -e19*/ /*lint -restore*/
#endif

extern xinc_log_module_dynamic_data_t XINC_LOG_ITEM_DATA_DYNAMIC(XINC_LOG_MODULE_NAME);
extern _CONST xinc_log_module_const_data_t XINC_LOG_ITEM_DATA_CONST(XINC_LOG_MODULE_NAME);

/**
 * Set of macros for encoding and decoding header for log entries.
 * There are 2 types of entries:
 * 1. Standard entry (STD)
 *    An entry consists of header, pointer to string and values. Header contains
 *    severity leveland determines number of arguments and thus size of the entry.
 *    Since flash address space starts from 0x00000000 and is limited to kB rather
 *    than MB 22 bits are used to store the address (4MB). It is used that way to
 *    save one RAM memory.
 *
 *    --------------------------------
 *    |TYPE|SEVERITY|NARGS|    P_STR |
 *    |------------------------------|
 *    |    Module_ID (optional)      |
 *    |------------------------------|
 *    |    TIMESTAMP (optional)      |
 *    |------------------------------|
 *    |             ARG0             |
 *    |------------------------------|
 *    |             ....             |
 *    |------------------------------|
 *    |             ARG(nargs-1)     |
 *    --------------------------------
 *
 * 2. Hexdump entry (HEXDUMP) is used for dumping raw data. An entry consists of
 *    header, optional timestamp, pointer to string and data. A header contains
 *    length (10bit) and offset which is updated after backend processes part of
 *    data.
 *
 *    --------------------------------
 *    |TYPE|SEVERITY|NARGS|OFFSET|LEN|
 *    |------------------------------|
 *    |    Module_ID (optional)      |
 *    |------------------------------|
 *    |    TIMESTAMP (optional)      |
 *    |------------------------------|
 *    |           P_STR              |
 *    |------------------------------|
 *    |             data             |
 *    |------------------------------|
 *    |  data |       dummy          |
 *    --------------------------------
 *
 */

#define STD_ADDR_MASK       ((uint32_t)(1U << 22) - 1U)
#define HEADER_TYPE_STD     1U
#define HEADER_TYPE_HEXDUMP 2U
#define HEADER_TYPE_INVALID 3U
#if 0
typedef struct
{
    uint32_t type       : 2;
    uint32_t in_progress: 1;
    uint32_t data       : 29;
} xinc_log_generic_header_t;

typedef struct
{
    uint32_t type       : 2;
    uint32_t in_progress: 1;
    uint32_t severity   : 3;
    uint32_t nargs      : 4;
    uint32_t addr       : 22;
} xinc_log_std_header_t;

typedef struct
{
    uint32_t type       : 2;
    uint32_t in_progress: 1;
    uint32_t severity   : 3;
    uint32_t offset     : 10;
    uint32_t reserved   : 6;
    uint32_t len        : 10;
} xinc_log_hexdump_header_t;
#else
typedef struct
{
    uint32_t type       ;
    uint32_t in_progress;
    uint32_t data       ;
} xinc_log_generic_header_t;

typedef struct
{
    uint32_t type       ;
    uint32_t in_progress;
    uint32_t severity   ;
    uint32_t nargs      ;
    uint32_t addr       ;
} xinc_log_std_header_t;

typedef struct
{
    uint32_t type       ;
    uint32_t in_progress;
    uint32_t severity   ;
    uint32_t offset     ;
    uint32_t reserved   ;
    uint32_t len        ;
} xinc_log_hexdump_header_t;

#endif
typedef union
{
    xinc_log_generic_header_t generic;
    xinc_log_std_header_t     std;
    xinc_log_hexdump_header_t hexdump;
    uint32_t                 raw;
} xinc_log_main_header_t;

typedef struct
{
    xinc_log_main_header_t base;
    uint16_t module_id;
    uint16_t dropped;
    uint32_t timestamp;
} xinc_log_header_t;

#define HEADER_SIZE         (sizeof(xinc_log_header_t)/sizeof(uint32_t) - \
                (XINC_LOG_USES_TIMESTAMP ? 0 : 1))

/**
 * @brief A function for logging raw string.
 *
 * @param severity_mid Severity.
 * @param p_str        A pointer to a string.
 */
void xinc_log_frontend_std_0(uint32_t severity_mid, char const * const p_str);

/**
 * @brief A function for logging a formatted string with one argument.
 *
 * @param severity_mid  Severity.
 * @param p_str         A pointer to a formatted string.
 * @param val0          An argument.
 */
void xinc_log_frontend_std_1(uint32_t           severity_mid,
                            char const * const p_str,
                            uint32_t           val0);

/**
 * @brief A function for logging a formatted string with 2 arguments.
 *
 * @param severity_mid   Severity.
 * @param p_str          A pointer to a formatted string.
 * @param val0, val1     Arguments for formatting string.
 */
void xinc_log_frontend_std_2(uint32_t           severity_mid,
                            char const * const p_str,
                            uint32_t           val0,
                            uint32_t           val1);

/**
 * @brief A function for logging a formatted string with 3 arguments.
 *
 * @param severity_mid     Severity.
 * @param p_str            A pointer to a formatted string.
 * @param val0, val1, val2 Arguments for formatting string.
 */
void xinc_log_frontend_std_3(uint32_t           severity_mid,
                            char const * const p_str,
                            uint32_t           val0,
                            uint32_t           val1,
                            uint32_t           val2);

/**
 * @brief A function for logging a formatted string with 4 arguments.
 *
 * @param severity_mid           Severity.
 * @param p_str                  A pointer to a formatted string.
 * @param val0, val1, val2, val3 Arguments for formatting string.
 */
void xinc_log_frontend_std_4(uint32_t           severity_mid,
                            char const * const p_str,
                            uint32_t           val0,
                            uint32_t           val1,
                            uint32_t           val2,
                            uint32_t           val3);

/**
 * @brief A function for logging a formatted string with 5 arguments.
 *
 * @param severity_mid                 Severity.
 * @param p_str                        A pointer to a formatted string.
 * @param val0, val1, val2, val3, val4 Arguments for formatting string.
 */
void xinc_log_frontend_std_5(uint32_t           severity_mid,
                            char const * const p_str,
                            uint32_t           val0,
                            uint32_t           val1,
                            uint32_t           val2,
                            uint32_t           val3,
                            uint32_t           val4);

/**
 * @brief A function for logging a formatted string with 6 arguments.
 *
 * @param severity_mid                       Severity.
 * @param p_str                              A pointer to a formatted string.
 * @param val0, val1, val2, val3, val4, val5 Arguments for formatting string.
 */
void xinc_log_frontend_std_6(uint32_t           severity_mid,
                            char const * const p_str,
                            uint32_t           val0,
                            uint32_t           val1,
                            uint32_t           val2,
                            uint32_t           val3,
                            uint32_t           val4,
                            uint32_t           val5);

/**
 * @brief A function for logging raw data.
 *
 * @param severity_mid Severity.
 * @param p_str        A pointer to a string which is prefixing the data.
 * @param p_data       A pointer to data to be dumped.
 * @param length       Length of data (in bytes).
 *
 */
void xinc_log_frontend_hexdump(uint32_t           severity_mid,
                              const void * const p_data,
                              uint16_t           length);

/**
 * @brief A function for reading a byte from log backend.
 *
 * @return Byte.
 */
uint8_t xinc_log_getchar(void);
#endif // XINC_LOG_INTERNAL_H__
