/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include <xincx.h>

#if XINCX_CHECK(XINCX_KBS_ENABLED)
#include <xincx_kbs.h>
#include <xincx_gpio.h>
#include "xinc_bitmask.h"
#include <string.h>
#include "xinc_delay.h"

#define BIT_PER_MTXKEY 4
#define MTXKEYS MAX_MTXKEY_NUMBER

STATIC_ASSERT(BIT_PER_MTXKEY == 4);

static uint8_t m_mtxkey_states[MTXKEYS*BIT_PER_MTXKEY/8];

static uint8_t m_mtxkey_long_press_time[MTXKEYS];

//按键检测过程中检测到mtxkey的状态
typedef enum {
    MTXKEY_IDLE,
    MTXKEY_PRESS_ARMED,
    MTXKEY_PRESS_DETECTED,
    MTXKEY_PRESSED,
    MTXKEY_LONG_PRESSED_DETECTED,
    MTXKEY_LONG_PRESSED,
    MTXKEY_RELEASE_DETECTED
} mtxkey_state_t;

//KSB 检测过程中整个矩阵键盘的状态
typedef enum {
    KBS_IDLE,
    KBS_ACTIVE,
    KBS_RELEASE_DETECTED,
    KBS_RELEASED,
} kbs_state_t;

/* Retrieve given pin state. States are stored in pairs (4 bit per mtxkey) in byte array. */
static mtxkey_state_t mtxkey_state_get(uint8_t mtxkey_idx)
{
    uint8_t pair_state = m_mtxkey_states[mtxkey_idx >> 1];
    uint8_t state = (mtxkey_idx & 0x1) ? (pair_state >> BIT_PER_MTXKEY) : (pair_state & 0x0F);

    return (mtxkey_state_t)state;
}

/* Set mtxkey state. */
static void mtxkey_state_set(uint8_t mtxkey_idx, mtxkey_state_t state)
{
    uint8_t mask = (mtxkey_idx & 1) ? 0x0F : 0xF0;
    uint8_t state_mask = (mtxkey_idx & 1) ?
                        ((uint8_t)state << BIT_PER_MTXKEY) : (uint8_t)state;
    m_mtxkey_states[mtxkey_idx >> 1] &= mask;
    m_mtxkey_states[mtxkey_idx >> 1] |= state_mask;
}


typedef struct
{
    xincx_kbs_mtxkey_cfg_t     mtxkey_cfg[KBS_ROW_PIN_COUNT * KBS_COL_PIN_COUNT];
    
    int16_t                    row_pin_assignments[KBS_ROW_PIN_COUNT];
    int16_t                    col_pin_assignments[KBS_COL_PIN_COUNT];
    
    uint16_t                   mtxkey_size;
    uint32_t                   row_Bits_En;
    uint32_t                   col_Bits_En;
    
    uint8_t                    row_Bits_num;
    uint8_t                    detected_fun;//根据 row_Bits_num 的个数以及按下的row 个数选择相应的检测算法
    
    uint8_t                    dbc_intval;//debounce间隔
    uint16_t                   rprs_intval;//重复按键间隔
    uint8_t                    lprs_intval;//长按按键间隔
    
    uint16_t                   lprs_cal_cnt;
    uint32_t                   g_row_value;
    kbs_state_t                kbs_state;
    xincx_drv_state_t          state;
} kbs_control_block_t;

static kbs_control_block_t m_cb;

/*
*根据row_pin 值换算得到其在矩阵当中所在的mko_idx
其每个pin对应的矩阵idx可参考芯片手册文档
*/
__STATIC_INLINE int16_t kbs_mtxkey_row_pin_to_mko_idx(uint8_t row_pin_no)
{
    uint8_t mko_idx;
    if(row_pin_no <= XINC_GPIO_7)
    {
        mko_idx = row_pin_no;
        return mko_idx;
    }
    return -1;  
}

