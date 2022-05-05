
#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H


// <<< Use Configuration Wizard in Context Menu >>>\n

// <h> Xinc_BLE 

//==========================================================
// <e> XINC_BLE_STACK_ENABLED  - ble_stack - enable ble module
 

#ifndef XINC_BLE_STACK_ENABLED
#define XINC_BLE_STACK_ENABLED 0
#endif

// <q> XINC_BLE_ADVERTISING_ENABLED  - ble_advertising - Advertising module

#ifndef XINC_BLE_ADVERTISING_ENABLED
#define XINC_BLE_ADVERTISING_ENABLED 1
#endif

// </e>
// </h>

// <h> Xinc_Drivers 

// <e> XINCX_DMAS_ENABLED - xincx_dmas - DMAS peripheral driver
//==========================================================
#ifndef XINCX_DMAS_ENABLED
#define XINCX_DMAS_ENABLED 1
#endif


// <o> XINCX_DMAS_CONFIG_IRQ_PRIORITY  - Interrupt priority
 
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef XINCX_DMAS_CONFIG_IRQ_PRIORITY
#define XINCX_DMAS_CONFIG_IRQ_PRIORITY 7
#endif
// </e> 

// <e> XINCX_WDT_ENABLED - xincx_wdt - WDT peripheral driver
//==========================================================
#ifndef XINCX_WDT_ENABLED
#define XINCX_WDT_ENABLED 1
#endif

// <q> XINCX_WDT0_ENABLED  - Enable WDT0 instance
 
#ifndef XINCX_WDT0_ENABLED
#define XINCX_WDT0_ENABLED 1
#endif

// <o> XINCX_WDT_CONFIG_RELOAD_VALUE - Reload value

// <0=> 0xFFFF 
// <1=> 0x1FFFF 
// <2=> 0x3FFFF 
// <3=> 0x7FFFF
// <4=> 0xFFFFF
// <5=> 0x1FFFFF 
// <6=> 0x3FFFFF 
// <7=> 0x7FFFFF 
// <8=> 0xFFFFFF 
// <9=>  0x1FFFFFF
// <10=> 0x3FFFFFF
// <11=> 0x7FFFFFF
// <12=> 0xFFFFFFF
// <13=> 0x1FFFFFFF
// <14=> 0x3FFFFFFF
// <15=> 0x7FFFFFFF
#ifndef XINCX_WDT_CONFIG_RELOAD_VALUE
#define XINCX_WDT_CONFIG_RELOAD_VALUE 10
#endif

// <o> XINCX_WDT_CONFIG_MODE  -  WDT  run work mode from WDT driver
 
// <0=> WDT_CR_RMOD_Mode0
// <1=> WDT_CR_RMOD_Mode1

#ifndef XINCX_WDT_CONFIG_MODE
#define XINCX_WDT_CONFIG_MODE 1
#endif

// <o> XINCX_WDT_CONFIG_IRQ_PRIORITY  - Interrupt priority
 
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef XINCX_WDT_CONFIG_IRQ_PRIORITY
#define XINCX_WDT_CONFIG_IRQ_PRIORITY 0
#endif

// <e> XINCX_WDT_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef XINCX_WDT_CONFIG_LOG_ENABLED
#define XINCX_WDT_CONFIG_LOG_ENABLED 1
#endif
// <o> XINCX_WDT_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef XINCX_WDT_CONFIG_LOG_LEVEL
#define XINCX_WDT_CONFIG_LOG_LEVEL 3
#endif

// <o> XINCX_WDT_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_WDT_CONFIG_INFO_COLOR
#define XINCX_WDT_CONFIG_INFO_COLOR 0
#endif

// <o> XINCX_WDT_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_WDT_CONFIG_DEBUG_COLOR
#define XINCX_WDT_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <e> XINCX_TIMER_ENABLED - xincx_timer - TIMER periperal driver
//==========================================================
#ifndef XINCX_TIMER_ENABLED
#define XINCX_TIMER_ENABLED 1
#endif

// <q> XINCX_TIMER0_ENABLED  - Enable TIMER0 instance

 #ifndef XINCX_TIMER0_ENABLED
#define XINCX_TIMER0_ENABLED 1
#endif
 
 
// <q> XINCX_TIMER1_ENABLED  - Enable TIMER1 instance

#ifndef XINCX_TIMER1_ENABLED
#define XINCX_TIMER1_ENABLED 1
#endif

// <q> XINCX_TIMER2_ENABLED  - Enable TIMER2 instance
 
#ifndef XINCX_TIMER2_ENABLED
#define XINCX_TIMER2_ENABLED 1
#endif

// <q> XINCX_TIMER3_ENABLED  - Enable TIMER3 instance
 

#ifndef XINCX_TIMER3_ENABLED
#define XINCX_TIMER3_ENABLED 1
#endif



// <o> XINCX_TIMER_DEFAULT_CONFIG_FREQUENCY  - Timer frequency if in Timer mode
 
// <0=> (16 MHz @ 32M clk div) |(16KHz @ 32K clk div)
// <1=> 8 MHz (8 MHz @ 32M clk div) |(8KHz @ 32K clk div)
// <2=> 4 MHz (4 MHz @ 32M clk div) |(4KHz @ 32K clk div)
// <3=> 2 MHz (2 MHz @ 32M clk div) |(2KHz @ 32K clk div)
// <4=> 1 MHz (1 MHz @ 32M clk div) |(1KHz @ 32K clk div)
// <5=> 500 kHz (500 kHz @ 32M clk div) | (500Hz @ 32K clk div)
// <6=> 250 kHz (250 kHz @ 32M clk div) | (250Hz @ 32K clk div)
// <7=> 125 kHz (125 kHz @ 32M clk div) | (125Hz @ 32K clk div)
// <8=> 62.5 kHz (62.5 kHz @ 32M clk div) | (62.5Hz @ 32K clk div)
// <9=> 32KHz
#ifndef XINCX_TIMER_DEFAULT_CONFIG_FREQUENCY
#define XINCX_TIMER_DEFAULT_CONFIG_FREQUENCY 4
#endif

// <o> XINCX_TIMER_DEFAULT_CONFIG_MODE  - Timer mode or operation
 
// <0=> Timer 
// <1=> Counter 

#ifndef XINCX_TIMER_DEFAULT_CONFIG_MODE
#define XINCX_TIMER_DEFAULT_CONFIG_MODE 1
#endif

// <o> XINCX_TIMER_DEFAULT_CONFIG_CLK_SRC  - Timer CLK_SRC 
 
// <0=> 32Mhz clk src div
// <1=> 32K clk src div
// <4=> 32K clk src 

#ifndef XINCX_TIMER_DEFAULT_CONFIG_CLK_SRC
#define XINCX_TIMER_DEFAULT_CONFIG_CLK_SRC 1
#endif

// <o> XINCX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority
 
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef XINCX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY
#define XINCX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY 6
#endif

// <e> XINCX_TIMER_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef XINCX_TIMER_CONFIG_LOG_ENABLED
#define XINCX_TIMER_CONFIG_LOG_ENABLED 0
#endif
// <o> XINCX_TIMER_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 
#ifndef XINCX_TIMER_CONFIG_LOG_LEVEL
#define XINCX_TIMER_CONFIG_LOG_LEVEL 3
#endif

// <o> XINCX_TIMER_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_TIMER_CONFIG_INFO_COLOR
#define XINCX_TIMER_CONFIG_INFO_COLOR 0
#endif

// <o> XINCX_TIMER_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_TIMER_CONFIG_DEBUG_COLOR
#define XINCX_TIMER_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <e> XINCX_PWM_ENABLED - xincx_pwm - PWM peripheral driver
//==========================================================
#ifndef XINCX_PWM_ENABLED
#define XINCX_PWM_ENABLED 1
#endif

// <q> XINCX_PWM0_ENABLED  - Enable PWM0 instance
 
#ifndef XINCX_PWM0_ENABLED
#define XINCX_PWM0_ENABLED 1
#endif

// <q> XINCX_PWM1_ENABLED  - Enable PWM1 instance
 

#ifndef XINCX_PWM1_ENABLED
#define XINCX_PWM1_ENABLED 1
#endif

// <q> XINCX_PWM2_ENABLED  - Enable PWM2 instance
 

#ifndef XINCX_PWM2_ENABLED
#define XINCX_PWM2_ENABLED 1
#endif

// <q> XINCX_PWM3_ENABLED  - Enable PWM3 instance
 

#ifndef XINCX_PWM3_ENABLED
#define XINCX_PWM3_ENABLED 1
#endif

// <q> XINCX_PWM4_ENABLED  - Enable PWM4 instance
 

#ifndef XINCX_PWM4_ENABLED
#define XINCX_PWM4_ENABLED 1
#endif

// <q> XINCX_PWM5_ENABLED  - Enable PWM5 instance
 

#ifndef XINCX_PWM5_ENABLED
#define XINCX_PWM5_ENABLED 1
#endif

// <o> XINCX_PWM_DEFAULT_CONFIG_OUT0_PIN - Out0 pin  <0-31> 


#ifndef XINCX_PWM_DEFAULT_CONFIG_OUT0_PIN
#define XINCX_PWM_DEFAULT_CONFIG_OUT0_PIN 0
#endif

// <o> XINCX_PWM_DEFAULT_CONFIG_OUT0_INV_PIN - Out0 pin  <0-31> 

#ifndef XINCX_PWM_DEFAULT_CONFIG_OUT0_INV_PIN
#define XINCX_PWM_DEFAULT_CONFIG_OUT0_INV_PIN 1
#endif

// <o> XINCX_PWM_DEFAULT_CONFIG_OUT1_PIN - Out1 pin  <0-31> 


#ifndef XINCX_PWM_DEFAULT_CONFIG_OUT1_PIN
#define XINCX_PWM_DEFAULT_CONFIG_OUT1_PIN 2
#endif

// <o> XINCX_PWM_DEFAULT_CONFIG_OUT1_INV_PIN - Out1 pin  <0-31> 

#ifndef XINCX_PWM_DEFAULT_CONFIG_OUT1_INV_PIN
#define XINCX_PWM_DEFAULT_CONFIG_OUT1_INV_PIN 3
#endif


// <o> XINCX_PWM_DEFAULT_CONFIG_OUT2_PIN - Out2 pin  <0-31> 


#ifndef XINCX_PWM_DEFAULT_CONFIG_OUT2_PIN
#define XINCX_PWM_DEFAULT_CONFIG_OUT2_PIN 0
#endif
// <o> XINCX_PWM_DEFAULT_CONFIG_OUT2_INV_PIN - Out2 pin

#ifndef XINCX_PWM_DEFAULT_CONFIG_OUT2_INV_PIN
#define XINCX_PWM_DEFAULT_CONFIG_OUT2_INV_PIN 0xFF
#endif


