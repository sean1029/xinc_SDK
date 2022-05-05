/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_FSTORAGE_H__
#define XINC_FSTORAGE_H__

/**
 * @file
 *
 * @defgroup xinc_fstorage Flash storage (fstorage)
 * @ingroup app_common
 * @{
 *
 * @brief   Flash abstraction library that provides basic read, write, and erase operations.
 *
 * @details The fstorage library can be implemented in different ways. Two implementations are provided:
 * - The @ref xinc_fstorage_sd implements flash access through the SoftDevice.
 * - The @ref xinc_fstorage_nvmc implements flash access through the non-volatile memory controller.
 *
 * You can select the implementation that should be used independently for each instance of fstorage.
 */

#include <stdint.h>
#include <stdbool.h>
#include "sdk_errors.h"
#include "xinc_section.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t ret_code_t;

/**@brief   Macro for defining an fstorage instance.
 *
 * Users of fstorage must define an instance variable by using this macro.
 * Each instance is tied to an API implementation and contains information such
 * as the program and erase units for the target flash peripheral.
 * Instance variables are placed in the "fs_data" section of the binary.
 *
 * @param[in]   inst    A definition of an @ref xinc_fstorage_t variable.
 */
#define XINC_FSTORAGE_DEF(inst)          XINC_SECTION_ITEM_REGISTER(fs_data, inst)

/**@brief   Macro for retrieving an fstorage instance. */
#define XINC_FSTORAGE_INSTANCE_GET(i)    XINC_SECTION_ITEM_GET(fs_data, xinc_fstorage_t, (i))

/**@brief   Macro for retrieving the total number of fstorage instances. */
#define XINC_FSTORAGE_INSTANCE_CNT       XINC_SECTION_ITEM_COUNT(fs_data, xinc_fstorage_t)


/**@brief   Event IDs. */
typedef enum
{
    XINC_FSTORAGE_EVT_READ_RESULT,   //!< Unused event reserved for a possible future feature.
    XINC_FSTORAGE_EVT_WRITE_RESULT,  //!< Event for @ref xinc_fstorage_write.
    XINC_FSTORAGE_EVT_ERASE_RESULT   //!< Event for @ref xinc_fstorage_erase.
} xinc_fstorage_evt_id_t;


/**@brief   An fstorage event. */
typedef struct
{
    xinc_fstorage_evt_id_t   id;         //!< The event ID.
    ret_code_t              result;     //!< Result of the operation.
    uint32_t                addr;       //!< Address at which the operation was performed.
    void            const * p_src;      //!< Buffer written to flash.
    uint32_t                len;        //!< Length of the operation.
    void                  * p_param;    //!< User-defined parameter passed to the event handler.
} xinc_fstorage_evt_t;


/**@brief   Event handler function prototype.
 *
 * @param[in]   p_evt   The event.
 */
typedef void (*xinc_fstorage_evt_handler_t)(xinc_fstorage_evt_t * p_evt);


/**@brief   Information about the implementation and the flash peripheral. */
typedef struct
{
    uint32_t erase_unit;        //!< Size of a flash page (in bytes). A flash page is the smallest unit that can be erased.
    uint32_t program_unit;      //!< Size of the smallest programmable unit (in bytes).
    bool     rmap;              //!< The device address space is memory mapped to the MCU address space.
    bool     wmap;              //!< The device address space is memory mapped to a writable MCU address space.
} const xinc_fstorage_info_t;


/* Necessary forward declaration. */
struct xinc_fstorage_api_s;


/**@brief   An fstorage instance.
 *
 * @details Use the @ref XINC_FSTORAGE_DEF macro to define an fstorage instance.
 *
 * An instance is tied to an API implementation and contains information about the flash device,
 * such as the program and erase units as well and implementation-specific functionality.
 */
typedef struct
{
    /**@brief   The API implementation used by this instance. */
    struct xinc_fstorage_api_s const * p_api;

    /**@brief   Information about the implementation functionality and the flash peripheral. */
    xinc_fstorage_info_t * p_flash_info;

    /**@brief   The event handler function.
     *
     * If set to NULL, no events will be sent.
     */
    xinc_fstorage_evt_handler_t evt_handler;

    /**@brief   The beginning of the flash space on which this fstorage instance should operate.
     *          All flash operations must be within the address specified in
     *          this field and @ref end_addr.
     *
     * This field must be set manually.
     */
    uint32_t start_addr;

    /**@brief   The last address (exclusive) of flash on which this fstorage instance should operate.
     *          All flash operations must be within the address specified in
     *          this field and @ref start_addr.
     *
     * This field must be set manually.
     */
    uint32_t end_addr;
} xinc_fstorage_t;


