#include "test_h264_encoder.h"
#include "h264_encoder.h"

void command_encoder_dump_brc(void)
{
    H264_Encoder_DumpFrameCount();
}

void command_encoder_update_brc(unsigned char br)
{
    H264_Encoder_UpdateBitrate(0, br);
}

void command_encoder_update_video(unsigned int width, unsigned int hight, unsigned int framerate)
{
    //H264_Encoder_UpdateVideoInfo(0, width, hight, framerate);
}