// <o> XINCX_PWM_DEFAULT_CONFIG_OUT3_PIN - Out3 pin   


#ifndef XINCX_PWM_DEFAULT_CONFIG_OUT3_PIN
#define XINCX_PWM_DEFAULT_CONFIG_OUT3_PIN 1
#endif
// <o> XINCX_PWM_DEFAULT_CONFIG_OUT3_INV_PIN - Out3 pin

#ifndef XINCX_PWM_DEFAULT_CONFIG_OUT3_INV_PIN
#define XINCX_PWM_DEFAULT_CONFIG_OUT3_INV_PIN 0xFF
#endif

// <o> XINCX_PWM_DEFAULT_CONFIG_OUT4_PIN - Out4 pin 


#ifndef XINCX_PWM_DEFAULT_CONFIG_OUT4_PIN
#define XINCX_PWM_DEFAULT_CONFIG_OUT4_PIN 12
#endif

// <o> XINCX_PWM_DEFAULT_CONFIG_OUT4_INV_PIN - Out4 pin

#ifndef XINCX_PWM_DEFAULT_CONFIG_OUT4_INV_PIN
#define XINCX_PWM_DEFAULT_CONFIG_OUT4_INV_PIN 0xFF
#endif

// <o> XINCX_PWM_DEFAULT_CONFIG_OUT5_PIN - Out5 pin  <0-31>  


#ifndef XINCX_PWM_DEFAULT_CONFIG_OUT5_PIN
#define XINCX_PWM_DEFAULT_CONFIG_OUT5_PIN 13
#endif
// <o> XINCX_PWM_DEFAULT_CONFIG_OUT5_INV_PIN - Out5 pin

#ifndef XINCX_PWM_DEFAULT_CONFIG_OUT5_INV_PIN
#define XINCX_PWM_DEFAULT_CONFIG_OUT5_INV_PIN 0xFF
#endif

// <o> XINCX_TIMER_DEFAULT_CONFIG_CLK_SRC  - Timer CLK_SRC 
 
// <0=> 32Mhz clk src div
// <1=> 32K clk src div
// <4=> 32K clk src 

#ifndef XINCX_PWM_DEFAULT_CONFIG_CLK_SRC
#define XINCX_PWM_DEFAULT_CONFIG_CLK_SRC 4
#endif

// <o> XINCX_PWM_DEFAULT_CONFIG_BASE_CLOCK  - Base clock
 
// <0=> (16 MHz @ 32M clk div) |(16KHz @ 32K clk div)
// <1=> 8 MHz (8 MHz @ 32M clk div) |(8KHz @ 32K clk div)
// <2=> 4 MHz (4 MHz @ 32M clk div) |(4KHz @ 32K clk div)
// <3=> 2 MHz (2 MHz @ 32M clk div) |(2KHz @ 32K clk div)
// <4=> 1 MHz (1 MHz @ 32M clk div) |(1KHz @ 32K clk div)
// <5=> 500 kHz (500 kHz @ 32M clk div) | (500Hz @ 32K clk div)
// <6=> 250 kHz (250 kHz @ 32M clk div) | (250Hz @ 32K clk div)
// <7=> 125 kHz (125 kHz @ 32M clk div) | (125Hz @ 32K clk div)
// <8=> 62.5 kHz (62.5 kHz @ 32M clk div) | (62.5Hz @ 32K clk div)
// <9=> 32KHz

#ifndef XINCX_PWM_DEFAULT_CONFIG_REF_CLOCK
#define XINCX_PWM_DEFAULT_CONFIG_REF_CLOCK 4
#endif

// <o> XINCX_PWM_DEFAULT_CONFIG_FREQUENCY  - frequency Hz <1-100000>Hz
 
#ifndef XINCX_PWM_DEFAULT_CONFIG_FREQUENCY
#define XINCX_PWM_DEFAULT_CONFIG_FREQUENCY 4
#endif

// <o> XINCX_PWM_DEFAULT_CONFIG_COUNT_MODE  - Count mode
 
// <0=> Up 
// <1=> Up and Down 

#ifndef XINCX_PWM_DEFAULT_CONFIG_COUNT_MODE
#define XINCX_PWM_DEFAULT_CONFIG_COUNT_MODE 0
#endif

// <o> XINCX_PWM_DEFAULT_CONFIG_TOP_VALUE - Top value 
#ifndef XINCX_PWM_DEFAULT_CONFIG_TOP_VALUE
#define XINCX_PWM_DEFAULT_CONFIG_TOP_VALUE 1000
#endif


// <o> XINCX_PWM_DEFAULT_CONFIG_LOAD_MODE  - Load mode
 
// <0=> Common 
// <1=> Grouped 
// <2=> Individual 
// <3=> Waveform 

#ifndef XINCX_PWM_DEFAULT_CONFIG_LOAD_MODE
#define XINCX_PWM_DEFAULT_CONFIG_LOAD_MODE 0
#endif


// <o> XINCX_PWM_DEFAULT_CONFIG_STEP_MODE  - Step mode
 
// <0=> Auto 
// <1=> Triggered 

#ifndef XINCX_PWM_DEFAULT_CONFIG_STEP_MODE
#define XINCX_PWM_DEFAULT_CONFIG_STEP_MODE 0
#endif

// <o> XINCX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority
 
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef XINCX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY
#define XINCX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY 6
#endif

// <e> XINCX_PWM_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef XINCX_PWM_CONFIG_LOG_ENABLED
#define XINCX_PWM_CONFIG_LOG_ENABLED 0
#endif
// <o> XINCX_PWM_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef XINCX_PWM_CONFIG_LOG_LEVEL
#define XINCX_PWM_CONFIG_LOG_LEVEL 3
#endif

// <o> XINCX_PWM_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_PWM_CONFIG_INFO_COLOR
#define XINCX_PWM_CONFIG_INFO_COLOR 0
#endif

// <o> XINCX_PWM_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_PWM_CONFIG_DEBUG_COLOR
#define XINCX_PWM_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>


// <e> XINCX_I2C_ENABLED - xincx_i2c - I2C peripheral driver
//==========================================================
#ifndef XINCX_I2C_ENABLED
#define XINCX_I2C_ENABLED 1
#endif

// <q> XINCX_I2C0_ENABLED  - Enable I2C0 instance
#ifndef XINCX_I2C0_ENABLED
#define XINCX_I2C0_ENABLED 1
#endif

// <o> XINCX_I2C_DEFAULT_CONFIG_FREQUENCY  - Frequency
 
// <0x0063484FUL=> 100k 
// <0x00650C13UL=> 400k 

#ifndef XINCX_I2C_DEFAULT_CONFIG_FREQUENCY
#define XINCX_I2C_DEFAULT_CONFIG_FREQUENCY 6506575
#endif

// <q> XINCX_I2C_DEFAULT_CONFIG_HOLD_BUS_UNINIT  - Enables bus holding after uninit
 

#ifndef XINCX_I2C_DEFAULT_CONFIG_HOLD_BUS_UNINIT
#define XINCX_I2C_DEFAULT_CONFIG_HOLD_BUS_UNINIT 1
#endif

// <o> XINCX_I2C_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority
 
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef XINCX_I2C_DEFAULT_CONFIG_IRQ_PRIORITY
#define XINCX_I2C_DEFAULT_CONFIG_IRQ_PRIORITY 6
#endif

// <e> XINCX_I2C_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef XINCX_I2C_CONFIG_LOG_ENABLED
#define XINCX_I2C_CONFIG_LOG_ENABLED 1
#endif
// <o> XINCX_I2C_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef XINCX_I2C_CONFIG_LOG_LEVEL
#define XINCX_I2C_CONFIG_LOG_LEVEL 2
#endif

// <o> XINCX_I2C_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_I2C_CONFIG_INFO_COLOR
#define XINCX_I2C_CONFIG_INFO_COLOR 0
#endif

// <o> XINCX_I2C_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_I2C_CONFIG_DEBUG_COLOR
#define XINCX_I2C_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <e> XINCX_I2S_ENABLED - xincx_i2s - I2S peripheral driver
//==========================================================
#ifndef XINCX_I2S_ENABLED
#define XINCX_I2S_ENABLED 1
#endif


// <q> XINCX_I2SM0_ENABLED  - Enable I2SM0 instance
#ifndef XINCX_I2SM0_ENABLED
#define XINCX_I2SM0_ENABLED 1
#endif


// <o> XINCX_I2S_CONFIG_SCK_PIN - SCK pin  <0-31> 


#ifndef XINCX_I2S_CONFIG_SCK_PIN
#define XINCX_I2S_CONFIG_SCK_PIN 31
#endif

// <o> XINCX_I2S_CONFIG_LRCK_PIN - LRCK pin  <1-31> 


#ifndef XINCX_I2S_CONFIG_LRCK_PIN
#define XINCX_I2S_CONFIG_LRCK_PIN 30
#endif

// <o> XINCX_I2S_CONFIG_MCK_PIN - MCK pin 
#ifndef XINCX_I2S_CONFIG_MCK_PIN
#define XINCX_I2S_CONFIG_MCK_PIN 255
#endif

// <o> XINCX_I2S_CONFIG_SDOUT_PIN - SDOUT pin  <0-31> 


#ifndef XINCX_I2S_CONFIG_SDOUT_PIN
#define XINCX_I2S_CONFIG_SDOUT_PIN 29
#endif

// <o> XINCX_I2S_CONFIG_SDIN_PIN - SDIN pin  <0-31> 


#ifndef XINCX_I2S_CONFIG_SDIN_PIN
#define XINCX_I2S_CONFIG_SDIN_PIN 28
#endif

// <o> XINCX_I2S_CONFIG_MASTER  - Mode
 
// <0=> Master 
// <1=> Slave 

#ifndef XINCX_I2S_CONFIG_MASTER
#define XINCX_I2S_CONFIG_MASTER 0
#endif

// <o> XINCX_I2S_CONFIG_FORMAT  - Format
 
// <0=> I2S 
// <1=> Aligned 

#ifndef XINCX_I2S_CONFIG_FORMAT
#define XINCX_I2S_CONFIG_FORMAT 0
#endif

// <o> XINCX_I2S_CONFIG_ALIGN  - Alignment
 
// <0=> Left 
// <1=> Right 

#ifndef XINCX_I2S_CONFIG_ALIGN
#define XINCX_I2S_CONFIG_ALIGN 0
#endif

// <o> XINCX_I2S_CONFIG_SLOT_WIDTH  - Sample width (bits)
 
// <1=> 8 
// <2=> 12 
// <3=> 16
// <4=> 20 
// <5=> 24 
// <6=> 28 
// <7=> 32 

