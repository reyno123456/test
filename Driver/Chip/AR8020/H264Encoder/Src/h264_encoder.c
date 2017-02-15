#include <stddef.h>
#include <stdint.h>
#include "debuglog.h"
#include "data_type.h"
#include "enc_internal.h"
#include "brc.h"
#include "vsoc_enc.h"
#include "h264_encoder.h"
#include "interrupt.h"
#include "reg_map.h"
#include "sys_event.h"
#include "reg_rw.h"
#include "bb_types.h"

static STRU_EncoderStatus g_stEncoderStatus[2] = { 0 };

static int H264_Encoder_StartView(unsigned char view, unsigned int resW, unsigned int resH, unsigned int gop, unsigned int framerate, unsigned int bitrate)
{
    if (view >= 2)
    {
        return 0;
    }

    INTR_NVIC_DisableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);

    if (view == 0)
    {
        init_view0(resW, resH, gop, framerate, bitrate);
        my_v0_initial_all( );
        open_view0(g_stEncoderStatus[view].brc_enable);
    }
    else if (view == 1)
    {
        init_view1(resW, resH, gop, framerate, bitrate);
        my_v1_initial_all( );
        open_view1(g_stEncoderStatus[view].brc_enable);
    }

    g_stEncoderStatus[view].resW = resW;
    g_stEncoderStatus[view].resH = resH;
    g_stEncoderStatus[view].framerate = framerate;
    g_stEncoderStatus[view].running = 1;

    if ((g_stEncoderStatus[0].brc_enable && g_stEncoderStatus[0].running) || 
        (g_stEncoderStatus[1].brc_enable && g_stEncoderStatus[1].running)) // view0 and view1 share the same BRC interrupt
    {
        INTR_NVIC_EnableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);
    }

    return 1;
}

static int H264_Encoder_RestartView(unsigned char view, unsigned int resW, unsigned int resH, unsigned int gop, unsigned int framerate, unsigned int bitrate)
{
    if (view >= 2)
    {
        return 0;
    }
    
    INTR_NVIC_DisableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);

    if (view == 0)
    {
        close_view0();
        H264_Encoder_StartView(view, resW, resH, gop, framerate, bitrate);
    }
    else if (view == 1)
    {
        close_view1();
        H264_Encoder_StartView(view, resW, resH, gop, framerate, bitrate);
    }

    return 1;
}

static int H264_Encoder_CloseView(unsigned char view)
{
    if (view >= 2)
    {
        return 0;
    }

    INTR_NVIC_DisableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);
    
    if (view == 0)
    {
        close_view0();
    }
    else if (view == 1)
    {
        close_view1();
    }

    g_stEncoderStatus[view].resW = 0;
    g_stEncoderStatus[view].resH = 0;
    g_stEncoderStatus[view].framerate = 0;
    g_stEncoderStatus[view].running = 0;

    if ((g_stEncoderStatus[0].brc_enable && g_stEncoderStatus[0].running) || 
        (g_stEncoderStatus[1].brc_enable && g_stEncoderStatus[1].running)) // view0 and view1 share the same BRC interrupt
    {
        INTR_NVIC_EnableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);
    }

    return 1;
}

static int H264_Encoder_UpdateVideoInfo(unsigned char view, unsigned int resW, unsigned int resH, unsigned int framerate)
{
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(OSD_STATUS_SHM_ADDR);

    if (view >= 2)
    {
        return 0;
    }
    
    if(g_stEncoderStatus[view].resW != resW ||
       g_stEncoderStatus[view].resH != resH ||
       g_stEncoderStatus[view].framerate != framerate)
    {
        if ((resW == 0) || (resH == 0) || (framerate == 0))
        {
            H264_Encoder_CloseView(view);
        }
        else
        {
            H264_Encoder_RestartView(view, resW, resH, g_stEncoderStatus[view].gop, 
                                     framerate, g_stEncoderStatus[view].bitrate);
        }
        
        dlog_info("Video format change: %d, %d, %d, %d\n", view, resW, resH, framerate);
    }

    osdptr->video_width[view] = resW;
    osdptr->video_height[view] = resH;
    osdptr->frameRate[view] = framerate;

    return 1;
}

static void H264_Encoder_InputVideoFormatChangeCallback(void* p)
{
    uint8_t index  = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->index;
    uint16_t width = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->width;
    uint16_t hight = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->hight;
    uint8_t framerate = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->framerate;

    // ADV7611 0,1 is connected to H264 encoder 1,0
    H264_Encoder_UpdateVideoInfo((index == 0) ? 1 : 0, width, hight, framerate);
}

int H264_Encoder_UpdateGop(unsigned char view, unsigned char gop)
{
    uint32_t addr;

    if(view == 0 )
    {
        addr = ENC_REG_ADDR + (0x02 << 2);
    }
    else
    {
        addr = ENC_REG_ADDR + (0x1b << 2);
    }

    Reg_Write32_Mask(addr, (unsigned int)(gop << 24), BIT(31)|BIT(30)|BIT(29)|BIT(28)|BIT(27)|BIT(26)|BIT(25)|BIT(24));
}


int H264_Encoder_UpdateIpRatio(unsigned char view, unsigned char ipratio)
{
    uint32_t addr;

    if(view == 0 )
    {
        addr = ENC_REG_ADDR + (0x08 << 2);
    }
    else
    {
        addr = ENC_REG_ADDR + (0x21 << 2);
    }
    
    Reg_Write32_Mask(addr, (unsigned int)(ipratio << 24), BIT(27)|BIT(26)|BIT(25)|BIT(24));
}


