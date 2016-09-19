#include <stddef.h>
#include <stdint.h>
#include "debuglog.h"
#include "enc_internal.h"
#include "brc.h"
#include "vsoc_enc.h"
#include "interrupt.h"
#include "reg_map.h"

void VEBRC_IRQ_Wrap_Handler(void)
{
    VEBRC_IRQ_Handler();
}

int H264_Encoder_Init(void)
{
    //variable Declaraton 
    int saved_view0_frame_cnt_lyu=0;
    int saved_view1_frame_cnt_lyu=0;
    char spi_rd_dat, i2c_rd_dat;
    unsigned int wait_cnt, i;
    unsigned char read_cnt ;

    //==== Video_Soc Wait SDRAM INIT_DONE ===//
    sdram_init_check(); 
    dlog_info("sdram init OK\n");

    reg_IrqHandle(VEBRC_VECTOR_NUM, VEBRC_IRQ_Wrap_Handler);

    NVIC_ISER->ISER1 = 0x30000000;

#if 0 // Test in emulator    
    GPIO_CFG_PTR p_gpio_cfg = (GPIO_CFG_PTR)0x40b0007c;
    p_gpio_cfg->SFR_PAD_CTRL0   = 0x00000000;
    p_gpio_cfg->SFR_PAD_CTRL1   = 0x00000000;
    p_gpio_cfg->SFR_PAD_CTRL2   = 0x50000000;
    p_gpio_cfg->SFR_PAD_CTRL3   = 0xFFFFF005;
    p_gpio_cfg->SFR_PAD_CTRL4   = 0x005FFFFF;
    p_gpio_cfg->SFR_PAD_CTRL5   = 0xFFF00000;
    p_gpio_cfg->SFR_PAD_CTRL6   = 0x0F000FFF;
    p_gpio_cfg->SFR_PAD_CTRL7   = 0x00014000;
    p_gpio_cfg->SFR_PAD_CTRL8   = 0x00000000;
    p_gpio_cfg->SFR_DEBUG0      = 0x00000001;
#endif

    /* BR - 0: 8Mbps, 1: 1Mbps, 4: 4Mbps, 8: 500kbps, 10: 10Mbps */
    //init_view0( 1280, 720, 10, 30, 1); //init_view0( 1920, 1080, 10, 30, 10);
    init_view0( 640, 480, 10, 60, 1); //init_view1( 1920, 1080, 10, 30, 10);
    v0_poweron_rc_params_set = 1;
    //init_view1( 1280, 720, 10, 30, 1); //init_view1( 1920, 1080, 10, 30, 10);
    init_view1( 640, 480, 10, 60, 1); //init_view1( 1920, 1080, 10, 30, 10);
    v1_poweron_rc_params_set = 1;

    my_v0_initial_all( );
    dlog_info("View0 Initial Done\n");
    my_v1_initial_all( );
    dlog_info("View1 Initial Done\n");

    /* Enable view 0 */
    open_view0( 1 ); 

    //waiting for irq. update QP
    while(1) 
    {
        for(i=0;i<=1000;i++) 
        {
            if(i==2) 
            {
                /* printf gop_cnt & frame_cnt */
                if(rca.v0_frame_cnt != saved_view0_frame_cnt_lyu) 
                {
                    dlog_info("G: %d\n", rca.v0_gop_cnt);
                    dlog_info("F: %d\n", rca.v0_frame_cnt);
                    saved_view0_frame_cnt_lyu=rca.v0_frame_cnt;
                }
            }

            if(i==2) 
            {
                if(rca.v1_frame_cnt != saved_view1_frame_cnt_lyu) 
                {
                    dlog_info("G: %d\n", rca.v1_gop_cnt);
                    dlog_info("F: %d\n", rca.v1_frame_cnt);
                    saved_view1_frame_cnt_lyu=rca.v1_frame_cnt;
                }
            }
        }
	// update_aof();
    }

    return 1;
}
