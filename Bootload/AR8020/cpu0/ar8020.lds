OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY 
{
    ITCM0     : ORIGIN = 0x00000000, LENGTH = 0x80000
    DTCM0     : ORIGIN = 0x20000000, LENGTH = 0x80000
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
        _rodata_start = . ;
        *(.rodata*)
        . = ALIGN(4) ;
        _rodata_end = . ;
    } > ITCM0

    .data : AT (_rodata_end)
    {
        _data_start = . ;
        *(.data)
        . = ALIGN(4) ;
        *(.ramcode)
        _data_end = . ;
    } > DTCM0

    .bss :
    {
        _bss_start = .;
        *(.bss)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > DTCM0

    .stack :
    {
        *(.stack)
    } 
}


