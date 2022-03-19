/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "sdk_common.h"
#if XINC_MODULE_ENABLED(XINC_QUEUE)
#include "xinc_queue.h"
#include "app_util_platform.h"

#if XINC_QUEUE_CONFIG_LOG_ENABLED
    #define XINC_LOG_LEVEL             XINC_QUEUE_CONFIG_LOG_LEVEL
    #define XINC_LOG_INIT_FILTER_LEVEL XINC_QUEUE_CONFIG_LOG_INIT_FILTER_LEVEL
    #define XINC_LOG_INFO_COLOR        XINC_QUEUE_CONFIG_INFO_COLOR
    #define XINC_LOG_DEBUG_COLOR       XINC_QUEUE_CONFIG_DEBUG_COLOR
#else
    #define XINC_LOG_LEVEL       0
#endif // XINC_QUEUE_CONFIG_LOG_ENABLED
#include "xinc_log.h"

XINC_SECTION_DEF(xinc_queue, xinc_queue_t);

#if XINC_QUEUE_CLI_CMDS && XINC_CLI_ENABLED
#include "xinc_cli.h"

static void xinc_queue_status(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    UNUSED_PARAMETER(argv);

    if (xinc_cli_help_requested(p_cli))
    {
        xinc_cli_help_print(p_cli, NULL, 0);
        return;
    }

    if (argc > 1)
    {
        xinc_cli_fprintf(p_cli, XINC_CLI_ERROR, "Bad argument count");
        return;
    }

    uint32_t num_of_instances = XINC_SECTION_ITEM_COUNT(xinc_queue, xinc_queue_t);
    uint32_t i;

    for (i = 0; i < num_of_instances; i++)
    {
        const xinc_queue_t * p_instance = XINC_SECTION_ITEM_GET(xinc_queue, xinc_queue_t, i);

        uint32_t element_size = p_instance->element_size;
        uint32_t size         = p_instance->size;
        uint32_t max_util     = xinc_queue_max_utilization_get(p_instance);
        uint32_t util         = xinc_queue_utilization_get(p_instance);
        const char * p_name   = p_instance->p_name;
        xinc_cli_fprintf(p_cli, XINC_CLI_NORMAL,
                        "%s\r\n\t- Element size:\t%d\r\n"
                        "\t- Usage:\t%u%% (%u out of %u elements)\r\n"
                        "\t- Maximum:\t%u%% (%u out of %u elements)\r\n"
                        "\t- Mode:\t\t%s\r\n\r\n",
                        p_name, element_size,
                        100ul * util/size, util,size,
                        100ul * max_util/size, max_util,size,
                        (p_instance->mode == XINC_QUEUE_MODE_OVERFLOW) ? "Overflow" : "No overflow");

    }
}
// Register "queue" command and its subcommands in CLI.
XINC_CLI_CREATE_STATIC_SUBCMD_SET(xinc_queue_commands)
{
     XINC_CLI_CMD(status, NULL, "Print status of queue instances.", xinc_queue_status),
     XINC_CLI_SUBCMD_SET_END
};

XINC_CLI_CMD_REGISTER(queue, &xinc_queue_commands, "Commands for BALLOC management", xinc_queue_status);
#endif //XINC_QUEUE_CLI_CMDS

__STATIC_INLINE size_t circullar_buffer_size_get(xinc_queue_t const * p_queue)
{
    static const uint8_t full_queue_indicator = 1;

    /* When a queue is implemented as a cyclic buffer, it is not possible to
     * distinguish a full queue from an empty queue. In order to solve this
     * problem, the cyclic buffer has been implemented one element larger than
     * the queue size.
     */
    return p_queue->size + full_queue_indicator;
}

/**@brief Get next element index.
 *
 * @param[in]   p_queue     Pointer to the queue instance.
 * @param[in]   idx         Current index.
 *
 * @return      Next element index.
 */
__STATIC_INLINE size_t xinc_queue_next_idx(xinc_queue_t const * p_queue, size_t idx)
{
    ASSERT(p_queue != NULL);
    return (idx < p_queue->size) ? (idx + 1) : 0;
}

/**@brief Get current queue utilization. This function assumes that this process will not be interrupted.
 *
 * @param[in]   p_queue     Pointer to the queue instance.
 *
 * @return      Current queue utilization.
 */
