/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "sdk_common.h"
#if XINC_MODULE_ENABLED(XINC_LOG)
#include "app_util.h"
#include "app_util_platform.h"
#include "xinc_log.h"
#include "xinc_log_internal.h"
#include "xinc_log_ctrl.h"
#include "xinc_log_backend_interface.h"
#include "xinc_log_str_formatter.h"
#include "xinc_section.h"
#include "xinc_ringbuf.h"
#include "xinc_memobj.h"
#include "xinc_atomic.h"
#include <string.h>

STATIC_ASSERT((XINC_LOG_BUFSIZE % 4) == 0);
STATIC_ASSERT(IS_POWER_OF_TWO(XINC_LOG_BUFSIZE));

#define XINC_LOG_BUF_WORDS (XINC_LOG_BUFSIZE/4)

#if XINC_MODULE_ENABLED(FDS) && XINC_LOG_FILTERS_ENABLED
#define LOG_CONFIG_LOAD_STORE_ENABLED 1
#else
#define LOG_CONFIG_LOAD_STORE_ENABLED 0
#endif

#if XINC_LOG_BUF_WORDS < 32
#warning "XINC_LOG_BUFSIZE too small, significant number of logs may be lost."
#endif

XINC_MEMOBJ_POOL_DEF(log_mempool, XINC_LOG_MSGPOOL_ELEMENT_SIZE, XINC_LOG_MSGPOOL_ELEMENT_COUNT);
XINC_RINGBUF_DEF(m_log_push_ringbuf, XINC_LOG_STR_PUSH_BUFFER_SIZE);

#define XINC_LOG_BACKENDS_FULL           0xFF
#define XINC_LOG_FILTER_BITS_PER_BACKEND 3
#define XINC_LOG_MAX_BACKENDS           (32/XINC_LOG_FILTER_BITS_PER_BACKEND)
#define XINC_LOG_MAX_HEXDUMP            (XINC_LOG_MSGPOOL_ELEMENT_SIZE*XINC_LOG_MSGPOOL_ELEMENT_COUNT/2)
#define XINC_LOG_INVALID_BACKEND_U32    0xFFFFFFFF

/* Mask is extracted from the structure log_data_t to allow compile time initialization which
 * is needed to allow usage of the logger (logging) before logger is initialized.
 * It cannot be part of the log_data_t because some compilers would put whole log_data_t structure
 * into flash (including buffer) just because one field is initilized.
 */
static uint32_t m_buffer_mask =  XINC_LOG_BUF_WORDS - 1; // Size of buffer (must be power of 2) presented as mask

/**
 * brief An internal control block of the logger
 *
 * @note Circular buffer is using never cleared indexes and a mask. It means
 * that logger may break when indexes overflows. However, it is quite unlikely.
 * With rate of 1000 log entries with 2 parameters per second such situation
 * would happen after 12 days.
 */
typedef struct
{
    bool                      autoflush;
    uint32_t                  wr_idx;          // Current write index (never reset)
    uint32_t                  rd_idx;          // Current read index  (never_reset)
    uint32_t                  buffer[XINC_LOG_BUF_WORDS];
    xinc_log_timestamp_func_t  timestamp_func;  // A pointer to function that returns timestamp
    xinc_log_backend_t const * p_backend_head;
    xinc_atomic_flag_t         log_skipping;
    xinc_atomic_flag_t         log_skipped;
    xinc_atomic_u32_t          log_dropped_cnt;
} log_data_t;

static log_data_t   m_log_data;

XINC_LOG_MODULE_REGISTER();

// Helper macros for section variables.
#define XINC_LOG_DYNAMIC_SECTION_VARS_GET(i)        XINC_SECTION_ITEM_GET(log_dynamic_data, xinc_log_module_dynamic_data_t, (i))
#define XINC_LOG_FILTER_SECTION_VARS_GET(i)         XINC_SECTION_ITEM_GET(log_filter_data, xinc_log_module_filter_data_t, (i))

#define XINC_LOG_CONST_SECTION_VARS_GET(i)          XINC_SECTION_ITEM_GET(log_const_data, xinc_log_module_const_data_t, (i))
#define XINC_LOG_CONST_SECTION_VARS_COUNT           XINC_SECTION_ITEM_COUNT(log_const_data, xinc_log_module_const_data_t)

//  _name, _str_name, _info_color, _debug_color, _initial_lvl, _compiled_lvl) 
//XINC_LOG_INTERNAL_ITEM_REGISTER(XINC_LOG_CONST_SECTION_NAME(_module_name),STRINGIFY(XINC_LOG_CONST_SECTION_NAME(_module_name)),0,0,3,4);

ret_code_t xinc_log_init(xinc_log_timestamp_func_t timestamp_func, uint32_t timestamp_freq)
{
    (void)XINC_LOG_ITEM_DATA_CONST(app);

    if (XINC_LOG_USES_TIMESTAMP && (timestamp_func == NULL))
    {
        return XINC_ERROR_INVALID_PARAM;
    }

    m_log_data.autoflush    = true;// XINC_LOG_DEFERRED ? false : true;

    if (XINC_LOG_USES_TIMESTAMP)
    {
        xinc_log_str_formatter_timestamp_freq_set(timestamp_freq);
        m_log_data.timestamp_func = timestamp_func;
    }

#ifdef UNIT_TEST
    m_buffer_mask = XINC_LOG_BUF_WORDS - 1;
#endif

    ret_code_t err_code = xinc_memobj_pool_init(&log_mempool);
    if (err_code != XINC_SUCCESS)
    {
        return err_code;
    }

    xinc_ringbuf_init(&m_log_push_ringbuf);

    uint32_t modules_cnt = XINC_LOG_CONST_SECTION_VARS_COUNT;
    uint32_t i;
		printf("modules_cnt :%d\r\n",XINC_LOG_CONST_SECTION_VARS_COUNT);
	//	printf("XINC_LOG_CONST:%s\r\n",STRINGIFY(XINC_LOG_CONST_SECTION_NAME(_module_name)));
		
    if (XINC_LOG_FILTERS_ENABLED)
    {
        uint32_t j;
        //sort modules by name
        for (i = 0; i < modules_cnt; i++)
        {
            uint32_t idx = 0;

            for (j = 0; j < modules_cnt; j++)
            {
                if  (i != j)
                {
                    char const * p_name0 = XINC_LOG_CONST_SECTION_VARS_GET(i)->p_module_name;
                    char const * p_name1 = XINC_LOG_CONST_SECTION_VARS_GET(j)->p_module_name;
									  printf("p_name0 :%s,p_name1:%s\r\n",p_name0,p_name1);
                    if (strncmp(p_name0, p_name1, 20) > 0)
                    {
                        idx++;
                    }
                }

            }
            xinc_log_module_dynamic_data_t * p_module_ddata = XINC_LOG_DYNAMIC_SECTION_VARS_GET(i);
            p_module_ddata->order_idx = idx;
        }

        /* Initialize filters */
        for (i = 0; i < modules_cnt; i++)
        {
            xinc_log_module_dynamic_data_t * p_module_ddata = XINC_LOG_DYNAMIC_SECTION_VARS_GET(i);
            xinc_log_module_filter_data_t * p_module_filter = XINC_LOG_FILTER_SECTION_VARS_GET(i);
            p_module_ddata->filter = 0;
            p_module_filter->filter_lvls = 0;
        }
    }

    return XINC_SUCCESS;
}

