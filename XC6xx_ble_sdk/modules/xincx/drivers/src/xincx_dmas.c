/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include <xincx.h>

#if !(XINCX_CHECK(XINCX_DMAS_ENABLED))
#error "No enabled DMAS instances. Check <xincx_config.h>."
#else
#include <xincx_dmas.h>

#define XINCX_LOG_MODULE DMAS
#include <xincx_log.h>


#ifdef DMAS_PRESENT
#define INSTANCE_CH_COUNT   DMAS_CH_COUNT
#else
#define INSTANCE_CH_COUNT   DMAS_COUNT
#endif

// Control block - driver instance local data.
typedef struct
{
    xincx_dmas_evt_handler_t  handler;

    void *                    p_context;
    uint8_t                   tx_ch;
    uint8_t                   rx_ch;
    uint32_t                  flags;

    xincx_drv_state_t         state;
    bool                      error;
} dmas_control_block_t;

static dmas_control_block_t m_cb[XINCX_DMAS_ENABLED_COUNT];
static xincx_dmas_ch_evt_handler_t  ch_handler[INSTANCE_CH_COUNT];

static xincx_dmas_t m_dmas = XINCX_DMAS_INSTANCE(0);

xincx_err_t xincx_dmas_init(xincx_dmas_t const *        p_instance0,
                         xincx_dmas_config_t const * p_config,
                         xincx_dmas_evt_handler_t    event_handler,
                         void *                    p_context)
{

    xincx_err_t err_code = XINCX_SUCCESS;
    xincx_dmas_t const*  p_instance;
    
    if(p_instance0 == NULL)
    {
        p_instance  = (xincx_dmas_t *)&m_dmas;
    }else
    {
        p_instance = p_instance0;
    }

    dmas_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    
    p_cb->handler = event_handler;
    XINC_DMAS_Type * p_reg = p_instance->p_dmas;
    
    if (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                            __func__,
                            XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    
    p_cb->state  = XINCX_DRV_STATE_INITIALIZED;

    return err_code;
}


bool xincx_dmas_is_init(xincx_dmas_t const * p_instance0)
{
    xincx_dmas_t const*  p_instance;
    
    if(p_instance0 == NULL)
    {
        p_instance  = (xincx_dmas_t *)&m_dmas;
    }else
    {
        p_instance = p_instance0;
    }
    dmas_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    
    
    return (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED) ? true : false;
}

xincx_err_t xincx_dmas_ch_param_set(xincx_dmas_t const *p_instance0,xincx_dmas_ch_set_t set)
{

    xincx_err_t err_code = XINCX_SUCCESS;

    xincx_dmas_t const*  p_instance;
    
    if(p_instance0 == NULL)
    {
        p_instance  = (xincx_dmas_t *)&m_dmas;
    }else
    {
        p_instance = p_instance0;
    }
    
    dmas_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    
    XINC_DMAS_Type * p_reg = p_instance->p_dmas;
    
    volatile uint32_t*    p_dma_ch_sar_base;
    volatile uint32_t*    p_dma_ch_dar_base;  
    volatile uint32_t*    p_dma_ch_ctl0_base;
    volatile uint32_t*    p_dma_ch_ctl1_base;     
    uint8_t ch_idx;
    switch(set.ch)
    {

        case 0: 
        case 1: 
        case 2: 
        case 3:         
        {
            ch_idx = set.ch - 0;
            p_dma_ch_sar_base = (uint32_t*)&p_reg->DMAs_CH0_SAR;
            p_dma_ch_dar_base = (uint32_t*)&p_reg->DMAs_CH0_DAR;
            p_dma_ch_ctl0_base = (uint32_t*)&p_reg->DMAs_CH0_CTL0;
            p_dma_ch_ctl1_base = (uint32_t*)&p_reg->DMAs_CH0_CTL1;
            p_dma_ch_sar_base += (ch_idx * 8);
            p_dma_ch_dar_base += (ch_idx * 8);
            p_dma_ch_ctl0_base += (ch_idx * 8);
            p_dma_ch_ctl1_base += (ch_idx * 8);
                      
            *p_dma_ch_sar_base = set.src_addr;
            *p_dma_ch_dar_base = set.dst_addr;
            *p_dma_ch_ctl1_base = set.ctl1;
            *p_dma_ch_ctl0_base = set.ctl0;


        }
        break;

        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        {
            uint8_t ch_idx = set.ch - 8;
            p_dma_ch_sar_base = (uint32_t*)&p_reg->DMAs_CH8_SAR;
            p_dma_ch_dar_base = (uint32_t*)&p_reg->DMAs_CH8_DAR;
            p_dma_ch_ctl0_base = (uint32_t*)&p_reg->DMAs_CH8_CTL0;
            p_dma_ch_ctl1_base = (uint32_t*)&p_reg->DMAs_CH8_CTL1;
            p_dma_ch_sar_base += (ch_idx * 8);
            p_dma_ch_dar_base += (ch_idx * 8);
            p_dma_ch_ctl0_base += (ch_idx * 8);
            p_dma_ch_ctl1_base += (ch_idx * 8);
                        
            *p_dma_ch_sar_base = set.src_addr;
            *p_dma_ch_dar_base = set.dst_addr;
            *p_dma_ch_ctl1_base = set.ctl1;
            *p_dma_ch_ctl0_base = set.ctl0;
             
        }break;
    }


    return err_code;
}

xincx_err_t xincx_dmas_ch_enable(xincx_dmas_t const *p_instance0,xinc_dma_ch_t ch)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    xincx_dmas_t const*  p_instance;
    
    if(p_instance0 == NULL)
    {
        p_instance  = (xincx_dmas_t *)&m_dmas;
    }else
    {
        p_instance = p_instance0;
    }
    dmas_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    
    XINC_DMAS_Type * p_reg = p_instance->p_dmas;
    
    xinc_dmas_enable(p_reg,ch);

    return err_code;
}


