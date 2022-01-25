#include    "platform.h"
#include    "xinchip_ota.h"




#ifdef OTA_TEST

#define ZONE_A_SECORT  0     //A����һ��������(A��flash��ַ�ռ�0--128*1024-1)
#define ZONE_B_SECORT  32    //B����һ��������(B��flash��ַ�ռ�128*1024--256*1024-1)
//#define ZONE_B_SECORT  0    //B����һ��������

typedef struct ota_updata{
	uint16_t index;       //���ջ���������
	uint8_t  buf[4096];   //���ջ�����
	uint8_t  secort_index;    //��������
	uint32_t checksum0;       //����APP��������bin�ļ���У����
	uint32_t checksum1;       //��д��ɺ������д���������У���� ������ checksum0�ȶ�
	uint32_t time_t;          //��ʱ��
	uint8_t  rev_compl_flag;  //������ɱ�־
}ota_updata_t;

ota_updata_t  ota_pack={
    .index=0,              //���ջ���������
  	.secort_index=0,           //��������
	.checksum0=0,              //����APP��������bin�ļ���У����
	.checksum1=0,              //��д��ɺ������д���������У���� ������ checksum0�ȶ�
	.time_t=0,                 //��ʱ��
	.rev_compl_flag=0          //������ɱ�־
};
/********************************************************************************************************
������:void ota_rev_packet(const uint8_t *buffer,uint16_t buffer_size)
��  ��:��ȡBootLoader�������� �Ա�OTA���� ʱʹ��
��  ��:���յ������������ȼ������׵�ַ
����ֵ:��
********************************************************************************************************/
void  ota_rev_packet(const uint8_t *buffer,uint16_t buffer_size)
{
   
		ota_pack.time_t=1;
        if(buffer_size==4){//APP�����ĺ�У����--���Ҵ����ݰ������һ��
		   ota_pack.checksum0=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]);
		   ota_pack.rev_compl_flag =1;
		}else{//APP ��������OTA������ ---APP ÿ�η�����64���ֽ�
			for(uint32_t i=0; i<buffer_size; i++){
				ota_pack.buf[ota_pack.index++]=buffer[i];
				
			}
			if(ota_pack.index==4096){//������4096���ֽ�(flashһ��������4096���ֽ�)				
				ota_pack.index=0;
				spi_flash_Release_powerdown();
				spi_flash_write((ZONE_B_SECORT+ota_pack.secort_index)*4096,ota_pack.buf,4096);//д��flashһ������
				spi_flash_Enter_powerdown();
				ota_pack.secort_index++;	
			}
		}
	


}

