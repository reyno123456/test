OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY
{
    startup : ORIGIN = 0x00000000, LENGTH = 0x10000
    ram     : ORIGIN = 0x20000000, LENGTH = 0x10000
}

SECTIONS
{
    .vectors :
    {
        __startup = . ;
        *(.vectors)
    } > startup

    .start :
    {
        *(.start)
    } > startup

    .text :
    {
        ___c_start = . ;
        text_start = . ;
        *(.text)
    } > startup

    .rodata :
    {
        *(.rodata*)
        __etext = . ;
    } > startup

    .data_array :
    {
        *(.data_array*)
        __etext = . ;
    } > startup

    .data :
    {
        *(.data)
        . = ALIGN(4);
        *(.ramcode)
    } > ram

    .bss :
    {
        *(.bss)
        *(COMMON)
    } > ram

    .stack :
    {
        *(.stack)
    } > ram
}


