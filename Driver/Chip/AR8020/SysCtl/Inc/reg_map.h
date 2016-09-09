#ifndef REG_MAP_H
#define REG_MAP_H

/*==========================================================*/
/*     ARM_M7 Interrupt Vector Enable  Declaration          */
/*==========================================================*/
#define NVIC_ISER_BASE 0xE000E100
typedef struct
{
   unsigned int ISER0; // 0x00
   unsigned int ISER1; // 0x04
   unsigned int ISER2; // 0x08
   unsigned int ISER3; // 0x0c
   unsigned int ISER4; // 0x10
   unsigned int ISER5; // 0x14
   unsigned int ISER6; // 0x18
   unsigned int ISER7; // 0x1c
} NVIC_ISER_type;
#define NVIC_ISER ((NVIC_ISER_type *) NVIC_ISER_BASE)

//Video SoC Global Reg1 ADDR
#define VSOC_GLOBAL1_BASE               0x40B00000
//Global_Reg1 Register Mapping
typedef struct _GPIO_CFG{
    unsigned int SFR_PAD_CTRL0;             // Address: 0x40b0007c
    unsigned int SFR_PAD_CTRL1;
    unsigned int SFR_PAD_CTRL2;
    unsigned int SFR_PAD_CTRL3;
    unsigned int SFR_PAD_CTRL4;
    unsigned int SFR_PAD_CTRL5;
    unsigned int SFR_PAD_CTRL6;
    unsigned int SFR_PAD_CTRL7;
    unsigned int SFR_PAD_CTRL8;
    unsigned int SFR_DEBUG0;
    unsigned int reserved[9];
    unsigned int SFR_SDRAM_CLK_SEL;
} GPIO_CFG, *GPIO_CFG_PTR;

#endif

