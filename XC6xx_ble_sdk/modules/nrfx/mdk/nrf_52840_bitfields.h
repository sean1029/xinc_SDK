/*

Copyright (c) 2010 - 2020, Nordic Semiconductor ASA

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form, except as embedded into a Nordic
   Semiconductor ASA integrated circuit in a product or a software update for
   such product, must reproduce the above copyright notice, this list of
   conditions and the following disclaimer in the documentation and/or other
   materials provided with the distribution.

3. Neither the name of Nordic Semiconductor ASA nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

4. This software, with or without modification, must only be used with a
   Nordic Semiconductor ASA integrated circuit.

5. Any software provided in binary form under this license must not be reverse
   engineered, decompiled, modified and/or disassembled.

THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef __NRF52840_BITS_H
#define __NRF52840_BITS_H

/*lint ++flb "Enter library region" */





/* Peripheral: ACL */
/* Description: Access control lists */










/* Peripheral: CLOCK */
/* Description: Clock control */


























/* Peripheral: GPIOTE */
/* Description: GPIO Tasks and Events */

/* Register: GPIOTE_TASKS_OUT */
/* Description: Description collection: Task for writing to pin specified in CONFIG[n].PSEL. Action on pin is configured in CONFIG[n].POLARITY. */

/* Bit 0 : Task for writing to pin specified in CONFIG[n].PSEL. Action on pin is configured in CONFIG[n].POLARITY. */
#define GPIOTE_TASKS_OUT_TASKS_OUT_Pos (0UL) /*!< Position of TASKS_OUT field. */
#define GPIOTE_TASKS_OUT_TASKS_OUT_Msk (0x1UL << GPIOTE_TASKS_OUT_TASKS_OUT_Pos) /*!< Bit mask of TASKS_OUT field. */
#define GPIOTE_TASKS_OUT_TASKS_OUT_Trigger (1UL) /*!< Trigger task */

/* Register: GPIOTE_TASKS_SET */
/* Description: Description collection: Task for writing to pin specified in CONFIG[n].PSEL. Action on pin is to set it high. */

/* Bit 0 : Task for writing to pin specified in CONFIG[n].PSEL. Action on pin is to set it high. */
#define GPIOTE_TASKS_SET_TASKS_SET_Pos (0UL) /*!< Position of TASKS_SET field. */
#define GPIOTE_TASKS_SET_TASKS_SET_Msk (0x1UL << GPIOTE_TASKS_SET_TASKS_SET_Pos) /*!< Bit mask of TASKS_SET field. */
#define GPIOTE_TASKS_SET_TASKS_SET_Trigger (1UL) /*!< Trigger task */

/* Register: GPIOTE_TASKS_CLR */
/* Description: Description collection: Task for writing to pin specified in CONFIG[n].PSEL. Action on pin is to set it low. */

/* Bit 0 : Task for writing to pin specified in CONFIG[n].PSEL. Action on pin is to set it low. */
#define GPIOTE_TASKS_CLR_TASKS_CLR_Pos (0UL) /*!< Position of TASKS_CLR field. */
#define GPIOTE_TASKS_CLR_TASKS_CLR_Msk (0x1UL << GPIOTE_TASKS_CLR_TASKS_CLR_Pos) /*!< Bit mask of TASKS_CLR field. */
#define GPIOTE_TASKS_CLR_TASKS_CLR_Trigger (1UL) /*!< Trigger task */

/* Register: GPIOTE_EVENTS_IN */
/* Description: Description collection: Event generated from pin specified in CONFIG[n].PSEL */

/* Bit 0 : Event generated from pin specified in CONFIG[n].PSEL */
#define GPIOTE_EVENTS_IN_EVENTS_IN_Pos (0UL) /*!< Position of EVENTS_IN field. */
#define GPIOTE_EVENTS_IN_EVENTS_IN_Msk (0x1UL << GPIOTE_EVENTS_IN_EVENTS_IN_Pos) /*!< Bit mask of EVENTS_IN field. */
#define GPIOTE_EVENTS_IN_EVENTS_IN_NotGenerated (0UL) /*!< Event not generated */
#define GPIOTE_EVENTS_IN_EVENTS_IN_Generated (1UL) /*!< Event generated */

/* Register: GPIOTE_EVENTS_PORT */
/* Description: Event generated from multiple input GPIO pins with SENSE mechanism enabled */

/* Bit 0 : Event generated from multiple input GPIO pins with SENSE mechanism enabled */
#define GPIOTE_EVENTS_PORT_EVENTS_PORT_Pos (0UL) /*!< Position of EVENTS_PORT field. */
#define GPIOTE_EVENTS_PORT_EVENTS_PORT_Msk (0x1UL << GPIOTE_EVENTS_PORT_EVENTS_PORT_Pos) /*!< Bit mask of EVENTS_PORT field. */
#define GPIOTE_EVENTS_PORT_EVENTS_PORT_NotGenerated (0UL) /*!< Event not generated */
#define GPIOTE_EVENTS_PORT_EVENTS_PORT_Generated (1UL) /*!< Event generated */

/* Register: GPIOTE_INTENSET */
/* Description: Enable interrupt */

/* Bit 31 : Write '1' to enable interrupt for event PORT */
#define GPIOTE_INTENSET_PORT_Pos (31UL) /*!< Position of PORT field. */
#define GPIOTE_INTENSET_PORT_Msk (0x1UL << GPIOTE_INTENSET_PORT_Pos) /*!< Bit mask of PORT field. */
#define GPIOTE_INTENSET_PORT_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENSET_PORT_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENSET_PORT_Set (1UL) /*!< Enable */

/* Bit 7 : Write '1' to enable interrupt for event IN[7] */
#define GPIOTE_INTENSET_IN7_Pos (7UL) /*!< Position of IN7 field. */
#define GPIOTE_INTENSET_IN7_Msk (0x1UL << GPIOTE_INTENSET_IN7_Pos) /*!< Bit mask of IN7 field. */
#define GPIOTE_INTENSET_IN7_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENSET_IN7_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENSET_IN7_Set (1UL) /*!< Enable */

/* Bit 6 : Write '1' to enable interrupt for event IN[6] */
#define GPIOTE_INTENSET_IN6_Pos (6UL) /*!< Position of IN6 field. */
#define GPIOTE_INTENSET_IN6_Msk (0x1UL << GPIOTE_INTENSET_IN6_Pos) /*!< Bit mask of IN6 field. */
#define GPIOTE_INTENSET_IN6_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENSET_IN6_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENSET_IN6_Set (1UL) /*!< Enable */

/* Bit 5 : Write '1' to enable interrupt for event IN[5] */
#define GPIOTE_INTENSET_IN5_Pos (5UL) /*!< Position of IN5 field. */
#define GPIOTE_INTENSET_IN5_Msk (0x1UL << GPIOTE_INTENSET_IN5_Pos) /*!< Bit mask of IN5 field. */
#define GPIOTE_INTENSET_IN5_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENSET_IN5_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENSET_IN5_Set (1UL) /*!< Enable */

/* Bit 4 : Write '1' to enable interrupt for event IN[4] */
#define GPIOTE_INTENSET_IN4_Pos (4UL) /*!< Position of IN4 field. */
#define GPIOTE_INTENSET_IN4_Msk (0x1UL << GPIOTE_INTENSET_IN4_Pos) /*!< Bit mask of IN4 field. */
#define GPIOTE_INTENSET_IN4_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENSET_IN4_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENSET_IN4_Set (1UL) /*!< Enable */

/* Bit 3 : Write '1' to enable interrupt for event IN[3] */
#define GPIOTE_INTENSET_IN3_Pos (3UL) /*!< Position of IN3 field. */
#define GPIOTE_INTENSET_IN3_Msk (0x1UL << GPIOTE_INTENSET_IN3_Pos) /*!< Bit mask of IN3 field. */
#define GPIOTE_INTENSET_IN3_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENSET_IN3_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENSET_IN3_Set (1UL) /*!< Enable */

/* Bit 2 : Write '1' to enable interrupt for event IN[2] */
#define GPIOTE_INTENSET_IN2_Pos (2UL) /*!< Position of IN2 field. */
#define GPIOTE_INTENSET_IN2_Msk (0x1UL << GPIOTE_INTENSET_IN2_Pos) /*!< Bit mask of IN2 field. */
#define GPIOTE_INTENSET_IN2_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENSET_IN2_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENSET_IN2_Set (1UL) /*!< Enable */

/* Bit 1 : Write '1' to enable interrupt for event IN[1] */
#define GPIOTE_INTENSET_IN1_Pos (1UL) /*!< Position of IN1 field. */
#define GPIOTE_INTENSET_IN1_Msk (0x1UL << GPIOTE_INTENSET_IN1_Pos) /*!< Bit mask of IN1 field. */
#define GPIOTE_INTENSET_IN1_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENSET_IN1_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENSET_IN1_Set (1UL) /*!< Enable */

/* Bit 0 : Write '1' to enable interrupt for event IN[0] */
#define GPIOTE_INTENSET_IN0_Pos (0UL) /*!< Position of IN0 field. */
#define GPIOTE_INTENSET_IN0_Msk (0x1UL << GPIOTE_INTENSET_IN0_Pos) /*!< Bit mask of IN0 field. */
#define GPIOTE_INTENSET_IN0_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENSET_IN0_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENSET_IN0_Set (1UL) /*!< Enable */

/* Register: GPIOTE_INTENCLR */
/* Description: Disable interrupt */

/* Bit 31 : Write '1' to disable interrupt for event PORT */
#define GPIOTE_INTENCLR_PORT_Pos (31UL) /*!< Position of PORT field. */
#define GPIOTE_INTENCLR_PORT_Msk (0x1UL << GPIOTE_INTENCLR_PORT_Pos) /*!< Bit mask of PORT field. */
#define GPIOTE_INTENCLR_PORT_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENCLR_PORT_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENCLR_PORT_Clear (1UL) /*!< Disable */

/* Bit 7 : Write '1' to disable interrupt for event IN[7] */
#define GPIOTE_INTENCLR_IN7_Pos (7UL) /*!< Position of IN7 field. */
#define GPIOTE_INTENCLR_IN7_Msk (0x1UL << GPIOTE_INTENCLR_IN7_Pos) /*!< Bit mask of IN7 field. */
#define GPIOTE_INTENCLR_IN7_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENCLR_IN7_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENCLR_IN7_Clear (1UL) /*!< Disable */

/* Bit 6 : Write '1' to disable interrupt for event IN[6] */
#define GPIOTE_INTENCLR_IN6_Pos (6UL) /*!< Position of IN6 field. */
#define GPIOTE_INTENCLR_IN6_Msk (0x1UL << GPIOTE_INTENCLR_IN6_Pos) /*!< Bit mask of IN6 field. */
#define GPIOTE_INTENCLR_IN6_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENCLR_IN6_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENCLR_IN6_Clear (1UL) /*!< Disable */

/* Bit 5 : Write '1' to disable interrupt for event IN[5] */
#define GPIOTE_INTENCLR_IN5_Pos (5UL) /*!< Position of IN5 field. */
#define GPIOTE_INTENCLR_IN5_Msk (0x1UL << GPIOTE_INTENCLR_IN5_Pos) /*!< Bit mask of IN5 field. */
#define GPIOTE_INTENCLR_IN5_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENCLR_IN5_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENCLR_IN5_Clear (1UL) /*!< Disable */

/* Bit 4 : Write '1' to disable interrupt for event IN[4] */
#define GPIOTE_INTENCLR_IN4_Pos (4UL) /*!< Position of IN4 field. */
#define GPIOTE_INTENCLR_IN4_Msk (0x1UL << GPIOTE_INTENCLR_IN4_Pos) /*!< Bit mask of IN4 field. */
#define GPIOTE_INTENCLR_IN4_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENCLR_IN4_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENCLR_IN4_Clear (1UL) /*!< Disable */

/* Bit 3 : Write '1' to disable interrupt for event IN[3] */
#define GPIOTE_INTENCLR_IN3_Pos (3UL) /*!< Position of IN3 field. */
#define GPIOTE_INTENCLR_IN3_Msk (0x1UL << GPIOTE_INTENCLR_IN3_Pos) /*!< Bit mask of IN3 field. */
#define GPIOTE_INTENCLR_IN3_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENCLR_IN3_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENCLR_IN3_Clear (1UL) /*!< Disable */

/* Bit 2 : Write '1' to disable interrupt for event IN[2] */
#define GPIOTE_INTENCLR_IN2_Pos (2UL) /*!< Position of IN2 field. */
#define GPIOTE_INTENCLR_IN2_Msk (0x1UL << GPIOTE_INTENCLR_IN2_Pos) /*!< Bit mask of IN2 field. */
#define GPIOTE_INTENCLR_IN2_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENCLR_IN2_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENCLR_IN2_Clear (1UL) /*!< Disable */

/* Bit 1 : Write '1' to disable interrupt for event IN[1] */
#define GPIOTE_INTENCLR_IN1_Pos (1UL) /*!< Position of IN1 field. */
#define GPIOTE_INTENCLR_IN1_Msk (0x1UL << GPIOTE_INTENCLR_IN1_Pos) /*!< Bit mask of IN1 field. */
#define GPIOTE_INTENCLR_IN1_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENCLR_IN1_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENCLR_IN1_Clear (1UL) /*!< Disable */

/* Bit 0 : Write '1' to disable interrupt for event IN[0] */
#define GPIOTE_INTENCLR_IN0_Pos (0UL) /*!< Position of IN0 field. */
#define GPIOTE_INTENCLR_IN0_Msk (0x1UL << GPIOTE_INTENCLR_IN0_Pos) /*!< Bit mask of IN0 field. */
#define GPIOTE_INTENCLR_IN0_Disabled (0UL) /*!< Read: Disabled */
#define GPIOTE_INTENCLR_IN0_Enabled (1UL) /*!< Read: Enabled */
#define GPIOTE_INTENCLR_IN0_Clear (1UL) /*!< Disable */

/* Register: GPIOTE_CONFIG */
/* Description: Description collection: Configuration for OUT[n], SET[n] and CLR[n] tasks and IN[n] event */

/* Bit 20 : When in task mode: Initial value of the output when the GPIOTE channel is configured. When in event mode: No effect. */
#define GPIOTE_CONFIG_OUTINIT_Pos (20UL) /*!< Position of OUTINIT field. */
#define GPIOTE_CONFIG_OUTINIT_Msk (0x1UL << GPIOTE_CONFIG_OUTINIT_Pos) /*!< Bit mask of OUTINIT field. */
#define GPIOTE_CONFIG_OUTINIT_Low (0UL) /*!< Task mode: Initial value of pin before task triggering is low */
#define GPIOTE_CONFIG_OUTINIT_High (1UL) /*!< Task mode: Initial value of pin before task triggering is high */

/* Bits 17..16 : When In task mode: Operation to be performed on output when OUT[n] task is triggered. When In event mode: Operation on input that shall trigger IN[n] event. */
#define GPIOTE_CONFIG_POLARITY_Pos (16UL) /*!< Position of POLARITY field. */
#define GPIOTE_CONFIG_POLARITY_Msk (0x3UL << GPIOTE_CONFIG_POLARITY_Pos) /*!< Bit mask of POLARITY field. */
#define GPIOTE_CONFIG_POLARITY_None (0UL) /*!< Task mode: No effect on pin from OUT[n] task. Event mode: no IN[n] event generated on pin activity. */
#define GPIOTE_CONFIG_POLARITY_LoToHi (1UL) /*!< Task mode: Set pin from OUT[n] task. Event mode: Generate IN[n] event when rising edge on pin. */
#define GPIOTE_CONFIG_POLARITY_HiToLo (2UL) /*!< Task mode: Clear pin from OUT[n] task. Event mode: Generate IN[n] event when falling edge on pin. */
#define GPIOTE_CONFIG_POLARITY_Toggle (3UL) /*!< Task mode: Toggle pin from OUT[n]. Event mode: Generate IN[n] when any change on pin. */

/* Bit 13 : Port number */
#define GPIOTE_CONFIG_PORT_Pos (13UL) /*!< Position of PORT field. */
#define GPIOTE_CONFIG_PORT_Msk (0x1UL << GPIOTE_CONFIG_PORT_Pos) /*!< Bit mask of PORT field. */

/* Bits 12..8 : GPIO number associated with SET[n], CLR[n] and OUT[n] tasks and IN[n] event */
#define GPIOTE_CONFIG_PSEL_Pos (8UL) /*!< Position of PSEL field. */
#define GPIOTE_CONFIG_PSEL_Msk (0x1FUL << GPIOTE_CONFIG_PSEL_Pos) /*!< Bit mask of PSEL field. */

/* Bits 1..0 : Mode */
#define GPIOTE_CONFIG_MODE_Pos (0UL) /*!< Position of MODE field. */
#define GPIOTE_CONFIG_MODE_Msk (0x3UL << GPIOTE_CONFIG_MODE_Pos) /*!< Bit mask of MODE field. */
#define GPIOTE_CONFIG_MODE_Disabled (0UL) /*!< Disabled. Pin specified by PSEL will not be acquired by the GPIOTE module. */
#define GPIOTE_CONFIG_MODE_Event (1UL) /*!< Event mode */
#define GPIOTE_CONFIG_MODE_Task (3UL) /*!< Task mode */


/* Peripheral: I2S */
/* Description: Inter-IC Sound */







/* Peripheral: NVMC */
/* Description: Non Volatile Memory Controller */

/* Register: NVMC_READY */
/* Description: Ready flag */







/* Peripheral: GPIO */
/* Description: GPIO Port 1 */

/* Register: GPIO_OUT */
/* Description: Write GPIO port */

/* Bit 31 : Pin 31 */
#define GPIO_OUT_PIN31_Pos (31UL) /*!< Position of PIN31 field. */
#define GPIO_OUT_PIN31_Msk (0x1UL << GPIO_OUT_PIN31_Pos) /*!< Bit mask of PIN31 field. */
#define GPIO_OUT_PIN31_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN31_High (1UL) /*!< Pin driver is high */

/* Bit 30 : Pin 30 */
#define GPIO_OUT_PIN30_Pos (30UL) /*!< Position of PIN30 field. */
#define GPIO_OUT_PIN30_Msk (0x1UL << GPIO_OUT_PIN30_Pos) /*!< Bit mask of PIN30 field. */
#define GPIO_OUT_PIN30_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN30_High (1UL) /*!< Pin driver is high */

/* Bit 29 : Pin 29 */
#define GPIO_OUT_PIN29_Pos (29UL) /*!< Position of PIN29 field. */
#define GPIO_OUT_PIN29_Msk (0x1UL << GPIO_OUT_PIN29_Pos) /*!< Bit mask of PIN29 field. */
#define GPIO_OUT_PIN29_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN29_High (1UL) /*!< Pin driver is high */

/* Bit 28 : Pin 28 */
#define GPIO_OUT_PIN28_Pos (28UL) /*!< Position of PIN28 field. */
#define GPIO_OUT_PIN28_Msk (0x1UL << GPIO_OUT_PIN28_Pos) /*!< Bit mask of PIN28 field. */
#define GPIO_OUT_PIN28_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN28_High (1UL) /*!< Pin driver is high */

/* Bit 27 : Pin 27 */
#define GPIO_OUT_PIN27_Pos (27UL) /*!< Position of PIN27 field. */
#define GPIO_OUT_PIN27_Msk (0x1UL << GPIO_OUT_PIN27_Pos) /*!< Bit mask of PIN27 field. */
#define GPIO_OUT_PIN27_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN27_High (1UL) /*!< Pin driver is high */

/* Bit 26 : Pin 26 */
#define GPIO_OUT_PIN26_Pos (26UL) /*!< Position of PIN26 field. */
#define GPIO_OUT_PIN26_Msk (0x1UL << GPIO_OUT_PIN26_Pos) /*!< Bit mask of PIN26 field. */
#define GPIO_OUT_PIN26_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN26_High (1UL) /*!< Pin driver is high */

/* Bit 25 : Pin 25 */
#define GPIO_OUT_PIN25_Pos (25UL) /*!< Position of PIN25 field. */
#define GPIO_OUT_PIN25_Msk (0x1UL << GPIO_OUT_PIN25_Pos) /*!< Bit mask of PIN25 field. */
#define GPIO_OUT_PIN25_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN25_High (1UL) /*!< Pin driver is high */

/* Bit 24 : Pin 24 */
#define GPIO_OUT_PIN24_Pos (24UL) /*!< Position of PIN24 field. */
#define GPIO_OUT_PIN24_Msk (0x1UL << GPIO_OUT_PIN24_Pos) /*!< Bit mask of PIN24 field. */
#define GPIO_OUT_PIN24_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN24_High (1UL) /*!< Pin driver is high */

/* Bit 23 : Pin 23 */
#define GPIO_OUT_PIN23_Pos (23UL) /*!< Position of PIN23 field. */
#define GPIO_OUT_PIN23_Msk (0x1UL << GPIO_OUT_PIN23_Pos) /*!< Bit mask of PIN23 field. */
#define GPIO_OUT_PIN23_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN23_High (1UL) /*!< Pin driver is high */

/* Bit 22 : Pin 22 */
#define GPIO_OUT_PIN22_Pos (22UL) /*!< Position of PIN22 field. */
#define GPIO_OUT_PIN22_Msk (0x1UL << GPIO_OUT_PIN22_Pos) /*!< Bit mask of PIN22 field. */
#define GPIO_OUT_PIN22_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN22_High (1UL) /*!< Pin driver is high */

/* Bit 21 : Pin 21 */
#define GPIO_OUT_PIN21_Pos (21UL) /*!< Position of PIN21 field. */
#define GPIO_OUT_PIN21_Msk (0x1UL << GPIO_OUT_PIN21_Pos) /*!< Bit mask of PIN21 field. */
#define GPIO_OUT_PIN21_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN21_High (1UL) /*!< Pin driver is high */

/* Bit 20 : Pin 20 */
#define GPIO_OUT_PIN20_Pos (20UL) /*!< Position of PIN20 field. */
#define GPIO_OUT_PIN20_Msk (0x1UL << GPIO_OUT_PIN20_Pos) /*!< Bit mask of PIN20 field. */
#define GPIO_OUT_PIN20_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN20_High (1UL) /*!< Pin driver is high */

/* Bit 19 : Pin 19 */
#define GPIO_OUT_PIN19_Pos (19UL) /*!< Position of PIN19 field. */
#define GPIO_OUT_PIN19_Msk (0x1UL << GPIO_OUT_PIN19_Pos) /*!< Bit mask of PIN19 field. */
#define GPIO_OUT_PIN19_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN19_High (1UL) /*!< Pin driver is high */

/* Bit 18 : Pin 18 */
#define GPIO_OUT_PIN18_Pos (18UL) /*!< Position of PIN18 field. */
#define GPIO_OUT_PIN18_Msk (0x1UL << GPIO_OUT_PIN18_Pos) /*!< Bit mask of PIN18 field. */
#define GPIO_OUT_PIN18_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN18_High (1UL) /*!< Pin driver is high */

/* Bit 17 : Pin 17 */
#define GPIO_OUT_PIN17_Pos (17UL) /*!< Position of PIN17 field. */
#define GPIO_OUT_PIN17_Msk (0x1UL << GPIO_OUT_PIN17_Pos) /*!< Bit mask of PIN17 field. */
#define GPIO_OUT_PIN17_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN17_High (1UL) /*!< Pin driver is high */

/* Bit 16 : Pin 16 */
#define GPIO_OUT_PIN16_Pos (16UL) /*!< Position of PIN16 field. */
#define GPIO_OUT_PIN16_Msk (0x1UL << GPIO_OUT_PIN16_Pos) /*!< Bit mask of PIN16 field. */
#define GPIO_OUT_PIN16_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN16_High (1UL) /*!< Pin driver is high */

/* Bit 15 : Pin 15 */
#define GPIO_OUT_PIN15_Pos (15UL) /*!< Position of PIN15 field. */
#define GPIO_OUT_PIN15_Msk (0x1UL << GPIO_OUT_PIN15_Pos) /*!< Bit mask of PIN15 field. */
#define GPIO_OUT_PIN15_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN15_High (1UL) /*!< Pin driver is high */

/* Bit 14 : Pin 14 */
#define GPIO_OUT_PIN14_Pos (14UL) /*!< Position of PIN14 field. */
#define GPIO_OUT_PIN14_Msk (0x1UL << GPIO_OUT_PIN14_Pos) /*!< Bit mask of PIN14 field. */
#define GPIO_OUT_PIN14_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN14_High (1UL) /*!< Pin driver is high */

/* Bit 13 : Pin 13 */
#define GPIO_OUT_PIN13_Pos (13UL) /*!< Position of PIN13 field. */
#define GPIO_OUT_PIN13_Msk (0x1UL << GPIO_OUT_PIN13_Pos) /*!< Bit mask of PIN13 field. */
#define GPIO_OUT_PIN13_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN13_High (1UL) /*!< Pin driver is high */

/* Bit 12 : Pin 12 */
#define GPIO_OUT_PIN12_Pos (12UL) /*!< Position of PIN12 field. */
#define GPIO_OUT_PIN12_Msk (0x1UL << GPIO_OUT_PIN12_Pos) /*!< Bit mask of PIN12 field. */
#define GPIO_OUT_PIN12_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN12_High (1UL) /*!< Pin driver is high */

/* Bit 11 : Pin 11 */
#define GPIO_OUT_PIN11_Pos (11UL) /*!< Position of PIN11 field. */
#define GPIO_OUT_PIN11_Msk (0x1UL << GPIO_OUT_PIN11_Pos) /*!< Bit mask of PIN11 field. */
#define GPIO_OUT_PIN11_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN11_High (1UL) /*!< Pin driver is high */

/* Bit 10 : Pin 10 */
#define GPIO_OUT_PIN10_Pos (10UL) /*!< Position of PIN10 field. */
#define GPIO_OUT_PIN10_Msk (0x1UL << GPIO_OUT_PIN10_Pos) /*!< Bit mask of PIN10 field. */
#define GPIO_OUT_PIN10_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN10_High (1UL) /*!< Pin driver is high */

/* Bit 9 : Pin 9 */
#define GPIO_OUT_PIN9_Pos (9UL) /*!< Position of PIN9 field. */
#define GPIO_OUT_PIN9_Msk (0x1UL << GPIO_OUT_PIN9_Pos) /*!< Bit mask of PIN9 field. */
#define GPIO_OUT_PIN9_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN9_High (1UL) /*!< Pin driver is high */

/* Bit 8 : Pin 8 */
#define GPIO_OUT_PIN8_Pos (8UL) /*!< Position of PIN8 field. */
#define GPIO_OUT_PIN8_Msk (0x1UL << GPIO_OUT_PIN8_Pos) /*!< Bit mask of PIN8 field. */
#define GPIO_OUT_PIN8_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN8_High (1UL) /*!< Pin driver is high */

/* Bit 7 : Pin 7 */
#define GPIO_OUT_PIN7_Pos (7UL) /*!< Position of PIN7 field. */
#define GPIO_OUT_PIN7_Msk (0x1UL << GPIO_OUT_PIN7_Pos) /*!< Bit mask of PIN7 field. */
#define GPIO_OUT_PIN7_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN7_High (1UL) /*!< Pin driver is high */

/* Bit 6 : Pin 6 */
#define GPIO_OUT_PIN6_Pos (6UL) /*!< Position of PIN6 field. */
#define GPIO_OUT_PIN6_Msk (0x1UL << GPIO_OUT_PIN6_Pos) /*!< Bit mask of PIN6 field. */
#define GPIO_OUT_PIN6_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN6_High (1UL) /*!< Pin driver is high */

/* Bit 5 : Pin 5 */
#define GPIO_OUT_PIN5_Pos (5UL) /*!< Position of PIN5 field. */
#define GPIO_OUT_PIN5_Msk (0x1UL << GPIO_OUT_PIN5_Pos) /*!< Bit mask of PIN5 field. */
#define GPIO_OUT_PIN5_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN5_High (1UL) /*!< Pin driver is high */

/* Bit 4 : Pin 4 */
#define GPIO_OUT_PIN4_Pos (4UL) /*!< Position of PIN4 field. */
#define GPIO_OUT_PIN4_Msk (0x1UL << GPIO_OUT_PIN4_Pos) /*!< Bit mask of PIN4 field. */
#define GPIO_OUT_PIN4_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN4_High (1UL) /*!< Pin driver is high */

/* Bit 3 : Pin 3 */
#define GPIO_OUT_PIN3_Pos (3UL) /*!< Position of PIN3 field. */
#define GPIO_OUT_PIN3_Msk (0x1UL << GPIO_OUT_PIN3_Pos) /*!< Bit mask of PIN3 field. */
#define GPIO_OUT_PIN3_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN3_High (1UL) /*!< Pin driver is high */

/* Bit 2 : Pin 2 */
#define GPIO_OUT_PIN2_Pos (2UL) /*!< Position of PIN2 field. */
#define GPIO_OUT_PIN2_Msk (0x1UL << GPIO_OUT_PIN2_Pos) /*!< Bit mask of PIN2 field. */
#define GPIO_OUT_PIN2_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN2_High (1UL) /*!< Pin driver is high */

/* Bit 1 : Pin 1 */
#define GPIO_OUT_PIN1_Pos (1UL) /*!< Position of PIN1 field. */
#define GPIO_OUT_PIN1_Msk (0x1UL << GPIO_OUT_PIN1_Pos) /*!< Bit mask of PIN1 field. */
#define GPIO_OUT_PIN1_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN1_High (1UL) /*!< Pin driver is high */

/* Bit 0 : Pin 0 */
#define GPIO_OUT_PIN0_Pos (0UL) /*!< Position of PIN0 field. */
#define GPIO_OUT_PIN0_Msk (0x1UL << GPIO_OUT_PIN0_Pos) /*!< Bit mask of PIN0 field. */
#define GPIO_OUT_PIN0_Low (0UL) /*!< Pin driver is low */
#define GPIO_OUT_PIN0_High (1UL) /*!< Pin driver is high */

/* Register: GPIO_OUTSET */
/* Description: Set individual bits in GPIO port */