/*
*根据col_pin 值换算得到其在矩阵当中所在的mki_idx
其每个pin对应的矩阵idx可参考芯片手册文档
*/
__STATIC_INLINE int16_t kbs_mtxkey_col_pin_to_mki_idx(uint8_t col_pin_no)
{
    int8_t mki_idx;
    
    switch(col_pin_no)
    {
        case XINC_GPIO_8:
        case XINC_GPIO_9:
        {
            mki_idx = col_pin_no - 8UL; // 0 -- 1
        }break;
        
        case XINC_GPIO_20:
        case XINC_GPIO_21:
        case XINC_GPIO_22:
        case XINC_GPIO_23:
        case XINC_GPIO_24:
        case XINC_GPIO_25:
        {
            mki_idx = col_pin_no - 18UL;//2 -- 7
        }break;
        
        case XINC_GPIO_10:
        {
            mki_idx = 8UL; //8
        }break;
        
        case XINC_GPIO_14:
        case XINC_GPIO_15:
        case XINC_GPIO_16:
        case XINC_GPIO_17:
        {
            mki_idx = col_pin_no - 5UL;//9 -- 12
        }break;
        
        case XINC_GPIO_26:
        case XINC_GPIO_27:
        case XINC_GPIO_28:
        case XINC_GPIO_29:
        case XINC_GPIO_30:
        {
            mki_idx = col_pin_no - 13UL;//13 -- 17
        }break;
        
        default:
            mki_idx = -1;
        break;
            
    }
  return mki_idx;  
    
 
}

__STATIC_INLINE int16_t kbs_mtxkey_cfg_idx_get(uint8_t row_pin,uint8_t col_pin)
{
    for(uint8_t i = 0; i < MAX_MTXKEY_NUMBER; i++)
    {
        if((m_cb.mtxkey_cfg[i].row_pin == row_pin) && (m_cb.mtxkey_cfg[i].col_pin == col_pin))
        {
            return i;
        }
    }
    
    return NO_MTXKEY_ID;  
}

//保存矩阵中每个元素的信息
__STATIC_INLINE void xincx_kbs_mtxkey_use_by_handler_set(uint8_t        key_val,
                                                uint8_t                  row_pin,
                                                uint8_t                  col_pin,
                                                uint32_t                  mtxkey_id,
                                                xincx_kbs_mtxkey_handler_t handler)
{

    m_cb.mtxkey_cfg[mtxkey_id].key_val = key_val;
    m_cb.mtxkey_cfg[mtxkey_id].row_pin = row_pin;
    m_cb.mtxkey_cfg[mtxkey_id].col_pin = col_pin;
    m_cb.mtxkey_cfg[mtxkey_id].handler = handler;
}

//为矩阵按键中的每个元素分配一个未使用的 mtxkey_cfg 结构体
static int16_t xincx_kbs_mtxkey_handler_use_alloc(uint8_t key_val,uint8_t row_pin,
                                        uint8_t col_pin, xincx_kbs_mtxkey_handler_t handler)
{
    int16_t   id = NO_MTXKEY_ID;
    uint32_t i;

    uint32_t start_idx = 0;
    uint32_t end_idx   = MAX_MTXKEY_NUMBER;

    // critical section

    for (i = start_idx; i < end_idx; i++)
    {
        if (m_cb.mtxkey_cfg[i].handler == FORBIDDEN_HANDLER_ADDRESS)
        {
            xincx_kbs_mtxkey_use_by_handler_set(key_val,row_pin,col_pin, i, handler);
            id = i;
            break;
        }
    }
  //  printf("pin_handler_use_alloc :%d\r\n",handler_id);
    // critical section
    return id;
}


