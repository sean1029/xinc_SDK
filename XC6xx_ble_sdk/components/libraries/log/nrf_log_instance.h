/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_LOG_INSTANCE_H
#define XINC_LOG_INSTANCE_H

#include "sdk_config.h"
#include "nrf_section.h"
#include "nrf_log_types.h"
#include <stdint.h>


/*
 * For GCC, sections are sorted in the group by the linker. For IAR and KEIL, it is assumed that linker will sort
 * dynamic and const section in the same order (but in different locations). Proper message formatting
 * is based on that assumption.
 */
#if defined(__GNUC__)
#define XINC_LOG_DYNAMIC_SECTION_NAME(_module_name) CONCAT_2(log_dynamic_data_,_module_name)
#define XINC_LOG_FILTER_SECTION_NAME(_module_name) CONCAT_2(log_filter_data_,_module_name)
#define XINC_LOG_CONST_SECTION_NAME(_module_name) CONCAT_2(log_const_data_,_module_name)
#else
#define XINC_LOG_DYNAMIC_SECTION_NAME(_module_name) log_dynamic_data
#define XINC_LOG_FILTER_SECTION_NAME(_module_name) log_filter_data
#define XINC_LOG_CONST_SECTION_NAME(_module_name)   log_const_data
#endif

#define XINC_LOG_ITEM_DATA(_name)         CONCAT_3(m_nrf_log_,_name,_logs_data)
#define XINC_LOG_ITEM_DATA_DYNAMIC(_name) CONCAT_2(XINC_LOG_ITEM_DATA(_name),_dynamic)
#define XINC_LOG_ITEM_DATA_FILTER(_name)  CONCAT_2(XINC_LOG_ITEM_DATA(_name),_filter)
#define XINC_LOG_ITEM_DATA_CONST(_name)   CONCAT_2(XINC_LOG_ITEM_DATA(_name),_const)

#ifdef UNIT_TEST
#define _CONST
#else
#define _CONST const
#endif

/*lint -save -esym(526,log_const_data*) -esym(526,log_dynamic_data*)*/
XINC_SECTION_DEF(log_dynamic_data, nrf_log_module_dynamic_data_t);
XINC_SECTION_DEF(log_filter_data, nrf_log_module_filter_data_t);
XINC_SECTION_DEF(log_const_data, nrf_log_module_const_data_t);
/*lint -restore*/

//#define XINC_LOG_INTERNAL_CONST_ITEM_REGISTER(                                             \
//            _name, _str_name, _info_color, _debug_color, _initial_lvl, _compiled_lvl)     \
//            XINC_SECTION_ITEM_REGISTER(XINC_LOG_CONST_SECTION_NAME(_name),                  \
//            _CONST nrf_log_module_const_data_t XINC_LOG_ITEM_DATA_CONST(_name)) = {        \
//                .p_module_name   = _str_name,                                             \
//                .info_color_id   = (_info_color),                                         \
//                .debug_color_id  = (_debug_color),                                        \
//                .compiled_lvl    = (nrf_log_severity_t)(_compiled_lvl),                   \
//                .initial_lvl     = (nrf_log_severity_t)(_initial_lvl),                    \
//            }

#define XINC_LOG_INTERNAL_CONST_ITEM_REGISTER(                                             \
            _name, _str_name, _info_color, _debug_color, _initial_lvl, _compiled_lvl)     \
            XINC_SECTION_ITEM_REGISTER(XINC_LOG_CONST_SECTION_NAME(_name),                  \
            _CONST nrf_log_module_const_data_t XINC_LOG_ITEM_DATA_CONST(_name)) = {        \
                .p_module_name   = _str_name,                                             \
                .info_color_id   = (_info_color),                                         \
                .debug_color_id  = (_debug_color),                                        \
                .compiled_lvl    = (nrf_log_severity_t)(_compiled_lvl),                   \
                .initial_lvl     = (nrf_log_severity_t)(_initial_lvl),                    \
            }

#if XINC_LOG_FILTERS_ENABLED
#define XINC_LOG_INTERNAL_ITEM_REGISTER( \
                         _name, _str_name, _info_color, _debug_color, _initial_lvl, _compiled_lvl) \
    XINC_LOG_INTERNAL_CONST_ITEM_REGISTER(_name,                                                    \
                                         _str_name,                                                \
                                         _info_color,                                              \
                                         _debug_color,                                             \
                                         _initial_lvl,                                             \
                                         _compiled_lvl);                                           \
    XINC_SECTION_ITEM_REGISTER(XINC_LOG_DYNAMIC_SECTION_NAME(_name),                                 \
                    nrf_log_module_dynamic_data_t XINC_LOG_ITEM_DATA_DYNAMIC(_name));               \
    XINC_SECTION_ITEM_REGISTER(XINC_LOG_FILTER_SECTION_NAME(_name),                                  \
                          nrf_log_module_filter_data_t XINC_LOG_ITEM_DATA_FILTER(_name))

#else
#define XINC_LOG_INTERNAL_ITEM_REGISTER( \
                         _name, _str_name, _info_color, _debug_color, _initial_lvl, _compiled_lvl) \
    XINC_LOG_INTERNAL_CONST_ITEM_REGISTER(_name,                                                    \
                                         _str_name,                                                \
                                         _info_color,                                              \
                                         _debug_color,                                             \
                                         _initial_lvl,                                             \
                                         _compiled_lvl)

#endif
/**@file
 *
 * @defgroup nrf_log_instance Macros for logging on instance level
 * @{
 * @ingroup nrf_log
 *
 * @brief Macros for logging on instance level
 */

/** @def XINC_LOG_INSTANCE_PTR_DECLARE
 * @brief Macro for declaring a logger instance pointer in the module stucture.
 */

/** @def XINC_LOG_INSTANCE_REGISTER
 * @brief Macro for creating an independent module instance.
 *
 * Module instance provides filtering of logs on instance level instead of module level.
 */

/** @def XINC_LOG_INSTANCE_PTR_INIT
 * @brief Macro for initializing a pointer to the logger instance.
 */
 
 
 /** @} */
#if XINC_LOG_ENABLED && XINC_LOG_FILTERS_ENABLED
#define XINC_LOG_INSTANCE_PTR_DECLARE(_p_name) nrf_log_module_dynamic_data_t * _p_name;

#define XINC_LOG_INSTANCE_REGISTER( \
               _module_name, _inst_name, _info_color, _debug_color, _initial_lvl, _compiled_lvl)   \
               XINC_LOG_INTERNAL_ITEM_REGISTER(CONCAT_3(_module_name,_,_inst_name),                 \
                                              STRINGIFY(_module_name._inst_name),                  \
                                              _info_color,                                         \
                                              _debug_color,                                        \
                                              _initial_lvl,                                        \
                                              _compiled_lvl)

#define XINC_LOG_INSTANCE_PTR_INIT(_p_name, _module_name, _inst_name) \
                       ._p_name = &XINC_LOG_ITEM_DATA_DYNAMIC(CONCAT_3(_module_name,_,_inst_name)),

#else
#define XINC_LOG_INSTANCE_PTR_DECLARE(_p_name)
#define XINC_LOG_INSTANCE_REGISTER(_module_name, _inst_name, info_color, debug_color, _initial_lvl, compiled_lvl)
#define XINC_LOG_INSTANCE_PTR_INIT(_p_name, _module_name, _inst_name)
#endif

#endif //XINC_LOG_INSTANCE_H
