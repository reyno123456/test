#include "test_sram.h"
#include "interrupt.h"
#include "sram.h"

void test_sram_init(void)
{
    /* register the irq handler */
    reg_IrqHandle(BB_SRAM_READY_IRQ_0_VECTOR_NUM, SRAM_Ready0IRQHandler);

    /* enable the SRAM_READY_0 IRQ */
    INTR_NVIC_EnableIRQ(BB_SRAM_READY_IRQ_0_VECTOR_NUM);

    /* register the irq handler */
    reg_IrqHandle(BB_SRAM_READY_IRQ_1_VECTOR_NUM, SRAM_Ready1IRQHandler);

    /* enable the SRAM_READY_1 IRQ */
    INTR_NVIC_EnableIRQ(BB_SRAM_READY_IRQ_1_VECTOR_NUM);
}


void test_sram_task(void)
{
    
}