static void usr_event(int16_t mtxkey_idx, uint8_t type)
{

    if ((mtxkey_idx != NO_MTXKEY_ID)  && m_cb.mtxkey_cfg[mtxkey_idx].handler)
    {
       
        m_cb.mtxkey_cfg[mtxkey_idx].handler(mtxkey_idx,m_cb.mtxkey_cfg[mtxkey_idx].key_val, m_cb.mtxkey_cfg[mtxkey_idx].row_pin,m_cb.mtxkey_cfg[mtxkey_idx].col_pin, type);
    }
}

/* State machine processing. */
static void kbs_evt_handle(uint8_t mtxkey_idx, uint8_t value)
{
//	printf("evt_handle pin:%d,value:%d,state:%d\r\n",pin,value,state_get(pin));
    switch(mtxkey_state_get(mtxkey_idx))
    {
    case MTXKEY_IDLE:
        if (value)
        {
           
           // mtxkey_state_set(mtxkey_idx, MTXKEY_PRESS_ARMED);
            mtxkey_state_set(mtxkey_idx, MTXKEY_PRESS_DETECTED);

        }
        else
        {
            /* stay in IDLE */
            mtxkey_state_set(mtxkey_idx, MTXKEY_IDLE);

		//	printf("pin:%d,value:%d,sta:%d\r\n",pin,value,state_get(pin));
        }
        break;
    case MTXKEY_PRESS_ARMED:
        mtxkey_state_set(mtxkey_idx, value ? MTXKEY_PRESS_DETECTED : MTXKEY_IDLE);
        		
        break;
    case MTXKEY_PRESS_DETECTED:
        if (value)
        {
            mtxkey_state_set(mtxkey_idx, MTXKEY_PRESSED);
            usr_event(mtxkey_idx, KBS_MTXKEY_PUSH);
         //   printf("evt_handle mtxkey_idx:%d,MTXKEY_PRESSED\r\n",mtxkey_idx);
        }
        else
        {
            mtxkey_state_set(mtxkey_idx, MTXKEY_PRESS_ARMED);
        }
        
        break;
    case MTXKEY_PRESSED:
        if (value == 0)
        {
            
            mtxkey_state_set(mtxkey_idx, MTXKEY_RELEASE_DETECTED);
        }
        else
        {
            /* stay in pressed ,detecte long press*/
            
            m_mtxkey_long_press_time[mtxkey_idx]++;
            if(m_mtxkey_long_press_time[mtxkey_idx] > m_cb.lprs_cal_cnt)
            {
                mtxkey_state_set(mtxkey_idx, MTXKEY_LONG_PRESSED_DETECTED);
                usr_event(mtxkey_idx, KBS_MTXKEY_LONG_PUSH);
            //    printf("evt_handle mtxkey_idx:%d,LONG_PRESSED\r\n",mtxkey_idx);
            }
            
        }
        break;
        
    case  MTXKEY_LONG_PRESSED_DETECTED:
        if (value == 0)
        {
            mtxkey_state_set(mtxkey_idx, MTXKEY_RELEASE_DETECTED);
        }else
        {
            mtxkey_state_set(mtxkey_idx, MTXKEY_LONG_PRESSED);
        }
    break;
        
      case MTXKEY_LONG_PRESSED:
        if (value == 0)
        {
            
            mtxkey_state_set(mtxkey_idx, MTXKEY_RELEASE_DETECTED);
        }else
        {
            /* stay in long pressed */
        }        
          
      
      break;
    case MTXKEY_RELEASE_DETECTED:
        if (value)
        {
            mtxkey_state_set(mtxkey_idx, MTXKEY_PRESSED);
        }
        else
        {
            mtxkey_state_set(mtxkey_idx, MTXKEY_IDLE);
            m_mtxkey_long_press_time[mtxkey_idx] = 0;
            usr_event(mtxkey_idx, KBS_MTXKEY_RELEASE);
         //   printf("evt_handle mtxkey_idx:%d,RELEASE\r\n",mtxkey_idx);
        }
     
        break;
    }
}