__STATIC_INLINE size_t queue_utilization_get(xinc_queue_t const * p_queue)
{
    size_t front    = p_queue->p_cb->front;
    size_t back     = p_queue->p_cb->back;

    return (back >= front) ? (back - front) :
        (circullar_buffer_size_get(p_queue) - front + back);
}

bool xinc_queue_is_full(xinc_queue_t const * p_queue)
{
    ASSERT(p_queue != NULL);
    size_t front    = p_queue->p_cb->front;
    size_t back     = p_queue->p_cb->back;

    return (xinc_queue_next_idx(p_queue, back) == front);
}

ret_code_t xinc_queue_push(xinc_queue_t const * p_queue, void const * p_element)
{
    ret_code_t status = XINC_SUCCESS;

    ASSERT(p_queue != NULL);
    ASSERT(p_element != NULL);

    CRITICAL_REGION_ENTER();
    bool is_full = xinc_queue_is_full(p_queue);

    if (!is_full || (p_queue->mode == XINC_QUEUE_MODE_OVERFLOW))
    {
        // Get write position.
        size_t write_pos = p_queue->p_cb->back;
        p_queue->p_cb->back = xinc_queue_next_idx(p_queue, p_queue->p_cb->back);
        if (is_full)
        {
            // Overwrite the oldest element.
            XINC_LOG_INST_WARNING(p_queue->p_log, "Queue full. Overwriting oldest element.");
            p_queue->p_cb->front = xinc_queue_next_idx(p_queue, p_queue->p_cb->front);
        }

        // Write a new element.
        switch (p_queue->element_size)
        {
            case sizeof(uint8_t):
                ((uint8_t *)p_queue->p_buffer)[write_pos] = *((uint8_t *)p_element);
                break;

            case sizeof(uint16_t):
                ((uint16_t *)p_queue->p_buffer)[write_pos] = *((uint16_t *)p_element);
                break;

            case sizeof(uint32_t):
                ((uint32_t *)p_queue->p_buffer)[write_pos] = *((uint32_t *)p_element);
                break;

            case sizeof(uint64_t):
                ((uint64_t *)p_queue->p_buffer)[write_pos] = *((uint64_t *)p_element);
                break;

            default:
                memcpy((void *)((size_t)p_queue->p_buffer + write_pos * p_queue->element_size),
                       p_element,
                       p_queue->element_size);
                break;
        }

        // Update utilization.
        size_t utilization = queue_utilization_get(p_queue);
        if (p_queue->p_cb->max_utilization < utilization)
        {
            p_queue->p_cb->max_utilization = utilization;
        }
    }
    else
    {
        status = XINC_ERROR_NO_MEM;
    }

    CRITICAL_REGION_EXIT();

    XINC_LOG_INST_DEBUG(p_queue->p_log, "pushed element 0x%08X, status:%d", p_element, status);
    return status;
}

ret_code_t xinc_queue_generic_pop(xinc_queue_t const * p_queue,
                                 void              * p_element,
                                 bool                just_peek)
{
    ret_code_t status = XINC_SUCCESS;

    ASSERT(p_queue      != NULL);
    ASSERT(p_element    != NULL);

    CRITICAL_REGION_ENTER();

    if (!xinc_queue_is_empty(p_queue))
    {
        // Get read position.
        size_t read_pos = p_queue->p_cb->front;

        // Update next read position.
        if (!just_peek)
        {
            p_queue->p_cb->front = xinc_queue_next_idx(p_queue, p_queue->p_cb->front);
        }

        // Read element.
        switch (p_queue->element_size)
        {
            case sizeof(uint8_t):
                *((uint8_t *)p_element) = ((uint8_t *)p_queue->p_buffer)[read_pos];
                break;

            case sizeof(uint16_t):
                *((uint16_t *)p_element) = ((uint16_t *)p_queue->p_buffer)[read_pos];
                break;

            case sizeof(uint32_t):
                *((uint32_t *)p_element) = ((uint32_t *)p_queue->p_buffer)[read_pos];
                break;

            case sizeof(uint64_t):
                *((uint64_t *)p_element) = ((uint64_t *)p_queue->p_buffer)[read_pos];
                break;

            default:
                memcpy(p_element,
                       (void const *)((size_t)p_queue->p_buffer + read_pos * p_queue->element_size),
                       p_queue->element_size);
                break;
        }
    }
    else
    {
        status = XINC_ERROR_NOT_FOUND;
    }

    CRITICAL_REGION_EXIT();
    XINC_LOG_INST_DEBUG(p_queue->p_log, "%s element 0x%08X, status:%d",
                                         just_peek ? "peeked" : "popped", p_element, status);
    return status;
}

