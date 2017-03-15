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

#define H264_ENCODER_BUFFER_HIGH_LEVEL    (1<<19)
#define H264_ENCODER_BUFFER_LOW_LEVEL     (1<<17)

static STRU_EncoderStatus g_stEncoderStatus[2] = { 0 };

static int H264_Encoder_UpdateGop(unsigned char view, unsigned char gop);

static int H264_Encoder_StartView(unsigned char view, unsigned int resW, unsigned int resH, unsigned int gop, unsigned int framerate, unsigned int bitrate, ENUM_ENCODER_INPUT_SRC src)
{
    if (view >= 2)
    {
        return 0;
    }

    INTR_NVIC_DisableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);

    if (view == 0)
    {
        init_view0(resW, resH, gop, framerate, bitrate, src);
        my_v0_initial_all( );
        open_view0(g_stEncoderStatus[view].brc_enable);
    }
    else if (view == 1)
    {
        init_view1(resW, resH, gop, framerate, bitrate, src);
        my_v1_initial_all( );
        open_view1(g_stEncoderStatus[view].brc_enable);
    }

    g_stEncoderStatus[view].resW = resW;
    g_stEncoderStatus[view].resH = resH;
    g_stEncoderStatus[view].framerate = framerate;
    g_stEncoderStatus[view].running = 1;

    H264_Encoder_UpdateBitrate(view, bitrate);
    H264_Encoder_UpdateGop(view, framerate);
        
    if ((g_stEncoderStatus[0].brc_enable && g_stEncoderStatus[0].running) || 
        (g_stEncoderStatus[1].brc_enable && g_stEncoderStatus[1].running)) // view0 and view1 share the same BRC interrupt
    {
        INTR_NVIC_EnableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);
    }

    return 1;
}

