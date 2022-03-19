/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef _COMPILER_ABSTRACTION_H
#define _COMPILER_ABSTRACTION_H

/*lint ++flb "Enter library region" */

#ifndef XINC_STRING_CONCATENATE_IMPL
    #define XINC_STRING_CONCATENATE_IMPL(lhs, rhs) lhs ## rhs
#endif
#ifndef XINC_STRING_CONCATENATE
    #define XINC_STRING_CONCATENATE(lhs, rhs) XINC_STRING_CONCATENATE_IMPL(lhs, rhs)
#endif
#if  __LINT__ == 1
    #ifndef XINC_STATIC_ASSERT
        #define XINC_STATIC_ASSERT(cond, msg)
    #endif
#endif

#if defined ( __CC_ARM )

    #ifndef __ASM
        #define __ASM               __asm
    #endif

    #ifndef __INLINE
        #define __INLINE            __inline
    #endif
		
		 #ifndef __STATIC_INLINE
        #define __STATIC_INLINE       static __inline
    #endif

    #ifndef __WEAK
        #define __WEAK              __weak
    #endif

    #ifndef __ALIGN
        #define __ALIGN(n)          __align(n)
    #endif

    #ifndef __PACKED
        #define __PACKED            __packed
    #endif

    #ifndef __UNUSED
        #define __UNUSED            __attribute__((unused))
    #endif

    #define GET_SP()                __current_sp()

    #ifndef XINC_STATIC_ASSERT
        #define XINC_STATIC_ASSERT(cond, msg) \
            ;enum { XINC_STRING_CONCATENATE(static_assert_on_line_, __LINE__) = 1 / (!!(cond)) }
    #endif
    
#elif defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)

    #ifndef __ASM
        #define __ASM               __asm
    #endif

    #ifndef __INLINE
        #define __INLINE            __inline
    #endif

    #ifndef __WEAK
        #define __WEAK              __attribute__((weak))
    #endif

    #ifndef __ALIGN
        #define __ALIGN(n)          __attribute__((aligned(n)))
    #endif

    #ifndef __PACKED
        #define __PACKED            __attribute__((packed, aligned(1)))
    #endif

    #ifndef __UNUSED
        #define __UNUSED            __attribute__((unused))
    #endif

    #define GET_SP()                __current_sp()

    #ifndef XINC_STATIC_ASSERT
        #define XINC_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
    #endif

#elif defined ( __ICCARM__ )

    #ifndef __ASM
        #define __ASM               __asm
    #endif

    #ifndef __INLINE
        #define __INLINE            inline
    #endif

    #ifndef __WEAK
        #define __WEAK              __weak
    #endif

    #ifndef __ALIGN
        #define STRING_PRAGMA(x) _Pragma(#x)
        #define __ALIGN(n) STRING_PRAGMA(data_alignment = n)
    #endif

    #ifndef __PACKED
        #define __PACKED            __packed
    #endif

    #ifndef __UNUSED
        #define __UNUSED
    #endif
    
    #define GET_SP()                __get_SP()

    #ifndef XINC_STATIC_ASSERT
        #define XINC_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
    #endif

#elif defined   ( __GNUC__ ) ||  defined   ( __clang__ )

    #ifndef __ASM
        #define __ASM               __asm
    #endif

    #ifndef __INLINE
        #define __INLINE            inline
    #endif

    #ifndef __WEAK
        #define __WEAK              __attribute__((weak))
    #endif

    #ifndef __ALIGN
        #define __ALIGN(n)          __attribute__((aligned(n)))
    #endif

    #ifndef __PACKED
        #define __PACKED           __attribute__((packed)) 
    #endif

    #ifndef __UNUSED
        #define __UNUSED            __attribute__((unused))
    #endif

    #define GET_SP()                gcc_current_sp()

    static inline unsigned int gcc_current_sp(void)
    {
        unsigned int stack_pointer = 0;
        __asm__ __volatile__ ("mov %0, sp" : "=r"(stack_pointer));
        return stack_pointer;
    }

    #ifndef XINC_STATIC_ASSERT
        #define XINC_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
    #endif

#elif defined   ( __TASKING__ )

    #ifndef __ASM
        #define __ASM               __asm
    #endif

    #ifndef __INLINE
        #define __INLINE            inline
    #endif

    #ifndef __WEAK
        #define __WEAK              __attribute__((weak))
    #endif

    #ifndef __ALIGN
        #define __ALIGN(n)          __align(n)
    #endif
    
    /* Not defined for TASKING. */
    #ifndef __PACKED
        #define __PACKED
    #endif

    #ifndef __UNUSED
        #define __UNUSED            __attribute__((unused))
    #endif

    #define GET_SP()                __get_MSP()

    #ifndef XINC_STATIC_ASSERT
        #define XINC_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
    #endif

#endif

#define XINC_MDK_VERSION_ASSERT_AT_LEAST(major, minor, micro) \
    XINC_STATIC_ASSERT( \
        ( \
            (major < MDK_MAJOR_VERSION) || \
            (major == MDK_MAJOR_VERSION && minor < MDK_MINOR_VERSION) || \
            (major == MDK_MAJOR_VERSION && minor == MDK_MINOR_VERSION && micro < MDK_MICRO_VERSION) \
        ), "MDK version mismatch.")

#define XINC_MDK_VERSION_ASSERT_EXACT(major, minor, micro) \
    XINC_STATIC_ASSERT( \
        ( \
            (major != MDK_MAJOR_VERSION) || \
            (major != MDK_MAJOR_VERSION) || \
            (major != MDK_MAJOR_VERSION) \
        ), "MDK version mismatch.")

/*lint --flb "Leave library region" */

#endif
