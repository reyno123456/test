#include "interrupt.h"
#include "debuglog.h"

void _start(void);
void default_isr(void);
void hardfault_isr(void);
void SVC_Handler(void);
void PendSV_Handler(void);

unsigned int vectors[] __attribute__((section(".vectors"))) = {

    [0]   = (unsigned int)0x20010000,
    [1]   = (unsigned int)&_start,
    [2]   = (unsigned int)&default_isr,
    [3]   = (unsigned int)&hardfault_isr,
    [4]   = (unsigned int)&default_isr,
    [5]   = (unsigned int)&default_isr,
    [6]   = (unsigned int)&default_isr,
    [7]   = (unsigned int)&default_isr,
    [8]   = (unsigned int)&default_isr,
    [9]   = (unsigned int)&default_isr,
    [10]  = (unsigned int)&default_isr,
    [11]  = (unsigned int)&SVC_Handler,
    [12]  = (unsigned int)&default_isr,
    [13]  = (unsigned int)&default_isr,
    [14]  = (unsigned int)&PendSV_Handler, 
    [15]  = (unsigned int)&SYSTICK_IRQHandler,
    /* External IRQ */
    [16]  = (unsigned int)&IRQHandler_16,
    [17]  = (unsigned int)&IRQHandler_17,
    [18]  = (unsigned int)&IRQHandler_18,
    [19]  = (unsigned int)&IRQHandler_19,
    [20]  = (unsigned int)&IRQHandler_20,
    [21]  = (unsigned int)&IRQHandler_21,
    [22]  = (unsigned int)&IRQHandler_22,
    [23]  = (unsigned int)&IRQHandler_23,
    [24]  = (unsigned int)&IRQHandler_24,
    [25]  = (unsigned int)&IRQHandler_25,
    [26]  = (unsigned int)&IRQHandler_26,
    [27]  = (unsigned int)&IRQHandler_27,
    [28]  = (unsigned int)&IRQHandler_28,
    [29]  = (unsigned int)&IRQHandler_29,
    [30]  = (unsigned int)&IRQHandler_30,
    [31]  = (unsigned int)&IRQHandler_31,
    [32]  = (unsigned int)&IRQHandler_32,
    [33]  = (unsigned int)&IRQHandler_33,
    [34]  = (unsigned int)&IRQHandler_34,
    [35]  = (unsigned int)&IRQHandler_35,
    [36]  = (unsigned int)&IRQHandler_36,
    [37]  = (unsigned int)&IRQHandler_37,
    [38]  = (unsigned int)&IRQHandler_38,
    [39]  = (unsigned int)&IRQHandler_39,
    [40]  = (unsigned int)&IRQHandler_40,
    [41]  = (unsigned int)&IRQHandler_41,
    [42]  = (unsigned int)&IRQHandler_42,
    [43]  = (unsigned int)&IRQHandler_43,
    [44]  = (unsigned int)&IRQHandler_44,
    [45]  = (unsigned int)&IRQHandler_45,
    [46]  = (unsigned int)&IRQHandler_46,
    [47]  = (unsigned int)&IRQHandler_47,
    [48]  = (unsigned int)&IRQHandler_48,
    [49]  = (unsigned int)&IRQHandler_49,
    [50]  = (unsigned int)&IRQHandler_50,
    [51]  = (unsigned int)&IRQHandler_51,
    [52]  = (unsigned int)&IRQHandler_52,
    [53]  = (unsigned int)&IRQHandler_53,
    [54]  = (unsigned int)&IRQHandler_54,
    [55]  = (unsigned int)&IRQHandler_55,
    [56]  = (unsigned int)&IRQHandler_56,
    [57]  = (unsigned int)&IRQHandler_57,
    [58]  = (unsigned int)&IRQHandler_58,
    [59]  = (unsigned int)&IRQHandler_59,
    [60]  = (unsigned int)&IRQHandler_60,
    [61]  = (unsigned int)&IRQHandler_61,
    [62]  = (unsigned int)&IRQHandler_62,
    [63]  = (unsigned int)&IRQHandler_63,
    [64]  = (unsigned int)&IRQHandler_64,
    [65]  = (unsigned int)&IRQHandler_65,
    [66]  = (unsigned int)&IRQHandler_66,
    [67]  = (unsigned int)&IRQHandler_67,
    [68]  = (unsigned int)&IRQHandler_68,
    [69]  = (unsigned int)&IRQHandler_69,
    [70]  = (unsigned int)&IRQHandler_70,
    [71]  = (unsigned int)&IRQHandler_71,
    [72]  = (unsigned int)&IRQHandler_72,
    [73]  = (unsigned int)&IRQHandler_73,
    [74]  = (unsigned int)&IRQHandler_74,
    [75]  = (unsigned int)&IRQHandler_75,
    [76]  = (unsigned int)&IRQHandler_76,
    [77]  = (unsigned int)&IRQHandler_77,
    [78]  = (unsigned int)&IRQHandler_78,
    [79]  = (unsigned int)&IRQHandler_79,
    [80]  = (unsigned int)&IRQHandler_80,
    [81]  = (unsigned int)&IRQHandler_81,
    [82]  = (unsigned int)&IRQHandler_82,
    [83]  = (unsigned int)&IRQHandler_83,
    [84]  = (unsigned int)&IRQHandler_84,
    [85]  = (unsigned int)&IRQHandler_85,
    [86]  = (unsigned int)&IRQHandler_86,
    [87]  = (unsigned int)&IRQHandler_87,
    [88]  = (unsigned int)&IRQHandler_88,
    [89]  = (unsigned int)&IRQHandler_89,
    [90]  = (unsigned int)&IRQHandler_90,
    [91]  = (unsigned int)&IRQHandler_91,
    [92]  = (unsigned int)&IRQHandler_92,
    [93]  = (unsigned int)&IRQHandler_93,
    [94]  = (unsigned int)&IRQHandler_94,
    [95]  = (unsigned int)&IRQHandler_95,
    [96]  = (unsigned int)&IRQHandler_96,
    [97]  = (unsigned int)&IRQHandler_97,
    [98]  = (unsigned int)&IRQHandler_98,
    [99 ... 165] = (unsigned int)&default_isr
};

extern int _rodata_end;
extern int _data_start;
extern int _data_end;
extern int _bss_start;
extern int _bss_end;

void hardfault_isr(void) 
{
    dlog_info("hard fault\n");
}

void __attribute__((section(".start"))) _start(void)
{
    int* src = &_rodata_end;
    int* dst = &_data_start;
    while (dst < &_data_end)
    {
        *dst++ = *src++;
    }
    dst = &_bss_start;
    while (dst < &_bss_end)
    {
        *dst++ = 0;
    }

    main();
}


void default_isr(void)
{
    ;
}
