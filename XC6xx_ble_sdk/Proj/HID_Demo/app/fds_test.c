
#include <string.h>
#include "fds_test.h"
//#include "es_util.h"
#include "app_scheduler.h"

#include "fds.h"


static volatile uint32_t m_num_pending_ops;                         //!< Current number of outstanding FDS operations.
static volatile bool     m_factory_reset_done;                      //!< Has a factory reset operation been completed.

#define SIZE_OF_PRIV_KEY 16 //!< Size of ECDH private key.
#define SIZE_OF_PUB_KEY 64  //!< Size of ECDH public key.
#define SIZE_OF_LOCK_KEY 16  //!< Size of lock key.
#define FILE_ID_ES_FLASH 0x1337             //!< File ID used for all flash access EXCEPT lock code.
#define FILE_ID_ES_FLASH_LOCK_KEY 0x1338    //!< File ID used for lock code flash access.
#define RECORD_KEY_FLAGS 0x1                //!< File record for flash flags.
#define RECORD_KEY_PRIV_KEY 0x2             //!< File record for private key.
#define RECORD_KEY_PUB_KEY 0x3              //!< File record for public key.
#define RECORD_KEY_LOCK_KEY 0x4             //!< File record for lock key.
#define RECORD_KEY_BEACON_CONFIG 0x5        //!< File record for lock key.

static uint16_t RECORD_KEY_SLOTS[5] = {0x6, 0x7, 0x8, 0x9, 0xa}; //!< File record for slots.

/**@brief Structure used for invoking flash access function. */
typedef struct
{
    uint16_t          record_key;
    uint16_t          file_id;
    uint8_t *         p_data_buf;
    uint8_t *         p_data;
    uint16_t          size_bytes;
    es_flash_access_t access_type;
} flash_access_params_t;



__ALIGN(4) static uint8_t lock_key_buf[SIZE_OF_LOCK_KEY];   //!< Buffer for lock key flash access.



__ALIGN(4) static uint8_t flash_flags_buf[sizeof(es_flash_flags_t)];            //!< Buffer for flash flags flash access.


/**@brief Function handling FDS events.
 *
 * @param[in] p_evt FDS event.
 */
static void fds_cb(fds_evt_t const * const p_evt)
{
    ret_code_t err_code;

    printf("fds test fds_cb evt:%d,result:%d\r\n",p_evt->id,p_evt->result);
    switch (p_evt->id)
    {
        case FDS_EVT_INIT:
            m_num_pending_ops = 0;
            break;
        case FDS_EVT_DEL_FILE:
            if (p_evt->del.file_id == FILE_ID_ES_FLASH)
            {
                m_factory_reset_done = true;
            }
            // Fall through
        case FDS_EVT_DEL_RECORD:
            // Schedule garbage collection
          //  err_code = app_sched_event_put(NULL, 0, fds_gc_event);
            APP_ERROR_CHECK(err_code);
            break;

        case FDS_EVT_GC:
            // During factory reset, a file is deleted, and garbage collection is scheduled
            // when the callback for that deletion is invoked.
            // So here we know that the factory reset is completed.
            if (m_factory_reset_done)
            {
               
                {
                    m_factory_reset_done = false;
     
                }
            }
            // Fall through:
        case FDS_EVT_UPDATE:
            // Fall through:
        case FDS_EVT_WRITE:
            if (m_num_pending_ops > 0)
            {
                m_num_pending_ops--;
            }
            break;
    }
}

/**@brief Function performing flash access (read/write/clear).
 *
 * @param[in] p_params Flash access parameters.
 */
