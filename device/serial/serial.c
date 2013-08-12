#include "s3c2440.h"
#include "serial.h"

#define	TXD0READY	(1<<2)
#define	RXD0READY	(1)

void init_uart(void)
{
	GPHCON |= 0xa0;			//GPH2,GPH3 used as TXD0,RXD0
	GPHUP	= 0x0c;			//GPH2,GPH3禁止上拉

	ULCON0	= 0x03;			//8N1，8位数据，一位停止位，无奇偶校验
	UCON0	= 0x05;			//使用查询方式，时钟采用PCLK
	UFCON0 	= 0x00;			//不使用FIFO
	UMCON0 	= 0x00;			//不使用流控
	UBRDIV0 = 26;			//波特率为115200

}

void putc(unsigned char c)
{
	while( ! (UTRSTAT0 & TXD0READY) );
	UTXH0 = c;
}

unsigned char getc(void)
{
#ifdef SERIAL_ECHO
	unsigned char ret;
#endif	
	while( ! (UTRSTAT0 & RXD0READY) );
	ret = URXH0;
#ifdef SERIAL_ECHO					//如果支持回显，将接收到的数据立即显示在串口上
	if (ret == 0x0d || ret == 0x0a) //一个是换行一个是回车
	{
		putc(0x0d);
		putc(0x0a);
	}
	else
	{
		putc(ret);		
	}
#endif	
	return ret;
}

