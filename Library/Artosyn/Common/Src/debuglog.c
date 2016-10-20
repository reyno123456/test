#include <stdio.h>
#include "debuglog.h"
#include "serial.h"

static char* g_debug_log_write_pos = (char*)SRAM_DEBUG_LOG_BUFFER_ST_ADDR_0;
static char* g_debug_log_read_pos = (char*)SRAM_DEBUG_LOG_BUFFER_ST_ADDR_0;
static unsigned char g_debug_log_sram_buf_index = SRAM_DEBUG_LOG_SPACE_0;
static char* g_debug_log_buf_head = (char*)SRAM_DEBUG_LOG_BUFFER_ST_ADDR_0;
static char* g_debug_log_buf_tail = (char*)SRAM_DEBUG_LOG_BUFFER_END_ADDR_0;

static unsigned int strcpy_to_debug_log_buf(const char *src)
{
    char* dst  = g_debug_log_write_pos;

    while (*src)
    {
        *dst = *src;
        if (dst >= g_debug_log_buf_tail)
        {
            dst = g_debug_log_buf_head;
        }
        else
        {
            dst++;
        }
        src++;

#ifdef DEBUG_LOG_BUFFER_WRITE_NO_OVERLAP
        // If reach the read position, then stop the write.
        if (dst == g_debug_log_read_pos)
        {
            break;
        }
#endif
    }    

    g_debug_log_write_pos = dst;
}

int puts(const char * s)
{
#ifdef USE_SRAM_DEBUG_LOG_BUFFER
    // Print to buffer
    strcpy_to_debug_log_buf(s);
#else
    // Print to serial
    serial_puts(s);
#endif

    return 0;
}

unsigned int dlog_output(unsigned int byte_num)
{
    unsigned int iByte = 0;

    char* src  = g_debug_log_read_pos;

    // Print to serial
    while (src != g_debug_log_write_pos)
    {
        serial_putc(*src);
        if (src >= g_debug_log_buf_tail)
        {
            src = g_debug_log_buf_head;
        }
        else
        {
            src++;
        }

        iByte++;
        if (iByte >= byte_num)
        {
            break;
        }
    }

    g_debug_log_read_pos = src;
    
    return iByte;
}

void dlog_init(unsigned int index)
{
    switch (index)
    {
    case SRAM_DEBUG_LOG_SPACE_0:
        g_debug_log_write_pos = (char*)SRAM_DEBUG_LOG_BUFFER_ST_ADDR_0;
        g_debug_log_read_pos = (char*)SRAM_DEBUG_LOG_BUFFER_ST_ADDR_0;
        g_debug_log_sram_buf_index = SRAM_DEBUG_LOG_SPACE_0;
        g_debug_log_buf_head = (char*)SRAM_DEBUG_LOG_BUFFER_ST_ADDR_0;
        g_debug_log_buf_tail = (char*)SRAM_DEBUG_LOG_BUFFER_END_ADDR_0;
        break;
    case SRAM_DEBUG_LOG_SPACE_1:
        g_debug_log_write_pos = (char*)SRAM_DEBUG_LOG_BUFFER_ST_ADDR_1;
        g_debug_log_read_pos = (char*)SRAM_DEBUG_LOG_BUFFER_ST_ADDR_1;
        g_debug_log_sram_buf_index = SRAM_DEBUG_LOG_SPACE_1;
        g_debug_log_buf_head = (char*)SRAM_DEBUG_LOG_BUFFER_ST_ADDR_1;
        g_debug_log_buf_tail = (char*)SRAM_DEBUG_LOG_BUFFER_END_ADDR_1;
        break;
    case SRAM_DEBUG_LOG_SPACE_2:
        g_debug_log_write_pos = (char*)SRAM_DEBUG_LOG_BUFFER_ST_ADDR_2;
        g_debug_log_read_pos = (char*)SRAM_DEBUG_LOG_BUFFER_ST_ADDR_2;
        g_debug_log_sram_buf_index = SRAM_DEBUG_LOG_SPACE_2;
        g_debug_log_buf_head = (char*)SRAM_DEBUG_LOG_BUFFER_ST_ADDR_2;
        g_debug_log_buf_tail = (char*)SRAM_DEBUG_LOG_BUFFER_END_ADDR_2;
        break;
    default:
        break;
    }
}

