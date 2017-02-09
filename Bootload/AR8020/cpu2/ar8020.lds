OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(vectors)

/* Highest address of the user mode stack */
_estack = 0x20010000;     /* end of RAM */


MEMORY
{
    ITCM2      : ORIGIN = 0x00000000, LENGTH = 0x10000
    DTCM2      : ORIGIN = 0x20000000, LENGTH = 0x10000
}

SECTIONS
{
    .vectors :
    {
        *(.isr_vectors)
    } > ITCM2

    .start :
    {
        *(.start*)
    } > ITCM2


    .text :
    {
        _text_start = . ;
        *(.text)
    } > ITCM2

    _text_end = _text_start + SIZEOF(.text) ;
    

    _data_start = . ;
    .rodata :
    {
        *(.rodata*)
    } > DTCM2 AT> ITCM2

   /* .data_array :
    {
        *(.data_array*)
    } > DTCM2 AT> ITCM2
    */
    .data :
    {
        *(.data)
        . = ALIGN(4);
        *(.ramcode)
    } > DTCM2 AT> ITCM2

    .data_e :
    {
        _data_end = . ;
    } AT> ITCM2

    .bss :
    {
        _bss_start = .;
        *(.bss)
        *(COMMON)
        _bss_end = .;
    } > DTCM2
}


