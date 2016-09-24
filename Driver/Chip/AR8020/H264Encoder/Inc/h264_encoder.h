#ifndef H264_ENCODER_H
#define H264_ENCODER_H

int H264_Encoder_Init(void);
int H264_Encoder_UpdateVideoInfo(unsigned char view, unsigned int resW, unsigned int resH, unsigned int framerate);
int H264_Encoder_UpdateBitrate(unsigned char view, unsigned char br);
void H264_Encoder_DumpFrameCount(void);

#endif

