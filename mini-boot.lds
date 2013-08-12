ENTRY(_start)
SECTIONS
{
	. = 0x33000000;
	.text :{
		*(.text)
	}	
	. = ALIGN(32);
	.data :{
		*(.data)
	}
	. = ALIGN(32);
	.bss :{
		*(.bss)
	}
}
