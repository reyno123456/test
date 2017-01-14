#ifndef MEMORY_H
#define MEMORY_H

void *malloc_simple(size_t size);
void free_simple(void *ap);
unsigned int get_configure_from_flash(void);
#define GET_WORD_FROM_ANY_ADDR(any_addr) ((uint32_t)(*any_addr) | \
                                         (((uint32_t)(*(any_addr+1))) << 8) | \
                                         (((uint32_t)(*(any_addr+2))) << 16) | \
                                         ((uint32_t)((*(any_addr+3))) << 24))
typedef struct 
{
    unsigned char hdmi_configure[263][3];
    unsigned char bb_sky_configure[4][256];
    unsigned char bb_grd_configure[4][256];
    unsigned char rf_configure[128];
}setting_configure;

#endif

