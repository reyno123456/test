/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_nvic.h
Description: this module contains the helper fucntions set interrupt
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/26
History:
         0.0.1    2016/12/26    The initial version of hal_nvic.h
*****************************************************************************/
#ifndef __HAL_NVIC_H
#define __HAL_NVIC_H

#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bits for pre-emption priority
                                                                 5 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bits for pre-emption priority
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority
                                                                 1 bits for subpriority */
#define NVIC_PRIORITYGROUP_5         ((uint32_t)0x00000002) /*!< 5 bits for pre-emption priority
                                                                 0 bits for subpriority */
/**
  * @brief  register interrupt handler and clear handler.
  * @param  IRQn: External interrupt number.This parameter can be an enumerator of IRQ_type enumeration
  * @param  NVIC_Handler:         interrupt handler.
  * @param  NVIC_ClearHandler: clear handler.       
  * @retval HAL_OK            function is well done.
  * @note   None
  */
HAL_RET_T HAL_NVIC_RegisterHandler(IRQ_type IRQn, Irq_handler NVIC_Handler, Irq_handler NVIC_ClearHandler);


/**
  * @brief  unregister interrupt handler and clear handler.
  * @param  IRQn: External interrupt number.This parameter can be an enumerator of IRQ_type enumeration    
  * @retval HAL_OK            function is well done.
  * @note   None
  */
HAL_RET_T HAL_NVIC_UNRegisterHandler(IRQ_type IRQn);

/**
  * @brief  Sets the priority grouping field (preemption priority and subpriority)
  *         using the required unlock sequence.
  * @param  PriorityGroup: The priority grouping bits length. 
  *         This parameter can be one of the following values:
  *         @arg NVIC_PRIORITYGROUP_0: 0 bits for preemption priority
  *                                    5 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_1: 1 bits for preemption priority
  *                                    4 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_2: 2 bits for preemption priority
  *                                    3 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_3: 3 bits for preemption priority
  *                                    2 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_4: 4 bits for preemption priority
  *                                    1 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_5: 5 bits for preemption priority
  *                                    0 bits for subpriority
  * @retval HAL_OK            function is well done.
  * @note   When the NVIC_PriorityGroup_0 is selected, IRQ preemption is no more possible. 
  *         The pending IRQ priority will be managed only by the subpriority. 
  */
HAL_RET_T HAL_NVIC_SetPriorityGrouping(uint32_t u32_priorityGroup);
/**
  * @brief  Sets the priority of an interrupt.
  * @param  IRQn: External interrupt number.This parameter can be an enumerator of IRQ_type enumeration
  * @param  PreemptPriority: The preemption priority for the IRQn channel.
  * @param  SubPriority: the subpriority level for the IRQ channel.       
  * @retval HAL_OK            function is well done.
  * @note   None
  */
HAL_RET_T HAL_NVIC_SetPriority(IRQ_type IRQn, uint32_t u32_preemptPriority, uint32_t u32_subPriority);
/**
  * @brief  Enables a device specific interrupt in the NVIC interrupt controller. 
  * @param  IRQn: External interrupt number.This parameter can be an enumerator of IRQ_type enumeration
  * @retval HAL_OK            function is well done.
  * @note   To configure interrupts priority correctly, the NVIC_PriorityGroupConfig()
  *         function should be called before.
  */
HAL_RET_T HAL_NVIC_EnableIRQ(IRQ_type IRQn);
/**
  * @brief  Disables a device specific interrupt in the NVIC interrupt controller.
  * @param  IRQn: External interrupt number.This parameter can be an enumerator of IRQ_type enumeration
  * @retval HAL_OK            function is well done.
  * @note   None
  */
HAL_RET_T HAL_NVIC_DisableIRQ(IRQ_type IRQn);
/**
  * @brief  Gets the priority grouping field from the NVIC Interrupt Controller.
  * @param  p_retPriorityGroup: Priority grouping field (SCB->AIRCR [10:8] PRIGROUP field)
  * @retval HAL_OK             function is well done.
  * @note   None
  */
HAL_RET_T HAL_NVIC_GetPriorityGrouping(uint32_t *p_retPriorityGroup);
/**
  * @brief  Gets the priority of an interrupt.
  * @param  IRQn: External interrupt number.This parameter can be an enumerator of IRQ_type enumeration
  * @param  PriorityGroup: the priority grouping bits length.
  *         This parameter can be one of the following values:
  *         @arg NVIC_PRIORITYGROUP_0: 0 bits for preemption priority
  *                                    5 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_1: 1 bits for preemption priority
  *                                    4 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_2: 2 bits for preemption priority
  *                                    3 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_3: 3 bits for preemption priority
  *                                    2 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_4: 4 bits for preemption priority
  *                                    1 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_5: 5 bits for preemption priority
  *                                    0 bits for subpriority
  * @param  pPreemptPriority: Pointer on the Preemptive priority value (starting from 0).
  * @param  pSubPriority: Pointer on the Subpriority value (starting from 0).
  * @retval HAL_OK            function is well done.
  * @note   None
  */
HAL_RET_T HAL_NVIC_GetPriority(IRQ_type IRQn, uint32_t PriorityGroup, uint32_t *p_preemptPriority, uint32_t *p_subPriority);
/**
  * @brief  Sets Pending bit of an external interrupt.
  * @param  IRQn: External interrupt number.This parameter can be an enumerator of IRQ_type enumeration
  * @retval HAL_OK            function is well done.
  * @note   None
  */
HAL_RET_T HAL_NVIC_SetPendingIRQ(IRQ_type IRQn);
/**
  * @brief  Gets Pending Interrupt (reads the pending register in the NVIC 
  *         and returns the pending bit for the specified interrupt).
  * @param  IRQn： External interrupt number.This parameter can be an enumerator of IRQ_type enumeration
  * @param  p_retPending  Pointer on the get pending.
  *               status  - 0  Interrupt status is not pending.
  *                       - 1  Interrupt status is pending.
  * @retval HAL_OK            function is well done.
  * @note   None
  */
HAL_RET_T HAL_NVIC_GetPendingIRQ(IRQ_type IRQn,uint32_t *p_retPending);
/**
  * @brief  Clears the pending bit of an external interrupt.
  * @param  IRQn: External interrupt number.This parameter can be an enumerator of IRQ_type enumeration
  * @retval HAL_OK            function is well done.
  * @note   None
  */
HAL_RET_T HAL_NVIC_ClearPendingIRQ(IRQ_type IRQn);
/**
  * @brief Gets active interrupt ( reads the active register in NVIC and returns the active bit).
  * @param IRQn: External interrupt number.This parameter can be an enumerator of IRQ_type enumeration
  * @param  p_retActive: Pointer on the get active pending.
  *                      status   - 0  Interrupt status is not pending.
  *                               - 1  Interrupt status is pending.
  * @retval HAL_OK            function is well done.
  * @note   None
  */
HAL_RET_T HAL_NVIC_GetActive(IRQ_type IRQn, uint32_t *p_retActive);


#endif 

