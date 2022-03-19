/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "xinc_atomic.h"

#ifndef XINC_ATOMIC_USE_BUILD_IN
#if (defined(__GNUC__) && defined(WIN32))
    #define XINC_ATOMIC_USE_BUILD_IN 1
#else
    #define XINC_ATOMIC_USE_BUILD_IN 0
#endif
#endif // XINC_ATOMIC_USE_BUILD_IN

#if ((__CORTEX_M >= 0x03U) || (__CORTEX_SC >= 300U))
#define STREX_LDREX_PRESENT
#else
#include "app_util_platform.h"
#endif


#if (XINC_ATOMIC_USE_BUILD_IN == 0) && defined(STREX_LDREX_PRESENT)
#include "xinc_atomic_internal.h"
#endif

uint32_t xinc_atomic_u32_fetch_store(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    return __atomic_exchange_n(p_data, value, __ATOMIC_SEQ_CST);

#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;
    XINC_ATOMIC_OP(mov, old_val, new_val, p_data, value);

    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return old_val;
#else
    CRITICAL_REGION_ENTER();
    uint32_t old_val = *p_data;
    *p_data = value;
    CRITICAL_REGION_EXIT();
    return old_val;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_u32_store(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    __atomic_store_n(p_data, value, __ATOMIC_SEQ_CST);
    return value;
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(mov, old_val, new_val, p_data, value);

    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return new_val;
#else
    CRITICAL_REGION_ENTER();
    *p_data = value;
    CRITICAL_REGION_EXIT();
    return value;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_u32_fetch_or(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    return __atomic_fetch_or(p_data, value, __ATOMIC_SEQ_CST);
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(orr, old_val, new_val, p_data, value);
    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return old_val;
#else
    CRITICAL_REGION_ENTER();
    uint32_t old_val = *p_data;
    *p_data |= value;
    CRITICAL_REGION_EXIT();
    return old_val;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_u32_or(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    return __atomic_or_fetch(p_data, value, __ATOMIC_SEQ_CST);
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(orr, old_val, new_val, p_data, value);
    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return new_val;
#else
    CRITICAL_REGION_ENTER();
    *p_data |= value;
    uint32_t new_value = *p_data;
    CRITICAL_REGION_EXIT();
    return new_value;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_u32_fetch_and(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    return __atomic_fetch_and(p_data, value, __ATOMIC_SEQ_CST);
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(and, old_val, new_val, p_data, value);
    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return old_val;
#else
    CRITICAL_REGION_ENTER();
    uint32_t old_val = *p_data;
    *p_data &= value;
    CRITICAL_REGION_EXIT();
    return old_val;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_u32_and(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    return __atomic_and_fetch(p_data, value, __ATOMIC_SEQ_CST);
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(and, old_val, new_val, p_data, value);
    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return new_val;
#else
    CRITICAL_REGION_ENTER();
    *p_data &= value;
    uint32_t new_value = *p_data;
    CRITICAL_REGION_EXIT();
    return new_value;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_u32_fetch_xor(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    return __atomic_fetch_xor(p_data, value, __ATOMIC_SEQ_CST);
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(eor, old_val, new_val, p_data, value);
    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return old_val;
#else
    CRITICAL_REGION_ENTER();
    uint32_t old_val = *p_data;
    *p_data ^= value;
    CRITICAL_REGION_EXIT();
    return old_val;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_u32_xor(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    return __atomic_xor_fetch(p_data, value, __ATOMIC_SEQ_CST);
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(eor, old_val, new_val, p_data, value);
    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return new_val;
#else
    CRITICAL_REGION_ENTER();
    *p_data ^= value;
    uint32_t new_value = *p_data;
    CRITICAL_REGION_EXIT();
    return new_value;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_u32_fetch_add(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    return __atomic_fetch_add(p_data, value, __ATOMIC_SEQ_CST);
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(add, old_val, new_val, p_data, value);
    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return old_val;
#else
    CRITICAL_REGION_ENTER();
    uint32_t old_val = *p_data;
    *p_data += value;
    CRITICAL_REGION_EXIT();
    return old_val;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_u32_add(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    return __atomic_add_fetch(p_data, value, __ATOMIC_SEQ_CST);
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(add, old_val, new_val, p_data, value);
    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return new_val;
#else
    CRITICAL_REGION_ENTER();
    *p_data += value;
    uint32_t new_value = *p_data;
    CRITICAL_REGION_EXIT();
    return new_value;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_u32_fetch_sub(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    return __atomic_fetch_sub(p_data, value, __ATOMIC_SEQ_CST);
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(sub, old_val, new_val, p_data, value);
    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return old_val;
#else
    CRITICAL_REGION_ENTER();
    uint32_t old_val = *p_data;
    *p_data -= value;
    CRITICAL_REGION_EXIT();
    return old_val;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_u32_sub(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    return __atomic_sub_fetch(p_data, value, __ATOMIC_SEQ_CST);
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(sub, old_val, new_val, p_data, value);
    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return new_val;
#else
    CRITICAL_REGION_ENTER();
    *p_data -= value;
    uint32_t new_value = *p_data;
    CRITICAL_REGION_EXIT();
    return new_value;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

bool xinc_atomic_u32_cmp_exch(xinc_atomic_u32_t * p_data,
                                           uint32_t *         p_expected,
                                           uint32_t           desired)
{
#if XINC_ATOMIC_USE_BUILD_IN
    return __atomic_compare_exchange(p_data,
                                     p_expected,
                                     &desired,
                                     1,
                                     __ATOMIC_SEQ_CST,
                                     __ATOMIC_SEQ_CST);
#elif defined(STREX_LDREX_PRESENT)
    return xinc_atomic_internal_cmp_exch(p_data, p_expected, desired);
#else
    bool ret;
    CRITICAL_REGION_ENTER();
    if (*p_data == *p_expected)
    {
        *p_data = desired;
        ret = true;
    }
    else
    {
        *p_expected = *p_data;
        ret = false;
    }
    CRITICAL_REGION_EXIT();
    return ret;
#endif
}

uint32_t xinc_atomic_u32_fetch_sub_hs(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    uint32_t expected = *p_data;
    uint32_t new_val;
    bool     success;

    do
    {
        if (expected >= value)
        {
            new_val = expected - value;
        }
        else
        {
            new_val = expected;
        }
        success = __atomic_compare_exchange(p_data,
                                            &expected,
                                            &new_val,
                                            1,
                                            __ATOMIC_SEQ_CST,
                                            __ATOMIC_SEQ_CST);
    } while(!success);
    return expected;
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(sub_hs, old_val, new_val, p_data, value);
    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return old_val;
#else
    CRITICAL_REGION_ENTER();
    uint32_t old_val = *p_data;
    *p_data -= value;
    CRITICAL_REGION_EXIT();
    return old_val;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_u32_sub_hs(xinc_atomic_u32_t * p_data, uint32_t value)
{
#if XINC_ATOMIC_USE_BUILD_IN
    uint32_t expected = *p_data;
    uint32_t new_val;
    bool     success;

    do
    {
        if (expected >= value)
        {
            new_val = expected - value;
        }
        else
        {
            new_val = expected;
        }
        success = __atomic_compare_exchange(p_data,
                                            &expected,
                                            &new_val,
                                            1,
                                            __ATOMIC_SEQ_CST,
                                            __ATOMIC_SEQ_CST);
    } while(!success);
    return new_val;
#elif defined(STREX_LDREX_PRESENT)
    uint32_t old_val;
    uint32_t new_val;

    XINC_ATOMIC_OP(sub_hs, old_val, new_val, p_data, value);
    UNUSED_PARAMETER(old_val);
    UNUSED_PARAMETER(new_val);
    return new_val;
#else
    CRITICAL_REGION_ENTER();
    *p_data -= value;
    uint32_t new_value = *p_data;
    CRITICAL_REGION_EXIT();
    return new_value;
#endif //XINC_ATOMIC_USE_BUILD_IN
}

uint32_t xinc_atomic_flag_set_fetch(xinc_atomic_flag_t * p_data)
{
    return xinc_atomic_u32_fetch_or(p_data, 1);
}

uint32_t xinc_atomic_flag_set(xinc_atomic_flag_t * p_data)
{
    return xinc_atomic_u32_or(p_data, 1);
}

uint32_t xinc_atomic_flag_clear_fetch(xinc_atomic_flag_t * p_data)
{
    return xinc_atomic_u32_fetch_and(p_data, 0);
}

uint32_t xinc_atomic_flag_clear(xinc_atomic_flag_t * p_data)
{
    return xinc_atomic_u32_and(p_data, 0);
}

