#include <stddef.h>
#include <stdint.h>
#include "debuglog.h"
#include "enc_internal.h"
#include "brc.h"
#include "vsoc_enc.h"
#include "interrupt.h"
#include "reg_map.h"

//#define RUN_IN_EMULATOR

static STRU_EncoderStatus g_stEncoderStatus[2] = { 0 };

void VEBRC_IRQ_Wrap_Handler(void)
{
    VEBRC_IRQ_Handler();
}

int H264_Encoder_Init(void)
{
    //variable Declaraton 
    char spi_rd_dat, i2c_rd_dat;
    unsigned int wait_cnt, i;
    unsigned char read_cnt ;

    //==== Video_Soc Wait SDRAM INIT_DONE ===//
    sdram_init_check(); 
    dlog_info("sdram init OK\n");

    reg_IrqHandle(VIDEO_ARMCM7_IRQ_VECTOR_NUM, VEBRC_IRQ_Wrap_Handler);

    NVIC_ISER->ISER1 = 0x30000000;

#ifdef RUN_IN_EMULATOR    // Test in emulator    
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
    init_view0( 720, 480, 10, 60, 1);
    v0_poweron_rc_params_set = 1;
    g_stEncoderStatus[0].resW = 720;
    g_stEncoderStatus[0].resH = 480;
    g_stEncoderStatus[0].gop = 10;
    g_stEncoderStatus[0].framerate = 60;
    g_stEncoderStatus[0].bitrate = 1;

    init_view1( 720, 480, 10, 60, 1);
    v1_poweron_rc_params_set = 1;
    g_stEncoderStatus[1].resW = 720;
    g_stEncoderStatus[1].resH = 480;
    g_stEncoderStatus[1].gop = 10;
    g_stEncoderStatus[1].framerate = 60;
    g_stEncoderStatus[1].bitrate = 1;

    my_v0_initial_all( );
    dlog_info("View0 Initial Done\n");
    my_v1_initial_all( );
    dlog_info("View1 Initial Done\n");

    /* Enable view 0 */
    open_view0( 1 ); 
    g_stEncoderStatus[0].brc_enable = 1;

    return 1;
}

int H264_Encoder_UpdateVideoInfo(unsigned char view, unsigned int resW, unsigned int resH, unsigned int framerate)
{
    if(g_stEncoderStatus[view].resW != resW ||
       g_stEncoderStatus[view].resH != resH ||
       g_stEncoderStatus[view].framerate != framerate)
    {
        if (view == 0)
        {
            close_view0();
            init_view0(resW, resH, g_stEncoderStatus[0].gop, framerate, g_stEncoderStatus[0].bitrate);
            open_view0(g_stEncoderStatus[0].brc_enable);
        }
        else if (view == 1)
        {
            close_view1();
            init_view1(resW, resH, g_stEncoderStatus[1].gop, framerate, g_stEncoderStatus[1].bitrate);
            open_view1(g_stEncoderStatus[1].brc_enable);
        }
        
        g_stEncoderStatus[view].resW = resW;
        g_stEncoderStatus[view].resH = resH;
        g_stEncoderStatus[view].framerate = framerate;
    }
}

int H264_Encoder_UpdateBitrate(unsigned char view, unsigned char br)
{
    if (g_stEncoderStatus[view].brc_enable && (g_stEncoderStatus[view].bitrate != br))
    {
        if (view == 0)
        {
            close_view0();
            init_view0(g_stEncoderStatus[0].resW, g_stEncoderStatus[0].resH, g_stEncoderStatus[0].gop, g_stEncoderStatus[0].framerate, br);
            open_view0(g_stEncoderStatus[0].brc_enable);
        }
        else if (view == 1)
        {
            close_view1();
            init_view1(g_stEncoderStatus[1].resW, g_stEncoderStatus[1].resH, g_stEncoderStatus[1].gop, g_stEncoderStatus[1].framerate, br);
            open_view1(g_stEncoderStatus[1].brc_enable);
        }
        g_stEncoderStatus[view].bitrate = br;
    }
}

void H264_Encoder_DumpFrameCount(void)
{
    //waiting for irq. update QP
    unsigned char i;
    static int saved_view0_frame_cnt_lyu=0;
    static int saved_view1_frame_cnt_lyu=0;

    /* printf gop_cnt & frame_cnt */
    if(rca.v0_frame_cnt != saved_view0_frame_cnt_lyu) 
    {
        dlog_info("G: %d\n", rca.v0_gop_cnt);
        dlog_info("F: %d\n", rca.v0_frame_cnt);
        saved_view0_frame_cnt_lyu=rca.v0_frame_cnt;
    }

    if(rca.v1_frame_cnt != saved_view1_frame_cnt_lyu) 
    {
        dlog_info("G: %d\n", rca.v1_gop_cnt);
        dlog_info("F: %d\n", rca.v1_frame_cnt);
        saved_view1_frame_cnt_lyu=rca.v1_frame_cnt;
    }
    // update_aof();
}
