#ifndef __ISR__H
#define __ISR__H

typedef enum
{
    SVC_VECTOR_NUM = 11,
    PENDSV_VECTOR_NUM = 14,
    SYSTICK_VECTOR_NUM = 15,
    UART0_VECTOR_NUM = 16,
    UART1_VECYOR_NUM = 17,
    USB_OTG0_VECTOR_NUM = 72,
    VEBRC_VECTOR_NUM = 77,
}IRQ_type;

typedef void(*Irq_handler)(void);

int reg_IrqHandle(IRQ_type vct, Irq_handler hdl);
int rmv_IrqHandle(IRQ_type vct);

#endif

