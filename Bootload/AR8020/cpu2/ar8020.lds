OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(vectors)

/* Highest address of the user mode stack */
_estack = 0x20004000;     /* end of RAM */


MEMORY
{
    ITCM2      : ORIGIN = 0x00000000, LENGTH = 0x10000
    DTCM2      : ORIGIN = 0x20000000, LENGTH = 0x10000
    DTCM2_DATA : ORIGIN = 0X20004000, LENGTH = 0X10000
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
    } > DTCM2_DATA AT> ITCM2

   /* .data_array :
    {
        *(.data_array*)
    } > DTCM2_DATA AT> ITCM2
    */
    .data :
    {
        *(.data)
        . = ALIGN(4);
        *(.ramcode)
    } > DTCM2_DATA AT> ITCM2

    _data_end = _data_start + SIZEOF(.rodata) + SIZEOF(.data); 

    .bss :
    {
        _bss_start = .;
        *(.bss)
        *(COMMON)
        _bss_end = .;
    } > DTCM2_DATA AT> ITCM2


}


