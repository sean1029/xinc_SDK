/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include "xinc_memobj.h"
#include "xinc_atomic.h"
#include "xinc_assert.h"

typedef struct memobj_elem_s memobj_elem_t;

/** @brief Standard chunk header. */
typedef struct
{
    memobj_elem_t * p_next; ///< Pointer to the next element.
} memobj_header_t;

/** @brief Head header extension fields. */
typedef struct
{
    uint8_t  user_cnt;    ///< User counter (see @ref xinc_memobj_get and @ref xinc_memobj_put).
    uint8_t  chunk_cnt;   ///< Number of chunks in the object.
    uint16_t chunk_size;  ///< Single chunk size
} memobj_head_header_fields_t;

/** @brief Head header extension. */
typedef struct
{
    union
    {
        xinc_atomic_u32_t            atomic_user_cnt;
        memobj_head_header_fields_t fields;
    } data;
} memobj_head_header_t;

/** @brief Head chunk structure. */
typedef struct
{
    memobj_header_t      header;      ///< Standard header.
    memobj_head_header_t head_header; ///< Head-specific header part.
    uint8_t              data[1];     ///< Data.
} memobj_head_t;

STATIC_ASSERT(sizeof(memobj_header_t) == XINC_MEMOBJ_STD_HEADER_SIZE);

/** @brief Standard chunk structure. */
struct memobj_elem_s
{
    memobj_header_t  header;  ///< Standard header.
    uint8_t          data[1]; ///< Data.
};

ret_code_t xinc_memobj_pool_init(xinc_memobj_pool_t const * p_pool)
{
    return xinc_balloc_init((xinc_balloc_t const *)p_pool);
}

xinc_memobj_t * xinc_memobj_alloc(xinc_memobj_pool_t const * p_pool,
                                size_t size)
{
    uint32_t bsize = (uint32_t)XINC_BALLOC_ELEMENT_SIZE((xinc_balloc_t const *)p_pool) - sizeof(memobj_header_t);
    uint8_t num_of_chunks = (uint8_t)CEIL_DIV(size + sizeof(memobj_head_header_t), bsize);

    memobj_head_t * p_head = xinc_balloc_alloc((xinc_balloc_t const *)p_pool);
    if (p_head == NULL)
    {
        return NULL;
    }
    p_head->head_header.data.fields.user_cnt = 0;
    p_head->head_header.data.fields.chunk_cnt = 1;
    p_head->head_header.data.fields.chunk_size = bsize;

    memobj_header_t * p_prev = (memobj_header_t *)p_head;
    memobj_header_t * p_curr;
    uint32_t i;
    uint32_t chunk_less1 = (uint32_t)num_of_chunks - 1;

    p_prev->p_next =  (memobj_elem_t *)p_pool;
    for (i = 0; i < chunk_less1; i++)
    {
        p_curr = (memobj_header_t *)xinc_balloc_alloc((xinc_balloc_t const *)p_pool);
        if (p_curr)
        {
            (p_head->head_header.data.fields.chunk_cnt)++;
            p_prev->p_next = (memobj_elem_t *)p_curr;
            p_curr->p_next = (memobj_elem_t *)p_pool;
            p_prev = p_curr;
        }
        else
        {
            //Could not allocate all requested buffers
            xinc_memobj_free((xinc_memobj_t *)p_head);
            return NULL;
        }
    }
    return (xinc_memobj_t *)p_head;
}

void xinc_memobj_free(xinc_memobj_t * p_obj)
{
    memobj_head_t * p_head = (memobj_head_t *)p_obj;
    uint8_t chunk_cnt = p_head->head_header.data.fields.chunk_cnt;
    uint32_t i;
    memobj_header_t * p_curr = (memobj_header_t *)p_obj;
    memobj_header_t * p_next;
    uint32_t chunk_less1 = (uint32_t)chunk_cnt - 1;

    for (i = 0; i < chunk_less1; i++)
    {
        p_curr = (memobj_header_t *)p_curr->p_next;
    }
    xinc_balloc_t const * p_pool2 = (xinc_balloc_t const *)p_curr->p_next;

    p_curr = (memobj_header_t *)p_obj;
    for (i = 0; i < chunk_cnt; i++)
    {
        p_next = (memobj_header_t *)p_curr->p_next;
        xinc_balloc_free(p_pool2, p_curr);
        p_curr = p_next;
    }
}

