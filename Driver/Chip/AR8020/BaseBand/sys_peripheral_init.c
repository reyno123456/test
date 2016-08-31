#include "sys_peripheral_init.h"
#include "stm32f7xx_hal_tim.h"


TIM_HandleTypeDef Tim2Handle;
TIM_HandleTypeDef Tim3Handle;

/*********************Initial Systemclk*********************/

void  SystemClock_Config(void)
{
   //todo:
}


void Timer2_Init(void)
{
#if 0
	//enable timer2
	__TIM2_CLK_ENABLE();

	Tim2Handle.Instance = TIM2;

	//定时1ms
	Tim2Handle.Init.Period = 1000 - 1;
	Tim2Handle.Init.Prescaler = (uint32_t) 72-1;
	Tim2Handle.Init.ClockDivision = 0;

	Tim2Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&Tim2Handle);
	HAL_NVIC_SetPriority(TIM2_IRQn, 2,0);
#endif
}

#ifdef BASEBAND_SKY

void Sky_Timer3_Init(void)
{
#if 0
  // 开时钟
    __TIM3_CLK_ENABLE();

   Tim3Handle.Instance = TIM3;

   Tim3Handle.Init.Period = 1000 - 1;
   Tim3Handle.Init.Prescaler = (uint32_t) 72-1;
   Tim3Handle.Init.ClockDivision = 0;

   Tim3Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
   HAL_TIM_Base_Init(&Tim3Handle);
   HAL_NVIC_SetPriority(TIM3_IRQn, 2,0);
#endif
}
#endif

#ifdef BASEBAND_GRD

void Grd_Timer3_Init(void)
{
#if 0
	// 开时钟
	__TIM3_CLK_ENABLE();

	Tim3Handle.Instance = TIM3;

	//定时1.25ms
	Tim3Handle.Init.Period = 1000 - 1;
	Tim3Handle.Init.Prescaler = (uint32_t) 90-1;
	Tim3Handle.Init.ClockDivision = 0;

	Tim3Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&Tim3Handle);

	HAL_NVIC_SetPriority(TIM3_IRQn, 2,0);
#endif
}
#endif


UART_HandleTypeDef Uart1Handle;
UART_HandleTypeDef Uart3Handle;

void Baseband_Spi_Initial(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	// START CLOCK
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitStruct.Pin =  SPI_NSS;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(SPI_NSS_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(SPI_NSS_GPIO_PORT, SPI_NSS, GPIO_PIN_SET);

	GPIO_InitStruct.Pin =  SPI_SCK ;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(SPI_SCK_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SPI_MOSI;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(SPI_MOSI_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SPI_MISO;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(SPI_MISO_GPIO_PORT, &GPIO_InitStruct);
}

//***********************Sky_Ground_Sel***********
void Sky_Grd_Sel_Initial(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = SEL_GS_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(SEL_GS_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(SEL_GS_GPIO_PORT, SEL_GS_PIN, GPIO_PIN_SET);

}


//**********************Baseband reset initial****
void Baseband_Reset_Initial(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {
		.Pin = BASEBAND_RESET,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_PULLUP,
		.Speed = GPIO_SPEED_HIGH,
	};
	
	HAL_GPIO_Init(BASEBAND_RESET_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(BASEBAND_RESET_GPIO_PORT, BASEBAND_RESET, GPIO_PIN_RESET);
}

void Tx_Rx_Irq_Initial(void)
{
     GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = TX_RX_EXIT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(TX_RX_EXIT_GPIO_PORT, &GPIO_InitStruct);

    #ifdef EXTI9_5_EN
      HAL_NVIC_SetPriority(EXTI9_5_IRQn, 3, 0);
      HAL_Delay(1);
      HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    #endif
	
    #ifdef EXTI15_10_EN
      HAL_NVIC_SetPriority(EXTI15_10_IRQn, 3, 0);
      HAL_Delay(1);
      HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    #endif
}

#ifdef EXTI9_5_EN
void EXTI9_5_IRQHandler(void)
{
#if 0
	__HAL_GPIO_EXTI_CLEAR_IT(TX_RX_EXIT_PIN);
	//Start timer2
	HAL_TIM_Base_Start_IT(&Tim2Handle);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	HAL_TIM_Base_Start_IT(&Tim3Handle);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
#endif
}
#endif

#ifdef EXTI15_10_EN
void EXTI15_10_IRQHandler(void)
{
#if 0
	__HAL_GPIO_EXTI_CLEAR_IT(TX_RX_EXIT_PIN);
	//Start timer2
	HAL_TIM_Base_Start_IT(&Tim2Handle);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
#endif	
}

#endif


//baseband Uart2<----->stm32 Uart1
void Baseband_Usart2_Initial(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	__USART3_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	/**USART1 GPIO Configuration
	PB6     ------> USART1_TX
	PB7     ------> USART1_RX
	*/
	GPIO_InitStruct.Pin = USART2_RX_PIN;//PB6     ------> USART1_TX
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = USART2_TX_PIN;//PB7     ------> USART1_RX
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStruct);

	Uart1Handle.Instance = USART1;
	Uart1Handle.Init.BaudRate = 256000;
	Uart1Handle.Init.WordLength = UART_WORDLENGTH_8B;
	Uart1Handle.Init.StopBits = UART_STOPBITS_1;
	Uart1Handle.Init.Parity = UART_PARITY_NONE;
	Uart1Handle.Init.Mode = UART_MODE_TX_RX;
	Uart1Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	Uart1Handle.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&Uart1Handle);

	HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);        //   14
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	__HAL_UART_ENABLE_IT(&Uart1Handle, UART_IT_RXNE);
}

//baseband Uart<----->stm32 Uart3
void Baseband_Usart_Initial(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	 __USART1_CLK_ENABLE();
	 __GPIOB_CLK_ENABLE();
	/**USART3 GPIO Configuration
	PB10     ------> USART3_TX
	PB11     ------> USART3_RX
	*/
	GPIO_InitStruct.Pin = USART_RX_PIN;	//PB10     ------> USART3_TX
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(USART_RX_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = USART_TX_PIN;	//PB11     ------> USART3_RX
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USART_TX_GPIO_PORT, &GPIO_InitStruct);

	Uart3Handle.Instance = USART3;
	Uart3Handle.Init.BaudRate = 256000;
	Uart3Handle.Init.WordLength = UART_WORDLENGTH_8B;
	Uart3Handle.Init.StopBits = UART_STOPBITS_1;
	Uart3Handle.Init.Parity = UART_PARITY_NONE;
	Uart3Handle.Init.Mode = UART_MODE_TX_RX;
	Uart3Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	Uart3Handle.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&Uart3Handle);

	HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);        //   14
	HAL_NVIC_EnableIRQ(USART3_IRQn);
	__HAL_UART_ENABLE_IT(&Uart3Handle, UART_IT_RXNE);
}