xincx_err_t  xincx_kbs_init(xincx_kbs_mtxkey_cfg_t const *       p_mtxkeys,
                         uint8_t                        mtxkey_size,
                         xincx_kbs_intval_config_t  const *       p_config)
{
    xincx_err_t err_code = XINCX_SUCCESS;

    XINC_KBS_Type * p_reg = xinc_kbs_decode();
    
    XINC_CPR_CTL_Type * p_cpr = XINC_CPR;
    uint32_t int_en;
    uint16_t mtxkey_count;
    
    if (m_cb.state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
       
        printf("Function: %s, error code: %x.\r\n",
                            __func__,
                            (err_code));
        return err_code;
    }
    
    m_cb.mtxkey_size = mtxkey_size;
    
    mtxkey_count = mtxkey_size;
    
    uint16_t mtxkey_idx;
    for(uint8_t i = 0; i < KBS_ROW_PIN_COUNT; i++)
    {
        m_cb.row_pin_assignments[i] = PIN_NOT_USED;
        for(uint8_t j = 0; j < KBS_COL_PIN_COUNT; j++)
        {
            mtxkey_idx = i * KBS_ROW_PIN_COUNT + j;
            m_cb.mtxkey_cfg[mtxkey_idx].handler = FORBIDDEN_HANDLER_ADDRESS;
        }
    }
    
    for(uint8_t j = 0; j < KBS_COL_PIN_COUNT; j++)
    {
        m_cb.col_pin_assignments[j] = PIN_NOT_USED;
    }
       
    p_cpr->CTLAPBCLKEN_GRCTL = (CPR_CTLAPBCLKEN_GRCTL_KBS_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_KBS_PCLK_EN_Pos) | 
                                (CPR_CTLAPBCLKEN_GRCTL_KBS_PCLK_EN_Msk << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET);//0x840084;
    
    
    p_cpr->OTHERCLKEN_GRCTL = (CPR_OTHERCLKEN_GRCTL_KBS_CLK_EN_Enable << CPR_OTHERCLKEN_GRCTL_KBS_CLK_EN_Pos) | 
                              (CPR_OTHERCLKEN_GRCTL_KBS_CLK_EN_Msk << CPR_OTHERCLKEN_GRCTL_MASK_OFFSET);//0x90009;
    

    
    p_reg->CTL = KBS_KBS_CTL_MTXKEY_EN_Msk ;
    
    uint32_t kbs_mask = KBS_KBS_MASK_MTXKEY_ROW_MASK_Msk | KBS_KBS_MASK_MTXKEY_COL_MASK_Msk;

    printf("mtxkey_count:%d\r\n",mtxkey_count);
    
    if (p_config == NULL)
    {
        static const xincx_kbs_intval_config_t default_config = XINCX_KBS_INTVAL_DEFAULT_CONFIG;
        p_config = &default_config;
    }
    
    mtxkey_idx = 0;
    while (mtxkey_count--)
    {
        xincx_kbs_mtxkey_cfg_t const * p_mtxkey = &p_mtxkeys[mtxkey_idx];
               
        int16_t mtxkey_id = xincx_kbs_mtxkey_handler_use_alloc(p_mtxkey->key_val,p_mtxkey->row_pin,p_mtxkey->col_pin, p_mtxkey->handler);
        printf("mtxkey_id:%d\r\n",mtxkey_id);
        if (mtxkey_id != NO_MTXKEY_ID)
        {     
            int8_t mko_idx,mki_idx;
                     
            mko_idx = kbs_mtxkey_row_pin_to_mko_idx(p_mtxkey->row_pin);
            printf("mko_idx:%d\r\n",mko_idx);
            if(PIN_NO_FUN == mko_idx)
            {
                err_code = XINCX_ERROR_FORBIDDEN;
                return err_code;
            }
            if(m_cb.row_pin_assignments[mko_idx] == PIN_NOT_USED)
            {
                m_cb.row_pin_assignments[mko_idx] = p_mtxkey->row_pin;
                kbs_mask &= ~(0x01UL << mko_idx);
                m_cb.row_Bits_En |= (0x01UL << mko_idx);
                err_code = xinc_gpio_secfun_config(p_mtxkey->row_pin,XINC_GPIO_PIN_KBS_MTXKEY_MKO);
                printf("secfun_config p_mtxkey->row_pin:%d,err_code:%d\r\n",p_mtxkey->row_pin,err_code);
            } 
            
            mki_idx = kbs_mtxkey_col_pin_to_mki_idx(p_mtxkey->col_pin);
            printf("mki_idx:%d\r\n",mki_idx);
            if(PIN_NO_FUN == mki_idx)
            {
                err_code = XINCX_ERROR_FORBIDDEN;
                return err_code;
            }
            if(m_cb.col_pin_assignments[mki_idx] == PIN_NOT_USED)
            {
                m_cb.col_pin_assignments[mki_idx] = p_mtxkey->col_pin;
                kbs_mask &= ~(0x01UL << (mki_idx + KBS_KBS_MASK_MTXKEY_COL_MASK_Pos));
                m_cb.col_Bits_En |= (0x01UL << mki_idx);
                err_code = xinc_gpio_secfun_config(p_mtxkey->col_pin,XINC_GPIO_PIN_KBS_MTXKEY_MKI);
                printf("secfun_config p_mtxkey->col_pin:%d,err_code:%d\r\n",p_mtxkey->col_pin,err_code);
            } 
        }
        else
        {
            err_code = XINCX_ERROR_NO_MEM;
        }
        mtxkey_idx++;
        printf("p_mtxkey->mtxkey_id:%d,row_pin:%d,col_pin:%d\r\n",p_mtxkey->key_val,p_mtxkey->row_pin,p_mtxkey->col_pin);
    }

   
    p_reg->DETECT_INTVAL = ((p_config->prs_intval << KBS_DETECT_INTVAL_MTXKEY_PRS_INTVAL_Pos) & KBS_DETECT_INTVAL_MTXKEY_PRS_INTVAL_Msk) |
                           ((p_config->rls_intval << KBS_DETECT_INTVAL_MTXKEY_RLS_INTVAL_Pos) & KBS_DETECT_INTVAL_MTXKEY_RLS_INTVAL_Msk);
    p_reg->DBC_INTVAL = p_config->dbc_intval & KBS_DBC_INTVAL_MTXKEY_DBC_INTVAL_Msk;
   
    p_reg->LPRS_INTVAL = ((p_config->rprs_intval << KBS_LPRS_INTVAL_KBS_RPRS_INTVAL_Pos) & KBS_LPRS_INTVAL_KBS_RPRS_INTVAL_Msk) |
                           ((p_config->lprs_intval << KBS_LPRS_INTVAL_KBS_LPRS_INTVAL_Pos) & KBS_LPRS_INTVAL_KBS_LPRS_INTVAL_Msk);
    
    m_cb.dbc_intval = p_config->dbc_intval;//debounce间隔
    m_cb.rprs_intval = p_config->rprs_intval;//重复按键间隔
    m_cb.lprs_intval = p_config->lprs_intval;//长按按键间隔


    //根据所设置的长按时间计算长按所需要的计数值
    m_cb.lprs_cal_cnt = m_cb.lprs_intval * m_cb.rprs_intval / m_cb.dbc_intval;
    
    printf("mask_set:%x,m_cb.lprs_cal_cnt:%d\r\n",kbs_mask,m_cb.lprs_cal_cnt);
    xinc_kbs_mask_set(p_reg,kbs_mask);
    int_en = KBS_MTXKEY_INT_PRS_INT_EN_Msk;
    
    xinc_kbs_int_enable(p_reg,int_en);
    printf("xinc_kbs_int_enable p_reg:%p,ctl:0x%x,en:0x%x\r\n",p_reg,p_reg->CTL,p_reg->MTXKEY_INT_EN);

    NVIC_EnableIRQ(KBS_IRQn);
    
    printf("xinc_kbs_int_enable:%x\r\n",int_en);
    
    m_cb.state = XINCX_DRV_STATE_INITIALIZED;
    m_cb.kbs_state = KBS_IDLE;
      
    return err_code;
}




