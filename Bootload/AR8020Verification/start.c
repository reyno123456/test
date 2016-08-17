#include "interrupt.h"

void _start(void);
void default_isr(void);

unsigned int vectors[] __attribute__((section(".vectors"))) = {

	[0] = (unsigned int)0x20010000,


	[1] = (unsigned int)&_start,

	[2 ... 10] = (unsigned int)&default_isr,
	[11] = (unsigned int)&SVC_Handler,
	[12 ... 15] = (unsigned int)&default_isr,
	[16] = (unsigned int)&UART0_IRQHandler,
	[17] = (unsigned int)&UART1_IRQHandler,
	[18 ... 165] = (unsigned int)&default_isr
};

void __attribute__((section(".start"))) _start(void)
{
	main();
}

void default_isr(void)
{
	;
}

