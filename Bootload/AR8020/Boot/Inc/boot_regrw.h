#ifndef BOOT_REGRW_H
#define BOOT_REGRW_H

uint32_t Reg_Read32(uint32_t regAddr);


void Reg_Write32(uint32_t regAddr, uint32_t regData);

void Reg_Write32_Mask(uint32_t regAddr, uint32_t regData, uint32_t regDataMask);
#endif
