#include "printlog.h"
#include "serial.h"

void putchar(char c)
{
    serial_putc(c);
}

void printf(const char *s)
{
    serial_puts(s);
}
