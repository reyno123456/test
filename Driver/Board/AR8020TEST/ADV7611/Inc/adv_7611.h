#ifndef ADV_7611_H
#define ADV_7611_H

void ADV_7611_WriteByte(uint8_t slv_addr, uint8_t sub_addr, uint8_t val);
uint8_t ADV_7611_ReadByte(uint8_t slv_addr, uint8_t sub_addr);
void ADV_7611_Initial(uint8_t index);
void ADV_7611_DumpOutEdidData(uint8_t index);
void ADV_7611_DumpOutDefaultSettings(uint8_t index);
void ADV_7611_GetVideoFormat(uint8_t index, uint32_t* widthPtr, uint32_t* hightPtr, uint32_t* framteratePtr);

#endif