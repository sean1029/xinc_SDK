#include    <stdio.h>
#include    <string.h>
#include	"Includes.h"



#include "xinc_gpio.h"

#include "xincx_gpio.h"
#include "AT24C02.h"


#include "xinc_drv_timer.h"

#include "app_button.h"
#include "app_error.h"
#include "app_timer.h"
#include "bsp.h"
#include "app_scheduler.h"
#include "xinc_drv_i2s.h"
#include "xinc_delay.h"




extern uint32_t  GulSystickCount;
uint32_t  LastTimeGulSystickCount=0xFF;


void key_init(void)
{
	Init_gpio();

}


/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press/release).
 */
#define S1BUTTON_BUTTON_PIN            BSP_BUTTON_0   
#define S2BUTTON_BUTTON_PIN            BSP_BUTTON_1   

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(10) 


static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;
    uint8_t rxbuff[AT24Cxx_PAGESIZE];
    uint8_t txbuff[AT24Cxx_PAGESIZE];
    uint16_t addr;
    switch (pin_no)
    {
        case S1BUTTON_BUTTON_PIN:
        {
            if(button_action == APP_BUTTON_RELEASE)
            {
                addr = 0x00;
                txbuff[0] = 0x55;
                printf("write one Byte:0x%x at addr :0x%x\r\n",addr,txbuff[0]);
                AT24Cxx_write_byte(addr,txbuff[0]);
                AT24Cxx_read_buf(0x00,rxbuff,1);
                printf("read one Byte data at addr:0x%x,data:0x%x\r\n",addr,rxbuff[0]);
            }

            
        }break;
          
        case S2BUTTON_BUTTON_PIN:
        {
            if(button_action == APP_BUTTON_RELEASE)
            {              
                addr = 0x00;

                for(int i = 0 ;i < AT24Cxx_PAGESIZE;i++)
                {
                    txbuff[i] = 0x1 + i;
                }
                
                
                AT24Cxx_write_page(1,txbuff,AT24Cxx_PAGESIZE);
                printf("write one page data at addr :0x%x\r\n",addr);
                AT24Cxx_read_buf(0x00,rxbuff,AT24Cxx_PAGESIZE);
                printf("read page data at addr:0x%x,data \r\n",addr);
                
                for(int i = 0 ;i < AT24Cxx_PAGESIZE;i++)
                {
                    printf(":0x%x",rxbuff[i]);
                }
                printf("\r\n");
            }
        
        }break;

            
        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}


static void buttons_init(void)
{
    ret_code_t err_code;

    //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {S1BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},
        {S2BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},
    };

    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);
}


/** @brief Function configuring gpio for pin toggling.
 */
static void buton_config(void)
{
    buttons_init();
   
}

static const xinc_drv_i2s_t m_i2s = XINC_DRV_I2S_INSTANCE(0);



#define LED_OK      BSP_BOARD_LED_0
#define LED_ERROR   BSP_BOARD_LED_1

#define I2S_DATA_BLOCK_WORDS    512
static uint32_t m_buffer_rx[2][I2S_DATA_BLOCK_WORDS];
static uint32_t m_buffer_tx[2][I2S_DATA_BLOCK_WORDS];

// Delay time between consecutive I2S transfers performed in the main loop
// (in milliseconds).
#define PAUSE_TIME          500
// Number of blocks of data to be contained in each transfer.
#define BLOCKS_TO_TRANSFER  20

static uint8_t volatile m_blocks_transferred     = 0;
static uint8_t          m_zero_samples_to_ignore = 0;
static uint16_t         m_sample_value_to_send;
static uint16_t         m_sample_value_expected;
static bool             m_error_encountered;

static uint32_t       * volatile mp_block_to_fill  = NULL;
static uint32_t const * volatile mp_block_to_check = NULL;


static void prepare_tx_data(uint32_t * p_block)
{
    // These variables will be both zero only at the very beginning of each
    // transfer, so we use them as the indication that the re-initialization
    // should be performed.
    if (m_blocks_transferred == 0 && m_zero_samples_to_ignore == 0)
    {
        // Number of initial samples (actually pairs of L/R samples) with zero
        // values that should be ignored - see the comment in 'check_samples'.
        m_zero_samples_to_ignore = 2;
        m_sample_value_to_send   = 0xCAFE;
        m_sample_value_expected  = 0xCAFE;
        m_error_encountered      = false;
    }

    // [each data word contains two 16-bit samples]
    uint16_t i;
    for (i = 0; i < I2S_DATA_BLOCK_WORDS; ++i)
    {
        uint16_t sample_l = m_sample_value_to_send - 1;
        uint16_t sample_r = m_sample_value_to_send + 1;
        ++m_sample_value_to_send;

        uint32_t * p_word = &p_block[i];
        ((uint16_t *)p_word)[0] = sample_l;
        ((uint16_t *)p_word)[1] = sample_r;
    }
}


static bool check_samples(uint32_t const * p_block)
{
    // [each data word contains two 16-bit samples]
    uint16_t i;
    for (i = 0; i < I2S_DATA_BLOCK_WORDS; ++i)
    {
        uint32_t const * p_word = &p_block[i];
        uint16_t actual_sample_l = ((uint16_t const *)p_word)[0];
        uint16_t actual_sample_r = ((uint16_t const *)p_word)[1];

        // Normally a couple of initial samples sent by the I2S peripheral
        // will have zero values, because it starts to output the clock
        // before the actual data is fetched by EasyDMA. As we are dealing
        // with streaming the initial zero samples can be simply ignored.
        if (m_zero_samples_to_ignore > 0 &&
            actual_sample_l == 0 &&
            actual_sample_r == 0)
        {
            --m_zero_samples_to_ignore;
        }
        else
        {
            m_zero_samples_to_ignore = 0;

            uint16_t expected_sample_l = m_sample_value_expected - 1;
            uint16_t expected_sample_r = m_sample_value_expected + 1;
            ++m_sample_value_expected;

            if (actual_sample_l != expected_sample_l ||
                actual_sample_r != expected_sample_r)
            {
                printf("%3u: %04x/%04x, expected: %04x/%04x (i: %u)\r\n",
                    m_blocks_transferred, actual_sample_l, actual_sample_r,
                    expected_sample_l, expected_sample_r, i);
                return false;
            }
        }
    }
    printf("%3u: OK\r\n", m_blocks_transferred);
    return true;
}


