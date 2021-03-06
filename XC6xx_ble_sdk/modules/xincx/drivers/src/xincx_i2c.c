/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include <xincx.h>
#if XINCX_CHECK(XINCX_I2C_ENABLED)

#if !(XINCX_CHECK(XINCX_I2C0_ENABLED))
#error "No enabled I2C instances. Check <xincx_config.h>."
#endif

#include <xincx_i2c.h>
#include <hal/xinc_gpio.h>
#include "bsp_clk.h"
#define XINCX_LOG_MODULE I2C

//#define XINC_LOG_MODULE_NAME 	XINCX_LOG_MODULE
//#define XINC_LOG_LEVEL       (XINCX_I2C_CONFIG_LOG_ENABLED ? XINCX_I2C_CONFIG_LOG_LEVEL : 0)
//#define XINC_LOG_INFO_COLOR   XINCX_I2C_CONFIG_INFO_COLOR
//#define XINC_LOG_DEBUG_COLOR  XINCX_I2C_CONFIG_DEBUG_COLOR
#include <xincx_log.h>
//XINC_LOG_MODULE_REGISTER();


#define TRANSFER_TO_STR(type)                   \
    (type == XINCX_I2C_XFER_TX   ? "XFER_TX"   : \
    (type == XINCX_I2C_XFER_RX   ? "XFER_RX"   : \
    (type == XINCX_I2C_XFER_TXRX ? "XFER_TXRX" : \
    (type == XINCX_I2C_XFER_TXTX ? "XFER_TXTX" : \
                                  "UNKNOWN TRANSFER TYPE"))))

#define I2C_PIN_INIT(_pin) 

#define I2C_FLAG_HAS_START(flags)           (flags & XINCX_I2C_FLAG_TX_HAS_START)
#define I2C_FLAG_NO_STOP(flags)             (flags & XINCX_I2C_FLAG_TX_NO_STOP)
#define I2C_FLAG_SUSPEND(flags)             (flags & XINCX_I2C_FLAG_SUSPEND)
#define I2C_FLAG_NO_HANDLER_IN_USE(flags)   (flags & XINCX_I2C_FLAG_NO_XFER_EVT_HANDLER)

#define HW_TIMEOUT      100000

/* I2C master driver suspend types. */
typedef enum
{
    I2C_NO_SUSPEND, //< Last transfer was not suspended.
    I2C_SUSPEND_TX, //< Last transfer was TX and was suspended.
    I2C_SUSPEND_RX  //< Last transfer was RX and was suspended.
} i2c_suspend_t;

// Control block - driver instance local data.
typedef struct
{
    xincx_i2c_evt_handler_t  handler;
    void *                  p_context;
    volatile uint32_t       int_mask;
    xincx_i2c_xfer_desc_t    xfer_desc;
    uint32_t                flags;
    uint8_t *               p_curr_buf;
    size_t                  curr_length;
    bool                    curr_tx_no_stop;
    i2c_suspend_t           prev_suspend;
    xincx_drv_state_t        state;
    bool                    error;
    volatile bool           busy;
    bool                    repeated;
    size_t                  bytes_transferred;
    size_t                  bytes_transForRx;
    bool                    hold_bus_uninit;
} i2c_control_block_t;

static i2c_control_block_t m_cb[XINCX_I2C_ENABLED_COUNT];

static xincx_err_t i2c_process_error(uint32_t errorsrc)
{
    xincx_err_t ret = XINCX_ERROR_INTERNAL;

    return ret;
}

static bool xfer_completeness_check(XINC_I2C_Type * p_i2c, i2c_control_block_t const * p_cb)
{
    // If the actual number of transferred bytes is not equal to what was requested,
    // but there was no error signaled by the peripheral, this means that something
    // unexpected, like a premature STOP condition, was received on the bus.
    // In such case the peripheral has to be disabled and re-enabled, so that its
    // internal state machine is reinitialized.

    if (p_cb->bytes_transferred != p_cb->curr_length)
    {
        xinc_i2c_disable(p_i2c);
        xinc_i2c_enable(p_i2c);
        return false;
    }
    else
    {
        return true;
    }
}