#ifndef XINCX_I2S_CONFIG_SLOT_WIDTH
#define XINCX_I2S_CONFIG_SLOT_WIDTH 3
#endif

// <o> XINCX_I2S_CONFIG_CHANNELS  - Channels
 
// <0=> Stereo 
// <1=> Left 
// <2=> Right 

#ifndef XINCX_I2S_CONFIG_CHANNELS
#define XINCX_I2S_CONFIG_CHANNELS 0
#endif

// <o> XINCX_I2S_CONFIG_MCK_SETUP  - MCK behavior
 
// <0=> 16MHz 
// <1=> 16MHz/2 
// <2=> 16MHz/3 
// <3=> 16MHz/4 
// <4=> 16MHz/5 
// <5=> 16MHz/6 
// <7=> 16MHz/8 
// <9=> 16MHz/10 
// <10=> 16MHz/11 
// <14=> 16MHz/15 
// <15=> 16MHz/16 
// <20=> 16MHz/21 
// <22=> 16MHz/23 
// <29=> 16MHz/30 
// <30=> 16MHz/31 
// <31=> 16MHz/32 
// <41=> 16MHz/42 
// <63=> 16MHz/63 
// <124=> 16MHz/125 

#ifndef XINCX_I2S_CONFIG_MCK_SETUP
#define XINCX_I2S_CONFIG_MCK_SETUP 124
#endif

// <o> XINCX_I2S_CONFIG_RATIO  - MCK/LRCK ratio
 
// <0=> 32x 
// <1=> 48x 
// <2=> 64x 
// <3=> 96x 
// <4=> 128x 
// <5=> 192x 
// <6=> 256x 
// <7=> 384x 
// <8=> 512x 

#ifndef XINCX_I2S_CONFIG_RATIO
#define XINCX_I2S_CONFIG_RATIO 0
#endif

// <o> XINCX_I2S_CONFIG_IRQ_PRIORITY  - Interrupt priority
 
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef XINCX_I2S_CONFIG_IRQ_PRIORITY
#define XINCX_I2S_CONFIG_IRQ_PRIORITY 6
#endif

// <e> XINCX_I2S_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef XINCX_I2S_CONFIG_LOG_ENABLED
#define XINCX_I2S_CONFIG_LOG_ENABLED 0
#endif
// <o> XINCX_I2S_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef XINCX_I2S_CONFIG_LOG_LEVEL
#define XINCX_I2S_CONFIG_LOG_LEVEL 3
#endif

// <o> XINCX_I2S_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_I2S_CONFIG_INFO_COLOR
#define XINCX_I2S_CONFIG_INFO_COLOR 0
#endif

// <o> XINCX_I2S_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_I2S_CONFIG_DEBUG_COLOR
#define XINCX_I2S_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <e> XINCX_SPIM_ENABLED - xincx_spim - SPIM peripheral driver
//==========================================================
#ifndef XINCX_SPIM_ENABLED
#define XINCX_SPIM_ENABLED 1
#endif

// <q> XINCX_SPIM0_ENABLED  - Enable SPIM0 instance
 

#ifndef XINCX_SPIM0_ENABLED
#define XINCX_SPIM0_ENABLED 1
#endif

// <q> XINCX_SPIM1_ENABLED  - Enable SPIM1 instance
 

#ifndef XINCX_SPIM1_ENABLED
#define XINCX_SPIM1_ENABLED 1
#endif

// <q> XINCX_SPIM2_ENABLED  - Enable SPIM2 instance
 

#ifndef XINCX_SPIM2_ENABLED
#define XINCX_SPIM2_ENABLED 0
#endif

// <o> XINCX_SPIM_MISO_PULL_CFG  - MISO pin pull configuration.
 
// <0=> XINC_GPIO_PIN_NOPULL 
// <1=> XINC_GPIO_PIN_PULLDOWN 
// <3=> XINC_GPIO_PIN_PULLUP 

#ifndef XINCX_SPIM_MISO_PULL_CFG
#define XINCX_SPIM_MISO_PULL_CFG 1
#endif

// </e>

// <e> XINCX_GPIO_ENABLED - xincx_gpio - GPIOTE peripheral driver
//==========================================================
#ifndef XINCX_GPIO_ENABLED
#define XINCX_GPIO_ENABLED 1
#endif
// <o> XINCX_GPIO_CONFIG_NUM_OF_LOW_POWER_EVENTS - Number of lower power input pins 
#ifndef XINCX_GPIO_CONFIG_NUM_OF_LOW_POWER_EVENTS
#define XINCX_GPIO_CONFIG_NUM_OF_LOW_POWER_EVENTS 1
#endif
// </e>

// <e> XINCX_KBS_ENABLED - xincx_kbs - KBS peripheral driver
//==========================================================
#ifndef XINCX_KBS_ENABLED
#define XINCX_KBS_ENABLED 1
#endif

// <o> XINCX_KBS_CONFIG_PRS_INTVAL - PRS_INTVAL(Unit 0.5ms)  <1-255> 
#ifndef XINCX_KBS_CONFIG_PRS_INTVAL
#define XINCX_KBS_CONFIG_PRS_INTVAL 10
#endif

// <o> XINCX_KBS_CONFIG_RLS_INTVA - RLS_INTVAL(Unit 0.5ms)  <1-255> 
#ifndef XINCX_KBS_CONFIG_RLS_INTVAL
#define XINCX_KBS_CONFIG_RLS_INTVAL 10
#endif

// <o> XINCX_KBS_CONFIG_DBC_INTVAL - DBC_INTVAL(Unit 1ms)  <1-255> 
#ifndef XINCX_KBS_CONFIG_DBC_INTVAL
#define XINCX_KBS_CONFIG_DBC_INTVAL 5
#endif

// <o> XINCX_KBS_CONFIG_RPRS_INTVAL - RPRS_INTVAL(Unit 1ms)  <1-1023> 
#ifndef XINCX_KBS_CONFIG_RPRS_INTVAL
#define XINCX_KBS_CONFIG_RPRS_INTVAL 200
#endif

// <o> XINCX_KBS_CONFIG_LPRS_INTVAL - LPRS_INTVAL(Unit 1ms)  <1-63> 
#ifndef XINCX_KBS_CONFIG_LPRS_INTVAL
#define XINCX_KBS_CONFIG_LPRS_INTVAL 5
#endif

// </e>


// <e> XINCX_RTC_ENABLED - xincx_rtc - RTC peripheral driver
//==========================================================
#ifndef XINCX_RTC_ENABLED
#define XINCX_RTC_ENABLED 1
#endif

// <q> XINCX_RTC0_ENABLED  - Enable RTC0 instance
 
#ifndef XINCX_RTC0_ENABLED
#define XINCX_RTC0_ENABLED 1
#endif

// </e>

// <e> XINCX_SAADC_ENABLED - xinc_saadc - SAADC peripheral driver
//==========================================================
#ifndef XINCX_SAADC_ENABLED
#define XINCX_SAADC_ENABLED 1
#endif
// <o> XINCX_SAADC_CONFIG_RESOLUTION  - Resolution
 
// <0=> 8 bit 
// <1=> 10 bit 
// <2=> 12 bit 
// <3=> 14 bit 

#ifndef XINCX_SAADC_CONFIG_RESOLUTION
#define XINCX_SAADC_CONFIG_RESOLUTION 1
#endif

// <o> XINCX_SAADC_CONFIG_OVERSAMPLE  - Sample period
 
// <0=> Disabled 
// <1=> 2x 
// <2=> 4x 
// <3=> 8x 
// <4=> 16x 
// <5=> 32x 
// <6=> 64x 
// <7=> 128x 
// <8=> 256x 

#ifndef XINCX_SAADC_CONFIG_OVERSAMPLE
#define XINCX_SAADC_CONFIG_OVERSAMPLE 0
#endif

// <o> XINCX_SAADC_CONFIG_FIFO_LEN  - Fifo len
 
// <1=> 1 
// <2=> 2 
// <4=> 3 
// <6=> 4 
// <8=> 8 
// <10=> 10 
// <12=> 12 
// <14=> 14 

#ifndef XINCX_SAADC_CONFIG_FIFO_LEN
#define XINCX_SAADC_CONFIG_FIFO_LEN 8
#endif

// <q> XINCX_SAADC_CONFIG_LP_MODE  - Enabling low power mode
 

#ifndef XINCX_SAADC_CONFIG_LP_MODE
#define XINCX_SAADC_CONFIG_LP_MODE 0
#endif

// <o> XINCX_SAADC_CONFIG_IRQ_PRIORITY  - Interrupt priority
 
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef XINCX_SAADC_CONFIG_IRQ_PRIORITY
#define XINCX_SAADC_CONFIG_IRQ_PRIORITY 6
#endif


// <e> XINCX_SAADC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef XINCX_SAADC_CONFIG_LOG_ENABLED
#define XINCX_SAADC_CONFIG_LOG_ENABLED 0
#endif
// <o> XINCX_SAADC_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 
#ifndef XINCX_SAADC_CONFIG_LOG_LEVEL
#define XINCX_SAADC_CONFIG_LOG_LEVEL 3
#endif


// <o> XINCX_SAADC_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_SAADC_CONFIG_INFO_COLOR
#define XINCX_SAADC_CONFIG_INFO_COLOR 0
#endif

// <o> XINCX_SAADC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_SAADC_CONFIG_DEBUG_COLOR
#define XINCX_SAADC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>


// <e> XINCX_AUDIO_ADC_ENABLED - xinc_audio_adc - AUDIO ADC peripheral driver
//==========================================================
#ifndef XINCX_AUDIO_ADC_ENABLED
#define XINCX_AUDIO_ADC_ENABLED 1
#endif

// <q> XINCX_AUDIO_ADC0_ENABLED  - Enable AUDIO_ADC0 instance
 

#ifndef XINCX_AUDIO_ADC0_ENABLED
#define XINCX_AUDIO_ADC0_ENABLED 1
#endif


// <o> XINCX_AUDIO_ADC_CONFIG_RESOLUTION  - Resolution
 
// <0=> 8 bit 
// <1=> 10 bit 
// <2=> 12 bit 
// <3=> 14 bit 

#ifndef XINCX_AUDIO_ADC_CONFIG_RESOLUTION
#define XINCX_AUDIO_ADC_CONFIG_RESOLUTION 1
#endif

// <o> XINCX_CDC_CONFIG_OVERSAMPLE  - Sample period
 
// <0=> Disabled 
// <1=> 2x 
// <2=> 4x 
// <3=> 8x 
// <4=> 16x 
// <5=> 32x 
// <6=> 64x 
// <7=> 128x 
// <8=> 256x 

