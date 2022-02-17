
/**
 *************************************************************************************
 * @file    voice_ringbuf.c
 * @brief   An implementation of SBC decoder for bluetooth.
 *
 * @author  feng.Cheng
 * @version V1.0.0
 *
 * &copy; 2022 XinChip Corporation Ltd. All rights reserved.
 *
 *************************************************************************************
 */
#include <stdint.h>
#include <string.h>
#include "btstack_ring_buffer.h"

#include "voice_ringbuf.h"
//Defines an array to store audio data that has been encoded
uint8_t encode_voice_data[TOTAL_BLOCK_NUM][20];
//reaord index
uint8_t pbuff_write = 0;
//read index
uint8_t pbuff_read = 0;


static uint8_t encode_voice_data[TOTAL_BLOCK_NUM][20];
static btstack_ring_buffer_t encode_buffer;

void voice_ring_buffer_init(void)
{
	btstack_ring_buffer_init(&encode_buffer, (uint8_t*)&encode_voice_data, sizeof(encode_voice_data));
}


int voice_ring_buffer_empty(void)
{
   return btstack_ring_buffer_empty(&encode_buffer);
}	


 /*******************************************************************************
 * Function: read_encode_data
 * Description: read encoded data from loop buffer
 * Input: uint8_t*
 * Output: void
 * Return: uint8_t
 * Others: void
*******************************************************************************/
uint32_t  read_encode_data(uint8_t *buf)
{
	//Read 20 encode data from loop buffer to designated buffer 
	uint32_t num_bytes_read = 0;
	btstack_ring_buffer_read(&encode_buffer, (uint8_t*)buf, 20, &num_bytes_read);
	return num_bytes_read;
}



/*******************************************************************************
 * Function: store_encode_data
 * Description: store encoded data into loop buffer
 * Input: uint8_t*
 * Output: void
 * Return: uint8_t
 * Others: void
*******************************************************************************/
uint8_t store_encode_data(uint8_t *buf)
{
	uint8_t status ;
	
	status  = btstack_ring_buffer_write(&encode_buffer, (uint8_t*)buf, 20);;
		
	return status;
}