static void check_rx_data(uint32_t const * p_block)
{
    ++m_blocks_transferred;

    if (!m_error_encountered)
    {
        m_error_encountered = !check_samples(p_block);
    }

    if (m_error_encountered)
    {
        bsp_board_led_off(LED_OK);
        bsp_board_led_invert(LED_ERROR);
    }
    else
    {
        bsp_board_led_off(LED_ERROR);
        bsp_board_led_invert(LED_OK);
    }
}

static void data_handler(xinc_drv_i2s_data_handler_t const * p_event,
                         void *                     p_context)
{
    // 'nrf_drv_i2s_next_buffers_set' is called directly from the handler
    // each time next buffers are requested, so data corruption is not
    // expected.
    ASSERT(p_event->p_released);

    // When the handler is called after the transfer has been stopped
    // (no next buffers are needed, only the used buffers are to be
    // released), there is nothing to do.
    if (!(p_event->status & XINC_DRV_I2S_STATUS_NEXT_BUFFERS_NEEDED))
    {
        return;
    }

    // First call of this handler occurs right after the transfer is started.
    // No data has been transferred yet at this point, so there is nothing to
    // check. Only the buffers for the next part of the transfer should be
    // provided.
    if (!p_event->p_released->p_rx_buffer)
    {
        xinc_drv_i2s_buffers_t const next_buffers = {
            .p_rx_buffer = m_buffer_rx[1],
            .p_tx_buffer = m_buffer_tx[1],
        };
        APP_ERROR_CHECK(xinc_drv_i2s_next_buffers_set(&m_i2s,&next_buffers));

        mp_block_to_fill = m_buffer_tx[1];
    }
    else
    {
        mp_block_to_check = p_event->p_released->p_rx_buffer;
        // The driver has just finished accessing the buffers pointed by
        // 'p_released'. They can be used for the next part of the transfer
        // that will be scheduled now.
        APP_ERROR_CHECK(xinc_drv_i2s_next_buffers_set(&m_i2s,p_event->p_released));

        // The pointer needs to be typecasted here, so that it is possible to
        // modify the content it is pointing to (it is marked in the structure
        // as pointing to constant data because the driver is not supposed to
        // modify the provided data).
        mp_block_to_fill = (uint32_t *)p_event->p_released->p_tx_buffer;
    }
}

static void I2S_init(void)
{
    uint32_t err_code = XINC_SUCCESS;
    
    bsp_board_init(BSP_INIT_LEDS);
    
    xinc_drv_i2s_config_t config = XINC_DRV_I2S_DEFAULT_CONFIG;
    // In Master mode the MCK frequency and the MCK/LRCK ratio should be
    // set properly in order to achieve desired audio sample rate (which
    // is equivalent to the LRCK frequency).
    // For the following settings we'll get the LRCK frequency equal to
    // 15873 Hz (the closest one to 16 kHz that is possible to achieve).

    err_code = xinc_drv_i2s_init(&m_i2s,&config, data_handler,NULL);
    
    for (;;)
    {
        m_blocks_transferred = 0;
        mp_block_to_fill  = NULL;
        mp_block_to_check = NULL;

        prepare_tx_data(m_buffer_tx[0]);

        xinc_drv_i2s_buffers_t const initial_buffers = {
            .p_tx_buffer = m_buffer_tx[0],
            .p_rx_buffer = m_buffer_rx[0],
        };
        err_code = xinc_drv_i2s_start(&m_i2s,&initial_buffers, I2S_DATA_BLOCK_WORDS, 0);
        APP_ERROR_CHECK(err_code);

        do {

            __WFI();

            if (mp_block_to_fill)
            {
                prepare_tx_data(mp_block_to_fill);
                mp_block_to_fill = NULL;
            }
            if (mp_block_to_check)
            {
                check_rx_data(mp_block_to_check);
                mp_block_to_check = NULL;
            }
        } while (m_blocks_transferred < BLOCKS_TO_TRANSFER);

        xinc_drv_i2s_stop(&m_i2s);



        bsp_board_leds_off();
        xinc_delay_ms(PAUSE_TIME);
    }
    
}
#define SCHED_MAX_EVENT_DATA_SIZE           APP_TIMER_SCHED_EVENT_DATA_SIZE             //!< Maximum size of the scheduler event data.
#define SCHED_QUEUE_SIZE                    10                                          //!< Size of the scheduler queue.

static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

     
int	main(void)
{

    key_init();
    xincx_gpio_init();
    scheduler_init();
    app_timer_init();
    

    SysTick_Config(32000000/100);
//    SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;

    
    buton_config();
    I2S_init();

    while(1) {

       app_sched_execute();

       if(LastTimeGulSystickCount!=GulSystickCount)//10msִ��һ��
	   {		   

		   LastTimeGulSystickCount=GulSystickCount;
           if(LastTimeGulSystickCount % 200 == 0)
           {
               printf("LastTimeGulSystickCount:%d\n",LastTimeGulSystickCount/200);            
           } 
			 
	   }		   


    }
}