#ifndef XINCX_AUDIO_ADC_CONFIG_OVERSAMPLE
#define XINCX_AUDIO_ADC_CONFIG_OVERSAMPLE 0
#endif

// <o> XINCX_AUDIO_ADC_CONFIG_FIFO_LEN  - Fifo len
 
// <1=> 1 
// <2=> 2 
// <4=> 3 
// <6=> 4 
// <8=> 8 
// <10=> 10 
// <12=> 12 
// <14=> 14 

#ifndef XINCX_AUDIO_ADC_CONFIG_FIFO_LEN
#define XINCX_AUDIO_ADC_CONFIG_FIFO_LEN 8
#endif


// <o> XINCX_CDC_CONFIG_IRQ_PRIORITY  - Interrupt priority
 
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef XINCX_AUDIO_ADC_CONFIG_IRQ_PRIORITY
#define XINCX_AUDIO_ADC_CONFIG_IRQ_PRIORITY 6
#endif


// <e> XINCX_AUDIO_ADC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef XINCX_AUDIO_ADC_CONFIG_LOG_ENABLED
#define XINCX_AUDIO_ADC_CONFIG_LOG_ENABLED 0
#endif
// <o> XINCX_CDC_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 
#ifndef XINCX_AUDIO_ADC_CONFIG_LOG_LEVEL
#define XINCX_AUDIO_ADC_CONFIG_LOG_LEVEL 3
#endif


// <o> XINCX_AUDIO_ADC_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_AUDIO_ADC_CONFIG_INFO_COLOR
#define XINCX_AUDIO_ADC_CONFIG_INFO_COLOR 0
#endif

// <o> XINCX_AUDIO_ADC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_AUDIO_ADC_CONFIG_DEBUG_COLOR
#define XINCX_AUDIO_ADC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>


// <e> XINCX_UARTE_ENABLED  - xincx_uarte - UARTE peripheral driver
//==========================================================
#ifndef XINCX_UARTE_ENABLED
#define XINCX_UARTE_ENABLED 1
#endif

// <q> XINCX_UARTE0_ENABLED - Enable UARTE0 instance 

#ifndef XINCX_UARTE0_ENABLED
#define XINCX_UARTE0_ENABLED 1
#endif

// <q> XINCX_UART1_ENABLED - Enable UARTE1 instance 
#ifndef XINCX_UARTE1_ENABLED
#define XINCX_UARTE1_ENABLED 1
#endif


// <o> XINCX_UART_DEFAULT_CONFIG_HWFC  - Hardware Flow Control
 
// <0=> Disabled 
// <1=> Enabled 

#ifndef XINCX_UARTE_DEFAULT_CONFIG_HWFC
#define XINCX_UARTE_DEFAULT_CONFIG_HWFC 0
#endif

// <o> XINCX_UARTE_DEFAULT_CONFIG_PARITY  - Parity
 
// <0=> Excluded 
// <1=> Included 

#ifndef XINCX_UARTE_DEFAULT_CONFIG_PARITY
#define XINCX_UARTE_DEFAULT_CONFIG_PARITY 0
#endif

// <o> XINCX_UARTE_DEFAULT_CONFIG_PARITY_TYPE  - Parity type
// <0=> Included Odd  
// <1=> Included Even 
 
#ifndef XINCX_UARTE_DEFAULT_CONFIG_PARITY_TYPE
#define XINCX_UARTE_DEFAULT_CONFIG_PARITY_TYPE 0
#endif

// <o> XINCX_UARTE_DEFAULT_CONFIG_DATA_BITS  - Data bits
// <0=> 5 bits
// <1=> 6 bits
// <2=> 7 bits
// <3=> 8 bits
 
#ifndef XINCX_UARTE_DEFAULT_CONFIG_DATA_BITS
#define XINCX_UARTE_DEFAULT_CONFIG_DATA_BITS 3
#endif

// <o> XINCX_UART_DEFAULT_CONFIG_STOP_BITS  - Stop bits
// <0=> 1 bits
// <1=> 1.5 / 2 bits
#ifndef XINCX_UARTE_DEFAULT_CONFIG_STOP_BITS
#define XINCX_UARTE_DEFAULT_CONFIG_STOP_BITS 1
#endif





// <o> XINCX_UARTE_DEFAULT_CONFIG_BAUDRATE  - Default Baudrate
 
// <0x0120271C=> 2400 baud 
// <0x0240271C=> 4800 baud 
// <0x0480271C=> 9600 baud 
// <0x04802719=> 12800 baud 
// <0x04802718=> 14400 baud 
// <0x04802716=> 19200 baud 
// <0x04802715=> 23040 baud 
// <0x04802713=> 38400 baud 
// <0x04802712=> 57600 baud 
// <0x04802711=> 115200 baud 
// <0x030007D3=> 1280000 baud 
// <0x0120271C=> 230400 baud 
// <0x060007D3=> 256000 baud 
// <0x12002711=> 460800 baud 
// <0x24002711=> 921600 baud 
// <0x00100011=> 1M baud 

#ifndef XINCX_UARTE_DEFAULT_CONFIG_BAUDRATE
#define XINCX_UARTE_DEFAULT_CONFIG_BAUDRATE 75507473
#endif

// <e> XINCX_UARTE_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef XINCX_UARTE_CONFIG_LOG_ENABLED
#define XINCX_UARTE_CONFIG_LOG_ENABLED 0
#endif
// <o> XINCX_UARTE_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef XINCX_UARTE_CONFIG_LOG_LEVEL
#define XINCX_UARTE_CONFIG_LOG_LEVEL 3
#endif

// <o> XINCX_UART_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_UARTE_CONFIG_INFO_COLOR
#define XINCX_UARTE_CONFIG_INFO_COLOR 0
#endif

// <o> XINCX_UARTE_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_UARTE_CONFIG_DEBUG_COLOR
#define XINCX_UARTE_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>


// <e> XINCX_UART_ENABLED - xincx_uart - UART peripheral driver
//==========================================================
#ifndef XINCX_UART_ENABLED
#define XINCX_UART_ENABLED 1
#endif


// <q> XINCX_UART0_ENABLED - Enable UART0 instance 

#ifndef XINCX_UART0_ENABLED
#define XINCX_UART0_ENABLED 1
#endif

// <q> XINCX_UART1_ENABLED - Enable UART1 instance 
#ifndef XINCX_UART1_ENABLED
#define XINCX_UART1_ENABLED 1
#endif

// <q> XINCX_UART2_ENABLED - Enable UART2 instance 
#ifndef XINCX_UART2_ENABLED
#define XINCX_UART2_ENABLED 0
#endif

// <o> XINCX_UART_DEFAULT_CONFIG_HWFC  - Hardware Flow Control
 
// <0=> Disabled 
// <1=> Enabled 

#ifndef XINCX_UART_DEFAULT_CONFIG_HWFC
#define XINCX_UART_DEFAULT_CONFIG_HWFC 0
#endif

// <o> XINCX_UART_DEFAULT_CONFIG_PARITY  - Parity
 
// <0=> Excluded 
// <1=> Included 

#ifndef XINCX_UART_DEFAULT_CONFIG_PARITY
#define XINCX_UART_DEFAULT_CONFIG_PARITY 0
#endif

// <o> XINCX_UART_DEFAULT_CONFIG_PARITY_TYPE  - Parity type
// <0=> Included Odd  
// <1=> Included Even 
 
#ifndef XINCX_UART_DEFAULT_CONFIG_PARITY_TYPE
#define XINCX_UART_DEFAULT_CONFIG_PARITY_TYPE 0
#endif

// <o> XINCX_UART_DEFAULT_CONFIG_DATA_BITS  - Data bits
// <0=> 5 bits
// <1=> 6 bits
// <2=> 7 bits
// <3=> 8 bits
 
#ifndef XINCX_UART_DEFAULT_CONFIG_DATA_BITS
#define XINCX_UART_DEFAULT_CONFIG_DATA_BITS 3
#endif

// <o> XINCX_UART_DEFAULT_CONFIG_STOP_BITS  - Stop bits
// <0=> 1 bits
// <1=> 1.5 / 2 bits
#ifndef XINCX_UART_DEFAULT_CONFIG_STOP_BITS
#define XINCX_UART_DEFAULT_CONFIG_STOP_BITS 1
#endif



// <o> XINCX_UART_DEFAULT_CONFIG_BAUDRATE  - Default Baudrate
 
// <0x0120271C=> 2400 baud 
// <0x0240271C=> 4800 baud 
// <0x0480271C=> 9600 baud 
// <0x04802719=> 12800 baud 
// <0x04802718=> 14400 baud 
// <0x04802716=> 19200 baud 
// <0x04802715=> 23040 baud 
// <0x04802713=> 38400 baud 
// <0x04802712=> 57600 baud 
// <0x04802711=> 115200 baud 
// <0x030007D3=> 1280000 baud 
// <0x0120271C=> 230400 baud 
// <0x060007D3=> 256000 baud 
// <0x12002711=> 460800 baud 
// <0x24002711=> 921600 baud 
// <0x00100011=> 1M baud 

#ifndef XINCX_UART_DEFAULT_CONFIG_BAUDRATE
#define XINCX_UART_DEFAULT_CONFIG_BAUDRATE 75507473
#endif

// <e> XINCX_UART_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef XINCX_UART_CONFIG_LOG_ENABLED
#define XINCX_UART_CONFIG_LOG_ENABLED 0
#endif
// <o> XINCX_UART_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef XINCX_UART_CONFIG_LOG_LEVEL
#define XINCX_UART_CONFIG_LOG_LEVEL 3
#endif

// <o> XINCX_UART_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_UART_CONFIG_INFO_COLOR
#define XINCX_UART_CONFIG_INFO_COLOR 0
#endif

// <o> XINCX_UART_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINCX_UART_CONFIG_DEBUG_COLOR
#define XINCX_UART_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>


// <e> UART_ENABLED - xinc_drv_uart - UART/UARTE peripheral driver - legacy layer
//==========================================================
#ifndef UART_ENABLED
#define UART_ENABLED 1
#endif
// <o> UART_DEFAULT_CONFIG_HWFC  - Hardware Flow Control
 
// <0=> Disabled 
// <1=> Enabled 

#ifndef UART_DEFAULT_CONFIG_HWFC
#define UART_DEFAULT_CONFIG_HWFC 0
#endif

// <o> UART_DEFAULT_CONFIG_PARITY  - Parity
 
// <0=> Excluded 
// <14=> Included 

#ifndef UART_DEFAULT_CONFIG_PARITY
#define UART_DEFAULT_CONFIG_PARITY 0
#endif

// <o> UART_DEFAULT_CONFIG_BAUDRATE  - Default Baudrate
 
