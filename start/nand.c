
#define TACLS	0
#define	TWRPH0	3
#define TWRPH1	0

#define BUSY	1

#define NAND_SECTOR_SIZE	2048		//一页2K大小
#define NAND_BLOCK_MASK		(NAND_SECTOR_SIZE - 1)

#define _REGb(x)	(*(volatile unsigned char *)(x))
#define _REGi(x)	(*(volatile unsigned int *)(x))

#define NANDFLASH_BASE		0x4E000000	//配置寄存器基地址

#define	NANDFLASH_CONF		_REGi(NANDFLASH_BASE + 0x0)
#define	NANDFLASH_CONT		_REGi(NANDFLASH_BASE + 0x4)
#define	NANDFLASH_CMD		_REGb(NANDFLASH_BASE + 0x8)
#define	NANDFLASH_ADDR		_REGb(NANDFLASH_BASE + 0xC)
#define	NANDFLASH_DATA		_REGb(NANDFLASH_BASE + 0x10)
#define	NANDFLASH_STAT		_REGb(NANDFLASH_BASE + 0x20)


static void s3c2440_wait_idle(void);
static void s3c2440_nand_select_chip(void);
static void s3c2440_nand_deselect_chip(void);
static void s3c2440_write_cmd(int);



static void delay(void)
{
	int i;
	for(i=0; i<10; i++);
}

static void s3c2440_nand_reset(void)
{
	s3c2440_nand_select_chip();
	s3c2440_write_cmd(0xff);
	s3c2440_wait_idle();
	s3c2440_nand_deselect_chip();
}

static void s3c2440_wait_idle(void)
{
	while(!(NANDFLASH_STAT & BUSY))
		delay();
}

static void s3c2440_nand_select_chip(void)
{
	NANDFLASH_CONT &= ~(1<<1);
	delay();
}

static void s3c2440_nand_deselect_chip(void)
{
	NANDFLASH_CONT |= (1<<1);
	delay();
}

static void s3c2440_write_cmd(int cmd)
{
	NANDFLASH_CMD = cmd;
}

static void s3c2440_write_addr(unsigned int addr)
{
	int col, page;
	
	col = addr & NAND_BLOCK_MASK;					//取地址的低11位，所为行地址
	page = addr / NAND_SECTOR_SIZE;					//去掉地址的低11位，保留其余位，也就是剩下的都作为列地址
	
	NANDFLASH_ADDR = col & 0xff;
	delay();
	NANDFLASH_ADDR = (col >> 8) & 0x0f;
	delay();
	NANDFLASH_ADDR = page & 0xff;
	delay();
	NANDFLASH_ADDR = (page >> 8) & 0xff;
	delay();	
}

void s3c2440_nand_init(void)
{
	NANDFLASH_CONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
	NANDFLASH_CONT = (1<<4)|(1<<1)|(1<<0);
	
	s3c2440_nand_reset();
}

void s3c2440_nand_read(unsigned char *buf, unsigned int start_addr, int size)
{
	int i,j;	
	
	if((start_addr & NAND_BLOCK_MASK) || (size & NAND_BLOCK_MASK))
		return;										//地址或长度不对齐
	s3c2440_nand_select_chip();
	for(i=start_addr; i<(start_addr + size);)
	{
		s3c2440_write_cmd(0x00);
		s3c2440_write_addr(i);
		s3c2440_write_cmd(0x30);
		s3c2440_wait_idle();
		
		for(j=0; j<NAND_SECTOR_SIZE; j++, i++)
		{
			*buf = (NANDFLASH_DATA & 0xFF);
			buf++;
		}
	}
	
	s3c2440_nand_deselect_chip();	
}
