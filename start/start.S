/******************************************************************
*定义各模式堆栈空间
*******************************************************************/
.equ	FIQ_STACK,		0x32FFF000
.equ	IRQ_STACK,		0x32FFE000
.equ	ABORT_STACK,	0x32FFD000
.equ	UNDEFINE_STACK,	0x32FFC000
.equ	USER_STACK,		0x32FFB000
.equ	SVC_STACK,		0x32FFA000
/******************************************************************
*中断向量表
*******************************************************************/
.text
.global	_start
_start:	b	start_code						@0x00,复位异常
		ldr	pc, _undefined_instruction		@0x04，未定义指令异常
		ldr pc, _software_interrupt			@0x08，软中断
		ldr pc, _prefetch_abort				@0x0c，指令预取终止异常
		ldr	pc, _data_abort					@0x10，数据访问终止异常
		ldr	pc, _not_used					@0x14，保留
		ldr	pc, _irq						@0x18，中断
		ldr	pc, _fiq						@0x1c，快速中断
_undefined_instruction: .word undefined_instruction
_software_interrupt:	.word software_interrupt
_prefetch_abort:		.word prefetch_abort
_data_abort:			.word data_abort
_not_used:				.word not_used
_irq:					.word irq
_fiq:					.word fiq

.balignl 16, 0xdeadbeef
/******************************************************************
*代码开始
*******************************************************************/
start_code:
		mrs r0, cpsr						@设置为管理模式
		bic r0, r0, #0x1F					@ARM指令，禁止中断
		orr r0, r0, #0xD3					
		msr cpsr, r0

		ldr r0, =0x53000000					@关闭看门狗
		mov r1, #0x0
		str r1, [r0]
		
		ldr r0, =0x4C000014					@设置时钟分频比
		mov r1, #0x05						@FCLK:HCLK:PCLK = 1:4:8
		str r1, [r0]						
		
		mrc p15, 0, r0, c1, c0, 0			@如果HDIVN不为0，使其从
		orr r0, r0, #0xC0000000				@由快总线模式变为异步总线模式
		mcr p15, 0, r0, c1, c0, 0


		ldr r0, =0x4C000008					@设置UCLK=48MHz
		ldr r1, =0x00038022					@(56<<12)|(2<<4)|(2)
		str r1, [r0]


		ldr r0, =0x4C000004					@设置FCLK=400MHz
		ldr r1, =0x0005C011					@(92<<12)|(1<<4)|(1)			
		str	r1, [r0]

		mov r1, #0x48000000					@设置SDRAM
		adrl r2, mem_cfg_val
		add r3, r1, #52
1:
		ldr r4, [r2], #4
		str r4, [r1], #4
		cmp r1, r3
		bne 1b
/******************************************************************
*判断是从norflash还是从nandflash启动，然后复制代码到SDRAM，跳入第二阶段执行
*******************************************************************/
		/*adr r0, _start						//判断是在flash中还是在ram中运行
		ldr r1, =0x30000000
		cmp r0, r1
		beq stack_setup*/						
		
		ldr r1, =((4<<28)|(3<<4)|(3<<2))	//SRAM地址0X4000003C写0
		mov r0, #0
		str r0,[r1]
		
		mov r1, #0x3C						//读取地址0X0000003C处的值是否为0，若不是，则从norflash启动
		ldr r0,[r1]
		cmp r0,#0
		bne relocate
		
		ldr r0, =(0xdeadbeef)				//若为0，则从nandflash启动，并恢复地址0X4000003C处原来的值
		ldr r1, =((4<<28)|(3<<4)|(3<<2))
		str r0, [r1]
		
		/*#define BWSCON 0x48000000
		ldr r0, =BWSCON
		ldr r0, [r0]
		ands r0, r0, #6
		cmp r0, #0
		bne copy_loop*/
		
		ldr sp, =4096						//从nandflash中复制代码到SDRAM中
		bl s3c2440_nand_init
		ldr r0, =0x33000000
		mov r1, #0
		mov	r2, #102400
		bl s3c2440_nand_read
		b laststep

not_match:									//如果从norflash中验证0x0000003c处不是0xdeadbeef,则出错
error:
		b error

relocate:	
		ldr r1, =(0xdeadbeef)				//验证0x0000003c处是不是0xdeadbeef
		cmp r0, r1
		bne error

		ldr r0, =0					//从norflash中复制代码到SDRAM
		ldr r1, =0x33000000
		mov r2, #102400
		
copy_loop:
		ldmia r0!, {r3-r10}
		stmia r1!, {r3-r10}
		cmp r0, r2
		ble copy_loop

		/*ldr r0, =0x56000010					@点亮LED4
		mov r1, #(1<<(8*2))
		str r1,[r0]
		ldr r0, =0x56000014
		mov r1, #(~(1<<8))
		str r1, [r0]*/

laststep:
		ldr lr, =halt_loop
		ldr pc, =main

halt_loop:
		b halt_loop

/*******************************************************************
*存储控制器13个寄存器的设置值
********************************************************************/
.align 4
mem_cfg_val:
		.long	0x220D1110
		.long	0x00000700
		.long	0x00000700
		.long	0x00000700
		.long	0x00000700
		.long	0x00001f7c
		.long	0x00000700
		.long	0x00018005
		.long	0x00018005
		.long	0x008C07A3
		.long	0x000000B2
		.long	0x00000030
		.long	0x00000030

/******************************************************************
*设置各模式堆栈
*******************************************************************/
stack_setup:
		mrs r0, cpsr
		bic	r0, r0, #0x1F
		orr r0, r0, #0xDB
		msr cpsr, r0
		ldr sp, =UNDEFINE_STACK

		mrs r0, cpsr
		bic	r0, r0, #0x1F
		orr r0, r0, #0xD7
		msr cpsr, r0
		ldr sp, =ABORT_STACK

		mrs r0, cpsr
		bic	r0, r0, #0x1F
		orr r0, r0, #0xD2
		msr cpsr, r0
		ldr sp, =IRQ_STACK
		
		mrs r0, cpsr
		bic	r0, r0, #0x1F
		orr r0, r0, #0xD1
		msr cpsr, r0
		ldr sp, =FIQ_STACK

		mrs r0, cpsr
		bic	r0, r0, #0x1F
		orr r0, r0, #0xD0
		msr cpsr, r0
		ldr sp, =USER_STACK

		mrs r0, cpsr
		bic	r0, r0, #0x1F
		orr r0, r0, #0xD3
		msr cpsr, r0
		ldr sp, =SVC_STACK
/*******************************************************************
*中断处理
********************************************************************/
undefined_instruction:
		b undefined_instruction
software_interrupt:
		b software_interrupt
prefetch_abort:
		b prefetch_abort
data_abort:
		b data_abort
not_used:
		b not_used
irq:
		b irq
fiq:
		b fiq
		
