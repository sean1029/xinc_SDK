/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
/** @cond */
/**@file
 *
 * @defgroup sdk_os SDK OS Abstraction
 * @ingroup experimental_api
 * @details In order to made SDK modules independent of use of an embedded OS, and permit
 *          application with varied task architecture, SDK abstracts the OS specific
 *          elements here in order to make all other modules agnostic to the OS or task
 *          architecture.
 * @{
 */

#ifndef SDK_OS_H__
#define SDK_OS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SDK_MUTEX_DEFINE(X)
#define SDK_MUTEX_INIT(X)
#define SDK_MUTEX_LOCK(X)
#define SDK_MUTEX_UNLOCK(X)

/**
 * @defgroup os_data_type Data types.
 */

/** @} */
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif // SDK_OS_H__