int H264_Encoder_UpdateBitrate(unsigned char view, unsigned char br_idx)
{
    uint8_t ratio = 0;

    if (view >= 2)
    {
        return 0;
    }

    if(br_idx == 1)       //600Kbps
    {
        ratio = 8;
    }
    else if(br_idx <= 3)  //1.2Mbps, 2.4Mbps
    {
        ratio = 7;
    }
    else if(br_idx <= 5)  //4.8Mbps
    {
        ratio = 6;
    }
    else                  //7.5Mbps, 10Mbps
    {
        ratio = 4;
    }
       
    if (g_stEncoderStatus[view].running && g_stEncoderStatus[view].brc_enable && (g_stEncoderStatus[view].bitrate != br_idx))
    {
        if (view == 0)
        {
            Reg_Write32_Mask(ENC_REG_ADDR+(0xA<<2), (unsigned int)(br_idx << 26), BIT(26) | BIT(27) | BIT(28) | BIT(29) | BIT(30));
            if(br_idx >= 1 && br_idx <= 5)
            {
                H264_Encoder_UpdateGop(view, 60);
            }
            else
            {
                H264_Encoder_UpdateGop(view, 30);
            }  

            H264_Encoder_UpdateIpRatio(view, ratio);
        }
        else
        {
            Reg_Write32_Mask(ENC_REG_ADDR+(0x23<<2), (unsigned int)(br_idx << 26), BIT(26) | BIT(27) | BIT(28) | BIT(29) | BIT(30));
            if(br_idx >= 1 && br_idx <= 5)
            {
                H264_Encoder_UpdateGop(view, 60);
            }
            else
            {
                H264_Encoder_UpdateGop(view, 30);
            }

            H264_Encoder_UpdateIpRatio(view, ratio);            
        }

        dlog_info("Encoder bitrate change: %d, %d %d\n", view, br_idx, ratio);
    }
    
    g_stEncoderStatus[view].bitrate = br_idx;
    return 1;
}

static void H264_Encoder_BBModulationChangeCallback(void* p)
{
    uint8_t br_idx = ((STRU_SysEvent_BB_ModulationChange *)p)->BB_MAX_support_br;
    uint8_t ch = ((STRU_SysEvent_BB_ModulationChange *)p)->u8_bbCh;
    
    /*dlog_info("H264 bitidx: %d \r\n", br_idx);
    H264_Encoder_UpdateBitrate(0, br_idx);
    H264_Encoder_UpdateBitrate(1, br_idx);*/
    if (0 == ch)
    {
        H264_Encoder_UpdateBitrate(0, br_idx);
        dlog_info("H264 bitidx ch1: %d \r\n", br_idx);
    }
    else if (1 == ch)
    {
        H264_Encoder_UpdateBitrate(1, br_idx);
        dlog_info("H264 bitidx ch2: %d \r\n", br_idx);
    }
    else
    {
    }

}

static void VEBRC_IRQ_Wrap_Handler(uint32_t u32_vectorNum)
{
    uint32_t view0_feedback = Reg_Read32(ENC_REG_ADDR+(0x09<<2));
    uint32_t view1_feedback = Reg_Read32(ENC_REG_ADDR+(0x22<<2));

    if( (g_stEncoderStatus[0].running == 1 && (view0_feedback & BIT(6))) || (g_stEncoderStatus[1].running == 1 && (view1_feedback & BIT(6))) )
    {
        // Encoder hang happens, then do reset.
        Reg_Write32(VSOC_SOFT_RESET, (~(BIT(3))));
        Reg_Write32(VSOC_SOFT_RESET, 0xFF);

        if (g_stEncoderStatus[0].running == 1)
        {
            H264_Encoder_RestartView(0, g_stEncoderStatus[0].resW, g_stEncoderStatus[0].resH, 
                                     g_stEncoderStatus[0].gop, g_stEncoderStatus[0].framerate, 
                                     g_stEncoderStatus[0].bitrate);
            dlog_info("Reset view0");
        }

        if (g_stEncoderStatus[1].running == 1)
        {
            H264_Encoder_RestartView(1, g_stEncoderStatus[1].resW, g_stEncoderStatus[1].resH, 
                                     g_stEncoderStatus[1].gop, g_stEncoderStatus[1].framerate, 
                                     g_stEncoderStatus[1].bitrate);
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
            VEBRC_IRQ_Handler(view0_feedback, view1_feedback);
        }
    }
}

int H264_Encoder_Init(uint8_t gop0, uint8_t br0, uint8_t brc0_e, uint8_t gop1, uint8_t br1, uint8_t brc1_e)
{
    // variable Declaraton 
    char spi_rd_dat, i2c_rd_dat;
    unsigned int wait_cnt, i;
    unsigned char read_cnt ;

    // Video_Soc Wait SDRAM INIT_DONE
    sdram_init_check(); 

    reg_IrqHandle(VIDEO_ARMCM7_IRQ_VECTOR_NUM, VEBRC_IRQ_Wrap_Handler, NULL);
	INTR_NVIC_SetIRQPriority(VIDEO_ARMCM7_IRQ_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_VIDEO_ARMCM7,0));
    INTR_NVIC_DisableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);

    g_stEncoderStatus[0].gop = gop0;
    g_stEncoderStatus[0].bitrate = br0;
    g_stEncoderStatus[0].brc_enable = brc0_e;
    v0_poweron_rc_params_set = 1;

    g_stEncoderStatus[1].gop = gop1;
    g_stEncoderStatus[1].bitrate = br1;
    g_stEncoderStatus[1].brc_enable = brc1_e;
    v1_poweron_rc_params_set = 1;

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE_LOCAL, H264_Encoder_InputVideoFormatChangeCallback);
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_SUPPORT_BR_CHANGE, H264_Encoder_BBModulationChangeCallback);

    dlog_info("h264 encoder init OK\n");

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