static bool kbs_mtxkey_value_check_funA(void)
{
    uint32_t row_out_reg ;
    uint32_t col_in_reg ;
    uint8_t rw_flag = 1;
    bool has_value = false; 
    XINC_KBS_Type * p_reg = xinc_kbs_decode();
    for(uint8_t row_idx  = 0 ; row_idx < KBS_ROW_PIN_COUNT; )
    {
        if((0x01UL << row_idx) & m_cb.row_Bits_En)
        {
            if(rw_flag == 1)
            {
                row_out_reg = (0x01UL << row_idx);
                p_reg->MTXKEY_MANUAL_ROWOUT = row_out_reg;
               //  printf("row_idx:%d,row_out_reg:0x%x \r\n",row_idx,row_out_reg);
                rw_flag = 0;
                xinc_delay_us(80);

            }
            else
            {
                col_in_reg =  p_reg->MTXKEY_MANUAL_COLIN ;
              //  printf("row_idx:%d,col_in_reg:0x%x \r\n",row_idx,col_in_reg);
                if(col_in_reg != 0)
                {
                    has_value = true;
                }
                for (uint8_t col_idx = 0; col_idx < KBS_COL_PIN_COUNT; col_idx++)
                {
                    int16_t mtxkey_idx;
                    bool is_set = 0;
                    if((0x01UL << col_idx) & m_cb.col_Bits_En)
                    {
                        is_set = col_in_reg & (0x01UL << col_idx);

                        mtxkey_idx = kbs_mtxkey_cfg_idx_get(m_cb.row_pin_assignments[row_idx],m_cb.col_pin_assignments[col_idx]);
                        if(mtxkey_idx != NO_MTXKEY_ID)
                        {
                            kbs_evt_handle(mtxkey_idx, is_set);
                        }
                    }
                }
                rw_flag = 1UL;
                row_idx++;
            }           
        }
        else
        {
            row_idx++;
        }
                                      
     }
         
     return  has_value;
}



