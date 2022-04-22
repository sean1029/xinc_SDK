/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef _XINC620610_PERIPHERALS_H
#define _XINC620610_PERIPHERALS_H


/* Clock Peripheral */
#define CLOCK_PRESENT
#define CLOCK_COUNT 1


/* DMAS Controller */
#define DMAS_PRESENT
#define DMAS_COUNT 1
#define DMAS_CH_COUNT 13



/* Systick timer */
#define SYSTICK_PRESENT
#define SYSTICK_COUNT 1

/* KBS */
#define KBS_PRESENT
#define KBS_ROW_PIN_COUNT 8 //矩阵键盘行个数
#define KBS_COL_PIN_COUNT 18 //矩阵键盘列个数

/* GPIO */
#define GPIO_PRESENT
#define GPIO_COUNT 1

#define P0_PIN_NUM 32
#define P1_PIN_NUM 4

#define P0_FEATURE_PINS_PRESENT 0xFFFFFFFFUL
#define P1_FEATURE_PINS_PRESENT 0x000000000UL


/* Timer/Counter */
#define TIMER_PRESENT
#define TIMER_COUNT 5

#define TIMER0_MAX_SIZE 32
#define TIMER1_MAX_SIZE 32
#define TIMER2_MAX_SIZE 32
#define TIMER3_MAX_SIZE 32
#define TIMER4_MAX_SIZE 32



/* Real Time Counter */
#define RTC_PRESENT
#define RTC_COUNT 3


/* Watchdog Timer */
#define WDT_PRESENT
#define WDT_COUNT 1

/* Temperature Sensor */
#define TEMP_PRESENT
#define TEMP_COUNT 1



/* Serial Peripheral Interface Master with DMA */
#define SPIM_PRESENT
#define SPIM_COUNT 2

#define SPIM0_MAX_DATARATE  8
#define SPIM1_MAX_DATARATE  8
#define SPIM2_MAX_DATARATE  8

#define SPIM0_FEATURE_HARDWARE_CSN_PRESENT  0
#define SPIM1_FEATURE_HARDWARE_CSN_PRESENT  0
#define SPIM2_FEATURE_HARDWARE_CSN_PRESENT  0


#define SPIM0_EASYDMA_MAXCNT_SIZE 8
#define SPIM1_EASYDMA_MAXCNT_SIZE 8
#define SPIM2_EASYDMA_MAXCNT_SIZE 8

#define SPIM0_FEATURE_DCX_PRESENT  0
#define SPIM1_FEATURE_DCX_PRESENT  0
#define SPIM2_FEATURE_DCX_PRESENT  0


#define SPIM0_FEATURE_RXDELAY_PRESENT  0
#define SPIM1_FEATURE_RXDELAY_PRESENT  0
#define SPIM2_FEATURE_RXDELAY_PRESENT  0



/* Two Wire Interface Master */
#define I2C_PRESENT
#define I2C_COUNT 1


/* Two Wire Interface Slave with DMA */
#define I2CS_PRESENT
#define I2CS_COUNT 2

#define I2CS0_EASYDMA_MAXCNT_SIZE 16
#define I2CS1_EASYDMA_MAXCNT_SIZE 16

/* Universal Asynchronous Receiver-Transmitter */
#define UART_PRESENT
#define UART_COUNT 2

/* Universal Asynchronous Receiver-Transmitter with DMA */
#define UARTE_PRESENT
#define UARTE_COUNT 2

#define UARTE0_EASYDMA_MAXCNT_SIZE 16
#define UARTE1_EASYDMA_MAXCNT_SIZE 16

/* Quadrature Decoder */
#define QDEC_PRESENT
#define QDEC_COUNT 1

/* Successive Approximation Analog to Digital Converter */
#define SAADC_PRESENT
#define SAADC_COUNT 1

#define SAADC_EASYDMA_MAXCNT_SIZE 15

#define SAADC_CH_NUM 8

/* GPIO Tasks and Events */
#define GPIOTE_PRESENT
#define GPIOTE_COUNT 1

#define GPIOTE_CH_NUM 32


/* Low Power Comparator */
#define LPCOMP_PRESENT
#define LPCOMP_COUNT 1

#define LPCOMP_REFSEL_RESOLUTION 16

#define LPCOMP_FEATURE_HYST_PRESENT

/* Comparator */
#define COMP_PRESENT
#define COMP_COUNT 1

/* Pulse Width Modulator */
#define PWM_PRESENT
#define PWM_COUNT 4

#define PWM0_CH_NUM 4
#define PWM1_CH_NUM 4
#define PWM2_CH_NUM 4
#define PWM3_CH_NUM 4



/* Inter-IC Sound Interface */
#define I2S_PRESENT
#define I2S_COUNT 1

#define I2S_EASYDMA_MAXCNT_SIZE 14

/* Universal Serial Bus Device */
#define USBD_PRESENT
#define USBD_COUNT 1

#define USBD_EASYDMA_MAXCNT_SIZE 7


/* Quad SPI */
#define QSPI_PRESENT
#define QSPI_COUNT 1

#define QSPI_EASYDMA_MAXCNT_SIZE 20

#endif      // _XINC628_A_PERIPHERALS_H
