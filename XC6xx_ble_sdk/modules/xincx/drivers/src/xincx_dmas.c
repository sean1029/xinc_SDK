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

static dmas_control_block_t m_cb;//[XINCX_DMAS_ENABLED_COUNT];
static xincx_dmas_ch_evt_handler_t  ch_handler[INSTANCE_CH_COUNT];



xincx_err_t xincx_dmas_init(xincx_dmas_config_t const * p_config,
                         xincx_dmas_evt_handler_t    event_handler,
                         void *                    p_context)
{

    xincx_err_t err_code = XINCX_SUCCESS;

    dmas_control_block_t * p_cb = &m_cb;//[p_instance->drv_inst_idx];;
    
    p_cb->handler = event_handler;
    
   // if(p_cb->handler)
    {
        NVIC_EnableIRQ((IRQn_Type)(DMAS_IRQn));
    }
    XINC_DMAS_Type * p_reg = xinc_dmas_reg_decode();
    
    if (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                            __func__,
                            XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    
     xinc_dmas_intv_unit_set(p_reg,0xff);
    p_cb->state  = XINCX_DRV_STATE_INITIALIZED;

    return err_code;
}


bool xincx_dmas_is_init(void)
{
   
    dmas_control_block_t * p_cb = &m_cb;//[p_instance->drv_inst_idx];
    
    
    return (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED) ? true : false;
}

xincx_err_t xincx_dmas_ch_param_set(xincx_dmas_ch_set_t set)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    
    XINC_DMAS_Type * p_reg = xinc_dmas_reg_decode();
    
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
    #if defined (XC66XX_M4) 
        case 4:
        case 5:
    #endif            
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
//			printf("TX Channel=%d, addr:%p,DMAS_CHx_SAR:%08x\r\n",ch_idx ,p_dma_ch_sar_base,*p_dma_ch_sar_base);
//			printf("TX Channel=%d, addr:%p,DMAS_CHx_DAR:%08x\r\n",ch_idx , p_dma_ch_dar_base,*p_dma_ch_dar_base);
//			printf("TX Channel=%d, addr:%p,DMAS_CHx_CTL0:%08x\r\n",ch_idx , p_dma_ch_ctl0_base,*p_dma_ch_ctl0_base);
//			printf("TX Channel=%d, addr:%p,DMAS_CHx_CTL1:%08x\r\n",ch_idx , p_dma_ch_ctl1_base ,*p_dma_ch_ctl1_base);


        }
        break;

        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
    #if defined (XC66XX_M4) 
        case 13:
        case 14:
        case 15:
    #endif  
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
            
            if(!xincx_is_word_aligned((void const *)set.dst_addr))
            {
                err_code = XINCX_ERROR_INVALID_ADDR;
            }
            *p_dma_ch_sar_base = set.src_addr;
            *p_dma_ch_dar_base = set.dst_addr;
            *p_dma_ch_ctl1_base = set.ctl1;
            *p_dma_ch_ctl0_base = set.ctl0;
//            
//            printf("RX Channel=%d, addr:%p,DMAS_CHx_SAR:%08x\r\n",set.ch , p_dma_ch_sar_base,*p_dma_ch_sar_base);
//			printf("RX Channel=%d, addr:%p,DMAS_CHx_DAR:%08x\r\n",set.ch , p_dma_ch_dar_base,*p_dma_ch_dar_base);
//			printf("RX Channel=%d, addr:%p,DMAS_CHx_CTL0:%08x\r\n",set.ch , p_dma_ch_ctl0_base,*p_dma_ch_ctl0_base);
//			printf("RX Channel=%d, addr:%p,DMAS_CHx_CTL1:%08x\r\n",set.ch , p_dma_ch_ctl1_base ,*p_dma_ch_ctl1_base);
//             
//             
        }break;
    }


    return err_code;
}

xincx_err_t xincx_dmas_ch_enable(xinc_dma_ch_t ch)
{
    xincx_err_t err_code = XINCX_SUCCESS;
   
    XINC_DMAS_Type * p_reg = xinc_dmas_reg_decode();
    
    xinc_dmas_enable(p_reg,ch);

    return err_code;
}


xincx_err_t xincx_dmas_ch_disable(xinc_dma_ch_t ch)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    
   
       
    XINC_DMAS_Type * p_reg = xinc_dmas_reg_decode();
    
    xinc_dmas_clear(p_reg,ch);

    return err_code;
}

uint32_t xincx_dmas_stat_get(void)
{
        
    XINC_DMAS_Type * p_reg = xinc_dmas_reg_decode();
    
    return xinc_dmas_stat(p_reg);

}

xincx_err_t xincx_dmas_int_enable(uint32_t mask)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    
       
    XINC_DMAS_Type * p_reg = xinc_dmas_reg_decode();
    xinc_dmas_int_enable(p_reg,mask);
    
    return err_code;
}

xincx_err_t xincx_dmas_int_disable(uint32_t mask)
{
    xincx_err_t err_code = XINCX_SUCCESS;
          
    XINC_DMAS_Type * p_reg = xinc_dmas_reg_decode();
    
    xinc_dmas_int_disable(p_reg,mask);
    
    return err_code;
}

uint32_t xincx_dmas_int_stat_get(void)
{
         
    XINC_DMAS_Type * p_reg = xinc_dmas_reg_decode();
    
    return xinc_dmas_int_sta(p_reg);
}

uint32_t xincx_dmas_int_raw_stat_get(void)
{
         
    XINC_DMAS_Type * p_reg = xinc_dmas_reg_decode();
    
    return xinc_dmas_int_raw_sta(p_reg);
}

xincx_err_t xincx_dmas_int_sta_clear(uint32_t clr_bits)
{
    xincx_err_t err_code = XINCX_SUCCESS;
          
    XINC_DMAS_Type * p_reg = xinc_dmas_reg_decode();
    
    xinc_dmas_int_sta_clr(p_reg,clr_bits);
    
    return err_code;
}

xincx_err_t xincx_dmas_ch_ca_get(uint8_t ch,uint32_t *ch_ca)
{
    xincx_err_t err_code = XINCX_SUCCESS;

    ASSERT(ch_ca);
       
    volatile uint32_t* p_dma_ch_ca_base;
    
    XINC_DMAS_Type *p_reg = xinc_dmas_reg_decode();
    
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
		case 13:	
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


xincx_err_t xincx_dmas_low_power_ctl(uint32_t ctl)
{
    xincx_err_t err_code = XINCX_SUCCESS;
       
    XINC_DMAS_Type * p_reg = xinc_dmas_reg_decode();
    
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

#if defined (XC60XX_M0)  
    dmas_ch  = (int0 & 0x3F0F);
    dmas_ch |= ((int0 >> 16UL) & 0xF);
    dmas_ch |= (((int0 >> 24UL) & 0x3F) << 8);
#elif defined (XC66XX_M4)
    dmas_ch  = (int0 & 0xFF3F);
    dmas_ch |= ((int0 >> 16UL) & 0x3F);
    dmas_ch |= (((int0 >> 24UL) & 0xFF) << 8);
#endif    
    printf("dma int0:%x\r\n",int0);
    
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
                    xincx_dmas_ch_ca_get(i,&dmas_ca);
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
    irq_dma_handler(XINC_DMAS0, &m_cb);
}



#endif //






