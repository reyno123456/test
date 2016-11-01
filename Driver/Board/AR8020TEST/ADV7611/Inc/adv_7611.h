#ifndef ADV_7611_H
#define ADV_7611_H

#define ADV7611_0_DEVICE_ENABLE_MASK (1<<0)
#define ADV7611_1_DEVICE_ENABLE_MASK (1<<1)

typedef struct _ADV7611Format
{
    uint16_t width;
    uint16_t hight;
    uint8_t  framerate;
} STRU_ADV7611Format;

typedef struct _ADV7611Status
{
    uint8_t device_mask;
    STRU_ADV7611Format video_format[2];
} STRU_ADV7611Status;

void ADV_7611_WriteByte(uint8_t slv_addr, uint8_t sub_addr, uint8_t val);
uint8_t ADV_7611_ReadByte(uint8_t slv_addr, uint8_t sub_addr);
void ADV_7611_Initial(uint8_t index);
void ADV_7611_DumpOutEdidData(uint8_t index);
void ADV_7611_DumpOutDefaultSettings(uint8_t index);
void ADV_7611_GetVideoFormat(uint8_t index, uint32_t* widthPtr, uint32_t* hightPtr, uint32_t* framteratePtr);

#endif