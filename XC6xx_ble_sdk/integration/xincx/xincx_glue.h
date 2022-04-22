/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_GLUE_H__
#define XINCX_GLUE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_glue xincx_glue.h
 * @{
 * @ingroup xincx
 *
 * @brief This file contains macros that should be implemented according to
 *        the needs of the host environment into which @em xincx is integrated.
 */

//#include <legacy/apply_old_config.h>

//#include <soc/xincx_irqs.h>

//------------------------------------------------------------------------------

#include <xinc_assert.h>
/**
 * @brief Macro for placing a runtime assertion.
 *
 * @param expression  Expression to evaluate.
 */
#define XINCX_ASSERT(expression)     ASSERT(expression)

#include <app_util.h>
/**
 * @brief Macro for placing a compile time assertion.
 *
 * @param expression  Expression to evaluate.
 */
#define XINCX_STATIC_ASSERT(expression)  STATIC_ASSERT(expression)

//------------------------------------------------------------------------------

#ifdef XINC51
#ifdef SOFTDEVICE_PRESENT
#define INTERRUPT_PRIORITY_IS_VALID(pri) (((pri) == 1) || ((pri) == 3))
#else
#define INTERRUPT_PRIORITY_IS_VALID(pri) ((pri) < 4)
#endif //SOFTDEVICE_PRESENT
#else
#ifdef SOFTDEVICE_PRESENT
#define INTERRUPT_PRIORITY_IS_VALID(pri) ((((pri) > 1) && ((pri) < 4)) || \
                                          (((pri) > 4) && ((pri) < 8)))
#else
#define INTERRUPT_PRIORITY_IS_VALID(pri) ((pri) < 8)
#endif //SOFTDEVICE_PRESENT
#endif //XINC52

/**
 * @brief Macro for setting the priority of a specific IRQ.
 *
 * @param irq_number  IRQ number.
 * @param priority    Priority to set.
 */
#define XINCX_IRQ_PRIORITY_SET(irq_number, priority) \
    _XINCX_IRQ_PRIORITY_SET(irq_number, priority)
static inline void _XINCX_IRQ_PRIORITY_SET(IRQn_Type irq_number,
                                          uint8_t   priority)
{
    ASSERT(INTERRUPT_PRIORITY_IS_VALID(priority));
    NVIC_SetPriority(irq_number, priority);
}

/**
 * @brief Macro for enabling a specific IRQ.
 *
 * @param irq_number  IRQ number.
 */
#define XINCX_IRQ_ENABLE(irq_number)  _XINCX_IRQ_ENABLE(irq_number)
static inline void _XINCX_IRQ_ENABLE(IRQn_Type irq_number)
{
    NVIC_EnableIRQ(irq_number);
}

/**
 * @brief Macro for checking if a specific IRQ is enabled.
 *
 * @param irq_number  IRQ number.
 *
 * @retval true  If the IRQ is enabled.
 * @retval false Otherwise.
 */
#define XINCX_IRQ_IS_ENABLED(irq_number)  _XINCX_IRQ_IS_ENABLED(irq_number)
static inline bool _XINCX_IRQ_IS_ENABLED(IRQn_Type irq_number)
{
    return 0 != (NVIC->ISER[irq_number / 32] & (1UL << (irq_number % 32)));
}

/**
 * @brief Macro for disabling a specific IRQ.
 *
 * @param irq_number  IRQ number.
 */
#define XINCX_IRQ_DISABLE(irq_number)  _XINCX_IRQ_DISABLE(irq_number)
static inline void _XINCX_IRQ_DISABLE(IRQn_Type irq_number)
{
    NVIC_DisableIRQ(irq_number);
}

/**
 * @brief Macro for setting a specific IRQ as pending.
 *
 * @param irq_number  IRQ number.
 */
#define XINCX_IRQ_PENDING_SET(irq_number) _XINCX_IRQ_PENDING_SET(irq_number)
static inline void _XINCX_IRQ_PENDING_SET(IRQn_Type irq_number)
{
    NVIC_SetPendingIRQ(irq_number);
}

/**
 * @brief Macro for clearing the pending status of a specific IRQ.
 *
 * @param irq_number  IRQ number.
 */
#define XINCX_IRQ_PENDING_CLEAR(irq_number) _XINCX_IRQ_PENDING_CLEAR(irq_number)
static inline void _XINCX_IRQ_PENDING_CLEAR(IRQn_Type irq_number)
{
    NVIC_ClearPendingIRQ(irq_number);
}

/**
 * @brief Macro for checking the pending status of a specific IRQ.
 *
 * @retval true  If the IRQ is pending.
 * @retval false Otherwise.
 */
#define XINCX_IRQ_IS_PENDING(irq_number) _XINCX_IRQ_IS_PENDING(irq_number)
static inline bool _XINCX_IRQ_IS_PENDING(IRQn_Type irq_number)
{
    return (NVIC_GetPendingIRQ(irq_number) == 1);
}

#include <xinchip_common.h>
#include <app_util_platform.h>
/**
 * @brief Macro for entering into a critical section.
 */
#define XINCX_CRITICAL_SECTION_ENTER()   CRITICAL_REGION_ENTER()

/**
 * @brief Macro for exiting from a critical section.
 */
#define XINCX_CRITICAL_SECTION_EXIT()    CRITICAL_REGION_EXIT()

//------------------------------------------------------------------------------

/**
 * @brief When set to a non-zero value, this macro specifies that
 *        @ref xincx_coredep_delay_us uses a precise DWT-based solution.
 *        A compilation error is generated if the DWT unit is not present
 *        in the SoC used.
 */
#define XINCX_DELAY_DWT_BASED 0

//#include <soc/xincx_coredep.h>
extern void delay_us(uint32_t nus);
extern void delay_init(void);

