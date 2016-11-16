OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(vectors)

/* Highest address of the user mode stack */
_estack = 0x2003FFFF;     /* end of RAM */


MEMORY
{
    ITCM1      : ORIGIN = 0x00000000, LENGTH = 0x40000
    DTCM1      : ORIGIN = 0x20000000, LENGTH = 0x40000
}

SECTIONS
{
    .vectors :
    {
        *(.isr_vectors)
    } > ITCM1

    .start :
    {
        *(.start*)
    } > ITCM1


    .text :
    {
    _text_start = . ;
        *(.text)
    } > ITCM1

    

    .rodata :
    {
        *(.rodata*)
    } > ITCM1

    _text_end = _text_start + SIZEOF(.text) + SIZEOF(.rodata);

    _data_start = . ;

    .data :
    {
        *(.data)
        . = ALIGN(4);
        *(.ramcode)
    } > DTCM1 AT > ITCM1

    _data_end = _data_start + SIZEOF(.data);
    
    .bss :
    {
        _bss_start = .; 
        *(.bss)
        *(COMMON)
        _bss_end = .;
    } > DTCM1


}


