
#include <stdint.h>
#include "mpu.h"
#include "memory_config.h"

static void MPU_ControlEnable(void)
{
    MPU->CTRL = MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk | MPU_CTRL_ENABLE_Msk;  
    __asm volatile ("dsb 0xF":::"memory");
    __asm volatile ("isb 0xF":::"memory");
    return;
}

static void MPU_ControlDisable(void)
{    
    __asm volatile ("dmb 0xF":::"memory");
    MPU->CTRL = 0;
    return;
}

static void MPU_RegionConfigDisable(uint8_t u8_regionNum)
{    

    MPU->RNR  = u8_regionNum;
    MPU->RBAR = 0;
    MPU->RASR = 0;
}

static void MPU_RegionConfigEnable(uint8_t u8_regionNum, uint32_t u32_addr, uint32_t u32_attributes)
{    

    MPU->RNR  = u8_regionNum;
    MPU->RBAR = u32_addr | (1 << 4) | (u8_regionNum << 0);
    MPU->RASR = u32_attributes;
}

static void MPU_SRAMRegionConfigEnable(void)
{
    MPU_RegionConfigEnable(SRAM_MEMORY_MPU_REGION_NUMBER, SRAM_MEMORY_MPU_REGION_ST_ADDR_0, SRAM_MEMORY_MPU_REGION_ATTR_0);
    MPU_RegionConfigEnable(SRAM_CONFIGURE_MEMORY_MPU_REGION_NUMBER, SRAM_CONFIGURE_MEMORY_MPU_REGION_ST_ADDR_1, SRAM_CONFIGURE_MEMORY_MPU_REGION_ATTR_1);
    MPU_RegionConfigEnable(SRAM_DEBUG_MEMORY_MPU_REGION_NUMBER, SRAM_DEBUG_MEMORY_MPU_REGION_ST_ADDR_2, SRAM_DEBUG_MEMORY_MPU_REGION_ATTR_2);
    MPU_RegionConfigEnable(SRAM_AV_MPU_REGION_NUMBER, SRAM_AV_MPU_REGION_ST_ADDR_4, SRAM_AV_MPU_REGION_ATTR_4);
}


int32_t MPU_SetUp(void)
{
    if (0 == MPU->TYPE)
    {
        return 1;
    }

    MPU_ControlDisable();
    MPU_SRAMRegionConfigEnable();
    MPU_RegionConfigEnable(QUAD_SPI_MPU_REGION_NUMBER, QUAD_SPI_MPU_REGION_ST_ADDR_3, QUAD_SPI_MPU_REGION_ATTR_3);
    MPU_RegionConfigEnable(SRAM_AV_MPU_REGION_NUMBER, SRAM_AV_MPU_REGION_ST_ADDR_4, SRAM_AV_MPU_REGION_ATTR_4);
    
    
    MPU_RegionConfigDisable(5);
    MPU_RegionConfigDisable(6);
    MPU_RegionConfigDisable(7);

    MPU_ControlEnable();
}

int32_t MPU_QuadspiProtectEnable(void)
{

    if (0 == MPU->TYPE)
    {
        return 1;
    }

    MPU_ControlDisable();
    MPU_SRAMRegionConfigEnable();
    MPU_RegionConfigEnable(QUAD_SPI_MPU_REGION_NUMBER, QUAD_SPI_MPU_REGION_ST_ADDR_3, QUAD_SPI_MPU_REGION_ATTR_3);
    MPU_RegionConfigEnable(SRAM_AV_MPU_REGION_NUMBER, SRAM_AV_MPU_REGION_ST_ADDR_4, SRAM_AV_MPU_REGION_ATTR_4);
    MPU_RegionConfigDisable(5);
    MPU_RegionConfigDisable(6);
    MPU_RegionConfigDisable(7);

    MPU_ControlEnable();

}

int32_t MPU_QuadspiProtectDisable(void)
{

    if (0 == MPU->TYPE)
    {
        return 1;
    }

    MPU_ControlDisable();

    MPU_SRAMRegionConfigEnable();

    MPU_RegionConfigDisable(QUAD_SPI_MPU_REGION_NUMBER);
    MPU_RegionConfigEnable(SRAM_AV_MPU_REGION_NUMBER, SRAM_AV_MPU_REGION_ST_ADDR_4, SRAM_AV_MPU_REGION_ATTR_4);
    MPU_RegionConfigDisable(5);
    MPU_RegionConfigDisable(6);
    MPU_RegionConfigDisable(7);

    MPU_ControlEnable();

}
