#include <stddef.h>
#include <stdint.h>
#include "debuglog.h"
#include "data_type.h"
#include "enc_internal.h"
#include "brc.h"
#include "vsoc_enc.h"
#include "interrupt.h"
#include "reg_map.h"
#include "sys_event.h"

static STRU_EncoderStatus g_stEncoderStatus[2] = { 0 };

static int H264_Encoder_RestartView(unsigned char view, unsigned int resW, unsigned int resH, unsigned int gop, unsigned int framerate, unsigned int bitrate, unsigned char brc_enable)
{
    if (view >= 2)
    {
        return 0;
    }
    
    INTR_NVIC_DisableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);

    if (view == 0)
    {
        close_view0();
        init_view0(resW, resH, gop, framerate, bitrate);
        my_v0_initial_all( );
        open_view0(brc_enable);
    }
    else if (view == 1)
    {
        close_view1();
        init_view1(resW, resH, gop, framerate, bitrate);
        my_v1_initial_all( );
        open_view1(brc_enable);
    }

    if (brc_enable)
    {
        INTR_NVIC_EnableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);
    }
}

static int H264_Encoder_UpdateVideoInfo(unsigned char view, unsigned int resW, unsigned int resH, unsigned int framerate)
{
    if (view >= 2)
    {
        return 0;
    }
    
    if(g_stEncoderStatus[view].resW != resW ||
       g_stEncoderStatus[view].resH != resH ||
       g_stEncoderStatus[view].framerate != framerate)
    {
        H264_Encoder_RestartView(view, resW, resH, g_stEncoderStatus[view].gop, 
                                 framerate, g_stEncoderStatus[view].bitrate, 
                                 g_stEncoderStatus[view].brc_enable);
        
        g_stEncoderStatus[view].resW = resW;
        g_stEncoderStatus[view].resH = resH;
        g_stEncoderStatus[view].framerate = framerate;

        g_stEncoderStatus[view].running = 1;

        dlog_info("Video format change: %d, %d, %d, %d\n", view, resW, resH, framerate);
    }

    return 1;
}

static void H264_Encoder_InputVideoFormatChangeCallback(void* p)
{
    uint8_t index  = ((STRU_SysEvent_ADV7611FormatChangeParameter*)p)->index;
    uint16_t width = ((STRU_SysEvent_ADV7611FormatChangeParameter*)p)->width;
    uint16_t hight = ((STRU_SysEvent_ADV7611FormatChangeParameter*)p)->hight;
    uint8_t framerate = ((STRU_SysEvent_ADV7611FormatChangeParameter*)p)->framerate;

    // ADV7611 0,1 is connected to H264 encoder 1,0
    H264_Encoder_UpdateVideoInfo((index == 0) ? 1 : 0, width, hight, framerate);    
}

/*
 * br:   0->8M      1->1M       2->2M       3->3M       4->4M
 *       5->5M      6->6M       7->7M       8->500K     9->9M
 *       10->10M    11->11M     12->12M     13->13M     14->14M  
 *       15->15M    16->16M     17->17M     18->18M     19->19M  
 *       20->20M    21->21M     22->22M     23->23M     24->24M  
 *       25->25M    26->26M     27->27M     28->28M     29->29M     30->30M
*/
int H264_Encoder_UpdateBitrate(unsigned char view, unsigned char br)
{
    if (view >= 2)
    {
        return 0;
    }
    
    if (g_stEncoderStatus[view].running && g_stEncoderStatus[view].brc_enable && (g_stEncoderStatus[view].bitrate != br))
    {
        if (view == 0)
        {
            Reg_Write32_Mask(ENC_REG_ADDR+(0xA<<2), br << 26, BIT(26) | BIT(27) | BIT(28) | BIT(29) | BIT(30));
        }
        else
        {
            Reg_Write32_Mask(ENC_REG_ADDR+(0x23<<2), br << 26, BIT(26) | BIT(27) | BIT(28) | BIT(29) | BIT(30));
        }
        dlog_info("Encoder bitrate change: %d, %d\n", view, br);
    }
    
    g_stEncoderStatus[view].bitrate = br;
    return 1;
}