/* Bit 31 : Pin 31 */
#define GPIO_OUTSET_PIN31_Pos (31UL) /*!< Position of PIN31 field. */
#define GPIO_OUTSET_PIN31_Msk (0x1UL << GPIO_OUTSET_PIN31_Pos) /*!< Bit mask of PIN31 field. */
#define GPIO_OUTSET_PIN31_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN31_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN31_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 30 : Pin 30 */
#define GPIO_OUTSET_PIN30_Pos (30UL) /*!< Position of PIN30 field. */
#define GPIO_OUTSET_PIN30_Msk (0x1UL << GPIO_OUTSET_PIN30_Pos) /*!< Bit mask of PIN30 field. */
#define GPIO_OUTSET_PIN30_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN30_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN30_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 29 : Pin 29 */
#define GPIO_OUTSET_PIN29_Pos (29UL) /*!< Position of PIN29 field. */
#define GPIO_OUTSET_PIN29_Msk (0x1UL << GPIO_OUTSET_PIN29_Pos) /*!< Bit mask of PIN29 field. */
#define GPIO_OUTSET_PIN29_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN29_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN29_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 28 : Pin 28 */
#define GPIO_OUTSET_PIN28_Pos (28UL) /*!< Position of PIN28 field. */
#define GPIO_OUTSET_PIN28_Msk (0x1UL << GPIO_OUTSET_PIN28_Pos) /*!< Bit mask of PIN28 field. */
#define GPIO_OUTSET_PIN28_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN28_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN28_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 27 : Pin 27 */
#define GPIO_OUTSET_PIN27_Pos (27UL) /*!< Position of PIN27 field. */
#define GPIO_OUTSET_PIN27_Msk (0x1UL << GPIO_OUTSET_PIN27_Pos) /*!< Bit mask of PIN27 field. */
#define GPIO_OUTSET_PIN27_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN27_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN27_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 26 : Pin 26 */
#define GPIO_OUTSET_PIN26_Pos (26UL) /*!< Position of PIN26 field. */
#define GPIO_OUTSET_PIN26_Msk (0x1UL << GPIO_OUTSET_PIN26_Pos) /*!< Bit mask of PIN26 field. */
#define GPIO_OUTSET_PIN26_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN26_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN26_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 25 : Pin 25 */
#define GPIO_OUTSET_PIN25_Pos (25UL) /*!< Position of PIN25 field. */
#define GPIO_OUTSET_PIN25_Msk (0x1UL << GPIO_OUTSET_PIN25_Pos) /*!< Bit mask of PIN25 field. */
#define GPIO_OUTSET_PIN25_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN25_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN25_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 24 : Pin 24 */
#define GPIO_OUTSET_PIN24_Pos (24UL) /*!< Position of PIN24 field. */
#define GPIO_OUTSET_PIN24_Msk (0x1UL << GPIO_OUTSET_PIN24_Pos) /*!< Bit mask of PIN24 field. */
#define GPIO_OUTSET_PIN24_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN24_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN24_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 23 : Pin 23 */
#define GPIO_OUTSET_PIN23_Pos (23UL) /*!< Position of PIN23 field. */
#define GPIO_OUTSET_PIN23_Msk (0x1UL << GPIO_OUTSET_PIN23_Pos) /*!< Bit mask of PIN23 field. */
#define GPIO_OUTSET_PIN23_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN23_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN23_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 22 : Pin 22 */
#define GPIO_OUTSET_PIN22_Pos (22UL) /*!< Position of PIN22 field. */
#define GPIO_OUTSET_PIN22_Msk (0x1UL << GPIO_OUTSET_PIN22_Pos) /*!< Bit mask of PIN22 field. */
#define GPIO_OUTSET_PIN22_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN22_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN22_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 21 : Pin 21 */
#define GPIO_OUTSET_PIN21_Pos (21UL) /*!< Position of PIN21 field. */
#define GPIO_OUTSET_PIN21_Msk (0x1UL << GPIO_OUTSET_PIN21_Pos) /*!< Bit mask of PIN21 field. */
#define GPIO_OUTSET_PIN21_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN21_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN21_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 20 : Pin 20 */
#define GPIO_OUTSET_PIN20_Pos (20UL) /*!< Position of PIN20 field. */
#define GPIO_OUTSET_PIN20_Msk (0x1UL << GPIO_OUTSET_PIN20_Pos) /*!< Bit mask of PIN20 field. */
#define GPIO_OUTSET_PIN20_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN20_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN20_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 19 : Pin 19 */
#define GPIO_OUTSET_PIN19_Pos (19UL) /*!< Position of PIN19 field. */
#define GPIO_OUTSET_PIN19_Msk (0x1UL << GPIO_OUTSET_PIN19_Pos) /*!< Bit mask of PIN19 field. */
#define GPIO_OUTSET_PIN19_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN19_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN19_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 18 : Pin 18 */
#define GPIO_OUTSET_PIN18_Pos (18UL) /*!< Position of PIN18 field. */
#define GPIO_OUTSET_PIN18_Msk (0x1UL << GPIO_OUTSET_PIN18_Pos) /*!< Bit mask of PIN18 field. */
#define GPIO_OUTSET_PIN18_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN18_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN18_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 17 : Pin 17 */
#define GPIO_OUTSET_PIN17_Pos (17UL) /*!< Position of PIN17 field. */
#define GPIO_OUTSET_PIN17_Msk (0x1UL << GPIO_OUTSET_PIN17_Pos) /*!< Bit mask of PIN17 field. */
#define GPIO_OUTSET_PIN17_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN17_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN17_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 16 : Pin 16 */
#define GPIO_OUTSET_PIN16_Pos (16UL) /*!< Position of PIN16 field. */
#define GPIO_OUTSET_PIN16_Msk (0x1UL << GPIO_OUTSET_PIN16_Pos) /*!< Bit mask of PIN16 field. */
#define GPIO_OUTSET_PIN16_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN16_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN16_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 15 : Pin 15 */
#define GPIO_OUTSET_PIN15_Pos (15UL) /*!< Position of PIN15 field. */
#define GPIO_OUTSET_PIN15_Msk (0x1UL << GPIO_OUTSET_PIN15_Pos) /*!< Bit mask of PIN15 field. */
#define GPIO_OUTSET_PIN15_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN15_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN15_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 14 : Pin 14 */
#define GPIO_OUTSET_PIN14_Pos (14UL) /*!< Position of PIN14 field. */
#define GPIO_OUTSET_PIN14_Msk (0x1UL << GPIO_OUTSET_PIN14_Pos) /*!< Bit mask of PIN14 field. */
#define GPIO_OUTSET_PIN14_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN14_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN14_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 13 : Pin 13 */
#define GPIO_OUTSET_PIN13_Pos (13UL) /*!< Position of PIN13 field. */
#define GPIO_OUTSET_PIN13_Msk (0x1UL << GPIO_OUTSET_PIN13_Pos) /*!< Bit mask of PIN13 field. */
#define GPIO_OUTSET_PIN13_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN13_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN13_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 12 : Pin 12 */
#define GPIO_OUTSET_PIN12_Pos (12UL) /*!< Position of PIN12 field. */
#define GPIO_OUTSET_PIN12_Msk (0x1UL << GPIO_OUTSET_PIN12_Pos) /*!< Bit mask of PIN12 field. */
#define GPIO_OUTSET_PIN12_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN12_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN12_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 11 : Pin 11 */
#define GPIO_OUTSET_PIN11_Pos (11UL) /*!< Position of PIN11 field. */
#define GPIO_OUTSET_PIN11_Msk (0x1UL << GPIO_OUTSET_PIN11_Pos) /*!< Bit mask of PIN11 field. */
#define GPIO_OUTSET_PIN11_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN11_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN11_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 10 : Pin 10 */
#define GPIO_OUTSET_PIN10_Pos (10UL) /*!< Position of PIN10 field. */
#define GPIO_OUTSET_PIN10_Msk (0x1UL << GPIO_OUTSET_PIN10_Pos) /*!< Bit mask of PIN10 field. */
#define GPIO_OUTSET_PIN10_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN10_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN10_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 9 : Pin 9 */
#define GPIO_OUTSET_PIN9_Pos (9UL) /*!< Position of PIN9 field. */
#define GPIO_OUTSET_PIN9_Msk (0x1UL << GPIO_OUTSET_PIN9_Pos) /*!< Bit mask of PIN9 field. */
#define GPIO_OUTSET_PIN9_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN9_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN9_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 8 : Pin 8 */
#define GPIO_OUTSET_PIN8_Pos (8UL) /*!< Position of PIN8 field. */
#define GPIO_OUTSET_PIN8_Msk (0x1UL << GPIO_OUTSET_PIN8_Pos) /*!< Bit mask of PIN8 field. */
#define GPIO_OUTSET_PIN8_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN8_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN8_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 7 : Pin 7 */
#define GPIO_OUTSET_PIN7_Pos (7UL) /*!< Position of PIN7 field. */
#define GPIO_OUTSET_PIN7_Msk (0x1UL << GPIO_OUTSET_PIN7_Pos) /*!< Bit mask of PIN7 field. */
#define GPIO_OUTSET_PIN7_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN7_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN7_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 6 : Pin 6 */
#define GPIO_OUTSET_PIN6_Pos (6UL) /*!< Position of PIN6 field. */
#define GPIO_OUTSET_PIN6_Msk (0x1UL << GPIO_OUTSET_PIN6_Pos) /*!< Bit mask of PIN6 field. */
#define GPIO_OUTSET_PIN6_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN6_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN6_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 5 : Pin 5 */
#define GPIO_OUTSET_PIN5_Pos (5UL) /*!< Position of PIN5 field. */
#define GPIO_OUTSET_PIN5_Msk (0x1UL << GPIO_OUTSET_PIN5_Pos) /*!< Bit mask of PIN5 field. */
#define GPIO_OUTSET_PIN5_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN5_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN5_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 4 : Pin 4 */
#define GPIO_OUTSET_PIN4_Pos (4UL) /*!< Position of PIN4 field. */
#define GPIO_OUTSET_PIN4_Msk (0x1UL << GPIO_OUTSET_PIN4_Pos) /*!< Bit mask of PIN4 field. */
#define GPIO_OUTSET_PIN4_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN4_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN4_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 3 : Pin 3 */
#define GPIO_OUTSET_PIN3_Pos (3UL) /*!< Position of PIN3 field. */
#define GPIO_OUTSET_PIN3_Msk (0x1UL << GPIO_OUTSET_PIN3_Pos) /*!< Bit mask of PIN3 field. */
#define GPIO_OUTSET_PIN3_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN3_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN3_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 2 : Pin 2 */
#define GPIO_OUTSET_PIN2_Pos (2UL) /*!< Position of PIN2 field. */
#define GPIO_OUTSET_PIN2_Msk (0x1UL << GPIO_OUTSET_PIN2_Pos) /*!< Bit mask of PIN2 field. */
#define GPIO_OUTSET_PIN2_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN2_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN2_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 1 : Pin 1 */
#define GPIO_OUTSET_PIN1_Pos (1UL) /*!< Position of PIN1 field. */
#define GPIO_OUTSET_PIN1_Msk (0x1UL << GPIO_OUTSET_PIN1_Pos) /*!< Bit mask of PIN1 field. */
#define GPIO_OUTSET_PIN1_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN1_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN1_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Bit 0 : Pin 0 */
#define GPIO_OUTSET_PIN0_Pos (0UL) /*!< Position of PIN0 field. */
#define GPIO_OUTSET_PIN0_Msk (0x1UL << GPIO_OUTSET_PIN0_Pos) /*!< Bit mask of PIN0 field. */
#define GPIO_OUTSET_PIN0_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTSET_PIN0_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTSET_PIN0_Set (1UL) /*!< Write: writing a '1' sets the pin high; writing a '0' has no effect */

/* Register: GPIO_OUTCLR */
/* Description: Clear individual bits in GPIO port */

/* Bit 31 : Pin 31 */
#define GPIO_OUTCLR_PIN31_Pos (31UL) /*!< Position of PIN31 field. */
#define GPIO_OUTCLR_PIN31_Msk (0x1UL << GPIO_OUTCLR_PIN31_Pos) /*!< Bit mask of PIN31 field. */
#define GPIO_OUTCLR_PIN31_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN31_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN31_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 30 : Pin 30 */
#define GPIO_OUTCLR_PIN30_Pos (30UL) /*!< Position of PIN30 field. */
#define GPIO_OUTCLR_PIN30_Msk (0x1UL << GPIO_OUTCLR_PIN30_Pos) /*!< Bit mask of PIN30 field. */
#define GPIO_OUTCLR_PIN30_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN30_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN30_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 29 : Pin 29 */
#define GPIO_OUTCLR_PIN29_Pos (29UL) /*!< Position of PIN29 field. */
#define GPIO_OUTCLR_PIN29_Msk (0x1UL << GPIO_OUTCLR_PIN29_Pos) /*!< Bit mask of PIN29 field. */
#define GPIO_OUTCLR_PIN29_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN29_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN29_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 28 : Pin 28 */
#define GPIO_OUTCLR_PIN28_Pos (28UL) /*!< Position of PIN28 field. */
#define GPIO_OUTCLR_PIN28_Msk (0x1UL << GPIO_OUTCLR_PIN28_Pos) /*!< Bit mask of PIN28 field. */
#define GPIO_OUTCLR_PIN28_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN28_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN28_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 27 : Pin 27 */
#define GPIO_OUTCLR_PIN27_Pos (27UL) /*!< Position of PIN27 field. */
#define GPIO_OUTCLR_PIN27_Msk (0x1UL << GPIO_OUTCLR_PIN27_Pos) /*!< Bit mask of PIN27 field. */
#define GPIO_OUTCLR_PIN27_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN27_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN27_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 26 : Pin 26 */
#define GPIO_OUTCLR_PIN26_Pos (26UL) /*!< Position of PIN26 field. */
#define GPIO_OUTCLR_PIN26_Msk (0x1UL << GPIO_OUTCLR_PIN26_Pos) /*!< Bit mask of PIN26 field. */
#define GPIO_OUTCLR_PIN26_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN26_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN26_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 25 : Pin 25 */
#define GPIO_OUTCLR_PIN25_Pos (25UL) /*!< Position of PIN25 field. */
#define GPIO_OUTCLR_PIN25_Msk (0x1UL << GPIO_OUTCLR_PIN25_Pos) /*!< Bit mask of PIN25 field. */
#define GPIO_OUTCLR_PIN25_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN25_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN25_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 24 : Pin 24 */
#define GPIO_OUTCLR_PIN24_Pos (24UL) /*!< Position of PIN24 field. */
#define GPIO_OUTCLR_PIN24_Msk (0x1UL << GPIO_OUTCLR_PIN24_Pos) /*!< Bit mask of PIN24 field. */
#define GPIO_OUTCLR_PIN24_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN24_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN24_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 23 : Pin 23 */
#define GPIO_OUTCLR_PIN23_Pos (23UL) /*!< Position of PIN23 field. */
#define GPIO_OUTCLR_PIN23_Msk (0x1UL << GPIO_OUTCLR_PIN23_Pos) /*!< Bit mask of PIN23 field. */
#define GPIO_OUTCLR_PIN23_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN23_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN23_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 22 : Pin 22 */
#define GPIO_OUTCLR_PIN22_Pos (22UL) /*!< Position of PIN22 field. */
#define GPIO_OUTCLR_PIN22_Msk (0x1UL << GPIO_OUTCLR_PIN22_Pos) /*!< Bit mask of PIN22 field. */
#define GPIO_OUTCLR_PIN22_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN22_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN22_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 21 : Pin 21 */
#define GPIO_OUTCLR_PIN21_Pos (21UL) /*!< Position of PIN21 field. */
#define GPIO_OUTCLR_PIN21_Msk (0x1UL << GPIO_OUTCLR_PIN21_Pos) /*!< Bit mask of PIN21 field. */
#define GPIO_OUTCLR_PIN21_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN21_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN21_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 20 : Pin 20 */
#define GPIO_OUTCLR_PIN20_Pos (20UL) /*!< Position of PIN20 field. */
#define GPIO_OUTCLR_PIN20_Msk (0x1UL << GPIO_OUTCLR_PIN20_Pos) /*!< Bit mask of PIN20 field. */
#define GPIO_OUTCLR_PIN20_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN20_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN20_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 19 : Pin 19 */
#define GPIO_OUTCLR_PIN19_Pos (19UL) /*!< Position of PIN19 field. */
#define GPIO_OUTCLR_PIN19_Msk (0x1UL << GPIO_OUTCLR_PIN19_Pos) /*!< Bit mask of PIN19 field. */
#define GPIO_OUTCLR_PIN19_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN19_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN19_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 18 : Pin 18 */
#define GPIO_OUTCLR_PIN18_Pos (18UL) /*!< Position of PIN18 field. */
#define GPIO_OUTCLR_PIN18_Msk (0x1UL << GPIO_OUTCLR_PIN18_Pos) /*!< Bit mask of PIN18 field. */
#define GPIO_OUTCLR_PIN18_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN18_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN18_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 17 : Pin 17 */
#define GPIO_OUTCLR_PIN17_Pos (17UL) /*!< Position of PIN17 field. */
#define GPIO_OUTCLR_PIN17_Msk (0x1UL << GPIO_OUTCLR_PIN17_Pos) /*!< Bit mask of PIN17 field. */
#define GPIO_OUTCLR_PIN17_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN17_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN17_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 16 : Pin 16 */
#define GPIO_OUTCLR_PIN16_Pos (16UL) /*!< Position of PIN16 field. */
#define GPIO_OUTCLR_PIN16_Msk (0x1UL << GPIO_OUTCLR_PIN16_Pos) /*!< Bit mask of PIN16 field. */
#define GPIO_OUTCLR_PIN16_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN16_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN16_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 15 : Pin 15 */
#define GPIO_OUTCLR_PIN15_Pos (15UL) /*!< Position of PIN15 field. */
#define GPIO_OUTCLR_PIN15_Msk (0x1UL << GPIO_OUTCLR_PIN15_Pos) /*!< Bit mask of PIN15 field. */
#define GPIO_OUTCLR_PIN15_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN15_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN15_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 14 : Pin 14 */
#define GPIO_OUTCLR_PIN14_Pos (14UL) /*!< Position of PIN14 field. */
#define GPIO_OUTCLR_PIN14_Msk (0x1UL << GPIO_OUTCLR_PIN14_Pos) /*!< Bit mask of PIN14 field. */
#define GPIO_OUTCLR_PIN14_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN14_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN14_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 13 : Pin 13 */
#define GPIO_OUTCLR_PIN13_Pos (13UL) /*!< Position of PIN13 field. */
#define GPIO_OUTCLR_PIN13_Msk (0x1UL << GPIO_OUTCLR_PIN13_Pos) /*!< Bit mask of PIN13 field. */
#define GPIO_OUTCLR_PIN13_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN13_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN13_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 12 : Pin 12 */
#define GPIO_OUTCLR_PIN12_Pos (12UL) /*!< Position of PIN12 field. */
#define GPIO_OUTCLR_PIN12_Msk (0x1UL << GPIO_OUTCLR_PIN12_Pos) /*!< Bit mask of PIN12 field. */
#define GPIO_OUTCLR_PIN12_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN12_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN12_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 11 : Pin 11 */
#define GPIO_OUTCLR_PIN11_Pos (11UL) /*!< Position of PIN11 field. */
#define GPIO_OUTCLR_PIN11_Msk (0x1UL << GPIO_OUTCLR_PIN11_Pos) /*!< Bit mask of PIN11 field. */
#define GPIO_OUTCLR_PIN11_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN11_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN11_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 10 : Pin 10 */
#define GPIO_OUTCLR_PIN10_Pos (10UL) /*!< Position of PIN10 field. */
#define GPIO_OUTCLR_PIN10_Msk (0x1UL << GPIO_OUTCLR_PIN10_Pos) /*!< Bit mask of PIN10 field. */
#define GPIO_OUTCLR_PIN10_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN10_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN10_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 9 : Pin 9 */
#define GPIO_OUTCLR_PIN9_Pos (9UL) /*!< Position of PIN9 field. */
#define GPIO_OUTCLR_PIN9_Msk (0x1UL << GPIO_OUTCLR_PIN9_Pos) /*!< Bit mask of PIN9 field. */
#define GPIO_OUTCLR_PIN9_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN9_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN9_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 8 : Pin 8 */
#define GPIO_OUTCLR_PIN8_Pos (8UL) /*!< Position of PIN8 field. */
#define GPIO_OUTCLR_PIN8_Msk (0x1UL << GPIO_OUTCLR_PIN8_Pos) /*!< Bit mask of PIN8 field. */
#define GPIO_OUTCLR_PIN8_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN8_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN8_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 7 : Pin 7 */
#define GPIO_OUTCLR_PIN7_Pos (7UL) /*!< Position of PIN7 field. */
#define GPIO_OUTCLR_PIN7_Msk (0x1UL << GPIO_OUTCLR_PIN7_Pos) /*!< Bit mask of PIN7 field. */
#define GPIO_OUTCLR_PIN7_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN7_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN7_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 6 : Pin 6 */
#define GPIO_OUTCLR_PIN6_Pos (6UL) /*!< Position of PIN6 field. */
#define GPIO_OUTCLR_PIN6_Msk (0x1UL << GPIO_OUTCLR_PIN6_Pos) /*!< Bit mask of PIN6 field. */
#define GPIO_OUTCLR_PIN6_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN6_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN6_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 5 : Pin 5 */
#define GPIO_OUTCLR_PIN5_Pos (5UL) /*!< Position of PIN5 field. */
#define GPIO_OUTCLR_PIN5_Msk (0x1UL << GPIO_OUTCLR_PIN5_Pos) /*!< Bit mask of PIN5 field. */
#define GPIO_OUTCLR_PIN5_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN5_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN5_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 4 : Pin 4 */
#define GPIO_OUTCLR_PIN4_Pos (4UL) /*!< Position of PIN4 field. */
#define GPIO_OUTCLR_PIN4_Msk (0x1UL << GPIO_OUTCLR_PIN4_Pos) /*!< Bit mask of PIN4 field. */
#define GPIO_OUTCLR_PIN4_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN4_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN4_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 3 : Pin 3 */
#define GPIO_OUTCLR_PIN3_Pos (3UL) /*!< Position of PIN3 field. */
#define GPIO_OUTCLR_PIN3_Msk (0x1UL << GPIO_OUTCLR_PIN3_Pos) /*!< Bit mask of PIN3 field. */
#define GPIO_OUTCLR_PIN3_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN3_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN3_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 2 : Pin 2 */
#define GPIO_OUTCLR_PIN2_Pos (2UL) /*!< Position of PIN2 field. */
#define GPIO_OUTCLR_PIN2_Msk (0x1UL << GPIO_OUTCLR_PIN2_Pos) /*!< Bit mask of PIN2 field. */
#define GPIO_OUTCLR_PIN2_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN2_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN2_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 1 : Pin 1 */
#define GPIO_OUTCLR_PIN1_Pos (1UL) /*!< Position of PIN1 field. */
#define GPIO_OUTCLR_PIN1_Msk (0x1UL << GPIO_OUTCLR_PIN1_Pos) /*!< Bit mask of PIN1 field. */
#define GPIO_OUTCLR_PIN1_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN1_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN1_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Bit 0 : Pin 0 */
#define GPIO_OUTCLR_PIN0_Pos (0UL) /*!< Position of PIN0 field. */
#define GPIO_OUTCLR_PIN0_Msk (0x1UL << GPIO_OUTCLR_PIN0_Pos) /*!< Bit mask of PIN0 field. */
#define GPIO_OUTCLR_PIN0_Low (0UL) /*!< Read: pin driver is low */
#define GPIO_OUTCLR_PIN0_High (1UL) /*!< Read: pin driver is high */
#define GPIO_OUTCLR_PIN0_Clear (1UL) /*!< Write: writing a '1' sets the pin low; writing a '0' has no effect */

/* Register: GPIO_IN */
/* Description: Read GPIO port */

/* Bit 31 : Pin 31 */
#define GPIO_IN_PIN31_Pos (31UL) /*!< Position of PIN31 field. */
#define GPIO_IN_PIN31_Msk (0x1UL << GPIO_IN_PIN31_Pos) /*!< Bit mask of PIN31 field. */
#define GPIO_IN_PIN31_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN31_High (1UL) /*!< Pin input is high */

/* Bit 30 : Pin 30 */
#define GPIO_IN_PIN30_Pos (30UL) /*!< Position of PIN30 field. */
#define GPIO_IN_PIN30_Msk (0x1UL << GPIO_IN_PIN30_Pos) /*!< Bit mask of PIN30 field. */
#define GPIO_IN_PIN30_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN30_High (1UL) /*!< Pin input is high */

/* Bit 29 : Pin 29 */
#define GPIO_IN_PIN29_Pos (29UL) /*!< Position of PIN29 field. */
#define GPIO_IN_PIN29_Msk (0x1UL << GPIO_IN_PIN29_Pos) /*!< Bit mask of PIN29 field. */
#define GPIO_IN_PIN29_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN29_High (1UL) /*!< Pin input is high */

/* Bit 28 : Pin 28 */
#define GPIO_IN_PIN28_Pos (28UL) /*!< Position of PIN28 field. */
#define GPIO_IN_PIN28_Msk (0x1UL << GPIO_IN_PIN28_Pos) /*!< Bit mask of PIN28 field. */
#define GPIO_IN_PIN28_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN28_High (1UL) /*!< Pin input is high */

/* Bit 27 : Pin 27 */
#define GPIO_IN_PIN27_Pos (27UL) /*!< Position of PIN27 field. */
#define GPIO_IN_PIN27_Msk (0x1UL << GPIO_IN_PIN27_Pos) /*!< Bit mask of PIN27 field. */
#define GPIO_IN_PIN27_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN27_High (1UL) /*!< Pin input is high */

/* Bit 26 : Pin 26 */
#define GPIO_IN_PIN26_Pos (26UL) /*!< Position of PIN26 field. */
#define GPIO_IN_PIN26_Msk (0x1UL << GPIO_IN_PIN26_Pos) /*!< Bit mask of PIN26 field. */
#define GPIO_IN_PIN26_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN26_High (1UL) /*!< Pin input is high */

/* Bit 25 : Pin 25 */
#define GPIO_IN_PIN25_Pos (25UL) /*!< Position of PIN25 field. */
#define GPIO_IN_PIN25_Msk (0x1UL << GPIO_IN_PIN25_Pos) /*!< Bit mask of PIN25 field. */
#define GPIO_IN_PIN25_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN25_High (1UL) /*!< Pin input is high */

/* Bit 24 : Pin 24 */
#define GPIO_IN_PIN24_Pos (24UL) /*!< Position of PIN24 field. */
#define GPIO_IN_PIN24_Msk (0x1UL << GPIO_IN_PIN24_Pos) /*!< Bit mask of PIN24 field. */
#define GPIO_IN_PIN24_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN24_High (1UL) /*!< Pin input is high */

/* Bit 23 : Pin 23 */
#define GPIO_IN_PIN23_Pos (23UL) /*!< Position of PIN23 field. */
#define GPIO_IN_PIN23_Msk (0x1UL << GPIO_IN_PIN23_Pos) /*!< Bit mask of PIN23 field. */
#define GPIO_IN_PIN23_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN23_High (1UL) /*!< Pin input is high */

/* Bit 22 : Pin 22 */
#define GPIO_IN_PIN22_Pos (22UL) /*!< Position of PIN22 field. */
#define GPIO_IN_PIN22_Msk (0x1UL << GPIO_IN_PIN22_Pos) /*!< Bit mask of PIN22 field. */
#define GPIO_IN_PIN22_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN22_High (1UL) /*!< Pin input is high */

/* Bit 21 : Pin 21 */
#define GPIO_IN_PIN21_Pos (21UL) /*!< Position of PIN21 field. */
#define GPIO_IN_PIN21_Msk (0x1UL << GPIO_IN_PIN21_Pos) /*!< Bit mask of PIN21 field. */
#define GPIO_IN_PIN21_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN21_High (1UL) /*!< Pin input is high */

/* Bit 20 : Pin 20 */
#define GPIO_IN_PIN20_Pos (20UL) /*!< Position of PIN20 field. */
#define GPIO_IN_PIN20_Msk (0x1UL << GPIO_IN_PIN20_Pos) /*!< Bit mask of PIN20 field. */
#define GPIO_IN_PIN20_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN20_High (1UL) /*!< Pin input is high */

/* Bit 19 : Pin 19 */
#define GPIO_IN_PIN19_Pos (19UL) /*!< Position of PIN19 field. */
#define GPIO_IN_PIN19_Msk (0x1UL << GPIO_IN_PIN19_Pos) /*!< Bit mask of PIN19 field. */
#define GPIO_IN_PIN19_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN19_High (1UL) /*!< Pin input is high */

/* Bit 18 : Pin 18 */
#define GPIO_IN_PIN18_Pos (18UL) /*!< Position of PIN18 field. */
#define GPIO_IN_PIN18_Msk (0x1UL << GPIO_IN_PIN18_Pos) /*!< Bit mask of PIN18 field. */
#define GPIO_IN_PIN18_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN18_High (1UL) /*!< Pin input is high */

/* Bit 17 : Pin 17 */
#define GPIO_IN_PIN17_Pos (17UL) /*!< Position of PIN17 field. */
#define GPIO_IN_PIN17_Msk (0x1UL << GPIO_IN_PIN17_Pos) /*!< Bit mask of PIN17 field. */
#define GPIO_IN_PIN17_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN17_High (1UL) /*!< Pin input is high */

/* Bit 16 : Pin 16 */
#define GPIO_IN_PIN16_Pos (16UL) /*!< Position of PIN16 field. */
#define GPIO_IN_PIN16_Msk (0x1UL << GPIO_IN_PIN16_Pos) /*!< Bit mask of PIN16 field. */
#define GPIO_IN_PIN16_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN16_High (1UL) /*!< Pin input is high */

/* Bit 15 : Pin 15 */
#define GPIO_IN_PIN15_Pos (15UL) /*!< Position of PIN15 field. */
#define GPIO_IN_PIN15_Msk (0x1UL << GPIO_IN_PIN15_Pos) /*!< Bit mask of PIN15 field. */
#define GPIO_IN_PIN15_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN15_High (1UL) /*!< Pin input is high */

/* Bit 14 : Pin 14 */
#define GPIO_IN_PIN14_Pos (14UL) /*!< Position of PIN14 field. */
#define GPIO_IN_PIN14_Msk (0x1UL << GPIO_IN_PIN14_Pos) /*!< Bit mask of PIN14 field. */
#define GPIO_IN_PIN14_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN14_High (1UL) /*!< Pin input is high */

/* Bit 13 : Pin 13 */
#define GPIO_IN_PIN13_Pos (13UL) /*!< Position of PIN13 field. */
#define GPIO_IN_PIN13_Msk (0x1UL << GPIO_IN_PIN13_Pos) /*!< Bit mask of PIN13 field. */
#define GPIO_IN_PIN13_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN13_High (1UL) /*!< Pin input is high */

/* Bit 12 : Pin 12 */
#define GPIO_IN_PIN12_Pos (12UL) /*!< Position of PIN12 field. */
#define GPIO_IN_PIN12_Msk (0x1UL << GPIO_IN_PIN12_Pos) /*!< Bit mask of PIN12 field. */
#define GPIO_IN_PIN12_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN12_High (1UL) /*!< Pin input is high */

/* Bit 11 : Pin 11 */
#define GPIO_IN_PIN11_Pos (11UL) /*!< Position of PIN11 field. */
#define GPIO_IN_PIN11_Msk (0x1UL << GPIO_IN_PIN11_Pos) /*!< Bit mask of PIN11 field. */
#define GPIO_IN_PIN11_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN11_High (1UL) /*!< Pin input is high */

/* Bit 10 : Pin 10 */
#define GPIO_IN_PIN10_Pos (10UL) /*!< Position of PIN10 field. */
#define GPIO_IN_PIN10_Msk (0x1UL << GPIO_IN_PIN10_Pos) /*!< Bit mask of PIN10 field. */
#define GPIO_IN_PIN10_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN10_High (1UL) /*!< Pin input is high */

/* Bit 9 : Pin 9 */
#define GPIO_IN_PIN9_Pos (9UL) /*!< Position of PIN9 field. */
#define GPIO_IN_PIN9_Msk (0x1UL << GPIO_IN_PIN9_Pos) /*!< Bit mask of PIN9 field. */
#define GPIO_IN_PIN9_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN9_High (1UL) /*!< Pin input is high */

/* Bit 8 : Pin 8 */
#define GPIO_IN_PIN8_Pos (8UL) /*!< Position of PIN8 field. */
#define GPIO_IN_PIN8_Msk (0x1UL << GPIO_IN_PIN8_Pos) /*!< Bit mask of PIN8 field. */
#define GPIO_IN_PIN8_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN8_High (1UL) /*!< Pin input is high */

/* Bit 7 : Pin 7 */
#define GPIO_IN_PIN7_Pos (7UL) /*!< Position of PIN7 field. */
#define GPIO_IN_PIN7_Msk (0x1UL << GPIO_IN_PIN7_Pos) /*!< Bit mask of PIN7 field. */
#define GPIO_IN_PIN7_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN7_High (1UL) /*!< Pin input is high */

/* Bit 6 : Pin 6 */
#define GPIO_IN_PIN6_Pos (6UL) /*!< Position of PIN6 field. */
#define GPIO_IN_PIN6_Msk (0x1UL << GPIO_IN_PIN6_Pos) /*!< Bit mask of PIN6 field. */
#define GPIO_IN_PIN6_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN6_High (1UL) /*!< Pin input is high */

/* Bit 5 : Pin 5 */
#define GPIO_IN_PIN5_Pos (5UL) /*!< Position of PIN5 field. */
#define GPIO_IN_PIN5_Msk (0x1UL << GPIO_IN_PIN5_Pos) /*!< Bit mask of PIN5 field. */
#define GPIO_IN_PIN5_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN5_High (1UL) /*!< Pin input is high */

/* Bit 4 : Pin 4 */
#define GPIO_IN_PIN4_Pos (4UL) /*!< Position of PIN4 field. */
#define GPIO_IN_PIN4_Msk (0x1UL << GPIO_IN_PIN4_Pos) /*!< Bit mask of PIN4 field. */
#define GPIO_IN_PIN4_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN4_High (1UL) /*!< Pin input is high */

/* Bit 3 : Pin 3 */
#define GPIO_IN_PIN3_Pos (3UL) /*!< Position of PIN3 field. */
#define GPIO_IN_PIN3_Msk (0x1UL << GPIO_IN_PIN3_Pos) /*!< Bit mask of PIN3 field. */
#define GPIO_IN_PIN3_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN3_High (1UL) /*!< Pin input is high */

/* Bit 2 : Pin 2 */
#define GPIO_IN_PIN2_Pos (2UL) /*!< Position of PIN2 field. */
#define GPIO_IN_PIN2_Msk (0x1UL << GPIO_IN_PIN2_Pos) /*!< Bit mask of PIN2 field. */
#define GPIO_IN_PIN2_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN2_High (1UL) /*!< Pin input is high */

/* Bit 1 : Pin 1 */
#define GPIO_IN_PIN1_Pos (1UL) /*!< Position of PIN1 field. */
#define GPIO_IN_PIN1_Msk (0x1UL << GPIO_IN_PIN1_Pos) /*!< Bit mask of PIN1 field. */
#define GPIO_IN_PIN1_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN1_High (1UL) /*!< Pin input is high */

