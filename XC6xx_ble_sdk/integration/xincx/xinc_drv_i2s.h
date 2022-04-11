/**
 * Copyright (c) 2015 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef XINC_DRV_I2S_H__
#define XINC_DRV_I2S_H__

#include <xincx_i2sm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef I2SM_PRESENT
    #include <xincx_i2sm.h>
    
    /** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_I2S_PIN_NOT_USED               XINCX_I2SM_PIN_NOT_USED
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_I2S_FLAG_SYNCHRONIZED_MODE     XINCX_I2SM_FLAG_SYNCHRONIZED_MODE
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_I2S_DEFAULT_CONFIG             XINCX_I2SM_DEFAULT_CONFIG
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_I2S_STATUS_NEXT_BUFFERS_NEEDED XINCX_I2SM_STATUS_NEXT_BUFFERS_NEEDED

/** @brief Type definition for forwarding the new implementation. */
typedef xincx_i2sm_config_t   xinc_drv_i2s_config_t;
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_i2sm_buffers_t  xinc_drv_i2s_buffers_t;




#else
    // Compilers (at least the smart ones) will remove the I2CM related code
    // (blocks starting with "if (XINC_DRV_I2C_USE_I2CM)") when it is not used,
    // but to perform the compilation they need the following definitions.
    #define xincx_i2sm_init(...)                 0
    #define xincx_i2sm_uninit(...)
    #define xincx_i2sm_start(...)                0
    #define xincx_i2sm_next_buffers_set(...)     0
    #define xincx_i2sm_stop(...)
#endif



#ifdef I2S_PRESENT
    #include <xincx_i2s.h>
    
    /** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_I2S_PIN_NOT_USED               XINCX_I2S_PIN_NOT_USED
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_I2S_FLAG_SYNCHRONIZED_MODE     XINCX_I2S_FLAG_SYNCHRONIZED_MODE
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_I2S_DEFAULT_CONFIG             XINCX_I2S_DEFAULT_CONFIG
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_I2S_STATUS_NEXT_BUFFERS_NEEDED XINCX_I2S_STATUS_NEXT_BUFFERS_NEEDED

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_i2s_data_handler_t             xincx_i2s_data_handler_t

/** @brief Type definition for forwarding the new implementation. */
typedef xincx_i2s_config_t   xinc_drv_i2s_config_t;
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_i2s_buffers_t  xinc_drv_i2s_buffers_t;




#else
    // Compilers (at least the smart ones) will remove the I2S related code
    // (blocks starting with "if (XINC_DRV_I2S_USE_I2S)") when it is not used,
    // but to perform the compilation they need the following definitions.
    #define xincx_i2s_init(...)                  0
    #define xincx_i2s_uninit(...)
    #define xincx_i2s_start(...)                 0
    #define xincx_i2s_next_buffers_set(...)      0
    #define xincx_i2s_stop(...)
    
#endif


/**
 * @brief Structure for the I2S master driver instance.
 */
typedef struct
{
    uint8_t inst_idx;
    union
    {
#ifdef I2SM_PRESENT
        xincx_i2sm_t i2sm;
#endif
#ifdef I2S_PRESENT
        xincx_i2sm_t  i2s;
#endif
    } u;
    bool    use_easy_dma;
} xinc_drv_i2s_t;


/**
 * @brief Macro for creating a I2C master driver instance.
 */
#define XINC_DRV_I2S_INSTANCE(id)    XINC_DRV_I2S_INSTANCE_(id)
#define XINC_DRV_I2S_INSTANCE_(id)   XINC_DRV_I2S_INSTANCE_ ## id
#if XINCX_CHECK(XINCX_I2SM0_ENABLED)
    #define XINC_DRV_I2S_INSTANCE_0 \
        { 0, { .i2sm = XINCX_I2SM_INSTANCE(0) }, true }
#elif XINCX_CHECK(XINCX_I2S0_ENABLED)
    #define XINC_DRV_I2S_INSTANCE_0 \
        { 0, { .i2c = XINC_I2S_INSTANCE(0) }, false }
#endif

        

/**
 * @defgroup xinc_drv_i2sm I2S driver - legacy layer
 * @{
 * @ingroup  xinc_i2sm
 *
 * @brief    @tagAPI52 Layer providing compatibility with the former API.
 */