static int H264_Encoder_RestartView(unsigned char view, unsigned int resW, unsigned int resH, unsigned int gop, unsigned int framerate, unsigned int bitrate, ENUM_ENCODER_INPUT_SRC src)
{
    if (view >= 2)
    {
        return 0;
    }
    
    INTR_NVIC_DisableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);

    if (view == 0)
    {
        close_view0();
        H264_Encoder_StartView(view, resW, resH, gop, framerate, bitrate, src);
    }
    else if (view == 1)
    {
        close_view1();
        H264_Encoder_StartView(view, resW, resH, gop, framerate, bitrate, src);
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

static int H264_Encoder_UpdateVideoInfo(unsigned char view, unsigned int resW, unsigned int resH, unsigned int framerate, ENUM_ENCODER_INPUT_SRC src)
{
    unsigned int u32_data; 
    unsigned int tmp_resW;
    unsigned int tmp_resH;
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(OSD_STATUS_SHM_ADDR);

    if (view >= 2)
    {
        return 0;
    }
    
    if(g_stEncoderStatus[view].resW != resW ||
       g_stEncoderStatus[view].resH != resH ||
       g_stEncoderStatus[view].framerate != framerate || 
       g_stEncoderStatus[view].src != src)
    {
        if ((resW == 0) || (resH == 0) || (framerate == 0) || (src == 0))
        {
            H264_Encoder_CloseView(view);
        }
        else
        {
            if (g_stEncoderStatus[view].over_flow == 0)
            {
            H264_Encoder_RestartView(view, resW, resH, g_stEncoderStatus[view].gop, 
                                     framerate, g_stEncoderStatus[view].bitrate, src);
			}
            else
            {
                g_stEncoderStatus[view].resW = resW;
                g_stEncoderStatus[view].resH = resH;
                g_stEncoderStatus[view].framerate = framerate;
            }
        }
        
        dlog_info("Video format change: %d, %d, %d, %d\n", view, resW, resH, framerate);
    }

    osdptr->video_width[view] = resW;
    osdptr->video_height[view] = resH;
    osdptr->frameRate[view] = framerate;
    
    READ_WORD((ENC_REG_ADDR+(0x01<<2)), u32_data);
    tmp_resW = (u32_data >> 16) & 0xFFFF;
    tmp_resH = (u32_data >> 0) & 0xFFFF;
    if ((tmp_resW == (g_stEncoderStatus[0].resW)) && (tmp_resH == (g_stEncoderStatus[0].resH)))
    {
        osdptr->encoder_status |= 0x01;
    }
    else
    {
        osdptr->encoder_status &= ~0x01;
    }
    
    READ_WORD((ENC_REG_ADDR+(0x1a<<2)), u32_data);
    tmp_resW = (u32_data >> 16) & 0xFFFF;
    tmp_resH = (u32_data >> 0) & 0xFFFF;
    if ((tmp_resW == (g_stEncoderStatus[1].resW)) && (tmp_resH == (g_stEncoderStatus[1].resH)))
    {
        osdptr->encoder_status |= 0x02;
    }
    else
    {
        osdptr->encoder_status &= ~0x02;
    }

    return 1;
}

static uint32_t H264_Encoder_GetBufferLevel(unsigned char view)
{
    uint32_t buf_level = 0;

    if (view == 0)
    {
        //read buffer counter
        Reg_Write32_Mask(ENC_REG_ADDR + 0xDC, (unsigned int)(0x21 << 24), BIT(31)|BIT(30)|BIT(29)|BIT(28)|BIT(27)|BIT(26)|BIT(25)|BIT(24));
        Reg_Write32_Mask(ENC_REG_ADDR + 0xD8, (unsigned int)(0x04 <<  8), BIT(11)|BIT(10)|BIT(9)|BIT(8));       // Switch to vdb debug register
        
        buf_level = Reg_Read32(ENC_REG_ADDR + 0xF8);
        Reg_Write32_Mask(ENC_REG_ADDR + 0xDC, (unsigned int)(0x00 << 24), BIT(31)|BIT(30)|BIT(29)|BIT(28)|BIT(27)|BIT(26)|BIT(25)|BIT(24));
        //dlog_info("F: %d, L: %d.", rca.v0_frame_cnt, buf_level );
    }

    return buf_level;
}

static void H264_Encoder_IdleCallback(void* p)
{
    if ((g_stEncoderStatus[0].over_flow == 1) && (g_stEncoderStatus[0].running == 0))
    {
        uint32_t buf_level = H264_Encoder_GetBufferLevel(0);
        
        if (buf_level <= H264_ENCODER_BUFFER_LOW_LEVEL)
        {
            Reg_Write32( 0xa003008c, 0x0);
            H264_Encoder_StartView(0, g_stEncoderStatus[0].resW, g_stEncoderStatus[0].resH, 
                                      g_stEncoderStatus[0].gop, g_stEncoderStatus[0].framerate, 
                                      g_stEncoderStatus[0].bitrate,
                                      g_stEncoderStatus[0].src
                                      );

            g_stEncoderStatus[0].over_flow = 0;
            
            dlog_info("Buffer level %d, open view 0.", buf_level);
        }
    }
}
static void H264_Encoder_InputVideoFormatChangeCallback(void* p)
{
    uint8_t index  = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->index;
    uint16_t width = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->width;
    uint16_t hight = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->hight;
    uint8_t framerate = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->framerate;
    ENUM_ENCODER_INPUT_SRC src = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->e_h264InputSrc;

    // ADV7611 0,1 is connected to H264 encoder 1,0
    H264_Encoder_UpdateVideoInfo(index, width, hight, framerate, src);
}

static int H264_Encoder_UpdateGop(unsigned char view, unsigned char gop)
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

static int H264_Encoder_UpdateMinQP(unsigned char view, unsigned char br)
{
    unsigned int addr, addr_wireless_screen, minqp, maxqp;
    switch( br ) {
        case 0 : minqp = 10 ; break; // 8Mbps
        case 1 : minqp = 19 ; break; // 600kbps
        case 2 : minqp = 18 ; break; // 1.2Mbps
        case 3 : minqp = 17 ; break; // 2.4Mbps
        case 4 : minqp = 16 ; break; // 3Mbps
        case 5 : minqp = 15 ; break; // 3.5Mbps
        case 6 : minqp = 15 ; break; // 4Mbps
        case 7 : minqp = 14 ; break; // 4.8Mbps
        case 8 : minqp = 14 ; break; // 5Mbps
        case 9 : minqp = 12 ; break; // 6Mbps
        case 10: minqp = 12 ; break; // 7Mbps
        case 11: minqp = 11 ; break; // 7.5Mbps
        case 12: minqp = 10 ; break; // 9Mbps
        case 13: minqp = 9  ; break; // 10Mbps
        default: minqp = 16 ; break; // 3Mbps
    }
    if(view==0) {
    	addr = ENC_REG_ADDR+(0x06<<2);
    	addr_wireless_screen = ENC_REG_ADDR+(0x18<<2);
    } else {
        addr = ENC_REG_ADDR+(0x1F<<2);
    	addr_wireless_screen = ENC_REG_ADDR+(0x31<<2);
    }
    Reg_Write32_Mask(addr, (unsigned int)(minqp<<8), BIT(15)|BIT(14)|BIT(13)|BIT(12)|BIT(11)|BIT(10)|BIT(9)|BIT(8)); // set minqp, lhu
    // set maxqp when wireless_screen application, lhu
    if (((Reg_Read32(addr_wireless_screen))>>2)&0x1) {
    	maxqp = minqp + 30;
        Reg_Write32_Mask(addr, (unsigned int)(maxqp<<8), BIT(23)|BIT(22)|BIT(21)|BIT(20)|BIT(19)|BIT(18)|BIT(17)|BIT(16));
    }
}

int H264_Encoder_UpdateBitrate(unsigned char view, unsigned char br_idx)
{
    uint8_t ratio = 0;

    if (view >= 2)
    {
        return 0;
    }

    switch( br_idx ) {
        case 0 : ratio = 4  ; break; // 8Mbps
        case 1 : ratio = 12 ; break; // 600kbps
        case 2 : ratio = 11 ; break; // 1.2Mbps
        case 3 : ratio = 9 ; break;  // 2.4Mbps
        case 4 : ratio = 8 ; break;  // 3Mbps
        case 5 : ratio = 7 ; break;  // 3.5Mbps
        case 6 : ratio = 7 ; break;  // 4Mbps
        case 7 : ratio = 6 ; break;  // 4.8Mbps
        case 8 : ratio = 6 ; break;  // 5Mbps
        case 9 : ratio = 6 ; break;  // 6Mbps
        case 10: ratio = 5 ; break;  // 7Mbps
        case 11: ratio = 5 ; break;  // 7.5Mbps
        case 12: ratio = 4 ; break;  // 9Mbps
        case 13: ratio = 4;  break;  // 10Mbps
        default: ratio = 8;  break;  //3Mbps
    }

    dlog_info("%d %d %d %d %d %d\n", g_stEncoderStatus[0].running, g_stEncoderStatus[1].running, 
                                     g_stEncoderStatus[0].brc_enable, g_stEncoderStatus[1].brc_enable, 
                                     g_stEncoderStatus[0].bitrate, g_stEncoderStatus[1].bitrate);
    if (g_stEncoderStatus[view].running && g_stEncoderStatus[view].brc_enable /*&& (g_stEncoderStatus[view].bitrate != br_idx)*/)
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
	g_stEncoderStatus[0].bu_cnt ++;

    if(g_stEncoderStatus[0].running == 1)  // View0 is opened
    {
        // check if this is the last row
        uint32_t v0_last_row = ((view0_feedback >> 8) & 0x01);

        if(v0_last_row)
        {
            //dlog_info("%d,%d\n", rca.v0_frame_cnt, rca.v0_intra_period );
            uint32_t buf_level = H264_Encoder_GetBufferLevel(0);
            if(buf_level >= H264_ENCODER_BUFFER_HIGH_LEVEL)
            {
                //Close Encoder
                Reg_Write32( (unsigned int) 0xa003008c, 0x01);
                g_stEncoderStatus[0].over_flow = 1;
                close_view0();
                g_stEncoderStatus[0].running = 0;
                //dlog_info("BL %d, close 0. %d", buf_level, g_stEncoderStatus[0].bu_cnt);
                //while( 1 )
                //{
                //    buf_level = H264_Encoder_GetBufferLevel( 0 );
                //    if ( buf_level <= H264_ENCODER_BUFFER_LOW_LEVEL )
                //    {
                //        Reg_Write32( (unsigned int) 0xa003008c, 0x00);
                //        H264_Encoder_StartView(0, g_stEncoderStatus[0].resW, g_stEncoderStatus[0].resH, 
                //                                  g_stEncoderStatus[0].gop, g_stEncoderStatus[0].framerate, 
                //                                  g_stEncoderStatus[0].bitrate);

                //        g_stEncoderStatus[0].over_flow = 0;
                //        
                //        dlog_info("Buffer level %d, open view 0.", buf_level);
                //        break;
                //    }
                //}
            }
			g_stEncoderStatus[0].bu_cnt = 0;
        }
    }


    if(g_stEncoderStatus[1].running == 1)  // View0 is opened
    {
        // check if this is the last row
        uint32_t v1_last_row = ((view1_feedback >> 8) & 0x01);

        if(v1_last_row)
        {
            //dlog_info("%d %d\n", rca.v1_frame_cnt,rca.v1_intra_period );
            uint32_t buf_level = H264_Encoder_GetBufferLevel(1);
            if(buf_level >= H264_ENCODER_BUFFER_HIGH_LEVEL)
            {
                //Close Encoder
                g_stEncoderStatus[1].over_flow = 1;
                close_view1();
                g_stEncoderStatus[1].running = 0;
                dlog_info("Buffer level %d, close view 1.", buf_level);
            }
        }
    }

    if( (g_stEncoderStatus[0].running == 1 && (view0_feedback & BIT(6))) || (g_stEncoderStatus[1].running == 1 && (view1_feedback & BIT(6))) )
    {
        // Encoder hang happens, then do reset.
        Reg_Write32(VSOC_SOFT_RESET, (~(BIT(3))));
        Reg_Write32(VSOC_SOFT_RESET, 0xFF);

        if (g_stEncoderStatus[0].running == 1)
        {
            H264_Encoder_RestartView(0, g_stEncoderStatus[0].resW, g_stEncoderStatus[0].resH, 
                                     g_stEncoderStatus[0].gop, g_stEncoderStatus[0].framerate, 
                                     g_stEncoderStatus[0].bitrate, g_stEncoderStatus[0].src);
            dlog_info("Reset view0");
        }

        if (g_stEncoderStatus[1].running == 1)
        {
            H264_Encoder_RestartView(1, g_stEncoderStatus[1].resW, g_stEncoderStatus[1].resH, 
                                     g_stEncoderStatus[1].gop, g_stEncoderStatus[1].framerate, 
                                     g_stEncoderStatus[1].bitrate, g_stEncoderStatus[1].src);
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

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_IDLE, H264_Encoder_IdleCallback);
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE, H264_Encoder_InputVideoFormatChangeCallback);
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