/* Bit 0 : Pin 0 */
#define GPIO_IN_PIN0_Pos (0UL) /*!< Position of PIN0 field. */
#define GPIO_IN_PIN0_Msk (0x1UL << GPIO_IN_PIN0_Pos) /*!< Bit mask of PIN0 field. */
#define GPIO_IN_PIN0_Low (0UL) /*!< Pin input is low */
#define GPIO_IN_PIN0_High (1UL) /*!< Pin input is high */

/* Register: GPIO_DIR */
/* Description: Direction of GPIO pins */

/* Bit 31 : Pin 31 */
#define GPIO_DIR_PIN31_Pos (31UL) /*!< Position of PIN31 field. */
#define GPIO_DIR_PIN31_Msk (0x1UL << GPIO_DIR_PIN31_Pos) /*!< Bit mask of PIN31 field. */
#define GPIO_DIR_PIN31_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN31_Output (1UL) /*!< Pin set as output */

/* Bit 30 : Pin 30 */
#define GPIO_DIR_PIN30_Pos (30UL) /*!< Position of PIN30 field. */
#define GPIO_DIR_PIN30_Msk (0x1UL << GPIO_DIR_PIN30_Pos) /*!< Bit mask of PIN30 field. */
#define GPIO_DIR_PIN30_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN30_Output (1UL) /*!< Pin set as output */

/* Bit 29 : Pin 29 */
#define GPIO_DIR_PIN29_Pos (29UL) /*!< Position of PIN29 field. */
#define GPIO_DIR_PIN29_Msk (0x1UL << GPIO_DIR_PIN29_Pos) /*!< Bit mask of PIN29 field. */
#define GPIO_DIR_PIN29_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN29_Output (1UL) /*!< Pin set as output */

/* Bit 28 : Pin 28 */
#define GPIO_DIR_PIN28_Pos (28UL) /*!< Position of PIN28 field. */
#define GPIO_DIR_PIN28_Msk (0x1UL << GPIO_DIR_PIN28_Pos) /*!< Bit mask of PIN28 field. */
#define GPIO_DIR_PIN28_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN28_Output (1UL) /*!< Pin set as output */

/* Bit 27 : Pin 27 */
#define GPIO_DIR_PIN27_Pos (27UL) /*!< Position of PIN27 field. */
#define GPIO_DIR_PIN27_Msk (0x1UL << GPIO_DIR_PIN27_Pos) /*!< Bit mask of PIN27 field. */
#define GPIO_DIR_PIN27_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN27_Output (1UL) /*!< Pin set as output */

/* Bit 26 : Pin 26 */
#define GPIO_DIR_PIN26_Pos (26UL) /*!< Position of PIN26 field. */
#define GPIO_DIR_PIN26_Msk (0x1UL << GPIO_DIR_PIN26_Pos) /*!< Bit mask of PIN26 field. */
#define GPIO_DIR_PIN26_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN26_Output (1UL) /*!< Pin set as output */

/* Bit 25 : Pin 25 */
#define GPIO_DIR_PIN25_Pos (25UL) /*!< Position of PIN25 field. */
#define GPIO_DIR_PIN25_Msk (0x1UL << GPIO_DIR_PIN25_Pos) /*!< Bit mask of PIN25 field. */
#define GPIO_DIR_PIN25_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN25_Output (1UL) /*!< Pin set as output */

/* Bit 24 : Pin 24 */
#define GPIO_DIR_PIN24_Pos (24UL) /*!< Position of PIN24 field. */
#define GPIO_DIR_PIN24_Msk (0x1UL << GPIO_DIR_PIN24_Pos) /*!< Bit mask of PIN24 field. */
#define GPIO_DIR_PIN24_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN24_Output (1UL) /*!< Pin set as output */

/* Bit 23 : Pin 23 */
#define GPIO_DIR_PIN23_Pos (23UL) /*!< Position of PIN23 field. */
#define GPIO_DIR_PIN23_Msk (0x1UL << GPIO_DIR_PIN23_Pos) /*!< Bit mask of PIN23 field. */
#define GPIO_DIR_PIN23_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN23_Output (1UL) /*!< Pin set as output */

/* Bit 22 : Pin 22 */
#define GPIO_DIR_PIN22_Pos (22UL) /*!< Position of PIN22 field. */
#define GPIO_DIR_PIN22_Msk (0x1UL << GPIO_DIR_PIN22_Pos) /*!< Bit mask of PIN22 field. */
#define GPIO_DIR_PIN22_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN22_Output (1UL) /*!< Pin set as output */

/* Bit 21 : Pin 21 */
#define GPIO_DIR_PIN21_Pos (21UL) /*!< Position of PIN21 field. */
#define GPIO_DIR_PIN21_Msk (0x1UL << GPIO_DIR_PIN21_Pos) /*!< Bit mask of PIN21 field. */
#define GPIO_DIR_PIN21_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN21_Output (1UL) /*!< Pin set as output */

/* Bit 20 : Pin 20 */
#define GPIO_DIR_PIN20_Pos (20UL) /*!< Position of PIN20 field. */
#define GPIO_DIR_PIN20_Msk (0x1UL << GPIO_DIR_PIN20_Pos) /*!< Bit mask of PIN20 field. */
#define GPIO_DIR_PIN20_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN20_Output (1UL) /*!< Pin set as output */

/* Bit 19 : Pin 19 */
#define GPIO_DIR_PIN19_Pos (19UL) /*!< Position of PIN19 field. */
#define GPIO_DIR_PIN19_Msk (0x1UL << GPIO_DIR_PIN19_Pos) /*!< Bit mask of PIN19 field. */
#define GPIO_DIR_PIN19_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN19_Output (1UL) /*!< Pin set as output */

/* Bit 18 : Pin 18 */
#define GPIO_DIR_PIN18_Pos (18UL) /*!< Position of PIN18 field. */
#define GPIO_DIR_PIN18_Msk (0x1UL << GPIO_DIR_PIN18_Pos) /*!< Bit mask of PIN18 field. */
#define GPIO_DIR_PIN18_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN18_Output (1UL) /*!< Pin set as output */

/* Bit 17 : Pin 17 */
#define GPIO_DIR_PIN17_Pos (17UL) /*!< Position of PIN17 field. */
#define GPIO_DIR_PIN17_Msk (0x1UL << GPIO_DIR_PIN17_Pos) /*!< Bit mask of PIN17 field. */
#define GPIO_DIR_PIN17_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN17_Output (1UL) /*!< Pin set as output */

/* Bit 16 : Pin 16 */
#define GPIO_DIR_PIN16_Pos (16UL) /*!< Position of PIN16 field. */
#define GPIO_DIR_PIN16_Msk (0x1UL << GPIO_DIR_PIN16_Pos) /*!< Bit mask of PIN16 field. */
#define GPIO_DIR_PIN16_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN16_Output (1UL) /*!< Pin set as output */

/* Bit 15 : Pin 15 */
#define GPIO_DIR_PIN15_Pos (15UL) /*!< Position of PIN15 field. */
#define GPIO_DIR_PIN15_Msk (0x1UL << GPIO_DIR_PIN15_Pos) /*!< Bit mask of PIN15 field. */
#define GPIO_DIR_PIN15_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN15_Output (1UL) /*!< Pin set as output */

/* Bit 14 : Pin 14 */
#define GPIO_DIR_PIN14_Pos (14UL) /*!< Position of PIN14 field. */
#define GPIO_DIR_PIN14_Msk (0x1UL << GPIO_DIR_PIN14_Pos) /*!< Bit mask of PIN14 field. */
#define GPIO_DIR_PIN14_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN14_Output (1UL) /*!< Pin set as output */

/* Bit 13 : Pin 13 */
#define GPIO_DIR_PIN13_Pos (13UL) /*!< Position of PIN13 field. */
#define GPIO_DIR_PIN13_Msk (0x1UL << GPIO_DIR_PIN13_Pos) /*!< Bit mask of PIN13 field. */
#define GPIO_DIR_PIN13_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN13_Output (1UL) /*!< Pin set as output */

/* Bit 12 : Pin 12 */
#define GPIO_DIR_PIN12_Pos (12UL) /*!< Position of PIN12 field. */
#define GPIO_DIR_PIN12_Msk (0x1UL << GPIO_DIR_PIN12_Pos) /*!< Bit mask of PIN12 field. */
#define GPIO_DIR_PIN12_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN12_Output (1UL) /*!< Pin set as output */

/* Bit 11 : Pin 11 */
#define GPIO_DIR_PIN11_Pos (11UL) /*!< Position of PIN11 field. */
#define GPIO_DIR_PIN11_Msk (0x1UL << GPIO_DIR_PIN11_Pos) /*!< Bit mask of PIN11 field. */
#define GPIO_DIR_PIN11_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN11_Output (1UL) /*!< Pin set as output */

/* Bit 10 : Pin 10 */
#define GPIO_DIR_PIN10_Pos (10UL) /*!< Position of PIN10 field. */
#define GPIO_DIR_PIN10_Msk (0x1UL << GPIO_DIR_PIN10_Pos) /*!< Bit mask of PIN10 field. */
#define GPIO_DIR_PIN10_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN10_Output (1UL) /*!< Pin set as output */

/* Bit 9 : Pin 9 */
#define GPIO_DIR_PIN9_Pos (9UL) /*!< Position of PIN9 field. */
#define GPIO_DIR_PIN9_Msk (0x1UL << GPIO_DIR_PIN9_Pos) /*!< Bit mask of PIN9 field. */
#define GPIO_DIR_PIN9_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN9_Output (1UL) /*!< Pin set as output */

/* Bit 8 : Pin 8 */
#define GPIO_DIR_PIN8_Pos (8UL) /*!< Position of PIN8 field. */
#define GPIO_DIR_PIN8_Msk (0x1UL << GPIO_DIR_PIN8_Pos) /*!< Bit mask of PIN8 field. */
#define GPIO_DIR_PIN8_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN8_Output (1UL) /*!< Pin set as output */

/* Bit 7 : Pin 7 */
#define GPIO_DIR_PIN7_Pos (7UL) /*!< Position of PIN7 field. */
#define GPIO_DIR_PIN7_Msk (0x1UL << GPIO_DIR_PIN7_Pos) /*!< Bit mask of PIN7 field. */
#define GPIO_DIR_PIN7_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN7_Output (1UL) /*!< Pin set as output */

/* Bit 6 : Pin 6 */
#define GPIO_DIR_PIN6_Pos (6UL) /*!< Position of PIN6 field. */
#define GPIO_DIR_PIN6_Msk (0x1UL << GPIO_DIR_PIN6_Pos) /*!< Bit mask of PIN6 field. */
#define GPIO_DIR_PIN6_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN6_Output (1UL) /*!< Pin set as output */

/* Bit 5 : Pin 5 */
#define GPIO_DIR_PIN5_Pos (5UL) /*!< Position of PIN5 field. */
#define GPIO_DIR_PIN5_Msk (0x1UL << GPIO_DIR_PIN5_Pos) /*!< Bit mask of PIN5 field. */
#define GPIO_DIR_PIN5_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN5_Output (1UL) /*!< Pin set as output */

/* Bit 4 : Pin 4 */
#define GPIO_DIR_PIN4_Pos (4UL) /*!< Position of PIN4 field. */
#define GPIO_DIR_PIN4_Msk (0x1UL << GPIO_DIR_PIN4_Pos) /*!< Bit mask of PIN4 field. */
#define GPIO_DIR_PIN4_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN4_Output (1UL) /*!< Pin set as output */

/* Bit 3 : Pin 3 */
#define GPIO_DIR_PIN3_Pos (3UL) /*!< Position of PIN3 field. */
#define GPIO_DIR_PIN3_Msk (0x1UL << GPIO_DIR_PIN3_Pos) /*!< Bit mask of PIN3 field. */
#define GPIO_DIR_PIN3_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN3_Output (1UL) /*!< Pin set as output */

/* Bit 2 : Pin 2 */
#define GPIO_DIR_PIN2_Pos (2UL) /*!< Position of PIN2 field. */
#define GPIO_DIR_PIN2_Msk (0x1UL << GPIO_DIR_PIN2_Pos) /*!< Bit mask of PIN2 field. */
#define GPIO_DIR_PIN2_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN2_Output (1UL) /*!< Pin set as output */

/* Bit 1 : Pin 1 */
#define GPIO_DIR_PIN1_Pos (1UL) /*!< Position of PIN1 field. */
#define GPIO_DIR_PIN1_Msk (0x1UL << GPIO_DIR_PIN1_Pos) /*!< Bit mask of PIN1 field. */
#define GPIO_DIR_PIN1_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN1_Output (1UL) /*!< Pin set as output */

/* Bit 0 : Pin 0 */
#define GPIO_DIR_PIN0_Pos (0UL) /*!< Position of PIN0 field. */
#define GPIO_DIR_PIN0_Msk (0x1UL << GPIO_DIR_PIN0_Pos) /*!< Bit mask of PIN0 field. */
#define GPIO_DIR_PIN0_Input (0UL) /*!< Pin set as input */
#define GPIO_DIR_PIN0_Output (1UL) /*!< Pin set as output */

/* Register: GPIO_DIRSET */
/* Description: DIR set register */

/* Bit 31 : Set as output pin 31 */
#define GPIO_DIRSET_PIN31_Pos (31UL) /*!< Position of PIN31 field. */
#define GPIO_DIRSET_PIN31_Msk (0x1UL << GPIO_DIRSET_PIN31_Pos) /*!< Bit mask of PIN31 field. */
#define GPIO_DIRSET_PIN31_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN31_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN31_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 30 : Set as output pin 30 */
#define GPIO_DIRSET_PIN30_Pos (30UL) /*!< Position of PIN30 field. */
#define GPIO_DIRSET_PIN30_Msk (0x1UL << GPIO_DIRSET_PIN30_Pos) /*!< Bit mask of PIN30 field. */
#define GPIO_DIRSET_PIN30_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN30_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN30_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 29 : Set as output pin 29 */
#define GPIO_DIRSET_PIN29_Pos (29UL) /*!< Position of PIN29 field. */
#define GPIO_DIRSET_PIN29_Msk (0x1UL << GPIO_DIRSET_PIN29_Pos) /*!< Bit mask of PIN29 field. */
#define GPIO_DIRSET_PIN29_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN29_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN29_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 28 : Set as output pin 28 */
#define GPIO_DIRSET_PIN28_Pos (28UL) /*!< Position of PIN28 field. */
#define GPIO_DIRSET_PIN28_Msk (0x1UL << GPIO_DIRSET_PIN28_Pos) /*!< Bit mask of PIN28 field. */
#define GPIO_DIRSET_PIN28_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN28_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN28_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 27 : Set as output pin 27 */
#define GPIO_DIRSET_PIN27_Pos (27UL) /*!< Position of PIN27 field. */
#define GPIO_DIRSET_PIN27_Msk (0x1UL << GPIO_DIRSET_PIN27_Pos) /*!< Bit mask of PIN27 field. */
#define GPIO_DIRSET_PIN27_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN27_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN27_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 26 : Set as output pin 26 */
#define GPIO_DIRSET_PIN26_Pos (26UL) /*!< Position of PIN26 field. */
#define GPIO_DIRSET_PIN26_Msk (0x1UL << GPIO_DIRSET_PIN26_Pos) /*!< Bit mask of PIN26 field. */
#define GPIO_DIRSET_PIN26_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN26_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN26_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 25 : Set as output pin 25 */
#define GPIO_DIRSET_PIN25_Pos (25UL) /*!< Position of PIN25 field. */
#define GPIO_DIRSET_PIN25_Msk (0x1UL << GPIO_DIRSET_PIN25_Pos) /*!< Bit mask of PIN25 field. */
#define GPIO_DIRSET_PIN25_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN25_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN25_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 24 : Set as output pin 24 */
#define GPIO_DIRSET_PIN24_Pos (24UL) /*!< Position of PIN24 field. */
#define GPIO_DIRSET_PIN24_Msk (0x1UL << GPIO_DIRSET_PIN24_Pos) /*!< Bit mask of PIN24 field. */
#define GPIO_DIRSET_PIN24_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN24_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN24_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 23 : Set as output pin 23 */
#define GPIO_DIRSET_PIN23_Pos (23UL) /*!< Position of PIN23 field. */
#define GPIO_DIRSET_PIN23_Msk (0x1UL << GPIO_DIRSET_PIN23_Pos) /*!< Bit mask of PIN23 field. */
#define GPIO_DIRSET_PIN23_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN23_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN23_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 22 : Set as output pin 22 */
#define GPIO_DIRSET_PIN22_Pos (22UL) /*!< Position of PIN22 field. */
#define GPIO_DIRSET_PIN22_Msk (0x1UL << GPIO_DIRSET_PIN22_Pos) /*!< Bit mask of PIN22 field. */
#define GPIO_DIRSET_PIN22_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN22_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN22_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 21 : Set as output pin 21 */
#define GPIO_DIRSET_PIN21_Pos (21UL) /*!< Position of PIN21 field. */
#define GPIO_DIRSET_PIN21_Msk (0x1UL << GPIO_DIRSET_PIN21_Pos) /*!< Bit mask of PIN21 field. */
#define GPIO_DIRSET_PIN21_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN21_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN21_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 20 : Set as output pin 20 */
#define GPIO_DIRSET_PIN20_Pos (20UL) /*!< Position of PIN20 field. */
#define GPIO_DIRSET_PIN20_Msk (0x1UL << GPIO_DIRSET_PIN20_Pos) /*!< Bit mask of PIN20 field. */
#define GPIO_DIRSET_PIN20_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN20_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN20_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 19 : Set as output pin 19 */
#define GPIO_DIRSET_PIN19_Pos (19UL) /*!< Position of PIN19 field. */
#define GPIO_DIRSET_PIN19_Msk (0x1UL << GPIO_DIRSET_PIN19_Pos) /*!< Bit mask of PIN19 field. */
#define GPIO_DIRSET_PIN19_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN19_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN19_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 18 : Set as output pin 18 */
#define GPIO_DIRSET_PIN18_Pos (18UL) /*!< Position of PIN18 field. */
#define GPIO_DIRSET_PIN18_Msk (0x1UL << GPIO_DIRSET_PIN18_Pos) /*!< Bit mask of PIN18 field. */
#define GPIO_DIRSET_PIN18_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN18_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN18_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 17 : Set as output pin 17 */
#define GPIO_DIRSET_PIN17_Pos (17UL) /*!< Position of PIN17 field. */
#define GPIO_DIRSET_PIN17_Msk (0x1UL << GPIO_DIRSET_PIN17_Pos) /*!< Bit mask of PIN17 field. */
#define GPIO_DIRSET_PIN17_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN17_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN17_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 16 : Set as output pin 16 */
#define GPIO_DIRSET_PIN16_Pos (16UL) /*!< Position of PIN16 field. */
#define GPIO_DIRSET_PIN16_Msk (0x1UL << GPIO_DIRSET_PIN16_Pos) /*!< Bit mask of PIN16 field. */
#define GPIO_DIRSET_PIN16_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN16_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN16_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 15 : Set as output pin 15 */
#define GPIO_DIRSET_PIN15_Pos (15UL) /*!< Position of PIN15 field. */
#define GPIO_DIRSET_PIN15_Msk (0x1UL << GPIO_DIRSET_PIN15_Pos) /*!< Bit mask of PIN15 field. */
#define GPIO_DIRSET_PIN15_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN15_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN15_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 14 : Set as output pin 14 */
#define GPIO_DIRSET_PIN14_Pos (14UL) /*!< Position of PIN14 field. */
#define GPIO_DIRSET_PIN14_Msk (0x1UL << GPIO_DIRSET_PIN14_Pos) /*!< Bit mask of PIN14 field. */
#define GPIO_DIRSET_PIN14_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN14_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN14_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 13 : Set as output pin 13 */
#define GPIO_DIRSET_PIN13_Pos (13UL) /*!< Position of PIN13 field. */
#define GPIO_DIRSET_PIN13_Msk (0x1UL << GPIO_DIRSET_PIN13_Pos) /*!< Bit mask of PIN13 field. */
#define GPIO_DIRSET_PIN13_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN13_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN13_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 12 : Set as output pin 12 */
#define GPIO_DIRSET_PIN12_Pos (12UL) /*!< Position of PIN12 field. */
#define GPIO_DIRSET_PIN12_Msk (0x1UL << GPIO_DIRSET_PIN12_Pos) /*!< Bit mask of PIN12 field. */
#define GPIO_DIRSET_PIN12_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN12_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN12_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 11 : Set as output pin 11 */
#define GPIO_DIRSET_PIN11_Pos (11UL) /*!< Position of PIN11 field. */
#define GPIO_DIRSET_PIN11_Msk (0x1UL << GPIO_DIRSET_PIN11_Pos) /*!< Bit mask of PIN11 field. */
#define GPIO_DIRSET_PIN11_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN11_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN11_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 10 : Set as output pin 10 */
#define GPIO_DIRSET_PIN10_Pos (10UL) /*!< Position of PIN10 field. */
#define GPIO_DIRSET_PIN10_Msk (0x1UL << GPIO_DIRSET_PIN10_Pos) /*!< Bit mask of PIN10 field. */
#define GPIO_DIRSET_PIN10_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN10_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN10_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 9 : Set as output pin 9 */
#define GPIO_DIRSET_PIN9_Pos (9UL) /*!< Position of PIN9 field. */
#define GPIO_DIRSET_PIN9_Msk (0x1UL << GPIO_DIRSET_PIN9_Pos) /*!< Bit mask of PIN9 field. */
#define GPIO_DIRSET_PIN9_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN9_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN9_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 8 : Set as output pin 8 */
#define GPIO_DIRSET_PIN8_Pos (8UL) /*!< Position of PIN8 field. */
#define GPIO_DIRSET_PIN8_Msk (0x1UL << GPIO_DIRSET_PIN8_Pos) /*!< Bit mask of PIN8 field. */
#define GPIO_DIRSET_PIN8_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN8_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN8_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 7 : Set as output pin 7 */
#define GPIO_DIRSET_PIN7_Pos (7UL) /*!< Position of PIN7 field. */
#define GPIO_DIRSET_PIN7_Msk (0x1UL << GPIO_DIRSET_PIN7_Pos) /*!< Bit mask of PIN7 field. */
#define GPIO_DIRSET_PIN7_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN7_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN7_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 6 : Set as output pin 6 */
#define GPIO_DIRSET_PIN6_Pos (6UL) /*!< Position of PIN6 field. */
#define GPIO_DIRSET_PIN6_Msk (0x1UL << GPIO_DIRSET_PIN6_Pos) /*!< Bit mask of PIN6 field. */
#define GPIO_DIRSET_PIN6_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN6_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN6_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 5 : Set as output pin 5 */
#define GPIO_DIRSET_PIN5_Pos (5UL) /*!< Position of PIN5 field. */
#define GPIO_DIRSET_PIN5_Msk (0x1UL << GPIO_DIRSET_PIN5_Pos) /*!< Bit mask of PIN5 field. */
#define GPIO_DIRSET_PIN5_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN5_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN5_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 4 : Set as output pin 4 */
#define GPIO_DIRSET_PIN4_Pos (4UL) /*!< Position of PIN4 field. */
#define GPIO_DIRSET_PIN4_Msk (0x1UL << GPIO_DIRSET_PIN4_Pos) /*!< Bit mask of PIN4 field. */
#define GPIO_DIRSET_PIN4_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN4_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN4_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 3 : Set as output pin 3 */
#define GPIO_DIRSET_PIN3_Pos (3UL) /*!< Position of PIN3 field. */
#define GPIO_DIRSET_PIN3_Msk (0x1UL << GPIO_DIRSET_PIN3_Pos) /*!< Bit mask of PIN3 field. */
#define GPIO_DIRSET_PIN3_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN3_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN3_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 2 : Set as output pin 2 */
#define GPIO_DIRSET_PIN2_Pos (2UL) /*!< Position of PIN2 field. */
#define GPIO_DIRSET_PIN2_Msk (0x1UL << GPIO_DIRSET_PIN2_Pos) /*!< Bit mask of PIN2 field. */
#define GPIO_DIRSET_PIN2_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN2_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN2_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 1 : Set as output pin 1 */
#define GPIO_DIRSET_PIN1_Pos (1UL) /*!< Position of PIN1 field. */
#define GPIO_DIRSET_PIN1_Msk (0x1UL << GPIO_DIRSET_PIN1_Pos) /*!< Bit mask of PIN1 field. */
#define GPIO_DIRSET_PIN1_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN1_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN1_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Bit 0 : Set as output pin 0 */
#define GPIO_DIRSET_PIN0_Pos (0UL) /*!< Position of PIN0 field. */
#define GPIO_DIRSET_PIN0_Msk (0x1UL << GPIO_DIRSET_PIN0_Pos) /*!< Bit mask of PIN0 field. */
#define GPIO_DIRSET_PIN0_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRSET_PIN0_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRSET_PIN0_Set (1UL) /*!< Write: writing a '1' sets pin to output; writing a '0' has no effect */

/* Register: GPIO_DIRCLR */
/* Description: DIR clear register */

/* Bit 31 : Set as input pin 31 */
#define GPIO_DIRCLR_PIN31_Pos (31UL) /*!< Position of PIN31 field. */
#define GPIO_DIRCLR_PIN31_Msk (0x1UL << GPIO_DIRCLR_PIN31_Pos) /*!< Bit mask of PIN31 field. */
#define GPIO_DIRCLR_PIN31_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN31_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN31_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 30 : Set as input pin 30 */
#define GPIO_DIRCLR_PIN30_Pos (30UL) /*!< Position of PIN30 field. */
#define GPIO_DIRCLR_PIN30_Msk (0x1UL << GPIO_DIRCLR_PIN30_Pos) /*!< Bit mask of PIN30 field. */
#define GPIO_DIRCLR_PIN30_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN30_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN30_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 29 : Set as input pin 29 */
#define GPIO_DIRCLR_PIN29_Pos (29UL) /*!< Position of PIN29 field. */
#define GPIO_DIRCLR_PIN29_Msk (0x1UL << GPIO_DIRCLR_PIN29_Pos) /*!< Bit mask of PIN29 field. */
#define GPIO_DIRCLR_PIN29_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN29_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN29_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 28 : Set as input pin 28 */
#define GPIO_DIRCLR_PIN28_Pos (28UL) /*!< Position of PIN28 field. */
#define GPIO_DIRCLR_PIN28_Msk (0x1UL << GPIO_DIRCLR_PIN28_Pos) /*!< Bit mask of PIN28 field. */
#define GPIO_DIRCLR_PIN28_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN28_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN28_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 27 : Set as input pin 27 */
#define GPIO_DIRCLR_PIN27_Pos (27UL) /*!< Position of PIN27 field. */
#define GPIO_DIRCLR_PIN27_Msk (0x1UL << GPIO_DIRCLR_PIN27_Pos) /*!< Bit mask of PIN27 field. */
#define GPIO_DIRCLR_PIN27_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN27_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN27_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 26 : Set as input pin 26 */
#define GPIO_DIRCLR_PIN26_Pos (26UL) /*!< Position of PIN26 field. */
#define GPIO_DIRCLR_PIN26_Msk (0x1UL << GPIO_DIRCLR_PIN26_Pos) /*!< Bit mask of PIN26 field. */
#define GPIO_DIRCLR_PIN26_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN26_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN26_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 25 : Set as input pin 25 */
#define GPIO_DIRCLR_PIN25_Pos (25UL) /*!< Position of PIN25 field. */
#define GPIO_DIRCLR_PIN25_Msk (0x1UL << GPIO_DIRCLR_PIN25_Pos) /*!< Bit mask of PIN25 field. */
#define GPIO_DIRCLR_PIN25_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN25_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN25_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 24 : Set as input pin 24 */
#define GPIO_DIRCLR_PIN24_Pos (24UL) /*!< Position of PIN24 field. */
#define GPIO_DIRCLR_PIN24_Msk (0x1UL << GPIO_DIRCLR_PIN24_Pos) /*!< Bit mask of PIN24 field. */
#define GPIO_DIRCLR_PIN24_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN24_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN24_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 23 : Set as input pin 23 */
#define GPIO_DIRCLR_PIN23_Pos (23UL) /*!< Position of PIN23 field. */
#define GPIO_DIRCLR_PIN23_Msk (0x1UL << GPIO_DIRCLR_PIN23_Pos) /*!< Bit mask of PIN23 field. */
#define GPIO_DIRCLR_PIN23_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN23_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN23_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 22 : Set as input pin 22 */
#define GPIO_DIRCLR_PIN22_Pos (22UL) /*!< Position of PIN22 field. */
#define GPIO_DIRCLR_PIN22_Msk (0x1UL << GPIO_DIRCLR_PIN22_Pos) /*!< Bit mask of PIN22 field. */
#define GPIO_DIRCLR_PIN22_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN22_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN22_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 21 : Set as input pin 21 */
#define GPIO_DIRCLR_PIN21_Pos (21UL) /*!< Position of PIN21 field. */
#define GPIO_DIRCLR_PIN21_Msk (0x1UL << GPIO_DIRCLR_PIN21_Pos) /*!< Bit mask of PIN21 field. */
#define GPIO_DIRCLR_PIN21_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN21_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN21_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 20 : Set as input pin 20 */
#define GPIO_DIRCLR_PIN20_Pos (20UL) /*!< Position of PIN20 field. */
#define GPIO_DIRCLR_PIN20_Msk (0x1UL << GPIO_DIRCLR_PIN20_Pos) /*!< Bit mask of PIN20 field. */
#define GPIO_DIRCLR_PIN20_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN20_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN20_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 19 : Set as input pin 19 */
#define GPIO_DIRCLR_PIN19_Pos (19UL) /*!< Position of PIN19 field. */
#define GPIO_DIRCLR_PIN19_Msk (0x1UL << GPIO_DIRCLR_PIN19_Pos) /*!< Bit mask of PIN19 field. */
#define GPIO_DIRCLR_PIN19_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN19_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN19_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 18 : Set as input pin 18 */
#define GPIO_DIRCLR_PIN18_Pos (18UL) /*!< Position of PIN18 field. */
#define GPIO_DIRCLR_PIN18_Msk (0x1UL << GPIO_DIRCLR_PIN18_Pos) /*!< Bit mask of PIN18 field. */
#define GPIO_DIRCLR_PIN18_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN18_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN18_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 17 : Set as input pin 17 */
#define GPIO_DIRCLR_PIN17_Pos (17UL) /*!< Position of PIN17 field. */
#define GPIO_DIRCLR_PIN17_Msk (0x1UL << GPIO_DIRCLR_PIN17_Pos) /*!< Bit mask of PIN17 field. */
#define GPIO_DIRCLR_PIN17_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN17_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN17_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 16 : Set as input pin 16 */
#define GPIO_DIRCLR_PIN16_Pos (16UL) /*!< Position of PIN16 field. */
#define GPIO_DIRCLR_PIN16_Msk (0x1UL << GPIO_DIRCLR_PIN16_Pos) /*!< Bit mask of PIN16 field. */
#define GPIO_DIRCLR_PIN16_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN16_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN16_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 15 : Set as input pin 15 */
#define GPIO_DIRCLR_PIN15_Pos (15UL) /*!< Position of PIN15 field. */
#define GPIO_DIRCLR_PIN15_Msk (0x1UL << GPIO_DIRCLR_PIN15_Pos) /*!< Bit mask of PIN15 field. */
#define GPIO_DIRCLR_PIN15_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN15_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN15_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 14 : Set as input pin 14 */
#define GPIO_DIRCLR_PIN14_Pos (14UL) /*!< Position of PIN14 field. */
#define GPIO_DIRCLR_PIN14_Msk (0x1UL << GPIO_DIRCLR_PIN14_Pos) /*!< Bit mask of PIN14 field. */
#define GPIO_DIRCLR_PIN14_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN14_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN14_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 13 : Set as input pin 13 */
#define GPIO_DIRCLR_PIN13_Pos (13UL) /*!< Position of PIN13 field. */
#define GPIO_DIRCLR_PIN13_Msk (0x1UL << GPIO_DIRCLR_PIN13_Pos) /*!< Bit mask of PIN13 field. */
#define GPIO_DIRCLR_PIN13_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN13_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN13_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 12 : Set as input pin 12 */
#define GPIO_DIRCLR_PIN12_Pos (12UL) /*!< Position of PIN12 field. */
#define GPIO_DIRCLR_PIN12_Msk (0x1UL << GPIO_DIRCLR_PIN12_Pos) /*!< Bit mask of PIN12 field. */
#define GPIO_DIRCLR_PIN12_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN12_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN12_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 11 : Set as input pin 11 */
#define GPIO_DIRCLR_PIN11_Pos (11UL) /*!< Position of PIN11 field. */
#define GPIO_DIRCLR_PIN11_Msk (0x1UL << GPIO_DIRCLR_PIN11_Pos) /*!< Bit mask of PIN11 field. */
#define GPIO_DIRCLR_PIN11_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN11_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN11_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 10 : Set as input pin 10 */
#define GPIO_DIRCLR_PIN10_Pos (10UL) /*!< Position of PIN10 field. */
#define GPIO_DIRCLR_PIN10_Msk (0x1UL << GPIO_DIRCLR_PIN10_Pos) /*!< Bit mask of PIN10 field. */
#define GPIO_DIRCLR_PIN10_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN10_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN10_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 9 : Set as input pin 9 */
#define GPIO_DIRCLR_PIN9_Pos (9UL) /*!< Position of PIN9 field. */
#define GPIO_DIRCLR_PIN9_Msk (0x1UL << GPIO_DIRCLR_PIN9_Pos) /*!< Bit mask of PIN9 field. */
#define GPIO_DIRCLR_PIN9_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN9_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN9_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 8 : Set as input pin 8 */
#define GPIO_DIRCLR_PIN8_Pos (8UL) /*!< Position of PIN8 field. */
#define GPIO_DIRCLR_PIN8_Msk (0x1UL << GPIO_DIRCLR_PIN8_Pos) /*!< Bit mask of PIN8 field. */
#define GPIO_DIRCLR_PIN8_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN8_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN8_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 7 : Set as input pin 7 */
#define GPIO_DIRCLR_PIN7_Pos (7UL) /*!< Position of PIN7 field. */
#define GPIO_DIRCLR_PIN7_Msk (0x1UL << GPIO_DIRCLR_PIN7_Pos) /*!< Bit mask of PIN7 field. */
#define GPIO_DIRCLR_PIN7_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN7_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN7_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 6 : Set as input pin 6 */
#define GPIO_DIRCLR_PIN6_Pos (6UL) /*!< Position of PIN6 field. */
#define GPIO_DIRCLR_PIN6_Msk (0x1UL << GPIO_DIRCLR_PIN6_Pos) /*!< Bit mask of PIN6 field. */
#define GPIO_DIRCLR_PIN6_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN6_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN6_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 5 : Set as input pin 5 */
#define GPIO_DIRCLR_PIN5_Pos (5UL) /*!< Position of PIN5 field. */
#define GPIO_DIRCLR_PIN5_Msk (0x1UL << GPIO_DIRCLR_PIN5_Pos) /*!< Bit mask of PIN5 field. */
#define GPIO_DIRCLR_PIN5_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN5_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN5_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 4 : Set as input pin 4 */
#define GPIO_DIRCLR_PIN4_Pos (4UL) /*!< Position of PIN4 field. */
#define GPIO_DIRCLR_PIN4_Msk (0x1UL << GPIO_DIRCLR_PIN4_Pos) /*!< Bit mask of PIN4 field. */
#define GPIO_DIRCLR_PIN4_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN4_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN4_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 3 : Set as input pin 3 */
#define GPIO_DIRCLR_PIN3_Pos (3UL) /*!< Position of PIN3 field. */
#define GPIO_DIRCLR_PIN3_Msk (0x1UL << GPIO_DIRCLR_PIN3_Pos) /*!< Bit mask of PIN3 field. */
#define GPIO_DIRCLR_PIN3_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN3_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN3_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 2 : Set as input pin 2 */
#define GPIO_DIRCLR_PIN2_Pos (2UL) /*!< Position of PIN2 field. */
#define GPIO_DIRCLR_PIN2_Msk (0x1UL << GPIO_DIRCLR_PIN2_Pos) /*!< Bit mask of PIN2 field. */
#define GPIO_DIRCLR_PIN2_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN2_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN2_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 1 : Set as input pin 1 */
#define GPIO_DIRCLR_PIN1_Pos (1UL) /*!< Position of PIN1 field. */
#define GPIO_DIRCLR_PIN1_Msk (0x1UL << GPIO_DIRCLR_PIN1_Pos) /*!< Bit mask of PIN1 field. */
#define GPIO_DIRCLR_PIN1_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN1_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN1_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Bit 0 : Set as input pin 0 */
#define GPIO_DIRCLR_PIN0_Pos (0UL) /*!< Position of PIN0 field. */
#define GPIO_DIRCLR_PIN0_Msk (0x1UL << GPIO_DIRCLR_PIN0_Pos) /*!< Bit mask of PIN0 field. */
#define GPIO_DIRCLR_PIN0_Input (0UL) /*!< Read: pin set as input */
#define GPIO_DIRCLR_PIN0_Output (1UL) /*!< Read: pin set as output */
#define GPIO_DIRCLR_PIN0_Clear (1UL) /*!< Write: writing a '1' sets pin to input; writing a '0' has no effect */