static void mtxkey_set_mki_out_mko_input(void)
{
    for(uint8_t row_idx  = 0 ; row_idx < KBS_ROW_PIN_COUNT;row_idx++ )
    {
        if((0x01UL << row_idx) & m_cb.row_Bits_En)
        {                  
           xinc_gpio_mux_ctl(m_cb.row_pin_assignments[row_idx],0);
           xinc_gpio_fun_sel(m_cb.row_pin_assignments[row_idx],XINC_GPIO_PIN_GPIODx);
           xinc_gpio_pin_dir_set(m_cb.row_pin_assignments[row_idx],XINC_GPIO_PIN_DIR_INPUT);            
           xinc_gpio_pull_sel(m_cb.row_pin_assignments[row_idx],XINC_GPIO_PIN_PULLDOWN);

        }
    
    }
    
    for (uint8_t col_idx = 0; col_idx < KBS_COL_PIN_COUNT; col_idx++)
    {
        if((0x01UL << col_idx) & m_cb.col_Bits_En)
        {
            xinc_gpio_mux_ctl(m_cb.col_pin_assignments[col_idx],0);
            xinc_gpio_fun_sel(m_cb.col_pin_assignments[col_idx],XINC_GPIO_PIN_GPIODx);
            xinc_gpio_cfg_output(m_cb.col_pin_assignments[col_idx]);
            xinc_gpio_pin_dir_set(m_cb.col_pin_assignments[col_idx],XINC_GPIO_PIN_DIR_OUTPUT);
            xinc_gpio_pin_set(m_cb.col_pin_assignments[col_idx]);
        }
    }
}

