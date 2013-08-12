#include "atag.h"
#include "string.h"
#include "stdio.h"
#include "configs.h"
#include <mini-boot.h>

static struct Atag *pCurTag; 							/* used to point at the current ulTag */

void (*theKernel)(int, int, unsigned int);
  
const char *CmdLine = "root=/dev/mtdblock3 console=ttySAC0,115200 mem=64M init=/linuxrc";
static void setup_core_tag(void *pStartAddr)
{
    pCurTag = (struct Atag *)pStartAddr;      			/* start at given address */

    pCurTag->stHdr.ulTag = ATAG_CORE;          			/* start with the stCore ulTag */
    pCurTag->stHdr.nSize = TAG_SIZE(TagCore); 			/* nSize the ulTag */

    pCurTag->u.stCore.ulFlags = 0;               		/* can write */
    pCurTag->u.stCore.nPageSize = 4096;     			/* systems pagenSize (4k) */
    pCurTag->u.stCore.ulRootDev = 0;     				/* zero (typicaly overidden from commandline )*/

    pCurTag = TAG_NEXT(pCurTag);              			/* move pointer to next ulTag */
}

static void setup_mem_tag(unsigned int start, unsigned int len)
{
    pCurTag->stHdr.ulTag = ATAG_MEM;             		/* Memory ulTag */
    pCurTag->stHdr.nSize = TAG_SIZE(TagMem32);  		/* nSize ulTag */

    pCurTag->u.stMem.ulStart = start;     				/* Start of memory area (physical address) */
    pCurTag->u.stMem.nSize = len;               		/* Length of area */

    pCurTag = TAG_NEXT(pCurTag);              			/* move pointer to next ulTag */
}

static void setup_cmdline_tag(const char * line)
{
    int linelen = strlen(line);

    if(!linelen)
        return;                  						/* do not insert a ulTag for an empty commandline */

    pCurTag->stHdr.ulTag = ATAG_CMDLINE;         		/* Commandline ulTag */
    pCurTag->stHdr.nSize = (sizeof(struct TagHeader) + linelen + 1+4) >> 2;

    strcpy(pCurTag->u.stCmdLine.cCmdLine,line); 		/* place commandline into ulTag */

    pCurTag = TAG_NEXT(pCurTag);              			/* move pointer to next ulTag */
}

static void setup_end_tag(void)
{
    pCurTag->stHdr.ulTag = ATAG_NONE;            		/* Empty ulTag ends list */
    pCurTag->stHdr.nSize = 0;                   		/* zero length */
}

int boot_linux()
{
    setup_core_tag( (void *)SDRAM_TAGS_START );   /* standard stCore ulTag 4k pagenSize */
    setup_mem_tag(SDRAM_ADDR_START, SDRAM_TOTAL_SIZE);    /* 64Mb at 0x30000000 */
    setup_cmdline_tag( CmdLine );   
    setup_end_tag();                    /* end of ulTags */
    
    s3c2440_nand_init();

    printf("copy Kernel from nandflash to SDRAM... ...\n\r");
    s3c2440_nand_read((unsigned char *)SDRAM_KERNEL_START, NAND_KERNEL_START, NAND_KERNEL_SZIE);
    printf("copy Kernel from nandflash to SDRAM has finished!\n\r");
    printf("Now jump to the kernel entry!\n\r");
    
    theKernel = (void (*)(int, int, unsigned int))SDRAM_KERNEL_START;

    theKernel(0, S3C2440_MATHINE_TYPE, SDRAM_TAGS_START);
    
    return 0;
}



int boot_linux_serial()
{
    printf("Before GtSerialLoad\n\r");
    GtSerialLoad((void *)SDRAM_KERNEL_START);
    printf("After GtSerialLoad\n\r");
    
    return 0;
}

int boot_linux_sdram()
{
    setup_core_tag( (void *)SDRAM_TAGS_START );  		/* standard stCore ulTag 4k pagenSize */
    setup_mem_tag(SDRAM_ADDR_START, SDRAM_TOTAL_SIZE);  /* 64Mb at 0x30000000 */
    setup_cmdline_tag( CmdLine );   
    setup_end_tag();                    				/* end of ulTags */
   
    theKernel = (void (*)(int, int, unsigned int))SDRAM_KERNEL_START;

    theKernel(0, S3C2440_MATHINE_TYPE, SDRAM_TAGS_START);

    return 0;
}



