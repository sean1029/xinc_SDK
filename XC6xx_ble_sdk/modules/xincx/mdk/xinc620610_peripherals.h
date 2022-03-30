/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef _XINC52840_PERIPHERALS_H
#define _XINC52840_PERIPHERALS_H


/* Clock Peripheral */
#define CLOCK_PRESENT
#define CLOCK_COUNT 1

/* Power Peripheral */
#define POWER_PRESENT
#define POWER_COUNT 1

#define POWER_FEATURE_RAM_REGISTERS_PRESENT
#define POWER_FEATURE_RAM_REGISTERS_COUNT       9

#define POWER_FEATURE_VDDH_PRESENT
#define POWER_FEATURE_VDDH_DCDC_PRESENT

/* DMAS Controller */
#define DMAS_PRESENT
#define DMAS_COUNT 1
#define DMAS_CH_COUNT 13

/* Non-Volatile Memory Controller */
#define NVMC_PRESENT
#define NVMC_COUNT 1

#define NVMC_FEATURE_CACHE_PRESENT

/* Floating Point Unit */
#define FPU_PRESENT
#define FPU_COUNT 1

/* Systick timer */
#define SYSTICK_PRESENT
#define SYSTICK_COUNT 1

/* Software Interrupts */
#define SWI_PRESENT
#define SWI_COUNT 6

/* Memory Watch Unit */
#define MWU_PRESENT
#define MWU_COUNT 1

/* GPIO */
#define GPIO_PRESENT
#define GPIO_COUNT 1

#define P0_PIN_NUM 32
#define P1_PIN_NUM 32

#define P0_FEATURE_PINS_PRESENT 0xFFFFFFFFUL
#define P1_FEATURE_PINS_PRESENT 0xFFFFFFFFUL

/* ACL */
#define ACL_PRESENT

#define ACL_REGIONS_COUNT 8

/* Radio */
#define RADIO_PRESENT
#define RADIO_COUNT 1

#define RADIO_EASYDMA_MAXCNT_SIZE 8
#define RADIO_FEATURE_IEEE_802_15_4_PRESENT

#define RADIO_TXPOWER_TXPOWER_Max RADIO_TXPOWER_TXPOWER_Pos8dBm

/* Accelerated Address Resolver */
#define AAR_PRESENT
#define AAR_COUNT 1

#define AAR_MAX_IRK_NUM 16

/* AES Electronic CodeBook mode encryption */
#define ECB_PRESENT
#define ECB_COUNT 1

/* AES CCM mode encryption */
#define CCM_PRESENT
#define CCM_COUNT 1

/* NFC Tag */
#define NFCT_PRESENT
#define NFCT_COUNT 1

#define NFCT_EASYDMA_MAXCNT_SIZE 9

/* Peripheral to Peripheral Interconnect */
#define PPI_PRESENT
#define PPI_COUNT 1

#define PPI_CH_NUM 20
#define PPI_FIXED_CH_NUM 12
#define PPI_GROUP_NUM 6
#define PPI_FEATURE_FORKS_PRESENT

/* Event Generator Unit */
#define EGU_PRESENT
#define EGU_COUNT 6

#define EGU0_CH_NUM 16
#define EGU1_CH_NUM 16
#define EGU2_CH_NUM 16
#define EGU3_CH_NUM 16
#define EGU4_CH_NUM 16
#define EGU5_CH_NUM 16

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

#define RTC0_CC_NUM 3
#define RTC1_CC_NUM 4
#define RTC2_CC_NUM 4

/* RNG */
#define RNG_PRESENT
#define RNG_COUNT 1

/* Watchdog Timer */
#define WDT_PRESENT
#define WDT_COUNT 1

/* Temperature Sensor */
#define TEMP_PRESENT
#define TEMP_COUNT 1

/* Serial Peripheral Interface Master */
//#define SPI_PRESENT
//#define SPI_COUNT 3

/* Serial Peripheral Interface Master with DMA */
#define SPIM_PRESENT
#define SPIM_COUNT 2

#define SPIM0_MAX_DATARATE  8
#define SPIM1_MAX_DATARATE  8
#define SPIM2_MAX_DATARATE  8
#define SPIM3_MAX_DATARATE  32

#define SPIM0_FEATURE_HARDWARE_CSN_PRESENT  0
#define SPIM1_FEATURE_HARDWARE_CSN_PRESENT  0
#define SPIM2_FEATURE_HARDWARE_CSN_PRESENT  0
#define SPIM3_FEATURE_HARDWARE_CSN_PRESENT  1

#define SPIM0_FEATURE_DCX_PRESENT  0
#define SPIM1_FEATURE_DCX_PRESENT  0
#define SPIM2_FEATURE_DCX_PRESENT  0
#define SPIM3_FEATURE_DCX_PRESENT  1

#define SPIM0_FEATURE_RXDELAY_PRESENT  0
#define SPIM1_FEATURE_RXDELAY_PRESENT  0
#define SPIM2_FEATURE_RXDELAY_PRESENT  0
#define SPIM3_FEATURE_RXDELAY_PRESENT  1

#define SPIM0_EASYDMA_MAXCNT_SIZE 16
#define SPIM1_EASYDMA_MAXCNT_SIZE 16
#define SPIM2_EASYDMA_MAXCNT_SIZE 16
#define SPIM3_EASYDMA_MAXCNT_SIZE 16

/* Serial Peripheral Interface Slave with DMA*/
#define SPIS_PRESENT
#define SPIS_COUNT 3

#define SPIS0_EASYDMA_MAXCNT_SIZE 16
#define SPIS1_EASYDMA_MAXCNT_SIZE 16
#define SPIS2_EASYDMA_MAXCNT_SIZE 16

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

#define GPIOTE_FEATURE_SET_PRESENT
#define GPIOTE_FEATURE_CLR_PRESENT

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

#define PWM0_EASYDMA_MAXCNT_SIZE 15
#define PWM1_EASYDMA_MAXCNT_SIZE 15
#define PWM2_EASYDMA_MAXCNT_SIZE 15
#define PWM3_EASYDMA_MAXCNT_SIZE 15

/* Pulse Density Modulator */
#define PDM_PRESENT
#define PDM_COUNT 1

#define PDM_EASYDMA_MAXCNT_SIZE 15

/* Inter-IC Sound Interface */
#define I2S_PRESENT
#define I2S_COUNT 1

#define I2S_EASYDMA_MAXCNT_SIZE 14

/* Universal Serial Bus Device */
#define USBD_PRESENT
#define USBD_COUNT 1

#define USBD_EASYDMA_MAXCNT_SIZE 7

/* ARM TrustZone Cryptocell 310 */
#define CRYPTOCELL_PRESENT
#define CRYPTOCELL_COUNT 1

/* Quad SPI */
#define QSPI_PRESENT
#define QSPI_COUNT 1

#define QSPI_EASYDMA_MAXCNT_SIZE 20

#endif      // _XINC52840_PERIPHERALS_H