static uint32_t mtxkey_get_mkoio_input_value(void)
{
    uint32_t io_value = 0;

    uint32_t port_value;
    port_value = xinc_gpio_port_read(0);
    for(uint8_t row_idx  = 0 ; row_idx < KBS_ROW_PIN_COUNT;row_idx++  )
    {
        if((0x01UL << row_idx) & m_cb.row_Bits_En)
        {
           // io_value |= (xinc_gpio_pin_read(m_cb.row_pin_assignments[row_idx]) << row_idx); 

            io_value |= ((( port_value & (0x01 << m_cb.row_pin_assignments[row_idx])) >> m_cb.row_pin_assignments[row_idx] ) << row_idx);             
        }
    }
    return io_value;
}


static void mtxkey_io_fun_relase(void)
{

    for(uint8_t row_idx  = 0 ; row_idx < KBS_ROW_PIN_COUNT;row_idx++  )
    {
        if((0x01UL << row_idx) & m_cb.row_Bits_En)
        {
            xinc_gpio_mux_ctl(m_cb.row_pin_assignments[row_idx],1);
            xinc_gpio_pin_dir_set(m_cb.row_pin_assignments[row_idx],XINC_GPIO_PIN_DIR_OUTPUT);
            
        }
    
    }
    
    for (uint8_t col_idx = 0; col_idx < KBS_COL_PIN_COUNT; col_idx++)
    {
        if((0x01UL << col_idx) & m_cb.col_Bits_En)
        {          
            xinc_gpio_mux_ctl(m_cb.col_pin_assignments[col_idx],1);
            xinc_gpio_pin_dir_set(m_cb.col_pin_assignments[col_idx],XINC_GPIO_PIN_DIR_INPUT);
            xinc_gpio_pull_sel(m_cb.col_pin_assignments[col_idx],XINC_GPIO_PIN_PULLDOWN);
        }
    }
}

static uint32_t kbs_mtxkey_row_value_check_by_gpio(void)
{
      
    XINC_KBS_Type * p_reg = xinc_kbs_decode();
    
    mtxkey_set_mki_out_mko_input();
    xinc_delay_us(80); 
    m_cb.g_row_value = mtxkey_get_mkoio_input_value();
         
    mtxkey_io_fun_relase();
     
    return  m_cb.g_row_value;
}


static bool kbs_mtxkey_value_check_funB(void)
{
    uint32_t row_out_reg ;
    uint32_t col_in_reg ;
    uint8_t rw_flag = 1;
    bool has_value = false;
    
    kbs_mtxkey_row_value_check_by_gpio();
    
    XINC_KBS_Type * p_reg = xinc_kbs_decode();
    for(uint8_t row_idx  = 0 ; row_idx < KBS_ROW_PIN_COUNT; )
    {
        if(((0x01UL << row_idx) & m_cb.row_Bits_En))
        {
            if(((0x01UL << row_idx) & m_cb.g_row_value) == 0 )
            {
                for (uint8_t col_idx = 0; col_idx < KBS_COL_PIN_COUNT; col_idx++)
                {
                    int16_t mtxkey_idx;
                    bool is_set = 0;
                    if((0x01UL << col_idx) & m_cb.col_Bits_En)
                    {
                        is_set = false;

                        mtxkey_idx = kbs_mtxkey_cfg_idx_get(m_cb.row_pin_assignments[row_idx],m_cb.col_pin_assignments[col_idx]);
                        if(mtxkey_idx != NO_MTXKEY_ID)
                        {
                            kbs_evt_handle(mtxkey_idx, is_set);
                        }
                    }
                }
                row_idx++;
            }
            else
            {     
                if(rw_flag == 1)
                {
                    row_out_reg = (0x01UL << row_idx);
                    p_reg->MTXKEY_MANUAL_ROWOUT = row_out_reg;
                   //  printf("row_idx:%d,row_out_reg:0x%x \r\n",row_idx,row_out_reg);
                    rw_flag = 0;
                    xinc_delay_us(80);

                }
                else
                {
                    col_in_reg =  p_reg->MTXKEY_MANUAL_COLIN ;
                  //  printf("row_idx:%d,col_in_reg:0x%x \r\n",row_idx,col_in_reg);
                    if(col_in_reg != 0)
                    {
                        has_value = true;
                    }
                    for (uint8_t col_idx = 0; col_idx < KBS_COL_PIN_COUNT; col_idx++)
                    {
                        int16_t mtxkey_idx;
                        bool is_set = 0;
                        if((0x01UL << col_idx) & m_cb.col_Bits_En)
                        {
                            is_set = col_in_reg & (0x01UL << col_idx);

                            mtxkey_idx = kbs_mtxkey_cfg_idx_get(m_cb.row_pin_assignments[row_idx],m_cb.col_pin_assignments[col_idx]);
                            if(mtxkey_idx != NO_MTXKEY_ID)
                            {
                                kbs_evt_handle(mtxkey_idx, is_set);
                            }
                        }
                    }
                    rw_flag = 1UL;
                    row_idx++;
                }

            }          
        }
        else
        {
            row_idx++;
        }
                                      
     }
         
     return  has_value;
}



