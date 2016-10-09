/**
  ******************************************************************************
  * @file      start.s
  * @author    Min Zhao
  * @Version    V1.1.0
  * @Date       14-September-2016
  ******************************************************************************
  */
#include "debuglog.h"
#include "interrupt.h"
#include <serial.h>

.equ  ITCM0,              0x00000000
.equ  ITCM1,              0x44100000
.equ  ITCM2,              0xB0000000

.equ  DTCM0_HEAP_START,   0x20010000
.equ  DTCM0_HEAP_END,     0x20000000
.equ  DTCM0_DATA_START,   0x20018000
.equ  DATA_START,         0x20018000

.equ  MCU2_CPU_WAIT,      0x40B000CC  /* ENABLE CPU1 */
.equ  MCU3_CPU_WAIT,      0xA0030088  /* ENABLE CPU2 */

.equ  CPU1_LENGTH,  0x10000000
.equ  CPU1_START,   0x10000004
.equ  CPU2_LENGTH,  0x10100000
.equ  CPU2_START,   0x10100004

.syntax unified
.cpu cortex-m7
.fpu softvfp
.thumb

.global  vectors
.global  Default_Handler

/* defined in linker script */
/* start address for the initialization values of the .data section. */
.word  _vectors_start
/* start address for the text section */
.word  _text_start
/* end address for the text section */
.word  _text_end
/* start address for the .data/.rodata/.bss section. */  
.word  _data_start
/* end address for the .data/.rodata/.bss section. */
.word  _data_end
/* start address for the .bss section. */
.word  _bss_start
/* end address for the .bss section. */
.word  _bss_end
/* start address for the .rodata section. */
.word  _rodata_start

.word _flash_start
.word _flash_end


/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called. 
 * @param  None
 * @retval : None
*/
    .section  .start.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:  
  ldr  sp,  =_estack             /* set stack pointer */

/* Copy CPU0 */
/* Copy the code from flash to ITCM0 */
  movs r1, #0
  ldr  r4, =_text_start     /* The start addr of code image */ 
  ldr  r0, =ITCM0           /* The start addr of itcm0 */
  b  LoopCopyCode
CopyInitCode:
  ldr  r3, =_text_start
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4
LoopCopyCode:
  ldr  r3, =_text_end       /* The end addr of code image */
  adds r2, r4, r1
  cmp  r2, r3
  bcc  CopyInitCode

/* Copy the data from flash to DTCM0 */
  movs r1, #0
  ldr  r4, =_data_start     /* The start addr of data/rodata/bss image */ 
  ldr  r0, =DATA_START      /* The start addr of dtcm0 */
  b  LoopCopyData
CopyInitData:
  ldr  r3, =_data_start
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4
LoopCopyData:
  ldr  r3, =_data_end       /* The end addr of data/rodata/bss image */
  adds  r2, r4, r1
  cmp  r2, r3
  bcc  CopyInitData

  /* clear the bss section */
  ldr r0, =0x0
  ldr r1, =_bss_start
  ldr r2, =_bss_end
LoopClearBss:
  str r0, [r1]
  add r1, r1, #4
  cmp r1, r2
  bcc LoopClearBss

 /* copy the CPU1 data/text from FLASH to ITCM1 */
  add  r0, r3, #0x1
  ldr  r2, [r0]                   /* length of cpu1 image */
  movs r1, #0
  add  r4, r0, #0x4               /* The start addr of data image */ 
  ldr  r5, =ITCM1                 /* The start addr of ITCM1 */
  b  LoopCopyCPU1
CopyInitCPU1:
  add  r3, r0, #0x4
  ldr  r3, [r3, r1]
  str  r3, [r5, r1]
  adds r1, #0x4
LoopCopyCPU1:
  add  r3, r4, r2                 /* r3: the end addr of data image */
  adds r6, r4, r1
  cmp  r6, r3
  bcc  CopyInitCPU1

/* copy the CPU2 data/text from FLASH to ITCM2 */
  add  r0, r3, #0x0
  ldr  r2, [r0]                   /* length of cpu2 image */
  movs r1, #0
  add  r4, r0, #0x4               /* The start addr of data image */ 
  ldr  r5, =ITCM2                 /* The start addr of ITCM2 */
  b  LoopCopyCPU2
CopyInitCPU2:
  add  r3, r0, #0x4
  ldr  r3, [r3, r1]
  str  r3, [r5, r1]
  adds r1, #0x4