/* Register: GPIO_LATCH */
/* Description: Latch register indicating what GPIO pins that have met the criteria set in the PIN_CNF[n].SENSE registers */

/* Bit 31 : Status on whether PIN31 has met criteria set in PIN_CNF31.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN31_Pos (31UL) /*!< Position of PIN31 field. */
#define GPIO_LATCH_PIN31_Msk (0x1UL << GPIO_LATCH_PIN31_Pos) /*!< Bit mask of PIN31 field. */
#define GPIO_LATCH_PIN31_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN31_Latched (1UL) /*!< Criteria has been met */

/* Bit 30 : Status on whether PIN30 has met criteria set in PIN_CNF30.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN30_Pos (30UL) /*!< Position of PIN30 field. */
#define GPIO_LATCH_PIN30_Msk (0x1UL << GPIO_LATCH_PIN30_Pos) /*!< Bit mask of PIN30 field. */
#define GPIO_LATCH_PIN30_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN30_Latched (1UL) /*!< Criteria has been met */

/* Bit 29 : Status on whether PIN29 has met criteria set in PIN_CNF29.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN29_Pos (29UL) /*!< Position of PIN29 field. */
#define GPIO_LATCH_PIN29_Msk (0x1UL << GPIO_LATCH_PIN29_Pos) /*!< Bit mask of PIN29 field. */
#define GPIO_LATCH_PIN29_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN29_Latched (1UL) /*!< Criteria has been met */

/* Bit 28 : Status on whether PIN28 has met criteria set in PIN_CNF28.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN28_Pos (28UL) /*!< Position of PIN28 field. */
#define GPIO_LATCH_PIN28_Msk (0x1UL << GPIO_LATCH_PIN28_Pos) /*!< Bit mask of PIN28 field. */
#define GPIO_LATCH_PIN28_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN28_Latched (1UL) /*!< Criteria has been met */

/* Bit 27 : Status on whether PIN27 has met criteria set in PIN_CNF27.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN27_Pos (27UL) /*!< Position of PIN27 field. */
#define GPIO_LATCH_PIN27_Msk (0x1UL << GPIO_LATCH_PIN27_Pos) /*!< Bit mask of PIN27 field. */
#define GPIO_LATCH_PIN27_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN27_Latched (1UL) /*!< Criteria has been met */

/* Bit 26 : Status on whether PIN26 has met criteria set in PIN_CNF26.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN26_Pos (26UL) /*!< Position of PIN26 field. */
#define GPIO_LATCH_PIN26_Msk (0x1UL << GPIO_LATCH_PIN26_Pos) /*!< Bit mask of PIN26 field. */
#define GPIO_LATCH_PIN26_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN26_Latched (1UL) /*!< Criteria has been met */

/* Bit 25 : Status on whether PIN25 has met criteria set in PIN_CNF25.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN25_Pos (25UL) /*!< Position of PIN25 field. */
#define GPIO_LATCH_PIN25_Msk (0x1UL << GPIO_LATCH_PIN25_Pos) /*!< Bit mask of PIN25 field. */
#define GPIO_LATCH_PIN25_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN25_Latched (1UL) /*!< Criteria has been met */

/* Bit 24 : Status on whether PIN24 has met criteria set in PIN_CNF24.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN24_Pos (24UL) /*!< Position of PIN24 field. */
#define GPIO_LATCH_PIN24_Msk (0x1UL << GPIO_LATCH_PIN24_Pos) /*!< Bit mask of PIN24 field. */
#define GPIO_LATCH_PIN24_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN24_Latched (1UL) /*!< Criteria has been met */

/* Bit 23 : Status on whether PIN23 has met criteria set in PIN_CNF23.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN23_Pos (23UL) /*!< Position of PIN23 field. */
#define GPIO_LATCH_PIN23_Msk (0x1UL << GPIO_LATCH_PIN23_Pos) /*!< Bit mask of PIN23 field. */
#define GPIO_LATCH_PIN23_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN23_Latched (1UL) /*!< Criteria has been met */

/* Bit 22 : Status on whether PIN22 has met criteria set in PIN_CNF22.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN22_Pos (22UL) /*!< Position of PIN22 field. */
#define GPIO_LATCH_PIN22_Msk (0x1UL << GPIO_LATCH_PIN22_Pos) /*!< Bit mask of PIN22 field. */
#define GPIO_LATCH_PIN22_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN22_Latched (1UL) /*!< Criteria has been met */

/* Bit 21 : Status on whether PIN21 has met criteria set in PIN_CNF21.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN21_Pos (21UL) /*!< Position of PIN21 field. */
#define GPIO_LATCH_PIN21_Msk (0x1UL << GPIO_LATCH_PIN21_Pos) /*!< Bit mask of PIN21 field. */
#define GPIO_LATCH_PIN21_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN21_Latched (1UL) /*!< Criteria has been met */

/* Bit 20 : Status on whether PIN20 has met criteria set in PIN_CNF20.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN20_Pos (20UL) /*!< Position of PIN20 field. */
#define GPIO_LATCH_PIN20_Msk (0x1UL << GPIO_LATCH_PIN20_Pos) /*!< Bit mask of PIN20 field. */
#define GPIO_LATCH_PIN20_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN20_Latched (1UL) /*!< Criteria has been met */

/* Bit 19 : Status on whether PIN19 has met criteria set in PIN_CNF19.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN19_Pos (19UL) /*!< Position of PIN19 field. */
#define GPIO_LATCH_PIN19_Msk (0x1UL << GPIO_LATCH_PIN19_Pos) /*!< Bit mask of PIN19 field. */
#define GPIO_LATCH_PIN19_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN19_Latched (1UL) /*!< Criteria has been met */

/* Bit 18 : Status on whether PIN18 has met criteria set in PIN_CNF18.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN18_Pos (18UL) /*!< Position of PIN18 field. */
#define GPIO_LATCH_PIN18_Msk (0x1UL << GPIO_LATCH_PIN18_Pos) /*!< Bit mask of PIN18 field. */
#define GPIO_LATCH_PIN18_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN18_Latched (1UL) /*!< Criteria has been met */

/* Bit 17 : Status on whether PIN17 has met criteria set in PIN_CNF17.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN17_Pos (17UL) /*!< Position of PIN17 field. */
#define GPIO_LATCH_PIN17_Msk (0x1UL << GPIO_LATCH_PIN17_Pos) /*!< Bit mask of PIN17 field. */
#define GPIO_LATCH_PIN17_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN17_Latched (1UL) /*!< Criteria has been met */

/* Bit 16 : Status on whether PIN16 has met criteria set in PIN_CNF16.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN16_Pos (16UL) /*!< Position of PIN16 field. */
#define GPIO_LATCH_PIN16_Msk (0x1UL << GPIO_LATCH_PIN16_Pos) /*!< Bit mask of PIN16 field. */
#define GPIO_LATCH_PIN16_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN16_Latched (1UL) /*!< Criteria has been met */

/* Bit 15 : Status on whether PIN15 has met criteria set in PIN_CNF15.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN15_Pos (15UL) /*!< Position of PIN15 field. */
#define GPIO_LATCH_PIN15_Msk (0x1UL << GPIO_LATCH_PIN15_Pos) /*!< Bit mask of PIN15 field. */
#define GPIO_LATCH_PIN15_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN15_Latched (1UL) /*!< Criteria has been met */

/* Bit 14 : Status on whether PIN14 has met criteria set in PIN_CNF14.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN14_Pos (14UL) /*!< Position of PIN14 field. */
#define GPIO_LATCH_PIN14_Msk (0x1UL << GPIO_LATCH_PIN14_Pos) /*!< Bit mask of PIN14 field. */
#define GPIO_LATCH_PIN14_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN14_Latched (1UL) /*!< Criteria has been met */

/* Bit 13 : Status on whether PIN13 has met criteria set in PIN_CNF13.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN13_Pos (13UL) /*!< Position of PIN13 field. */
#define GPIO_LATCH_PIN13_Msk (0x1UL << GPIO_LATCH_PIN13_Pos) /*!< Bit mask of PIN13 field. */
#define GPIO_LATCH_PIN13_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN13_Latched (1UL) /*!< Criteria has been met */

/* Bit 12 : Status on whether PIN12 has met criteria set in PIN_CNF12.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN12_Pos (12UL) /*!< Position of PIN12 field. */
#define GPIO_LATCH_PIN12_Msk (0x1UL << GPIO_LATCH_PIN12_Pos) /*!< Bit mask of PIN12 field. */
#define GPIO_LATCH_PIN12_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN12_Latched (1UL) /*!< Criteria has been met */

/* Bit 11 : Status on whether PIN11 has met criteria set in PIN_CNF11.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN11_Pos (11UL) /*!< Position of PIN11 field. */
#define GPIO_LATCH_PIN11_Msk (0x1UL << GPIO_LATCH_PIN11_Pos) /*!< Bit mask of PIN11 field. */
#define GPIO_LATCH_PIN11_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN11_Latched (1UL) /*!< Criteria has been met */

/* Bit 10 : Status on whether PIN10 has met criteria set in PIN_CNF10.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN10_Pos (10UL) /*!< Position of PIN10 field. */
#define GPIO_LATCH_PIN10_Msk (0x1UL << GPIO_LATCH_PIN10_Pos) /*!< Bit mask of PIN10 field. */
#define GPIO_LATCH_PIN10_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN10_Latched (1UL) /*!< Criteria has been met */

/* Bit 9 : Status on whether PIN9 has met criteria set in PIN_CNF9.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN9_Pos (9UL) /*!< Position of PIN9 field. */
#define GPIO_LATCH_PIN9_Msk (0x1UL << GPIO_LATCH_PIN9_Pos) /*!< Bit mask of PIN9 field. */
#define GPIO_LATCH_PIN9_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN9_Latched (1UL) /*!< Criteria has been met */

/* Bit 8 : Status on whether PIN8 has met criteria set in PIN_CNF8.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN8_Pos (8UL) /*!< Position of PIN8 field. */
#define GPIO_LATCH_PIN8_Msk (0x1UL << GPIO_LATCH_PIN8_Pos) /*!< Bit mask of PIN8 field. */
#define GPIO_LATCH_PIN8_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN8_Latched (1UL) /*!< Criteria has been met */

/* Bit 7 : Status on whether PIN7 has met criteria set in PIN_CNF7.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN7_Pos (7UL) /*!< Position of PIN7 field. */
#define GPIO_LATCH_PIN7_Msk (0x1UL << GPIO_LATCH_PIN7_Pos) /*!< Bit mask of PIN7 field. */
#define GPIO_LATCH_PIN7_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN7_Latched (1UL) /*!< Criteria has been met */

/* Bit 6 : Status on whether PIN6 has met criteria set in PIN_CNF6.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN6_Pos (6UL) /*!< Position of PIN6 field. */
#define GPIO_LATCH_PIN6_Msk (0x1UL << GPIO_LATCH_PIN6_Pos) /*!< Bit mask of PIN6 field. */
#define GPIO_LATCH_PIN6_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN6_Latched (1UL) /*!< Criteria has been met */

/* Bit 5 : Status on whether PIN5 has met criteria set in PIN_CNF5.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN5_Pos (5UL) /*!< Position of PIN5 field. */
#define GPIO_LATCH_PIN5_Msk (0x1UL << GPIO_LATCH_PIN5_Pos) /*!< Bit mask of PIN5 field. */
#define GPIO_LATCH_PIN5_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN5_Latched (1UL) /*!< Criteria has been met */

/* Bit 4 : Status on whether PIN4 has met criteria set in PIN_CNF4.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN4_Pos (4UL) /*!< Position of PIN4 field. */
#define GPIO_LATCH_PIN4_Msk (0x1UL << GPIO_LATCH_PIN4_Pos) /*!< Bit mask of PIN4 field. */
#define GPIO_LATCH_PIN4_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN4_Latched (1UL) /*!< Criteria has been met */

/* Bit 3 : Status on whether PIN3 has met criteria set in PIN_CNF3.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN3_Pos (3UL) /*!< Position of PIN3 field. */
#define GPIO_LATCH_PIN3_Msk (0x1UL << GPIO_LATCH_PIN3_Pos) /*!< Bit mask of PIN3 field. */
#define GPIO_LATCH_PIN3_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN3_Latched (1UL) /*!< Criteria has been met */

/* Bit 2 : Status on whether PIN2 has met criteria set in PIN_CNF2.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN2_Pos (2UL) /*!< Position of PIN2 field. */
#define GPIO_LATCH_PIN2_Msk (0x1UL << GPIO_LATCH_PIN2_Pos) /*!< Bit mask of PIN2 field. */
#define GPIO_LATCH_PIN2_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN2_Latched (1UL) /*!< Criteria has been met */

/* Bit 1 : Status on whether PIN1 has met criteria set in PIN_CNF1.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN1_Pos (1UL) /*!< Position of PIN1 field. */
#define GPIO_LATCH_PIN1_Msk (0x1UL << GPIO_LATCH_PIN1_Pos) /*!< Bit mask of PIN1 field. */
#define GPIO_LATCH_PIN1_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN1_Latched (1UL) /*!< Criteria has been met */

/* Bit 0 : Status on whether PIN0 has met criteria set in PIN_CNF0.SENSE register. Write '1' to clear. */
#define GPIO_LATCH_PIN0_Pos (0UL) /*!< Position of PIN0 field. */
#define GPIO_LATCH_PIN0_Msk (0x1UL << GPIO_LATCH_PIN0_Pos) /*!< Bit mask of PIN0 field. */
#define GPIO_LATCH_PIN0_NotLatched (0UL) /*!< Criteria has not been met */
#define GPIO_LATCH_PIN0_Latched (1UL) /*!< Criteria has been met */

/* Register: GPIO_DETECTMODE */
/* Description: Select between default DETECT signal behaviour and LDETECT mode */

/* Bit 0 : Select between default DETECT signal behaviour and LDETECT mode */
#define GPIO_DETECTMODE_DETECTMODE_Pos (0UL) /*!< Position of DETECTMODE field. */
#define GPIO_DETECTMODE_DETECTMODE_Msk (0x1UL << GPIO_DETECTMODE_DETECTMODE_Pos) /*!< Bit mask of DETECTMODE field. */
#define GPIO_DETECTMODE_DETECTMODE_Default (0UL) /*!< DETECT directly connected to PIN DETECT signals */
#define GPIO_DETECTMODE_DETECTMODE_LDETECT (1UL) /*!< Use the latched LDETECT behaviour */

/* Register: GPIO_PIN_CNF */
/* Description: Description collection: Configuration of GPIO pins */

/* Bits 17..16 : Pin sensing mechanism */
#define GPIO_PIN_CNF_SENSE_Pos (16UL) /*!< Position of SENSE field. */
#define GPIO_PIN_CNF_SENSE_Msk (0x3UL << GPIO_PIN_CNF_SENSE_Pos) /*!< Bit mask of SENSE field. */
#define GPIO_PIN_CNF_SENSE_Disabled (0UL) /*!< Disabled */
#define GPIO_PIN_CNF_SENSE_High (2UL) /*!< Sense for high level */
#define GPIO_PIN_CNF_SENSE_Low (3UL) /*!< Sense for low level */

/* Bits 10..8 : Drive configuration */
#define GPIO_PIN_CNF_DRIVE_Pos (8UL) /*!< Position of DRIVE field. */
#define GPIO_PIN_CNF_DRIVE_Msk (0x7UL << GPIO_PIN_CNF_DRIVE_Pos) /*!< Bit mask of DRIVE field. */
#define GPIO_PIN_CNF_DRIVE_S0S1 (0UL) /*!< Standard '0', standard '1' */
#define GPIO_PIN_CNF_DRIVE_H0S1 (1UL) /*!< High drive '0', standard '1' */
#define GPIO_PIN_CNF_DRIVE_S0H1 (2UL) /*!< Standard '0', high drive '1' */
#define GPIO_PIN_CNF_DRIVE_H0H1 (3UL) /*!< High drive '0', high 'drive '1'' */
#define GPIO_PIN_CNF_DRIVE_D0S1 (4UL) /*!< Disconnect '0' standard '1' (normally used for wired-or connections) */
#define GPIO_PIN_CNF_DRIVE_D0H1 (5UL) /*!< Disconnect '0', high drive '1' (normally used for wired-or connections) */
#define GPIO_PIN_CNF_DRIVE_S0D1 (6UL) /*!< Standard '0'. disconnect '1' (normally used for wired-and connections) */
#define GPIO_PIN_CNF_DRIVE_H0D1 (7UL) /*!< High drive '0', disconnect '1' (normally used for wired-and connections) */

/* Bits 3..2 : Pull configuration */
#define GPIO_PIN_CNF_PULL_Pos (2UL) /*!< Position of PULL field. */
#define GPIO_PIN_CNF_PULL_Msk (0x3UL << GPIO_PIN_CNF_PULL_Pos) /*!< Bit mask of PULL field. */
#define GPIO_PIN_CNF_PULL_Disabled (0UL) /*!< No pull */
#define GPIO_PIN_CNF_PULL_Pulldown (1UL) /*!< Pull down on pin */
#define GPIO_PIN_CNF_PULL_Pullup (3UL) /*!< Pull up on pin */

/* Bit 1 : Connect or disconnect input buffer */
#define GPIO_PIN_CNF_INPUT_Pos (1UL) /*!< Position of INPUT field. */
#define GPIO_PIN_CNF_INPUT_Msk (0x1UL << GPIO_PIN_CNF_INPUT_Pos) /*!< Bit mask of INPUT field. */
#define GPIO_PIN_CNF_INPUT_Connect (0UL) /*!< Connect input buffer */
#define GPIO_PIN_CNF_INPUT_Disconnect (1UL) /*!< Disconnect input buffer */

/* Bit 0-15: Pin direction. Same physical register as DIR register */
#define GPIO_PIN_CNF_DIR_Pos (0UL) /*!< Position of DIR field. */
#define GPIO_PIN_CNF_DIR_Msk (0x1UL << GPIO_PIN_CNF_DIR_Pos) /*!< Bit mask of DIR field. */
#define GPIO_PIN_CNF_DIR_Input (0UL) /*!< Configure pin as an input pin */
#define GPIO_PIN_CNF_DIR_Output (1UL) /*!< Configure pin as an output pin */




/* Bit 1 : Write '1' to disable interrupt for event END */
#define SAADC_INTENCLR_END_Pos (1UL) /*!< Position of END field. */
#define SAADC_INTENCLR_END_Msk (0x1UL << SAADC_INTENCLR_END_Pos) /*!< Bit mask of END field. */
#define SAADC_INTENCLR_END_Disabled (0UL) /*!< Read: Disabled */
#define SAADC_INTENCLR_END_Enabled (1UL) /*!< Read: Enabled */
#define SAADC_INTENCLR_END_Clear (1UL) /*!< Disable */

/* Bit 0 : Write '1' to disable interrupt for event STARTED */
#define SAADC_INTENCLR_STARTED_Pos (0UL) /*!< Position of STARTED field. */
#define SAADC_INTENCLR_STARTED_Msk (0x1UL << SAADC_INTENCLR_STARTED_Pos) /*!< Bit mask of STARTED field. */
#define SAADC_INTENCLR_STARTED_Disabled (0UL) /*!< Read: Disabled */
#define SAADC_INTENCLR_STARTED_Enabled (1UL) /*!< Read: Enabled */
#define SAADC_INTENCLR_STARTED_Clear (1UL) /*!< Disable */

/* Register: SAADC_STATUS */
/* Description: Status */

/* Bit 0 : Status */
#define SAADC_STATUS_STATUS_Pos (0UL) /*!< Position of STATUS field. */
#define SAADC_STATUS_STATUS_Msk (0x1UL << SAADC_STATUS_STATUS_Pos) /*!< Bit mask of STATUS field. */
#define SAADC_STATUS_STATUS_Ready (0UL) /*!< SAADC is ready. No on-going conversions. */
#define SAADC_STATUS_STATUS_Busy (1UL) /*!< SAADC is busy. Conversion in progress. */

/* Register: SAADC_ENABLE */
/* Description: Enable or disable SAADC */

/* Bit 0 : Enable or disable SAADC */
#define SAADC_ENABLE_ENABLE_Pos (0UL) /*!< Position of ENABLE field. */
#define SAADC_ENABLE_ENABLE_Msk (0x1UL << SAADC_ENABLE_ENABLE_Pos) /*!< Bit mask of ENABLE field. */
#define SAADC_ENABLE_ENABLE_Disabled (0UL) /*!< Disable SAADC */
#define SAADC_ENABLE_ENABLE_Enabled (1UL) /*!< Enable SAADC */

/* Register: SAADC_CH_PSELP */
/* Description: Description cluster: Input positive pin selection for CH[n] */

/* Bits 4..0 : Analog positive input channel */
#define SAADC_CH_PSELP_PSELP_Pos (0UL) /*!< Position of PSELP field. */
#define SAADC_CH_PSELP_PSELP_Msk (0x1FUL << SAADC_CH_PSELP_PSELP_Pos) /*!< Bit mask of PSELP field. */
#define SAADC_CH_PSELP_PSELP_NC (0UL) /*!< Not connected */
#define SAADC_CH_PSELP_PSELP_AnalogInput0 (1UL) /*!< AIN0 */
#define SAADC_CH_PSELP_PSELP_AnalogInput1 (2UL) /*!< AIN1 */
#define SAADC_CH_PSELP_PSELP_AnalogInput2 (3UL) /*!< AIN2 */
#define SAADC_CH_PSELP_PSELP_AnalogInput3 (4UL) /*!< AIN3 */
#define SAADC_CH_PSELP_PSELP_AnalogInput4 (5UL) /*!< AIN4 */
#define SAADC_CH_PSELP_PSELP_AnalogInput5 (6UL) /*!< AIN5 */
#define SAADC_CH_PSELP_PSELP_AnalogInput6 (7UL) /*!< AIN6 */
#define SAADC_CH_PSELP_PSELP_AnalogInput7 (8UL) /*!< AIN7 */
#define SAADC_CH_PSELP_PSELP_VDD (9UL) /*!< VDD */
#define SAADC_CH_PSELP_PSELP_VDDHDIV5 (0x0DUL) /*!< VDDH/5 */

/* Register: SAADC_CH_PSELN */
/* Description: Description cluster: Input negative pin selection for CH[n] */

/* Bits 4..0 : Analog negative input, enables differential channel */
#define SAADC_CH_PSELN_PSELN_Pos (0UL) /*!< Position of PSELN field. */
#define SAADC_CH_PSELN_PSELN_Msk (0x1FUL << SAADC_CH_PSELN_PSELN_Pos) /*!< Bit mask of PSELN field. */
#define SAADC_CH_PSELN_PSELN_NC (0UL) /*!< Not connected */
#define SAADC_CH_PSELN_PSELN_AnalogInput0 (1UL) /*!< AIN0 */
#define SAADC_CH_PSELN_PSELN_AnalogInput1 (2UL) /*!< AIN1 */
#define SAADC_CH_PSELN_PSELN_AnalogInput2 (3UL) /*!< AIN2 */
#define SAADC_CH_PSELN_PSELN_AnalogInput3 (4UL) /*!< AIN3 */
#define SAADC_CH_PSELN_PSELN_AnalogInput4 (5UL) /*!< AIN4 */
#define SAADC_CH_PSELN_PSELN_AnalogInput5 (6UL) /*!< AIN5 */
#define SAADC_CH_PSELN_PSELN_AnalogInput6 (7UL) /*!< AIN6 */
#define SAADC_CH_PSELN_PSELN_AnalogInput7 (8UL) /*!< AIN7 */
#define SAADC_CH_PSELN_PSELN_VDD (9UL) /*!< VDD */
#define SAADC_CH_PSELN_PSELN_VDDHDIV5 (0x0DUL) /*!< VDDH/5 */

/* Register: SAADC_CH_CONFIG */
/* Description: Description cluster: Input configuration for CH[n] */

/* Bit 24 : Enable burst mode */
#define SAADC_CH_CONFIG_BURST_Pos (24UL) /*!< Position of BURST field. */
#define SAADC_CH_CONFIG_BURST_Msk (0x1UL << SAADC_CH_CONFIG_BURST_Pos) /*!< Bit mask of BURST field. */
#define SAADC_CH_CONFIG_BURST_Disabled (0UL) /*!< Burst mode is disabled (normal operation) */
#define SAADC_CH_CONFIG_BURST_Enabled (1UL) /*!< Burst mode is enabled. SAADC takes 2^OVERSAMPLE number of samples as fast as it can, and sends the average to Data RAM. */

/* Bit 20 : Enable differential mode */
#define SAADC_CH_CONFIG_MODE_Pos (20UL) /*!< Position of MODE field. */
#define SAADC_CH_CONFIG_MODE_Msk (0x1UL << SAADC_CH_CONFIG_MODE_Pos) /*!< Bit mask of MODE field. */
#define SAADC_CH_CONFIG_MODE_SE (0UL) /*!< Single-ended, PSELN will be ignored, negative input to SAADC shorted to GND */
#define SAADC_CH_CONFIG_MODE_Diff (1UL) /*!< Differential */

/* Bits 18..16 : Acquisition time, the time the SAADC uses to sample the input voltage */
#define SAADC_CH_CONFIG_TACQ_Pos (16UL) /*!< Position of TACQ field. */
#define SAADC_CH_CONFIG_TACQ_Msk (0x7UL << SAADC_CH_CONFIG_TACQ_Pos) /*!< Bit mask of TACQ field. */
#define SAADC_CH_CONFIG_TACQ_3us (0UL) /*!< 3 us */
#define SAADC_CH_CONFIG_TACQ_5us (1UL) /*!< 5 us */
#define SAADC_CH_CONFIG_TACQ_10us (2UL) /*!< 10 us */
#define SAADC_CH_CONFIG_TACQ_15us (3UL) /*!< 15 us */
#define SAADC_CH_CONFIG_TACQ_20us (4UL) /*!< 20 us */
#define SAADC_CH_CONFIG_TACQ_40us (5UL) /*!< 40 us */

/* Bit 12 : Reference control */
#define SAADC_CH_CONFIG_REFSEL_Pos (12UL) /*!< Position of REFSEL field. */
#define SAADC_CH_CONFIG_REFSEL_Msk (0x1UL << SAADC_CH_CONFIG_REFSEL_Pos) /*!< Bit mask of REFSEL field. */
#define SAADC_CH_CONFIG_REFSEL_Internal (0UL) /*!< Internal reference (0.6 V) */
#define SAADC_CH_CONFIG_REFSEL_VDD1_4 (1UL) /*!< VDD/4 as reference */

/* Bits 10..8 : Gain control */
#define SAADC_CH_CONFIG_GAIN_Pos (8UL) /*!< Position of GAIN field. */
#define SAADC_CH_CONFIG_GAIN_Msk (0x7UL << SAADC_CH_CONFIG_GAIN_Pos) /*!< Bit mask of GAIN field. */
#define SAADC_CH_CONFIG_GAIN_Gain1_6 (0UL) /*!< 1/6 */
#define SAADC_CH_CONFIG_GAIN_Gain1_5 (1UL) /*!< 1/5 */
#define SAADC_CH_CONFIG_GAIN_Gain1_4 (2UL) /*!< 1/4 */
#define SAADC_CH_CONFIG_GAIN_Gain1_3 (3UL) /*!< 1/3 */
#define SAADC_CH_CONFIG_GAIN_Gain1_2 (4UL) /*!< 1/2 */
#define SAADC_CH_CONFIG_GAIN_Gain1 (5UL) /*!< 1 */
#define SAADC_CH_CONFIG_GAIN_Gain2 (6UL) /*!< 2 */
#define SAADC_CH_CONFIG_GAIN_Gain4 (7UL) /*!< 4 */

/* Bits 5..4 : Negative channel resistor control */
#define SAADC_CH_CONFIG_RESN_Pos (4UL) /*!< Position of RESN field. */
#define SAADC_CH_CONFIG_RESN_Msk (0x3UL << SAADC_CH_CONFIG_RESN_Pos) /*!< Bit mask of RESN field. */
#define SAADC_CH_CONFIG_RESN_Bypass (0UL) /*!< Bypass resistor ladder */
#define SAADC_CH_CONFIG_RESN_Pulldown (1UL) /*!< Pull-down to GND */
#define SAADC_CH_CONFIG_RESN_Pullup (2UL) /*!< Pull-up to VDD */
#define SAADC_CH_CONFIG_RESN_VDD1_2 (3UL) /*!< Set input at VDD/2 */

