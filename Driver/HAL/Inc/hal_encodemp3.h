#ifndef __HAL_MP3_ENCODER__H__
#define __HAL_MP3_ENCODER__H__


#define MPE3_ENCODER_DATA_ADDR        (0x81E00000)
#define AUDIO_BYPASS_START            (0xB1800000)
typedef enum
{
    HAL_MP3_ENCODE_STEREO = 0,
    HAL_MP3_ENCODE_JOINT_STEREO,
    HAL_MP3_ENCODE_DUAL_CHANNEL,
    HAL_MP3_ENCODE_MONO
} ENUM_HAL_MP3_ENCODE_MODE;

typedef enum
{
    HAL_MP3_ENCODE_44100 = 44100,
    HAL_MP3_ENCODE_48000 = 48000
} ENUM_HAL_MP3_ENCODE_SAMPLERATR;


typedef struct
{
    uint32_t u32_rawDataAddr;
    uint32_t u32_rawDataLenght;
    uint32_t u32_encodeDataAddr;
    uint32_t u32_newPcmDataFlagAddr;
    ENUM_HAL_MP3_ENCODE_SAMPLERATR e_samplerate;
    ENUM_HAL_MP3_ENCODE_MODE e_modes;
    uint8_t u8_channel;
    
} STRU_MP3_ENCODE_CONFIGURE_WAVE;
/**
* @brief    mp3 encoder initialization
* @param    STRU_MP3_ENCODE_CONFIGURE_WAVE: I2S initialization.
* @retval   HAL_TRUE    means the initialization is well done.
* @retval   HAL_FALSE   paramenter error.
* @note     none
*/
HAL_BOOL_T HAL_MP3EncodePcmInit(const STRU_MP3_ENCODE_CONFIGURE_WAVE *st_mp3EncodeConfg);

/**
* @brief    mp3 encoder
* @param    none.
* @retval   none.
* @note     none
*/
void HAL_MP3EncodePcm(void);


#endif /* __HAL_MP3_ENCODER__H__ */