// <0x0120271C=> 2400 baud 
// <0x0240271C=> 4800 baud 
// <0x0480271C=> 9600 baud 
// <0x04802719=> 12800 baud 
// <0x04802718=> 14400 baud 
// <0x04802716=> 19200 baud 
// <0x04802715=> 23040 baud 
// <0x04802713=> 38400 baud 
// <0x04802712=> 57600 baud 
// <0x04802711=> 115200 baud 
// <0x030007D3=> 1280000 baud 
// <0x0120271C=> 230400 baud 
// <0x060007D3=> 256000 baud 
// <0x12002711=> 460800 baud 
// <0x24002711=> 921600 baud 
// <0x00100011=> 1M baud 

#ifndef UART_DEFAULT_CONFIG_BAUDRATE
#define UART_DEFAULT_CONFIG_BAUDRATE 75507473
#endif

// <o> UART_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority
 

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef UART_DEFAULT_CONFIG_IRQ_PRIORITY
#define UART_DEFAULT_CONFIG_IRQ_PRIORITY 6
#endif

// <q> UART_EASY_DMA_SUPPORT  - Driver supporting EasyDMA
 

#ifndef UART_EASY_DMA_SUPPORT
#define UART_EASY_DMA_SUPPORT 0
#endif

// <q> UART_LEGACY_SUPPORT  - Driver supporting Legacy mode
 

#ifndef UART_LEGACY_SUPPORT
#define UART_LEGACY_SUPPORT 1
#endif

// <e> UART0_ENABLED - Enable UART0 instance
//==========================================================
#ifndef UART0_ENABLED
#define UART0_ENABLED 1
#endif
// <q> UART0_CONFIG_USE_EASY_DMA  - Default setting for using EasyDMA
 

#ifndef UART0_CONFIG_USE_EASY_DMA
#define UART0_CONFIG_USE_EASY_DMA 0
#endif

// </e>

// </e>

// </h> Xinc_Drivers 
//==========================================================
 




// <h> Xinc_Libraries 

//==========================================================
// <q> APP_FIFO_ENABLED  - app_fifo - Software FIFO implementation
 

#ifndef APP_FIFO_ENABLED
#define APP_FIFO_ENABLED 1
#endif


// <e> APP_SCHEDULER_ENABLED - app_scheduler - Events scheduler
//==========================================================
#ifndef APP_SCHEDULER_ENABLED
#define APP_SCHEDULER_ENABLED 1
#endif
// <q> APP_SCHEDULER_WITH_PAUSE  - Enabling pause feature
 

#ifndef APP_SCHEDULER_WITH_PAUSE
#define APP_SCHEDULER_WITH_PAUSE 0
#endif

// <q> APP_SCHEDULER_WITH_PROFILER  - Enabling scheduler profiling
 

#ifndef APP_SCHEDULER_WITH_PROFILER
#define APP_SCHEDULER_WITH_PROFILER 0
#endif

// </e>

// <e> APP_TIMER_ENABLED - app_timer - Application timer functionality
//==========================================================
#ifndef APP_TIMER_ENABLED
#define APP_TIMER_ENABLED 1
#endif
// <o> APP_TIMER_CONFIG_TIMER_FREQUENCY  - Configure RTC prescaler.
 
// <0=> 1000 Hz 

#ifndef APP_TIMER_CONFIG_TIMER_FREQUENCY
#define APP_TIMER_CONFIG_TIMER_FREQUENCY 0
#endif


// <o> APP_TIMER_CONFIG_OP_QUEUE_SIZE - Capacity of timer requests queue. 
// <i> Size of the queue depends on how many timers are used
// <i> in the system, how often timers are started and overall
// <i> system latency. If queue size is too small app_timer calls
// <i> will fail.

#ifndef APP_TIMER_CONFIG_OP_QUEUE_SIZE
#define APP_TIMER_CONFIG_OP_QUEUE_SIZE 10
#endif

// <q> APP_TIMER_CONFIG_USE_SCHEDULER  - Enable scheduling app_timer events to app_scheduler
 

#ifndef APP_TIMER_CONFIG_USE_SCHEDULER
#define APP_TIMER_CONFIG_USE_SCHEDULER 1
#endif

// <q> APP_TIMER_KEEPS_RTC_ACTIVE  - Enable RTC always on
 

// <i> If option is enabled RTC is kept running even if there is no active timers.
// <i> This option can be used when app_timer is used for timestamping.

#ifndef APP_TIMER_KEEPS_RTC_ACTIVE
#define APP_TIMER_KEEPS_RTC_ACTIVE 0
#endif

// <o> APP_TIMER_SAFE_WINDOW_MS - Maximum possible latency (in milliseconds) of handling app_timer event. 
// <i> Maximum possible timeout that can be set is reduced by safe window.
// <i> Example: RTC frequency 16384 Hz, maximum possible timeout 1024 seconds - APP_TIMER_SAFE_WINDOW_MS.
// <i> Since RTC is not stopped when processor is halted in debugging session, this value
// <i> must cover it if debugging is needed. It is possible to halt processor for APP_TIMER_SAFE_WINDOW_MS
// <i> without corrupting app_timer behavior.

#ifndef APP_TIMER_SAFE_WINDOW_MS
#define APP_TIMER_SAFE_WINDOW_MS 300000
#endif


// <h> App Timer Legacy configuration - Legacy configuration.

//==========================================================
// <q> APP_TIMER_WITH_PROFILER  - Enable app_timer profiling
 

#ifndef APP_TIMER_WITH_PROFILER
#define APP_TIMER_WITH_PROFILER 0
#endif

// <q> APP_TIMER_CONFIG_SWI_NUMBER  - Configure SWI instance used.
 

#ifndef APP_TIMER_CONFIG_SWI_NUMBER
#define APP_TIMER_CONFIG_SWI_NUMBER 0
#endif
// </h> 
//==========================================================
// </e>






// <e> APP_UART_ENABLED - app_uart - UART driver
//==========================================================
#ifndef APP_UART_ENABLED
#define APP_UART_ENABLED 1
#endif

// <q> APP_UART_DRIVER_INSTANCE0  - Enable APP UART0 instance

#ifndef APP_UART_DRIVER_INSTANCE0
#define APP_UART_DRIVER_INSTANCE0 1
#endif

// <q> APP_UART_DRIVER_INSTANCE1  - Enable APP UART1 instance

#ifndef APP_UART_DRIVER_INSTANCE1
#define APP_UART_DRIVER_INSTANCE1 1
#endif

// <q> APP_UART_DRIVER_INSTANCE2  - Enable APP UART2 instance

#ifndef APP_UART_DRIVER_INSTANCE2
#define APP_UART_DRIVER_INSTANCE2 0
#endif

// <o> APP_UART_DRIVER_BUFFSIZE  - UART BUFFSIZE <1-255>Bytes

#ifndef APP_UART_DRIVER_BUFFSIZE
#define APP_UART_DRIVER_BUFFSIZE 32
#endif

// </e>


// <e> FDS_ENABLED - fds - Flash data storage module
//==========================================================
#ifndef FDS_ENABLED
#define FDS_ENABLED 0
#endif


// <h> Pages - Virtual page settings

// <i> Configure the number of virtual pages to use and their size.
//==========================================================
// <o> FDS_VIRTUAL_PAGES - Number of virtual flash pages to use. 
// <i> One of the virtual pages is reserved by the system for garbage collection.
// <i> Therefore, the minimum is two virtual pages: one page to store data and one page to be used by the system for garbage collection.
// <i> The total amount of flash memory that is used by FDS amounts to @ref FDS_VIRTUAL_PAGES * @ref FDS_VIRTUAL_PAGE_SIZE * 4 bytes.

#ifndef FDS_VIRTUAL_PAGES
#define FDS_VIRTUAL_PAGES 3
#endif

// <o> FDS_VIRTUAL_PAGE_SIZE  - The size of a virtual flash page.
 

// <i> Expressed in number of 4-byte words.
// <i> By default, a virtual page is the same size as a physical page.
// <i> The size of a virtual page must be a multiple of the size of a physical page.
// <1024=> 1024 
// <2048=> 2048 

#ifndef FDS_VIRTUAL_PAGE_SIZE
#define FDS_VIRTUAL_PAGE_SIZE 512
#endif

// <o> FDS_VIRTUAL_PAGES_RESERVED - The number of virtual flash pages that are used by other modules. 
// <i> FDS module stores its data in the last pages of the flash memory.
// <i> By setting this value, you can move flash end address used by the FDS.
// <i> As a result the reserved space can be used by other modules.

#ifndef FDS_VIRTUAL_PAGES_RESERVED
#define FDS_VIRTUAL_PAGES_RESERVED 0
#endif

// </h> 
//==========================================================

// <h> Backend - Backend configuration

// <i> Configure which xinc_fstorage backend is used by FDS to write to flash.
//==========================================================
// <o> FDS_BACKEND  - FDS flash backend.
 

// <i> XINC_FSTORAGE_SD uses the xinc_fstorage_sd backend implementation using the SoftDevice API. Use this if you have a SoftDevice present.
// <i> XINC_FSTORAGE_NVMC uses the xinc_fstorage_nvmc implementation. Use this setting if you don't use the SoftDevice.
// <1=> XINC_FSTORAGE_NVMC 
// <2=> XINC_FSTORAGE_SD 

#ifndef FDS_BACKEND
#define FDS_BACKEND 1
#endif

// </h> 
//==========================================================

// <h> Queue - Queue settings

//==========================================================
// <o> FDS_OP_QUEUE_SIZE - Size of the internal queue. 
// <i> Increase this value if you frequently get synchronous FDS_ERR_NO_SPACE_IN_QUEUES errors.

#ifndef FDS_OP_QUEUE_SIZE
#define FDS_OP_QUEUE_SIZE 4
#endif

// </h> 
//==========================================================

// <h> CRC - CRC functionality

//==========================================================
// <e> FDS_CRC_CHECK_ON_READ - Enable CRC checks.

// <i> Save a record's CRC when it is written to flash and check it when the record is opened.
// <i> Records with an incorrect CRC can still be 'seen' by the user using FDS functions, but they cannot be opened.
// <i> Additionally, they will not be garbage collected until they are deleted.
//==========================================================
#ifndef FDS_CRC_CHECK_ON_READ
#define FDS_CRC_CHECK_ON_READ 0
#endif
// <o> FDS_CRC_CHECK_ON_WRITE  - Perform a CRC check on newly written records.
 

// <i> Perform a CRC check on newly written records.
// <i> This setting can be used to make sure that the record data was not altered while being written to flash.
// <1=> Enabled 
// <0=> Disabled 

#ifndef FDS_CRC_CHECK_ON_WRITE
#define FDS_CRC_CHECK_ON_WRITE 0
#endif

// </e>