uint32_t xinc_log_module_cnt_get(void)
{
    return XINC_LOG_CONST_SECTION_VARS_COUNT;
}

static ret_code_t module_idx_get(uint32_t * p_idx, bool ordered_idx)
{
    if (ordered_idx)
    {
        uint32_t module_cnt = xinc_log_module_cnt_get();
        uint32_t i;
        for (i = 0; i < module_cnt; i++)
        {
            xinc_log_module_dynamic_data_t * p_module_data = XINC_LOG_DYNAMIC_SECTION_VARS_GET(i);
            if (p_module_data->order_idx == *p_idx)
            {
                *p_idx = i;
                return XINC_SUCCESS;
            }
        }
        return XINC_ERROR_NOT_FOUND;
    }
    else
    {
        return XINC_SUCCESS;
    }
}
const char * xinc_log_module_name_get(uint32_t module_id, bool ordered_idx)
{
    if (module_idx_get(&module_id, ordered_idx) == XINC_SUCCESS)
    {
        xinc_log_module_const_data_t * p_module_data = XINC_LOG_CONST_SECTION_VARS_GET(module_id);
        return p_module_data->p_module_name;
    }
    else
    {
        return NULL;
    }
}

uint8_t xinc_log_color_id_get(uint32_t module_id, xinc_log_severity_t severity)
{
    xinc_log_module_const_data_t * p_module_data = XINC_LOG_CONST_SECTION_VARS_GET(module_id);
    uint8_t color_id;
    switch (severity)
    {
    case XINC_LOG_SEVERITY_ERROR:
        color_id = XINC_LOG_ERROR_COLOR;
        break;
    case XINC_LOG_SEVERITY_WARNING:
        color_id = XINC_LOG_WARNING_COLOR;
        break;
    case XINC_LOG_SEVERITY_INFO:
        color_id = p_module_data->info_color_id;
        break;
    case XINC_LOG_SEVERITY_DEBUG:
        color_id = p_module_data->debug_color_id;
        break;
    default:
        color_id = 0;
        break;
    }
    return color_id;
}

static uint16_t higher_lvl_get(uint32_t lvls)
{
    uint16_t top_lvl = 0;
    uint16_t tmp_lvl;
    uint32_t i;

    //Find highest level enabled by backends
    for (i = 0; i < (32/XINC_LOG_LEVEL_BITS); i+=XINC_LOG_LEVEL_BITS)
    {
        tmp_lvl = (uint16_t)BF_GET(lvls,XINC_LOG_LEVEL_BITS, i);
        if (tmp_lvl > top_lvl)
        {
            top_lvl = tmp_lvl;
        }
    }
    return top_lvl;
}

void xinc_log_module_filter_set(uint32_t backend_id, uint32_t module_id, xinc_log_severity_t severity)
{
    if (XINC_LOG_FILTERS_ENABLED)
    {
        xinc_log_module_dynamic_data_t * p_module_data = XINC_LOG_DYNAMIC_SECTION_VARS_GET(module_id);
        xinc_log_module_filter_data_t *  p_filter      = XINC_LOG_FILTER_SECTION_VARS_GET(module_id);

        p_filter->filter_lvls &= ~BF_MASK(XINC_LOG_LEVEL_BITS, (XINC_LOG_LEVEL_BITS * backend_id));
        p_filter->filter_lvls |= BF_VAL(severity, XINC_LOG_LEVEL_BITS, (XINC_LOG_LEVEL_BITS * backend_id));

        p_module_data->filter = higher_lvl_get(p_filter->filter_lvls);
    }
}

static xinc_log_severity_t xinc_log_module_init_filter_get(uint32_t module_id)
{
    xinc_log_module_const_data_t * p_module_data =
                                        XINC_LOG_CONST_SECTION_VARS_GET(module_id);
    return XINC_LOG_FILTERS_ENABLED ? p_module_data->initial_lvl : p_module_data->compiled_lvl;
}

xinc_log_severity_t xinc_log_module_filter_get(uint32_t backend_id,
                                             uint32_t module_id,
                                             bool ordered_idx,
                                             bool dynamic)
{
    xinc_log_severity_t severity = XINC_LOG_SEVERITY_NONE;
    if (XINC_LOG_FILTERS_ENABLED && dynamic)
    {
        if (module_idx_get(&module_id, ordered_idx) == XINC_SUCCESS)
        {
            xinc_log_module_filter_data_t * p_filter = XINC_LOG_FILTER_SECTION_VARS_GET(module_id);
            severity = (xinc_log_severity_t)BF_GET(p_filter->filter_lvls,
                                                  XINC_LOG_LEVEL_BITS,
                                                  (backend_id*XINC_LOG_LEVEL_BITS));
        }
    }
    else if (!dynamic)
    {
        if (module_idx_get(&module_id, ordered_idx) == XINC_SUCCESS)
        {
            xinc_log_module_const_data_t * p_module_data =
                                                XINC_LOG_CONST_SECTION_VARS_GET(module_id);
            severity = (xinc_log_severity_t)p_module_data->compiled_lvl;
        }
    }
    return severity;
}
/**
 * Function examines current header and omits packets which are in progress.
 */
static bool invalid_packets_omit(xinc_log_header_t const * p_header, uint32_t * p_rd_idx)
{
    bool ret = false;
    if (p_header->base.generic.in_progress == 1)
    {
        switch (p_header->base.generic.type)
        {
        case HEADER_TYPE_STD:
            *p_rd_idx += (HEADER_SIZE + p_header->base.std.nargs);
            break;
        case HEADER_TYPE_HEXDUMP:
            *p_rd_idx += (HEADER_SIZE + p_header->base.hexdump.len);
            break;
        default:
            break;
        }
        ret = true;
    }
    return ret;
}
/**
 * @brief Skips the oldest, not processed logs to make space for new logs.
 * @details This function moves forward read index to prepare space for new logs.
 */