typedef struct
{
    xinc_drv_i2s_buffers_t const * p_released;
    uint32_t                   status;
}xinc_drv_i2s_data_handler_t;


/**
 * @brief I2C event handler prototype.
 */
typedef void (* xinc_drv_i2s_evt_handler_t)(xinc_drv_i2s_data_handler_t const * p_event,
                                           void *                    p_context);






#define XINC_DRV_I2S_USE_I2SM  true
#define XINC_DRV_I2S_USE_I2S  (!XINC_DRV_I2S_USE_I2SM)

/**
 * @brief Function for initializing the I2S driver.
 *
 * @param[in] p_config Pointer to the structure with initial configuration.
 *                     If NULL, the default configuration is used.
 * @param[in] handler  Data handler provided by the user. Must not be NULL.
 *
 * @retval NRF_SUCCESS             If initialization was successful.
 * @retval NRF_ERROR_INVALID_STATE If the driver was already initialized.
 * @retval NRF_ERROR_INVALID_PARAM If the requested combination of configuration
  *                                 options is not allowed by the I2S peripheral.
*/

ret_code_t xinc_drv_i2s_init(xinc_drv_i2s_t const *        p_instance,
                            xinc_drv_i2s_config_t const * p_config,
                            xinc_drv_i2s_evt_handler_t    handler,
                            void *                       p_context);
                            


  
                            
__STATIC_INLINE
void xinc_drv_i2s_uninit(xinc_drv_i2s_t const *        p_instance);
                            
__STATIC_INLINE
ret_code_t xinc_drv_i2s_start(xinc_drv_i2s_t const *        p_instance,
                            xinc_drv_i2s_buffers_t const * p_initial_buffers,
                          uint16_t                   buffer_size,
                          uint8_t                    flags);  

__STATIC_INLINE
ret_code_t xinc_drv_i2s_next_buffers_set(xinc_drv_i2s_t const *        p_instance,
                                        xinc_drv_i2s_buffers_t const * p_buffers); 


__STATIC_INLINE
void xinc_drv_i2s_stop(xinc_drv_i2s_t const *        p_instance);
                            


__STATIC_INLINE
void xinc_drv_i2s_uninit(xinc_drv_i2s_t const *        p_instance)
{
    if (XINC_DRV_I2S_USE_I2SM)
    {
        xincx_i2sm_uninit(&p_instance->u.i2sm);
    }
    else if (XINC_DRV_I2S_USE_I2S)
    {
        xincx_i2s_uninit(&p_instance->u.i2s);
    }

}

__STATIC_INLINE
ret_code_t xinc_drv_i2s_start(xinc_drv_i2s_t const *        p_instance,
                            xinc_drv_i2s_buffers_t const * p_initial_buffers,
                          uint16_t                   buffer_size,
                          uint8_t                    flags)
{
    ret_code_t result = XINC_SUCCESS;
    
    if (XINC_DRV_I2S_USE_I2SM)
    {
        result = xincx_i2sm_start(&p_instance->u.i2sm,p_initial_buffers,buffer_size,flags);
    }
    else if (XINC_DRV_I2S_USE_I2S)
    {
        result = xincx_i2s_start(&p_instance->u.i2s,p_initial_buffers,buffer_size,flags);
    }

    return result;
}


__STATIC_INLINE
ret_code_t xinc_drv_i2s_next_buffers_set(xinc_drv_i2s_t const *        p_instance,
                                        xinc_drv_i2s_buffers_t const * p_buffers)
{
    ret_code_t result = XINC_SUCCESS;
    
    if (XINC_DRV_I2S_USE_I2SM)
    {
        result = xincx_i2sm_next_buffers_set(&p_instance->u.i2sm,p_buffers);
    }
    else if (XINC_DRV_I2S_USE_I2S)
    {
        result = xincx_i2s_next_buffers_set(&p_instance->u.i2s,p_buffers);
    }
       
    return result;
}


__STATIC_INLINE
void xinc_drv_i2s_stop(xinc_drv_i2s_t const *        p_instance)
{
    if (XINC_DRV_I2S_USE_I2SM)
    {
        xincx_i2sm_stop(&p_instance->u.i2sm);
    }
    else if (XINC_DRV_I2S_USE_I2S)
    {
        xincx_i2s_stop(&p_instance->u.i2s);
    }
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_DRV_I2S_H__
