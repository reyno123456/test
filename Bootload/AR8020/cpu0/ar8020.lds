OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(vectors)

/* Highest address of the user mode stack */
_estack = 0x20010000;     /* end of RAM */
_Heap_Size = 0x8000;      /* required amount of heap  */
_Stack_Size = 0x10000;    /* required amount of stack */
_Data_Size = 0x20000;      /* amount of Data */

MEMORY 
{
        ITCM0     : ORIGIN = 0x00000300, LENGTH = 0x80000
        /*DTCM0     : ORIGIN = 0x20000000, LENGTH = 0x80000*/
        DTCM0_RO  : ORIGIN = 0X20018000, LENGTH = 0x80000
        FLASH     : ORIGIN = 0x10000000, LENGTH = 0x80000
}

SECTIONS
{
    .isr_vectors :
    {
		_vectors_start = . ;
		*(.isr_vectors)
		_vectors_end = . ;
    } > FLASH

	.start :
	{
		*(.start.*)
	} > FLASH

	_text_start = ADDR(.start) + SIZEOF(.start) ;

    .text : 
    {
		*(.text)
    } >ITCM0 AT > FLASH

    _text_end = _text_start + SIZEOF(.text) ;

	_data_start = _text_end;
    
    .rodata :
    {
		_rodata_start = . ;
		*(.rodata*)
		. = ALIGN(4) ;
		_rodata_end = . ;
    } >DTCM0_RO AT> FLASH

    /* the flash data is added to test the sd read/write function */
    .flash_data :
    {
        . = ALIGN(4) ; 
        _flash_start = . ;
        *(.flash_data)
        _flash_end = . ;
    } > DTCM0_RO AT > FLASH

    .data : 
    {
        *(.data)
        . = ALIGN(4) ;
        *(.ramcode)
    } >DTCM0_RO AT> FLASH

    _data_end = _data_start + SIZEOF(.rodata) + SIZEOF(.data) + SIZEOF(.flash_data);

    .bss :
    {
        _bss_start = . ;
		*(.bss)
		*(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > DTCM0_RO AT > FLASH



}