static void kbs_irq_handler(XINC_KBS_Type * p_kbs, kbs_control_block_t * p_cb)
{
	 uint32_t reg_kbs_int = 0;//
	 uint32_t reg_kbs_int_prs = 0;
	 uint32_t reg_kbs_int_en = 0;//

     bool has_key_presss = false;

     XINC_KBS_Type * p_reg = xinc_kbs_decode();

	 reg_kbs_int = p_reg->MTXKEY_INT;

	 reg_kbs_int_prs = reg_kbs_int & KBS_MTXKEY_INT_PRS_INT_Msk;//дݼ

  //   printf("int_prs:0x%x\r\n",reg_kbs_int_prs);
	 if(reg_kbs_int_prs)
	 {

         reg_kbs_int_en = p_reg->MTXKEY_INT_EN;
		 reg_kbs_int_en &= ~(KBS_MTXKEY_INT_PRS_INT_EN_Msk);
         p_reg->MTXKEY_INT_EN = reg_kbs_int_en;
                       
         has_key_presss = kbs_mtxkey_value_check_funB();
         
         if(has_key_presss == false)
         {
             if(m_cb.kbs_state == KBS_RELEASE_DETECTED)
             {
                m_cb.kbs_state = KBS_RELEASED;
             }
             else
             {
                m_cb.kbs_state = KBS_RELEASE_DETECTED;
             }
         }else
        {   
            m_cb.kbs_state = KBS_ACTIVE;
        }         
	 }
     

    if(m_cb.kbs_state == KBS_RELEASED)
    {
         p_reg->MTXKEY_MANUAL_ROWOUT = KBS_MTXKEY_MANUAL_ROWOUT_MANUAL_RLS_Msk;

         xinc_delay_us(80); 
    }
    
    reg_kbs_int_prs |= KBS_MTXKEY_INT_PRS_INT_Msk;
    p_reg->MTXKEY_INT = reg_kbs_int_prs;

    reg_kbs_int_en = KBS_MTXKEY_INT_PRS_INT_EN_Msk;
    p_reg->MTXKEY_INT_EN = reg_kbs_int_en;

    if(m_cb.kbs_state == KBS_RELEASED)
    {
        p_reg->MTXKEY_MANUAL_ROWOUT = 0;   
        m_cb.kbs_state = KBS_IDLE;
    }

}

void KBS_Handler(void)
{
    kbs_irq_handler(XINC_KBS,&m_cb);
}



/*lint -restore*/
#endif // XINCX_CHECK(XINCX_KBS_ENABLED)