xincx_err_t xincx_i2c_init(xincx_i2c_t const *        p_instance,
                         xincx_i2c_config_t const * p_config,
                         xincx_i2c_evt_handler_t    event_handler,
                         void *                    p_context)
{
    XINCX_ASSERT(p_config);
    XINCX_ASSERT(p_config->scl != p_config->sda);
    i2c_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
    xincx_err_t err_code;
    printf("%s\r\n",__func__);
    XINCX_LOG_INFO("Function: xincx_i2c_init");
    if (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                            __func__,
                            XINCX_LOG_ERROR_STRING_GET(err_code));
         printf("Function: %s, error code: %x.\r\n",
                            __func__,
                            (err_code));
        return err_code;
    }

    p_cb->handler         = event_handler;
    p_cb->p_context       = p_context;
    p_cb->int_mask        = 0;
    p_cb->prev_suspend    = I2C_NO_SUSPEND;
    p_cb->repeated        = false;
    p_cb->busy            = false;
    p_cb->hold_bus_uninit = p_config->hold_bus_uninit;


    XINC_CPR_CTL_Type * p_cpr = p_instance->p_cpr;

    //??????pclk_i2c?????????p_cpr->CTLAPBCLKEN_GRCTL = 0x8000800;
    p_cpr->CTLAPBCLKEN_GRCTL =  (CPR_CTLAPBCLKEN_GRCTL_I2C_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_I2C_PCLK_EN_Pos) |
                                (CPR_CTLAPBCLKEN_GRCTL_I2C_PCLK_EN_Msk << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET);

    // ??????i2c??????  p_cpr->RSTCTL_SUBRST_SW = 0x400000; p_cpr->RSTCTL_SUBRST_SW = 0x400040;                  
    p_cpr->RSTCTL_SUBRST_SW =   (CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Enable << CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Pos) |
                                (CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET);
    
    p_cpr->RSTCTL_SUBRST_SW =   (CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Disable << CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Pos) |
                                (CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET);
  
    // p_cpr->I2C_CLK_CTL = 0x110011; //-> i2c_mclk = 16mhz.
    //??????I2C??????????????????
    p_cpr->I2C_CLK_CTL = ((1UL << CPR_I2C_CLK_CTL_I2C_CLK_DIV_Pos) & CPR_I2C_CLK_CTL_I2C_CLK_DIV_Msk) |
                         ((1UL <<CPR_I2C_CLK_CTL_I2C_CLK_DIV_Pos) << CPR_I2C_CLK_CTL_MASK_OFFSET);
    //I2C_CLK ????????????
    p_cpr->I2C_CLK_CTL = (CPR_I2C_CLK_CTL_I2C_CLK_EN_Enable << CPR_I2C_CLK_CTL_I2C_CLK_EN_Pos) | 
                         ((1UL << CPR_I2C_CLK_CTL_I2C_CLK_EN_Pos) << CPR_I2C_CLK_CTL_MASK_OFFSET);
   


    XINC_I2C_Type * p_i2c = p_instance->p_i2c;

    xinc_i2c_disable(p_i2c);
    
    p_i2c->i2c_RX_TL = 0;
    p_i2c->i2c_TX_TL = 0;
    
    xinc_i2c_int_disable(p_i2c,XINC_I2C_INT_DIS_MASK_ALL);//????????????????????? 0x

    err_code = xinc_gpio_secfun_config(p_config->scl,XINC_GPIO_PIN_I2C_SCL);
    printf("p_config->scl:%d,secfun_config ret:0x%x\r\n",p_config->scl,err_code);
    if(err_code != XINCX_SUCCESS)
    {
        return err_code;
    }
    err_code = xinc_gpio_secfun_config(p_config->sda,XINC_GPIO_PIN_I2C_SDA);
     printf("p_config->sda:%d,secfun_config ret:0x%x\r\n",p_config->sda,err_code);
    if(err_code != XINCX_SUCCESS)
    {
        return err_code;
    }
   // xinc_i2c_frequency_set(p_i2c,(xinc_i2c_frequency_t)p_config->frequency);
    if(p_config->frequency == XINC_I2C_FREQ_100K)//0x0063 48 4FUL
    {
        p_i2c->i2c_CON = (p_config->frequency >> 16) & 0xFF;
        p_i2c->i2c_SS_SCL_HCNT = (p_config->frequency >> 8) & 0xFF;
        p_i2c->i2c_SS_SCL_LCNT = (p_config->frequency >> 0) & 0xFF;
        printf("speed config 100k\r\n");
    }
    else if(p_config->frequency == XINC_I2C_FREQ_400K)//0x0065 0C 13 UL
    {
        p_i2c->i2c_CON = (p_config->frequency >> 16) & 0xFF;
        p_i2c->i2c_FS_SCL_HCNT = (p_config->frequency >> 8) & 0xFF;
        p_i2c->i2c_FS_SCL_LCNT = (p_config->frequency >> 0) & 0xFF;
        printf("speed config 400k\r\n");
    }
    else
    {
        err_code = XINCX_ERROR_INVALID_PARAM;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                        __func__,
                        XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    printf("speed config:0x%02x,0x%02x,0x%02x\r\n",(p_config->frequency >> 16) & 0xFF,(p_config->frequency >> 8) & 0xFF,(p_config->frequency >> 0) & 0xFF);
		
    if (p_cb->handler)
    {
        XINCX_IRQ_ENABLE(I2C_IRQn);
    }

    p_cb->state = XINCX_DRV_STATE_INITIALIZED;

    err_code = XINCX_SUCCESS;
    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

void xincx_i2c_uninit(xincx_i2c_t const * p_instance)
{
    i2c_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINCX_ASSERT(p_cb->state != XINCX_DRV_STATE_UNINITIALIZED);

    if (p_cb->handler)
    {
        XINCX_IRQ_DISABLE(I2C_IRQn);
    }
    xincx_i2c_disable(p_instance);

#if XINCX_CHECK(XINCX_PRS_ENABLED)
    xincx_prs_release(p_instance->p_i2c);
#endif

    if (!p_cb->hold_bus_uninit)
    {

    }

    p_cb->state = XINCX_DRV_STATE_UNINITIALIZED;
    XINCX_LOG_INFO("Instance uninitialized: %d.", p_instance->drv_inst_idx);
}

void xincx_i2c_enable(xincx_i2c_t const * p_instance)
{
    i2c_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINCX_ASSERT(p_cb->state == XINCX_DRV_STATE_INITIALIZED);

    XINC_I2C_Type * p_i2c = p_instance->p_i2c;
	
    xinc_i2c_enable(p_i2c);
	
    p_cb->state = XINCX_DRV_STATE_POWERED_ON;
    XINCX_LOG_INFO("Instance enabled: %d.", p_instance->drv_inst_idx);
}

void xincx_i2c_disable(xincx_i2c_t const * p_instance)
{
    i2c_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINCX_ASSERT(p_cb->state != XINCX_DRV_STATE_UNINITIALIZED);

    XINC_I2C_Type * p_i2c = p_instance->p_i2c;
    
    xinc_i2c_int_disable(p_i2c, XINC_I2C_INT_DIS_MASK_ALL);
    xinc_i2c_disable(p_i2c);
    
    p_cb->state = XINCX_DRV_STATE_INITIALIZED;
    XINCX_LOG_INFO("Instance disabled: %d.", p_instance->drv_inst_idx);
}

static bool i2c_send_byte(XINC_I2C_Type          * p_i2c,
                          i2c_control_block_t   * p_cb)
{
		
  //  printf("%s,bytes_transferred:%d,curr_length:%d\r\n",__func__,p_cb->bytes_transferred,p_cb->curr_length);
  //  printf("i2c_STATUS:0x%x\r\n",p_i2c->i2c_STATUS);
    
    
    if (p_cb->bytes_transferred  == p_cb->curr_length)
    {
        return false;
    }
    if((p_i2c->i2c_STATUS & XINC_I2C_STATUS_MASK_TFNF) != XINC_I2C_STATUS_MASK_TFNF)
    {
        return true;	
    }
    
    uint16_t data_cmd =  I2C_FLAG_HAS_START(p_cb->flags )? (I2C_DATA_CMD_RESTART_Set << I2C_DATA_CMD_RESTART_Pos) : 0x0;
    if ((p_cb->bytes_transferred  +  1) < p_cb->curr_length)
    {
        xinc_i2c_txd_set(p_i2c, p_cb->p_curr_buf[p_cb->bytes_transferred] | data_cmd);
    }
    else
    {
        if (p_cb->curr_tx_no_stop)
        {
            xinc_i2c_txd_set(p_i2c, p_cb->p_curr_buf[p_cb->bytes_transferred] | data_cmd);
            return false;
        }
        else if(I2C_FLAG_SUSPEND(p_cb->flags))
        {
            xinc_i2c_txd_set(p_i2c, p_cb->p_curr_buf[p_cb->bytes_transferred] | data_cmd);
            p_cb->prev_suspend = I2C_SUSPEND_TX;
            return false;
        }
        else
        {
            xinc_i2c_txd_set(p_i2c, p_cb->p_curr_buf[p_cb->bytes_transferred] | (I2C_DATA_CMD_STOP_Set << I2C_DATA_CMD_STOP_Pos) | data_cmd);
            return false;
        }
    }
    return true;
}

static bool i2c_receive_byte(XINC_I2C_Type         * p_i2c,
                             i2c_control_block_t  * p_cb)
{
	

	//	hw_timeout = HW_TIMEOUT;
    uint16_t data_cmd =  (I2C_DATA_CMD_CMD_Read << I2C_DATA_CMD_CMD_Pos) & I2C_DATA_CMD_CMD_Msk;

 //   printf("curr_length:%d, transferred:%d,transForRx:%d\r\n ",p_cb->curr_length,p_cb->bytes_transferred,p_cb->bytes_transForRx);
    if(p_cb->bytes_transForRx)
    {
        if((p_i2c->i2c_STATUS & XINC_I2C_STATUS_MASK_RFNE) == XINC_I2C_STATUS_MASK_RFNE)//??????RX_FIFO??????
        {
            p_cb->p_curr_buf[p_cb->bytes_transferred] = xinc_i2c_rxd_get(p_i2c) & I2C_DATA_CMD_DAT_Msk;
            ++(p_cb->bytes_transferred);		
        }
    }
    if(p_cb->bytes_transferred == p_cb->curr_length)
    {
        return false;
    }
        
    if (p_cb->bytes_transForRx  + 1 < p_cb->curr_length)
    {					
        if((p_i2c->i2c_STATUS & XINC_I2C_STATUS_MASK_TFNF) != XINC_I2C_STATUS_MASK_TFNF)
        {
        //	printf("txd_set :0x%x fail\r\n ",data_cmd);
            return true;
        }

        xinc_i2c_txd_set(p_i2c, data_cmd);
        p_cb->bytes_transForRx++;
                                        
    }
    else if((p_cb->bytes_transForRx + 1) == p_cb->curr_length)
    {
        if ((p_cb->bytes_transferred == p_cb->curr_length - 2) && (!I2C_FLAG_SUSPEND(p_cb->flags)))
        {							
            if((p_i2c->i2c_STATUS & XINC_I2C_STATUS_MASK_TFNF) != XINC_I2C_STATUS_MASK_TFNF)
            {
                return true;
            }
            data_cmd |= (I2C_DATA_CMD_STOP_Set << I2C_DATA_CMD_STOP_Pos);
            xinc_i2c_txd_set(p_i2c, data_cmd);
            p_cb->bytes_transForRx++;

            if((p_i2c->i2c_STATUS & XINC_I2C_STATUS_MASK_RFNE) != XINC_I2C_STATUS_MASK_RFNE)
            {
                return true;
            }
            p_cb->p_curr_buf[p_cb->bytes_transferred] = xinc_i2c_rxd_get(p_i2c) & I2C_DATA_CMD_DAT_Msk;
            ++(p_cb->bytes_transferred);
        
            if((p_i2c->i2c_STATUS & XINC_I2C_STATUS_MASK_RFNE) != XINC_I2C_STATUS_MASK_RFNE)
            {
                return true;
            }
            p_cb->p_curr_buf[p_cb->bytes_transferred] = xinc_i2c_rxd_get(p_i2c) & I2C_DATA_CMD_DAT_Msk;
            ++(p_cb->bytes_transferred);
            p_cb->bytes_transForRx = 0;
            return false;
        }
        else if ((p_cb->bytes_transferred == p_cb->curr_length - 1) && (!I2C_FLAG_SUSPEND(p_cb->flags)))
        {		
          //  printf("i2c_STATUS:%x\r\n",p_i2c->i2c_STATUS);
            if((p_i2c->i2c_STATUS & XINC_I2C_STATUS_MASK_TFNF) != XINC_I2C_STATUS_MASK_TFNF)
            {
                return true;
            }
            
            data_cmd |= I2C_DATA_CMD_STOP_Msk;
            xinc_i2c_txd_set(p_i2c, data_cmd);
            p_cb->bytes_transForRx++;
            if((p_i2c->i2c_STATUS & XINC_I2C_STATUS_MASK_RFNE) != XINC_I2C_STATUS_MASK_RFNE)
            {
                return true;
            }
            p_cb->p_curr_buf[p_cb->bytes_transferred] = xinc_i2c_rxd_get(p_i2c) & I2C_DATA_CMD_DAT_Msk;
            ++(p_cb->bytes_transferred);
            p_cb->bytes_transForRx = 0;

            return false;
        }
		
    }
    return true;
}

static bool i2c_transfer(XINC_I2C_Type           * p_i2c,
                         i2c_control_block_t    * p_cb)
{
    bool stopped = false;

    {
        if (p_cb->xfer_desc.type == XINCX_I2C_XFER_TX)
        {
            ++(p_cb->bytes_transferred);
            XINCX_LOG_DEBUG("I2C: Type: %s.", TRANSFER_TO_STR(XINCX_I2C_XFER_TX));
    
            if (!i2c_send_byte(p_i2c, p_cb))
            {
                stopped = true;
                return false;
            }
            
        }
		else if (p_cb->xfer_desc.type == XINCX_I2C_XFER_RX)
        {
            XINCX_LOG_DEBUG("I2C: Type: %s.", TRANSFER_TO_STR(XINCX_I2C_XFER_RX));
             
            if (!i2c_receive_byte(p_i2c, p_cb))
            {
                stopped = true;
                return false;
            }else
            {
            //printf("contiu receice byte\r\n");
            }
            
        }
    }

    if (stopped)
    {
        p_cb->prev_suspend = I2C_NO_SUSPEND;
        if (!p_cb->error)
        {
            p_cb->error = !xfer_completeness_check(p_i2c, p_cb);
        }
        return false;
    }

    return true;
}

static xincx_err_t i2c_tx_start_transfer(XINC_I2C_Type        * p_i2c,
                                        i2c_control_block_t * p_cb)
{
    xincx_err_t ret_code = XINCX_SUCCESS;
    volatile int32_t hw_timeout;
    bool contiu_send_flag = true;

    hw_timeout = HW_TIMEOUT;
	//	printf("%s\r\n",__func__);

    p_cb->bytes_transferred = 0;
    p_cb->bytes_transForRx = 0;
    p_cb->error             = false;
    p_cb->flags &= ~(XINCX_I2C_FLAG_TX_HAS_START);
	


    if (p_cb->prev_suspend != I2C_SUSPEND_TX)
    {
        p_cb->flags |= (XINCX_I2C_FLAG_TX_HAS_START);
    }
   
    xinc_i2c_disable(p_i2c);
    p_i2c->i2c_TAR = p_cb->xfer_desc.address;
    xinc_i2c_enable(p_i2c);	
    contiu_send_flag = i2c_send_byte(p_i2c, p_cb);
		
    p_cb->flags &= ~(XINCX_I2C_FLAG_TX_HAS_START);

    if (p_cb->handler)
    {
        p_cb->int_mask = XINC_I2C_INT_EN_MASK_TX_EMPTY;

        xinc_i2c_enable(p_i2c);
        while(((p_i2c->i2c_STATUS & XINC_I2C_STATUS_MASK_ACTIVITY) != XINC_I2C_STATUS_MASK_ACTIVITY) && (hw_timeout > 0))
        {
               hw_timeout --;
        }
        if (hw_timeout > 0)
        {
            xinc_i2c_int_enable(p_i2c,p_cb->int_mask);
            XINCX_IRQ_ENABLE(I2C_IRQn);
        }else
        {
            xinc_i2c_disable(p_i2c);
            XINCX_IRQ_DISABLE(I2C_IRQn);
            ret_code = XINCX_ERROR_TIMEOUT;
        }
			
    }
    else
    {
        if(contiu_send_flag)
        {
            while ((hw_timeout > 0) &&
            i2c_transfer(p_i2c, p_cb))
            {
                hw_timeout--;
            }
        }		
        if (p_cb->error)
        {
            uint32_t errorsrc =  xinc_i2c_errorsrc_get_and_clear(p_i2c);

            if (errorsrc)
            {
                ret_code = i2c_process_error(errorsrc);
            }
            else
            {
                ret_code = XINCX_ERROR_INTERNAL;
            }
        }
        if (hw_timeout <= 0)
        {
            xinc_i2c_disable(p_i2c);
            ret_code = XINCX_ERROR_INTERNAL;
        }
    }
    return ret_code;
}

static xincx_err_t i2c_rx_start_transfer(XINC_I2C_Type        * p_i2c,
                                        i2c_control_block_t * p_cb)
{
    xincx_err_t ret_code = XINCX_SUCCESS;
    volatile int32_t hw_timeout;

    hw_timeout = HW_TIMEOUT;

    p_cb->bytes_transferred = 0;
    p_cb->bytes_transForRx = 0;
    p_cb->error             = false;

    xinc_i2c_disable(p_i2c);

    if (p_cb->prev_suspend != I2C_SUSPEND_RX)
    {

    }
	
    if (p_cb->handler)
    {
        p_cb->int_mask = XINC_I2C_INT_EN_MASK_RX_FULL|XINC_I2C_INT_EN_MASK_TX_EMPTY;
        
        xinc_i2c_enable(p_i2c);
  
        while(((p_i2c->i2c_STATUS & XINC_I2C_STATUS_MASK_ACTIVITY) != XINC_I2C_STATUS_MASK_ACTIVITY) && (hw_timeout > 0))
        {
               hw_timeout --;
        }
        if (hw_timeout > 0)
        {
            printf("hw_timeout:%d\r\n",hw_timeout);
            xinc_i2c_int_enable(p_i2c,p_cb->int_mask);
            XINCX_IRQ_ENABLE(I2C_IRQn);
        }else
        {
            xinc_i2c_disable(p_i2c);
            XINCX_IRQ_DISABLE(I2C_IRQn);
            ret_code = XINCX_ERROR_TIMEOUT;
        }
        
    }
    else
    {
        while ((hw_timeout > 0) &&
               i2c_transfer(p_i2c, p_cb))
        {
            hw_timeout--;
        }
        if (p_cb->error)
        {
            uint32_t errorsrc =  xinc_i2c_errorsrc_get_and_clear(p_i2c);

            if (errorsrc)
            {
                ret_code = i2c_process_error(errorsrc);
            }
            else
            {
                ret_code = XINCX_ERROR_INTERNAL;
            }
        }
        if (hw_timeout <= 0)
        {
           
            xinc_i2c_disable(p_i2c);
            ret_code = XINCX_ERROR_INTERNAL;
        }
    }
    printf("rx_start_transfer ret_code:%x\r\n",ret_code);
    return ret_code;
}

__STATIC_INLINE xincx_err_t i2c_xfer(XINC_I2C_Type               * p_i2c,
                                    i2c_control_block_t        * p_cb,
                                    xincx_i2c_xfer_desc_t const * p_xfer_desc,
                                    uint32_t                     flags)
{

    xincx_err_t err_code = XINCX_SUCCESS;

    if ((p_cb->prev_suspend == I2C_SUSPEND_TX) && (p_xfer_desc->type == XINCX_I2C_XFER_RX))
    {
        /* RX is invalid after TX suspend */
        return XINCX_ERROR_INVALID_STATE;
    }
    else if ((p_cb->prev_suspend == I2C_SUSPEND_RX) && (p_xfer_desc->type != XINCX_I2C_XFER_RX))
    {
        /* TX, TXRX and TXTX are invalid after RX suspend */
        return XINCX_ERROR_INVALID_STATE;
    }

    /* Block I2C interrupts to ensure that function is not interrupted by I2C interrupt. */

    uint32_t reg;
    xinc_i2c_int_disable(p_i2c,XINC_I2C_INT_DIS_MASK_ALL);
    reg = p_i2c->i2c_CLR_INTR;
    
    if (p_cb->busy)
    {
        err_code = XINCX_ERROR_BUSY;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    else
    {
        p_cb->busy = (I2C_FLAG_NO_HANDLER_IN_USE(flags)) ? false : true;
    }

    p_cb->flags       = flags;
    p_cb->xfer_desc   = *p_xfer_desc;
    p_cb->curr_length = p_xfer_desc->primary_length;
    p_cb->p_curr_buf  = p_xfer_desc->p_primary_buf;

    if (p_xfer_desc->type != XINCX_I2C_XFER_RX)
    {
        p_cb->curr_tx_no_stop = ((p_xfer_desc->type == XINCX_I2C_XFER_TX) &&
                                 !(flags & XINCX_I2C_FLAG_TX_NO_STOP)) ? false : true;

        err_code = i2c_tx_start_transfer(p_i2c, p_cb);
    }
    else
    {
        err_code = i2c_rx_start_transfer(p_i2c, p_cb);
    }
    if (p_cb->handler == NULL)
    {
        p_cb->busy = false;
    }
    
    (void)reg;
    return err_code;
}

bool xincx_i2c_is_busy(xincx_i2c_t const * p_instance)
{
    i2c_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    return p_cb->busy;
}

xincx_err_t xincx_i2c_xfer(xincx_i2c_t           const * p_instance,
                         xincx_i2c_xfer_desc_t const * p_xfer_desc,
                         uint32_t                     flags)
{

    xincx_err_t err_code = XINCX_SUCCESS;
    i2c_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    // TXRX and TXTX transfers are supported only in non-blocking mode.
    XINCX_ASSERT( !((p_cb->handler == NULL) && (p_xfer_desc->type == XINCX_I2C_XFER_TXRX)));
    XINCX_ASSERT( !((p_cb->handler == NULL) && (p_xfer_desc->type == XINCX_I2C_XFER_TXTX)));

    XINCX_LOG_INFO("Transfer Type: %s.", TRANSFER_TO_STR(p_xfer_desc->type));
    XINCX_LOG_INFO("Transfer buffers length: primary: %d, secondary: %d.",
                  p_xfer_desc->primary_length,
                  p_xfer_desc->secondary_length);
    XINCX_LOG_DEBUG("Primary buffer data:");
    XINCX_LOG_HEXDUMP_DEBUG(p_xfer_desc->p_primary_buf,
                           p_xfer_desc->primary_length * sizeof(p_xfer_desc->p_primary_buf[0]));
    XINCX_LOG_DEBUG("Secondary buffer data:");
    XINCX_LOG_HEXDUMP_DEBUG(p_xfer_desc->p_secondary_buf,
                           p_xfer_desc->secondary_length * sizeof(p_xfer_desc->p_secondary_buf[0]));

    err_code = i2c_xfer((XINC_I2C_Type  *)p_instance->p_i2c, p_cb, p_xfer_desc, flags);
    XINCX_LOG_WARNING("Function: %s, error code: %s.",
                     __func__,
                     XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

xincx_err_t xincx_i2c_tx(xincx_i2c_t const * p_instance,
                       uint8_t            address,
                       uint8_t    const * p_data,
                       size_t             length,
                       bool               no_stop)
{
    xincx_i2c_xfer_desc_t xfer = XINCX_I2C_XFER_DESC_TX(address, (uint8_t*)p_data, length);
    return xincx_i2c_xfer(p_instance, &xfer, no_stop ? XINCX_I2C_FLAG_TX_NO_STOP : 0);
}

xincx_err_t xincx_i2c_rx(xincx_i2c_t const * p_instance,
                       uint8_t            address,
                       uint8_t *          p_data,
                       size_t             length)
{
    xincx_i2c_xfer_desc_t xfer = XINCX_I2C_XFER_DESC_RX(address, p_data, length);
    return xincx_i2c_xfer(p_instance, &xfer, 0);
}

size_t xincx_i2c_data_count_get(xincx_i2c_t const * const p_instance)
{
    return m_cb[p_instance->drv_inst_idx].bytes_transferred;
}


static void i2c_irq_handler(XINC_I2C_Type * p_i2c, i2c_control_block_t * p_cb)
{
    XINCX_ASSERT(p_cb->handler);

    uint32_t reg;
    if (i2c_transfer(p_i2c, p_cb))
    {
        return;
    }
    if(p_i2c->i2c_TXFLR)
    {
        return;
    }
    xinc_i2c_disable(p_i2c);
    reg = p_i2c->i2c_CLR_INTR;
    xinc_i2c_int_disable(p_i2c,XINC_I2C_INT_DIS_MASK_ALL);
    XINCX_IRQ_DISABLE(I2C_IRQn);
    
    if (!p_cb->error &&
    ((p_cb->xfer_desc.type == XINCX_I2C_XFER_TXRX) ||
        (p_cb->xfer_desc.type == XINCX_I2C_XFER_TXTX)) &&
    p_cb->p_curr_buf == p_cb->xfer_desc.p_primary_buf)
    {
        p_cb->p_curr_buf      = p_cb->xfer_desc.p_secondary_buf;
        p_cb->curr_length     = p_cb->xfer_desc.secondary_length;
        p_cb->curr_tx_no_stop = (p_cb->flags & XINCX_I2C_FLAG_TX_NO_STOP);
        p_cb->prev_suspend    = I2C_NO_SUSPEND;

        if (p_cb->xfer_desc.type == XINCX_I2C_XFER_TXTX)
        {
            (void)i2c_tx_start_transfer(p_i2c, p_cb);
        }
        else
        {
            (void)i2c_rx_start_transfer(p_i2c, p_cb);
        }
    }
    else
    {
        xincx_i2c_evt_t event;
        event.xfer_desc = p_cb->xfer_desc;

        if (p_cb->error)
        {
          //  uint32_t errorsrc = xinc_i2c_errorsrc_get_and_clear(p_i2c);
            uint32_t errorsrc = (p_i2c->i2c_TX_ABRT_SOURCE);
            
        }
        else
        {
            event.type = XINCX_I2C_EVT_DONE;
        }

        p_cb->busy = false;

        if (!(I2C_FLAG_NO_HANDLER_IN_USE(p_cb->flags)) || p_cb->error)
        {
            p_cb->handler(&event, p_cb->p_context);
					
        }
    }
    
   (void)reg;

}


#if XINCX_CHECK(XINCX_I2C0_ENABLED)
void I2C_Handler()
{
	xincx_i2c_0_irq_handler();

}
void xincx_i2c_0_irq_handler(void)
{
    i2c_irq_handler(XINC_I2C0, &m_cb[XINCX_I2C0_INST_IDX]);
}
#endif


#endif // XINCX_CHECK(XINCX_I2C_ENABLED)
