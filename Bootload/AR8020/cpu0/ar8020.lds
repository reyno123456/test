OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(vectors)

/* Highest address of the user mode stack */
_estack = 0x20004000;     /* end of RAM */

MEMORY
{
    ITCM0      : ORIGIN = 0x00000000, LENGTH = 0x80000
    DTCM0      : ORIGIN = 0x20004000, LENGTH = 0x80000
}

SECTIONS
{
    .vectors :
    {
        *(.isr_vectors)
    } > ITCM0

    .start :
    {
        *(.start*)
    } > ITCM0

    .text :
    {
        _text_start = . ;
        *(.text)
    } > ITCM0

    _text_end = _text_start + SIZEOF(.text) ;
    
    _data_start = . ;

    .rodata :
    {
        *(.rodata*)
    } > DTCM0 AT> ITCM0

    .data :
    {
        *(.data)
        . = ALIGN(4);
        *(.ramcode)
    } > DTCM0 AT> ITCM0

    _data_end = _data_start + SIZEOF(.rodata) + SIZEOF(.data); 

    .bss :
    {
        _bss_start = .;
        *(.bss)
        *(COMMON)
        _bss_end = .;
    } > DTCM0
}


