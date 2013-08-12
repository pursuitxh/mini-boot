#include <stdio.h>
#include <dm9000.h>
#include <net.h>
#include <mini-boot.h>


void dm9000_reg_write(unsigned char reg, unsigned char data)			//write the register
{
	msdelay(1);
	DM9000_ADDR = reg;
	msdelay(1);
	DM9000_DATA = data;
}

unsigned char dm9000_reg_read(unsigned char reg)						//read the register,8 bits mode
{
	msdelay(1);
	DM9000_ADDR = reg;
	msdelay(1);
	return DM9000_DATA;
}

unsigned short dm9000_reg_read16(unsigned char reg)						//read the register,16 bits mode
{
	msdelay(1);
	DM9000_ADDR = reg;
	msdelay(1);
	return DM9000_DATA;
}

void dm9000_test(void)
{
	unsigned long id_val, i;
	id_val = 0;
	/*try multiple times ,DM9000 sometimes gets the read wrong*/
	for (i = 0; i < 8; i++)								
	{
		id_val = dm9000_reg_read(DM9000_VIDL);
		id_val |=(unsigned long)dm9000_reg_read(DM9000_VIDH) << 8;
		id_val |=(unsigned long)dm9000_reg_read(DM9000_PIDL) << 16;
		id_val |=(unsigned long)dm9000_reg_read(DM9000_PIDH) << 24;
	
	if (id_val == DM9000_ID)
		{
			//printf("DM9000 ID : 0x%08x\n\r", id_val);
			break;
		}	
	}
	if (id_val != DM9000_ID)	
		printf("\nwrong id : 0x%08x\n", id_val);
}


void dm9000_init(void)
{

	dm9000_test();											//test if the chip is dm9000
	
	unsigned int i;	
	
	dm9000_reg_write(DM9000_GPCR, 0x01);					//start dm9000
	dm9000_reg_write(DM9000_GPR, 0x00);				
	msdelay(50);
	
	
	dm9000_reg_write(DM9000_NCR, 0x03);						//make two soft reset,this is the first
	msdelay(50);	
	dm9000_reg_write(DM9000_NCR, 0x00);			
			
	dm9000_reg_write(DM9000_NCR, 0x03);						//this is the second soft reset 	
	msdelay(50);
	dm9000_reg_write(DM9000_NCR, 0x00);				
	
	dm9000_reg_write(DM9000_NSR, 0x2c);						//clear the status	
	dm9000_reg_write(DM9000_ISR, 0x3f);						//16 bit bus mode
		
	dm9000_reg_write(DM9000_RCR, 0x39);						//receive control		
	dm9000_reg_write(DM9000_TCR, 0x00);						//send control
	
	dm9000_reg_write(DM9000_BPTR, 0x3f);			
	dm9000_reg_write(DM9000_FCTR, 0x3a);			
	dm9000_reg_write(DM9000_RTFCR, 0xff);			
	dm9000_reg_write(DM9000_SMCR, 0x00);				
		
	for (i = 0; i < 6; i++)									//mac address
		dm9000_reg_write(DM9000_PAR + i, mac_addr[i]);		
				
	dm9000_reg_write(DM9000_NSR,0x2c);						//clear the status and interrupt bits
	dm9000_reg_write(DM9000_ISR,0x3f);						//16 bit bus mode
	
	dm9000_reg_write(DM9000_IMR,0x80);						//disable the interrupt 
}

void dm9000_send_packet(unsigned char * datas, unsigned int length)
{
	unsigned int i, len;
	
	len = length;
	
	dm9000_reg_write(DM9000_IMR,0x80);						//first disable the interrupt
	
	dm9000_reg_write(DM9000_TXPLH,(len >> 8)&0x0ff);		//write the length of your packet which will be send
	dm9000_reg_write(DM9000_TXPLL,len&0x0ff);
	
	/*the inter SDRAM,which we will write our data into, and the pointer of the SDRAM will increase automatically*/
	DM9000_ADDR = DM9000_MWCMD;								
	
	for (i = 0; i < len; i += 2)							//mini2440 has connect dm9000 in 16 bits mode
	{
		msdelay(1);
		DM9000_DATA = datas[i] | (datas[i+1] << 8);
	}
	
	dm9000_reg_write(DM9000_TCR, 0x01);						//set send mode bit ,and the data will send	
	
	while(1)												//when send completed, TCR bits0 will set to 0
	{
		if(((dm9000_reg_read(DM9000_TCR)) & 0x01) == 0x00)
			break;
	}
	
	while((dm9000_reg_read(DM9000_NSR)&0x0c) == 0)			//check again	
	msdelay(1);

	dm9000_reg_write(DM9000_NSR,0x2c);		
}

unsigned int dm9000_receive_packet(void)
{
	unsigned char *datas;
	unsigned int i, tem;
	unsigned int status, len;
	unsigned char ready;
	ready = 0;											//we expect to receive 0x01
	status = 0;											//receive status,the content is as same as 0x06H register
	len = 0;											//packet length
	
	ready = dm9000_reg_read(DM9000_MRCMDX);				//this is MRCMDX,after we first read,the point will not increase
	if((ready & 0x01) != 0x01)							//first we will read 0x00H
		{
			ready = dm9000_reg_read(DM9000_MRCMDX);		//second we will read 0x01H
			if((ready & 0x01) != 0x01)
				{
					if((ready & 0x01) != 0x00)			//after we read twice,if not 0x01or0x00,there is a error init it
						{
							dm9000_init();
						}
					return 0;
				}						
		}
		
	status = dm9000_reg_read16(DM9000_MRCMD);;			//notice:this is MRCMD,as we will read the data,so we use MRCMD
	msdelay(1);
	len = DM9000_DATA;									//read the length,16 bits mode,the reg has set by the prior step
	datas = (unsigned char *)&recv_buffer[0];		//now we begin to receive the real data

	for(i = 0; i < len; i += 2)							//16 bits mode
		{
			msdelay(1);									//the reg MRCMD has set by the prior prior step
			tem = DM9000_DATA;
			datas[i] = tem & 0x0ff;
			datas[i+1] = (tem>>8)&0x0ff;
		}
		
	recv_packet_len = len;
	
	net_receive(&recv_buffer[0],recv_packet_len);
		
	return recv_packet_len;
		
}

/******************************************************************************************************
* only we read the MRCMD,MRCMDX,or we write the MWCMD,MWCMDX,the point of the SDRAM will increase 1 or 2
* or 4 by the bits mode we have set ,otherwise it will not increase automactically.
*********************************************************************************************************/