static uint32_t log_skip(void)
{
    uint16_t dropped = 0;

    (void)xinc_atomic_flag_set(&m_log_data.log_skipped);
    (void)xinc_atomic_flag_set(&m_log_data.log_skipping);

    uint32_t           rd_idx = m_log_data.rd_idx;
    uint32_t           mask   = m_buffer_mask;
    xinc_log_header_t * p_header = (xinc_log_header_t *)&m_log_data.buffer[rd_idx & mask];
    xinc_log_header_t   header;

    // Skip packets that may be invalid (interrupted while being in progress)
    do {
        if (invalid_packets_omit(p_header, &rd_idx))
        {
            //something was omitted. Point to new header and try again.
            p_header = (xinc_log_header_t *)&m_log_data.buffer[rd_idx & mask];
        }
        else
        {
            break;
        }
    } while (true);

    uint32_t i;
    for (i = 0; i < HEADER_SIZE; i++)
    {
        ((uint32_t*)&header)[i] = m_log_data.buffer[rd_idx++ & mask];
    }

    switch (header.base.generic.type)
    {
        case HEADER_TYPE_HEXDUMP:
            dropped = header.dropped;
            rd_idx += CEIL_DIV(header.base.hexdump.len, sizeof(uint32_t));
            break;
        case HEADER_TYPE_STD:
            dropped = header.dropped;
            rd_idx += header.base.std.nargs;
            break;
        default:
            ASSERT(false);
            break;
    }

    uint32_t log_skipping_tmp = xinc_atomic_flag_clear_fetch(&m_log_data.log_skipping);
    //update read index only if log_skip was not interrupted by another log skip
    if (log_skipping_tmp)
    {
        m_log_data.rd_idx = rd_idx;
    }

    return (uint32_t)dropped;
}

/**
 * @brief Function for getting number of dropped logs. Dropped counter is reset after reading.
 *
 * @return Number of dropped logs saturated to 16 bits.
 */
uint32_t __USAT(int32_t val, uint32_t sat)
  {
    if (sat <= 31U)
    {
      const uint32_t max = ((1U << sat) - 1U);
      if (val > (int32_t)max)
      {
        return max;
      }
      else if (val < 0)
      {
        return 0U;
      }
    }
    return (uint32_t)val;
  }
	
static inline uint32_t dropped_sat16_get(void)
{
    uint32_t dropped = xinc_atomic_u32_fetch_store(&m_log_data.log_dropped_cnt, 0);
    return __USAT(dropped, 16); //Saturate to 16 bits
}


static inline void std_header_set(uint32_t severity_mid,
                                      char const * const p_str,
                                      uint32_t nargs,
                                      uint32_t wr_idx,
                                      uint32_t mask)
{


    //Prepare header - in reverse order to ensure that packet type is validated (set to STD as last action)
    uint32_t module_id = severity_mid >> XINC_LOG_MODULE_ID_POS;
    uint32_t dropped   = dropped_sat16_get();
    ASSERT(module_id < xinc_log_module_cnt_get());
    m_log_data.buffer[(wr_idx + 1) & mask] = module_id | (dropped << 16);

    if (XINC_LOG_USES_TIMESTAMP)
    {
        m_log_data.buffer[(wr_idx + 2) & mask] = m_log_data.timestamp_func();
    }

    xinc_log_header_t * p_header    = (xinc_log_header_t *)&m_log_data.buffer[wr_idx & mask];
    p_header->base.std.severity    = severity_mid & XINC_LOG_LEVEL_MASK;
    p_header->base.std.nargs       = nargs;
    p_header->base.std.addr        = ((uint32_t)(p_str) /*& STD_ADDR_MASK*/);
	//	printf("p_str:%p\r\n",p_str);
    p_header->base.std.type        = HEADER_TYPE_STD;
    p_header->base.std.in_progress = 0;
}

#if XINC_LOG_DEFERRED
/**
 * When using RTOS, application can overide this function so that it could unblock 
 * the task that is responsible for flushing the logs.
 */
__WEAK void log_pending_hook( void )
{
}
#endif

/**
 * @brief Allocates chunk in a buffer for one entry and injects overflow if
 * there is no room for requested entry.
 *
 * @param content_len   Number of 32bit arguments. In case of allocating for hex dump it
 *                      is the size of the buffer in 32bit words (ceiled).
 * @param p_wr_idx      Pointer to write index.
 *
 * @return True if successful allocation, false otherwise.
 *
 */
static inline bool buf_prealloc(uint32_t content_len, uint32_t * p_wr_idx, bool std)
{
    uint32_t req_len = content_len + HEADER_SIZE;
    bool     ret            = true;
    CRITICAL_REGION_ENTER();
    *p_wr_idx = m_log_data.wr_idx;
    uint32_t available_words = (m_buffer_mask + 1) - (m_log_data.wr_idx - m_log_data.rd_idx);
    while (req_len > available_words)
    {
        UNUSED_RETURN_VALUE(xinc_atomic_u32_add(&m_log_data.log_dropped_cnt, 1));
        if (XINC_LOG_ALLOW_OVERFLOW)
        {
            uint32_t dropped_in_skip = log_skip();
            UNUSED_RETURN_VALUE(xinc_atomic_u32_add(&m_log_data.log_dropped_cnt, dropped_in_skip));
            available_words = (m_buffer_mask + 1) - (m_log_data.wr_idx - m_log_data.rd_idx);
        }
        else
        {
            ret = false;
            break;
        }
    }

    if (ret)
    {
        xinc_log_main_header_t invalid_header;
        invalid_header.raw = 0;

        if (std)
        {
            invalid_header.std.type        = HEADER_TYPE_STD;
            invalid_header.std.in_progress = 1;
            invalid_header.std.nargs       = content_len;
        }
        else
        {
            invalid_header.hexdump.type = HEADER_TYPE_HEXDUMP;
            invalid_header.hexdump.in_progress = 1;
            invalid_header.hexdump.len = content_len;
        }

        xinc_log_main_header_t * p_header =
                   (xinc_log_main_header_t *)&m_log_data.buffer[m_log_data.wr_idx & m_buffer_mask];

        p_header->raw = invalid_header.raw;

        m_log_data.wr_idx += req_len;
    }

    CRITICAL_REGION_EXIT();
    return ret;
}

char const * xinc_log_push(char * const p_str)
{
    if ((m_log_data.autoflush) || (p_str == NULL))
    {
        return p_str;
    }

    size_t ssize = strlen(p_str) + 1; // + 1 for null termination
    uint8_t * p_dst;
    // Allocate space in the ring buffer. It may be smaller than the requested string in case of buffer wrapping or when the ring buffer size is too small.
    // Once the string is copied into the buffer, the space is immediately freed. The string is kept in the buffer but can be overwritten.
    // It is done that way because there is no other place where space could be freed since string processing happens in
    // the logger backends, often by modules which are generic and not aware of internals of the logger.
    if (xinc_ringbuf_alloc(&m_log_push_ringbuf, &p_dst, &ssize, true) == XINC_SUCCESS)
    {
        ret_code_t err;

        memcpy(p_dst, p_str, ssize);

        //Terminate in case string was partial.
        p_dst[ssize - 1] = '\0';

        err = xinc_ringbuf_put(&m_log_push_ringbuf, ssize);
        ASSERT(err == XINC_SUCCESS);

        //Immediately free the space where string was put.
        err = xinc_ringbuf_free(&m_log_push_ringbuf, ssize);
        ASSERT(err == XINC_SUCCESS);

        return (char const *)p_dst;
    }
    else
    {
        return NULL;
    }
}