/* Purpose of this function is to provide number of continous bytes in the queue's
 * array before circullar buffer needs to wrapp.
 */
static size_t continous_items_get(xinc_queue_t const * p_queue, bool write)
{
    size_t front    = p_queue->p_cb->front;
    size_t back     = p_queue->p_cb->back;

    /* Number of continous items for queue write operation */
    if (write)
    {
        return (back >= front) ? circullar_buffer_size_get(p_queue) - back : front - back;
    }
    else
    {
        return (back >= front) ? back - front : circullar_buffer_size_get(p_queue) - front;
    }
}

/**@brief Write elements to the queue. This function assumes that there is enough room in the queue
 *        to write the requested number of elements and that this process will not be interrupted.
 *
 * @param[in]   p_queue             Pointer to the xinc_queue_t instance.
 * @param[in]   p_data              Pointer to the buffer with elements to write.
 * @param[in]   element_count       Number of elements to write.
 */
static void queue_write(xinc_queue_t const * p_queue, void const * p_data, uint32_t element_count)
{
    size_t prev_available = xinc_queue_available_get(p_queue);
    size_t continuous     = continous_items_get(p_queue, true);
    void * p_write_ptr    = (void *)((size_t)p_queue->p_buffer
                          + p_queue->p_cb->back * p_queue->element_size);

    if (element_count <= continuous)
    {
        memcpy(p_write_ptr,
               p_data,
               element_count * p_queue->element_size);

        p_queue->p_cb->back = ((p_queue->p_cb->back + element_count) <= p_queue->size)
                            ? (p_queue->p_cb->back + element_count)
                            : 0;
    }
    else
    {
        size_t first_write_length = continuous * p_queue->element_size;
        memcpy(p_write_ptr,
               p_data,
               first_write_length);

        size_t elements_left = element_count - continuous;
        memcpy(p_queue->p_buffer,
               (void const *)((size_t)p_data + first_write_length),
               elements_left * p_queue->element_size);

        p_queue->p_cb->back = elements_left;
        if (prev_available < element_count)
        {
            // Overwrite the oldest elements.
            p_queue->p_cb->front = xinc_queue_next_idx(p_queue, p_queue->p_cb->back);
        }
    }

    // Update utilization.
    size_t utilization = queue_utilization_get(p_queue);
    if (p_queue->p_cb->max_utilization < utilization)
    {
        p_queue->p_cb->max_utilization = utilization;
    }
}

ret_code_t xinc_queue_write(xinc_queue_t const * p_queue,
                           void const        * p_data,
                           size_t              element_count)
{
    ret_code_t status = XINC_SUCCESS;

    ASSERT(p_queue != NULL);
    ASSERT(p_data != NULL);
    ASSERT(element_count <= p_queue->size);

    if (element_count == 0)
    {
        return XINC_SUCCESS;
    }

    CRITICAL_REGION_ENTER();

    if ((xinc_queue_available_get(p_queue) >= element_count)
     || (p_queue->mode == XINC_QUEUE_MODE_OVERFLOW))
    {
        queue_write(p_queue, p_data, element_count);
    }
    else
    {
        status = XINC_ERROR_NO_MEM;
    }

    CRITICAL_REGION_EXIT();

    XINC_LOG_INST_DEBUG(p_queue->p_log, "Write %d elements (start address: 0x%08X), status:%d",
                                       element_count, p_data, status);
    return status;
}


size_t xinc_queue_in(xinc_queue_t const * p_queue,
                    void const        * p_data,
                    size_t              element_count)
{
    ASSERT(p_queue != NULL);
    ASSERT(p_data != NULL);

    size_t req_element_count = element_count;

    if (element_count == 0)
    {
        return 0;
    }

    CRITICAL_REGION_ENTER();

    if (p_queue->mode == XINC_QUEUE_MODE_OVERFLOW)
    {
        element_count = MIN(element_count, p_queue->size);
    }
    else
    {
        size_t available = xinc_queue_available_get(p_queue);
        element_count    = MIN(element_count, available);
    }

    queue_write(p_queue, p_data, element_count);

    CRITICAL_REGION_EXIT();

    XINC_LOG_INST_DEBUG(p_queue->p_log, "Put in %d elements (start address: 0x%08X), requested :%d",
                                       element_count, p_data, req_element_count);

    return element_count;
}

