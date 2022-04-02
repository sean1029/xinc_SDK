/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "app_util_platform.h"

#ifdef SOFTDEVICE_PRESENT
/* Global nvic state instance, required by xinc_nvic.h */
xinc_nvic_state_t xinc_nvic_state;
#endif

static uint32_t m_in_critical_region = 0;

void app_util_disable_irq(void)
{
    __disable_irq();
    m_in_critical_region++;
}

void app_util_enable_irq(void)
{
    m_in_critical_region--;
    if (m_in_critical_region == 0)
    {
        __enable_irq();
    }
}

void app_util_critical_region_enter(uint8_t *p_nested)
{
#if __CORTEX_M == (0x04U)
    ASSERT(APP_LEVEL_PRIVILEGED == privilege_level_get())
#endif

    app_util_disable_irq();

}

void app_util_critical_region_exit(uint8_t nested)
{
#if __CORTEX_M == (0x04U)
    ASSERT(APP_LEVEL_PRIVILEGED == privilege_level_get())
#endif

    app_util_enable_irq();

}


uint8_t privilege_level_get(void)
{
#if __CORTEX_M == (0x00U) || defined(_WIN32) || defined(__unix) || defined(__APPLE__)
    /* the Cortex-M0 has no concept of privilege */
    return APP_LEVEL_PRIVILEGED;
#elif __CORTEX_M >= (0x04U)
    uint32_t isr_vector_num = __get_IPSR() & IPSR_ISR_Msk ;
    if (0 == isr_vector_num)
    {
        /* Thread Mode, check nPRIV */
        int32_t control = __get_CONTROL();
        return control & CONTROL_nPRIV_Msk ? APP_LEVEL_UNPRIVILEGED : APP_LEVEL_PRIVILEGED;
    }
    else
    {
        /* Handler Mode, always privileged */
        return APP_LEVEL_PRIVILEGED;
    }
#endif
}


uint8_t current_int_priority_get(void)
{
    uint32_t isr_vector_num = __get_IPSR() & IPSR_ISR_Msk ;
    if (isr_vector_num > 0)
    {
        int32_t irq_type = ((int32_t)isr_vector_num - EXTERNAL_INT_VECTOR_OFFSET);
        return  (NVIC_GetPriority((IRQn_Type)irq_type) & 0xFF);
    }
    else
    {
        return APP_IRQ_PRIORITY_THREAD;
    }
}
