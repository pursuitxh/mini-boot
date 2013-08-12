/*
 * =====================================================================================
 *
 *       Filename:  timer.c
 *
 *    Description:  S3C2440时钟和定时器相关函数
 *
 *        Version:  1.0
 *        Created:  2011年04月01日 10时45分33秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Weimeng, pursuitxh@163.com
 *        Company:  Sourth East University 
 *
 * =====================================================================================
 */

#define TCFG0   *(volatile unsigned long *)(0x51000000)					//配置分频比寄存器
#define TCON    *(volatile unsigned long *)(0x51000008)					//配置寄存器，如自动转载，启动定时器
#define TCNTB4  *(volatile unsigned long *)(0x5100003C)					//装载寄存器计数值寄存器
#define TCNTO4  *(volatile unsigned long *)(0x51000040)   				//读取当前计数器值寄存器

#define CONFIG_SYS_HZ  1562500											//定时器工作频率：PCLK（50MHz）/（（15+1）*2）

int timer_load_val = 15625; 											//每次定时10ms 15625*（1/1562500）=0.1s=10 ms
static unsigned long timestamp;											//已经逝去的时间计数总量
static unsigned long lastdec;											//上一次递减到的计数值
int timer_init (void)
{
	TCFG0 = 0x0f00;																									
	if (timer_load_val == 0)
	{
	    timer_load_val = 15625;
	}
	TCNTB4 = timer_load_val;											//每次定时10ms					
	TCON = (TCON & ~0x0700000) | 0x600000;								//第一次，先手动装载计数值
	TCON = (TCON & ~0x0700000) | 0x500000;								//再启动自动转载计数值
	
	lastdec = timer_load_val;
    timestamp = 0;

	return (0);
}

unsigned long get_timer_masked (void)
{
	unsigned long now = TCNTO4 & 0xffff;								//读取当前计数器的值

	if (lastdec >= now) {												//小于上一次的值，直接进行累加
		timestamp += lastdec - now;
	} else {													//如果大于上一次的值，说明已经重新开始下一次定时，加上155625
		timestamp += lastdec + timer_load_val - now;
		/*timestamp += 155625*/
	}
	lastdec = now;

	return timestamp;													//返回已经计数的总量
}

void msdelay(unsigned long msec)
{
    timer_init();

    unsigned long tmo, now;
    tmo = timer_load_val * msec;
    tmo = tmo / 10;													    //定时msec毫秒，需要tmo个计数值全部减完

    unsigned long start = get_timer_masked();							//当前计数值
    do
    {
        now = get_timer_masked();
    }
    while ( (now - start) < tmo );										//直到减够tmo个数值时，就定时了msec毫秒
}
