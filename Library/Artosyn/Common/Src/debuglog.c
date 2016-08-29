#include <stdio.h>
#include "debuglog.h"
#include "serial.h"

/*
void putchar(char c)
{
    serial_putc(c);
}
*/

int puts(const char * s)
{
    serial_puts(s);
    return 0;
}

