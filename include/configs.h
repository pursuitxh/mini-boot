#ifndef CONFIGS_H
#define CONFIGS_H

/********** SDRAM and Nand Space Allocation ****************/
#define SDRAM_TOTAL_SIZE         0x04000000					//64M SDRAM
#define SDRAM_ADDR_START         0x30000000					//起始地址是0x3000，0000
#define SDRAM_LBOOT_START        0x33000000					//LBOOT的存放地址
#define SDRAM_TAGS_START         0x30000100					//tag列表的存放地址
#define SDRAM_KERNEL_START       0x33800000					//kernel的存放地址

#define NAND_KERNEL_START       0x00060000
#define NAND_KERNEL_SZIE        0x00500000

/*************** Linux Kernel Boot Parameters ***********/
#define S3C2440_MATHINE_TYPE    1999

#endif