/* Bits 1..0 : Positive channel resistor control */
#define SAADC_CH_CONFIG_RESP_Pos (0UL) /*!< Position of RESP field. */
#define SAADC_CH_CONFIG_RESP_Msk (0x3UL << SAADC_CH_CONFIG_RESP_Pos) /*!< Bit mask of RESP field. */
#define SAADC_CH_CONFIG_RESP_Bypass (0UL) /*!< Bypass resistor ladder */
#define SAADC_CH_CONFIG_RESP_Pulldown (1UL) /*!< Pull-down to GND */
#define SAADC_CH_CONFIG_RESP_Pullup (2UL) /*!< Pull-up to VDD */
#define SAADC_CH_CONFIG_RESP_VDD1_2 (3UL) /*!< Set input at VDD/2 */

/* Register: SAADC_CH_LIMIT */
/* Description: Description cluster: High/low limits for event monitoring of a channel */

/* Bits 31..16 : High level limit */
#define SAADC_CH_LIMIT_HIGH_Pos (16UL) /*!< Position of HIGH field. */
#define SAADC_CH_LIMIT_HIGH_Msk (0xFFFFUL << SAADC_CH_LIMIT_HIGH_Pos) /*!< Bit mask of HIGH field. */

/* Bits 15..0 : Low level limit */
#define SAADC_CH_LIMIT_LOW_Pos (0UL) /*!< Position of LOW field. */
#define SAADC_CH_LIMIT_LOW_Msk (0xFFFFUL << SAADC_CH_LIMIT_LOW_Pos) /*!< Bit mask of LOW field. */

/* Register: SAADC_RESOLUTION */
/* Description: Resolution configuration */

/* Bits 2..0 : Set the resolution */
#define SAADC_RESOLUTION_VAL_Pos (0UL) /*!< Position of VAL field. */
#define SAADC_RESOLUTION_VAL_Msk (0x7UL << SAADC_RESOLUTION_VAL_Pos) /*!< Bit mask of VAL field. */
#define SAADC_RESOLUTION_VAL_8bit (0UL) /*!< 8 bits */
#define SAADC_RESOLUTION_VAL_10bit (1UL) /*!< 10 bits */
#define SAADC_RESOLUTION_VAL_12bit (2UL) /*!< 12 bits */
#define SAADC_RESOLUTION_VAL_14bit (3UL) /*!< 14 bits */

/* Register: SAADC_OVERSAMPLE */
/* Description: Oversampling configuration. The RESOLUTION is applied before averaging, thus for high OVERSAMPLE a higher RESOLUTION should be used. */

/* Bits 3..0 : Oversample control */
#define SAADC_OVERSAMPLE_OVERSAMPLE_Pos (0UL) /*!< Position of OVERSAMPLE field. */
#define SAADC_OVERSAMPLE_OVERSAMPLE_Msk (0xFUL << SAADC_OVERSAMPLE_OVERSAMPLE_Pos) /*!< Bit mask of OVERSAMPLE field. */
#define SAADC_OVERSAMPLE_OVERSAMPLE_Bypass (0UL) /*!< Bypass oversampling */
#define SAADC_OVERSAMPLE_OVERSAMPLE_Over2x (1UL) /*!< Oversample 2x */
#define SAADC_OVERSAMPLE_OVERSAMPLE_Over4x (2UL) /*!< Oversample 4x */
#define SAADC_OVERSAMPLE_OVERSAMPLE_Over8x (3UL) /*!< Oversample 8x */
#define SAADC_OVERSAMPLE_OVERSAMPLE_Over16x (4UL) /*!< Oversample 16x */
#define SAADC_OVERSAMPLE_OVERSAMPLE_Over32x (5UL) /*!< Oversample 32x */
#define SAADC_OVERSAMPLE_OVERSAMPLE_Over64x (6UL) /*!< Oversample 64x */
#define SAADC_OVERSAMPLE_OVERSAMPLE_Over128x (7UL) /*!< Oversample 128x */
#define SAADC_OVERSAMPLE_OVERSAMPLE_Over256x (8UL) /*!< Oversample 256x */

/* Register: SAADC_SAMPLERATE */
/* Description: Controls normal or continuous sample rate */

/* Bit 12 : Select mode for sample rate control */
#define SAADC_SAMPLERATE_MODE_Pos (12UL) /*!< Position of MODE field. */
#define SAADC_SAMPLERATE_MODE_Msk (0x1UL << SAADC_SAMPLERATE_MODE_Pos) /*!< Bit mask of MODE field. */
#define SAADC_SAMPLERATE_MODE_Task (0UL) /*!< Rate is controlled from SAMPLE task */
#define SAADC_SAMPLERATE_MODE_Timers (1UL) /*!< Rate is controlled from local timer (use CC to control the rate) */

/* Bits 10..0 : Capture and compare value. Sample rate is 16 MHz/CC */
#define SAADC_SAMPLERATE_CC_Pos (0UL) /*!< Position of CC field. */
#define SAADC_SAMPLERATE_CC_Msk (0x7FFUL << SAADC_SAMPLERATE_CC_Pos) /*!< Bit mask of CC field. */

/* Register: SAADC_RESULT_PTR */
/* Description: Data pointer */

/* Bits 31..0 : Data pointer */
#define SAADC_RESULT_PTR_PTR_Pos (0UL) /*!< Position of PTR field. */
#define SAADC_RESULT_PTR_PTR_Msk (0xFFFFFFFFUL << SAADC_RESULT_PTR_PTR_Pos) /*!< Bit mask of PTR field. */

/* Register: SAADC_RESULT_MAXCNT */
/* Description: Maximum number of 16-bit samples to be written to output RAM buffer */

/* Bits 14..0 : Maximum number of 16-bit samples to be written to output RAM buffer */
#define SAADC_RESULT_MAXCNT_MAXCNT_Pos (0UL) /*!< Position of MAXCNT field. */
#define SAADC_RESULT_MAXCNT_MAXCNT_Msk (0x7FFFUL << SAADC_RESULT_MAXCNT_MAXCNT_Pos) /*!< Bit mask of MAXCNT field. */

/* Register: SAADC_RESULT_AMOUNT */
/* Description: Number of 16-bit samples written to output RAM buffer since the previous START task */

/* Bits 14..0 : Number of 16-bit samples written to output RAM buffer since the previous START task. This register can be read after an END or STOPPED event. */
#define SAADC_RESULT_AMOUNT_AMOUNT_Pos (0UL) /*!< Position of AMOUNT field. */
#define SAADC_RESULT_AMOUNT_AMOUNT_Msk (0x7FFFUL << SAADC_RESULT_AMOUNT_AMOUNT_Pos) /*!< Bit mask of AMOUNT field. */




/* Peripheral: SPIM */
/* Description: Serial Peripheral Interface Master with EasyDMA 0 */


/* Register: SPIM_FREQUENCY */
/* Description: SPI frequency. Accuracy depends on the HFCLK source selected. */

/* Bits 31..0 : SPI master data rate */
#define SPIM_FREQUENCY_FREQUENCY_Pos (0UL) /*!< Position of FREQUENCY field. */
#define SPIM_FREQUENCY_FREQUENCY_Msk (0xFFFFFFFFUL << SPIM_FREQUENCY_FREQUENCY_Pos) /*!< Bit mask of FREQUENCY field. */

#define SPIM_FREQUENCY_FREQUENCY_K125 (0x00000100UL) /*!< 125 kbps */
#define SPIM_FREQUENCY_FREQUENCY_K250 (0x00000080UL) /*!< 250 kbps */
#define SPIM_FREQUENCY_FREQUENCY_K500 (0x00000040UL) /*!< 500 kbps */

#define SPIM_FREQUENCY_FREQUENCY_M1  (0x00000020UL) /*!< 1 Mbps */
#define SPIM_FREQUENCY_FREQUENCY_M2  (0x00000010UL) /*!< 2 Mbps */
#define SPIM_FREQUENCY_FREQUENCY_M4  (0x00000008UL) /*!< 4 Mbps */
#define SPIM_FREQUENCY_FREQUENCY_M8  (0x00000004UL) /*!< 8 Mbps */ 
#define SPIM_FREQUENCY_FREQUENCY_M16 (0x00000002UL) /*!< 16 Mbps */  

/* Register: SPIM_RXD_PTR */
/* Description: Data pointer */

/* Bits 31..0 : Data pointer */
#define SPIM_RXD_PTR_PTR_Pos (0UL) /*!< Position of PTR field. */
#define SPIM_RXD_PTR_PTR_Msk (0xFFFFFFFFUL << SPIM_RXD_PTR_PTR_Pos) /*!< Bit mask of PTR field. */



/* Register: SPIM_TXD_PTR */
/* Description: Data pointer */

/* Bits 31..0 : Data pointer */
#define SPIM_TXD_PTR_PTR_Pos (0UL) /*!< Position of PTR field. */
#define SPIM_TXD_PTR_PTR_Msk (0xFFFFFFFFUL << SPIM_TXD_PTR_PTR_Pos) /*!< Bit mask of PTR field. */

/* Register: SPIM_TXD_MAXCNT */
/* Description: Number of bytes in transmit buffer */




/* Register: SPIM_CONFIG */
/* Description: Configuration register */

/* Bit 2 : Serial clock (SCK) polarity */
#define SPIM_CONFIG_CPOL_Pos (2UL) /*!< Position of CPOL field. */
#define SPIM_CONFIG_CPOL_Msk (0x1UL << SPIM_CONFIG_CPOL_Pos) /*!< Bit mask of CPOL field. */
#define SPIM_CONFIG_CPOL_ActiveHigh (0UL) /*!< Active high */
#define SPIM_CONFIG_CPOL_ActiveLow (1UL) /*!< Active low */

/* Bit 1 : Serial clock (SCK) phase */
#define SPIM_CONFIG_CPHA_Pos (1UL) /*!< Position of CPHA field. */
#define SPIM_CONFIG_CPHA_Msk (0x1UL << SPIM_CONFIG_CPHA_Pos) /*!< Bit mask of CPHA field. */
#define SPIM_CONFIG_CPHA_Leading (0UL) /*!< Sample on leading edge of clock, shift serial data on trailing edge */
#define SPIM_CONFIG_CPHA_Trailing (1UL) /*!< Sample on trailing edge of clock, shift serial data on leading edge */

/* Bit 0 : Bit order */
#define SPIM_CONFIG_ORDER_Pos (0UL) /*!< Position of ORDER field. */
#define SPIM_CONFIG_ORDER_Msk (0x1UL << SPIM_CONFIG_ORDER_Pos) /*!< Bit mask of ORDER field. */
#define SPIM_CONFIG_ORDER_MsbFirst (0UL) /*!< Most significant bit shifted out first */
#define SPIM_CONFIG_ORDER_LsbFirst (1UL) /*!< Least significant bit shifted out first */


/* Description: Polarity of CSN output */

/* Bit 0 : Polarity of CSN output */
#define SPIM_CSNPOL_CSNPOL_Pos (0UL) /*!< Position of CSNPOL field. */
#define SPIM_CSNPOL_CSNPOL_Msk (0x1UL << SPIM_CSNPOL_CSNPOL_Pos) /*!< Bit mask of CSNPOL field. */
#define SPIM_CSNPOL_CSNPOL_LOW (0UL) /*!< Active low (idle state high) */
#define SPIM_CSNPOL_CSNPOL_HIGH (1UL) /*!< Active high (idle state low) */









/////////////////////////////////////////////////////////////////////// XINCHIP TIMER   //////////////////////////////////////////////////////

/* Peripheral: TIMERx */
/* Description: TIMERx Interface  */

/* Register:TIMERx_TLC    RW */
/* Description:计数值载入寄存器
只能在定时器不使能时，对该寄存器做“写”操作。在定时器已经使能后，对该寄存器
进行“写”操作，会得到不可预知的结果。 */

/* Bit 0..31 : TIMERx TLC   RW */
/* Description: 载入计数器计数初值：计数器计数的初
值，该值应大于等于 0x4。 */
#define TIMERx_TLC_TLC_Pos (0UL) /*!< Position of TLC field. */
#define TIMERx_TLC_TLC_Msk (0xFFFFFFFFUL << TIMERx_TLC_TLC_Pos) /*!< Bit mask of TLC field. */
#define TIMERx_TLC_TLC_MIN_VAL (0x4)


/* Register:TIMER_TCV    R */
/* Description:读该寄存器可以得到计数器当前值。
 */
/* Bit 0..31 : TIMER TCV   RW */
/* Description: 当前计数器的值：显示当前计数器的计
数值。*/
#define TIMERx_TCV_TCV_Pos (0UL) /*!< Position of TCV field. */
#define TIMERx_TCV_TCV_Msk (0xFFFFFFFFUL << TIMERx_TCV_TCV_Pos) /*!< Bit mask of TCV field. */



/* Register:TIMERx_TCR    RW */
/* Description:控制寄存器
 */
/* Bit 0 : TIMERx TES   RW */
*/
#define TIMERx_TCR_TES_Pos (0UL) /*!< Position of TES field. */
#define TIMERx_TCR_TES_Msk (0x1UL << TIMERx_TCR_TES_Pos) /*!< Bit mask of TES field. */
#define TIMERx_TCR_TES_Enable (1UL) /*!< 使能定时器 */
#define TIMERx_TCR_TES_Disable (0UL) /*!< 不使能定时器 */

/* Bit 1 : TIMER TMS   RW */
/* Description: 定时器模式选择：
*/
#define TIMERx_TCR_TMS_Pos (1UL) /*!< Position of TMS field. */
#define TIMERx_TCR_TMS_Msk (0x1UL << TIMERx_TCR_TMS_Pos) /*!< Bit mask of TMS field. */
#define TIMERx_TCR_TES_MODE_AUTO (0UL) /*!< 自运行模式 */
#define TIMERx_TCR_TES_MODE_USER_COUNTER (1UL) /*!< 用户定义计数模式 */

/* Bit 2 : TIMERx TIM   RW */
/* Description:定时器中断屏蔽：
*/
#define TIMERx_TCR_TIM_Pos (2UL) /*!< Position of TIM field. */
#define TIMERx_TCR_TIM_Msk (0x1UL << TIMERx_TCR_TIM_Pos) /*!< Bit mask of TIM field. */
#define TIMERx_TCR_TIM_Disable (0UL) /*!< 不屏蔽定时器中断 */
#define TIMERx_TCR_TIM_Enable (1UL) /*!< 屏蔽定时器中断 */



/* Register:TIMERx_TIC    R */
/* Description:中断清除寄存器
 */
/* Bit 0 : TIMERx TIC   R */
/* Description: 中断清除：读该寄存器将清除定时器中断，
因此该寄存器的值也被清 0。
*/
#define TIMERx_TIC_TIC_Pos (0UL) /*!< Position of TIC field. */
#define TIMERx_TIC_TIC_Msk (0x1UL << TIMERx_TIC_TIC_Pos) /*!< Bit mask of TIC field. */


/* Register:TIMERx_TIS    R */
/* Description:中断状态寄存器
 */
/* Bit 0 : TIMERx TIS   R */
/* Description: 中断状态标志：标志定时器的中断状态，若
中断被屏蔽，则该值标志中断屏蔽后的中断
状态。对该寄存器进行读操作不会影响定时
器的中断。
*/
#define TIMERx_TIS_TIS_Pos (0UL) /*!< Position of TIS field. */
#define TIMERx_TIS_TIS_Msk (0x1UL << TIMERx_TIS_TIS_Pos) /*!< Bit mask of TIS field. */
#define TIMERx_TIS_TIS_Generated (1UL) /*!< 定时器发生中断 */
#define TIMERx_TIS_TIS_NotGenerated (0UL) /*!< 定时器没有发生中断 */


/* Register:TIMER_TIS    R */
/* Description:全局中断状态寄存器
标志定时器经过中断屏蔽后的所有的中断状态，对该寄存器进行读操作可以得到 4 个
定时器的中断状态，且不会影响定时器的中断。
 */
/* Bit 0 : TIMER TIS0   R */
/* Description: TIMER0 中断状态标志：标志定时器经过中
断屏蔽后的中断状态，对该位进行读操作不
会影响定时器的中断。
*/
#define TIMER_TIS_TIS0_Pos (0UL) /*!< Position of TIS0 field. */
#define TIMER_TIS_TIS0_Msk (0x1UL << TIMER_TIS_TIS0_Pos) /*!< Bit mask of TIS0 field. */
#define TIMER_TIS_TIS0_Generated (1UL) /*!< 定时器0发生中断 */
#define TIMER_TIS_TIS0_NotGenerated (0UL) /*!< 定时器0没有发生中断 */

/* Bit 1 : TIMER TIS1   R */
/* Description: TIMER1 中断状态标志：标志定时器经过中
断屏蔽后的中断状态，对该位进行读操作不
会影响定时器的中断。
*/
#define TIMER_TIS_TIS1_Pos (1UL) /*!< Position of TIS1 field. */
#define TIMER_TIS_TIS1_Msk (0x1UL << TIMER_TIS_TIS1_Pos) /*!< Bit mask of TIS1 field. */
#define TIMER_TIS_TIS1_Generated (1UL) /*!< 定时器1发生中断 */
#define TIMER_TIS_TIS1_NotGenerated (0UL) /*!< 定时器1没有发生中断 */

/* Bit 0 : TIMER TIS2   R */
/* Description: TIMER2 中断状态标志：标志定时器经过中
断屏蔽后的中断状态，对该位进行读操作不
会影响定时器的中断。
*/
#define TIMER_TIS_TIS2_Pos (2UL) /*!< Position of TIS2 field. */
#define TIMER_TIS_TIS2_Msk (0x1UL << TIMER_TIS_TIS2_Pos) /*!< Bit mask of TIS2 field. */
#define TIMER_TIS_TIS2_Generated (1UL) /*!< 定时器2发生中断 */
#define TIMER_TIS_TIS2_NotGenerated (0UL) /*!< 定时器2没有发生中断 */



/* Register:TIMER_TIC    R */
/* Description:全局中断清除寄存器
对全局中断清除寄存器进行读操作会清除 4 个定时器的全部中断。
 */
/* Bit 0 : TIMER TIC0   R */
/* Description: TIMER0 中断清除：读该位将清除定时器 0
中断，因此该位的值也被清 0。
*/
#define TIMER_TIC_TIC0_Pos (0UL) /*!< Position of TIS field. */
#define TIMER_TIC_TIC0_Msk (0x1UL << TIMER_TIC_TIC0_Pos) /*!< Bit mask of TIC0 field. */
#define TIMER_TIC_TIC0_Generated (1UL) /*!< 定时器0发生中断 */
#define TIMER_TIC_TIC0_NotGenerated (0UL) /*!< 定时器0没有发生中断 */

/* Bit 1 : TIMER TIS1   R */
/* Description: TIMER1 中断清除：读该位将清除定时器 0
中断，因此该位的值也被清 0。
*/
#define TIMER_TIC_TIC1_Pos (1UL) /*!< Position of TIS1 field. */
#define TIMER_TIC_TIC1_Msk (0x1UL << TIMER_TIC_TIC1_Pos) /*!< Bit mask of TIS1 field. */
#define TIMER_TIC_TIC1_Generated (1UL) /*!< 定时器1发生中断 */
#define TIMER_TIC_TIC1_NotGenerated (0UL) /*!< 定时器1没有发生中断 */

/* Bit 0 : TIMER TIC2   R */
/* Description: TIMER2 中断清除：读该位将清除定时器 0
中断，因此该位的值也被清 0。
*/
#define TIMER_TIC_TIC2_Pos (2UL) /*!< Position of TIS2 field. */
#define TIMER_TIC_TIC2_Msk (0x1UL << TIMER_TIC_TIC2_Pos) /*!< Bit mask of TIS2 field. */
#define TIMER_TIC_TIC2_Generated (1UL) /*!< 定时器2发生中断 */
#define TIMER_TIC_TIC2_NotGenerated (0UL) /*!< 定时器2没有发生中断 */



/* Register:TIMER_RTIS    R */
/* Description:全局中断状态寄存器
标志定时器没有经过中断屏蔽的所有的中断状态，对该寄存器进行读操作可以得到 4
个定时器的原始中断状态，且不会影响定时器的中断。
 */
/* Bit 0 : TIMER RTIS0   R */
/* Description: TIMER0 原始中断状态标志：标志定时器经过中
断屏蔽后的中断状态，对该位进行读操作不
会影响定时器的中断。
*/
#define TIMER_RTIS_RTIS0_Pos (0UL) /*!< Position of RTIS0 field. */
#define TIMER_RTIS_TIS0_Msk (0x1UL << TIMER_RTIS_RTIS0_Pos) /*!< Bit mask of RTIS0 field. */
#define TIMER_RTIS_RTIS0_Generated (1UL) /*!< 定时器0发生中断 */
#define TIMER_RTIS_RTIS0_NotGenerated (0UL) /*!< 定时器0没有发生中断 */

/* Bit 1 : TIMER RTIS1   R */
/* Description: TIMER1 原始中断状态标志：标志定时器经过中
断屏蔽后的中断状态，对该位进行读操作不
会影响定时器的中断。
*/
#define TIMER_RTIS_RTIS1_Pos (1UL) /*!< Position of RTIS1 field. */
#define TIMER_RTIS_RTIS1_Msk (0x1UL << TIMER_RTIS_RTIS1_Pos) /*!< Bit mask of RTIS1 field. */
#define TIMER_RTIS_RTIS1_Generated (1UL) /*!< 定时器1发生中断 */
#define TIMER_RTIS_RTIS1_NotGenerated (0UL) /*!< 定时器1没有发生中断 */

/* Bit 0 : TIMER TIS2   R */
/* Description: TIMER2 原始中断状态标志：标志定时器经过中
断屏蔽后的中断状态，对该位进行读操作不
会影响定时器的中断。
*/
#define TIMER_RTIS_RTIS2_Pos (2UL) /*!< Position of RTIS2 field. */
#define TIMER_RTIS_RTIS2_Msk (0x1UL << TIMER_RTIS_RTIS2_Pos) /*!< Bit mask of RTIS2 field. */
#define TIMER_RTIS_RTIS2_Generated (1UL) /*!< 定时器2发生中断 */
#define TIMER_RTIS_RTIS2_NotGenerated (0UL) /*!< 定时器2没有发生中断 */

/////////////////////////////////////////////////////////////////////// XINCHIP TIMER END  //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////// //////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////// XINCHIP RTC   //////////////////////////////////////////////////////

/* Peripheral: RTC */
/* Description: RTC Interface  */

/* Register:RTC_CCVR    R */
/* Description: RTC 当前计数值寄存器 
 RTC 读该寄存器可以得到计数器当前计数值。 
当前计数值：
用于记录计数器当前计数值。
Bit 31：17：天计数值（0－32767）
Bit 16：12：小时计数值（0－23）
Bit 11：6：分计数值（0－59）
Bit 5：0：秒计数值（0－59） 
*/
/* Bit 0..5 : RTC CCVR_SEC   R */
/* Description: 当前秒计数值 */
#define RTC_CCVR_SEC_Pos (0UL) /*!< Position of CCVR_SEC field. */
#define RTC_CCVR_SEC_Msk (0x3FUL << RTC_CCVR_SEC_Pos) /*!< Bit mask of CCVR_SEC field. */

/* Bit 6..11 : RTC CCVR_MIN   R */
/* Description: 当前秒计数值 */
#define RTC_CCVR_MIN_Pos (6UL) /*!< Position of CCVR_MIN field. */
#define RTC_CCVR_MIN_Msk (0x3FUL << RTC_CCVR_MIN_Pos) /*!< Bit mask of CCVR_MIN field. */

/* Bit 12..16 : RTC CCVR_HOUR   R */
/* Description: 当前秒计数值 */
#define RTC_CCVR_HOUR_Pos (12UL) /*!< Position of CCVR_HOUR field. */
#define RTC_CCVR_HOUR_Msk (0x1FUL << RTC_CCVR_HOUR_Pos) /*!< Bit mask of CCVR_HOUR field. */

/* Bit 17..31: RTC CCVR_DAY   R */
/* Description: 当前秒计数值 */
#define RTC_CCVR_DAY_Pos (17UL) /*!< Position of CCVR_DAY field. */
#define RTC_CCVR_DAY_Msk (0x7FFFUL << RTC_CCVR_DAY_Pos) /*!< Bit mask of CCVR_DAY field. */



/* Register:RTC_CLR  WR*/
/* Description: RTC 计数器初始值设置寄存器 
 RTC 读该寄存器可以得到计数器当前计数值。 */

/* Bit 0..5 : RTC CLR_SEC   R */
/* Description: 当前秒计数值 */
#define RTC_CLR_SEC_Pos (0UL) /*!< Position of CLR_SEC field. */
#define RTC_CLR_SEC_Msk (0x3FUL << RTC_CLR_SEC_Pos) /*!< Bit mask of CLR_SEC field. */

/* Bit 6..11 : RTC CLR_MIN   R */
/* Description: 当前秒计数值 */
#define RTC_CLR_MIN_Pos (6UL) /*!< Position of CLR_MIN field. */
#define RTC_CLR_MIN_Msk (0x3FUL << RTC_CLR_MIN_Pos) /*!< Bit mask of CLR_MIN field. */

/* Bit 12..16 : RTC CLR_HOUR   R */
/* Description: 当前秒计数值 */
#define RTC_CLR_HOUR_Pos (12UL) /*!< Position of CLR_HOUR field. */
#define RTC_CLR_HOUR_Msk (0x1FUL << RTC_CLR_HOUR_Pos) /*!< Bit mask of CLR_HOUR field. */

/* Bit 17..31: RTC CLR_DAY   R */
/* Description: 当前秒计数值 */
#define RTC_CLR_DAY_Pos (17UL) /*!< Position of CLR_DAY field. */
#define RTC_CLR_DAY_Msk (0x7FFFUL << RTC_CLR_DAY_Pos) /*!< Bit mask of CLR_DAY field. */



/* Register:RTC_CMR  WR*/
/* Description: RTC 定时匹配寄存器 
 用于设置定时中断的发生条件。 */

/* Bit 0..5 : RTC Second Match   WR */
/* Description:用于设置中断产生时的秒计数 */
#define RTC_CMR_SECOND_MATCH_Pos (0UL) /*!< Position of Second Match field. */
#define RTC_CMR_SECOND_MATCH_Msk (0x3FUL << RTC_CMR_SECOND_MATCH_Pos) /*!< Bit mask of Second Match field. */

/* Bit 6..11 : RTC Minute Match   WR */
/* Description:用于设置中断产生时的分钟计数 */
#define RTC_CMR_MINUTE_MATCH_Pos (6UL) /*!< Position of Minute Match field. */
#define RTC_CMR_MINUTE_MATCH_Msk (0x3FUL << RTC_CMR_MINUTE_MATCH_Pos) /*!< Bit mask of Minute Match field. */

/* Bit 12..16 : RTC Hour Match   WR */
/* Description:用于设置中断产生时的时计数 */
#define RTC_CMR_HOUR_MATCH_Pos (12UL) /*!< Position of Hour Match field. */
#define RTC_CMR_HOUR_MATCH_Msk (0x1FUL << RTC_CMR_HOUR_MATCH_Pos) /*!< Bit mask of Hour Match field. */


// Monday, Tuesday, Wednesday, Thursday, Friday, Saturday,Sunday
/* Bit 17..23 : RTC Week Match   WR */
/* Description:用于设置中断产生时的时计数 */
#define RTC_CMR_WEEK_MATCH_Pos (17UL) /*!< Position of Week Match field. */
#define RTC_CMR_WEEK_MATCH_Msk (0x3FUL << RTC_CMR_WEEK_MATCH_Pos) /*!< Bit mask of Week Match field. */

#define RTC_CMR_WEEK_MATCH_Sunday_Pos (17UL) /*!< Position of Week Match Sunday field. */
#define RTC_CMR_WEEK_MATCH_Sunday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Sunday_Pos) /*!< Bit mask of Week  Match Sunday field. */

#define RTC_CMR_WEEK_MATCH_Monday_Pos (18UL) /*!< Position of Week Match Monday field. */
#define RTC_CMR_WEEK_MATCH_Monday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Monday_Pos) /*!< Bit mask of Week Match Monday field. */

#define RTC_CMR_WEEK_MATCH_Tuesday_Pos (19UL) /*!< Position of Week Match Tuesday field. */
#define RTC_CMR_WEEK_MATCH_Tuesday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Tuesday_Pos) /*!< Bit mask of Week Match Tuesday field. */

#define RTC_CMR_WEEK_MATCH_Wednesday_Pos (20UL) /*!< Position of Week Match Wednesday field. */
#define RTC_CMR_WEEK_MATCH_Wednesday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Wednesday_Pos) /*!< Bit mask of Week Match Wednesday field. */

#define RTC_CMR_WEEK_MATCH_Thursday_Pos (21UL) /*!< Position of Week Match Thursday field. */
#define RTC_CMR_WEEK_MATCH_Thursday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Thursday_Pos) /*!< Bit mask of Week Match Thursday field. */

#define RTC_CMR_WEEK_MATCH_Friday_Pos (22UL) /*!< Position of Week Match Friday field. */
#define RTC_CMR_WEEK_MATCH_Friday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Friday_Pos) /*!< Bit mask of Week Match Friday field. */

#define RTC_CMR_WEEK_MATCH_Saturday_Pos (23UL) /*!< Position of Week Match Saturday field. */
#define RTC_CMR_WEEK_MATCH_Saturday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Saturday_Pos) /*!< Bit mask of Week Match Saturday field. */


/* Register:RTC_ICR  WR*/
/* Description: 中断控制寄存器 */

/* Bit 0 : RTC DaE  WR */
/* Description:天中断使能：
设置是否使能天中断 */
#define RTC_ICR_DaE_Pos (0UL) /*!< Position of DaE field. */
#define RTC_ICR_DaE_Msk (0x1UL << RTC_ICR_DaE_Pos) /*!< Bit mask of DaE field. */
#define RTC_ICR_DaE_Enable (1UL) /*!< 使能天中断 */
#define RTC_ICR_DaE_Disable (0UL) /*!< 不使能天中断 */

/* Bit 1 : RTC HoE  WR */
/* Description:小时中断使能：
设置是否使能小时中断 */
#define RTC_ICR_HoE_Pos (1UL) /*!< Position of HoE field. */
#define RTC_ICR_HoE_Msk (0x1UL << RTC_ICR_HoE_Pos) /*!< Bit mask of HoE field. */
#define RTC_ICR_HoE_Enable (1UL) /*!< 使能小时中断 */
#define RTC_ICR_HoE_Disable (0UL) /*!< 不使能小时中断 */

/* Bit 2 : RTC MiE  WR */
/* Description:分中断使能：
设置是否使能分中断 */
#define RTC_ICR_MiE_Pos (2UL) /*!< Position of MiE field. */
#define RTC_ICR_MiE_Msk (0x1UL << RTC_ICR_MiE_Pos) /*!< Bit mask of MiE field. */
#define RTC_ICR_MiE_Enable (1UL) /*!< 使能分中断 */
#define RTC_ICR_MiE_Disable (0UL) /*!< 不使能分中断 */

/* Bit 3 : RTC SeE  WR */
/* Description:秒中断使能：
设置是否使能秒中断 */
#define RTC_ICR_SeE_Pos (3UL) /*!< Position of SeE field. */
#define RTC_ICR_SeE_Msk (0x1UL << RTC_ICR_SeE_Pos) /*!< Bit mask of SeE field. */
#define RTC_ICR_SeE_Enable (1UL) /*!< 使能秒中断 */
#define RTC_ICR_SeE_Disable (0UL) /*!< 不使能秒中断 */



/* Bit 4 : RTC T2E  WR */
/* Description:定时中断 2 使能：
设置是否使能定时中断 2 */
#define RTC_ICR_T2E_Pos (4UL) /*!< Position of T2E field. */
#define RTC_ICR_T2E_Msk (0x1UL << RTC_ICR_T2E_Pos) /*!< Bit mask of T2E field. */
#define RTC_ICR_T2E_Enable (1UL) /*!< 使能定时中断 2 */
#define RTC_ICR_T2E_Disable (0UL) /*!< 不使能定时中断 2 */

/* Bit 5 : RTC T1E  WR */
/* Description:定时中断 1 使能：
设置是否使能定时中断 1 */
#define RTC_ICR_T1E_Pos (5UL) /*!< Position of T2E field. */
#define RTC_ICR_T1E_Msk (0x1UL << RTC_ICR_T1E_Pos) /*!< Bit mask of T2E field. */
#define RTC_ICR_T1E_Enable (1UL) /*!< 使能定时中断 1 */
#define RTC_ICR_T1E_Disable (0UL) /*!< 不使能定时中断 1 */

