#ifndef __ISR__H
#define __ISR__H

#define SVC_VECTOR_NUM 		(11)
#define	PENDSV_VECYOR_NUM 	(14)
#define UART0_VECYOR_NUM	(16)
#define UART1_VECYOR_NUM	(17)

int reg_IrqHandle(int num, unsigned int hdl);
int rmv_IrqHandle(int num);

#endif

