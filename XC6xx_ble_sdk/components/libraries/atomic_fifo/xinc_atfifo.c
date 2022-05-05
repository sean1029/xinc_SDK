/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "app_util.h"
#include "xinc_atfifo.h"
#include "xinc_atfifo_internal.h"

#if XINC_ATFIFO_CONFIG_LOG_ENABLED
    #define XINC_LOG_LEVEL             XINC_ATFIFO_CONFIG_LOG_LEVEL
    #define XINC_LOG_INIT_FILTER_LEVEL XINC_ATFIFO_CONFIG_LOG_INIT_FILTER_LEVEL
    #define XINC_LOG_INFO_COLOR        XINC_ATFIFO_CONFIG_INFO_COLOR
    #define XINC_LOG_DEBUG_COLOR       XINC_ATFIFO_CONFIG_DEBUG_COLOR
#else
    #define XINC_LOG_LEVEL       0
#endif // XINC_ATFIFO_CONFIG_LOG_ENABLED
#include "xinc_log.h"

/* Unions testing */
STATIC_ASSERT(sizeof(xinc_atfifo_postag_t) == sizeof(uint32_t));


ret_code_t xinc_atfifo_init(xinc_atfifo_t * const p_fifo, void * p_buf, uint16_t buf_size, uint16_t item_size)
{
    if (NULL == p_buf)
    {
        XINC_LOG_INST_ERROR(p_fifo->p_log, "Initialization failed. p_buf == NULL");
        return XINC_ERROR_NULL;
    }
    if (0 != (buf_size % item_size))
    {
        XINC_LOG_INST_ERROR(p_fifo->p_log, "Initialization failed. Buf_size not multiple of item_size");
        return XINC_ERROR_INVALID_LENGTH;
    }

    p_fifo->p_buf     = p_buf;
    p_fifo->tail.tag  = 0;
    p_fifo->tail.pos.wr = 0;
    p_fifo->tail.pos.rd = 0;
    p_fifo->head.tag  = 0;
    p_fifo->head.pos.wr = 0;
    p_fifo->head.pos.rd = 0;
    p_fifo->buf_size  = buf_size;
    p_fifo->item_size = item_size;

    XINC_LOG_INST_INFO(p_fifo->p_log, "Initialized.");
    return XINC_SUCCESS;
}


ret_code_t xinc_atfifo_clear(xinc_atfifo_t * const p_fifo)
{
    bool released = xinc_atfifo_space_clear(p_fifo);//
    
    XINC_LOG_INST_INFO(p_fifo->p_log, "Cleared result:%s", released ? "success" : "busy");
    return released ? XINC_SUCCESS : XINC_ERROR_BUSY;
}


ret_code_t xinc_atfifo_alloc_put(xinc_atfifo_t * const p_fifo, void const * p_var, size_t size, bool * const p_visible)
{
    xinc_atfifo_item_put_t context;
    bool visible;
    void * p_data = xinc_atfifo_item_alloc(p_fifo, &context);
    if (NULL == p_data)
    {
        XINC_LOG_INST_WARNING(p_fifo->p_log, "Copying in element (0x%08X) failed - no space.", p_var);
        return XINC_ERROR_NO_MEM;
    }

    memcpy(p_data, p_var, size);

    visible = xinc_atfifo_item_put(p_fifo, &context);
    if (NULL != p_visible)
    {
        *p_visible = visible;
    }
    XINC_LOG_INST_DEBUG(p_fifo->p_log, "Element (0x%08X) copied in.", p_var);
    return XINC_SUCCESS;
}


void * xinc_atfifo_item_alloc(xinc_atfifo_t * const p_fifo, xinc_atfifo_item_put_t * p_context)
{
    if (xinc_atfifo_wspace_req(p_fifo, &(p_context->last_tail)))
	//	p_context->last_tail = p_fifo->tail;
    {
        void * p_item = ((uint8_t*)(p_fifo->p_buf)) + p_context->last_tail.pos.wr;
        XINC_LOG_INST_DEBUG(p_fifo->p_log, "Allocated  element (0x%08X).", p_item);
        return p_item;
    }
    XINC_LOG_INST_WARNING(p_fifo->p_log, "Allocation failed - no space.");
    return NULL; 
}


bool xinc_atfifo_item_put(xinc_atfifo_t * const p_fifo, xinc_atfifo_item_put_t * p_context)
{
    if ((p_context->last_tail.pos.wr) == (p_context->last_tail.pos.rd))
    {
        XINC_LOG_INST_DEBUG(p_fifo->p_log, "Put (uninterrupted)");
		//	p_fifo->tail = p_context->last_tail;
        xinc_atfifo_wspace_close(p_fifo);
        return true;
    }
    XINC_LOG_INST_DEBUG(p_fifo->p_log, "Put (interrupted!)");
    return false;
}


ret_code_t xinc_atfifo_get_free(xinc_atfifo_t * const p_fifo, void * const p_var, size_t size, bool * p_released)
{
    xinc_atfifo_item_get_t context;
    bool released;
    void const * p_s = xinc_atfifo_item_get(p_fifo, &context);
    if (NULL == p_s)
    {
        XINC_LOG_INST_WARNING(p_fifo->p_log, "Copying out failed - no item in the FIFO.");
        return XINC_ERROR_NOT_FOUND;
    }

    memcpy(p_var, p_s, size);

    released = xinc_atfifo_item_free(p_fifo, &context);
    if (NULL != p_released)
    {
        *p_released = released;
    }
    XINC_LOG_INST_DEBUG(p_fifo->p_log, "Element (0x%08X) copied out.", p_var);
    return XINC_SUCCESS;
}


void * xinc_atfifo_item_get(xinc_atfifo_t * const p_fifo, xinc_atfifo_item_get_t * p_context)
{
    if (xinc_atfifo_rspace_req(p_fifo, &(p_context->last_head)))		
  //  p_context->last_head = p_fifo->head;
    {
    void * p_item = ((uint8_t*)(p_fifo->p_buf)) + p_context->last_head.pos.rd;
    XINC_LOG_INST_DEBUG(p_fifo->p_log, "Get element: 0x%08X", p_item);
    return p_item;
    }
    XINC_LOG_INST_WARNING(p_fifo->p_log, "Get failed - no item in the FIFO.");
    return NULL;
}


bool xinc_atfifo_item_free(xinc_atfifo_t * const p_fifo, xinc_atfifo_item_get_t * p_context)
{
    if ((p_context->last_head.pos.wr) == (p_context->last_head.pos.rd))
    {
        XINC_LOG_INST_DEBUG(p_fifo->p_log, "Free (uninterrupted)");
        p_fifo->head = p_context->last_head ;
        xinc_atfifo_rspace_close(p_fifo);
        return true;
    }
    XINC_LOG_INST_DEBUG(p_fifo->p_log, "Free (interrupted)");
    return false;
}
