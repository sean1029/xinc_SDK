/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_COMMON_H__
#define XINCX_COMMON_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <xinc.h>
#include <xinc_peripherals.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_common Common module
 * @{
 * @ingroup xincx
 * @brief Common module.
 */

/**
 * @brief Macro for checking if the specified identifier is defined and it has
 *        a non-zero value.
 *
 * Normally, preprocessors treat all undefined identifiers as having the value
 * zero. However, some tools, like static code analyzers, can issue a warning
 * when such identifier is evaluated. This macro gives the possibility to suppress
 * such warnings only in places where this macro is used for evaluation, not in
 * the whole analyzed code.
 */
#define XINCX_CHECK(module_enabled)  (module_enabled)

/**
 * @brief Macro for concatenating two tokens in macro expansion.
 *
 * @note This macro is expanded in two steps so that tokens given as macros
 *       themselves are fully expanded before they are merged.
 *
 * @param[in] p1 First token.
 * @param[in] p2 Second token.
 *
 * @return The two tokens merged into one, unless they cannot together form
 *         a valid token (in such case, the preprocessor issues a warning and
 *         does not perform the concatenation).
 *
 * @sa XINCX_CONCAT_3
 */
#define XINCX_CONCAT_2(p1, p2)       XINCX_CONCAT_2_(p1, p2)

/** @brief Internal macro used by @ref XINCX_CONCAT_2 to perform the expansion in two steps. */
#define XINCX_CONCAT_2_(p1, p2)      p1 ## p2

/**
 * @brief Macro for concatenating three tokens in macro expansion.
 *
 * @note This macro is expanded in two steps so that tokens given as macros
 *       themselves are fully expanded before they are merged.
 *
 * @param[in] p1 First token.
 * @param[in] p2 Second token.
 * @param[in] p3 Third token.
 *
 * @return The three tokens merged into one, unless they cannot together form
 *         a valid token (in such case, the preprocessor issues a warning and
 *         does not perform the concatenation).
 *
 * @sa XINCX_CONCAT_2
 */
#define XINCX_CONCAT_3(p1, p2, p3)   XINCX_CONCAT_3_(p1, p2, p3)

/** @brief Internal macro used by @ref XINCX_CONCAT_3 to perform the expansion in two steps. */
#define XINCX_CONCAT_3_(p1, p2, p3)  p1 ## p2 ## p3

/**
 * @brief Macro for performing rounded integer division (as opposed to
 *        truncating the result).
 *
 * @param[in] a Numerator.
 * @param[in] b Denominator.
 *
 * @return Rounded (integer) result of dividing @c a by @c b.
 */
#define XINCX_ROUNDED_DIV(a, b)  (((a) + ((b) / 2)) / (b))

/**
 * @brief Macro for performing integer division, making sure the result is rounded up.
 *
 * @details A typical use case for this macro is to compute the number of objects
 *          with size @c b required to hold @c a number of bytes.
 *
 * @param[in] a Numerator.
 * @param[in] b Denominator.
 *
 * @return Integer result of dividing @c a by @c b, rounded up.
 */
#define XINCX_CEIL_DIV(a, b)  ((((a) - 1) / (b)) + 1)

/**
 * @brief Macro for getting the number of elements in an array.
 *
 * @param[in] array Name of the array.
 *
 * @return Array element count.
 */
#define XINCX_ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

/**
 * @brief Macro for getting the offset (in bytes) from the beginning of a structure
 *        of the specified type to its specified member.
 *
 * @param[in] type   Structure type.
 * @param[in] member Structure member whose offset is searched for.
 *
 * @return Member offset in bytes.
 */
#define XINCX_OFFSETOF(type, member)  ((size_t)&(((type *)0)->member))

/**@brief Macro for checking if given lengths of EasyDMA transfers do not exceed
 *        the limit of the specified peripheral.
 *
 * @param[in] peripheral Peripheral to check the lengths against.
 * @param[in] length1    First length to be checked.
 * @param[in] length2    Second length to be checked (pass 0 if not needed).
 *
 * @retval true  The length of buffers does not exceed the limit of the specified peripheral.
 * @retval false The length of buffers exceeds the limit of the specified peripheral.
 */
#define XINCX_EASYDMA_LENGTH_VALIDATE(peripheral, length1, length2)            \
    (((length1) < (1U << XINCX_CONCAT_2(peripheral, _EASYDMA_MAXCNT_SIZE))) && \
     ((length2) < (1U << XINCX_CONCAT_2(peripheral, _EASYDMA_MAXCNT_SIZE))))

/**
 * @brief Macro for waiting until condition is met.
 *
 * @param[in]  condition Condition to meet.
 * @param[in]  attempts  Maximum number of condition checks. Must not be 0.
 * @param[in]  delay_us  Delay between consecutive checks, in microseconds.
 * @param[out] result    Boolean variable to store the result of the wait process.
 *                       Set to true if the condition is met or false otherwise.
 */
#define XINCX_WAIT_FOR(condition, attempts, delay_us, result) \
do {                                                         \
    result =  false;                                         \
    uint32_t remaining_attempts = (attempts);                \
    do {                                                     \
           if (condition)                                    \
           {                                                 \
               result =  true;                               \
               break;                                        \
           }                                                 \
           XINCX_DELAY_US(delay_us);                          \
    } while (--remaining_attempts);                          \
} while(0)

/**
 * @brief Macro for getting the ID number of the specified peripheral.
 *
 * For peripherals in Nordic SoCs, there is a direct relationship between their
 * ID numbers and their base addresses. See the chapter "Peripheral interface"
 * (section "Peripheral ID") in the Product Specification.
 *
 * @param[in] base_addr Peripheral base address or pointer.
 *
 * @return ID number associated with the specified peripheral.
 */
#define XINCX_PERIPHERAL_ID_GET(base_addr)  (uint8_t)((uint32_t)(base_addr) >> 12)

/**
 * @brief Macro for getting the interrupt number assigned to a specific
 *        peripheral.
 *
 * For peripherals in Nordic SoCs, the IRQ number assigned to a peripheral is
 * equal to its ID number. See the chapter "Peripheral interface" (sections
 * "Peripheral ID" and "Interrupts") in the Product Specification.
 *
 * @param[in] base_addr Peripheral base address or pointer.
 *
 * @return Interrupt number associated with the specified peripheral.
 */
#define XINCX_IRQ_NUMBER_GET(base_addr)  XINCX_PERIPHERAL_ID_GET(base_addr)

/** @brief IRQ handler type. */
typedef void (* xincx_irq_handler_t)(void);

/** @brief Driver state. */
typedef enum
{
    XINCX_DRV_STATE_UNINITIALIZED, ///< Uninitialized.
    XINCX_DRV_STATE_INITIALIZED,   ///< Initialized but powered off.
    XINCX_DRV_STATE_POWERED_ON,    ///< Initialized and powered on.
} xincx_drv_state_t;


/**
 * @brief Function for checking if an object is placed in the Data RAM region.
 *
 * Several peripherals (the ones using EasyDMA) require the transfer buffers
 * to be placed in the Data RAM region. This function can be used to check if
 * this condition is met.
 *
 * @param[in] p_object Pointer to an object whose location is to be checked.
 *
 * @retval true  The pointed object is located in the Data RAM region.
 * @retval false The pointed object is not located in the Data RAM region.
 */
__STATIC_INLINE bool xincx_is_in_ram(void const * p_object);

/**
 * @brief Function for checking if an object is aligned to a 32-bit word
 *
 * Several peripherals (the ones using EasyDMA) require the transfer buffers
 * to be aligned to a 32-bit word. This function can be used to check if
 * this condition is met.
 *
 * @param[in] p_object  Pointer to an object whose location is to be checked.
 *
 * @retval true  The pointed object is aligned to a 32-bit word.
 * @retval false The pointed object is not aligned to a 32-bit word.
 */
__STATIC_INLINE bool xincx_is_word_aligned(void const * p_object);

/**
 * @brief Function for getting the interrupt number for the specified peripheral.
 *
 * @param[in] p_reg Peripheral base pointer.
 *
 * @return Interrupt number associated with the pointed peripheral.
 */
__STATIC_INLINE IRQn_Type xincx_get_irq_number(void const * p_reg);

/**
 * @brief Function for converting an INTEN register bit position to the
 *        corresponding event identifier.
 *
 * The event identifier is the offset between the event register address and
 * the peripheral base address, and is equal (thus, can be directly cast) to
 * the corresponding value of the enumerated type from HAL (xinc_*_event_t).
 *
 * @param[in] bit INTEN register bit position.
 *
 * @return Event identifier.
 *
 * @sa xincx_event_to_bitpos
 */
__STATIC_INLINE uint32_t xincx_bitpos_to_event(uint32_t bit);

/**
 * @brief Function for converting an event identifier to the corresponding
 *        INTEN register bit position.
 *
 * The event identifier is the offset between the event register address and
 * the peripheral base address, and is equal (thus, can be directly cast) to
 * the corresponding value of the enumerated type from HAL (xinc_*_event_t).
 *
 * @param[in] event Event identifier.
 *
 * @return INTEN register bit position.
 *
 * @sa xincx_bitpos_to_event
 */
__STATIC_INLINE uint32_t xincx_event_to_bitpos(uint32_t event);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION

__STATIC_INLINE bool xincx_is_in_ram(void const * p_object)
{
 //   return ((((uint32_t)p_object) & 0xE0000000u) == 0x20000000u);
	return ((((uint32_t)p_object) > 0x10010000));
}

__STATIC_INLINE bool xincx_is_word_aligned(void const * p_object)
{
    return ((((uint32_t)p_object) & 0x3u) == 0u);
}

__STATIC_INLINE IRQn_Type xincx_get_irq_number(void const * p_reg)
{
    return (IRQn_Type)XINCX_IRQ_NUMBER_GET(p_reg);
}

__STATIC_INLINE uint32_t xincx_bitpos_to_event(uint32_t bit)
{
    static const uint32_t event_reg_offset = 0x100u;
    return event_reg_offset + (bit * sizeof(uint32_t));
}

__STATIC_INLINE uint32_t xincx_event_to_bitpos(uint32_t event)
{
    static const uint32_t event_reg_offset = 0x100u;
    return (event - event_reg_offset) / sizeof(uint32_t);
}

#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINCX_COMMON_H__