void xinc_memobj_get(xinc_memobj_t const * p_obj)
{
    memobj_head_t * p_head = (memobj_head_t *)p_obj;
    (void)xinc_atomic_u32_add(&p_head->head_header.data.atomic_user_cnt, 1);
}

void xinc_memobj_put(xinc_memobj_t * p_obj)
{
    memobj_head_t * p_head = (memobj_head_t *)p_obj;
    uint32_t user_cnt = xinc_atomic_u32_sub(&p_head->head_header.data.atomic_user_cnt, 1);
    memobj_head_header_fields_t * p_fields = (memobj_head_header_fields_t *)&user_cnt;
    if (p_fields->user_cnt == 0)
    {
        xinc_memobj_free(p_obj);
    }
}

static void memobj_op(xinc_memobj_t * p_obj,
                      void *         p_data,
                      size_t *       p_len,
                      size_t         offset,
                      bool read)
{

    ASSERT(p_obj);

    memobj_head_t * p_head       = (memobj_head_t *)p_obj;
    memobj_elem_t * p_curr_chunk = (memobj_elem_t *)p_obj;
    size_t          obj_capacity;
    size_t          chunk_size;
    size_t          chunk_idx;
    size_t          chunk_offset;
    size_t          len;

    obj_capacity = (p_head->head_header.data.fields.chunk_size *
                    p_head->head_header.data.fields.chunk_cnt) -
                    sizeof(memobj_head_header_fields_t);

    ASSERT(offset < obj_capacity);

    chunk_size   = p_head->head_header.data.fields.chunk_size;
    chunk_idx    = (offset + sizeof(memobj_head_header_fields_t)) / chunk_size;
    chunk_offset = (offset + sizeof(memobj_head_header_fields_t)) % chunk_size;
    len          = ((*p_len + offset) > obj_capacity) ? obj_capacity - offset : *p_len;

    //Return number of available bytes
    *p_len = len;

    //Move to the first chunk to be used
    while (chunk_idx > 0)
    {
        p_curr_chunk = p_curr_chunk->header.p_next;
        chunk_idx--;
    }

    size_t user_mem_offset  = 0;
    size_t curr_cpy_size    = chunk_size - chunk_offset;
    curr_cpy_size = curr_cpy_size > len ? len : curr_cpy_size;

    while (len)
    {
        void * p_user_mem = &((uint8_t *)p_data)[user_mem_offset];
        void * p_obj_mem  = &p_curr_chunk->data[chunk_offset];
        if (read)
        {
            memcpy(p_user_mem, p_obj_mem, curr_cpy_size);
        }
        else
        {
            memcpy(p_obj_mem, p_user_mem, curr_cpy_size);
        }

        chunk_offset     = 0;
        p_curr_chunk     = p_curr_chunk->header.p_next;
        len             -= curr_cpy_size;
        user_mem_offset += curr_cpy_size;
        curr_cpy_size    = (chunk_size > len) ? len : chunk_size;
    }
}

void xinc_memobj_write(xinc_memobj_t * p_obj,
                      void *         p_data,
                      size_t         len,
                      size_t         offset)
{

    size_t op_len = len;
    memobj_op(p_obj, p_data, &op_len, offset, false);
    ASSERT(op_len == len);
}

void xinc_memobj_read(xinc_memobj_t * p_obj,
                     void *         p_data,
                     size_t         len,
                     size_t         offset)
{
    size_t op_len = len;
    memobj_op(p_obj, p_data, &op_len, offset, true);
    ASSERT(op_len == len);

}
