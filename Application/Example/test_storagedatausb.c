#include "../cpu0/test_usbh.h"
#include "debuglog.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "systicks.h"
#include "hal_usb_host.h"
#include "minimp3.h"

void writeAudioPcm(uint8_t *id1)
{
    FRESULT    fileResult;
    FIL        MyFile;
    uint32_t byteswritten=0;
    uint8_t d1 = (uint8_t)(strtoul(id1, NULL, 0));
    dlog_info("start !!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    USBH_MountUSBDisk();
    while (HAL_USB_HOST_STATE_READY != HAL_USB_GetHostAppState())
    {
        dlog_info("finding mass storage\n");
        SysTicks_DelayMS(500);
    }


    if (1==d1)
    {
        fileResult = f_open(&MyFile,"audio.pcm" , FA_CREATE_ALWAYS | FA_WRITE);
        if (FR_OK != fileResult)
        {
            dlog_info("open or create file error: %d\n", fileResult);
            vTaskDelete(NULL);
        }
        fileResult = f_write(&MyFile, (uint8_t *)(0x81F00000), 1024*1024, (void *)&byteswritten);
        if((fileResult != FR_OK) || (byteswritten==0))
        {
            dlog_info("f_write error! \n");
            f_close(&MyFile);

        }
        f_close(&MyFile); 

        fileResult = f_open(&MyFile,"audio.mp3" , FA_CREATE_ALWAYS | FA_WRITE);
        if (FR_OK != fileResult)
        {
            dlog_info("open or create file error: %d\n", fileResult);

        }
        fileResult = f_write(&MyFile, (uint8_t *)(0x81E00000), 0xFE400, (void *)&byteswritten);
        if((fileResult != FR_OK) || (byteswritten==0))
        {
            dlog_info("f_write error! \n");
            f_close(&MyFile);

        }
        f_close(&MyFile);       
    }
    else
    {
        
        fileResult = f_open(&MyFile,"audio.mp3" , FA_CREATE_ALWAYS | FA_WRITE);
        if (FR_OK != fileResult)
        {
            dlog_info("open or create file error: %d\n", fileResult);

        }
        fileResult = f_write(&MyFile, (uint8_t *)(0x20060000), 0x20000, (void *)&byteswritten);
        if((fileResult != FR_OK) || (byteswritten==0))
        {
            dlog_info("f_write error! \n");
            f_close(&MyFile);

        }
        f_close(&MyFile);
    }

    dlog_info("end !!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
}

void test_StorageData(void)
{
    
    FRESULT    fileResult;
    FIL        MyFile;
    uint32_t byteswritten=0;
    char *test="asdfasdf";
    unsigned char *stream_pos = (unsigned char *) 0x21006000;
    signed short sample_buf[1152*2];
    int bytes_left=0x7C00;
    int frame_size;

    dlog_info("start !!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    USBH_MountUSBDisk();
    while (HAL_USB_HOST_STATE_READY != HAL_USB_GetHostAppState())
    {
        dlog_info("finding mass storage\n");
        SysTicks_DelayMS(500);
    }

    fileResult = f_open(&MyFile,"audiogrd.pcm" , FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fileResult)
    {
        dlog_info("open or create file error: %d\n", fileResult);
        
        //vTaskDelete(NULL);
    }
   /* fileResult = f_write(&MyFile, test, strlen(test), (void *)&byteswritten);
    if((fileResult != FR_OK) || (byteswritten==0))
    {
        dlog_info("f_write error! \n");
    }*/

    mp3_decoder_t mp3;
    mp3_info_t info;
    mp3 = mp3_create();

    uint32_t tick = SysTicks_GetTickCount();
    frame_size = mp3_decode(mp3, stream_pos, bytes_left, sample_buf, &info);
    if (!frame_size) {
        dlog_info("\nError: not a valid MP3 audio file!\n");
        return ;
    }
    dlog_info("info %d %d %d\n",info.audio_bytes,frame_size,SysTicks_GetTickCount()-tick);
    
/*
    fileResult = f_write(&MyFile, (const void *)sample_buf, info.audio_bytes, (void *)&byteswritten);
    if((fileResult != FR_OK) || (byteswritten==0))
    {
        dlog_info("f_write error! \n");
    }
    while ((bytes_left >= 0) && (frame_size > 0)) {
        stream_pos += frame_size;
        bytes_left -= frame_size;
        fileResult = f_write(&MyFile, sample_buf, info.audio_bytes, (void *)&byteswritten);
        if((fileResult != FR_OK) || (byteswritten==0))
        {
            dlog_info("f_write error! \n");
            f_close(&MyFile);

            break;
        }
        frame_size = mp3_decode(mp3, stream_pos, bytes_left, sample_buf, &info);
        dlog_info("info %d %d\n",info.audio_bytes,frame_size);
    }   */
    mp3_done(mp3);
    f_close(&MyFile);
    dlog_info("f_write %x !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",byteswritten);

}


#if 0
void test_StorageDataMp3(void)
{
    
    FRESULT    fileResult;
    FIL        MyFile;
    FIL        MyFileMp3;
    uint32_t byteswritten=0;
    char *test="asdfasdf";
    unsigned char *stream_pos = (unsigned char *) 0x21006000;
    signed short sample_buf[1152*2];
    int bytes_left=0x7C00;
    int frame_size;
    int frame_sizeMp3=192*3;
    unsigned char buffer[192*3] = {0};
    
    dlog_info("start !!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    USBH_MountUSBDisk();
    while (HAL_USB_HOST_STATE_READY != HAL_USB_GetHostAppState())
    {
        dlog_info("finding mass storage\n");
        SysTicks_DelayMS(500);
    }

    fileResult = f_open(&MyFileMp3,"test.mp3" , FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fileResult)
    {
        dlog_info("open or create file error: %d\n", fileResult);
        
    }

    fileResult = f_open(&MyFile,"audiogrd.pcm" , FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fileResult)
    {
        dlog_info("open or create file error: %d\n", fileResult);

    }

    if(f_read(&MyFileMp3, buffer, frame_sizeMp3, (UINT*)&samples_read) != FR_OK)
    {
        dlog_info("read samples error\n");
        return 0;
    }

    mp3_decoder_t mp3;
    mp3_info_t info;
    mp3 = mp3_create();
    frame_size = mp3_decode(mp3, buffer, bytes_left, sample_buf, &info);
    if (!frame_size) {
        dlog_info("\nError: not a valid MP3 audio file!\n");
        return ;
    }

    dlog_info("info %d %d\n",info.audio_bytes,frame_size);

    while ((bytes_left >= 0) && (frame_size > 0)) {
        stream_pos += frame_size;
        bytes_left -= frame_size;
        fileResult = f_write(&MyFile, sample_buf, info.audio_bytes, (void *)&byteswritten);
        if((fileResult != FR_OK) || (byteswritten==0))
        {
            dlog_info("f_write error! \n");
            f_close(&MyFile);

            break;
        }
        if(f_read(&MyFileMp3, buffer, frame_sizeMp3, (UINT*)&samples_read) != FR_OK)
        {
            dlog_info("read samples error\n");
            return 0;
        }
        frame_size = mp3_decode(mp3, buffer, bytes_left, sample_buf, &info);
        dlog_info("info %d %d\n",info.audio_bytes,frame_size);
    }   

    mp3_done(mp3);
    f_close(&MyFile);

    dlog_info("f_write %x !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",byteswritten);

}
#endif