static inline void std_n(uint32_t           severity_mid,
                         char const * const p_str,
                         uint32_t const *   args,
                         uint32_t           nargs)
{
    uint32_t mask   = m_buffer_mask;
    uint32_t wr_idx;
    if (buf_prealloc(nargs, &wr_idx, true))
    {
        // Proceed only if buffer was successfully preallocated.

        uint32_t data_idx = wr_idx + HEADER_SIZE;
        uint32_t i;
        for (i = 0; i < nargs; i++)
        {
            m_log_data.buffer[data_idx++ & mask] =args[i];
        }
        std_header_set(severity_mid, p_str, nargs, wr_idx, mask);

    }

    if (m_log_data.autoflush)
    {
#if XINC_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED
        CRITICAL_REGION_ENTER();
#endif // XINC_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED

        XINC_LOG_FLUSH();

#if XINC_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED
        CRITICAL_REGION_EXIT();
#endif // XINC_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED
    }

#if (XINC_LOG_DEFERRED == 1)
    log_pending_hook();
#endif

}

void xinc_log_frontend_std_0(uint32_t severity_mid, char const * const p_str)
{
    std_n(severity_mid, p_str, NULL, 0);
}


void xinc_log_frontend_std_1(uint32_t            severity_mid,
                            char const * const p_str,
                            uint32_t           val0)
{
    uint32_t args[] = {val0};
    std_n(severity_mid, p_str, args,  ARRAY_SIZE(args));
}


void xinc_log_frontend_std_2(uint32_t           severity_mid,
                            char const * const p_str,
                            uint32_t           val0,
                            uint32_t           val1)
{
    uint32_t args[] = {val0, val1};
    std_n(severity_mid, p_str, args,  ARRAY_SIZE(args));
}


void xinc_log_frontend_std_3(uint32_t           severity_mid,
                            char const * const p_str,
                            uint32_t           val0,
                            uint32_t           val1,
                            uint32_t           val2)
{
    uint32_t args[] = {val0, val1, val2};
    std_n(severity_mid, p_str, args,  ARRAY_SIZE(args));
}


void xinc_log_frontend_std_4(uint32_t           severity_mid,
                            char const * const p_str,
                            uint32_t           val0,
                            uint32_t           val1,
                            uint32_t           val2,
                            uint32_t           val3)
{
    uint32_t args[] = {val0, val1, val2, val3};
    std_n(severity_mid, p_str, args,  ARRAY_SIZE(args));
}


void xinc_log_frontend_std_5(uint32_t           severity_mid,
                            char const * const p_str,
                            uint32_t           val0,
                            uint32_t           val1,
                            uint32_t           val2,
                            uint32_t           val3,
                            uint32_t           val4)
{
    uint32_t args[] = {val0, val1, val2, val3, val4};
    std_n(severity_mid, p_str, args,  ARRAY_SIZE(args));
}


void xinc_log_frontend_std_6(uint32_t           severity_mid,
                            char const * const p_str,
                            uint32_t           val0,
                            uint32_t           val1,
                            uint32_t           val2,
                            uint32_t           val3,
                            uint32_t           val4,
                            uint32_t           val5)
{
    uint32_t args[] = {val0, val1, val2, val3, val4, val5};
    std_n(severity_mid, p_str, args,  ARRAY_SIZE(args));
}


void xinc_log_frontend_hexdump(uint32_t           severity_mid,
                              const void * const p_data,
                              uint16_t           length)
{
    uint32_t mask   = m_buffer_mask;

    uint32_t wr_idx;
    if (buf_prealloc(CEIL_DIV(length, sizeof(uint32_t)), &wr_idx, false))
    {
        uint32_t header_wr_idx = wr_idx;
        wr_idx += HEADER_SIZE;

        uint32_t space0 = sizeof(uint32_t) * (m_buffer_mask + 1 - (wr_idx & mask));
        if (length <= space0)
        {
            memcpy(&m_log_data.buffer[wr_idx & mask], p_data, length);
        }
        else
        {
            memcpy(&m_log_data.buffer[wr_idx & mask], p_data, space0);
            memcpy(&m_log_data.buffer[0], &((uint8_t *)p_data)[space0], length - space0);
        }

        //Prepare header - in reverse order to ensure that packet type is validated (set to HEXDUMP as last action)
        if (XINC_LOG_USES_TIMESTAMP)
        {
           m_log_data.buffer[(header_wr_idx + 2) & mask] = m_log_data.timestamp_func();
        }

        uint32_t module_id = severity_mid >> XINC_LOG_MODULE_ID_POS;
        uint32_t dropped   = dropped_sat16_get();
        m_log_data.buffer[(header_wr_idx + 1) & mask] = module_id | (dropped << 16);
        //Header prepare
        xinc_log_header_t * p_header = (xinc_log_header_t *)&m_log_data.buffer[header_wr_idx & mask];
        p_header->base.hexdump.severity    = severity_mid & XINC_LOG_LEVEL_MASK;
        p_header->base.hexdump.offset      = 0;
        p_header->base.hexdump.len         = length;
        p_header->base.hexdump.type        = HEADER_TYPE_HEXDUMP;
        p_header->base.hexdump.in_progress = 0;
    }

    if (m_log_data.autoflush)
    {
#if XINC_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED
        CRITICAL_REGION_ENTER();
#endif // XINC_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED

        XINC_LOG_FLUSH();

#if XINC_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED
        CRITICAL_REGION_EXIT();
#endif // XINC_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED
    }

#if (XINC_LOG_DEFERRED == 1)
    log_pending_hook();
#endif
}


bool buffer_is_empty(void)
{
    return (m_log_data.rd_idx == m_log_data.wr_idx);
}

