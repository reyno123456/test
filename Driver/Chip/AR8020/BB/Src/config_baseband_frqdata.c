#include "config_baseband_frqdata.h"
#include "config_functions_sel.h"

#include "stdint.h"
#if defined(GRD_RF8003_2P3) || defined(SKY_RF8003_2P3)
#endif

#if defined(GRD_RF8003_2P4) || defined(SKY_RF8003_2P4)



const struct RC_FRQ_CHANNEL Rc_frq[MAX_RC_FRQ_SIZE] = {         // 2.4G
    { 0,0x00,0x00,0x00,0x4b }, { 1,0x00,0x00,0x00,0x4c },
    { 2,0x00,0x00,0x00,0x4d }, { 3,0x00,0x00,0x00,0x4e },
    { 4,0x00,0x00,0x00,0x4f }, { 5,0x00,0x00,0x00,0x50 },
    { 6,0x00,0x00,0x00,0x51 }, { 7,0x00,0x00,0x00,0x52 },
    { 8,0x00,0x00,0x00,0x53 }, { 9,0x00,0x00,0x00,0x54 },
    { 10,0x00,0x00,0x00,0x55}, { 11,0x00,0x00,0x00,0x56}
};


const struct IT_FRQ_CHANNEL It_frq[MAX_RC_FRQ_SIZE] = {    //2.4G
    { 0,0x00,0x00,0x00,0x4d }, { 1,0x00,0x00,0x00,0x4e },
    { 2,0x00,0x00,0x00,0x4f }, { 3,0x00,0x00,0x00,0x50 },
    { 4,0x00,0x00,0x00,0x51 }, { 5,0x00,0x00,0x00,0x52 },
    { 6,0x00,0x00,0x00,0x53 }, { 7,0x00,0x00,0x00,0x54 },
};
#endif


//****************ad936x*************
#if defined(GRD_RF9363_2P3) || defined(SKY_RF9363_2P3)
#error "GRD_RF9363_2P3 SKY_RF9363_2P3 Not supported"
#endif

#if defined(GRD_RF9363_2P4) || defined(SKY_RF9363_2P4)
#error "GRD_RF9363_2P4 SKY_RF9363_2P4 Not supported"
#endif

#if defined(GRD_RF9363_2P5) || defined(SKY_RF9363_2P5)
#error "GRD_RF9363_2P5 SKY_RF9363_2P5 Not supported"
#endif

#if defined(GRD_RF9361_3p4) || defined(SKY_RF9361_3p4)
#error "GRD_RF9361_3p4 SKY_RF9361_3p4 Not supported"
#endif


#if defined(GRD_RF9361_3p5) || defined(SKY_RF9361_3p5)
#error "GRD_RF9361_3p5 SKY_RF9361_3p5 Not supported"
#endif

#if defined(GRD_RF9361_3p6) || defined(SKY_RF9361_3p6)
#error "GRD_RF9361_3p6 SKY_RF9361_3p6 Not supported"
#endif