static void H264_Encoder_BBModulationChangeCallback(void* p)
{
    uint8_t br = ((STRU_SysEvent_BB_ModulationChange *)p)->BB_MAX_support_br; //100Kbps

    if(br == 5)         //500Kbps
    {
        br = 8;
    }
    else if(br < 10)
    {
        dlog_error("br: not supported \r\n", br);
    }
    else if(br == 80)   //8Mbps
    {
        br = 0;
    }
    else
    {
        br = br / 10;
    }

    dlog_info("H264 bitrate: %d \r\n", br);
    H264_Encoder_UpdateBitrate(0, br);
    H264_Encoder_UpdateBitrate(1, br);
}

static void VEBRC_IRQ_Wrap_Handler(void)
{
    if (Reg_Read32(ENC_REG_ADDR+(0x09<<2)) & BIT(6))
    {
        // Encoder hang happens, then do reset.
        Reg_Write32(VSOC_SOFT_RESET, (~(BIT(3))));
        Reg_Write32(VSOC_SOFT_RESET, 0xFF);

        if (g_stEncoderStatus[0].running == 1)
        {
            H264_Encoder_RestartView(0, g_stEncoderStatus[0].resW, g_stEncoderStatus[0].resH, 
                                     g_stEncoderStatus[0].gop, g_stEncoderStatus[0].framerate, 
                                     g_stEncoderStatus[0].bitrate, g_stEncoderStatus[0].brc_enable);
            dlog_info("Reset view0");
        }

        if (g_stEncoderStatus[1].running == 1)
        {
            H264_Encoder_RestartView(1, g_stEncoderStatus[1].resW, g_stEncoderStatus[1].resH, 
                                     g_stEncoderStatus[1].gop, g_stEncoderStatus[1].framerate, 
                                     g_stEncoderStatus[1].bitrate, g_stEncoderStatus[1].brc_enable);
            dlog_info("Reset view1");
        }
    }
    else
    {
        uint8_t chReset = 0;
        
        if ((g_stEncoderStatus[0].running == 1) && (Reg_Read32(ENC_REG_ADDR+(0x34<<2)) & (BIT(24)|BIT(25)|BIT(26))))
        {
            // View0 encoder error, then do channel reset.
            Reg_Write32_Mask(ENC_REG_ADDR+(0x00<<2), 0, BIT(24));
            my_v0_initial_all( );
            Reg_Write32_Mask(ENC_REG_ADDR+(0x00<<2), BIT(24), BIT(24));
            Reg_Write32_Mask(ENC_REG_ADDR+(0x34<<2), 0, (BIT(24)|BIT(25)|BIT(26)|BIT(27)));
            chReset = 1;
            dlog_info("Reset channel 0");
        }

        if ((g_stEncoderStatus[1].running == 1) && (Reg_Read32(ENC_REG_ADDR+(0x34<<2)) & (BIT(28)|BIT(29)|BIT(30))))
        {
            // View1 encoder error, then do channel reset.
            Reg_Write32_Mask(ENC_REG_ADDR+(0x19<<2), 0, BIT(24));
            my_v1_initial_all( );
            Reg_Write32_Mask(ENC_REG_ADDR+(0x19<<2), BIT(24), BIT(24));
            Reg_Write32_Mask(ENC_REG_ADDR+(0x34<<2), 0, (BIT(28)|BIT(29)|BIT(30)|BIT(31)));
            chReset = 1;
            dlog_info("Reset channel 1");
        }

        if (chReset == 0)
        {
            // Normal status, then do bitrate control.
            VEBRC_IRQ_Handler();
        }
    }
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
    INTR_NVIC_DisableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);

    g_stEncoderStatus[0].gop = 10;
    g_stEncoderStatus[0].brc_enable = 1;
    g_stEncoderStatus[0].bitrate = 1;
    v0_poweron_rc_params_set = 1;

    g_stEncoderStatus[1].gop = 10;
    g_stEncoderStatus[1].brc_enable = 1;
    g_stEncoderStatus[1].bitrate = 1;
    v1_poweron_rc_params_set = 1;

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_ADV7611_FORMAT_CHANGE, H264_Encoder_InputVideoFormatChangeCallback);
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_SUPPORT_BR_CHANGE, H264_Encoder_BBModulationChangeCallback);

    return 1;
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