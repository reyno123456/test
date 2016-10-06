#ifndef __CONFIG_BASEBAND_FRQDATA_H
#define __CONFIG_BASEBAND_FRQDATA_H

#include "config_functions_sel.h"
#include "stdint.h"

struct RC_FRQ_CHANNEL             //  Remote Controller Freq Channnel
{
    uint8_t num;
    uint8_t frq1;
    uint8_t frq2;
    uint8_t frq3;
    uint8_t frq4;
    uint8_t frq5;
};
struct IT_FRQ_CHANNEL               //  Image Transmissions Freq Channnel
{
   uint8_t num;
   uint8_t frq1;
   uint8_t frq2;
   uint8_t frq3;
   uint8_t frq4;
   uint8_t frq5;
};

#define MAX_RC_FRQ_SIZE (12)

#define MAX_RC_FRQ_SIZE  (8)


#endif