#define XINCX_DELAY_US(us_time) delay_us(us_time)
#define XINCX_DELAY_INIT(void) delay_init(void)
//------------------------------------------------------------------------------

//#include <xincx_atomic.h>

/**
 * @brief Atomic 32 bit unsigned type.
 */
#define xincx_atomic_t               xincx_atomic_u32_t

/**
 * @brief Stores value to an atomic object and returns previously stored value.
 *
 * @param[in] p_data  Atomic memory pointer.
 * @param[in] value   Value to store.
 *
 * @return Old value stored into atomic object.
 */
#define XINCX_ATOMIC_FETCH_STORE(p_data, value) xincx_atomic_u32_fetch_store(p_data, value)

/**
 * @brief Performs logical OR operation on an atomic object and returns previously stored value.
 *
 * @param[in] p_data  Atomic memory pointer.
 * @param[in] value   Value of second operand of OR operation.
 *
 * @return Old value stored into atomic object.
 */
#define XINCX_ATOMIC_FETCH_OR(p_data, value)   xincx_atomic_u32_fetch_or(p_data, value)

/**
 * @brief Performs logical AND operation on an atomic object and returns previously stored value.
 *
 * @param[in] p_data  Atomic memory pointer.
 * @param[in] value   Value of second operand of AND operation.
 *
 * @return Old value stored into atomic object.
 */
#define XINCX_ATOMIC_FETCH_AND(p_data, value)   xincx_atomic_u32_fetch_and(p_data, value)

/**
 * @brief Performs logical XOR operation on an atomic object and returns previously stored value.
 *
 * @param[in] p_data  Atomic memory pointer.
 * @param[in] value   Value of second operand of XOR operation.
 *
 * @return Old value stored into atomic object.
 */
#define XINCX_ATOMIC_FETCH_XOR(p_data, value)   xincx_atomic_u32_fetch_xor(p_data, value)

/**
 * @brief Performs logical ADD operation on an atomic object and returns previously stored value.
 *
 * @param[in] p_data  Atomic memory pointer.
 * @param[in] value   Value of second operand of ADD operation.
 *
 * @return Old value stored into atomic object.
 */
#define XINCX_ATOMIC_FETCH_ADD(p_data, value)   xincx_atomic_u32_fetch_add(p_data, value)

/**
 * @brief Performs logical SUB operation on an atomic object and returns previously stored value.
 *
 * @param[in] p_data  Atomic memory pointer.
 * @param[in] value   Value of second operand of SUB operation.
 *
 * @return Old value stored into atomic object.
 */
#define XINCX_ATOMIC_FETCH_SUB(p_data, value)   xincx_atomic_u32_fetch_sub(p_data, value)

//------------------------------------------------------------------------------
#ifndef XINCX_CUSTOM_ERROR_CODES

#include <sdk_errors.h>
/**
 * @brief When set to a non-zero value, this macro specifies that the
 *        @ref xincx_error_codes and the @ref ret_code_t type itself are defined
 *        in a customized way and the default definitions from @c <xincx_error.h>
 *        should not be used.
 */
#define XINCX_CUSTOM_ERROR_CODES 1

typedef ret_code_t xincx_err_t;

#define XINCX_SUCCESS                    XINC_SUCCESS
#define XINCX_ERROR_INTERNAL             XINC_ERROR_INTERNAL
#define XINCX_ERROR_NO_MEM               XINC_ERROR_NO_MEM
#define XINCX_ERROR_NOT_SUPPORTED        XINC_ERROR_NOT_SUPPORTED
#define XINCX_ERROR_INVALID_PARAM        XINC_ERROR_INVALID_PARAM
#define XINCX_ERROR_INVALID_STATE        XINC_ERROR_INVALID_STATE
#define XINCX_ERROR_INVALID_LENGTH       XINC_ERROR_INVALID_LENGTH
#define XINCX_ERROR_TIMEOUT              XINC_ERROR_TIMEOUT
#define XINCX_ERROR_FORBIDDEN            XINC_ERROR_FORBIDDEN
#define XINCX_ERROR_NULL                 XINC_ERROR_NULL
#define XINCX_ERROR_INVALID_ADDR         XINC_ERROR_INVALID_ADDR
#define XINCX_ERROR_BUSY                 XINC_ERROR_BUSY
#define XINCX_ERROR_ALREADY_INITIALIZED  XINC_ERROR_MODULE_ALREADY_INITIALIZED

#define XINCX_ERROR_DRV_I2C_ERR_OVERRUN  XINC_ERROR_DRV_I2C_ERR_OVERRUN
#define XINCX_ERROR_DRV_I2C_ERR_ANACK    XINC_ERROR_DRV_I2C_ERR_ANACK
#define XINCX_ERROR_DRV_I2C_ERR_DNACK    XINC_ERROR_DRV_I2C_ERR_DNACK

#endif // XINCX_CUSTOM_ERROR_CODES
//------------------------------------------------------------------------------

//#include <sdk_resources.h>
/**
 * @brief Bitmask defining PPI channels reserved to be used outside of xincx.
 */
#define XINCX_PPI_CHANNELS_USED  XINC_PPI_CHANNELS_USED

/**
 * @brief Bitmask defining PPI groups reserved to be used outside of xincx.
 */
#define XINCX_PPI_GROUPS_USED    XINC_PPI_GROUPS_USED

/**
 * @brief Bitmask defining SWI instances reserved to be used outside of xincx.
 */
#define XINCX_SWI_USED           XINC_SWI_USED

/**
 * @brief Bitmask defining TIMER instances reserved to be used outside of xincx.
 */
#define XINCX_TIMERS_USED        XINC_TIMERS_USED

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINCX_GLUE_H__
