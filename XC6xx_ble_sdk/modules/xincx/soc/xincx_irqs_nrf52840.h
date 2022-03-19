/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_IRQS_NRF52840_H__
#define XINCX_IRQS_NRF52840_H__

#ifdef __cplusplus
extern "C" {
#endif


// POWER_CLOCK_IRQn
#define xincx_power_clock_irq_handler    POWER_CLOCK_IRQHandler

// RADIO_IRQn

// UARTE0_UART0_IRQn
#if XINCX_CHECK(XINCX_PRS_ENABLED) && XINCX_CHECK(XINCX_PRS_BOX_4_ENABLED)
#define xincx_prs_box_4_irq_handler  UARTE0_UART0_IRQHandler
#else
#define xincx_uarte_0_irq_handler    UARTE0_UART0_IRQHandler
#define xincx_uart_0_irq_handler     UARTE0_UART0_IRQHandler
#endif

// SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn
#if XINCX_CHECK(XINCX_PRS_ENABLED) && XINCX_CHECK(XINCX_PRS_BOX_0_ENABLED)
#define xincx_prs_box_0_irq_handler  SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
#else
#define xincx_spim_0_irq_handler     SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
#define xincx_spis_0_irq_handler     SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
#define xincx_twim_0_irq_handler     SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
#define xincx_twis_0_irq_handler     SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
#define xincx_spi_0_irq_handler      SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
#define xincx_twi_0_irq_handler      SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
#endif

// SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn
#if XINCX_CHECK(XINCX_PRS_ENABLED) && XINCX_CHECK(XINCX_PRS_BOX_1_ENABLED)
#define xincx_prs_box_1_irq_handler  SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
#else
#define xincx_spim_1_irq_handler     SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
#define xincx_spis_1_irq_handler     SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
#define xincx_twim_1_irq_handler     SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
#define xincx_twis_1_irq_handler     SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
#define xincx_spi_1_irq_handler      SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
#define xincx_twi_1_irq_handler      SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
#endif

// NFCT_IRQn
#define xincx_nfct_irq_handler       NFCT_IRQHandler

// GPIOTE_IRQn
#define xincx_gpiote_irq_handler     GPIOTE_IRQHandler

// SAADC_IRQn
#define xincx_saadc_irq_handler      SAADC_IRQHandler

// TIMER0_IRQn
#define xincx_timer_0_irq_handler    TIMER0_IRQHandler

// TIMER1_IRQn
#define xincx_timer_1_irq_handler    TIMER1_IRQHandler

// TIMER2_IRQn
#define xincx_timer_2_irq_handler    TIMER2_IRQHandler

// RTC0_IRQn
#define xincx_rtc_0_irq_handler      RTC0_IRQHandler

// TEMP_IRQn
#define xincx_temp_irq_handler       TEMP_IRQHandler

// RNG_IRQn
#define xincx_rng_irq_handler        RNG_IRQHandler

// ECB_IRQn

// CCM_AAR_IRQn

// WDT_IRQn
#define xincx_wdt_irq_handler        WDT_IRQHandler

// RTC1_IRQn
#define xincx_rtc_1_irq_handler      RTC1_IRQHandler

// QDEC_IRQn
#define xincx_qdec_irq_handler       QDEC_IRQHandler

// COMP_LPCOMP_IRQn
#if XINCX_CHECK(XINCX_PRS_ENABLED) && XINCX_CHECK(XINCX_PRS_BOX_3_ENABLED)
#define xincx_prs_box_3_irq_handler  COMP_LPCOMP_IRQHandler
#else
#define xincx_comp_irq_handler       COMP_LPCOMP_IRQHandler
#define xincx_lpcomp_irq_handler     COMP_LPCOMP_IRQHandler
#endif

// SWI0_EGU0_IRQn
#define xincx_swi_0_irq_handler      SWI0_EGU0_IRQHandler

// SWI1_EGU1_IRQn
#define xincx_swi_1_irq_handler      SWI1_EGU1_IRQHandler

// SWI2_EGU2_IRQn
#define xincx_swi_2_irq_handler      SWI2_EGU2_IRQHandler

// SWI3_EGU3_IRQn
#define xincx_swi_3_irq_handler      SWI3_EGU3_IRQHandler

// SWI4_EGU4_IRQn
#define xincx_swi_4_irq_handler      SWI4_EGU4_IRQHandler

// SWI5_EGU5_IRQn
#define xincx_swi_5_irq_handler      SWI5_EGU5_IRQHandler

// TIMER3_IRQn
#define xincx_timer_3_irq_handler    TIMER3_IRQHandler

// TIMER4_IRQn
#define xincx_timer_4_irq_handler    TIMER4_IRQHandler

// PWM0_IRQn
#define xincx_pwm_0_irq_handler      PWM0_IRQHandler

// PDM_IRQn
#define xincx_pdm_irq_handler        PDM_IRQHandler

// MWU_IRQn

// PWM1_IRQn
#define xincx_pwm_1_irq_handler      PWM1_IRQHandler

// PWM2_IRQn
#define xincx_pwm_2_irq_handler      PWM2_IRQHandler

// SPIM2_SPIS2_SPI2_IRQn
#if XINCX_CHECK(XINCX_PRS_ENABLED) && XINCX_CHECK(XINCX_PRS_BOX_2_ENABLED)
#define xincx_prs_box_2_irq_handler  SPIM2_SPIS2_SPI2_IRQHandler
#else
#define xincx_spim_2_irq_handler     SPIM2_SPIS2_SPI2_IRQHandler
#define xincx_spis_2_irq_handler     SPIM2_SPIS2_SPI2_IRQHandler
#define xincx_spi_2_irq_handler      SPIM2_SPIS2_SPI2_IRQHandler
#endif

// RTC2_IRQn
#define xincx_rtc_2_irq_handler      RTC2_IRQHandler

// I2S_IRQn
#define xincx_i2s_irq_handler        I2S_IRQHandler

// FPU_IRQn

// USBD_IRQn
#define xincx_usbd_irq_handler       USBD_IRQHandler

// UARTE1_IRQn
#define xincx_uarte_1_irq_handler    UARTE1_IRQHandler

// QSPI_IRQn
#define xincx_qspi_irq_handler       QSPI_IRQHandler

// CRYPTOCELL_IRQn

// PWM3_IRQn
#define xincx_pwm_3_irq_handler      PWM3_IRQHandler

// SPIM3_IRQn
#define xincx_spim_3_irq_handler     SPIM3_IRQHandler


#ifdef __cplusplus
}
#endif

#endif // XINCX_IRQS_NRF52840_H__