LoopCopyCPU2:
  add  r3, r4, r2                 /* r3: the end addr of data image */
  adds r6, r4, r1
  cmp  r6, r3
  bcc  CopyInitCPU2

/* enable CPU1 */
  ldr r0, =MCU2_CPU_WAIT
  ldr r1, =0x0
  str r1, [r0]

/* enable CPU2 */
  ldr r0, =MCU3_CPU_WAIT
  ldr r1, =0x0
  str r1, [r0]

 /* load main addr to PC */
  ldr r0, =main
  mov pc, r0   

/**
 * @brief  This is the code that gets called when the processor receives an 
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 * @param  None     
 * @retval None       
*/
  .section .start.Default_Handler, "ax", %progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler

/******************************************************************************
*
* The minimal vector table for a Cortex M3. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
* 
*******************************************************************************/
  .section  .isr_vectors, "a", %progbits
  .type   vectors, %object
  .size   vectors, .-vectors
   
vectors:
  .word     _estack
  .word     Reset_Handler
  .word     default_isr
  .word     hardfault_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     SVC_Handler
  .word     default_isr
  .word     default_isr
  .word     PendSV_Handler
  .word     SYSTICK_IRQHandler
  .word     IRQHandler_16
  .word     IRQHandler_17
  .word     IRQHandler_18
  .word     IRQHandler_19
  .word     IRQHandler_20
  .word     IRQHandler_21
  .word     IRQHandler_22
  .word     IRQHandler_23
  .word     IRQHandler_24
  .word     IRQHandler_25
  .word     IRQHandler_26
  .word     IRQHandler_27
  .word     IRQHandler_28
  .word     IRQHandler_29
  .word     IRQHandler_30
  .word     IRQHandler_31
  .word     IRQHandler_32
  .word     IRQHandler_33
  .word     IRQHandler_34
  .word     IRQHandler_35
  .word     IRQHandler_36
  .word     IRQHandler_37
  .word     IRQHandler_38
  .word     IRQHandler_39
  .word     IRQHandler_40
  .word     IRQHandler_41
  .word     IRQHandler_42
  .word     IRQHandler_43
  .word     IRQHandler_44
  .word     IRQHandler_45
  .word     IRQHandler_46
  .word     IRQHandler_47
  .word     IRQHandler_48
  .word     IRQHandler_49
  .word     IRQHandler_50
  .word     IRQHandler_51
  .word     IRQHandler_52
  .word     IRQHandler_53
  .word     IRQHandler_54
  .word     IRQHandler_55
  .word     IRQHandler_56
  .word     IRQHandler_57
  .word     IRQHandler_58
  .word     IRQHandler_59
  .word     IRQHandler_60
  .word     IRQHandler_61
  .word     IRQHandler_62
  .word     IRQHandler_63
  .word     IRQHandler_64
  .word     IRQHandler_65
  .word     IRQHandler_66
  .word     IRQHandler_67
  .word     IRQHandler_68
  .word     IRQHandler_69
  .word     IRQHandler_70
  .word     IRQHandler_71
  .word     IRQHandler_72
  .word     IRQHandler_73
  .word     IRQHandler_74
  .word     IRQHandler_75
  .word     IRQHandler_76
  .word     IRQHandler_77
  .word     IRQHandler_78
  .word     IRQHandler_79
  .word     IRQHandler_80
  .word     IRQHandler_81
  .word     IRQHandler_82
  .word     IRQHandler_83
  .word     IRQHandler_84
  .word     IRQHandler_85
  .word     IRQHandler_86
  .word     IRQHandler_87
  .word     IRQHandler_88
  .word     IRQHandler_89
  .word     IRQHandler_90
  .word     IRQHandler_91
  .word     IRQHandler_92
  .word     IRQHandler_93
  .word     IRQHandler_94
  .word     IRQHandler_95
  .word     IRQHandler_96
  .word     IRQHandler_97
  .word     IRQHandler_98
  
/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler. 
* As they are weak aliases, any function with the same name will override 
* this definition.
* 
*******************************************************************************/
.weak      default_isr
.thumb_set default_isr,Default_Handler

.weak      hardfault_isr
.thumb_set hardfault_isr,Default_Handler

.weak      SVC_Handler
.thumb_set SVC_Handler,Default_Handler

.weak      PendSV_Handler
.thumb_set PendSV_Handler,Default_Handler

.weak      SYSTICK_IRQHandler
.thumb_set SYSTICK_IRQHandler,Default_Handler