bool xinc_log_frontend_dequeue(void)
{

    if (buffer_is_empty())
    {
        return false;
    }
    m_log_data.log_skipped      = 0;
    //It has to be ensured that reading rd_idx occurs after skipped flag is cleared.
    __DSB();
    uint32_t           rd_idx   = m_log_data.rd_idx;
    uint32_t           mask     = m_buffer_mask;
    xinc_log_header_t * p_header = (xinc_log_header_t *)&m_log_data.buffer[rd_idx & mask];
    xinc_log_header_t   header;
    xinc_memobj_t *     p_msg_buf = NULL;
    size_t             memobj_offset = 0;
    uint32_t           severity = 0;

    // Skip any in progress packets.
    do {
        if (invalid_packets_omit(p_header, &rd_idx) && (m_log_data.log_skipped == 0))
        {
            //Check if end of data is not reached.
            if (rd_idx >= m_log_data.wr_idx)
            {
                m_log_data.rd_idx     = m_log_data.wr_idx;
                return false;
            }
            //something was omitted. Point to new header and try again.
            p_header = (xinc_log_header_t *)&m_log_data.buffer[rd_idx & mask];
        }
        else
        {
            break;
        }
    } while (true);

    uint32_t i;
    for (i = 0; i < HEADER_SIZE; i++)
    {
        ((uint32_t*)&header)[i] = m_log_data.buffer[rd_idx++ & mask];
    }

    if (header.base.generic.type == HEADER_TYPE_HEXDUMP)
    {
        uint32_t orig_data_len  = header.base.hexdump.len;
        uint32_t data_len       = MIN(header.base.hexdump.len, XINC_LOG_MAX_HEXDUMP); //limit the data
        header.base.hexdump.len = data_len;
        uint32_t msg_buf_size8  = sizeof(uint32_t)*HEADER_SIZE + data_len;
        severity = header.base.hexdump.severity;
        p_msg_buf = xinc_memobj_alloc(&log_mempool, msg_buf_size8);

        if (p_msg_buf)
        {
            xinc_memobj_get(p_msg_buf);
            xinc_memobj_write(p_msg_buf, &header, HEADER_SIZE*sizeof(uint32_t), memobj_offset);
            memobj_offset += HEADER_SIZE*sizeof(uint32_t);

            uint32_t space0 = sizeof(uint32_t) * (mask + 1 - (rd_idx & mask));
            if (data_len > space0)
                    {
                uint8_t * ptr0 = space0 ?
                                 (uint8_t *)&m_log_data.buffer[rd_idx & mask] :
                                 (uint8_t *)&m_log_data.buffer[0];
                uint8_t   len0 = space0 ? space0 : data_len;
                uint8_t * ptr1 = space0 ?
                                 (uint8_t *)&m_log_data.buffer[0] : NULL;
                uint8_t len1 = space0 ? data_len - space0 : 0;

                xinc_memobj_write(p_msg_buf, ptr0, len0, memobj_offset);
                memobj_offset += len0;
                if (ptr1)
                {
                    xinc_memobj_write(p_msg_buf, ptr1, len1, memobj_offset);
                }
            }
            else
            {
                uint8_t * p_data = (uint8_t *)&m_log_data.buffer[rd_idx & mask];
                xinc_memobj_write(p_msg_buf, p_data, data_len, memobj_offset);
            }
            rd_idx += CEIL_DIV(orig_data_len, 4);
        }
    }
    else if (header.base.generic.type == HEADER_TYPE_STD) // standard entry
    {
        header.base.std.nargs = MIN(header.base.std.nargs, XINC_LOG_MAX_NUM_OF_ARGS);
        uint32_t msg_buf_size32 = HEADER_SIZE + header.base.std.nargs;
        severity = header.base.std.severity;

        p_msg_buf = xinc_memobj_alloc(&log_mempool, msg_buf_size32*sizeof(uint32_t));
        if (p_msg_buf)
        {
            xinc_memobj_get(p_msg_buf);
            xinc_memobj_write(p_msg_buf, &header, HEADER_SIZE*sizeof(uint32_t), memobj_offset);
            memobj_offset += HEADER_SIZE*sizeof(uint32_t);

            for (i = 0; i < header.base.std.nargs; i++)
            {
                xinc_memobj_write(p_msg_buf, &m_log_data.buffer[rd_idx++ & mask],
                                 sizeof(uint32_t), memobj_offset);
                memobj_offset += sizeof(uint32_t);
            }
        }
    }
    else
    {
        //Do nothing. In case of log overflow buffer can contain corrupted data.
    }

    if (p_msg_buf)
    {
        xinc_log_backend_t const * p_backend = m_log_data.p_backend_head;
        if (XINC_LOG_ALLOW_OVERFLOW && m_log_data.log_skipped)
        {
            // Check if any log was skipped during log processing. Do not forward log if skipping 
            // occured because data may be invalid.

            xinc_memobj_put(p_msg_buf);
        }
        else
        {
            while (p_backend)
            {
                bool entry_accepted = false;
                if (xinc_log_backend_is_enabled(p_backend) == true)
                {
                    if (XINC_LOG_FILTERS_ENABLED)
                    {
                        uint8_t backend_id = xinc_log_backend_id_get(p_backend);
                        xinc_log_module_filter_data_t * p_module_filter =
                                                 XINC_LOG_FILTER_SECTION_VARS_GET(header.module_id);
                        uint32_t backend_lvl = BF_GET(p_module_filter->filter_lvls,
                                                      XINC_LOG_LEVEL_BITS,
                                                      (backend_id*XINC_LOG_LEVEL_BITS));

                        //Degrade INFO_RAW level to INFO.
                        severity = (severity == XINC_LOG_SEVERITY_INFO_RAW) ?
                                                                 XINC_LOG_SEVERITY_INFO : severity;
                        if (backend_lvl >= severity)
                        {
                            entry_accepted = true;
                        }
                    }
                    else
                    {
                        (void)severity;
                        entry_accepted = true;
                    }
                }
                if (entry_accepted)
                {
                    xinc_log_backend_put(p_backend, p_msg_buf);
                }
                p_backend = p_backend->p_cb->p_next;
            }

            xinc_memobj_put(p_msg_buf);

            if (XINC_LOG_ALLOW_OVERFLOW)
            {
                // Read index can be moved forward only if dequeueing process was not interrupt by
                // skipping procedure. If XINC_LOG_ALLOW_OVERFLOW is set then in case of buffer gets full
                // and new logger entry occurs, oldest entry is removed. In that case read index is
                // changed and updating it here would corrupt the internal circular buffer.
                CRITICAL_REGION_ENTER();
                if (m_log_data.log_skipped == 0)
                {
                    m_log_data.rd_idx = rd_idx;
                }
                CRITICAL_REGION_EXIT();
            }
            else
            {
                m_log_data.rd_idx = rd_idx;
            }
        }
    }
    else
    {
        //Could not allocate memobj - backends are not freeing them on time.
        xinc_log_backend_t const * p_backend = m_log_data.p_backend_head;
        //Flush all backends
        while (p_backend)
        {
            xinc_log_backend_flush(p_backend);
            p_backend = p_backend->p_cb->p_next;
        }
        XINC_LOG_WARNING("Backends flushed");
    }

    return buffer_is_empty() ? false : true;
}

static int32_t backend_id_assign(void)
{
    int32_t candidate_id;
    xinc_log_backend_t const * p_backend;
    bool id_available;
    for (candidate_id = 0; candidate_id < XINC_LOG_MAX_BACKENDS; candidate_id++)
    {
        p_backend = m_log_data.p_backend_head;
        id_available = true;
        while (p_backend)
        {
            if (xinc_log_backend_id_get(p_backend) == candidate_id)
            {
                id_available = false;
                break;
            }
            p_backend = p_backend->p_cb->p_next;
        }
        if (id_available)
        {
            return candidate_id;
        }
    }
    return -1;
}