xincx_err_t xincx_dmas_ch_disable(xincx_dmas_t const *p_instance0,xinc_dma_ch_t ch)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    
    xincx_dmas_t const*  p_instance;
    
    if(p_instance0 == NULL)
    {
        p_instance  = (xincx_dmas_t *)&m_dmas;
    }else
    {
        p_instance = p_instance0;
    }
       
    XINC_DMAS_Type * p_reg = p_instance->p_dmas;
    
    xinc_dmas_clear(p_reg,ch);

    return err_code;
}

uint32_t xincx_dmas_stat_get(xincx_dmas_t const *p_instance0)
{
    xincx_dmas_t const*  p_instance;
    
    if(p_instance0 == NULL)
    {
        p_instance  = (xincx_dmas_t *)&m_dmas;
    }else
    {
        p_instance = p_instance0;
    }
       
    XINC_DMAS_Type * p_reg = p_instance->p_dmas;
    
    return xinc_dmas_stat(p_reg);

}

xincx_err_t xincx_dmas_int_enable(xincx_dmas_t const *p_instance0,uint32_t mask)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    xincx_dmas_t const*  p_instance;
    
    if(p_instance0 == NULL)
    {
        p_instance  = (xincx_dmas_t *)&m_dmas;
    }else
    {
        p_instance = p_instance0;
    }
       
    XINC_DMAS_Type * p_reg = p_instance->p_dmas;
    xinc_dmas_int_enable(p_reg,mask);
    
    return err_code;
}

xincx_err_t xincx_dmas_int_disable(xincx_dmas_t const *p_instance0,uint32_t mask)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    xincx_dmas_t const*  p_instance;
    
    if(p_instance0 == NULL)
    {
        p_instance  = (xincx_dmas_t *)&m_dmas;
    }else
    {
        p_instance = p_instance0;
    }
       
    XINC_DMAS_Type * p_reg = p_instance->p_dmas;
    xinc_dmas_int_disable(p_reg,mask);
    
    return err_code;
}

uint32_t xincx_dmas_int_stat_get(xincx_dmas_t const *p_instance0)
{
    xincx_dmas_t const*  p_instance;
    
    if(p_instance0 == NULL)
    {
        p_instance  = (xincx_dmas_t *)&m_dmas;
    }else
    {
        p_instance = p_instance0;
    }
       
    XINC_DMAS_Type * p_reg = p_instance->p_dmas;
    
    return xinc_dmas_int_sta(p_reg);
}

uint32_t xincx_dmas_int_raw_stat_get(xincx_dmas_t const *p_instance0)
{
    xincx_dmas_t const*  p_instance;
    
    if(p_instance0 == NULL)
    {
        p_instance  = (xincx_dmas_t *)&m_dmas;
    }else
    {
        p_instance = p_instance0;
    }
       
    XINC_DMAS_Type * p_reg = p_instance->p_dmas;
    
    return xinc_dmas_int_raw_sta(p_reg);
}

xincx_err_t xincx_dmas_int_sta_clear(xincx_dmas_t const *p_instance0,uint32_t clr_bits)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    xincx_dmas_t const*  p_instance;
    
    if(p_instance0 == NULL)
    {
        p_instance  = (xincx_dmas_t *)&m_dmas;
    }else
    {
        p_instance = p_instance0;
    }
       
    XINC_DMAS_Type * p_reg = p_instance->p_dmas;
    xinc_dmas_int_sta_clr(p_reg,clr_bits);
    
    return err_code;
}

