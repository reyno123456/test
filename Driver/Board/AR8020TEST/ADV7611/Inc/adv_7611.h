#ifndef ADV_7611_H
#define ADV_7611_H

void ADV_7611_Initial(uint8_t index);
void ADV_7611_DumpOutEdidData(uint8_t index);
void ADV_7611_DumpOutDefaultSettings(uint8_t index);
void ADV_7611_GetVideoFormat(uint8_t index, uint16_t* widthPtr, uint16_t* hightPtr, uint8_t* framteratePtr);

#endif