/**@brief Read elements from the queue. This function assumes that there are enough elements
 *        in the queue to read and that this process will not be interrupted.
 *
 * @param[in]   p_queue             Pointer to the xinc_queue_t instance.
 * @param[out]  p_data              Pointer to the buffer where elements will be copied.
 * @param[in]   element_count       Number of elements to read.
 */
static void queue_read(xinc_queue_t const * p_queue, void * p_data, uint32_t element_count)
{
    size_t front        = p_queue->p_cb->front;
    size_t continuous   = continous_items_get(p_queue, false);
    void const * p_read_ptr = (void const *)((size_t)p_queue->p_buffer
                                           + front * p_queue->element_size);

    if (element_count <= continuous)
    {
        memcpy(p_data,
               p_read_ptr,
               element_count * p_queue->element_size);

        p_queue->p_cb->front = ((front + element_count) <= p_queue->size)
                             ? (front + element_count)
                             : 0;
    }
    else
    {
        size_t first_read_length = continuous * p_queue->element_size;
        memcpy(p_data,
               p_read_ptr,
               first_read_length);

        size_t elements_left = element_count - continuous;
        memcpy((void *)((size_t)p_data + first_read_length),
               p_queue->p_buffer,
               elements_left * p_queue->element_size);

        p_queue->p_cb->front = elements_left;
    }
}

ret_code_t xinc_queue_read(xinc_queue_t const * p_queue,
                          void              * p_data,
                          size_t              element_count)
{
    ret_code_t status = XINC_SUCCESS;

    ASSERT(p_queue != NULL);
    ASSERT(p_data != NULL);

    if (element_count == 0)
    {
        return XINC_SUCCESS;
    }

    CRITICAL_REGION_ENTER();

    if (element_count <= queue_utilization_get(p_queue))
    {
        queue_read(p_queue, p_data, element_count);
    }
    else
    {
        status = XINC_ERROR_NOT_FOUND;
    }

    CRITICAL_REGION_EXIT();

    XINC_LOG_INST_DEBUG(p_queue->p_log, "Read %d elements (start address: 0x%08X), status :%d",
                                       element_count, p_data, status);
    return status;
}

size_t xinc_queue_out(xinc_queue_t const * p_queue,
                     void              * p_data,
                     size_t              element_count)
{
    ASSERT(p_queue != NULL);
    ASSERT(p_data != NULL);

    size_t req_element_count = element_count;

    if (element_count == 0)
    {
        return 0;
    }

    CRITICAL_REGION_ENTER();

    size_t utilization = queue_utilization_get(p_queue);
    element_count      = MIN(element_count, utilization);

    queue_read(p_queue, p_data, element_count);

    CRITICAL_REGION_EXIT();

    XINC_LOG_INST_DEBUG(p_queue->p_log, "Out %d elements (start address: 0x%08X), requested :%d",
                                       element_count, p_data, req_element_count);
    return element_count;
}

void xinc_queue_reset(xinc_queue_t const * p_queue)
{
    ASSERT(p_queue != NULL);

    CRITICAL_REGION_ENTER();

    memset(p_queue->p_cb, 0, sizeof(xinc_queue_cb_t));

    CRITICAL_REGION_EXIT();

    XINC_LOG_INST_DEBUG(p_queue->p_log, "Reset");
}

size_t xinc_queue_utilization_get(xinc_queue_t const * p_queue)
{
    size_t utilization;
    ASSERT(p_queue != NULL);

    CRITICAL_REGION_ENTER();

    utilization = queue_utilization_get(p_queue);

    CRITICAL_REGION_EXIT();

    return utilization;
}

bool xinc_queue_is_empty(xinc_queue_t const * p_queue)
{
    ASSERT(p_queue != NULL);
    size_t front    = p_queue->p_cb->front;
    size_t back     = p_queue->p_cb->back;
    return (front == back);
}

size_t xinc_queue_available_get(xinc_queue_t const * p_queue)
{
    ASSERT(p_queue != NULL);
    return p_queue->size - xinc_queue_utilization_get(p_queue);
}

size_t xinc_queue_max_utilization_get(xinc_queue_t const * p_queue)
{
    ASSERT(p_queue != NULL);
    return p_queue->p_cb->max_utilization;
}

void xinc_queue_max_utilization_reset(xinc_queue_t const * p_queue)
{
    ASSERT(p_queue != NULL);
    p_queue->p_cb->max_utilization = 0;
}


#endif // XINC_MODULE_ENABLED(XINC_QUEUE)
