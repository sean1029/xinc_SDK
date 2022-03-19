/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
/**
 *@file
 *@brief NMVC driver implementation
 */

//#include <nrfx.h>
#include <string.h>
#include <stdio.h>
#include "xinc_flash.h"
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
//    uint32_t i;

    // Enable write.
//    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
//    __ISB();
//    __DSB();
		spi_flash_write(address,(uint8_t *)src,num_words * 4);

    
 //   NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
 //   __ISB();
 //   __DSB();
}

