

#include "xinc_fstorage_flash.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "cfc_atomic.h"
#include "cfc_flash.h"
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
   // app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

static nrf_fstorage_info_t m_flash_info =
{
	 .erase_unit = 4096,
#if defined(XC_620610)
    .erase_unit = 1024,
#elif defined(XC_XXXX)
    .erase_unit = 4096,
#endif
    .program_unit = 4,
    .rmap         = true,
    .wmap         = false,
};

 /* An operation initiated by fstorage is ongoing. */
static nrf_atomic_flag_t m_flash_operation_ongoing;


/* Send event to the event handler. */
static void event_send(nrf_fstorage_t        const * p_fs,
                       nrf_fstorage_evt_id_t         evt_id,
                       void const *                  p_src,
                       uint32_t                      addr,
                       uint32_t                      len,
                       void                        * p_param)
{
    if (p_fs->evt_handler == NULL)
    {
        /* Nothing to do. */
        return;
    }

    nrf_fstorage_evt_t evt =
    {
        .result  = NRF_SUCCESS,
        .id      = evt_id,
        .addr    = addr,
        .p_src   = p_src,
        .len     = len,
        .p_param = p_param,
    };

    p_fs->evt_handler(&evt);
}

static ret_code_t init(nrf_fstorage_t * p_fs, void * p_param)
{
    UNUSED_PARAMETER(p_param);

    p_fs->p_flash_info = &m_flash_info;
	
		printf("m_fstroage_flash init\n");

    return NRF_SUCCESS;
}

static ret_code_t uninit(nrf_fstorage_t * p_fs, void * p_param)
{
    UNUSED_PARAMETER(p_fs);
    UNUSED_PARAMETER(p_param);

    (void) nrf_atomic_flag_clear(&m_flash_operation_ongoing);

    return NRF_SUCCESS;
}


static ret_code_t read(nrf_fstorage_t const * p_fs, uint32_t src, void * p_dest, uint32_t len)
{
    UNUSED_PARAMETER(p_fs);

		printf("flash %s\n",__func__);
    memcpy(p_dest, (uint32_t*)src, len);

    return NRF_SUCCESS;
}


static ret_code_t write(nrf_fstorage_t const * p_fs,
                        uint32_t               dest,
                        void           const * p_src,
                        uint32_t               len,
                        void                 * p_param)
{
		printf("flash %s\n",__func__);
    if (nrf_atomic_flag_set_fetch(&m_flash_operation_ongoing))
    {
        return NRF_ERROR_BUSY;
    }

    nrf_nvmc_write_words(dest, (uint32_t*)p_src, (len / m_flash_info.program_unit));

    /* Clear the flag before sending the event, to allow API calls in the event context. */
    (void) nrf_atomic_flag_clear(&m_flash_operation_ongoing);

    event_send(p_fs, NRF_FSTORAGE_EVT_WRITE_RESULT, p_src, dest, len, p_param);

    return NRF_SUCCESS;
}


static ret_code_t erase(nrf_fstorage_t const * p_fs,
                        uint32_t               page_addr,
                        uint32_t               len,
                        void                 * p_param)
{
    uint32_t progress = 0;
		printf("flash %s\n",__func__);
    if (nrf_atomic_flag_set_fetch(&m_flash_operation_ongoing))
    {
        return NRF_ERROR_BUSY;
    }

    while (progress != len)
    {
        nrf_nvmc_page_erase(page_addr + (progress * m_flash_info.erase_unit));
        progress++;
    }

    /* Clear the flag before sending the event, to allow API calls in the event context. */
    (void) nrf_atomic_flag_clear(&m_flash_operation_ongoing);

    event_send(p_fs, NRF_FSTORAGE_EVT_ERASE_RESULT, NULL, page_addr, len, p_param);

    return NRF_SUCCESS;
}

static uint8_t const * rmap(nrf_fstorage_t const * p_fs, uint32_t addr)
{
    UNUSED_PARAMETER(p_fs);

    return (uint8_t*)addr;
}


static uint8_t * wmap(nrf_fstorage_t const * p_fs, uint32_t addr)
{
    UNUSED_PARAMETER(p_fs);
    UNUSED_PARAMETER(addr);

    /* Not supported. */
    return NULL;
}


static bool is_busy(nrf_fstorage_t const * p_fs)
{
    UNUSED_PARAMETER(p_fs);

    return m_flash_operation_ongoing;
}


/* The exported API. */
nrf_fstorage_api_t nrf_fstorage_nvmc =
{
    .init    = init,
    .uninit  = uninit,
    .read    = read,
    .write   = write,
    .erase   = erase,
    .rmap    = rmap,
    .wmap    = wmap,
    .is_busy = is_busy
};