/*****************************************************************************************************
������:void ota_fun(void)
��  ��:����ota������
��  ��:��
����ֵ:��
******************************************************************************************************/
void ota_fun(void)
{
	if(ota_pack.time_t)
	{   
		if(ota_pack.time_t==2) 
		{	
			//Uart_DMA_Send(0,"*",1);	
			static unsigned char cout=0;
			cout++;
			if(cout>64)//���յ�64*64=4096���ֽ�
			{
				cout=0;
				Uart_DMA_Send(0,"#",1);
			}				
		}
		ota_pack.time_t++;
		if(ota_pack.time_t>=300000)//��ѭ��ѭ��һ�δ�Լ��31us 31us*300000=9.3s  ���time_t ��ʱ����9.3s˵�����ݴ����ж�
		{
			ota_pack.time_t=0;
			ota_pack.index=0;
			ota_pack.secort_index=0;
			ota_pack.rev_compl_flag=0;
			ota_pack.checksum0=0;
			ota_pack.checksum1=0;
			Uart_DMA_Send(0,"Trans Interruption ...\n",sizeof("Trans Interruption ..\n"));
			
		}
		if(ota_pack.rev_compl_flag)//�������	
		{   
			Uart_DMA_Send(0,"\nplease wait ...\n",sizeof("\nplease wait ...\n"));
			ota_pack.time_t=0;
			spi_flash_Release_powerdown();
			if(ota_pack.index!=4096)//�����һ��д��flash
			    spi_flash_write((ZONE_B_SECORT+ota_pack.secort_index)*4096,ota_pack.buf,ota_pack.index);
			//��д�뵽flash�����ݶ��������к�У�� ������APP�������ĺ�У����ȶ�
			
			for(int kk=0;kk <= ota_pack.secort_index;kk++)
			{
			   if(kk==(ota_pack.secort_index))
			   { 
				  spi_flash_Read((kk+ZONE_B_SECORT)*4096,ota_pack.buf,ota_pack.index);
				  for(int j=0;j<ota_pack.index;j++)
				   {  
					  ota_pack.checksum1+=ota_pack.buf[j];
				   }
			   }
			   else
			   {
				  spi_flash_Read((kk+ZONE_B_SECORT)*4096,ota_pack.buf,4096);
				  for(int j=0;j<4096;j++)
				   {  
					  ota_pack.checksum1+=ota_pack.buf[j];
				   }
			   }
			}	
			if(ota_pack.checksum1==ota_pack.checksum0)//У��ɹ�
			{   
				#if 1//�ڴ˴�����¼����¼��A����flash��136�ֽڴ���macת�浽Bȥflash��32*4096+136��ַ��
				{
					uint8_t temp_mac_buf0[6]={0};
					uint8_t temp_mac_buf1[6]={0};
				    spi_flash_Read(136, temp_mac_buf0,6);
					temp_mac_buf1[0]=temp_mac_buf0[1];temp_mac_buf1[2]=temp_mac_buf0[3];temp_mac_buf1[4]=temp_mac_buf0[5];
					temp_mac_buf1[1]=temp_mac_buf0[0];temp_mac_buf1[3]=temp_mac_buf0[2];temp_mac_buf1[5]=temp_mac_buf0[4];
					spi_flash_write((32*4096)+136, temp_mac_buf1,6);
				}
				#endif
				//��B�������ݸ��ǵ�A��-��һ�����ٽ���У�飬��Ϊ��һ��������� ��оƬ�ͽ����שͷ��У�������˵�Ѿ�û������
				for(int kk=0;kk <= ota_pack.secort_index;kk++)
				{
				   if(kk==(ota_pack.secort_index))
				   { 
					  spi_flash_Read ((kk+ZONE_B_SECORT)*4096,ota_pack.buf,ota_pack.index);
					  for(int n=0;n<2048;n++) {
						  uint8_t temp=ota_pack.buf[2*n];
					      ota_pack.buf[2*n]=ota_pack.buf[2*n+1];
						  ota_pack.buf[2*n+1]=temp;
					  }
					  spi_flash_write((kk+ZONE_A_SECORT)*4096,ota_pack.buf,ota_pack.index); 
				   }
				   else
				   {
					  spi_flash_Read ((kk+ZONE_B_SECORT)*4096,ota_pack.buf,4096);
					  for(int n=0;n<2048;n++) {
						  uint8_t temp=ota_pack.buf[2*n];
					      ota_pack.buf[2*n]=ota_pack.buf[2*n+1];
						  ota_pack.buf[2*n+1]=temp;
					  }
					  spi_flash_write((kk+ZONE_A_SECORT)*4096,ota_pack.buf,4096);
				   }
				}	
				
				Uart_DMA_Send(0,"Update complete!\n",sizeof("Update complete!\n"));
				spi_flash_Enter_powerdown();
				xc_wdog_init(0x0);//�������Ź��ȴ�ϵͳҧ����λ
				while(1);

			}
			else//У��ʧ��
			{
				Uart_DMA_Send(0,"\nUpdate Fail!\n",sizeof("Update Fail!\n"));
				ota_pack.time_t=0;
				ota_pack.index=0;
				ota_pack.secort_index=0;
				ota_pack.rev_compl_flag=0;
				ota_pack.checksum0=0;
				ota_pack.checksum1=0;
				spi_flash_Enter_powerdown();		
				//xc_wdog_init(0x1);//�������Ź��ȴ�ϵͳҧ����λ
				//while(1);
		   }
		}
	}
}

#endif