/**@brief Functions provided by the API implementation. */
typedef struct xinc_fstorage_api_s
{
    /**@brief Initialize the flash peripheral. */
    ret_code_t (*init)(xinc_fstorage_t * p_fs, void * p_param);
    /**@brief Uninitialize the flash peripheral. */
    ret_code_t (*uninit)(xinc_fstorage_t * p_fs, void * p_param);
    /**@brief Read data from flash. */
    ret_code_t (*read)(xinc_fstorage_t const * p_fs, uint32_t src, void * p_dest, uint32_t len);
    /**@brief Write bytes to flash. */
    ret_code_t (*write)(xinc_fstorage_t const * p_fs, uint32_t dest, void const * p_src, uint32_t len, void * p_param);
    /**@brief Erase flash pages. */
    ret_code_t (*erase)(xinc_fstorage_t const * p_fs, uint32_t addr, uint32_t len, void * p_param);
    /**@brief Erase fds flash space. */
    ret_code_t (*space_init)(xinc_fstorage_t const * p_fs, uint32_t addr, uint32_t len, void * p_param);   
    /**@brief Map a device address to a readable address within the MCU address space. */
    uint8_t const * (*rmap)(xinc_fstorage_t const * p_fs, uint32_t addr);
    /**@brief Map a device address to a writable address within the MCU address space. */
    uint8_t * (*wmap)(xinc_fstorage_t const * p_fs, uint32_t addr);
    /**@brief Check if there are any pending flash operations. */
    bool (*is_busy)(xinc_fstorage_t const * p_fs);
} const xinc_fstorage_api_t;


/**@brief   Function for initializing fstorage.
 *
 * @param[in]   p_fs        The fstorage instance to initialize.
 * @param[in]   p_api       The API implementation to use.
 * @param[in]   p_param     An optional parameter to pass to the implementation-specific API call.
 *
 * @retval  XINC_SUCCESS         If initialization was successful.
 * @retval  XINC_ERROR_NULL      If @p p_fs or @p p_api field in @p p_fs is NULL.
 * @retval  XINC_ERROR_INTERNAL  If another error occurred.
 */
ret_code_t xinc_fstorage_init(xinc_fstorage_t     * p_fs,
                             xinc_fstorage_api_t * p_api,
                             void               * p_param);


/**@brief   Function for uninitializing an fstorage instance.
 *
 * @param[in]   p_fs        The fstorage instance to uninitialize.
 * @param[in]   p_param     An optional parameter to pass to the implementation-specific API call.
 *
 * @retval  XINC_SUCCESS                 If uninitialization was successful.
 * @retval  XINC_ERROR_NULL              If @p p_fs is NULL.
 * @retval  XINC_ERROR_INVALID_STATE     If the module is not initialized.
 * @retval  XINC_ERROR_INTERNAL          If another error occurred.
 */
ret_code_t xinc_fstorage_uninit(xinc_fstorage_t * p_fs, void * p_param);


/**@brief   Function for reading data from flash.
 *
 * Copy @p len bytes from @p addr to @p p_dest.
 *
 * @param[in]   p_fs    The fstorage instance.
 * @param[in]   addr    Address in flash where to read from.
 * @param[in]   p_dest  Buffer where the data should be copied.
 * @param[in]   len     Length of the data to be copied (in bytes).
 *
 * @retval  XINC_SUCCESS                 If the operation was successful.
 * @retval  XINC_ERROR_NULL              If @p p_fs or @p p_dest is NULL.
 * @retval  XINC_ERROR_INVALID_STATE     If the module is not initialized.
 * @retval  XINC_ERROR_INVALID_LENGTH    If @p len is zero or otherwise invalid.
 * @retval  XINC_ERROR_INVALID_ADDR      If the address @p addr is outside the flash memory
 *                                      boundaries specified in @p p_fs, or if it is unaligned.
 */
ret_code_t xinc_fstorage_read(xinc_fstorage_t const * p_fs,
                             uint32_t               addr,
                             void                 * p_dest,
                             uint32_t               len);


/**@brief   Function for writing data to flash.
 *
 * Write @p len bytes from @p p_src to @p dest.
 *
 * When using @ref xinc_fstorage_sd, the data is written by several calls to @ref sd_flash_write if
 * the length of the data exceeds @ref XINC_FSTORAGE_SD_MAX_WRITE_SIZE bytes.
 * Only one event is sent upon completion.
 *
 * @note The data to be written to flash must be kept in memory until the operation has
 *       terminated and an event is received.
 *
 * @param[in]   p_fs        The fstorage instance.
 * @param[in]   dest        Address in flash memory where to write the data.
 * @param[in]   p_src       Data to be written.
 * @param[in]   len         Length of the data (in bytes).
 * @param[in]   p_param     User-defined parameter passed to the event handler (may be NULL).
 *
 * @retval  XINC_SUCCESS                 If the operation was accepted.
 * @retval  XINC_ERROR_NULL              If @p p_fs or @p p_src is NULL.
 * @retval  XINC_ERROR_INVALID_STATE     If the module is not initialized.
 * @retval  XINC_ERROR_INVALID_LENGTH    If @p len is zero or not a multiple of the program unit,
 *                                      or if it is otherwise invalid.
 * @retval  XINC_ERROR_INVALID_ADDR      If the address @p dest is outside the flash memory
 *                                      boundaries specified in @p p_fs, or if it is unaligned.
 * @retval  XINC_ERROR_NO_MEM            If no memory is available to accept the operation.
 *                                      When using the @ref xinc_fstorage_sd, this error
 *                                      indicates that the internal queue of operations is full.
 */