// </h> 

//==========================================================


// <h> Users - Number of users

//==========================================================
// <o> FDS_MAX_USERS - Maximum number of callbacks that can be registered. 
#ifndef FDS_MAX_USERS
#define FDS_MAX_USERS 4
#endif

// </h> 
//==========================================================

// </e>

// <q> XINC_CLI_UART_ENABLED  - xinc_cli_uart - UART command line interface transport
 

#ifndef XINC_CLI_UART_ENABLED
#define XINC_CLI_UART_ENABLED 1
#endif

// <e> XINC_FSTORAGE_ENABLED - xinc_fstorage - Flash abstraction library
//==========================================================
#ifndef XINC_FSTORAGE_ENABLED
#define XINC_FSTORAGE_ENABLED 0
#endif
// <h> xinc_fstorage - Common settings

// <i> Common settings to all fstorage implementations
//==========================================================
// <q> XINC_FSTORAGE_PARAM_CHECK_DISABLED  - Disable user input validation
 

// <i> If selected, use ASSERT to validate user input.
// <i> This effectively removes user input validation in production code.
// <i> Recommended setting: OFF, only enable this setting if size is a major concern.

#ifndef XINC_FSTORAGE_PARAM_CHECK_DISABLED
#define XINC_FSTORAGE_PARAM_CHECK_DISABLED 0
#endif
// </h> 
// </e>  XINC_FSTORAGE_ENABLED - xinc_fstorage - Flash abstraction library

// <e> MEM_MANAGER_ENABLED - mem_manager - Dynamic memory allocator
//==========================================================
#ifndef MEM_MANAGER_ENABLED
#define MEM_MANAGER_ENABLED 1
#endif
// <o> MEMORY_MANAGER_SMALL_BLOCK_COUNT - Size of each memory blocks identified as 'small' block.  <0-255> 


#ifndef MEMORY_MANAGER_SMALL_BLOCK_COUNT
#define MEMORY_MANAGER_SMALL_BLOCK_COUNT 2
#endif

// <o> MEMORY_MANAGER_SMALL_BLOCK_SIZE -  Size of each memory blocks identified as 'small' block. 
// <i>  Size of each memory blocks identified as 'small' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_SMALL_BLOCK_SIZE
#define MEMORY_MANAGER_SMALL_BLOCK_SIZE 32
#endif

// <o> MEMORY_MANAGER_MEDIUM_BLOCK_COUNT - Size of each memory blocks identified as 'medium' block.  <0-255> 


#ifndef MEMORY_MANAGER_MEDIUM_BLOCK_COUNT
#define MEMORY_MANAGER_MEDIUM_BLOCK_COUNT 1
#endif

// <o> MEMORY_MANAGER_MEDIUM_BLOCK_SIZE -  Size of each memory blocks identified as 'medium' block. 
// <i>  Size of each memory blocks identified as 'medium' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_MEDIUM_BLOCK_SIZE
#define MEMORY_MANAGER_MEDIUM_BLOCK_SIZE 256
#endif

// <o> MEMORY_MANAGER_LARGE_BLOCK_COUNT - Size of each memory blocks identified as 'large' block.  <0-255> 


#ifndef MEMORY_MANAGER_LARGE_BLOCK_COUNT
#define MEMORY_MANAGER_LARGE_BLOCK_COUNT 0
#endif

// <o> MEMORY_MANAGER_LARGE_BLOCK_SIZE -  Size of each memory blocks identified as 'large' block. 
// <i>  Size of each memory blocks identified as 'large' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_LARGE_BLOCK_SIZE
#define MEMORY_MANAGER_LARGE_BLOCK_SIZE 256
#endif

// <o> MEMORY_MANAGER_XLARGE_BLOCK_COUNT - Size of each memory blocks identified as 'extra large' block.  <0-255> 


#ifndef MEMORY_MANAGER_XLARGE_BLOCK_COUNT
#define MEMORY_MANAGER_XLARGE_BLOCK_COUNT 1
#endif

// <o> MEMORY_MANAGER_XLARGE_BLOCK_SIZE -  Size of each memory blocks identified as 'extra large' block. 
// <i>  Size of each memory blocks identified as 'extra large' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_XLARGE_BLOCK_SIZE
#define MEMORY_MANAGER_XLARGE_BLOCK_SIZE 1320
#endif

// <o> MEMORY_MANAGER_XXLARGE_BLOCK_COUNT - Size of each memory blocks identified as 'extra extra large' block.  <0-255> 


#ifndef MEMORY_MANAGER_XXLARGE_BLOCK_COUNT
#define MEMORY_MANAGER_XXLARGE_BLOCK_COUNT 0
#endif

// <o> MEMORY_MANAGER_XXLARGE_BLOCK_SIZE -  Size of each memory blocks identified as 'extra extra large' block. 
// <i>  Size of each memory blocks identified as 'extra extra large' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_XXLARGE_BLOCK_SIZE
#define MEMORY_MANAGER_XXLARGE_BLOCK_SIZE 3444
#endif

// <o> MEMORY_MANAGER_XSMALL_BLOCK_COUNT - Size of each memory blocks identified as 'extra small' block.  <0-255> 


#ifndef MEMORY_MANAGER_XSMALL_BLOCK_COUNT
#define MEMORY_MANAGER_XSMALL_BLOCK_COUNT 0
#endif

// <o> MEMORY_MANAGER_XSMALL_BLOCK_SIZE -  Size of each memory blocks identified as 'extra small' block. 
// <i>  Size of each memory blocks identified as 'extra large' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_XSMALL_BLOCK_SIZE
#define MEMORY_MANAGER_XSMALL_BLOCK_SIZE 64
#endif

// <o> MEMORY_MANAGER_XXSMALL_BLOCK_COUNT - Size of each memory blocks identified as 'extra extra small' block.  <0-255> 


#ifndef MEMORY_MANAGER_XXSMALL_BLOCK_COUNT
#define MEMORY_MANAGER_XXSMALL_BLOCK_COUNT 0
#endif

// <o> MEMORY_MANAGER_XXSMALL_BLOCK_SIZE -  Size of each memory blocks identified as 'extra extra small' block. 
// <i>  Size of each memory blocks identified as 'extra extra small' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_XXSMALL_BLOCK_SIZE
#define MEMORY_MANAGER_XXSMALL_BLOCK_SIZE 32
#endif

// <e> MEM_MANAGER_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef MEM_MANAGER_CONFIG_LOG_ENABLED
#define MEM_MANAGER_CONFIG_LOG_ENABLED 0
#endif
// <o> MEM_MANAGER_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef MEM_MANAGER_CONFIG_LOG_LEVEL
#define MEM_MANAGER_CONFIG_LOG_LEVEL 3
#endif

// <o> MEM_MANAGER_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef MEM_MANAGER_CONFIG_INFO_COLOR
#define MEM_MANAGER_CONFIG_INFO_COLOR 0
#endif

// <o> MEM_MANAGER_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef MEM_MANAGER_CONFIG_DEBUG_COLOR
#define MEM_MANAGER_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <q> MEM_MANAGER_DISABLE_API_PARAM_CHECK  - Disable API parameter checks in the module.
 

#ifndef MEM_MANAGER_DISABLE_API_PARAM_CHECK
#define MEM_MANAGER_DISABLE_API_PARAM_CHECK 0
#endif

// </e>

// <e> XINC_BALLOC_ENABLED - xinc_balloc - Block allocator module
//==========================================================
#ifndef XINC_BALLOC_ENABLED
#define XINC_BALLOC_ENABLED 1
#endif
// <e> XINC_BALLOC_CONFIG_DEBUG_ENABLED - Enables debug mode in the module.
//==========================================================
#ifndef XINC_BALLOC_CONFIG_DEBUG_ENABLED
#define XINC_BALLOC_CONFIG_DEBUG_ENABLED 1
#endif
// <o> XINC_BALLOC_CONFIG_HEAD_GUARD_WORDS - Number of words used as head guard.  <0-255> 


#ifndef XINC_BALLOC_CONFIG_HEAD_GUARD_WORDS
#define XINC_BALLOC_CONFIG_HEAD_GUARD_WORDS 1
#endif

// <o> XINC_BALLOC_CONFIG_TAIL_GUARD_WORDS - Number of words used as tail guard.  <0-255> 


#ifndef XINC_BALLOC_CONFIG_TAIL_GUARD_WORDS
#define XINC_BALLOC_CONFIG_TAIL_GUARD_WORDS 1
#endif

// <q> XINC_BALLOC_CONFIG_BASIC_CHECKS_ENABLED  - Enables basic checks in this module.
 

#ifndef XINC_BALLOC_CONFIG_BASIC_CHECKS_ENABLED
#define XINC_BALLOC_CONFIG_BASIC_CHECKS_ENABLED 0
#endif

// <q> XINC_BALLOC_CONFIG_DOUBLE_FREE_CHECK_ENABLED  - Enables double memory free check in this module.
 

#ifndef XINC_BALLOC_CONFIG_DOUBLE_FREE_CHECK_ENABLED
#define XINC_BALLOC_CONFIG_DOUBLE_FREE_CHECK_ENABLED 0
#endif

// <q> XINC_BALLOC_CONFIG_DATA_TRASHING_CHECK_ENABLED  - Enables free memory corruption check in this module.
 

#ifndef XINC_BALLOC_CONFIG_DATA_TRASHING_CHECK_ENABLED
#define XINC_BALLOC_CONFIG_DATA_TRASHING_CHECK_ENABLED 0
#endif

// <q> XINC_BALLOC_CLI_CMDS  - Enable CLI commands specific to the module
 

#ifndef XINC_BALLOC_CLI_CMDS
#define XINC_BALLOC_CLI_CMDS 0
#endif

// </e>

// </e>


// <e> XINC_PWR_MGMT_ENABLED - xinc_pwr_mgmt - Power management module
//==========================================================
#ifndef XINC_PWR_MGMT_ENABLED
#define XINC_PWR_MGMT_ENABLED 1
#endif
// <e> XINC_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED - Enables pin debug in the module.

// <i> Selected pin will be set when CPU is in sleep mode.
//==========================================================
#ifndef XINC_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED
#define XINC_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED 0
#endif
// <o> XINC_PWR_MGMT_SLEEP_DEBUG_PIN  - Pin number
 