/* Bit 6 : RTC T3E  WR */
/* Description:定时中断 3 使能：
设置是否使能定时中断 3 */
#define RTC_ICR_T3E_Pos (6UL) /*!< Position of T3E field. */
#define RTC_ICR_T3E_Msk (0x1UL << RTC_ICR_T3E_Pos) /*!< Bit mask of T3E field. */
#define RTC_ICR_T3E_Enable (1UL) /*!< 使能定时中断 3 */
#define RTC_ICR_T3E_Disable (0UL) /*!< 不使能定时中断 3 */


/* Bit 7 : RTC MASK  WR */
/* Description:中断屏蔽：
设置是否屏蔽中断输出 */
#define RTC_ICR_MASK_ALL_Pos (7UL) /*!< Position of MASK field. */
#define RTC_ICR_MASK_ALL_Msk (0x1UL << RTC_ICR_MASK_ALL_Pos) /*!< Bit mask of MASK field. */
#define RTC_ICR_MASK_ALL_Enable (1UL) /*!< 屏蔽所有中断输出 */
#define RTC_ICR_MASK_ALL_Disable (0UL) /*!< 不屏蔽中断输出  */

/* Bit 8 : RTC CntE  WR */
/* Description:计数器使能：
控制计数器是否使能。*/
#define RTC_ICR_CntE_Pos (8UL) /*!< Position of CntE field. */
#define RTC_ICR_CntE_Msk (0x1UL << RTC_ICR_CntE_Pos) /*!< Bit mask of CntE field. */
#define RTC_ICR_CntE_Enable (1UL) /*!< 计数器使能 */
#define RTC_ICR_CntE_Disable (0UL) /*!< 计数器不使能  */



/* Register:RTC_ISR  WR*/
/* Description: 中断状态寄存器 
该寄存器用于显示 RTC 当前中断状态。*/

/* Bit 0 : RTC DaF  WR */
/* Description:中断状态：
该位是天中断的中断状态
*/
#define RTC_ISR_DaF_Pos (0UL) /*!< Position of DaF field. */
#define RTC_ISR_DaF_Msk (0x1UL << RTC_ISR_DaF_Pos) /*!< Bit mask of DaF field. */
#define RTC_ISR_DaF_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_DaF_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 1 : RTC  HoF  WR */
/* Description:中断状态：
该位是小时中断的中断状态
*/
#define RTC_ISR_HoF_Pos (1UL) /*!< Position of HoF field. */
#define RTC_ISR_HoF_Msk (0x1UL << RTC_ISR_HoF_Pos) /*!< Bit mask of HoF field. */
#define RTC_ISR_HoF_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_HoF_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 2 : RTC  MiF  WR */
/* Description:中断状态：
该位是分中断的中断状态
*/
#define RTC_ISR_MiF_Pos (2UL) /*!< Position of MiF field. */
#define RTC_ISR_MiF_Msk (0x1UL << RTC_ISR_MiF_Pos) /*!< Bit mask of MiF field. */
#define RTC_ISR_MiF_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_MiF_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 3 : RTC  SeF  WR */
/* Description:中断状态：
该位是秒中断的中断状态
*/
#define RTC_ISR_SeF_Pos (3UL) /*!< Position of SeF field. */
#define RTC_ISR_SeF_Msk (0x1UL << RTC_ISR_SeF_Pos) /*!< Bit mask of SeF field. */
#define RTC_ISR_SeF_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_SeF_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 4 : RTC  T2F  WR */
/* Description:中断状态：
该位是是定时中断 2的中断状态
*/
#define RTC_ISR_T2F_Pos (4UL) /*!< Position of T2F field. */
#define RTC_ISR_T2F_Msk (0x1UL << RTC_ISR_T2F_Pos) /*!< Bit mask of T2F field. */
#define RTC_ISR_T2F_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_T2F_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 5 : RTC  T1F  WR */
/* Description:中断状态：
该位是是定时中断 1的中断状态
*/
#define RTC_ISR_T1F_Pos (5UL) /*!< Position of T1F field. */
#define RTC_ISR_T1F_Msk (0x1UL << RTC_ISR_T1F_Pos) /*!< Bit mask of T1F field. */
#define RTC_ISR_T1F_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_T1F_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 6 : RTC  T3F  WR */
/* Description:中断状态：
该位是是定时中断 1的中断状态
*/
#define RTC_ISR_T3F_Pos (6UL) /*!< Position of T3F field. */
#define RTC_ISR_T3F_Msk (0x1UL << RTC_ISR_T3F_Pos) /*!< Bit mask of T3F field. */
#define RTC_ISR_T3F_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_T3F_NotGenerated (0UL) /*!< 没有发生中断  */



/* Register:RTC_EOI  WR*/
/* Description: 此寄存器用于清除 RTC 中断。
*/

/* Bit 0 : RTC EOI_da  WR */
/* Description:清天中断：
向寄存器该位写 1 可以清除天中断
*/
#define RTC_EOI_Da_Pos (0UL) /*!< Position of DaF field. */
#define RTC_EOI_Da_Msk (0x1UL << RTC_EOI_Da_Pos) /*!< Bit mask of DaF field. */
#define RTC_EOI_Da_Clear (1UL) /*!< 清天中断： */


/* Bit 1 : RTC  EOI_Ho  WR */
/* Description:中断状态：
该位是小时中断的中断状态
*/
#define RTC_EOI_Ho_Pos (1UL) /*!< Position of EOI_Ho field. */
#define RTC_EOI_Ho_Msk (0x1UL << RTC_EOI_HoF_Pos) /*!< Bit mask of EOI_Ho field. */
#define RTC_EOI_Ho_Clear (1UL) /*!< 清小时中断 */

/* Bit 2 : RTC  EOI_Mi  WR */
/* Description:中断状态：
该位是分中断的中断状态
*/
#define RTC_EOI_Mi_Pos (2UL) /*!< Position of EOI_Mi field. */
#define RTC_EOI_Mi_Msk (0x1UL << RTC_EOI_Mi_Pos) /*!< Bit mask of EOI_Mi field. */
#define RTC_EOI_Mi_Clear (1UL) /*!< 清分中断 */

/* Bit 3 : RTC  EOI_Se  WR */
/* Description:中断状态：
该位是秒中断的中断状态
*/
#define RTC_EOI_Se_Pos (3UL) /*!< Position of EOI_Se field. */
#define RTC_EOI_Se_Msk (0x1UL << RTC_EOI_Se_Pos) /*!< Bit mask of EOI_Se field. */
#define RTC_EOI_Se_Clear (1UL) /*!< 清秒中断 */

/* Bit 4 : RTC  EOI_T2  WR */
/* Description:中断状态：
该位是是定时中断 2的中断状态
*/
#define RTC_EOI_T2_Pos (4UL) /*!< Position of EOI_T2 field. */
#define RTC_EOI_T2_Msk (0x1UL << RTC_EOI_T2_Pos) /*!< Bit mask of EOI_T2 field. */
#define RTC_EOI_T2_Clear (1UL) /*!< 清定时中断 2： */

/* Bit 5 : RTC  EOI_T1  WR */
/* Description:中断状态：
该位是是定时中断 1的中断状态
*/
#define RTC_EOI_T1_Pos (5UL) /*!< Position of EOI_T1 field. */
#define RTC_EOI_T1_Msk (0x1UL << RTC_EOI_T1_Pos) /*!< Bit mask of EOI_T1 field. */
#define RTC_EOI_T1_Clear (1UL) /*!< 清定时中断 1： */

/* Bit 6 : RTC  EOI_T3  WR */
/* Description:中断状态：
该位是是定时中断 1的中断状态
*/
#define RTC_EOI_T3_Pos (6UL) /*!< Position of EOI_T3 field. */
#define RTC_EOI_T3_Msk (0x1UL << RTC_EOI_T3_Pos) /*!< Bit mask of EOI_T3 field. */
#define RTC_EOI_T3_Clear (1UL) /*!< 清定时中断 3： */



/* Register:RTC_WVR  R*/
/* Description: 当前周计数器值寄存器
*/

/* Bit 0..2 : RTC WVR  R */
/* Description:记录当前日期为星期几：
*/
#define RTC_WVR_Pos (0UL) /*!< Position of WVR field. */
#define RTC_WVR_Msk (0x7UL << RTC_WVR_Pos) /*!< Bit mask of WVR field. */

/* Register:RTC_WLR  WR*/
/* Description: 周计数器初值设置寄存器
*/

/* Bit 0..2 : RTC WLR  WR */
/* Description:设置写入 CLR 寄存器的天计数值对应星期几：
(该寄存器和 CLR 有相关性，写 CLR 时注意
WLR 寄存器的值是否匹配)
*/
#define RTC_WLR_Pos (0UL) /*!< Position of WLR field. */
#define RTC_WLR_Msk (0x7UL << RTC_WLR_Pos) /*!< Bit mask of WLR field. */

// Sunday,Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, 

#define RTC_WVR_WLR_Sunday (0UL) /*!< 周日 */
#define RTC_WVR_WLR_Monday (1UL) /*!< 周一 */
#define RTC_WVR_WLR_Tuesday (2UL) /*!< 周二 */
#define RTC_WVR_WLR_Wednesday (3UL) /*!< 周三 */
#define RTC_WVR_WLR_Thursday (4UL) /*!< 周四 */
#define RTC_WVR_WLR_Friday (5UL) /*!< 周五 */
#define RTC_WVR_WLR_Saturday (6UL) /*!< 周六 */



/* Register:RTC_RAW_LIMIT  RW*/
/* Description: 计数频率设置寄存器
*/
/* Bit 0..15 : RTC RAW_LIMIT  WR */
/* Description:RAW_LIMIT 的值为计数 1 秒所需要的
rtc_mclk 时钟周期数。例如，当 rtc_mclk 时
钟频率为 32768Hz 时，计数 1 秒需要 32768
个 rtc_mclk 时钟周期。当计数时钟不等于
32768Hz 时，可以设置 RAW_LIMIT 的值为
实际采用的计数时钟的频率数。
*/
#define RTC_RAW_LIMIT_Pos (0UL) /*!< Position of RAW_LIMIT field. */
#define RTC_RAW_LIMIT_Msk (0xFFFFUL << RTC_RAW_LIMIT_Pos) /*!< Bit mask of RAW_LIMIT field. */

/* Register:RTC_SECOND_LIMIT  RW*/
/* Description: 秒计数上限控制寄存器
*/
/* Bit 0..5 : RTC SECOND_LIMIT  WR */
/* Description:设置秒计数的上限，默认值为 60。主要
用于调试，设置为较小的值，可以减少等
待时间。
*/
#define RTC_SECOND_LIMIT_Pos (0UL) /*!< Position of SECOND_LIMIT field. */
#define RTC_SECOND_LIMIT_Msk (0x1FUL << RTC_SECOND_LIMIT_Pos) /*!< Bit mask of SECOND_LIMIT field. */

/* Register:RTC_MINUTE_LIMIT  RW*/
/* Description: 秒计数上限控制寄存器
*/
/* Bit 0..5 : RTC MINUTE_LIMIT  WR */
/* Description:设置分计数的上限，默认值为 60。主要
用于调试，设置为较小的值，可以减少等
待时间。
*/
#define RTC_MINUTE_LIMIT_Pos (0UL) /*!< Position of MINUTE_LIMIT field. */
#define RTC_MINUTE_LIMIT_Msk (0x1FUL << RTC_MINUTE_LIMIT_Pos) /*!< Bit mask of MINUTE_LIMIT field. */


/* Register:RTC_HOUR_LIMIT  RW*/
/* Description: 秒计数上限控制寄存器
*/
/* Bit 0..4 : RTC HOUR_LIMIT  WR */
/* Description:设置时计数的上限，默认值为 24。主要
用于调试，设置为较小的值，可以减少等
待时间。
*/
#define RTC_HOUR_LIMIT_Pos (0UL) /*!< Position of HOUR_LIMIT field. */
#define RTC_HOUR_LIMIT_Msk (0xFUL << RTC_HOUR_LIMIT_Pos) /*!< Bit mask of HOUR_LIMIT field. */


/* Register:RTC_ISR_RAW  R*/
/* Description: 中断原始状态寄存器
*/
/* Bit 0 : RTC DaF_raw  R */
/* Description:中断原始状态：
该位是天中断的原始中断状态
*/
#define RTC_ISR_RAW_DaF_raw_Pos (0UL) /*!< Position of DaF_raw field. */
#define RTC_ISR_RAW_DaF_raw_Msk (0x1UL << RTC_ISR_RAW_DaF_raw_Pos) /*!< Bit mask of DaF_raw field. */
#define RTC_ISR_RAW_DaF_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_DaF_raw_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 1 : RTC HoF_raw  R */
/* Description:中断原始状态：
该位是小时中断的原始中断状态
*/
#define RTC_ISR_RAW_HoF_raw_Pos (1UL) /*!< Position of HoF_raw field. */
#define RTC_ISR_RAW_HoF_raw_Msk (0x1UL << RTC_ISR_RAW_HoF_raw_Pos) /*!< Bit mask of HoF_raw field. */
#define RTC_ISR_RAW_HoF_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_HoF_raw_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 2 : RTC MiF_raw  R */
/* Description:中断原始状态：
该位是分中断的原始中断状态
*/
#define RTC_ISR_RAW_MiF_raw_Pos (2UL) /*!< Position of MiF_raw field. */
#define RTC_ISR_RAW_MiF_raw_Msk (0x1UL << RTC_ISR_RAW_MiF_raw_Pos) /*!< Bit mask of MiF_raw field. */
#define RTC_ISR_RAW_MiF_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_MiF_raw_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 3 : RTC SeF_raw  R */
/* Description:中断原始状态：
该位是秒中断的原始中断状态
*/
#define RTC_ISR_RAW_SeF_raw_Pos (3UL) /*!< Position of SeF_raw field. */
#define RTC_ISR_RAW_SeF_raw_Msk (0x1UL << RTC_ISR_RAW_SeF_raw_Pos) /*!< Bit mask of SeF_raw field. */
#define RTC_ISR_RAW_SeF_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_SeF_raw_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 4 : RTC T2F_raw  R */
/* Description:中断原始状态：
该位是定时中断 2 的原始中断状态
*/
#define RTC_ISR_RAW_T2F_raw_Pos (4UL) /*!< Position of T2F_raw field. */
#define RTC_ISR_RAW_T2F_raw_Msk (0x1UL << RTC_ISR_RAW_T2F_raw_Pos) /*!< Bit mask of T2F_raw field. */
#define RTC_ISR_RAW_T2F_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_T2F_raw_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 5 : RTC T1F_raw  R */
/* Description:中断原始状态：
该位是定时中断 1 的原始中断状态
*/
#define RTC_ISR_RAW_T1F_raw_Pos (5UL) /*!< Position of T1F_raw field. */
#define RTC_ISR_RAW_T1F_raw_Msk (0x1UL << RTC_ISR_RAW_T2F_raw_Pos) /*!< Bit mask of T1F_raw field. */
#define RTC_ISR_RAW_T1F_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_T1F_raw_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 4 : RTC T3F_raw  R */
/* Description:中断原始状态：
该位是定时中断 3 的原始中断状态
*/
#define RTC_ISR_RAW_T3F_raw_Pos (6UL) /*!< Position of T3F_raw field. */
#define RTC_ISR_RAW_T3F_raw_Msk (0x1UL << RTC_ISR_RAW_T2F_raw_Pos) /*!< Bit mask of T3F_raw field. */
#define RTC_ISR_RAW_T3F_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_T3F_raw_NotGenerated (0UL) /*!< 没有发生中断  */


/* Register:RTC_RVR  R*/
/* Description: 当前秒内计数值寄存器
*/
/* Bit 0..15 : RTC RVR  R */
/* Description:当前秒内计数值：
记录 1 秒计数器的计数值。
*/
#define RTC_RVR_Pos (0UL) /*!< Position of RVR field. */
#define RTC_RVR_Msk (0xFFFFUL << RTC_ISR_RAW_T2F_raw_Pos) /*!< Bit mask of RVR field. */


/* Register:AO_TIMER_CTL  WR*/
/* Description: 16 位计数器控制寄存器
*/
/* Bit 0..15 : RTC AO_TIMER_VALUE  WR */
/* Description:计数器目标值，从 0 计数到该值时产生中断
*/
#define RTC_AO_TIMER_CTL_AO_TIMER_VALUE_Pos (0UL) /*!< Position of AO_TIMER_VALUE field. */
#define RTC_AO_TIMER_CTL_AO_TIMER_VALUE_Msk (0xFFFFUL << RTC_AO_TIMER_CTL_AO_TIMER_VALUE_Pos) /*!< Bit mask of AO_TIMER_VALUE field. */

/* Bit 16 : RTC AO_TIMER_CLR  WR */
/* Description:中断清除
*/
#define RTC_AO_TIMER_CTL_AO_TIMER_CLR_Pos (16UL) /*!< Position of AO_TIMER_CLR field. */
#define RTC_AO_TIMER_CTL_AO_TIMER_CLR_Msk (0x1UL << RTC_AO_TIMER_CTL_AO_TIMER_CLR_Pos) /*!< Bit mask of AO_TIMER_CLR field. */
#define RTC_AO_TIMER_CTL_AO_TIMER_CLR_Clear (1UL) /*!< 中断清除 */

/* Bit 17 : RTC AO_TIMER_EN  WR */
/* Description:计数器使能
*/
#define RTC_AO_TIMER_CTL_AO_TIMER_EN_Pos (17UL) /*!< Position of AO_TIMER_EN field. */
#define RTC_AO_TIMER_CTL_AO_TIMER_EN_Msk (0x1UL << RTC_AO_TIMER_CTL_AO_TIMER_EN_Pos) /*!< Bit mask of AO_TIMER_EN field. */
#define RTC_AO_TIMER_CTL_AO_TIMER_EN_Enable (1UL) /*!< 计数器使能 */
#define RTC_AO_TIMER_CTL_AO_TIMER_EN_Disable (0UL) /*!< 计数器不使能 */

/* Bit 18 : RTC FREQ_TIMER_EN  WR */
/* Description:32K 校准计数器使能
*/
#define RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Pos (18UL) /*!< Position of FREQ_TIMER_EN field. */
#define RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Msk (0x1UL << RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Pos) /*!< Bit mask of FREQ_TIMER_EN field. */
#define RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Enable (1UL) /*!< 32K 校准计数器使能 */
#define RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Disable (0UL) /*!< 32K 校准计数器不使能 */



/* Register:AO_ALL_INTR  R*/
/* Description: AO 中断寄存器
*/
/* Bit 0..4 : RTC AOGPIO_INTR  R */
/* Description:gpio0~4 中断标志
*/
#define RTC_AO_ALL_INTR_AOGPIO_INTR_Pos (0UL) /*!< Position of AOGPIO_INTR field. */
#define RTC_AO_ALL_INTR_AOGPIO_INTR_Msk (0x1FUL << RTC_AO_ALL_INTR_AOGPIO_INTR_Pos) /*!< Bit mask of AOGPIO_INTR field. */

/* Bit 5 : RTC AO_TIMER_INTR  R */
/* Description:gpio0~4 中断标志
*/
#define RTC_AO_ALL_INTR_AO_TIMER_INTR_Pos (5UL) /*!< Position of AO_TIMER_INTR field. */
#define RTC_AO_ALL_INTR_AO_TIMER_INTR_Msk (0x1UL << RTC_AO_ALL_INTR_AO_TIMER_INTR_Pos) /*!< Bit mask of AO_TIMER_INTR field. */


/* Register:FREQ_TIMER_VAL  R*/
/* Description: FREQ_TIMER
*/
/* Bit 0..23 : FREQ_TIMER_VALUE  R */
/* Description:32K 校准计数周期
*/
#define RTC_FREQ_TIMER_VAL_Pos (0UL) /*!< Position of FREQ_TIMER_VALUE field. */
#define RTC_FREQ_TIMER_VAL_Msk (0xFFFFFFUL << RTC_FREQ_TIMER_VAL_Pos) /*!< Bit mask of FREQ_TIMER_VALUE field. */




/////////////////////////////////////////////////////////////////////// XINCHIP RTC END  //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////// //////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////// XINCHIP WDT   //////////////////////////////////////////////////////

/* Peripheral: WDT */
/* Description: WDT Interface  */

/* Register: WDT_CR */
/* Description: WDT 控制寄存器 
该寄存器用于控制 WDT 使能和选择工作模式 */

/* Bit 0 : WDT WDT_EN   */
/* Description: WDT 使能：软件设置 WDT 使能后，任何对该
位写 0 的操作都是无效操作。该位只能被 M0
复位清除。 */
#define WDT_CR_WDT_EN_Pos (0UL) /*!< Position of WDT_EN field. */
#define WDT_CR_WDT_EN_Msk (0x1UL << WDT_CR_WDT_EN_Pos) /*!< Bit mask of WDT_EN field. */
#define WDT_CR_WDT_EN_Enable (1UL) /*!< WDT 不使能 */
#define WDT_CR_WDT_EN_Disable (0UL) /*!< WDT 使能 */

/* Bit 1 : WDT RMOD   */
/* Description: 工作模式   */
#define WDT_CR_RMOD_Pos (1UL) /*!< Position of RMOD field. */
#define WDT_CR_RMOD_Msk (0x1UL << WDT_CR_RMOD_Pos) /*!< Bit mask of RMOD field. */
#define WDT_CR_RMOD_Mode1 (1UL) /*!< 工作模式 1，先产生一个中断，如果在下一个超时来临之前软件没有清除中断，那么会产生复位。 */
#define WDT_CR_RMOD_Mode0 (0UL) /*!< 工作模式 0，直接产生复位信号送给 CPR */

/* Bit 2..4 : WDT RPL   */
/* Description: 复位脉冲宽度：
M0 复位信号，维持多少个 pclk 时钟周期。   */

#define WDT_CR_RPL_Pos (2UL) /*!< Position of RPL field. */
#define WDT_CR_RPL_Msk (0x7UL << WDT_CR_RPL_Pos) /*!< Bit mask of RPL field. */

#define WDT_CR_RPL_2pclk (0UL) /*!< 000：2 pclk 时钟周期 */
#define WDT_CR_RPL_4pclk (1UL) /*!< 001：4 pclk 时钟周期 */
#define WDT_CR_RPL_8pclk (2UL) /*!< 010：8 pclk 时钟周期 */
#define WDT_CR_RPL_16pclk (3UL) /*!< 011：16 pclk 时钟周期 */
#define WDT_CR_RPL_32pclk (4UL) /*!< 100：32 pclk 时钟周期 */
#define WDT_CR_RPL_64pclk (5UL) /*!< 101：64 pclk 时钟周期 */
#define WDT_CR_RPL_128pclk (6UL) /*!< 110：128 pclk 时钟周期 */
#define WDT_CR_RPL_256pclk (7UL) /*!< 111：256 pclk 时钟周期 */



/* Register: WDT_TORR */
/* Description: WDT 超时范围寄存器
该寄存器用于设置超时时间。 */

/* Bit 0..3 : WDT TOP   */
/* Description: 设置超时周期：此字段用于设置 WDT 装载的
计数器值。如果用户想在计数器计数过程中修
改此字段，重启计数器或发生超时，修改后的
值都会被装载入计数器。更详细的描述见 WDT
计数器装载， */
#define WDT_TORR_TOP_Pos (0UL) /*!< Position of WDT_EN field. */
#define WDT_TORR_TOP_Msk (0xFUL << WDT_TORR_TOP_Pos) /*!< Bit mask of WDT_EN field. */
#define WDT_TORR_TOP_VAL_0xFFFF (0UL) /*!< 0000：0xFFFF */
#define WDT_TORR_TOP_VAL_0x1FFFF (1UL) /*!< 0001：0x1FFFF */
#define WDT_TORR_TOP_VAL_0x3FFFF (2UL) /*!< 0010：0x3FFFF */
#define WDT_TORR_TOP_VAL_0x7FFFF (3UL) /*!< 0011：0x7FFFF */
#define WDT_TORR_TOP_VAL_0xFFFFF (4UL) /*!< 0100：0xFFFFF */
#define WDT_TORR_TOP_VAL_0x1FFFFF (5UL) /*!< 0101：0x1FFFFF */
#define WDT_TORR_TOP_VAL_0x3FFFFF (6UL) /*!< 0110：0x3FFFFF */
#define WDT_TORR_TOP_VAL_0x7FFFFF (7UL) /*!< 0111：0x7FFFFF */
#define WDT_TORR_TOP_VAL_0xFFFFFF (8UL) /*!< 1000：0xFFFFFF */
#define WDT_TORR_TOP_VAL_0x1FFFFFF (9UL) /*!< 1001：0x1FFFFFF */
#define WDT_TORR_TOP_VAL_0x3FFFFFF (10UL) /*!< 1010：0x3FFFFFF */
#define WDT_TORR_TOP_VAL_0x7FFFFFF (11UL) /*!< 1011：0x7FFFFFF */
#define WDT_TORR_TOP_VAL_0xFFFFFFF (12UL) /*!< 1100：0xFFFFFFF */
#define WDT_TORR_TOP_VAL_0x1FFFFFFF (13UL) /*!< 1101：0x1FFFFFFF */
#define WDT_TORR_TOP_VAL_0x3FFFFFFF (14UL) /*!< 1110：0x3FFFFFFF */
#define WDT_TORR_TOP_VAL_0x7FFFFFFF (15UL) /*!< 1111：0x7FFFFFFF */



/* Register: WDT_CCVR */
/* Description: 当前计数值寄存器
只读寄存器，用于显示计数器当前计数值。 */

/* Bit 0..31 : WDT CCVR   */
/* Description: 当前计数值：记录当前计数器的计数值。 */
#define WDT_CCVR_CCVR_Pos (0UL) /*!< Position of CCVR field. */
#define WDT_CCVR_CCVR_Msk (0xFFFFFFFUL << WDT_CCVR_CCVR_Pos) /*!< Bit mask of CCVR field. */


/* Register: WDT_CRR */
/* Description:计数器重启寄存器
该寄存器可以设置计数器重启。 */

/* Bit 0..7 : WDT CRR   */
/* Description: 当前计数值：记录当前计数器的计数值。 */
#define WDT_CRR_CRR_Pos (0UL) /*!< Position of CRR field. */
#define WDT_CRR_CRR_Msk (0xFFUL << WDT_CRR_CRR_Pos) /*!< Bit mask of CRR field. */
#define WDT_CRR_CRR_Enable (0x76UL) /*!< 重启计数器：
用户可向该字段写入 0x76，计数器将会重启，
写其它任何值对该字段没有影响。用户可以在
任何时候对该寄存器进行写操作。重启计数器
会清除当前中断。 */

/* Register: WDT_STAT */
/* Description:中断状态寄存器
该寄存器用于显示 WDT 当前中断状态。 */

/* Bit 0 : WDT STAT   */
/* Description: 当前中断状态：显示 WDT 当前中断状态。 */
#define WDT_STAT_STAT_Pos (0UL) /*!< Position of STAT field. */
#define WDT_STAT_STAT_Msk (0x1UL << WDT_STAT_STAT_Pos) /*!< Bit mask of STAT field. */
#define WDT_STAT_STAT_NotGenerated (0x0UL) /*!< 0：没有发生中断 */
#define WDT_STAT_STAT_Generated (0x1UL) /*!< 1：发生中断 */



/* Register: WDT_ICR */
/* Description:中断清除寄存器
只读寄存器，用于清除 WDT 中断。 */

/* Bit 0 : WDT ICR   */
/* Description: 清除中断：读此寄存器会清除 WDT 当前中断。 */
#define WDT_ICR_ICR_Pos (0UL) /*!< Position of ICR field. */
#define WDT_ICR_ICR_Msk (0x1UL << WDT_ICR_ICR_Pos) /*!< Bit mask of ICR field. */
#define WDT_ICR_ICR_NotGenerated (0x0UL) /*!< 0：没有发生中断 */
#define WDT_ICR_ICR_Generated (0x1UL) /*!< 1：发生中断 */





/////////////////////////////////////////////////////////////////////// XINCHIP WDT END  //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////// //////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////// XINCHIP PWM   //////////////////////////////////////////////////////
/* Peripheral: PWM */
/* Description: PWM  Interface */

/* Register: PWM_EN */
/* Description: PWM 使能寄存器 
*/

/* Bit 0 : PWM PWM_EN   */
/* Description:PWM 使能控制位。
此寄存器控制 PWM 内部计数
器及输出的工作状态 */
#define PWM_EN_Pos (0UL) /*!< Position of PWM_EN field. */
#define PWM_EN_Msk (0x1UL << PWM_EN_Pos) /*!< Bit mask of PWM_EN field. */
#define PWM_EN_Enable (1UL) /*!< PWM 使能 */
#define PWM_EN_Disable (0UL) /*!< PWM 不使能*/


/* Register: PWM_UP */
/* Description: PWM 参数更新寄存器 
*/

/* Bit 0 : PWM UPDATE   */
/* Description:PWM 参数更新位。
向此位写 1 时，周期设置寄存
器和占空比设置寄存器中的新
值，只有在 PWM 不使能或者一
个完整的 PWM 波形输出后才
真正生效，然后此位自动清 0；
向此位写 0 无效。*/
#define PWM_UP_UPDATE_Pos (0UL) /*!< Position of UPDATE field. */
#define PWM_UP_UPDATE_Msk (0x1UL << PWM_UP_UPDATE_Pos) /*!< Bit mask of UPDATE field. */
#define PWM_UP_UPDATE_Enable (1UL) /*!< PWM UPDATE 使能 */
#define PWM_UP_UPDATE_Disable (0UL) /*!< 无效 */



/* Register: PWM_RST */
/* Description: PWM 复位寄存器 
*/

/* Bit 0 : PWM RESET   */
/* Description:PWM 模块复位
 * 此寄存器控制 PWM 复位操作 
*/
#define PWM_RST_RESET_Pos (0UL) /*!< Position of RESET field. */
#define PWM_RST_RESET_Msk (0x1UL << PWM_RST_RESET_Pos) /*!< Bit mask of RESET field. */
#define PWM_RST_RESET_Disable (0UL) /*!< PWM 模块正常工作 */
#define PWM_RST_RESET_Enable (1UL) /*!< PWM 模块进行复位 */



/* Register: PWM_P */
/* Description: PWM 周期设置寄存器
*/

/* Bit 0..7 : PWM PERIOD   */
/* Description:PWM PERIOD 用于控制周期计数器
的周期，在周期计数器增加到PERIOD 时，周期计数器归 0，
重新开始计数。
*/
#define PWM_P_PERIOD_Pos (0UL) /*!< Position of PERIOD field. */
#define PWM_P_PERIOD_Msk (0xFFUL << PWM_P_PERIOD_Pos) /*!< Bit mask of PERIOD field. */


/* Register: PWM_OCPY */
/* Description: PWM 占空比设置寄存器
*/

/* Bit 0..7 : PWM OCPY_RATIO   */
/* Description:OCPY_RATIO 用于控制占空
比计数器的占空比，占空比设
置寄存器用于和占空比计数器
进行比较，相等时，PWM 输出
为低电平；计数器由 99 归 0 时，
PWM 输出为高电平。调节
OCPY_RATIO 的值便可调节
占空比。
*/
#define PWM_OCPY_OCPY_RATIO_Pos (0UL) /*!< Position of OCPY_RATIO field. */
#define PWM_OCPY_OCPY_RATIO_Msk (0xFFUL << PWM_OCPY_OCPY_RATIO_Pos) /*!< Bit mask of OCPY_RATIO field. */


/* Register: PWM_COMP_EN */
/* Description: PWM 互补 PWM 输出、死区控制寄存器
*/

/* Bit 0 : PWM PWMCOMPEN   */
/* Description:使能 PWM 的互补信号输出
*/
#define PWM_COMP_EN_PWMCOMPEN_Pos (0UL) /*!< Position of PWMCOMPEN field. */
#define PWM_COMP_EN_PWMCOMPEN_Msk (0x1UL << PWM_COMP_EN_PWMCOMPEN_Pos) /*!< Bit mask of PWMCOMPEN field. */
#define PWM_COMP_EN_PWMCOMPEN_Disable (0UL) /*!< 1 使能 */
#define PWM_COMP_EN_PWMCOMPEN_Enable (1UL) /*!< 0 不使能 */


/* Register: PWM_COMP_TIME */
/* Description: PWM 互补 PWM 输出、死区控制寄存器
*/

/* Bit 0..2 : PWM PWMCOMPTIME   */
/* Description:使能 PWM 的互补信号输出
*/
#define PWM_COMP_TIME_PWMCOMPTIME_Pos (0UL) /*!< Position of PWMCOMPTIME field. */
#define PWM_COMP_TIME_PWMCOMPTIME_Pos_Msk (0xFFUL << PWM_COMP_TIME_PWMCOMPTIME_Pos) /*!< Bit mask of PWMCOMPTIME field. */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_1clk (0UL) /*!<0：死区为 1 个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_2clk (1UL) /*!<1：死区为 2个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_3clk (2UL) /*!<2：死区为 3 个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_4clk (3UL) /*!<3：死区为 4 个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_5clk (4UL) /*!<4：死区为 5 个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_6clk (5UL) /*!<5：死区为 6 个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_7clk (6UL) /*!<6：死区为 7 个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_8clk (7UL) /*!<7：死区为 8 个 pwm_clk 时钟周期 */


