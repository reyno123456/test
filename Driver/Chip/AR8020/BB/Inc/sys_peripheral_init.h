#ifndef __SYS_PERIPHERAL_INIT_H
#define __SYS_PERIPHERAL_INIT_H

#include "config_functions_sel.h"
 /*
  ==============================================================================
             #####  Interface pin define between MCU and Baseband #####
  ==============================================================================

      * @brief  Baseband Interface
      *
      * @param
                 (+) SPI_MISO/ SPI_MOSI/ SPI_SCK/ SPI_CS
                 (+) TX_ENABLE/ RX_ENABLE
                 (+) SKY_GROUND_SEL
                 (+) RST_N
                 (+) UART-RX/ UART-TX
                 (+) UART-RX-2/ UART-TX-2

	*/
// ****************** SPI_MISO/ SPI_MOSI/ SPI_SCK/ SPI_CS *****************

#define SPI_NSS_GPIO_PORT       GPIOB
#define SPI_NSS                 GPIO_PIN_14

#define SPI_SCK_GPIO_PORT       GPIOA
#define SPI_SCK                 GPIO_PIN_9

#define SPI_MISO_GPIO_PORT      GPIOA
#define SPI_MISO                GPIO_PIN_8

#define SPI_MOSI_GPIO_PORT      GPIOB
#define SPI_MOSI                GPIO_PIN_15



//****************** TX_ENABLE/ RX_ENABLE  ***************

//GROUND --> TX_ENABLE (PA10) SKY  -->  RX_ENABLE (PA11)
#if 0
#ifdef BASEBAND_GRD
  #define TX_RX_EXIT_GPIO_PORT   GPIOA
  #define TX_RX_EXIT_PIN         GPIO_PIN_10
  #define EXTI15_10_EN
#endif

#ifdef BASEBAND_SKY
  #define TX_RX_EXIT_GPIO_PORT   GPIOA
  #define TX_RX_EXIT_PIN         GPIO_PIN_11
  #define EXTI15_10_EN
#endif
#endif
//******************  SKY_GROUND_SEL   *****************

#define SEL_GS_GPIO_PORT         GPIOA
#define SEL_GS_PIN               GPIO_PIN_12

//***********************  RST_N  **********************

#define BASEBAND_RESET_GPIO_PORT     GPIOC
#define BASEBAND_RESET               GPIO_PIN_15



//******* UART-RX/ UART-TX UART-RX-2/ UART-TX-2 *******

#define USART_TX_GPIO_PORT     GPIOB
#define USART_TX_PIN           GPIO_PIN_11

#define USART_RX_GPIO_PORT     GPIOB
#define USART_RX_PIN           GPIO_PIN_10

#define USART2_TX_GPIO_PORT    GPIOB
#define USART2_TX_PIN          GPIO_PIN_7

#define USART2_RX_GPIO_PORT    GPIOB
#define USART2_RX_PIN          GPIO_PIN_6

/**
  ==============================================================================
                  ##### STM32_Cy7c68013 Interface Pin Define #####
  ==============================================================================

      * @brief  Cy7c68013 Interface
      * @param
                 (+) PD[0:7](Data_BUS)
      *          (+) READ_SYNCLK
                 (+) READ_ID
                 (+) WRITE_LOCK
                 (+) WRITE_SYNCLK
                 (+) USBRESET
      * @{
      */
    /**
      * @}
      */

#define CY7C68013_W_GPIO_PORT    GPIOB
#define CY7C68013_W_PIN          GPIO_PIN_12

#define CY7C68013_SCK_GPIO_PORT  GPIOB
#define CY7C68013_SCK_PIN        GPIO_PIN_13

#define MCU_W_GPIO_PORT          GPIOB
#define MCU_W_PIN                GPIO_PIN_5

#define MCU_SCK_GPIO_PORT        GPIOB
#define MCU_SCK_PIN              GPIO_PIN_2

#define CY7C68013_RESET_PORT    GPIOB
#define CY7C68013_RESET         GPIO_PIN_1
typedef enum 
{
  DISABLE = 0, 
  ENABLE = !DISABLE
} FunctionalState;


void SystemClock_Config(void);
void Timer2_Init(void);
void Sky_Timer3_Init(void);
void Grd_Timer3_Init(void);
void Baseband_Spi_Initial(void);
void Sky_Grd_Sel_Initial(void);
void Baseband_Reset_Initial(void);
void Tx_Rx_Irq_Initial(void);
void Baseband_Usart2_Initial(void);
void Baseband_Usart_Initial(void);

/**
  ==============================================================================
                      ##### STM32_Cy7c68013 Interface #####
  ==============================================================================
 */
void Cy7c68013_Interface_Initial(void);

#endif
