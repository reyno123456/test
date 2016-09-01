#include "interrupt.h"
#include "interrupt_internal.h"

void _start(void);
void default_isr(void);

unsigned int vectors[] __attribute__((section(".vectors"))) = {

	[0]  = (unsigned int)0x20010000,
	[1]  = (unsigned int)&_start,
	[2]  = (unsigned int)&default_isr,
	[3]  = (unsigned int)&default_isr,
	[4]  = (unsigned int)&default_isr,
	[5]  = (unsigned int)&default_isr,
	[6]  = (unsigned int)&default_isr,
	[7]  = (unsigned int)&default_isr,
	[8]  = (unsigned int)&default_isr,
	[9]  = (unsigned int)&default_isr,
	[10] = (unsigned int)&default_isr,
	[11] = (unsigned int)&SVC_IRQHandler,
	[12] = (unsigned int)&default_isr,
	[13] = (unsigned int)&default_isr,
	[14] = (unsigned int)&PendSV_IRQHandler, 
	[15] = (unsigned int)&SysTick_IRQHandler,
	[16] = (unsigned int)&UART0_IRQHandler,
	[17] = (unsigned int)&UART1_IRQHandler,
	[18 ... 165] = (unsigned int)&default_isr
};

extern int _rodata_end;
extern int _data_start;
extern int _data_end;
void __attribute__((section(".start"))) _start(void)
{
    int* src = &_rodata_end;
    int* dst = &_data_start;
    while (dst < &_data_end)
    {
        *dst++ = *src++;
    }

    main();
}

void default_isr(void)
{
	;
}