///////////////////////////////////////////////////////////////////////XINCHIP I2C   //////////////////////////////////////////////////////


/* Peripheral: I2C */
/* Description: I2C compatible Two-Wire Interface 0 */

/* Register: I2C_CON */
/* Description: I2C 控制寄存器 
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0 : I2C MASTER_MODE   */
#define I2C_CON_MASTER_MODE_Pos (0UL) /*!< Position of MASTER_MODE field. */
#define I2C_CON_MASTER_MODE_Msk (0x1UL << I2C_CON_MASTER_MODE_Pos) /*!< Bit mask of MASTER_MODE field. */
#define I2C_CON_MASTER_MODE_Enable (1UL) /*!< Enable Master Mode */
#define I2C_CON_MASTER_MODE_Disable (0UL) /*!< Enable Master Mode */

/* Bit 1..2  I2C SPEED   */
/* Description: I2C SPEED 选择控制位 */
#define I2C_CON_SPEED_Pos (1UL) /*!< Position of SPEED field. */
#define I2C_CON_SPEED_Msk (0x3UL << I2C_CON_SPEED_Pos) /*!< Bit mask of SPEED field. */
#define I2C_CON_SPEED_STANDARD_Mode (1UL) /*!< Enable Speed standard  0 --100 kbit/s*/
#define I2C_CON_SPEED_FAST_Mode (2UL) /*!< Enable Speed Fast Mode < 400 kbit/s*/

/* Bit 3:  I2C 10BITADDR_SLAVE   */ 
/* Description： Slave 模式时可响应的地址模式   */
#define I2C_CON_10BITADDR_SLAVE_Pos (3UL) /*!< Position of 10BITADDR_SLAVE field. */
#define I2C_CON_10BITADDR_SLAVE_Msk (0x1UL << I2C_CON_10BITADDR_SLAVE_Pos) /*!< Bit mask of 10BITADDR_SLAVE field. */
#define I2C_CON_10BITADDR_SLAVE_Mode (1UL) /*!< 10-bit 地址模式*/
#define I2C_CON_7BITADDR_SLAVE_Mode (0UL) /*!< 7-bit 地址模式*/

/* Bit 4:  I2C 10BITADDR_MASTER_R   */ 
/* Description：Master 模式下地址模式的只读标志位*/
#define I2C_CON_10BITADDR_MASTER_R_Pos (4UL) /*!< Position of 10BITADDR_MASTER_R field. */
#define I2C_CON_10BITADDR_MASTER_R_Msk (0x1UL << I2C_CON_10BITADDR_MASTER_R_Pos) /*!< Bit mask of 10BITADDR_SLAVE field. */
#define I2C_CON_10BITADDR_MASTER_Mode (1UL) /*!< 10-bit 地址模式*/
#define I2C_CON_7BITADDR_MASTER_Mode (0UL) /*!< 7-bit 地址模式*/

/* Bit 5:  I2C RESTART_EN   */ 
/* Description：Master 模式下发出 Restart 的使能位
如果禁止 Restart 指示，则无法
实现：
1. 一次传输发送多个字节
2. 一次传输中改变方向
3. 发出 Start Byte
4. 7字节地址和10字节地址下执
行组合格式传输
5. 10 字节地址下执行读操作
*/
#define I2C_CON_RESTART_EN_Pos (5UL) /*!< Position of RESTART_EN field. */
#define I2C_CON_RESTART_EN_Msk (0x1UL << I2C_CON_RESTART_EN_Pos) /*!< Bit mask of RESTART_EN field. */
#define I2C_CON_RESTART_EN_Enable (1UL) /*!< 使能*/
#define I2C_CON_RESTART_EN_Disable (0UL) /*!< 不使能*/

/* Bit 6:  I2C SLAVE_DISABLE   */ 
/* Description：Slave 模式使能控制位   */
#define I2C_CON_SLAVE_DISABLE_Pos (6UL) /*!< Position of 10BITADDR_MASTER_R field. */
#define I2C_CON_SLAVE_DISABLE_Msk (0x1UL << I2C_CON_SLAVE_DISABLE_Pos) /*!< Bit mask of 10BITADDR_SLAVE field. */
#define I2C_CON_SLAVE_DISABLE_Enable (0UL) /*!< Slave 模式使能*/
#define I2C_CON_SLAVE_DISABLE_Disable (1UL) /*!< Slave 模式不使能*/

/* Register: I2C_TAR */
/* Description: I2C 目标地址寄存器 
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。
*/

/* Bit 0..9  I2C I2C_TAR   */
/* Description： Master 数据传输的目标地址   */
#define I2C_TAR_I2C_TAR_Pos (0UL) /*!< Position of I2C_TAR field. */
#define I2C_TAR_I2C_TAR_Msk (0x1FFUL << I2C_TAR_I2C_TAR_Pos) /*!< Bit mask of 10BITADDR_SLAVE field. */

/* Bit 10:  I2C GC_OR_START   */ 
/* Description： 通用寻址 OR start   
通用寻址时，只有写命令能够被
执 行 ， 如 果 发 出 读 命 令 ， 则
TX_ABRT 置位。保持通用寻址模
式直到 SPECIAL 复位。
如果 SPECIAL＝0 时：无定义。
*/
#define I2C_TAR_GC_OR_START_Pos (10UL) /*!< Position of GC_OR_START field. */
#define I2C_TAR_GC_OR_START_Msk (0x1UL << I2C_TAR_GC_OR_START_Pos) /*!< Bit mask of GC_OR_START field. */
#define I2C_TAR_GC_OR_START_Gc (0UL) /*!< 通用寻址地址*/
#define I2C_TAR_GC_OR_START_Start (1UL) /*!<Start Byte*/

/* Bit 11:  I2C SPECIAL   */ 
/* Description： 确定软件是否执行通用寻址或者
发送 Start Byte 命令   */
#define I2C_TAR_SPECIAL_Pos (11UL) /*!< Position of SPECIAL field. */
#define I2C_TAR_SPECIA_Msk (0x1UL << I2C_TAR_SPECIAL_Pos) /*!< Bit mask of SPECIAL field. */
#define I2C_TAR_SPECIA_Execute (1UL) /*!<执行 GC_OR_START 所描述的特殊 I2C 命令*/
#define I2C_TAR_SPECIA_Ignore (0UL) /*!<忽略 GC_OR_START，正常使用 I2C_TAR*/

/* Bit 12:  I2C I2C_10BITADDR_MASTER   */ 
/* Description： 确定 Master 模式下传输的地址模式  */
#define I2C_TAR_I2C_10BITADDR_MASTER_Pos (12UL) /*!< Position of I2C_10BITADDR_MASTER field. */
#define I2C_TAR_I2C_10BITADDR_MASTER_Msk (0x1UL << I2C_TAR_I2C_10BITADDR_MASTER_Pos) /*!< Bit mask of I2C_10BITADDR_MASTER field. */
#define I2C_TAR_I2C_10BITADDR_Mode (1UL) /*!<10-bit 地址模式*/
#define I2C_TAR_I2C_7BITADDR_Mode (0UL) /*!<7-bit 地址模式*/



/* Register: I2C_SAR */
/* Description: I2C 从地址控制寄存器 
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0..9: I2C I2C_SAR   */
/* Description： Slave 数据传输的目标地址   */
#define I2C_TAR_I2C_SAR_Pos (0UL) /*!< Position of I2C_TAR field. */
#define I2C_TAR_I2C_SAR_Msk (0x1FFUL << I2C_TAR_I2C_SAR_Pos) /*!< Bit mask of I2C_TAR field. */


/* Register: I2C_DATA_CMD */
/* Description: I2C Rx/Tx 数据缓存和命令寄存器  */

/* Bit 0..7:  I2C DAT   */
/* Description： 包含需要发送或者接收到的 I2C 总线数据   */
#define I2C_DATA_CMD_DAT_Pos (0UL) /*!< Position of DAT field. */
#define I2C_DATA_CMD_DAT_Msk (0xFFUL << I2C_DATA_CMD_DAT_Pos) /*!< Bit mask of DAT field. */

/* Bit 8:  I2C CMD   */
/* Description： 读写控制   */
#define I2C_DATA_CMD_CMD_Pos (8UL) /*!< Position of CMD field. */
#define I2C_DATA_CMD_CMD_Msk (0x1UL << I2C_DATA_CMD_CMD_Pos) /*!< Bit mask of CMD field. */
#define I2C_DATA_CMD_CMD_Read (1UL) /*!<读 操作*/
#define I2C_DATA_CMD_CMD_Write (0UL) /*!<写 操作*/


/* Bit 9:  I2C STOP   */
/* Description： 该位控制在一个字节发送、接收
之后是否产生 STOP 信号   */
#define I2C_DATA_CMD_STOP_Pos (9UL) /*!< Position of STOP field. */
#define I2C_DATA_CMD_STOP_Msk (0x1UL << I2C_DATA_CMD_STOP_Pos) /*!< Bit mask of STOP field. */
#define I2C_DATA_CMD_STOP_Set (1UL) 
/*!不管发送 FIFO 是否为空，当
前字节完成后产生 STOP 信号。
发送 FIFO 非空时 I2C 接口模块
会立即产生 START 信号去获取
总线
*/
#define I2C_DATA_CMD_STOP_Clean (0UL) 
/*!不管发送 FIFO 是否为空，当
前字节完成后不产生 STOP 信
号。发送 FIFO 非空时总线根据
I2C_DATA_CMD.CMD 位 设 置
继续发送/接收字节；发送 FIFO
为空时，I2C 接口模块保持 SCL
低电平直到发送 FIFO 中有新的
可用命令
*/

/* Bit 10:  I2C RESTART   */
/* Description： 该位控制在发送或接收一个字节
之前是否产生 RESTART 信号   */
#define I2C_DATA_CMD_RESTART_Pos (10UL) /*!< Position of RESTART field. */
#define I2C_DATA_CMD_RESTART_Msk (0x1UL << I2C_DATA_CMD_RESTART_Pos) /*!< Bit mask of RESTART field. */
#define I2C_DATA_CMD_RESTART_Set (1UL) 
/*!<： 当
I2C_CON.I2C_RESTART_EN
设置为 1 时，不管传输方向是否
改变，都产生 RESTART 信号；
当
I2C_CON.I2C_RESTART_EN
设置为 0 时，会产生 STOP 和
START 信号代替 RESTART 信
号。
*/
#define I2C_DATA_CMD_RESTART_Clear (0UL) 
/*!： 当
I2C_CON.I2C_RESTART_EN
设置为 1 时，不管传输方向是否
改变，都产生 RESTART 信号；
当
I2C_CON.I2C_RESTART_EN
设置为 0 时，会产生 STOP 和
START 信号代替 RESTART 信
号。
*/


/* Register: I2C_SS_SCL_HCNT */
/* Description: 标准速率 I2C 时 钟 高 电 平 计 数 寄 存 器  
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0..15:  I2C_SS_SCL_HCNT    */
/* Description：为确保正确的 I/O 时序，此寄存
位数 名称 类型 复位值 描述
器需要在任何数据传输之前设
置，设置标准模式下 scl 高电平
计数，最小的合法值为 6。 */
#define I2C_SS_SCL_HCNT_Pos (0UL) /*!< Position of I2C_SS_SCL_HCNT field. */
#define I2C_SS_SCL_HCNT_Msk (0xFFFFUL << I2C_SS_SCL_HCNT_Pos) /*!< Bit mask of I2C_SS_SCL_HCNT field. */


/* Register: I2C_SS_SCL_LCNT */
/* Description: 标准速率 I2C 时 钟 低 电 平 计 数 寄 存 器  
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0..15:  I2C_SS_SCL_LCNT    */
/* Description： 为确保正确的 I/O 时序，此寄存器
需要在任何数据传输之前设置，
设置标准模式下 scl 低电平计数，
最小的合法值为 8。   */
#define I2C_SS_SCL_LCNT_Pos (0UL) /*!< Position of I2C_SS_SCL_LCNT field. */
#define I2C_SS_SCL_LCNT_Msk (0xFFFFUL << I2C_SS_SCL_LCNT_Pos) /*!< Bit mask of I2C_SS_SCL_LCNT field. */


/* Register: I2C_FS_SCL_HCNT */
/* Description: 快速速率 I2C 时 钟 高 电 平 计 数 寄 存 器  
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0..15:  I2C_FS_SCL_HCNT    */
/* Description： 为确保正确的 I/O 时序，此寄存
器需要在任何数据传输之前设
置，设置快速模式下 scl 高电平
计数，最小的合法值为 6。  */
#define I2C_FS_SCL_HCNT_Pos (0UL) /*!< Position of I2C_FS_SCL_HCNT field. */
#define I2C_FS_SCL_HCNT_Msk (0xFFFFUL << I2C_FS_SCL_HCNT_Pos) /*!< Bit mask of I2C_FS_SCL_HCNT field. */


/* Register: I2C_FS_SCL_LCNT */
/* Description: 快速速率 I2C 时 钟 低 电 平 计 数 寄 存 器  
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0..15:  I2C_FS_SCL_LCNT    */
/* Description： 为确保正确的 I/O 时序，此寄存
器需要在任何数据传输之前设
置，设置快速模式下 scl 高电平
计数，最小的合法值为 8。   */
#define I2C_FS_SCL_LCNT_Pos (0UL) /*!< Position of I2C_SS_SCL_LCNT field. */
#define I2C_FS_SCL_LCNT_Msk (0xFFFFUL << I2C_FS_SCL_LCNT_Pos) /*!< Bit mask of I2C_FS_SCL_LCNT field. */



#define I2C_FREQUENCY_FREQUENCY_K100 (0x0063484FUL) /*!< 100 kbps */
#define I2C_FREQUENCY_FREQUENCY_K400 (0x00650C13UL) /*!< 400 kbps */


/* Register: I2C_INTR_STAT */
/* Description: I2C 中断状态寄存器 */

/* Bit 0:  I2C RX_UNDER   */
/* Description： RX FIFO 读空标志   */
#define I2C_INTR_STAT_RX_UNDER_Pos (0UL) /*!< Position of RX_UNDER field. */
#define I2C_INTR_STAT_RX_UNDER_Msk (0x1UL << I2C_INTR_STAT_RX_UNDER_Pos) /*!< Bit mask of RX_UNDER field. */
#define I2C_INTR_STAT_RX_UNDER_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_RX_UNDER_Generated (1UL) /*!< RX_FIFO 读空错误. */

/* Bit 1:  I2C RX_OVER   */
/* Description： RX FIFO 写溢出标志   */
#define I2C_INTR_STAT_RX_OVER_Pos (1UL) /*!< Position of RX_OVER field. */
#define I2C_INTR_STAT_RX_OVER_Msk (0x1UL << I2C_INTR_STAT_RX_OVER_Pos) /*!< Bit mask of RX_OVER field. */
#define I2C_INTR_STAT_RX_OVER_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_RX_OVER_Generated (1UL) /*!< RX_FIFO 写溢出错误. */

/* Bit 2:  I2C RX_FULL   */
/* Description： RX FIFO 满标志   */
#define I2C_INTR_STAT_RX_FULL_Pos (2UL) /*!< Position of RX_FULL field. */
#define I2C_INTR_STAT_RX_FULL_Msk (0x1UL << I2C_INTR_STAT_RX_FULL_Pos) /*!< Bit mask of RX_FULL field. */
#define I2C_INTR_STAT_RX_FULL_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_RX_FULL_Generated (1UL) /*!< RX FIFO 数据大于或者等于RX FIFO 阈值。此比特随 FIFO状态自动更新。. */

/* Bit 3:  I2C TX_OVER   */
/* Description： TX FIFO 写溢出标志   */
#define I2C_INTR_STAT_TX_OVER_Pos (3UL) /*!< Position of TX_OVER field. */
#define I2C_INTR_STAT_TX_OVER_Msk (0x1UL << I2C_INTR_STAT_TX_OVER_Pos) /*!< Bit mask of TX_OVER field. */
#define I2C_INTR_STAT_TX_OVER_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_TX_OVER_Generated (1UL) /*!< TX_FIFO 写溢出错误. */

/* Bit 4:  I2C TX_EMPTY   */
/* Description： TX FIFO 空标志   */
#define I2C_INTR_STAT_TX_EMPTY_Pos (4UL) /*!< Position of TX_EMPTY field. */
#define I2C_INTR_STAT_TX_EMPTY_Msk (0x1UL << I2C_INTR_STAT_TX_EMPTY_Pos) /*!< Bit mask of TX_EMPTY field. */
#define I2C_INTR_STAT_TX_EMPTY_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_TX_EMPTY_Generated (1UL) /*!< TX FIFO 数据小于或者等于TX FIFO 阈值。此比特随 FIFO状态自动更新 */

/* Bit 5:  I2C RD_REQ   */
/* Description：作为 slave，当另外一个 master
尝试从 slave 读数据时产生此中
断。  Slave 保持 I2C 总线为等待
状态（SCL=0）直到中断服务被
响应。这表示 slave 被远端的
master 寻址，并且要求其发送数
据。处理器必须响应这个中断，
并 将 待 发 送 的 数 据 写 入
I2C_DATA_CMD 寄存器。 */
#define I2C_INTR_STAT_RD_REQ_Pos (5UL) /*!< Position of RD_REQ field. */
#define I2C_INTR_STAT_RD_REQ_Msk (0x1UL << I2C_INTR_STAT_RD_REQ_Pos) /*!< Bit mask of RD_REQ field. */
#define I2C_INTR_STAT_RD_REQ_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_RD_REQ_Generated (1UL) /*!< 有效*/

/* Bit 6:  I2C TX_ABRT   */
/* Description：传输异常标志 
作为 master，当接口模块无法完
成处理器的命令时，此位置 1 的
条件包括：
1 地址发出之后无 slave 确认信
号
2 被寻址的 slave 没有发出数据
3 仲裁丢失
4 I2C_RESTART_EN 置 0，但
是处理器发出在没有 restart 条
件就无法完成的命令
5 Start Byte 被确认
6 通用寻址地址无确认
7 读请求中断发生时，TX FIFO
位数 名称 类型 复位值 描述
有 残 留 数 据 ， 或 者 当
I2C_RESTART_EN 不使能时，
在数据传输时丢失总线的控制，
又被当作 I2C slaved 使用
8 通用寻址命令之后发出读命
令
9 响应 RD_REQ 之前，处理器
发出读命令
此位一旦置 1，发送和接收 FIFO
自动清空。*/
#define I2C_INTR_STAT_TX_ABRT_Pos (6UL) /*!< Position of TX_ABRT field. */
#define I2C_INTR_STAT_TX_ABRT_Msk (0x1UL << I2C_INTR_STAT_TX_ABRT_Pos) /*!< Bit mask of TX_ABRT field. */
#define I2C_INTR_STAT_TX_ABRT_NotGenerated  (0UL) /*!< 无效. */
#define I2C_INTR_STAT_TX_ABRT_Generated  (1UL) /*!< 有效 */


/* Bit 7:  I2C RX_DONE   */
/* Description： 作为 slave 发送数据时，如果
master 没有响应的话，在发送最
后一个字节后产生此中断，表示
发送结束  */
#define I2C_INTR_STAT_RX_DONE_Pos (7UL) /*!< Position of RX_DONE field. */
#define I2C_INTR_STAT_RX_DONE_Msk (0x1UL << I2C_INTR_STAT_RX_DONE_Pos) /*!< Bit mask of RX_DONE field. */
#define I2C_INTR_STAT_RX_DONE_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_RX_DONE_Generated (1UL) /*!< 有效 ，表示发送结束 . */

/* Bit 8:  I2C ACTIVITY   */
/* Description： 接口模块活动状态，保持置
位直到软件清除 */
#define I2C_INTR_STAT_ACTIVITY_Pos (8UL) /*!< Position of ACTIVITY field. */
#define I2C_INTR_STAT_ACTIVITY_Msk (0x1UL << I2C_INTR_STAT_ACTIVITY_Pos) /*!< Bit mask of ACTIVITY field. */
#define I2C_INTR_STAT_ACTIVITY_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_ACTIVITY_Generated (1UL) /*!< 有效 ，表示发送结束 . */

/* Bit 9:  I2C STOP_DET   */
/* Description： 结束条件标志 */
#define I2C_INTR_STAT_STOP_DET_Pos (9UL) /*!< Position of STOP_DET field. */
#define I2C_INTR_STAT_STOP_DET_Msk (0x1UL << I2C_INTR_STAT_STOP_DET_Pos) /*!< Bit mask of STOP_DET field. */
#define I2C_INTR_STAT_STOP_DET_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_STOP_DET_Generated (1UL) /*!< 有效, 总线出现结束条件 . */

/* Bit 10:  I2C START_DET   */
/* Description： 结束条件标志 */
#define I2C_INTR_STAT_START_DET_Pos (10UL) /*!< Position of START_DET field. */
#define I2C_INTR_STAT_START_DET_Msk (0x1UL << I2C_INTR_STAT_START_DET_Pos) /*!< Bit mask of START_DET field. */
#define I2C_INTR_STAT_START_DET_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_START_DET_Generated (1UL) /*!< 有效, 总线出现开始条件 . */

/* Bit 11:  I2C GEN_CALL   */
/* Description： 通用寻址请求标志 */
#define I2C_INTR_STAT_GEN_CALL_Pos (11UL) /*!< Position of GEN_CALL field. */
#define I2C_INTR_STAT_GEN_CALL_Msk (0x1UL << I2C_INTR_STAT_START_DET_Pos) /*!< Bit mask of GEN_CALL field. */
#define I2C_INTR_STAT_GEN_CALL_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_GEN_CALL_Generated (1UL) /*!< 有效, 收到通用寻址请求，接口模块将接收到的数据放入 RX 缓存中 . */



/* Register: I2C_INTR_EN */
/* Description: I2C 中断使能寄存器 */

/* Bit 0:  I2C EN_RX_UNDER   */
/* Description： RX FIFO 读空中断使能   */
#define I2C_INTR_EN_RX_UNDER_Pos (0UL) /*!< Position of EN_RX_UNDER field. */
#define I2C_INTR_EN_RX_UNDER_Msk (0x1UL << I2C_INTR_EN_RX_UNDER_Pos) /*!< Bit mask of EN_RX_UNDER field. */
#define I2C_INTR_EN_RX_UNDER_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_RX_UNDER_Enable (1UL) /*!< 中断使能. */

/* Bit 1:  I2C EN_RX_OVER   */
/* Description： RX FIFO 写满中断使能   */
#define I2C_INTR_EN_RX_OVER_Pos (1UL) /*!< Position of EN_RX_OVER field. */
#define I2C_INTR_EN_RX_OVER_Msk (0x1UL << I2C_INTR_EN_RX_OVER_Pos) /*!< Bit mask of EN_RX_OVER field. */
#define I2C_INTR_EN_RX_OVER_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_RX_OVER_Enable (1UL) /*!< 中断使能. */

/* Bit 2:  I2C EN_RX_FULL   */
/* Description： RX FIFO 满中断使能   */
#define I2C_INTR_EN_RX_FULL_Pos (2UL) /*!< Position of EN_RX_FULL field. */
#define I2C_INTR_EN_RX_FULL_Msk (0x1UL << I2C_INTR_EN_RX_FULL_Pos) /*!< Bit mask of EN_RX_FULL field. */
#define I2C_INTR_EN_RX_FULL_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_RX_FULL_Enable (1UL) /*!< 中断使能. */

/* Bit 3:  I2C EN_TX_OVER   */
/* Description： RX FIFO 满中断使能   */
#define I2C_INTR_EN_TX_OVER_Pos (3UL) /*!< Position of EN_TX_OVER field. */
#define I2C_INTR_EN_TX_OVER_Msk (0x1UL << I2C_INTR_EN_TX_OVER_Pos) /*!< Bit mask of EN_TX_OVER field. */
#define I2C_INTR_EN_TX_OVER_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_TX_OVER_Enable (1UL) /*!< 中断使能. */

/* Bit 4:  I2C EN_TX_EMPTY   */
/* Description： TX FIFO 空中断使能   */
#define I2C_INTR_EN_TX_EMPTY_Pos (4UL) /*!< Position of EN_TX_EMPTY field. */
#define I2C_INTR_EN_TX_EMPTY_Msk (0x1UL << I2C_INTR_EN_TX_EMPTY_Pos) /*!< Bit mask of EN_TX_EMPTY field. */
#define I2C_INTR_EN_TX_EMPTY_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_TX_EMPTY_Enable (1UL) /*!< 中断使能. */

/* Bit 5:  I2C EN_RD_REQ   */
/* Description： Slave 收到发送数据请求中断   */
#define I2C_INTR_EN_RD_REQ_Pos (5UL) /*!< Position of EN_RD_REQ field. */
#define I2C_INTR_EN_RD_REQ_Msk (0x1UL << I2C_INTR_EN_RD_REQ_Pos) /*!< Bit mask of EN_RD_REQ field. */
#define I2C_INTR_EN_RD_REQ_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_RD_REQ_Enable (1UL) /*!< 中断使能. */

/* Bit 6:  I2C EN_TX_ABRT   */
/* Description： 传输异常中断使能   */
#define I2C_INTR_EN_TX_ABRT_Pos (6UL) /*!< Position of EN_TX_ABRT field. */
#define I2C_INTR_EN_TX_ABRT_Msk (0x1UL << I2C_INTR_EN_TX_ABRT_Pos) /*!< Bit mask of EN_TX_ABRT field. */
#define I2C_INTR_EN_TX_ABRT_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_TX_ABRT_Enable (1UL) /*!< 中断使能. */

/* Bit 7:  I2C EN_RX_DONE   */
/* Description： Slave 发送数据完成中断   */
#define I2C_INTR_EN_RX_DONE_Pos (7UL) /*!< Position of EN_RX_DONE field. */
#define I2C_INTR_EN_RX_DONE_Msk (0x1UL << I2C_INTR_EN_RX_DONE_Pos) /*!< Bit mask of EN_RX_DONE field. */
#define I2C_INTR_EN_RX_DONE_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_RX_DONE_Enable (1UL) /*!< 中断使能. */

/* Bit 8:  I2C EN_ACTIVITY   */
/* Description： 活动状态中断使能   */
#define I2C_INTR_EN_ACTIVITY_Pos (8UL) /*!< Position of EN_ACTIVITY field. */
#define I2C_INTR_EN_ACTIVITY_Msk (0x1UL << I2C_INTR_EN_ACTIVITY_Pos) /*!< Bit mask of EN_ACTIVITY field. */
#define I2C_INTR_EN_ACTIVITY_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_ACTIVITY_Enable (1UL) /*!< 中断使能. */

/* Bit 9:  I2C EN_STOP_DET   */
/* Description： 结束标志中断使能   */
#define I2C_INTR_EN_STOP_DET_Pos (9UL) /*!< Position of EN_STOP_DET field. */
#define I2C_INTR_EN_STOP_DET_Msk (0x1UL << I2C_INTR_EN_STOP_DET_Pos) /*!< Bit mask of EN_STOP_DET field. */
#define I2C_INTR_EN_STOP_DET_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_STOP_DET_Enable (1UL) /*!< 中断使能. */

/* Bit 10:  I2C EN_START_DET   */
/* Description： 开始标志中断使能   */
#define I2C_INTR_EN_START_DET_Pos (10UL) /*!< Position of EN_START_DET field. */
#define I2C_INTR_EN_START_DET_Msk (0x1UL << I2C_INTR_EN_START_DET_Pos) /*!< Bit mask of EN_START_DET field. */
#define I2C_INTR_EN_START_DET_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_START_DET_Enable (1UL) /*!< 中断使能. */

/* Bit 11:  I2C EN_GEN_CALL   */
/* Description： 通用寻址中断使能   */
#define I2C_INTR_EN_GEN_CALL_Pos (11UL) /*!< Position of EN_GEN_CALL field. */
#define I2C_INTR_EN_GEN_CALL_Msk (0x1UL << I2C_INTR_EN_GEN_CALL_Pos) /*!< Bit mask of EN_GEN_CALL field. */
#define I2C_INTR_EN_GEN_CALL_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_GEN_CALL_Enable (1UL) /*!< 中断使能. */



/* Register: I2C_RAW_INTR_STAT */
/* Description: I2C  中断原始状态寄存器 */

/* Bit 0:  I2C R_RX_UNDER   */
/* Description： 对应中断状态寄存器
RX_UNDER 位 */
#define I2C_RAW_INTR_STAT_RX_UNDER_Pos (0UL) /*!< Position of R_RX_UNDER field. */
#define I2C_RAW_INTR_STAT_RX_UNDER_Msk (0x1UL << I2C_RAW_INTR_STAT_RX_UNDER_Pos) /*!< Bit mask of R_RX_UNDER field. */
#define I2C_RAW_INTR_STAT_RX_UNDER_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_RX_UNDERL_Generated (1UL) /*!< 有效. */

/* Bit 1:  I2C R_RX_OVER   */
/* Description： 对应中断状态寄存器
RX_OVER 位 */
#define I2C_RAW_INTR_STAT_RX_OVER_Pos (1UL) /*!< Position of R_RX_OVER field. */
#define I2C_RAW_INTR_STAT_RX_OVER_Msk (0x1UL << I2C_RAW_INTR_STAT_RX_OVER_Pos) /*!< Bit mask of R_RX_OVER field. */
#define I2C_RAW_INTR_STAT_RX_OVER_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_RX_OVER_Generated (1UL) /*!< 有效. */

/* Bit 2:  I2C R_RX_FULL   */
/* Description： 对应中断状态寄存器
RX_FULL 位 */
#define I2C_RAW_INTR_STAT_RX_FULL_Pos (2UL) /*!< Position of R_RX_FULL field. */
#define I2C_RAW_INTR_STAT_RX_FULL_Msk (0x1UL << I2C_RAW_INTR_STAT_RX_FULL_Pos) /*!< Bit mask of R_RX_FULL field. */
#define I2C_RAW_INTR_STAT_RX_FULL_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_RX_FULL_Generated (1UL) /*!< 有效. */

/* Bit 3:  I2C R_TX_OVER   */
/* Description： 对应中断状态寄存器
TX_OVER 位 */
#define I2C_RAW_INTR_STAT_TX_OVER_Pos (3UL) /*!< Position of R_TX_OVER field. */
#define I2C_RAW_INTR_STAT_TX_OVER_Msk (0x1UL << I2C_RAW_INTR_STAT_TX_OVER_Pos) /*!< Bit mask of R_TX_OVER field. */
#define I2C_RAW_INTR_STAT_TX_OVER_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_TX_OVER_Generated (1UL) /*!< 有效. */

/* Bit 4:  I2C R_TX_EMPTY   */
/* Description： 对应中断状态寄存器
TX_EMPTY 位 */
#define I2C_RAW_INTR_STAT_TX_EMPTY_Pos (4UL) /*!< Position of R_TX_EMPTY field. */
#define I2C_RAW_INTR_STAT_TX_EMPTY_Msk (0x1UL << I2C_RAW_INTR_STAT_TX_EMPTY_Pos) /*!< Bit mask of R_TX_EMPTY field. */
#define I2C_RAW_INTR_STAT_TX_EMPTY_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_TX_EMPTY_Generated (1UL) /*!< 有效. */

/* Bit 5:  I2C R_RD_REQ   */
/* Description： 对应中断状态寄存器
RD_REQ 位 */
#define I2C_RAW_INTR_STAT_RD_REQ_Pos (5UL) /*!< Position of R_RD_REQ field. */
#define I2C_RAW_INTR_STAT_RD_REQ_Msk (0x1UL << I2C_RAW_INTR_STAT_RD_REQ_Pos) /*!< Bit mask of R_RD_REQ field. */
#define I2C_RAW_INTR_STAT_RD_REQ_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_RD_REQ_Generated (1UL) /*!< 有效. */

/* Bit 6:  I2C R_TX_ABRT   */
/* Description： 对应中断状态寄存器
TX_ABRT 位 */
#define I2C_RAW_INTR_STAT_TX_ABRT_Pos (6UL) /*!< Position of R_TX_ABRT field. */
#define I2C_RAW_INTR_STAT_TX_ABRT_Msk (0x1UL << I2C_RAW_INTR_STAT_TX_ABRT_Pos) /*!< Bit mask of R_TX_ABRT field. */
#define I2C_RAW_INTR_STAT_TX_ABRT_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_TX_ABRT_Generated (1UL) /*!< 有效. */