xincx_err_t xincx_dmas_ch_ca_get(xincx_dmas_t const *p_instance0,uint8_t ch,uint32_t *ch_ca)
{
    xincx_err_t err_code = XINCX_SUCCESS;

    ASSERT(ch_ca);
    xincx_dmas_t const*  p_instance;
    
    if(p_instance0 == NULL)
    {
        p_instance  = (xincx_dmas_t *)&m_dmas;
    }else
    {
        p_instance = p_instance0;
    }    
    
    volatile uint32_t* p_dma_ch_ca_base;
    XINC_DMAS_Type *p_reg = p_instance->p_dmas;
    
    uint8_t ch_idx;
    switch(ch)
    {
        case 0: 
        case 1: 
        case 2: 
        case 3:
        {
            ch_idx = ch - 0;
            p_dma_ch_ca_base = (uint32_t*)&p_reg->DMAs_CH0_CA;
            p_dma_ch_ca_base+= (ch_idx * 8);
            
            *ch_ca = *p_dma_ch_ca_base;
            
        }break;
        
        case 8: 
        case 9: 
        case 10: 
        case 11:
        case 12:
        {
            ch_idx = ch - 8;
            p_dma_ch_ca_base = (uint32_t*)&p_reg->DMAs_CH8_CA;
            p_dma_ch_ca_base+= (ch_idx * 8);
            
            *ch_ca = *p_dma_ch_ca_base;
            
        }break;
        
        default:
        {
            err_code = XINCX_ERROR_INVALID_PARAM;
        } break;
                   
    }
     
    return err_code;
}


xincx_err_t xincx_dmas_low_power_ctl(xincx_dmas_t const *p_instance,uint32_t ctl)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    ASSERT(p_instance);
       
    XINC_DMAS_Type * p_reg = p_instance->p_dmas;
    xinc_dmas_lp_ctl(p_reg,ctl);
    
    return err_code;
}


xincx_err_t xincx_dmas_ch_handler_register(uint8_t ch,xincx_dmas_ch_evt_handler_t handler)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    if((ch > INSTANCE_CH_COUNT) || (handler == NULL))
    {
        err_code = XINCX_ERROR_INVALID_PARAM;
        return err_code;
    }
    
    ch_handler[ch] = handler;
    
    return err_code;
}


static void irq_dma_handler(XINC_DMAS_Type *        p_dmas,
                             dmas_control_block_t * p_cb)
{
    uint16_t            dmas_ch;
    uint32_t            i;
    uint32_t            mask  = (uint32_t)0x01;
    uint32_t            int0 = p_dmas->DMAs_INT0;
    uint32_t            dmas_ca;
    
    dmas_ch  = (int0 & 0x1F0F);
    dmas_ch |= ((int0 >> 16UL) & 0xF);
    dmas_ch |= (((int0 >> 24UL) & 0x1F) << 8);
    
    for (i = 0; i < 32; i++)
    {
        if (mask & int0)
        {
            p_dmas->DMAs_INT_CLR  = i;
        }
        mask <<= 1;
    }
    mask  = (uint32_t)0x01;
    if (dmas_ch)
    {
        for (i = 0; i < INSTANCE_CH_COUNT; i++)
        {
            if (mask & dmas_ch)
            {

                xincx_dmas_ch_evt_handler_t handler = ch_handler[i];
                if (handler)
                {
                    xincx_dmas_ch_ca_get(NULL,i,&dmas_ca);
                    xincx_dmas_ch_evt_t ch_evt = 
                    {
                        .dmas_ch = i,
                        .ch_ca = dmas_ca,
                    
                    };
                    handler(&ch_evt, NULL);
                }
            }
            mask <<= 1;
            /* Incrementing to next event, utilizing the fact that events are grouped together
            * in ascending order. */
        }
    }
}
void DMAS_Handler()
{
   //  uint32_t iWK,INT0,RD_ADDR;
  //  __read_hw_reg32(DMAS_INT0 , INT0);
    
  //  printf("INT0:0x%x\r\n",INT0);
 //   __write_hw_reg32(DMAS_INT_CLR , 0xFFFFFFFF);
  //   __read_hw_reg32(DMAS_INT_RAW , iWK);
   //  printf("iWK:0x%x\r\n",iWK);
  //   __write_hw_reg32(DMAS_INT_RAW , iWK);
    
    irq_dma_handler(XINC_DMAS0, &m_cb[XINCX_DMAS0_INST_IDX]);

 
}



#endif //