static ret_code_t access_flash_data(const flash_access_params_t * p_params)
{
    ret_code_t         err_code;
    fds_flash_record_t record = {0};
    fds_record_desc_t  desc   = {0};
    fds_find_token_t   ft     = {0};
    
    fds_record_t       record_to_write =
    {
        .data.p_data = p_params->p_data_buf,
        .file_id     = p_params->file_id
    };
    


    err_code = fds_record_find_by_key(p_params->record_key, &desc, &ft);
    printf("fds_record_find_by_key:%x\r\n",err_code);
        
    // If its a read or clear, we can not accept errors on lookup
    if (p_params->access_type == ES_FLASH_ACCESS_READ)
    {
        RETURN_IF_ERROR(err_code);
    }

    if (p_params->access_type == ES_FLASH_ACCESS_CLEAR && err_code == FDS_ERR_NOT_FOUND)
    {
        return XINC_SUCCESS;
    }

    switch (p_params->access_type)
    {
        case ES_FLASH_ACCESS_READ:
            record.p_rdata = p_params->p_data;
            record.size_bytes = p_params->size_bytes;
        
            err_code = fds_record_open(&desc, &record);
            printf("fds_record_open:%x\r\n",err_code);
            RETURN_IF_ERROR(err_code);
          
            err_code = fds_record_read(&desc, &record,p_params->p_data,p_params->size_bytes);

            err_code = fds_record_close(&desc);
            RETURN_IF_ERROR(err_code);

            break;

        case ES_FLASH_ACCESS_WRITE:
            memcpy(p_params->p_data_buf, p_params->p_data, p_params->size_bytes);

            record_to_write.data.length_words = (p_params->size_bytes +3) / 4;
            record_to_write.key               = p_params->record_key;

            if (err_code == FDS_ERR_NOT_FOUND)
            {
                err_code = fds_record_write(&desc, &record_to_write);
            }

            else
                
            {
                printf("fds_record_update addr:0x%p\r\n",&desc.p_record);
                err_code = fds_record_update(&desc, &record_to_write);
            }

            RETURN_IF_ERROR(err_code);
            m_num_pending_ops++;
            break;

        case ES_FLASH_ACCESS_CLEAR:
            err_code = fds_record_delete(&desc);
            RETURN_IF_ERROR(err_code);
            m_num_pending_ops++;
            break;

        default:
            break;
    }
    return XINC_SUCCESS;
}

ret_code_t es_flash_access_lock_key(uint8_t * p_lock_key, es_flash_access_t access_type)
{
    flash_access_params_t params = {.record_key  = RECORD_KEY_LOCK_KEY,
                                    .file_id     = FILE_ID_ES_FLASH_LOCK_KEY,
                                    .p_data_buf  = lock_key_buf,
                                    .p_data      = (uint8_t *)p_lock_key,
                                    .size_bytes  = SIZE_OF_LOCK_KEY,
                                    .access_type = access_type};

    return access_flash_data(&params);
}

ret_code_t es_flash_access_flags(es_flash_flags_t * p_flags, es_flash_access_t access_type)
{
    flash_access_params_t params = {.record_key  = RECORD_KEY_FLAGS,
                                    .file_id     = FILE_ID_ES_FLASH,
                                    .p_data_buf  = flash_flags_buf,
                                    .p_data      = (uint8_t *)p_flags,
                                    .size_bytes  = sizeof(es_flash_flags_t),
                                    .access_type = access_type};

    return access_flash_data(&params);
}


ret_code_t es_flash_factory_reset(void)
{
    // Delete everything except the lock key:
    ret_code_t ret_code = fds_file_delete(FILE_ID_ES_FLASH);

    return ret_code;
}


uint32_t es_flash_num_pending_ops(void)
{
    return m_num_pending_ops;
}




ret_code_t es_flash_init(void)
{
    ret_code_t err_code;

    m_num_pending_ops = 1; // Will be set to 0 when getting FDS_EVT_INIT event



    m_factory_reset_done = false;

    err_code = fds_register(fds_cb);
 

    err_code = fds_init();
    
    printf("fds_init err_code :0x%x\r\n",err_code);
    uint8_t lock_key[SIZE_OF_LOCK_KEY];
    for(int i = 0; i < SIZE_OF_LOCK_KEY;i ++)
    {
        lock_key[i] = 'b' + i;
    }
    err_code = es_flash_access_lock_key(lock_key,ES_FLASH_ACCESS_WRITE);
    if(err_code != XINC_SUCCESS)
    {
        printf("access_lock_key WRITE fail\r\n");
        return err_code;
    }
    err_code = es_flash_access_lock_key(lock_key,ES_FLASH_ACCESS_WRITE);
    for(int i = 0; i < SIZE_OF_LOCK_KEY;i ++)
    {
        lock_key[i] = 0;
    }
     printf("access_lock_key start read\r\n");
     err_code = es_flash_access_lock_key(lock_key,ES_FLASH_ACCESS_READ);
    
    if(err_code != XINC_SUCCESS)
    {
        printf("access_lock_key read fail\r\n");
        return err_code;
    }
    printf("access_lock_key read data:");
     for(int i = 0; i < SIZE_OF_LOCK_KEY;i ++)
    {
        printf("%c ",lock_key[i]);
    }printf("\r\n");
     printf("access_lock_key read data hex:");
     for(int i = 0; i < SIZE_OF_LOCK_KEY;i ++)
    {
        printf("%x ",lock_key[i]);
    }printf("\r\n");


    return XINC_SUCCESS;
}
