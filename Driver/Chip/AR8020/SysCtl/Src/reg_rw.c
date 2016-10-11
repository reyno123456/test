#include "stddef.h"
#include "stdint.h"
#include "reg_rw.h"

uint32_t Reg_Read32(uint32_t regAddr)
{
    volatile uint32_t* ptr_regAddr = (uint32_t*)regAddr;
    return *ptr_regAddr;
}

void Reg_Write32(uint32_t regAddr, uint32_t regData)
{
    volatile uint32_t* ptr_regAddr = (uint32_t*)regAddr;
    *ptr_regAddr = regData;
}

void Reg_Write32_Mask(uint32_t regAddr, uint32_t regData, uint32_t regDataMask)
{
    uint32_t u32_regDataTmp;
    volatile uint32_t* ptr_regAddr = (uint32_t*)regAddr;
    uint32_t u32_regDataWithMask = regData & regDataMask;

    u32_regDataTmp = *ptr_regAddr;
    u32_regDataTmp &= ~regDataMask;
    u32_regDataTmp |= u32_regDataWithMask;
     
    *ptr_regAddr = u32_regDataTmp;
}

void write_reg32(uint32_t *addr, uint32_t data)
{
    volatile uint32_t *reg_addr = (uint32_t *)addr;
    *reg_addr = data;
}

uint32_t read_reg32(uint32_t *addr)
{
    volatile uint32_t *reg_addr = (uint32_t *)addr;
    return (*reg_addr);
}