int32_t xinc_log_backend_add(xinc_log_backend_t const * p_backend, xinc_log_severity_t severity)
{
    int32_t id = backend_id_assign();
    if (id == -1)
    {
        return id;
    }

    xinc_log_backend_id_set(p_backend, id);
    //add to list
    if (m_log_data.p_backend_head == NULL)
    {
       m_log_data.p_backend_head   = p_backend;
       p_backend->p_cb->p_next = NULL;
    }
    else
    {
        p_backend->p_cb->p_next = m_log_data.p_backend_head->p_cb->p_next;
        m_log_data.p_backend_head->p_cb->p_next = p_backend;
    }

    if (XINC_LOG_FILTERS_ENABLED)
    {
        uint32_t i;
        for (i = 0; i < xinc_log_module_cnt_get(); i++)
        {
            xinc_log_severity_t buildin_lvl = xinc_log_module_init_filter_get(i);
            xinc_log_severity_t actual_severity = MIN(buildin_lvl, severity);
            xinc_log_module_filter_set(xinc_log_backend_id_get(p_backend), i, actual_severity);
        }
    }

    return id;
}

void xinc_log_backend_remove(xinc_log_backend_t const * p_backend)
{
    xinc_log_backend_t const * p_curr = m_log_data.p_backend_head;
    xinc_log_backend_t const * p_prev = NULL;
    while (p_curr != p_backend)
    {
        p_prev = p_curr;
        p_curr = p_curr->p_cb->p_next;
    }

    if (p_prev)
    {
        p_prev->p_cb->p_next = p_backend->p_cb->p_next;
    }
    else
    {
        m_log_data.p_backend_head = NULL;
    }

    p_backend->p_cb->id = XINC_LOG_BACKEND_INVALID_ID;
}

void xinc_log_panic(void)
{
    xinc_log_backend_t const * p_backend = m_log_data.p_backend_head;
    m_log_data.autoflush = true;
    while (p_backend)
    {
        xinc_log_backend_enable(p_backend);
        xinc_log_backend_panic_set(p_backend);
        p_backend = p_backend->p_cb->p_next;
    }
}

#if XINC_MODULE_ENABLED(LOG_CONFIG_LOAD_STORE)
#include "fds.h"
#define LOG_CONFIG_FILE_ID   0x106E
#define LOG_CONFIG_RECORD_ID 0x3427

ret_code_t xinc_log_config_store(void)
{
    fds_record_desc_t desc = {0};
    fds_find_token_t  token = {0};
    fds_record_t      record = {
            .file_id = LOG_CONFIG_FILE_ID,
            .key     = LOG_CONFIG_RECORD_ID,
            .data = {
                    .p_data       = XINC_LOG_FILTER_SECTION_VARS_GET(0),
                    .length_words = XINC_SECTION_LENGTH(log_filter_data)/sizeof(uint32_t)
            }
    };
    ret_code_t ret = fds_record_find(LOG_CONFIG_FILE_ID, LOG_CONFIG_RECORD_ID, &desc, &token);
    if (ret == XINC_SUCCESS)
    {
        ret = fds_record_update(&desc, &record);
        XINC_LOG_INFO("Logger configuration file updated with result:%d", ret);
    }
    else if (ret == FDS_ERR_NOT_FOUND)
    {
        ret = fds_record_write(&desc, &record);
        XINC_LOG_INFO("Logger configuration file written with result:%d", ret);
    }
    else
    {
        ret = XINC_ERROR_INTERNAL;
    }
    return ret;
}

ret_code_t xinc_log_config_load(void)
{
    fds_record_desc_t desc = {0};
    fds_find_token_t  token = {0};

    ret_code_t ret = fds_record_find(LOG_CONFIG_FILE_ID, LOG_CONFIG_RECORD_ID, &desc, &token);
    if (ret == XINC_SUCCESS)
    {
        fds_flash_record_t record = {0};
        ret = fds_record_open(&desc, &record);
        if (ret == XINC_SUCCESS)
        {
            void * p_dest = (void *)XINC_LOG_FILTER_SECTION_VARS_GET(0);
            uint32_t length   = XINC_SECTION_LENGTH(log_filter_data);
            memcpy(p_dest, record.p_data, length);
            ret = fds_record_close(&desc);
        }
    }
    else if (ret == FDS_ERR_NOT_FOUND)
    {
        XINC_LOG_WARNING("Logger configuration file not found.");
        ret = XINC_ERROR_NOT_FOUND;
    }
    else
    {
        ret = XINC_ERROR_INTERNAL;
    }

    return ret;
}
#endif //LOG_CONFIG_LOAD_STORE_ENABLED

#if XINC_LOG_CLI_CMDS && XINC_CLI_ENABLED
#include "xinc_cli.h"

typedef void (*xinc_log_cli_backend_cmd_t)(xinc_cli_t const *         p_cli,
                                          xinc_log_backend_t const * p_backend,
                                          size_t                    argc,
                                          char * *                  argv);

static const char * m_severity_lvls[] = {
        "none",
        "error",
        "warning",
        "info",
        "debug",
};

static const char * m_severity_lvls_sorted[] = {
        "debug",
        "error",
        "info",
        "none",
        "warning",
};

/**
 * @brief Function for finding backend instance with given name.
 *
 * @param p_name Name of the backend instance.
 *
 * @return Pointer to the instance or NULL.
 *
 */
static xinc_log_backend_t const * backend_find(char const * p_name)
{
    size_t num_of_backends;
    xinc_log_backend_t const * p_backend;

    num_of_backends = XINC_SECTION_ITEM_COUNT(XINC_LOG_BACKEND_SECTION_NAME, xinc_log_backend_t);
    for (size_t i = 0; i < num_of_backends; i++)
    {
        p_backend = XINC_SECTION_ITEM_GET(XINC_LOG_BACKEND_SECTION_NAME, xinc_log_backend_t, i);
        if (strcmp(p_name, p_backend->p_name) == 0)
        {
            return p_backend;
        }
    }
    return NULL;
}

/**
 * @brief Function for executing command on given backend.
 */
static void xinc_cli_backend_cmd_execute(xinc_cli_t const *         p_cli,
                                        size_t                    argc,
                                        char * *                  argv,
                                        xinc_log_cli_backend_cmd_t func)
{
    //Based on the structure of backend commands, name of the backend can be found at -1 (log backend <name> command).
    char const * p_backend_name = argv[-1];

    xinc_log_backend_t const * p_backend = backend_find(p_backend_name);

    if (p_backend)
    {
        func(p_cli, p_backend, argc, argv);
    }
    else
    {
        xinc_cli_fprintf(p_cli, XINC_CLI_ERROR, "Invalid backend: %s\r\n", p_backend_name);
    }
}