//***********************************************************************************

/**
  ==============================================================================
                      ##### STM32_Cy7c68013 Interface #####
  ==============================================================================

      * @brief  STM32_Cy7c68013 Interface
      * @note   Named for master(Cy7c68013)
      * @param
                 (+) PD[0:7]      -- Data_BUS
      *          (+) READ_SYNCLK
                 (+) READ_ID      -- When MCU write to cy7c68013 need to lock the databus,when READ_ID=0,
                                     the mcu is in write mode and tell the 68013 can not write
                 (+) WRITE_LOCK   -- When Cy7c68013 write to mcu need to lock the databus,when WRITE_LOCK=0,
                                     the Cy7c68013 is in write mode and tell the mcu,the mcu can not write
                 (+) WRITE_SYNCLK -- When Cy7c68013 write to MCU need a synclk
                 (+) USBRESET     -- Mcu Reset the Cy7c68013,hardware Reset
      * @{
      */
	  
/**
  * @}
  */
void Cy7c68013_Interface_Initial(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	//parallel port in for STM32 with cy7c68013---io port
	GPIO_InitStruct.Pin =GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
							|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//cy7c68013 write enable signal input pin--- cy7c68013 to stm32
	GPIO_InitStruct.Pin = CY7C68013_W_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(CY7C68013_W_GPIO_PORT, &GPIO_InitStruct);

	//cy7c68013 sclk signal input pin--- cy7c68013 to stm32
	GPIO_InitStruct.Pin = CY7C68013_SCK_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(CY7C68013_SCK_GPIO_PORT, &GPIO_InitStruct);

	//stm32 write enable signal output pin--- stm32 to cy7c68013
	GPIO_InitStruct.Pin = MCU_W_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(MCU_W_GPIO_PORT, &GPIO_InitStruct);

	//stm32 sclk signal input pin--- stm32 to cy7c68013
	GPIO_InitStruct.Pin = MCU_SCK_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(MCU_SCK_GPIO_PORT, &GPIO_InitStruct);

	//Cy7c68013 Reset signal
	GPIO_InitStruct.Pin = CY7C68013_RESET;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(CY7C68013_RESET_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(CY7C68013_RESET_PORT, CY7C68013_RESET, GPIO_PIN_SET);
}
