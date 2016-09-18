OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY
{
    ITCM2 : ORIGIN = 0x00000000, LENGTH = 0x10000
    DTCM2 : ORIGIN = 0x20000000, LENGTH = 0x10000
}

SECTIONS
{
    .vectors :
    {
        __startup = . ;
        *(.vectors)
    } > ITCM2

    .start :
    {
        *(.start)
    } > ITCM2

    .text :
    {
        ___c_start = . ;
        text_start = . ;
        *(.text)
    } > ITCM2

    .rodata :
    {
        *(.rodata*)
        __etext = . ;
        . = ALIGN(4) ;
    } > ITCM2

    .ARM.exidx :
    {
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidx.*)
        . = ALIGN(4) ;
        _data_lma_start = . ;
    } > ITCM2

    .data :
    {
        _data_start = . ;
        *(.data)
        . = ALIGN(4);
        *(.ramcode)
        _data_end = . ;
    } > DTCM2 AT > ITCM2

    .bss :
    {
        _bss_start = .;
        *(.bss)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > DTCM2
}


