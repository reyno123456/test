#ifndef ADV_7611_H
#define ADV_7611_H

void ADV_7611_Initial(void);
void ADV_7611_DumpOutEdidData(void);
void ADV_7611_DumpOutDefaultSettings(void);
void ADV_7611_GetVideoFormat(uint32_t* widthPtr, uint32_t* hightPtr, uint32_t* framteratePtr);

#endif