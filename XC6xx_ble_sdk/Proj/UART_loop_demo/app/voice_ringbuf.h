
/**
 *************************************************************************************
 * @file    voice_ringbuf.h
 * @brief   An implementation of SBC decoder for bluetooth.
 *
 * @author  feng.Cheng
 * @version V1.0.0
 *
 * &copy; 2022 XinChip Corporation Ltd. All rights reserved.
 *
 *************************************************************************************
 */
 
#ifndef __VOICE_RINGBUF_H
#define __VOICE_RINGBUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define  TOTAL_BLOCK_NUM  			 (160)

void voice_ring_buffer_init(void);
int voice_ring_buffer_empty(void);
uint32_t  read_encode_data(uint8_t *buf);
uint8_t store_encode_data(uint8_t *buf);
#endif // __VOICE_RINGBUF_H
