#include "s3c2440.h"
#include "stdio.h"
#include "serial.h"
#include "mini-boot.h"
#include "configs.h"
#include "dm9000.h"


unsigned char mac_addr[6]={0x22,0x44,0x66,0x88,0x00,0x22};
unsigned char ip_addr[4]={192,168,0,230};
unsigned char host_ip_addr[4]={192,168,0,231};
unsigned char host_mac_addr[6]={0xff,0xff,0xff,0xff,0xff,0xff};

unsigned char send_buffer[1024];
unsigned char recv_buffer[1024];
unsigned long send_packet_len;
unsigned long recv_packet_len;

void tftp_select(void);

int main(void)
{
	int num;	
	init_uart();										//波特率为115200，8位数据，一位停止位，无奇偶校验，无流控
	dm9000_init();
	msdelay(3000);	
	arp_request();
	
while(1){
	printf("\n******************************************\n\r");
	printf("\n**********mini-boot for mini2440**********\n\r");
	printf("\n*************love ARM/Linux***************\n\r");
	printf("1:Download Kernel from Kermit Serial.\n\r");
	printf("2:Download Kernel from TFTP Server.\n\r");
	printf("3:Boot Linux from SDRAM.\n\r");
	printf("4:Boot Linux from NandFlash.\n\r");
	printf("\nSelect Number: ");
	
	scanf("%d",&num);
	
	switch (num)
	{
		case 1:
			boot_linux_serial();		
			break;
		case 2:
			tftp_select();
			//tftp_download((unsigned char*)SDRAM_KERNEL_START, "zImage_T35");
			break;
		case 3:
			boot_linux_sdram();
			break;
        case 4:
            printf("now booting linux from nand flash ...\n\r");
			boot_linux();
            break;
		default:
			printf("error: wrong number!\n\r");
			break;
	}
}
	
	return 0;
} 

void tftp_select(void)
{
	char filename[20];
	int addr;	
	
	printf("Please input the file name you want.\n\r");
	//printf("And the address where you want to download.\n\r");	
	printf("You input: ");
	//scanf("%s %x",filename, &addr);
	scanf("%s",filename);
	//printf("you have input %s, %x\n\r", filename, addr);	
	//msdelay(1000);
	tftp_download((unsigned char*)SDRAM_KERNEL_START, filename);
	
}
