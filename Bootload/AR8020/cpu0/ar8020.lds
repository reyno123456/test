OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY 
{
        ITCM0     : ORIGIN = 0x00000000, LENGTH = 0x80000
        ITCM1     : ORIGIN = 0x00080000, LENGTH = 0x80000
        FLASH	  : ORIGIN = 0x10000000, LENGTH = 0x80000
        DTCM0     : ORIGIN = 0x44080000, LENGTH = 0x80000
        DTCM1     : ORIGIN = 0x44180000, LENGTH = 0x80000
        SRAM      : ORIGIN = 0X21000000, LENGTH = 0xFFFFF
}

SECTIONS
{
    .vectors :
    {
		_vectors_start = . ;
		*(.vectors)
		_vectors_end = . ;
    } > ITCM0

	.start :
	{
		*(.start)
	} > ITCM0

    .text :
    {
		_text_start = . ;
		*(.text)
		_text_end = . ;
    } > ITCM0

    .rodata :
    {
		*(.rodata*)
    } > ITCM0

    .data :
    {
		*(.data)
		. = ALIGN(4) ;
		*(.ramcode)
    } > ITCM0

	.flash_data :
	{
		. = ALIGN(4);
		_flash_data_start = . ;
		_flash_data_load  = LOADADDR(.flash_data) ;
		*(.flash_data)
		_flash_data_end = . ;
	} > ITCM0

    .bss :
    {
		*(.bss)
		*(COMMON)
    } > ITCM0

	.stack :
	{
		*(.stack)
	} 


}