// <0=> 0 (P0.0) 
// <1=> 1 (P0.1) 
// <2=> 2 (P0.2) 
// <3=> 3 (P0.3) 
// <4=> 4 (P0.4) 
// <5=> 5 (P0.5) 
// <6=> 6 (P0.6) 
// <7=> 7 (P0.7) 
// <8=> 8 (P0.8) 
// <9=> 9 (P0.9) 
// <10=> 10 (P0.10) 
// <11=> 11 (P0.11) 
// <12=> 12 (P0.12) 
// <13=> 13 (P0.13) 
// <14=> 14 (P0.14) 
// <15=> 15 (P0.15) 
// <16=> 16 (P0.16) 
// <17=> 17 (P0.17) 
// <18=> 18 (P0.18) 
// <19=> 19 (P0.19) 
// <20=> 20 (P0.20) 
// <21=> 21 (P0.21) 
// <22=> 22 (P0.22) 
// <23=> 23 (P0.23) 
// <24=> 24 (P0.24) 
// <25=> 25 (P0.25) 
// <26=> 26 (P0.26) 
// <27=> 27 (P0.27) 
// <28=> 28 (P0.28) 
// <29=> 29 (P0.29) 
// <30=> 30 (P0.30) 
// <31=> 31 (P0.31) 
// <32=> 32 (P1.0) 
// <33=> 33 (P1.1) 
// <34=> 34 (P1.2) 
// <35=> 35 (P1.3) 
// <36=> 36 (P1.4) 
// <37=> 37 (P1.5) 
// <38=> 38 (P1.6) 
// <39=> 39 (P1.7) 
// <40=> 40 (P1.8) 
// <41=> 41 (P1.9) 
// <42=> 42 (P1.10) 
// <43=> 43 (P1.11) 
// <44=> 44 (P1.12) 
// <45=> 45 (P1.13) 
// <46=> 46 (P1.14) 
// <47=> 47 (P1.15) 
// <4294967295=> Not connected 

#ifndef XINC_PWR_MGMT_SLEEP_DEBUG_PIN
#define XINC_PWR_MGMT_SLEEP_DEBUG_PIN 31
#endif

// </e>

// <e> XINC_PWR_MGMT_CONFIG_CPU_SLEEP_MONITOR_ENABLED Enable CPU Sleep.

#ifndef XINC_PWR_MGMT_CONFIG_CPU_SLEEP_MONITOR_ENABLED
#define XINC_PWR_MGMT_CONFIG_CPU_SLEEP_MONITOR_ENABLED 1
#endif

// <q> XINC_PWR_MGMT_CONFIG_BLE_SLEEP_STACK_ENABLED - Enable Ble Stack Sleep.
//==========================================================
#ifndef XINC_PWR_MGMT_CONFIG_BLE_STACK_SLEEP_ENABLED
#define XINC_PWR_MGMT_CONFIG_BLE_STACK_SLEEP_ENABLED 1
#endif
// </e>


// <e> XINC_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED - Enable standby timeout.
//==========================================================
#ifndef XINC_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED
#define XINC_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED 0
#endif
// <o> XINC_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S - Standby timeout (in seconds). 
// <i> Shutdown procedure will begin no earlier than after this number of seconds.

#ifndef XINC_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S
#define XINC_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S 3
#endif

// </e>

// <q> XINC_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED  - Enables FPU event cleaning.
 

#ifndef XINC_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED
#define XINC_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED 0
#endif

// <q> XINC_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY  - Blocked shutdown procedure will be retried every second.
 

#ifndef XINC_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY
#define XINC_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY 0
#endif

// <q> XINC_PWR_MGMT_CONFIG_USE_SCHEDULER  - Module will use @ref app_scheduler.
 

#ifndef XINC_PWR_MGMT_CONFIG_USE_SCHEDULER
#define XINC_PWR_MGMT_CONFIG_USE_SCHEDULER 1
#endif



// </e>


// <e> XINC_QUEUE_ENABLED - xinc_queue - Queue module

#ifndef XINC_QUEUE_ENABLED
#define XINC_QUEUE_ENABLED 1
#endif

// <q> XINC_SECTION_ITER_ENABLED  - xinc_section_iter - Section iterator
 

#ifndef XINC_SECTION_ITER_ENABLED
#define XINC_SECTION_ITER_ENABLED 1
#endif


// <q> XINC_QUEUE_CLI_CMDS  - Enable CLI commands specific to the module
 

#ifndef XINC_QUEUE_CLI_CMDS
#define XINC_QUEUE_CLI_CMDS 1
#endif

// </e>


// <q> XINC_STRERROR_ENABLED  - xinc_strerror - Library for converting error code to string.

#ifndef XINC_STRERROR_ENABLED
#define XINC_STRERROR_ENABLED 1
#endif


// <h> app_button - buttons handling module

//==========================================================
// <q> BUTTON_ENABLED  - Enables Button module
 

#ifndef BUTTON_ENABLED
#define BUTTON_ENABLED 1
#endif

// <q> BUTTON_HIGH_ACCURACY_ENABLED  - Enables GPIOTE high accuracy for buttons
 

#ifndef BUTTON_HIGH_ACCURACY_ENABLED
#define BUTTON_HIGH_ACCURACY_ENABLED 0
#endif

// </h> 
//==========================================================



// <h> xinc_cli - Command line interface

//==========================================================
// <q> XINC_CLI_ENABLED  - Enable/disable the CLI module.
 

#ifndef XINC_CLI_ENABLED
#define XINC_CLI_ENABLED 1
#endif

// <o> XINC_CLI_ARGC_MAX - Maximum number of parameters passed to the command handler. 
#ifndef XINC_CLI_ARGC_MAX
#define XINC_CLI_ARGC_MAX 12
#endif

// <q> XINC_CLI_BUILD_IN_CMDS_ENABLED  - CLI built-in commands.
 
#ifndef XINC_CLI_BUILD_IN_CMDS_ENABLED
#define XINC_CLI_BUILD_IN_CMDS_ENABLED 1
#endif

// <o> XINC_CLI_CMD_BUFF_SIZE - Maximum buffer size for a single command. 
#ifndef XINC_CLI_CMD_BUFF_SIZE
#define XINC_CLI_CMD_BUFF_SIZE 128
#endif

// <q> XINC_CLI_ECHO_STATUS  - CLI echo status. If set, echo is ON.
 
#ifndef XINC_CLI_ECHO_STATUS
#define XINC_CLI_ECHO_STATUS 1
#endif

// <q> XINC_CLI_WILDCARD_ENABLED  - Enable wildcard functionality for CLI commands.
 
#ifndef XINC_CLI_WILDCARD_ENABLED
#define XINC_CLI_WILDCARD_ENABLED 0
#endif

// <q> XINC_CLI_METAKEYS_ENABLED  - Enable additional control keys for CLI commands like ctrl+a, ctrl+e, ctrl+w, ctrl+u
 

#ifndef XINC_CLI_METAKEYS_ENABLED
#define XINC_CLI_METAKEYS_ENABLED 0
#endif

// <o> XINC_CLI_PRINTF_BUFF_SIZE - Maximum print buffer size. 
#ifndef XINC_CLI_PRINTF_BUFF_SIZE
#define XINC_CLI_PRINTF_BUFF_SIZE 23
#endif

// <e> XINC_CLI_HISTORY_ENABLED - Enable CLI history mode.
//==========================================================
#ifndef XINC_CLI_HISTORY_ENABLED
#define XINC_CLI_HISTORY_ENABLED 0
#endif
// <o> XINC_CLI_HISTORY_ELEMENT_SIZE - Size of one memory object reserved for CLI history. 
#ifndef XINC_CLI_HISTORY_ELEMENT_SIZE
#define XINC_CLI_HISTORY_ELEMENT_SIZE 32
#endif

// <o> XINC_CLI_HISTORY_ELEMENT_COUNT - Number of history memory objects. 
#ifndef XINC_CLI_HISTORY_ELEMENT_COUNT
#define XINC_CLI_HISTORY_ELEMENT_COUNT 8
#endif

// </e>

// <q> XINC_CLI_VT100_COLORS_ENABLED  - CLI VT100 colors.
 

#ifndef XINC_CLI_VT100_COLORS_ENABLED
#define XINC_CLI_VT100_COLORS_ENABLED 0
#endif


// <q> XINC_CLI_STATISTICS_ENABLED  - Enable CLI statistics.
 

#ifndef XINC_CLI_STATISTICS_ENABLED
#define XINC_CLI_STATISTICS_ENABLED 0
#endif

// <q> XINC_CLI_LOG_BACKEND  - Enable logger backend interface.
 

#ifndef XINC_CLI_LOG_BACKEND
#define XINC_CLI_LOG_BACKEND 0
#endif

// <q> XINC_CLI_USES_TASK_MANAGER_ENABLED  - Enable CLI to use task_manager
 

#ifndef XINC_CLI_USES_TASK_MANAGER_ENABLED
#define XINC_CLI_USES_TASK_MANAGER_ENABLED 0
#endif
// </h> 

//===============xinc_cli===================================


// <h> xinc_fprintf - fprintf function.

//==========================================================
// <q> XINC_FPRINTF_ENABLED  - Enable/disable fprintf module.
 

#ifndef XINC_FPRINTF_ENABLED
#define XINC_FPRINTF_ENABLED 1
#endif

// <q> XINC_FPRINTF_FLAG_AUTOMATIC_CR_ON_LF_ENABLED  - For each printed LF, function will add CR.
 

#ifndef XINC_FPRINTF_FLAG_AUTOMATIC_CR_ON_LF_ENABLED
#define XINC_FPRINTF_FLAG_AUTOMATIC_CR_ON_LF_ENABLED 0
#endif

// <q> XINC_FPRINTF_DOUBLE_ENABLED  - Enable IEEE-754 double precision formatting.
 

#ifndef XINC_FPRINTF_DOUBLE_ENABLED
#define XINC_FPRINTF_DOUBLE_ENABLED 0
#endif

// </h> 
//===============xinc_fprintf===================================


// </h>
//==========================================================



// <h> Xinc_Log 

// <e> XINC_LOG_BACKEND_UART_ENABLED - xinc_log_backend_uart - Log UART backend
//==========================================================
#ifndef XINC_LOG_BACKEND_UART_ENABLED
#define XINC_LOG_BACKEND_UART_ENABLED 0
#endif

// <o> XINC_LOG_BACKEND_UART_TX_PIN - UART TX pin 
#ifndef XINC_LOG_BACKEND_UART_TX_PIN
#define XINC_LOG_BACKEND_UART_TX_PIN 22
#endif

// <o> XINC_LOG_BACKEND_UART_BAUDRATE  - Default Baudrate
// <323584=> 1200 baud 
// <643072=> 2400 baud 
// <1290240=> 4800 baud 
// <2576384=> 9600 baud 
// <3862528=> 14400 baud 
// <5152768=> 19200 baud 
// <7716864=> 28800 baud 
// <10289152=> 38400 baud 
// <15400960=> 57600 baud 
// <20615168=> 76800 baud 
// <30801920=> 115200 baud 
// <61865984=> 230400 baud 
// <67108864=> 250000 baud 
// <121634816=> 460800 baud 
// <251658240=> 921600 baud 
// <268435456=> 1000000 baud 

