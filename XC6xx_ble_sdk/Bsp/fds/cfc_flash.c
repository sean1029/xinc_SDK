/**
 * Copyright (c) 2012 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 *@file
 *@brief NMVC driver implementation
 */

//#include <nrfx.h>
#include <string.h>
#include <stdio.h>
#include "cfc_flash.h"
#include  "bsp_spi_flash.h"

static inline void wait_for_flash_ready(void)
{
  //  while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {;}
}

void nrf_nvmc_page_erase(uint32_t address)
{

		spi_flash_page_erase(address);
    // Erase the page
}


void nrf_nvmc_write_byte(uint32_t address, uint8_t value)
{

    spi_flash_write(address,(uint8_t *)&value,1);

 //   NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);
 //   __ISB();
 //   __DSB();
}

void nrf_nvmc_write_word(uint32_t address, uint32_t value)
{

    spi_flash_write(address,(uint8_t *)&value,4);
    wait_for_flash_ready();

}

void nrf_nvmc_write_bytes(uint32_t address, const uint8_t * src, uint32_t num_bytes)
{
	printf("%s\r\n",__func__);
	//uint8_t *txBuff = src;
  spi_flash_write(address,src,num_bytes);
}

void nrf_nvmc_write_words(uint32_t address, const uint32_t * src, uint32_t num_words)
{
		printf("%s\r\n",__func__);
    uint32_t i;

    // Enable write.
//    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
//    __ISB();
//    __DSB();
		spi_flash_write(address,(uint8_t *)src,num_words * 4);

    
 //   NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
 //   __ISB();
 //   __DSB();
}