/* Bit 7:  I2C R_RX_DONE   */
/* Description： 对应中断状态寄存器
RX_DONE 位 */
#define I2C_RAW_INTR_STAT_RX_DONE_Pos (7UL) /*!< Position of R_RX_DONE field. */
#define I2C_RAW_INTR_STAT_RX_DONE_Msk (0x1UL << I2C_RAW_INTR_STAT_RX_DONE_Pos) /*!< Bit mask of R_RX_DONE field. */
#define I2C_RAW_INTR_STAT_RX_DONE_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_RX_DONE_Generated (1UL) /*!< 有效. */

/* Bit 8:  I2C R_ACTIVITY   */
/* Description： 对应中断状态寄存器
ACTIVITY 位 */
#define I2C_RAW_INTR_STAT_ACTIVITY_Pos (8UL) /*!< Position of R_ACTIVITY field. */
#define I2C_RAW_INTR_STAT_ACTIVITY_Msk (0x1UL << I2C_RAW_INTR_STAT_ACTIVITY_Pos) /*!< Bit mask of R_ACTIVITY field. */
#define I2C_RAW_INTR_STAT_ACTIVITY_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_ACTIVITY_Generated (1UL) /*!< 有效. */

/* Bit 9:  I2C R_STOP_DET   */
/* Description： 对应中断状态寄存器
STOP_DET 位 */
#define I2C_RAW_INTR_STAT_STOP_DET_Pos (9UL) /*!< Position of R_STOP_DET field. */
#define I2C_RAW_INTR_STAT_STOP_DET_Msk (0x1UL << I2C_RAW_INTR_STAT_STOP_DET_Pos) /*!< Bit mask of R_STOP_DET field. */
#define I2C_RAW_INTR_STAT_STOP_DET_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_STOP_DET_Generated (1UL) /*!< 有效. */

/* Bit 10:  I2C R_START_DET   */
/* Description： 对应中断状态寄存器
START_DET 位 */
#define I2C_RAW_INTR_STAT_START_DET_Pos (10UL) /*!< Position of R_START_DET field. */
#define I2C_RAW_INTR_STAT_START_DET_Msk (0x1UL << I2C_RAW_INTR_STAT_START_DET_Pos) /*!< Bit mask of R_START_DET field. */
#define I2C_RAW_INTR_STAT_START_DET_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_START_DET_Generated (1UL) /*!< 有效. */

/* Bit 11:  I2C R_GEN_CALL   */
/* Description： 对应中断状态寄存器
GEN_CALL 位 */
#define I2C_RAW_INTR_STAT_GEN_CALL_Pos (11UL) /*!< Position of R_GEN_CALL field. */
#define I2C_RAW_INTR_STAT_GEN_CALL_Msk (0x1UL << I2C_RAW_INTR_STAT_GEN_CALL_Pos) /*!< Bit mask of R_GEN_CALL field. */
#define I2C_RAW_INTR_STAT_GEN_CALL_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_GEN_CALL_Generated (1UL) /*!< 有效. */



/* Register: I2C_RX_TL */
/* Description: I2C  接收 FIFO 阈值寄存器 */

/* Bit 0..7:  I2C RX_TL   */
/* Description： RX FIFO 阈值，当 RX FIFO 数
据大于 RX FIFO 阈值时，引发
RX_FULL 中断.
当设置的
值大于 15 时，本寄存器的值均
为 15。
0x0：1 个
0x1：2 个
……
0xF：16 个 */
#define I2C_RX_TL_Pos (0UL) /*!< Position of RX_TL field. */
#define I2C_RX_TL_Msk (0xFFUL << I2C_RX_TL_Pos) /*!< Bit mask of RX_TL field. */


/* Register: I2C_TX_TL */
/* Description: I2C  发送 FIFO 阈值寄存器 */

/* Bit 0..7:  I2C TX_TL   */
/* Description： TX FIFO 阈值，当 TX FIFO 数
据小于 TX FIFO 阈值时，引发
TX_EMPTY 中断.
当设置的
值大于 15 时，本寄存器的值均
为 15。
0x0：0 个
0x1：1 个
……
0xF：15 个 */
#define I2C_TX_TL_Pos (0UL) /*!< Position of TX_TL field. */
#define I2C_TX_TL_Msk (0xFFUL << I2C_TX_TL_Pos) /*!< Bit mask of TX_TL field. */
#define I2C_TX_TL_FIFO_0 (0UL) 
#define I2C_TX_TL_FIFO_1 (0UL) 
#define I2C_TX_TL_FIFO_2 (0UL) 
#define I2C_TX_TL_FIFO_3 (0UL) 
#define I2C_TX_TL_FIFO_4 (0UL) 
#define I2C_TX_TL_FIFO_5 (0UL) 
#define I2C_TX_TL_FIFO_6 (0UL) 
#define I2C_TX_TL_FIFO_7 (0UL) 
#define I2C_TX_TL_FIFO_8 (0UL) 
#define I2C_TX_TL_FIFO_9 (0UL) 
#define I2C_TX_TL_FIFO_10 (0UL) 
#define I2C_TX_TL_FIFO_11 (0UL) 
#define I2C_TX_TL_FIFO_12 (0UL) 

/* Register: I2C_CLR_INTR */
/* Description: I2C  清除全局中断寄存器 */

/* Bit 0:  I2C CLR_INTR   */
/* Description： 读此寄存器清除组合中断，各个独立中断以及I2C_TX_ABRT_SOURCE 寄存器*/
#define I2C_CLR_INTR_Pos (0UL) /*!< Position of CLR_INTR field. */
#define I2C_CLR_INTR_Msk (0x1UL << I2C_CLR_INTR_Pos) /*!< Bit mask of CLR_INTR field. */



/* Register: I2C_CLR_RX_UNDER */
/* Description: I2C  清除 RX_UNDER 中断寄存器 */

/* Bit 0:  I2C CLR_RX_UNDER   */
/* Description： 读 此 寄 存 器 清 除 中 断 RX_UNDER  */
#define I2C_CLR_RX_UNDER_Pos (0UL) /*!< Position of CLR_RX_UNDER field. */
#define I2C_CLR_RX_UNDER_Msk (0x1UL << I2C_CLR_RX_UNDER_Pos) /*!< Bit mask of CLR_RX_UNDER field. */


/* Register: I2C_CLR_RX_OVER */
/* Description: I2C  清除 RX_OVER 中断寄存器 */

/* Bit 0:  I2C CLR_RX_OVER   */
/* Description： 读 此 寄 存 器 清 除 中 断 RX_OVER  */
#define I2C_CLR_RX_OVER_Pos (0UL) /*!< Position of CLR_RX_OVER field. */
#define I2C_CLR_RX_OVER_Msk (0x1UL << I2C_CLR_RX_OVER_Pos) /*!< Bit mask of CLR_RX_OVER field. */


/* Register: I2C_CLR_TX_OVER */
/* Description: I2C  清除 TX_OVER 中断寄存器 */

/* Bit 0:  I2C CLR_TX_OVER   */
/* Description： 读 此 寄 存 器 清 除 中 断 TX_OVER  */
#define I2C_CLR_TX_OVER_Pos (0UL) /*!< Position of CLR_TX_OVER field. */
#define I2C_CLR_TX_OVER_Msk (0x1UL << I2C_CLR_TX_OVER_Pos) /*!< Bit mask of CLR_TX_OVER field. */


/* Register: I2C_CLR_RD_REQ */
/* Description: I2C  清除 RD_REQ 中断寄存器 */

/* Bit 0:  I2C CLR_RD_REQ   */
/* Description： 读 此 寄 存 器 清 除 中 断 RD_REQ  */
#define I2C_CLR_RD_REQ_Pos (0UL) /*!< Position of CLR_RD_REQ field. */
#define I2C_CLR_RD_REQ_Msk (0x1UL << I2C_CLR_RD_REQ_Pos) /*!< Bit mask of CLR_RD_REQ field. */


/* Register: I2C_CLR_TX_ABRT */
/* Description: I2C  清除 TX_ABRT 中断寄存器 */

/* Bit 0:  I2C CLR_TX_ABRT   */
/* Description： 读 此 寄 存 器 清 除 中 断 TX_ABRT  */
#define I2C_CLR_TX_ABRT_Pos (0UL) /*!< Position of CLR_TX_ABRT field. */
#define I2C_CLR_TX_ABRT_Msk (0x1UL << I2C_CLR_TX_ABRT_Pos) /*!< Bit mask of CLR_TX_ABRT field. */


/* Register: I2C_CLR_RX_DONE */
/* Description: I2C  清除 RX_DONE 中断寄存器 */

/* Bit 0:  I2C CLR_RX_DONE   */
/* Description： 读 此 寄 存 器 清 除 中 断 RX_DONE  */
#define I2C_CLR_RX_DONE_Pos (0UL) /*!< Position of CLR_RX_DONE field. */
#define I2C_CLR_RX_DONE_Msk (0x1UL << I2C_CLR_RX_DONE_Pos) /*!< Bit mask of CLR_RX_DONE field. */



/* Register: I2C_CLR_ACTIVITY */
/* Description: I2C  清除 ACTIVITY 中断寄存器 */

/* Bit 0:  I2C CLR_ACTIVITY   */
/* Description： 读 此 寄 存 器 清 除 中 断 ACTIVITY  */
#define I2C_CLR_ACTIVITY_Pos (0UL) /*!< Position of CLR_ACTIVITY field. */
#define I2C_CLR_ACTIVITY_Msk (0x1UL << I2C_CLR_ACTIVITY_Pos) /*!< Bit mask of CLR_ACTIVITY field. */


/* Register: I2C_CLR_STOP_DET */
/* Description: I2C  清除 STOP_DET 中断寄存器 */

/* Bit 0:  I2C CLR_STOP_DET   */
/* Description： 读 此 寄 存 器 清 除 中 断 STOP_DET  */
#define I2C_CLR_STOP_DET_Pos (0UL) /*!< Position of CLR_STOP_DET field. */
#define I2C_CLR_STOP_DET_Msk (0x1UL << I2C_CLR_STOP_DET_Pos) /*!< Bit mask of CLR_STOP_DET field. */


/* Register: I2C_CLR_START_DET */
/* Description: I2C  清除 START_DET 中断寄存器 */

/* Bit 0:  I2C CLR_START_DET   */
/* Description： 读 此 寄 存 器 清 除 中 断 START_DET  */
#define I2C_CLR_START_DET_Pos (0UL) /*!< Position of CLR_START_DET field. */
#define I2C_CLR_START_DET_Msk (0x1UL << I2C_CLR_START_DET_Pos) /*!< Bit mask of CLR_START_DET field. */


/* Register: I2C_CLR_GEN_CALL */
/* Description: I2C  清除 GEN_CALL 中断寄存器 */

/* Bit 0:  I2C CLR_GEN_CALL   */
/* Description： 读 此 寄 存 器 清 除 中 断 GEN_CALL  */
#define I2C_CLR_GEN_CALL_Pos (0UL) /*!< Position of CLR_GEN_CALL field. */
#define I2C_CLR_GEN_CALL_Msk (0x1UL << I2C_CLR_GEN_CALL_Pos) /*!< Bit mask of CLR_GEN_CALL field. */


/* Register: I2C_ENABLE */
/* Description: I2C 使能寄存器 */

/* Bit 0:  I2C ENABLE  */
/* Description： 接口模块使能控制  
接口模块处于激活状态时，软件
不能进行禁止设置。可通过 I2C
状态寄存器（I2C_STATUS）来
查询模块是否处于激活状态。
使能/禁止存在两个 i2c_clk 延迟*/
#define I2C_ENABLE_EN_Pos (0UL) /*!< Position of ENABLE field. */
#define I2C_ENABLE_EN_Msk (0x1UL << I2C_ENABLE_EN_Pos) /*!< Bit mask of ENABLE field. */
#define I2C_ENABLE_EN_Disable (0UL) /*!< 禁止. */
#define I2C_ENABLE_EN_Enable (1UL) /*!< 使能. */



/* Register: I2C_STATUS */
/* Description: I2C 状态寄存器 */

/* Bit 0:  I2C ACTIVITY  */
/* Description： 激活状态 */
#define I2C_STATUS_ACTIVITY_Pos (0UL) /*!< Position of ACTIVITY field. */
#define I2C_STATUS_ACTIVITY_Msk (0x1UL << I2C_STATUS_ACTIVITY_Pos) /*!< Bit mask of ACTIVITY field. */
#define I2C_STATUS_ACTIVITY_NotGenerated (0UL) /*!< 不激活. */
#define I2C_STATUS_ACTIVITY_Generated (1UL) /*!< 激活. */

/* Bit 1:  I2C TFNF  */
/* Description： TX FIFO 不满 */
#define I2C_STATUS_TFNF_Pos (1UL) /*!< Position of TFNF field. */
#define I2C_STATUS_TFNF_Msk (0x1UL << I2C_STATUS_TFNF_Pos) /*!< Bit mask of TFNF field. */
#define I2C_STATUS_TFNF_NotGenerated (0UL) /*!< TX FIFO 满. */
#define I2C_STATUS_TFNF_Generated (1UL) /*!< TX FIFO 不满. */

/* Bit 2:  I2C TFE  */
/* Description：TX FIFO 空 */
#define I2C_STATUS_TFE_Pos (2UL) /*!< Position of TFE field. */
#define I2C_STATUS_TFE_Msk (0x1UL << I2C_STATUS_TFE_Pos) /*!< Bit mask of TFE field. */
#define I2C_STATUS_TFE_NotGenerated (0UL) /*!< TX FIFO 不空. */
#define I2C_STATUS_TFE_Generated (1UL) /*!< TX FIFO 空. */

/* Bit 3:  I2C RFNE  */
/* Description：RX FIFO 不空 */
#define I2C_STATUS_RFNE_Pos (3UL) /*!< Position of RFNE field. */
#define I2C_STATUS_RFNE_Msk (0x1UL << I2C_STATUS_RFNE_Pos) /*!< Bit mask of RFNE field. */
#define I2C_STATUS_RFNE_NotGenerated (0UL) /*!< RX FIFO 空. */
#define I2C_STATUS_RFNE_Generated (1UL) /*!< RX FIFO 不空. */


/* Bit 4:  I2C RFF  */
/* Description：RX FIFO 满 */
#define I2C_STATUS_RFF_Pos (4UL) /*!< Position of RFF field. */
#define I2C_STATUS_RFF_Msk (0x1UL << I2C_STATUS_RFF_Pos) /*!< Bit mask of RFF field. */
#define I2C_STATUS_RFF_NotGenerated (0UL) /*!< RX FIFO 不满. */
#define I2C_STATUS_RFF_Generated (1UL) /*!< RX FIFO 满. */



/* Register: I2C_TXFLR */
/* Description: 发送 FIFO 数据计数寄存器 */

/* Bit 0..4:  I2C TXFLR  */
/* Description： TX FIFO 有效数据项个数 */
#define I2C_TXFLR_Pos (0UL) /*!< Position of TXFLR field. */
#define I2C_TXFLR_Msk (0x1FUL << I2C_TXFLR_Pos) /*!< Bit mask of TXFLR field. */


/* Register: I2C_RXFLR */
/* Description: 接收 FIFO 数据计数寄存器 */

/* Bit 0..4:  I2C RXFLR  */
/* Description： RX FIFO 有效数据项个数 I2C 被禁止或者传输异常中止时，该寄存器清 0。*/
#define I2C_RXFLR_Pos (0UL) /*!< Position of RXFLR field. */
#define I2C_RXFLR_Msk (0x1FUL << I2C_RXFLR_Pos) /*!< Bit mask of RXFLR field. */


/* Register: I2C_SDA_HOLD */
/* Description: SDA 保持时间 */

/* Bit 0..15:  I2C I2C_SDA_HOLD  */
/* Description： 设置以 i2c_clk 为单位的 SDA 保持时间 */
#define I2C_SDA_HOLD_Pos (0UL) /*!< Position of I2C_SDA_HOLD field. */
#define I2C_SDA_HOLD_Msk (0xFFFFUL << I2C_RXFLR_Pos) /*!< Bit mask of I2C_SDA_HOLD field. */


/* Register: I2C_TX_ABRT_SOURCE */
/* Description: I2C 发送异常中止源寄存器 
指示 TX_ABRT 源，读该寄存器或者处理器发出清除所有中断时清 0。*/

/* Bit 0:  I2C ABRT_7B_ADDR_NOACK  */
/* Description： Master 处于 7 比特地址模式，地址字节无确认 */
#define I2C_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_Pos (0UL) /*!< Position of ABRT_7B_ADDR_NOACK field. */
#define I2C_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_Pos) 
/*!< Bit mask of ABRT_7B_ADDR_NOACK field. */

/* Bit 1:  I2C ABRT_10ADDR1_NOACK  */
/* Description： Master 处于 10 比特地址模
式，第一地址字节无确认 */
#define I2C_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_Pos (1UL) /*!< Position of ABRT_10ADDR1_NOACK field. */
#define I2C_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_Pos)
 /*!< Bit mask of ABRT_10ADDR1_NOACK field. */

/* Bit 2:  I2C ABRT_10ADDR2_NOACK  */
/* Description： Master 处于 10 比特地址模
式，第二地址字节无确认 */
#define I2C_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_Pos (2UL) /*!< Position of ABRT_10ADDR2_NOACK field. */
#define I2C_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_Pos)
 /*!< Bit mask of ABRT_10ADDR2_NOACK field. */


/* Bit 3:  I2C ABRT_TXDATA_NOACK  */
/* Description： Master 收到地址确认，但是
发送数据之后无确认 */
#define I2C_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_Pos (3UL) /*!< Position of ABRT_TXDATA_NOACK field. */
#define I2C_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_Pos)
 /*!< Bit mask of ABRT_TXDATA_NOACK field. */

/* Bit 4:  I2C ABRT_GCALL_NOACK  */
/* Description： Master 发出通用寻址而无
Slave 响应 */
#define I2C_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_Pos (4UL) /*!< Position of ABRT_GCALL_NOACK field. */
#define I2C_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_Pos)
 /*!< Bit mask of ABRT_GCALL_NOACK field. */

/* Bit 5:  I2C ABRT_GCALL_READ  */
/* Description： Master 发出通用寻址，而处
理器紧接着发出读请求 */
#define I2C_TX_ABRT_SOURCE_ABRT_GCALL_READ_Pos (5UL) /*!< Position of ABRT_GCALL_READ field. */
#define I2C_TX_ABRT_SOURCE_ABRT_GCALL_READ_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_GCALL_READ_Pos)
 /*!< Bit mask of ABRT_GCALL_READ field. */

/* Bit 7:  I2C ABRT_SBYTE_ACKDET  */
/* Description： Master 发送 Start Byte 而
Start Byte 得到确认 */
#define I2C_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_Pos (7UL) /*!< Position of ABRT_SBYTE_ACKDET field. */
#define I2C_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_Pos)
 /*!< Bit mask of ABRT_SBYTE_ACKDET field. */

/* Bit 9:  I2C ABRT_SBYTE_NORSTRT  */
/* Description： Restart 被禁止，处理器企图
发出 Start Byte */
#define I2C_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_Pos (9UL) /*!< Position of ABRT_SBYTE_NORSTRT field. */
#define I2C_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_Pos)
 /*!< Bit mask of ABRT_SBYTE_NORSTRT field. */


/* Bit 10:  I2C ABRT_10B_RD_NORSTRT  */
/* Description： Master 发送 Start Byte 而
Start Byte 得到确认 */
#define I2C_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_Pos (10UL) /*!< Position of ABRT_10B_RD_NORSTRT field. */
#define I2C_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_Pos)
 /*!< Bit mask of ABRT_10B_RD_NORSTRT field. */

/* Bit 11:  I2C ABRT_MASTER_DIS  */
/* Description：  处 理 器 企 图 使 用 禁 止 的
Master */
#define I2C_TX_ABRT_SOURCE_ABRT_MASTER_DIS_Pos (10UL) /*!< Position of ABRT_MASTER_DIS field. */
#define I2C_TX_ABRT_SOURCE_ABRT_MASTER_DIS_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_MASTER_DIS_Pos)
 /*!< Bit mask of ABRT_MASTER_DIS field. */

/* Bit 12:  I2C ABRT_LOST  */
/* Description： ：Master 丢失总线控制权，或
者本寄存器 bit14 置 1，表明发
送数据的 Slave 丢失总线控制
权。 */
#define I2C_TX_ABRT_SOURCE_ABRT_LOST_Pos (10UL) /*!< Position of ABRT_LOST field. */
#define I2C_TX_ABRT_SOURCE_ABRT_LOST_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_LOST_Pos)
 /*!< Bit mask of ABRT_LOST field. */


////////////////////////////////////////////////////////////////////////////////















/* Register: UART_INTENSET */
/* Description: Enable interrupt */

/* Bit 17 : Write '1' to enable interrupt for event RXTO */
#define UART_INTENSET_RXTO_Pos (17UL) /*!< Position of RXTO field. */
#define UART_INTENSET_RXTO_Msk (0x1UL << UART_INTENSET_RXTO_Pos) /*!< Bit mask of RXTO field. */
#define UART_INTENSET_RXTO_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENSET_RXTO_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENSET_RXTO_Set (1UL) /*!< Enable */

/* Bit 9 : Write '1' to enable interrupt for event ERROR */
#define UART_INTENSET_ERROR_Pos (9UL) /*!< Position of ERROR field. */
#define UART_INTENSET_ERROR_Msk (0x1UL << UART_INTENSET_ERROR_Pos) /*!< Bit mask of ERROR field. */
#define UART_INTENSET_ERROR_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENSET_ERROR_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENSET_ERROR_Set (1UL) /*!< Enable */

/* Bit 7 : Write '1' to enable interrupt for event TXDRDY */
#define UART_INTENSET_TXDRDY_Pos (7UL) /*!< Position of TXDRDY field. */
#define UART_INTENSET_TXDRDY_Msk (0x1UL << UART_INTENSET_TXDRDY_Pos) /*!< Bit mask of TXDRDY field. */
#define UART_INTENSET_TXDRDY_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENSET_TXDRDY_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENSET_TXDRDY_Set (1UL) /*!< Enable */

/* Bit 2 : Write '1' to enable interrupt for event RXDRDY */
#define UART_INTENSET_RXDRDY_Pos (2UL) /*!< Position of RXDRDY field. */
#define UART_INTENSET_RXDRDY_Msk (0x1UL << UART_INTENSET_RXDRDY_Pos) /*!< Bit mask of RXDRDY field. */
#define UART_INTENSET_RXDRDY_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENSET_RXDRDY_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENSET_RXDRDY_Set (1UL) /*!< Enable */

/* Bit 1 : Write '1' to enable interrupt for event NCTS */
#define UART_INTENSET_NCTS_Pos (1UL) /*!< Position of NCTS field. */
#define UART_INTENSET_NCTS_Msk (0x1UL << UART_INTENSET_NCTS_Pos) /*!< Bit mask of NCTS field. */
#define UART_INTENSET_NCTS_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENSET_NCTS_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENSET_NCTS_Set (1UL) /*!< Enable */

/* Bit 0 : Write '1' to enable interrupt for event CTS */
#define UART_INTENSET_CTS_Pos (0UL) /*!< Position of CTS field. */
#define UART_INTENSET_CTS_Msk (0x1UL << UART_INTENSET_CTS_Pos) /*!< Bit mask of CTS field. */
#define UART_INTENSET_CTS_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENSET_CTS_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENSET_CTS_Set (1UL) /*!< Enable */

/* Register: UART_INTENCLR */
/* Description: Disable interrupt */

/* Bit 17 : Write '1' to disable interrupt for event RXTO */
#define UART_INTENCLR_RXTO_Pos (17UL) /*!< Position of RXTO field. */
#define UART_INTENCLR_RXTO_Msk (0x1UL << UART_INTENCLR_RXTO_Pos) /*!< Bit mask of RXTO field. */
#define UART_INTENCLR_RXTO_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENCLR_RXTO_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENCLR_RXTO_Clear (1UL) /*!< Disable */

/* Bit 9 : Write '1' to disable interrupt for event ERROR */
#define UART_INTENCLR_ERROR_Pos (9UL) /*!< Position of ERROR field. */
#define UART_INTENCLR_ERROR_Msk (0x1UL << UART_INTENCLR_ERROR_Pos) /*!< Bit mask of ERROR field. */
#define UART_INTENCLR_ERROR_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENCLR_ERROR_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENCLR_ERROR_Clear (1UL) /*!< Disable */


/* Register: UART_ERRORSRC */
/* Description: Error source */



/* Register: UART_ENABLE */
/* Description: Enable UART */

/* Bits 3..0 : Enable or disable UART */
#define UART_ENABLE_ENABLE_Pos (0UL) /*!< Position of ENABLE field. */
#define UART_ENABLE_ENABLE_Msk (0xFUL << UART_ENABLE_ENABLE_Pos) /*!< Bit mask of ENABLE field. */
#define UART_ENABLE_ENABLE_Disabled (0UL) /*!< Disable UART */
#define UART_ENABLE_ENABLE_Enabled (4UL) /*!< Enable UART */

/* Register: UART_PSEL_RTS */
/* Description: Pin select for RTS */

/* Bit 31 : Connection */
#define UART_PSEL_RTS_CONNECT_Pos (31UL) /*!< Position of CONNECT field. */
#define UART_PSEL_RTS_CONNECT_Msk (0x1UL << UART_PSEL_RTS_CONNECT_Pos) /*!< Bit mask of CONNECT field. */
#define UART_PSEL_RTS_CONNECT_Connected (0UL) /*!< Connect */
#define UART_PSEL_RTS_CONNECT_Disconnected (1UL) /*!< Disconnect */

/* Bit 5 : Port number */
#define UART_PSEL_RTS_PORT_Pos (5UL) /*!< Position of PORT field. */
#define UART_PSEL_RTS_PORT_Msk (0x1UL << UART_PSEL_RTS_PORT_Pos) /*!< Bit mask of PORT field. */

/* Bits 4..0 : Pin number */
#define UART_PSEL_RTS_PIN_Pos (0UL) /*!< Position of PIN field. */
#define UART_PSEL_RTS_PIN_Msk (0x1FUL << UART_PSEL_RTS_PIN_Pos) /*!< Bit mask of PIN field. */

/* Register: UART_PSEL_TXD */
/* Description: Pin select for TXD */

/* Bit 31 : Connection */
#define UART_PSEL_TXD_CONNECT_Pos (31UL) /*!< Position of CONNECT field. */
#define UART_PSEL_TXD_CONNECT_Msk (0x1UL << UART_PSEL_TXD_CONNECT_Pos) /*!< Bit mask of CONNECT field. */
#define UART_PSEL_TXD_CONNECT_Connected (0UL) /*!< Connect */
#define UART_PSEL_TXD_CONNECT_Disconnected (1UL) /*!< Disconnect */

/* Bit 5 : Port number */
#define UART_PSEL_TXD_PORT_Pos (5UL) /*!< Position of PORT field. */
#define UART_PSEL_TXD_PORT_Msk (0x1UL << UART_PSEL_TXD_PORT_Pos) /*!< Bit mask of PORT field. */

/* Bits 4..0 : Pin number */
#define UART_PSEL_TXD_PIN_Pos (0UL) /*!< Position of PIN field. */
#define UART_PSEL_TXD_PIN_Msk (0x1FUL << UART_PSEL_TXD_PIN_Pos) /*!< Bit mask of PIN field. */

/* Register: UART_PSEL_CTS */
/* Description: Pin select for CTS */



/* Register: UART_BAUDRATE */
/* Description: Baud rate. Accuracy depends on the HFCLK source selected. */

/* Bits 31..0 : Baud rate */
#define UART_BAUDRATE_BAUDRATE_Pos (0UL) /*!< Position of BAUDRATE field. */
#define UART_BAUDRATE_BAUDRATE_Msk (0xFFFFFFFFUL << UART_BAUDRATE_BAUDRATE_Pos) /*!< Bit mask of BAUDRATE field. */
#define UART_BAUDRATE_BAUDRATE_Baud2400 (0x0120271CUL) /*!< 2400 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud4800 (0x0240271CUL) /*!< 4800 baud */
#define UART_BAUDRATE_BAUDRATE_Baud9600 (0x0480271CUL) /*!< 9600 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud12800 (0x04802719UL) /*!< 12800 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud14400 (0x04802718UL) /*!< 14400 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud19200 (0x04802716UL) /*!< 19200 baud  */

#define UART_BAUDRATE_BAUDRATE_Baud23040 (0x04802715UL) /*!< 23040 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud38400 (0x04802713UL) /*!< 38400 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud57600 (0x04802712UL) /*!< 57600 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud115200 (0x04802711UL) /*!< 115200 baud */

#define UART_BAUDRATE_BAUDRATE_Baud128000 (0x030007D3UL) /*!< 128000 baud */
#define UART_BAUDRATE_BAUDRATE_Baud230400 (0x0120271CUL) /*!< 230400 baud */
#define UART_BAUDRATE_BAUDRATE_Baud256000 (0x060007D3UL) /*!< 256000 baud */
#define UART_BAUDRATE_BAUDRATE_Baud460800 (0x12002711UL) /*!< 256000 baud */
#define UART_BAUDRATE_BAUDRATE_Baud921600 (0x24002711UL) /*!< 256000 baud */

#define UART_BAUDRATE_BAUDRATE_Baud1M (0x00100011UL) /*!< 1Mega baud */

/* Register: UART_CONFIG */
/* Description: Configuration of parity and hardware flow control */

/* Bit 4 : Stop bits */
#define UART_CONFIG_STOP_Pos (4UL) /*!< Position of STOP field. */
#define UART_CONFIG_STOP_Msk (0x1UL << UART_CONFIG_STOP_Pos) /*!< Bit mask of STOP field. */
#define UART_CONFIG_STOP_One (0UL) /*!< One stop bit */
#define UART_CONFIG_STOP_Two (1UL) /*!< Two stop bits */

/* Bits 3..1 : Parity */
#define UART_CONFIG_PARITY_Pos (1UL) /*!< Position of PARITY field. */
#define UART_CONFIG_PARITY_Msk (0x7UL << UART_CONFIG_PARITY_Pos) /*!< Bit mask of PARITY field. */
#define UART_CONFIG_PARITY_Excluded (0x0UL) /*!< Exclude parity bit */
#define UART_CONFIG_PARITY_Included (0x7UL) /*!< Include parity bit */

/* Bit 0 : Hardware flow control */
#define UART_CONFIG_HWFC_Pos (0UL) /*!< Position of HWFC field. */
#define UART_CONFIG_HWFC_Msk (0x1UL << UART_CONFIG_HWFC_Pos) /*!< Bit mask of HWFC field. */
#define UART_CONFIG_HWFC_Disabled (0UL) /*!< Disabled */
#define UART_CONFIG_HWFC_Enabled (1UL) /*!< Enabled */






/* Bits 31..0 : Baud rate */
#define UARTE_BAUDRATE_BAUDRATE_Pos (0UL) /*!< Position of BAUDRATE field. */
//#define UARTE_BAUDRATE_BAUDRATE_Msk (0xFFFFFFFFUL << UARTE_BAUDRATE_BAUDRATE_Pos) /*!< Bit mask of BAUDRATE field. */
//#define UARTE_BAUDRATE_BAUDRATE_Baud1200 (0x0004F000UL) /*!< 1200 baud (actual rate: 1205) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud2400 (0x0009D000UL) /*!< 2400 baud (actual rate: 2396) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud4800 (0x0013B000UL) /*!< 4800 baud (actual rate: 4808) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud9600 (0x00275000UL) /*!< 9600 baud (actual rate: 9598) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud14400 (0x003AF000UL) /*!< 14400 baud (actual rate: 14401) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud19200 (0x004EA000UL) /*!< 19200 baud (actual rate: 19208) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud28800 (0x0075C000UL) /*!< 28800 baud (actual rate: 28777) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud31250 (0x00800000UL) /*!< 31250 baud */
//#define UARTE_BAUDRATE_BAUDRATE_Baud38400 (0x009D0000UL) /*!< 38400 baud (actual rate: 38369) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud56000 (0x00E50000UL) /*!< 56000 baud (actual rate: 55944) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud57600 (0x00EB0000UL) /*!< 57600 baud (actual rate: 57554) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud76800 (0x013A9000UL) /*!< 76800 baud (actual rate: 76923) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud115200 (0x01D60000UL) /*!< 115200 baud (actual rate: 115108) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud230400 (0x03B00000UL) /*!< 230400 baud (actual rate: 231884) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud250000 (0x04000000UL) /*!< 250000 baud */
//#define UARTE_BAUDRATE_BAUDRATE_Baud460800 (0x07400000UL) /*!< 460800 baud (actual rate: 457143) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud921600 (0x0F000000UL) /*!< 921600 baud (actual rate: 941176) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud1M (0x10000000UL) /*!< 1Mega baud */

/* Register: UARTE_RXD_PTR */
/* Description: Data pointer */












/*lint --flb "Leave library region" */
#endif