ret_code_t xinc_fstorage_write(xinc_fstorage_t const * p_fs,
                              uint32_t               dest,
                              void           const * p_src,
                              uint32_t               len,
                              void                 * p_param);


/**@brief   Function for erasing flash pages.
 *
 * @details This function erases @p len pages starting from the page at address @p page_addr.
 *          The erase operation must be initiated on a page boundary.
 *
 * @param[in]   p_fs        The fstorage instance.
 * @param[in]   page_addr   Address of the page to erase.
 * @param[in]   len         Number of pages to erase.
 * @param[in]   p_param     User-defined parameter passed to the event handler (may be NULL).
 *
 * @retval  XINC_SUCCESS                 If the operation was accepted.
 * @retval  XINC_ERROR_NULL              If @p p_fs is NULL.
 * @retval  XINC_ERROR_INVALID_STATE     If the module is not initialized.
 * @retval  XINC_ERROR_INVALID_LENGTH    If @p len is zero.
 * @retval  XINC_ERROR_INVALID_ADDR      If the address @p page_addr is outside the flash memory
 *                                      boundaries specified in @p p_fs, or if it is unaligned.
 * @retval  XINC_ERROR_NO_MEM            If no memory is available to accept the operation.
 *                                      When using the @ref xinc_fstorage_sd, this error
 *                                      indicates that the internal queue of operations is full.
 */
ret_code_t xinc_fstorage_erase(xinc_fstorage_t const * p_fs,
                              uint32_t               page_addr,
                              uint32_t               len,
                              void                 * p_param);



/**@brief   Function for erasing fds flash space.
 *
 * @details This function erases @p len pages starting from the page at address @p page_addr.
 *          The erase operation must be initiated on a page boundary.
 *
 * @param[in]   p_fs        The fstorage instance.
 * @param[in]   page_addr   Address of the page to erase.
 * @param[in]   len         Number of pages to erase.
 * @param[in]   p_param     User-defined parameter passed to the event handler (may be NULL).
 *
 * @retval  XINC_SUCCESS                 If the operation was accepted.
 * @retval  XINC_ERROR_NULL              If @p p_fs is NULL.
 * @retval  XINC_ERROR_INVALID_STATE     If the module is not initialized.
 * @retval  XINC_ERROR_INVALID_LENGTH    If @p len is zero.
 * @retval  XINC_ERROR_INVALID_ADDR      If the address @p page_addr is outside the flash memory
 *                                      boundaries specified in @p p_fs, or if it is unaligned.
 * @retval  XINC_ERROR_NO_MEM            If no memory is available to accept the operation.
 *                                      When using the @ref xinc_fstorage_sd, this error
 *                                      indicates that the internal queue of operations is full.
 */
ret_code_t xinc_fstorage_space_init(xinc_fstorage_t const * p_fs,
                              uint32_t               page_addr,
                              uint32_t               len,
                              void                 * p_param);


/**@brief   Map a flash address to a pointer in the MCU address space that can be dereferenced.
 *
 * @param   p_fs    The fstorage instance.
 * @param   addr    The address to map.
 *
 * @retval  A pointer to the specified address,
 *          or @c NULL if the address cannot be mapped or if @p p_fs is @c NULL.
 */
uint8_t const * xinc_fstorage_rmap(xinc_fstorage_t const * p_fs, uint32_t addr);


/**@brief   Map a flash address to a pointer in the MCU address space that can be written to.
 *
 * @param   p_fs    The fstorage instance.
 * @param   addr    The address to map.
 *
 * @retval  A pointer to the specified address,
 *          or @c NULL if the address cannot be mapped or if @p p_fs is @c NULL.
 */
uint8_t * xinc_fstorage_wmap(xinc_fstorage_t const * p_fs, uint32_t addr);


/**@brief   Function for querying the status of fstorage.
 *
 * @details An uninitialized instance of fstorage is treated as not busy.
 *
 * @param[in]   p_fs    The fstorage instance. Pass NULL to query all instances.
 *
 * @returns If @p p_fs is @c NULL, this function returns true if any fstorage instance is busy or false otherwise.
 * @returns If @p p_fs is not @c NULL, this function returns true if the fstorage instance is busy or false otherwise.
 */
 bool xinc_fstorage_is_busy(xinc_fstorage_t const * p_fs);

/** @} */


#ifdef __cplusplus
}
#endif

#endif // XINC_FSTORAGE_H__