static void log_status(xinc_cli_t const *         p_cli,
                       xinc_log_backend_t const * p_backend,
                       size_t                    argc,
                       char * *                  argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    uint32_t modules_cnt = xinc_log_module_cnt_get();
    uint32_t i;


    if (!xinc_log_backend_is_enabled(p_backend))
    {
        xinc_cli_fprintf(p_cli, XINC_CLI_ERROR, "Logs are halted!\r\n");
    }
    xinc_cli_fprintf(p_cli, XINC_CLI_NORMAL, "%-40s | current | built-in \r\n", "module_name");
    xinc_cli_fprintf(p_cli, XINC_CLI_NORMAL, "----------------------------------------------------------\r\n");
    for (i = 0; i < modules_cnt; i++)
    {
        uint32_t backend_id = p_backend->p_cb->id;
        xinc_log_severity_t module_dynamic_lvl =
                              xinc_log_module_filter_get(backend_id, i, true, true);
        xinc_log_severity_t module_compiled_lvl =
                              xinc_log_module_filter_get(backend_id, i, true, false);
        xinc_log_severity_t actual_compiled_lvl =
                              MIN(module_compiled_lvl, (xinc_log_severity_t)XINC_LOG_DEFAULT_LEVEL);

        xinc_cli_fprintf(p_cli, XINC_CLI_NORMAL, "%-40s | %-7s | %s%s\r\n",
                                  xinc_log_module_name_get(i, true),
                                  m_severity_lvls[module_dynamic_lvl],
                                  m_severity_lvls[actual_compiled_lvl],
                                  actual_compiled_lvl < module_compiled_lvl ? "*" : "");
    }
}


static void log_self_status(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    log_status(p_cli, p_cli->p_log_backend, argc, argv);
}


static void log_backend_status(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    xinc_cli_backend_cmd_execute(p_cli, argc, argv, log_status);
}


static bool module_id_get(const char * p_name, uint32_t * p_id)
{
    uint32_t modules_cnt = xinc_log_module_cnt_get();
    const char * p_tmp_name;
    uint32_t j;
    for (j = 0; j < modules_cnt; j++)
    {
        p_tmp_name = xinc_log_module_name_get(j, false);
        if (strncmp(p_tmp_name, p_name, 32) == 0)
        {
            *p_id = j;
            break;
        }
    }
    return (j != modules_cnt);
}


static bool module_id_filter_set(uint32_t backend_id,
                                 uint32_t module_id,
                                 xinc_log_severity_t lvl)
{
    xinc_log_severity_t buildin_lvl = xinc_log_module_filter_get(backend_id, module_id, false, false);
    if (lvl > buildin_lvl)
    {
        return false;
    }
    else
    {
        xinc_log_module_filter_set(backend_id, module_id, lvl);
        return true;
    }
}


static void log_ctrl(xinc_cli_t const *         p_cli,
                     xinc_log_backend_t const * p_backend,
                     size_t                    argc,
                     char * *                  argv)
{
    xinc_log_severity_t lvl;
    uint32_t first_m_name_idx;
    uint32_t i;
    bool     all_modules = false;
    uint32_t backend_id = p_backend->p_cb->id;

    if (argc >  0)
    {
        if (strncmp(argv[0], "enable", 7) == 0)
        {
            if (argc == 1)
            {
                xinc_cli_fprintf(p_cli, XINC_CLI_ERROR, "Bad parameter count.\r\n");
                return;
            }

            if (argc == 2)
            {
                all_modules = true;
            }

            for (i = 0; i < ARRAY_SIZE(m_severity_lvls); i++)
            {
                if (strncmp(argv[1], m_severity_lvls[i], 10) == 0)
                {
                    break;
                }
            }

            if (i == ARRAY_SIZE(m_severity_lvls))
            {
                xinc_cli_fprintf(p_cli, XINC_CLI_ERROR, "Unknown severity level: %s\r\n", argv[1]);
                return;
            }

            lvl = (xinc_log_severity_t)i;
            first_m_name_idx = 2;

        }
        else if (strncmp(argv[0], "disable", 8) == 0)
        {
            if (argc == 1)
            {
                all_modules = true;
            }
            lvl = XINC_LOG_SEVERITY_NONE;
            first_m_name_idx = 1;
        }
        else
        {
            xinc_cli_fprintf(p_cli, XINC_CLI_ERROR, "Unknown option: %s\r\n", argv[0]);
            return;
        }

        if (all_modules)
        {
            for (i = 0; i < xinc_log_module_cnt_get(); i++)
            {
                if (module_id_filter_set(backend_id, i, lvl) == false)
                {
                    xinc_cli_fprintf(p_cli, XINC_CLI_ERROR,
                                    "Level unavailable for module: %s\r\n",
                                    xinc_log_module_name_get(i, false));
                }
            }
        }
        else
        {
            for (i = first_m_name_idx; i < argc; i++)
            {
                uint32_t module_id = 0;
                if (module_id_get(argv[i], &module_id) == false)
                {
                    xinc_cli_fprintf(p_cli, XINC_CLI_ERROR, "Unknown module:%s\r\n", argv[i]);
                }
                else if (module_id_filter_set(backend_id, module_id, lvl) == false)
                {
                    xinc_cli_fprintf(p_cli, XINC_CLI_ERROR,
                                    "Level unavailable for module: %s\r\n",
                                    xinc_log_module_name_get(module_id, false));
                }
                else
                {
                    /* empty */
                }
            }
        }
    }
}


static void log_self_ctrl(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    log_ctrl(p_cli, p_cli->p_log_backend, argc, argv);
}


static void log_backend_ctrl(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    xinc_cli_backend_cmd_execute(p_cli, argc, argv, log_ctrl);
}

static void module_name_get(size_t idx, xinc_cli_static_entry_t * p_static);

XINC_CLI_CREATE_DYNAMIC_CMD(m_module_name, module_name_get);

static void module_name_get(size_t idx, xinc_cli_static_entry_t * p_static)
{
    p_static->handler = NULL;
    p_static->p_help  = NULL;
    p_static->p_subcmd = &m_module_name;
    p_static->p_syntax = xinc_log_module_name_get(idx, true);
}


static void severity_lvl_get(size_t idx, xinc_cli_static_entry_t * p_static)
{
    p_static->handler = NULL;
    p_static->p_help  = NULL;
    p_static->p_subcmd = &m_module_name;
    p_static->p_syntax = (idx < ARRAY_SIZE(m_severity_lvls_sorted)) ?
                                                    m_severity_lvls_sorted[idx] : NULL;
}

XINC_CLI_CREATE_DYNAMIC_CMD(m_severity_lvl, severity_lvl_get);


static void log_halt(xinc_cli_t const *         p_cli,
                     xinc_log_backend_t const * p_backend,
                     size_t                    argc,
                     char * *                  argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    xinc_log_backend_disable(p_backend);
}


static void log_backend_halt(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    xinc_cli_backend_cmd_execute(p_cli, argc, argv, log_halt);
}


static void log_self_halt(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    log_halt(p_cli, p_cli->p_log_backend, argc, argv);
}