#ifndef XINC_LOG_BACKEND_UART_BAUDRATE
#define XINC_LOG_BACKEND_UART_BAUDRATE 30801920
#endif
// <o> XINC_LOG_BACKEND_UART_TEMP_BUFFER_SIZE - Size of buffer for partially processed strings. 
// <i> Size of the buffer is a trade-off between RAM usage and processing.
// <i> if buffer is smaller then strings will often be fragmented.
// <i> It is recommended to use size which will fit typical log and only the
// <i> longer one will be fragmented.

#ifndef XINC_LOG_BACKEND_UART_TEMP_BUFFER_SIZE
#define XINC_LOG_BACKEND_UART_TEMP_BUFFER_SIZE 164
#endif

// </e>

//==========================================================
// <e> XINC_LOG_ENABLED - xinc_log - Logger
//==========================================================
#ifndef XINC_LOG_ENABLED
#define XINC_LOG_ENABLED 0
#endif



// <h> Log message pool - Configuration of log message pool

//==========================================================
// <o> XINC_LOG_MSGPOOL_ELEMENT_SIZE - Size of a single element in the pool of memory objects. 
// <i> If a small value is set, then performance of logs processing
// <i> is degraded because data is fragmented. Bigger value impacts
// <i> RAM memory utilization. The size is set to fit a message with
// <i> a timestamp and up to 2 arguments in a single memory object.

#ifndef XINC_LOG_MSGPOOL_ELEMENT_SIZE
#define XINC_LOG_MSGPOOL_ELEMENT_SIZE 20
#endif

// <o> XINC_LOG_MSGPOOL_ELEMENT_COUNT - Number of elements in the pool of memory objects 
// <i> If a small value is set, then it may lead to a deadlock
// <i> in certain cases if backend has high latency and holds
// <i> multiple messages for long time. Bigger value impacts
// <i> RAM memory usage.

#ifndef XINC_LOG_MSGPOOL_ELEMENT_COUNT
#define XINC_LOG_MSGPOOL_ELEMENT_COUNT 8
#endif

// </h> 
//==========================================================

// <q> XINC_LOG_ALLOW_OVERFLOW  - Configures behavior when circular buffer is full.
 

// <i> If set then oldest logs are overwritten. Otherwise a 
// <i> marker is injected informing about overflow.

#ifndef XINC_LOG_ALLOW_OVERFLOW
#define XINC_LOG_ALLOW_OVERFLOW 1
#endif

// <o> XINC_LOG_BUFSIZE  - Size of the buffer for storing logs (in bytes).
 

// <i> Must be power of 2 and multiple of 4.
// <i> If XINC_LOG_DEFERRED = 0 then buffer size can be reduced to minimum.
// <128=> 128 
// <256=> 256 
// <512=> 512 
// <1024=> 1024 
// <2048=> 2048 
// <4096=> 4096 
// <8192=> 8192 
// <16384=> 16384 

#ifndef XINC_LOG_BUFSIZE
#define XINC_LOG_BUFSIZE 1024
#endif

// <q> XINC_LOG_CLI_CMDS  - Enable CLI commands for the module.
 

#ifndef XINC_LOG_CLI_CMDS
#define XINC_LOG_CLI_CMDS 0
#endif

// <o> XINC_LOG_DEFAULT_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef XINC_LOG_DEFAULT_LEVEL
#define XINC_LOG_DEFAULT_LEVEL 3
#endif

// <q> XINC_LOG_DEFERRED  - Enable deffered logger.
 

// <i> Log data is buffered and can be processed in idle.

#ifndef XINC_LOG_DEFERRED
#define XINC_LOG_DEFERRED 1
#endif

// <q> XINC_LOG_FILTERS_ENABLED  - Enable dynamic filtering of logs.
 

#ifndef XINC_LOG_FILTERS_ENABLED
#define XINC_LOG_FILTERS_ENABLED 0
#endif

// <q> XINC_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED  - Enable use of critical region for non deffered mode when flushing logs.
 

// <i> When enabled XINC_LOG_FLUSH is called from critical section when non deffered mode is used.
// <i> Log output will never be corrupted as access to the log backend is exclusive
// <i> but system will spend significant amount of time in critical section

#ifndef XINC_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED
#define XINC_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED 0
#endif

// <o> XINC_LOG_STR_PUSH_BUFFER_SIZE  - Size of the buffer dedicated for strings stored using @ref XINC_LOG_PUSH.
 
// <16=> 16 
// <32=> 32 
// <64=> 64 
// <128=> 128 
// <256=> 256 
// <512=> 512 
// <1024=> 1024 

#ifndef XINC_LOG_STR_PUSH_BUFFER_SIZE
#define XINC_LOG_STR_PUSH_BUFFER_SIZE 256
#endif

// <o> XINC_LOG_STR_PUSH_BUFFER_SIZE  - Size of the buffer dedicated for strings stored using @ref XINC_LOG_PUSH.
 
// <16=> 16 
// <32=> 32 
// <64=> 64 
// <128=> 128 
// <256=> 256 
// <512=> 512 
// <1024=> 1024 

#ifndef XINC_LOG_STR_PUSH_BUFFER_SIZE
#define XINC_LOG_STR_PUSH_BUFFER_SIZE 256
#endif

// <e> XINC_LOG_USES_COLORS - If enabled then ANSI escape code for colors is prefixed to every string
//==========================================================
#ifndef XINC_LOG_USES_COLORS
#define XINC_LOG_USES_COLORS 0
#endif

// <o> XINC_LOG_COLOR_DEFAULT  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINC_LOG_COLOR_DEFAULT
#define XINC_LOG_COLOR_DEFAULT 0
#endif

// <o> XINC_LOG_ERROR_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINC_LOG_ERROR_COLOR
#define XINC_LOG_ERROR_COLOR 2
#endif

// <o> XINC_LOG_WARNING_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINC_LOG_WARNING_COLOR
#define XINC_LOG_WARNING_COLOR 4
#endif

// </e>

// <e> XINC_LOG_USES_TIMESTAMP - Enable timestamping

// <i> Function for getting the timestamp is provided by the user
//==========================================================
#ifndef XINC_LOG_USES_TIMESTAMP
#define XINC_LOG_USES_TIMESTAMP 0
#endif
// <o> XINC_LOG_TIMESTAMP_DEFAULT_FREQUENCY - Default frequency of the timestamp (in Hz) or 0 to use app_timer frequency. 
#ifndef XINC_LOG_TIMESTAMP_DEFAULT_FREQUENCY
#define XINC_LOG_TIMESTAMP_DEFAULT_FREQUENCY 0
#endif

// </e>

// <h> xinc_log module configuration 
//==========================================================


// <h> xinc_log in nRF_Core 
//==========================================================

// </h> xinc_log in nRF_Core 
//==========================================================


// <h> xinc_log in nRF_Drivers 
//==========================================================

// </h>
//==========================================================


// <h> xinc_log in nRF_Libraries 
//==========================================================

// <e> XINC_ATFIFO_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef XINC_ATFIFO_CONFIG_LOG_ENABLED
#define XINC_ATFIFO_CONFIG_LOG_ENABLED 0
#endif
// <o> XINC_ATFIFO_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef XINC_ATFIFO_CONFIG_LOG_LEVEL
#define XINC_ATFIFO_CONFIG_LOG_LEVEL 3
#endif


// <o> XINC_ATFIFO_CONFIG_LOG_INIT_FILTER_LEVEL  - Initial severity level if dynamic filtering is enabled
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef XINC_ATFIFO_CONFIG_LOG_INIT_FILTER_LEVEL
#define XINC_ATFIFO_CONFIG_LOG_INIT_FILTER_LEVEL 3
#endif

// <o> XINC_ATFIFO_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINC_ATFIFO_CONFIG_INFO_COLOR
#define XINC_ATFIFO_CONFIG_INFO_COLOR 0
#endif

// <o> XINC_ATFIFO_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINC_ATFIFO_CONFIG_DEBUG_COLOR
#define XINC_ATFIFO_CONFIG_DEBUG_COLOR 0
#endif

// </e>
// <e> XINC_BALLOC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef XINC_BALLOC_CONFIG_LOG_ENABLED
#define XINC_BALLOC_CONFIG_LOG_ENABLED 0
#endif
// <o> XINC_BALLOC_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef XINC_BALLOC_CONFIG_LOG_LEVEL
#define XINC_BALLOC_CONFIG_LOG_LEVEL 0
#endif

// <o> XINC_BALLOC_CONFIG_INITIAL_LOG_LEVEL  - Initial severity level if dynamic filtering is enabled.
 

// <i> If module generates a lot of logs, initial log level can
// <i> be decreased to prevent flooding. Severity level can be
// <i> increased on instance basis.
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef XINC_BALLOC_CONFIG_INITIAL_LOG_LEVEL
#define XINC_BALLOC_CONFIG_INITIAL_LOG_LEVEL 0
#endif

// <o> XINC_BALLOC_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINC_BALLOC_CONFIG_INFO_COLOR
#define XINC_BALLOC_CONFIG_INFO_COLOR 0
#endif

// <o> XINC_BALLOC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINC_BALLOC_CONFIG_DEBUG_COLOR
#define XINC_BALLOC_CONFIG_DEBUG_COLOR 0
#endif

//</e>

// <e> XINC_CLI_UART_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef XINC_CLI_UART_CONFIG_LOG_ENABLED
#define XINC_CLI_UART_CONFIG_LOG_ENABLED 1
#endif
// <o> XINC_CLI_UART_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef XINC_CLI_UART_CONFIG_LOG_LEVEL
#define XINC_CLI_UART_CONFIG_LOG_LEVEL 3
#endif

// <o> XINC_CLI_UART_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINC_CLI_UART_CONFIG_INFO_COLOR
#define XINC_CLI_UART_CONFIG_INFO_COLOR 0
#endif

// <o> XINC_CLI_UART_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef XINC_CLI_UART_CONFIG_DEBUG_COLOR
#define XINC_CLI_UART_CONFIG_DEBUG_COLOR 0
#endif

// </e>


// </h>
//==========================================================

// </h>
//==========================================================



 
 // </e>
 
// <q> XINC_LOG_STR_FORMATTER_TIMESTAMP_FORMAT_ENABLED  - xinc_log_str_formatter - Log string formatter
 

#ifndef XINC_LOG_STR_FORMATTER_TIMESTAMP_FORMAT_ENABLED
#define XINC_LOG_STR_FORMATTER_TIMESTAMP_FORMAT_ENABLED 1
#endif
// </h>
//==========================================================

#endif

