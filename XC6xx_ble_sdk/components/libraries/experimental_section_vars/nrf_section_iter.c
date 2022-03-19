/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include "sdk_common.h"

#if NRF_MODULE_ENABLED(NRF_SECTION_ITER)

#include "nrf_section_iter.h"


#if !defined(__GNUC__)
static void nrf_section_iter_item_set(nrf_section_iter_t * p_iter)
{
    ASSERT(p_iter            != NULL);
    ASSERT(p_iter->p_set     != NULL);
    ASSERT(p_iter->p_section != NULL);

    while (true)
    {
        if (p_iter->p_section == p_iter->p_set->p_last)
        {
            // End of the section set.
            p_iter->p_item = NULL;
            return;
        }

        if (p_iter->p_section->p_start != p_iter->p_section->p_end)
        {
            // Not empty section.
            p_iter->p_item = p_iter->p_section->p_start;
            return;
        }

        // Next section.
        p_iter->p_section++;
    }
}
#endif


void nrf_section_iter_init(nrf_section_iter_t * p_iter, nrf_section_set_t const * p_set)
{
    ASSERT(p_iter != NULL);
    ASSERT(p_set  != NULL);

    p_iter->p_set = p_set;

#if defined(__GNUC__)
    p_iter->p_item = p_iter->p_set->section.p_start;
    if (p_iter->p_item == p_iter->p_set->section.p_end)
    {
        p_iter->p_item = NULL;
    }
#else
    p_iter->p_section = p_set->p_first;
    nrf_section_iter_item_set(p_iter);
#endif
}

void nrf_section_iter_next(nrf_section_iter_t * p_iter)
{
    ASSERT(p_iter        != NULL);
    ASSERT(p_iter->p_set != NULL);

    if (p_iter->p_item == NULL)
    {
        return;
    }

    p_iter->p_item = (void *)((size_t)(p_iter->p_item) + p_iter->p_set->item_size);

#if defined(__GNUC__)
    if (p_iter->p_item == p_iter->p_set->section.p_end)
    {
        p_iter->p_item = NULL;
    }
#else
    ASSERT(p_iter->p_section != NULL);
    // End of current section reached?
    if (p_iter->p_item == p_iter->p_section->p_end)
    {
        p_iter->p_section++;
        nrf_section_iter_item_set(p_iter);
    }
#endif
}

#endif // NRF_MODULE_ENABLED(NRF_SECTION_ITER)