static void log_go(xinc_cli_t const *         p_cli,
                   xinc_log_backend_t const * p_backend,
                   size_t                    argc,
                   char * *                  argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    xinc_log_backend_enable(p_backend);
}


static void log_backend_go(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    xinc_cli_backend_cmd_execute(p_cli, argc, argv, log_go);
}


static void log_self_go(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    log_go(p_cli, p_cli->p_log_backend, argc, argv);
}


static void log_cmd_backends_list(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    if (xinc_cli_help_requested(p_cli))
    {
        xinc_cli_help_print(p_cli, NULL, 0);
        return;
    }
    size_t num_of_backends;

    num_of_backends = XINC_SECTION_ITEM_COUNT(XINC_LOG_BACKEND_SECTION_NAME, xinc_log_backend_t);
    for (size_t i = 0; i < num_of_backends; i++)
    {
        xinc_log_backend_t const * p_backend =
                           XINC_SECTION_ITEM_GET(XINC_LOG_BACKEND_SECTION_NAME, xinc_log_backend_t, i);

        if (p_backend->p_cb->id == XINC_LOG_BACKEND_INVALID_ID)
        {
            xinc_cli_fprintf(p_cli, XINC_CLI_NORMAL,
                            "%s\r\n"
                            "\t- Status: deactivated\r\n\r\n",
                            p_backend->p_name);
        }
        else
        {
            xinc_cli_fprintf(p_cli, XINC_CLI_NORMAL,
                            "%s\r\n"
                            "\t- Status: %s\r\n"
                            "\t- ID: %d\r\n\r\n",
                            p_backend->p_name,
                            p_backend->p_cb->enabled ? "enabled" : "disabled",
                            p_backend->p_cb->id);
        }
    }
}


XINC_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_log_backend)
{
    XINC_CLI_CMD(disable, &m_module_name,
        "'log disable <module_0> .. <module_n>' disables logs in specified "
        "modules (all if no modules specified).",
        log_backend_ctrl),
    XINC_CLI_CMD(enable, &m_severity_lvl,
        "'log enable <level> <module_0> ...  <module_n>' enables logs up to given level in "
        "specified modules (all if no modules specified).",
        log_backend_ctrl),
    XINC_CLI_CMD(go, NULL, "Resume logging", log_backend_go),
    XINC_CLI_CMD(halt, NULL, "Halt logging", log_backend_halt),
    XINC_CLI_CMD(status, NULL, "Logger status", log_backend_status),
    XINC_CLI_SUBCMD_SET_END
};

static void backend_name_get(size_t idx, xinc_cli_static_entry_t * p_static)
{
    p_static->handler = NULL;
    p_static->p_help  = NULL;
    p_static->p_subcmd = &m_sub_log_backend;
    p_static->p_syntax  = NULL;

    xinc_log_backend_t const * p_backend;
    size_t                    active_idx = 0;
    uint32_t                  i;

    for (i = 0; i < XINC_SECTION_ITEM_COUNT(XINC_LOG_BACKEND_SECTION_NAME, xinc_log_backend_t); i++)
    {
        p_backend = XINC_SECTION_ITEM_GET(XINC_LOG_BACKEND_SECTION_NAME, xinc_log_backend_t, i);
        if (p_backend->p_cb->id != XINC_LOG_BACKEND_INVALID_ID)
        {
            if (idx == active_idx)
            {
                p_static->p_syntax = p_backend->p_name;
                break;
            }
            else
            {
                active_idx++;
            }
        }
    }
}

XINC_CLI_CREATE_DYNAMIC_CMD(m_backend_name_dynamic, backend_name_get);

static void log_config_load_cmd(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    if (xinc_cli_help_requested(p_cli))
    {
        xinc_cli_help_print(p_cli, NULL, 0);
        return;
    }

#if LOG_CONFIG_LOAD_STORE_ENABLED
    if (xinc_log_config_load() == XINC_SUCCESS)
    {
        xinc_cli_fprintf(p_cli, XINC_CLI_NORMAL, "Configuration loaded.\r\n");
    }
    else
    {
        xinc_cli_fprintf(p_cli, XINC_CLI_ERROR, "Failed to load the configuration.\r\n");
    }
#else
    xinc_cli_fprintf(p_cli, XINC_CLI_ERROR, "Not supported.\r\n");
#endif
}

static void log_config_store_cmd(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    if (xinc_cli_help_requested(p_cli))
    {
        xinc_cli_help_print(p_cli, NULL, 0);
        return;
    }

#if LOG_CONFIG_LOAD_STORE_ENABLED
    if (xinc_log_config_store() == XINC_SUCCESS)
    {
        xinc_cli_fprintf(p_cli, XINC_CLI_NORMAL, "Configuration stored.\r\n");
    }
    else
    {
        xinc_cli_fprintf(p_cli, XINC_CLI_ERROR, "Failed to store the configuration.\r\n");
    }
#else
    xinc_cli_fprintf(p_cli, XINC_CLI_ERROR, "Not supported.\r\n");
#endif
}

XINC_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_log_config)
{
    XINC_CLI_CMD(load, NULL, "Load configuration stored in non-volatile memory.", log_config_load_cmd),
    XINC_CLI_CMD(store, NULL, "Store current configuration in non-volatile memory.", log_config_store_cmd),
    XINC_CLI_SUBCMD_SET_END
};

XINC_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_log_stat)
{
    XINC_CLI_CMD(backend, &m_backend_name_dynamic, "Logger backends commands.", NULL),
    XINC_CLI_CMD(config, &m_sub_log_config, "Manage logger configuration", NULL),
    XINC_CLI_CMD(disable, &m_module_name,
        "'log disable <module_0> .. <module_n>' disables logs in specified "
        "modules (all if no modules specified).",
        log_self_ctrl),
    XINC_CLI_CMD(enable, &m_severity_lvl,
        "'log enable <level> <module_0> ...  <module_n>' enables logs up to given level in "
        "specified modules (all if no modules specified).",
        log_self_ctrl),
    XINC_CLI_CMD(go, NULL, "Resume logging", log_self_go),
    XINC_CLI_CMD(halt, NULL, "Halt logging", log_self_halt),
    XINC_CLI_CMD(list_backends, NULL, "Lists logger backends.", log_cmd_backends_list),
    XINC_CLI_CMD(status, NULL, "Logger status", log_self_status),
    XINC_CLI_SUBCMD_SET_END
};

static void log_cmd(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    if ((argc == 1) || xinc_cli_help_requested(p_cli))
    {
        xinc_cli_help_print(p_cli, NULL, 0);
        return;
    }

    xinc_cli_fprintf(p_cli, XINC_CLI_ERROR, "%s:%s%s\r\n", argv[0], " unknown parameter: ", argv[1]);
}

XINC_CLI_CMD_REGISTER(log, &m_sub_log_stat, "Commands for controlling logger", log_cmd);

#endif //XINC_LOG_CLI_CMDS

#endif // XINC_MODULE_ENABLED(XINC_LOG)
