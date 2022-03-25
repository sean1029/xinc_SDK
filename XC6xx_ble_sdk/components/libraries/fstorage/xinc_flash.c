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
 *@brief FLASH driver implementation
 */

#include <xincx.h>
#include <string.h>
#include <stdio.h>
#include "xinc_flash.h"
#include "xinc_spi_flash.h"

static inline void wait_for_flash_ready(void)
{
  //  while (XINC_NVMC->READY == NVMC_READY_READY_Busy) {;}
    while(spim_read_flash_status() & 0x01){;}
}

void xinc_flash_page_erase(uint32_t address)
{
  // Erase the page
    spim_flash_page_erase(address);
    wait_for_flash_ready();
}


void xinc_flash_write_byte(uint32_t address, uint8_t value)
{
    spim_flash_write(address,(uint8_t *)&value,1);
    wait_for_flash_ready();
}

void xinc_flash_write_word(uint32_t address, uint32_t value)
{

    spim_flash_write(address,(uint8_t *)&value,4);
    wait_for_flash_ready();

}

void xinc_flash_write_bytes(uint32_t address, uint8_t * src, uint32_t num_bytes)
{
   // printf("%s\r\n",__func__);
    spim_flash_write(address,src,num_bytes);
    wait_for_flash_ready();
}

void xinc_flash_write_words(uint32_t address, const uint32_t * src, uint32_t num_words)
{
  //  printf("%s\r\n",__func__);

    spim_flash_write(address,(uint8_t *)src,num_words * 4);
    wait_for_flash_ready();

}


void xinc_flash_read_bytes(uint32_t address, uint8_t * src, uint32_t num_bytes)
{
    spim_flash_read(address,src,num_bytes);
}


void xinc_flash_init(void)
{
    spim_flash_init();
}
