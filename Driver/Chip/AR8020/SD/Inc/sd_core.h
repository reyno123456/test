/**
  * @file    sd_core.h
  * @author  Minzhao
  * @version V1.0.0
  * @date    7-7-2016
  * @brief   Header file of sd core.
  *          This file contains:
  *           - SD's registers declarations and bits definition
  *           - Macros to access SD controller hardware
  */

#ifndef __SD_CORE_H
#define __SD_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f746xx.h"
#include "command.h"
/**
  * @brief SD controller register
  */
typedef struct
{
  uint32_t  CTRL         ;   /** Control */
  uint32_t  PWREN        ;   /** Power-enable */
  uint32_t  CLKDIV       ;   /** Clock divider */
  uint32_t  CLKSRC       ;   /** Clock source */
  uint32_t  CLKENA       ;   /** Clock enable */
  uint32_t  TMOUT        ;   /** Timeout */
  uint32_t  CTYPE        ;   /** Card type */
  uint32_t  BLKSIZ       ;   /** Block Size */
  uint32_t  BYCTNT       ;   /** Byte count */
  uint32_t  INTMASK      ;    /** Interrupt Mask */
  uint32_t  CMDARG       ;   /** Command Argument */
  uint32_t  CMD          ;   /** Command */
  uint32_t  RESP0        ;   /** Response 0 */
  uint32_t  RESP1        ;   /** Response 1 */
  uint32_t  RESP2        ;   /** Response 2 */
  uint32_t  RESP3        ;   /** Response 3 */
  uint32_t  MINTSTS      ;   /** Masked interrupt status */
  uint32_t  RINTSTS      ;   /** Raw interrupt status */
  uint32_t  STATUS       ;   /** Status */
  uint32_t  FIFOTH       ;   /** FIFO threshold */
  uint32_t  CDETECT      ;   /** Card detect */
  uint32_t  WRTPRT       ;   /** Write protect */
  uint32_t  GPIO         ;   /** General Purpose IO */
  uint32_t  TCBCNT       ;   /** Transferred CIU byte count */
  uint32_t  TBBCNT       ;   /** Transferred host/DMA to/from byte count */
  uint32_t  DEBNCE       ;   /** Card detect debounce */
  uint32_t  USRID        ;   /** User ID */
  uint32_t  VERID        ;   /** Version ID */
  uint32_t  HCON         ;   /** Hardware Configuration */
  uint32_t  UHSREG       ;   /** Reserved */
  uint32_t  RST_N        ;   /** Hardware reset */
  uint32_t  RESERVED     ;
  uint32_t  BMOD         ;   /** Bus mode Register */
  uint32_t  PLDMND       ;   /** Poll Demand */
  uint32_t  DBADDR       ;   /** Descriptor Base Address */
  uint32_t  IDSTS        ;   /** Internal DMAC Status */
  uint32_t  IDINTEN      ;   /** Internal DMAC Interrupt Enable */
  uint32_t  DSCADDR      ;   /** Current Host Descriptor Address */
  uint32_t  BUFADDR      ;   /** Current Host Buffer Address */
  uint32_t  CardThrCtl   ;   /** Card Read Threshold Enable (CardRdThrEn) Default value: 0x0 */
  uint32_t  Back_end_powe;   /** 16 bits; Back-end Power, default value: 0x0 */
  uint32_t  UHS_REG_EXT  ;   /** eMMC 4.5 1.2V register, default value: 0x0 */
  uint32_t  EMMC_DDR_REG ;   /** eMMC DDR START bit detection or HS400 mode enable control register */
  uint32_t  ENABLE_SHIFT ;   /** Phase shift control register, default value 0x0 */
} SDMMC_REG;

#define SDMMC_ADDR       ((SDMMC_REG *)SDMMC_BASE)
/* Power Enable Register */
#define SDMMC_PWREN_0                              (uint32_t)BIT(0)
#define SDMMC_PWREN_1                              (uint32_t)BIT(1)
#define SDMMC_PWREN_2                              (uint32_t)BIT(2)
#define SDMMC_PWREN_3                              (uint32_t)BIT(3)
#define SDMMC_PWREN_4                              (uint32_t)BIT(4)
/* Interrupt status & mask register defines */
#define SDMMC_INTMASK_SDIO(n)                      (uint32_t)BIT(16 + (n))
#define SDMMC_INTMASK_EBE                          (uint32_t)BIT(15)
#define SDMMC_INTMASK_ACD                          (uint32_t)BIT(14)
#define SDMMC_INTMASK_SBE                          (uint32_t)BIT(13)
#define SDMMC_INTMASK_HLE                          (uint32_t)BIT(12)
#define SDMMC_INTMASK_FRUN                         (uint32_t)BIT(11)
#define SDMMC_INTMASK_HTO                          (uint32_t)BIT(10)
#define SDMMC_INTMASK_DRTO                         (uint32_t)BIT(9)
#define SDMMC_INTMASK_RTO                          (uint32_t)BIT(8)
#define SDMMC_INTMASK_DCRC                         (uint32_t)BIT(7)
#define SDMMC_INTMASK_RCRC                         (uint32_t)BIT(6)
#define SDMMC_INTMASK_RXDR                         (uint32_t)BIT(5)
#define SDMMC_INTMASK_TXDR                         (uint32_t)BIT(4)
#define SDMMC_INTMASK_DATA_OVER                    (uint32_t)BIT(3)
#define SDMMC_INTMASK_CMD_DONE                     (uint32_t)BIT(2)
#define SDMMC_INTMASK_RESP_ERR                     (uint32_t)BIT(1)
#define SDMMC_INTMASK_CARD_DETECT                  (uint32_t)BIT(0)
#define SDMMC_INTMASK_ERROR                        (uint32_t)0xBFC2
/* Raw Interrupt Status Register */
#define SDMMC_RINTSTS_EBE                          (uint32_t)BIT(15)
#define SDMMC_RINTSTS_ACD                          (uint32_t)BIT(14)
#define SDMMC_RINTSTS_SBE                          (uint32_t)BIT(13)
#define SDMMC_RINTSTS_HLE                          (uint32_t)BIT(12)
#define SDMMC_RINTSTS_FRUN                         (uint32_t)BIT(11)
#define SDMMC_RINTSTS_HTO                          (uint32_t)BIT(10)
#define SDMMC_RINTSTS_DRTO                         (uint32_t)BIT(9)
#define SDMMC_RINTSTS_RTO                          (uint32_t)BIT(8)
#define SDMMC_RINTSTS_DCRC                         (uint32_t)BIT(7)
#define SDMMC_RINTSTS_RCRC                         (uint32_t)BIT(6)
#define SDMMC_RINTSTS_RXDR                         (uint32_t)BIT(5)
#define SDMMC_RINTSTS_TXDR                         (uint32_t)BIT(4)
#define SDMMC_RINTSTS_DATA_OVER                    (uint32_t)BIT(3)
#define SDMMC_RINTSTS_CMD_DONE                     (uint32_t)BIT(2)
#define SDMMC_RINTSTS_RESP_ERR                     (uint32_t)BIT(1)
#define SDMMC_RINTSTS_CARD_DETECT                  (uint32_t)BIT(0)
/* Control register define */
#define SDMMC_CTRL_USE_INTERNAL_IDMAC             (uint32_t)BIT(25)
#define SDMMC_CTRL_CEATA_DEVICE_INTERRUPT_STATUS  (uint32_t)BIT(11)
#define SDMMC_CTRL_SEND_AUTO_STOP_CCSD            (uint32_t)BIT(10)
#define SDMMC_CTRL_SEND_CCSD                      (uint32_t)BIT(9)
#define SDMMC_CTRL_ABORT_READ_DATA                (uint32_t)BIT(8)
#define SDMMC_CTRL_SEND_IRQ_RESPONSE              (uint32_t)BIT(7)
#define SDMMC_CTRL_READ_WAIT                      (uint32_t)BIT(6)
#define SDMMC_CTRL_DMA_ENABLE                     (uint32_t)BIT(5)
#define SDMMC_CTRL_INT_ENABLE                     (uint32_t)BIT(4)
#define SDMMC_CTRL_DMA_RESET                      (uint32_t)BIT(2)
#define SDMMC_CTRL_FIFO_RESET                     (uint32_t)BIT(1)
#define SDMMC_CTRL_CONTROLLER_RESET               (uint32_t)BIT(0)
/* Clock Divider Register */
#define SDMMC_CLKDIV_BIT7                          (uint32_t)BIT(7)
#define SDMMC_CLKDIV_BIT6                          (uint32_t)BIT(6)
#define SDMMC_CLKDIV_BIT5                          (uint32_t)BIT(5)
#define SDMMC_CLKDIV_BIT4                          (uint32_t)BIT(4)
#define SDMMC_CLKDIV_BIT3                          (uint32_t)BIT(3)
#define SDMMC_CLKDIV_BIT2                          (uint32_t)BIT(2)
#define SDMMC_CLKDIV_BIT1                          (uint32_t)BIT(1)
#define SDMMC_CLKDIV_BIT0                          (uint32_t)BIT(0)
/* Clock source register */
#define SDMMC_CLKSRC_CLKDIV0                       (uint32_t)0x00
#define SDMMC_CLKSRC_CLKDIV1                       (uint32_t)0x01
#define SDMMC_CLKSRC_CLKDIV2                       (uint32_t)0x02
#define SDMMC_CLKSRC_CLKDIV3                       (uint32_t)0x03
/* Clock Enable register defines */
#define SDMMC_CLKENA_LOW_PWR                       (uint32_t)BIT(16)
#define SDMMC_CLKENA_ENABLE                        (uint32_t)BIT(0)
/* time-out register defines */
#define SDMMC_TMOUT_DATA(n)                        (uint32_t)SBF(8, (n))
#define SDMMC_TMOUT_DATA_MSK                       (uint32_t)0xFFFFFF00
#define SDMMC_TMOUT_DEFAULT                        (uint32_t)0xFFFFFF40
#define SDMMC_TMOUT_RESP(n)                        (uint32_t)((n) & 0xFF)
#define SDMMC_TMOUT_RESP_MSK                       (uint32_t)0xFF
/* card-type register defines */
#define SDMMC_CTYPE_8BIT                           (uint32_t)BIT(16)
#define SDMMC_CTYPE_4BIT                           (uint32_t)BIT(0)
#define SDMMC_CTYPE_1BIT                           (uint32_t)0x00
/* Command register defines */
#define SDMMC_CMD_START_CMD                        (uint32_t)BIT(31)
#define SDMMC_CMD_USE_HOLD_REG                     (uint32_t)BIT(29)
#define SDMMC_CMD_VOLT_SWITCH                      (uint32_t)BIT(28)
#define SDMMC_CMD_BOOT_MODE                        (uint32_t)BIT(27)
#define SDMMC_CMD_DISABLE_BOOT                     (uint32_t)BIT(26)
#define SDMMC_CMD_EXPECT_BOOT_ACK                  (uint32_t)BIT(25)
#define SDMMC_CMD_ENABLE_BOOT                      (uint32_t)BIT(24)
#define SDMMC_CMD_CCS_EXP                          (uint32_t)BIT(23)
#define SDMMC_CMD_CEATA_RD                         (uint32_t)BIT(22)
#define SDMMC_CMD_UPDATE_CLK                       (uint32_t)BIT(21)
#define SDMMC_CMD_SEND_INIT                        (uint32_t)BIT(15)
#define SDMMC_CMD_STOP_ABORT_CMD                   (uint32_t)BIT(14)
#define SDMMC_CMD_PRV_DAT_WAIT                     (uint32_t)BIT(13)
#define SDMMC_CMD_SEND_STOP                        (uint32_t)BIT(12)
#define SDMMC_CMD_TRANSFER_MODE                    (uint32_t)BIT(11)
#define SDMMC_CMD_DAT_READ_WRITE                   (uint32_t)BIT(10)
#define SDMMC_CMD_DAT_EXP                          (uint32_t)BIT(9)
#define SDMMC_CMD_RESP_CRC                         (uint32_t)BIT(8)
#define SDMMC_CMD_RESP_LONG                        (uint32_t)BIT(7)
#define SDMMC_CMD_RESP_EXP                         (uint32_t)BIT(6)
#define SDMMC_CMD_INDX(n)                          (uint32_t)((n) & 0x3F)
/* Status register defines */
#define SDMMC_GET_FCNT(x)                          (uint32_t)(((x)>>17) & 0x1FFF)
/* Internal DMAC interrupt defines */
#define SDMMC_IDSTS_INT_AI                         (uint32_t)BIT(9)
#define SDMMC_IDSTS_INT_NI                         (uint32_t)BIT(8)
#define SDMMC_IDSTS_INT_CES                        (uint32_t)BIT(5)
#define SDMMC_IDSTS_INT_DU                         (uint32_t)BIT(4)
#define SDMMC_IDSTS_INT_FBE                        (uint32_t)BIT(2)
#define SDMMC_IDSTS_INT_RI                         (uint32_t)BIT(1)
#define SDMMC_IDSTS_INT_TI                         (uint32_t)BIT(0)
/* Internal DMAC bus mode bits */
#define SDMMC_BMOD_ENABLE                          (uint32_t)BIT(7)
#define SDMMC_BMOD_FB                              (uint32_t)BIT(1)
#define SDMMC_BMOD_SWRESET                         (uint32_t)BIT(0)
/* status register */
#define SDMMC_STATUS_DMA_REQ                       (uint32_t)BIT(31)
#define SDMMC_STATUS_DMA_ACK                       (uint32_t)BIT(30)
#define SDMMC_STATUS_FIFO_COUNT                    (uint32_t)SBF(0x1FFF, (29))
#define SDMMC_STATUS_RESP_INDEX                    (uint32_t)SBF(0x3F, (16))
#define SDMMC_STATUS_DATA_BUSY                     (uint32_t)BIT(9)
#define SDMMC_STATUS_FIFO_FULL                     (uint32_t)BIT(3)
#define SDMMC_STATUS_FIFO_EMPTY                    (uint32_t)BIT(2)
#define SDMMC_STATUS_FIFO_TXWMK                    (uint32_t)BIT(1)
#define SDMMC_STATUS_FIFO_RXWMK                    (uint32_t)BIT(0)
/* Internal DMAC Interrupt Enable Register */
#define SDMMC_IDINTEN_AI                           (uint32_t)BIT(9)
#define SDMMC_IDINTEN_NI                           (uint32_t)BIT(8)
#define SDMMC_IDINTEN_CES                          (uint32_t)BIT(5)
#define SDMMC_IDINTEN_DU                           (uint32_t)BIT(4)
#define SDMMC_IDINTEN_FBE                          (uint32_t)BIT(2)
#define SDMMC_IDINTEN_RI                           (uint32_t)BIT(1)
#define SDMMC_IDINTEN_TI                           (uint32_t)BIT(0)
/* IDMAC DES0 Configuration */
#define SDMMC_DES0_OWN                             (uint32_t)BIT(31)
#define SDMMC_DES0_CES                             (uint32_t)BIT(30)
#define SDMMC_DES0_ER                              (uint32_t)BIT(5)
#define SDMMC_DES0_CH                              (uint32_t)BIT(4)
#define SDMMC_DES0_FS                              (uint32_t)BIT(3)
#define SDMMC_DES0_LD                              (uint32_t)BIT(2)
#define SDMMC_DES0_DIC                             (uint32_t)BIT(1)

//===========================================
//RESPONSE STATUS
//===========================================
/* RESPONSE1 */
#define SDMMC_RESP1_OUT_OF_RANGE                    (uint32_t)BIT(31)
#define SDMMC_RESP1_ADDRESS_ERROR                   (uint32_t)BIT(30)
#define SDMMC_RESP1_BLOCK_LEN_ERROR                 (uint32_t)BIT(29)
#define SDMMC_RESP1_ERASE_SEQ_ERROR                 (uint32_t)BIT(28)
#define SDMMC_RESP1_ERASE_PARAM                     (uint32_t)BIT(27)
#define SDMMC_RESP1_WP_VIOLATION                    (uint32_t)BIT(26)
#define SDMMC_RESP1_CARD_IS_LOCKED                  (uint32_t)BIT(25)
#define SDMMC_RESP1_LOCK_UNLOCK_FAILED              (uint32_t)BIT(24)
#define SDMMC_RESP1_COM_CRC_ERROR                   (uint32_t)BIT(23)
#define SDMMC_RESP1_ILLEGAL_COMMAND                 (uint32_t)BIT(22)
#define SDMMC_RESP1_CARD_ECC_FAILED                 (uint32_t)BIT(21)
#define SDMMC_RESP1_CC_ERROR                        (uint32_t)BIT(20)
#define SDMMC_RESP1_ERROR                           (uint32_t)BIT(19)
#define SDMMC_RESP1_CSD_OVERWRITE                   (uint32_t)BIT(16)
#define SDMMC_RESP1_WP_ERASE_SKIP                   (uint32_t)BIT(15)
#define SDMMC_RESP1_CARD_ECC_DISABLED               (uint32_t)BIT(14)
#define SDMMC_RESP1_ERASE_RESET                     (uint32_t)BIT(13)
#define SDMMC_RESP1_CURRENT_STATE                   (uint32_t)SBF(0xF, (12))
#define SDMMC_RESP1_READY_FOR_DATA                  (uint32_t)BIT(8)
#define SDMMC_RESP1_APP_CMD                         (uint32_t)BIT(5)
#define SDMMC_RESP1_AKE_SEQ_ERROR                   (uint32_t)BIT(3)

#define SDMMC_RESP0                                ((uint32_t)0x00000000)
#define SDMMC_RESP1                                ((uint32_t)0x00000004)
#define SDMMC_RESP2                                ((uint32_t)0x00000008)
#define SDMMC_RESP3                                ((uint32_t)0x0000000C)
/**
  * @brief  SDMMC Configuration Structure definition
  */
typedef struct
{
  uint32_t  ClockEdge;            /*!< Specifies the clock transition on which the bit capture is made.
                                           This parameter can be a value of @ref SDMMC_LL_Clock_Edge                 */
  uint32_t  ClockBypass;          /*!< Specifies whether the SDMMC Clock divider bypass is
                                           enabled or disabled.
                                           This parameter can be a value of @ref SDMMC_LL_Clock_Bypass               */
  uint32_t  ClockPowerSave;       /*!< Specifies whether SDMMC Clock output is enabled or
                                           disabled when the bus is idle.
                                           This parameter can be a value of @ref SDMMC_LL_Clock_Power_Save           */
  uint32_t  BusWide;              /*!< Specifies the SDMMC bus width.
                                           This parameter can be a value of @ref SDMMC_LL_Bus_Wide                   */
  uint32_t  HardwareFlowControl;  /*!< Specifies whether the SDMMC hardware flow control is enabled or disabled.
                                           This parameter can be a value of @ref SDMMC_LL_Hardware_Flow_Control      */
  uint32_t  ClockDiv;             /*!< Specifies the clock frequency of the SDMMC controller.
                                           This parameter can be a value between Min_Data = 0 and Max_Data = 255 */
} SDMMC_InitTypeDef;

typedef enum
{
  SDMMC_RESPONSE_NO = 0,          /* no response*/
  SDMMC_RESPONSE_R1,              /*normal response command. length = 48bits */
  SDMMC_RESPONSE_R2,              /*136bits CID for CMD2 and CMD10 and CSD for CMD9*/
  SDMMC_RESPONSE_R3,              /*48bits OCR for ACMD41*/
  SDMMC_RESPONSE_R6,              /*48bits RCA for CMD3*/
  SDMMC_RESPONSE_R7               /*48bits support the voltage information for CMD8*/
} RESPTYPE;

/**
  * @brief  SDMMC Command Control structure
  */
typedef struct
{
  uint32_t Argument;            /*!< Specifies the SDMMC command argument which is sent
                                     to a card as part of a command message. If a command
                                     contains an argument, it must be loaded into this register
                                     before writing the command to the command register.              */
  uint32_t CmdIndex;            /*!< Specifies the SDMMC command index. It must be Min_Data = 0 and
                                     Max_Data = 64                                                    */
  RESPTYPE Response;            /* specifies the response type*/
  uint32_t Attribute;           /* specifies the CMD attribute*/
} SDMMC_CmdInitTypeDef;

/**
  * @brief  SDMMC Data Control structure
  */
typedef struct
{
  uint32_t DataTimeOut;         /*!< Specifies the data timeout period in card bus clock periods.  */
  uint32_t DataLength;          /*!< Specifies the number of data bytes to be transferred.         */
  uint32_t DataBlockSize;       /*!< Specifies the data block size for block transfer.
                                     This parameter can be a value of @ref SDMMC_LL_Data_Block_Size    */
  uint32_t TransferDir;         /*!< Specifies the data transfer direction, whether the transfer
                                     is a read or write.
                                     This parameter can be a value of @ref SDMMC_LL_Transfer_Direction */
  uint32_t TransferMode;        /*!< Specifies whether data transfer is in stream or block mode.
                                     This parameter can be a value of @ref SDMMC_LL_Transfer_Type      */
  uint32_t DPSM;                /*!< Specifies whether SDMMC Data path state machine (DPSM)
                                     is enabled or disabled.
                                     This parameter can be a value of @ref SDMMC_LL_DPSM_State         */
} SDMMC_DataInitTypeDef;

/**
* @def check Bus Width
*/
#define SDMMC_BUS_WIDE_1B                      0x00
#define SDMMC_BUS_WIDE_4B                      0x02

/**
  * @brief  Enable the SDMMC device.
  * @param  __INSTANCE__: SDMMC Instance
  * @retval None
  */
#define __SDMMC_ENABLE(__INSTANCE__)  ((__INSTANCE__)->CLKCR |= SDMMC_CLKCR_CLKEN)

/**
  * @brief  Disable the SDMMC device.
  * @param  __INSTANCE__: SDMMC Instance
  * @retval None
  */
#define __SDMMC_DISABLE(__INSTANCE__)  ((__INSTANCE__)->CLKCR &= ~SDMMC_CLKCR_CLKEN)

/**
  * @brief  Enable the SDMMC DMA transfer.
  * @param  __INSTANCE__: SDMMC Instance
  * @retval None
  */
#define __SDMMC_DMA_ENABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL |= SDMMC_DCTRL_DMAEN)
/**
  * @brief  Disable the SDMMC DMA transfer.
  * @param  __INSTANCE__: SDMMC Instance
  * @retval None
  */
#define __SDMMC_DMA_DISABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL &= ~SDMMC_DCTRL_DMAEN)

/**
  * @brief  Enable the SDMMC device interrupt.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @param  __INTERRUPT__ : specifies the SDMMC interrupt sources to be enabled.
  *         This parameter can be one or a combination of the following values:
  *            @arg SDMMC_IT_CCRCFAIL: Command response received (CRC check failed) interrupt
  *            @arg SDMMC_IT_DCRCFAIL: Data block sent/received (CRC check failed) interrupt
  *            @arg SDMMC_IT_CTIMEOUT: Command response timeout interrupt
  *            @arg SDMMC_IT_DTIMEOUT: Data timeout interrupt
  *            @arg SDMMC_IT_TXUNDERR: Transmit FIFO underrun error interrupt
  *            @arg SDMMC_IT_RXOVERR:  Received FIFO overrun error interrupt
  *            @arg SDMMC_IT_CMDREND:  Command response received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDSENT:  Command sent (no response required) interrupt
  *            @arg SDMMC_IT_DATAEND:  Data end (data counter, SDIDCOUNT, is zero) interrupt
  *            @arg SDMMC_IT_DBCKEND:  Data block sent/received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDACT:   Command transfer in progress interrupt
  *            @arg SDMMC_IT_TXACT:    Data transmit in progress interrupt
  *            @arg SDMMC_IT_RXACT:    Data receive in progress interrupt
  *            @arg SDMMC_IT_TXFIFOHE: Transmit FIFO Half Empty interrupt
  *            @arg SDMMC_IT_RXFIFOHF: Receive FIFO Half Full interrupt
  *            @arg SDMMC_IT_TXFIFOF:  Transmit FIFO full interrupt
  *            @arg SDMMC_IT_RXFIFOF:  Receive FIFO full interrupt
  *            @arg SDMMC_IT_TXFIFOE:  Transmit FIFO empty interrupt
  *            @arg SDMMC_IT_RXFIFOE:  Receive FIFO empty interrupt
  *            @arg SDMMC_IT_TXDAVL:   Data available in transmit FIFO interrupt
  *            @arg SDMMC_IT_RXDAVL:   Data available in receive FIFO interrupt
  *            @arg SDMMC_IT_SDIOIT:   SD I/O interrupt received interrupt
  * @retval None
  */
#define __SDMMC_ENABLE_IT(__INSTANCE__, __INTERRUPT__)  ((__INSTANCE__)->MASK |= (__INTERRUPT__))

/**
  * @brief  Disable the SDMMC device interrupt.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @param  __INTERRUPT__ : specifies the SDMMC interrupt sources to be disabled.
  *          This parameter can be one or a combination of the following values:
  *            @arg SDMMC_IT_CCRCFAIL: Command response received (CRC check failed) interrupt
  *            @arg SDMMC_IT_DCRCFAIL: Data block sent/received (CRC check failed) interrupt
  *            @arg SDMMC_IT_CTIMEOUT: Command response timeout interrupt
  *            @arg SDMMC_IT_DTIMEOUT: Data timeout interrupt
  *            @arg SDMMC_IT_TXUNDERR: Transmit FIFO underrun error interrupt
  *            @arg SDMMC_IT_RXOVERR:  Received FIFO overrun error interrupt
  *            @arg SDMMC_IT_CMDREND:  Command response received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDSENT:  Command sent (no response required) interrupt
  *            @arg SDMMC_IT_DATAEND:  Data end (data counter, SDIDCOUNT, is zero) interrupt
  *            @arg SDMMC_IT_DBCKEND:  Data block sent/received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDACT:   Command transfer in progress interrupt
  *            @arg SDMMC_IT_TXACT:    Data transmit in progress interrupt
  *            @arg SDMMC_IT_RXACT:    Data receive in progress interrupt
  *            @arg SDMMC_IT_TXFIFOHE: Transmit FIFO Half Empty interrupt
  *            @arg SDMMC_IT_RXFIFOHF: Receive FIFO Half Full interrupt
  *            @arg SDMMC_IT_TXFIFOF:  Transmit FIFO full interrupt
  *            @arg SDMMC_IT_RXFIFOF:  Receive FIFO full interrupt
  *            @arg SDMMC_IT_TXFIFOE:  Transmit FIFO empty interrupt
  *            @arg SDMMC_IT_RXFIFOE:  Receive FIFO empty interrupt
  *            @arg SDMMC_IT_TXDAVL:   Data available in transmit FIFO interrupt
  *            @arg SDMMC_IT_RXDAVL:   Data available in receive FIFO interrupt
  *            @arg SDMMC_IT_SDIOIT:   SD I/O interrupt received interrupt
  * @retval None
  */
#define __SDMMC_DISABLE_IT(__INSTANCE__, __INTERRUPT__)  ((__INSTANCE__)->MASK &= ~(__INTERRUPT__))

/**
  * @brief  Checks whether the specified SDMMC flag is set or not.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @param  __FLAG__: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg SDMMC_FLAG_CCRCFAIL: Command response received (CRC check failed)
  *            @arg SDMMC_FLAG_DCRCFAIL: Data block sent/received (CRC check failed)
  *            @arg SDMMC_FLAG_CTIMEOUT: Command response timeout
  *            @arg SDMMC_FLAG_DTIMEOUT: Data timeout
  *            @arg SDMMC_FLAG_TXUNDERR: Transmit FIFO underrun error
  *            @arg SDMMC_FLAG_RXOVERR:  Received FIFO overrun error
  *            @arg SDMMC_FLAG_CMDREND:  Command response received (CRC check passed)
  *            @arg SDMMC_FLAG_CMDSENT:  Command sent (no response required)
  *            @arg SDMMC_FLAG_DATAEND:  Data end (data counter, SDIDCOUNT, is zero)
  *            @arg SDMMC_FLAG_DBCKEND:  Data block sent/received (CRC check passed)
  *            @arg SDMMC_FLAG_CMDACT:   Command transfer in progress
  *            @arg SDMMC_FLAG_TXACT:    Data transmit in progress
  *            @arg SDMMC_FLAG_RXACT:    Data receive in progress
  *            @arg SDMMC_FLAG_TXFIFOHE: Transmit FIFO Half Empty
  *            @arg SDMMC_FLAG_RXFIFOHF: Receive FIFO Half Full
  *            @arg SDMMC_FLAG_TXFIFOF:  Transmit FIFO full
  *            @arg SDMMC_FLAG_RXFIFOF:  Receive FIFO full
  *            @arg SDMMC_FLAG_TXFIFOE:  Transmit FIFO empty
  *            @arg SDMMC_FLAG_RXFIFOE:  Receive FIFO empty
  *            @arg SDMMC_FLAG_TXDAVL:   Data available in transmit FIFO
  *            @arg SDMMC_FLAG_RXDAVL:   Data available in receive FIFO
  *            @arg SDMMC_FLAG_SDMMCIT:   SD I/O interrupt received
  * @retval The new state of SDMMC_FLAG (SET or RESET).
  */
#define __SDMMC_GET_FLAG(__INSTANCE__, __FLAG__)  (((__INSTANCE__)->STA &(__FLAG__)) != RESET)


/**
  * @brief  Clears the SDMMC pending flags.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @param  __FLAG__: specifies the flag to clear.
  *          This parameter can be one or a combination of the following values:
  *            @arg SDMMC_FLAG_CCRCFAIL: Command response received (CRC check failed)
  *            @arg SDMMC_FLAG_DCRCFAIL: Data block sent/received (CRC check failed)
  *            @arg SDMMC_FLAG_CTIMEOUT: Command response timeout
  *            @arg SDMMC_FLAG_DTIMEOUT: Data timeout
  *            @arg SDMMC_FLAG_TXUNDERR: Transmit FIFO underrun error
  *            @arg SDMMC_FLAG_RXOVERR:  Received FIFO overrun error
  *            @arg SDMMC_FLAG_CMDREND:  Command response received (CRC check passed)
  *            @arg SDMMC_FLAG_CMDSENT:  Command sent (no response required)
  *            @arg SDMMC_FLAG_DATAEND:  Data end (data counter, SDIDCOUNT, is zero)
  *            @arg SDMMC_FLAG_DBCKEND:  Data block sent/received (CRC check passed)
  *            @arg SDMMC_FLAG_SDMMCIT:   SD I/O interrupt received
  * @retval None
  */
#define __SDMMC_CLEAR_FLAG(__INSTANCE__, __FLAG__)  ((__INSTANCE__)->ICR = (__FLAG__))

/**
  * @brief  Checks whether the specified SDMMC interrupt has occurred or not.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @param  __INTERRUPT__: specifies the SDMMC interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg SDMMC_IT_CCRCFAIL: Command response received (CRC check failed) interrupt
  *            @arg SDMMC_IT_DCRCFAIL: Data block sent/received (CRC check failed) interrupt
  *            @arg SDMMC_IT_CTIMEOUT: Command response timeout interrupt
  *            @arg SDMMC_IT_DTIMEOUT: Data timeout interrupt
  *            @arg SDMMC_IT_TXUNDERR: Transmit FIFO underrun error interrupt
  *            @arg SDMMC_IT_RXOVERR:  Received FIFO overrun error interrupt
  *            @arg SDMMC_IT_CMDREND:  Command response received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDSENT:  Command sent (no response required) interrupt
  *            @arg SDMMC_IT_DATAEND:  Data end (data counter, SDIDCOUNT, is zero) interrupt
  *            @arg SDMMC_IT_DBCKEND:  Data block sent/received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDACT:   Command transfer in progress interrupt
  *            @arg SDMMC_IT_TXACT:    Data transmit in progress interrupt
  *            @arg SDMMC_IT_RXACT:    Data receive in progress interrupt
  *            @arg SDMMC_IT_TXFIFOHE: Transmit FIFO Half Empty interrupt
  *            @arg SDMMC_IT_RXFIFOHF: Receive FIFO Half Full interrupt
  *            @arg SDMMC_IT_TXFIFOF:  Transmit FIFO full interrupt
  *            @arg SDMMC_IT_RXFIFOF:  Receive FIFO full interrupt
  *            @arg SDMMC_IT_TXFIFOE:  Transmit FIFO empty interrupt
  *            @arg SDMMC_IT_RXFIFOE:  Receive FIFO empty interrupt
  *            @arg SDMMC_IT_TXDAVL:   Data available in transmit FIFO interrupt
  *            @arg SDMMC_IT_RXDAVL:   Data available in receive FIFO interrupt
  *            @arg SDMMC_IT_SDIOIT:   SD I/O interrupt received interrupt
  * @retval The new state of SDMMC_IT (SET or RESET).
  */
#define __SDMMC_GET_IT(__INSTANCE__, __INTERRUPT__)  (((__INSTANCE__)->STA &(__INTERRUPT__)) == (__INTERRUPT__))

/**
  * @brief  Clears the SDMMC's interrupt pending bits.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @param  __INTERRUPT__: specifies the interrupt pending bit to clear.
  *          This parameter can be one or a combination of the following values:
  *            @arg SDMMC_IT_CCRCFAIL: Command response received (CRC check failed) interrupt
  *            @arg SDMMC_IT_DCRCFAIL: Data block sent/received (CRC check failed) interrupt
  *            @arg SDMMC_IT_CTIMEOUT: Command response timeout interrupt
  *            @arg SDMMC_IT_DTIMEOUT: Data timeout interrupt
  *            @arg SDMMC_IT_TXUNDERR: Transmit FIFO underrun error interrupt
  *            @arg SDMMC_IT_RXOVERR:  Received FIFO overrun error interrupt
  *            @arg SDMMC_IT_CMDREND:  Command response received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDSENT:  Command sent (no response required) interrupt
  *            @arg SDMMC_IT_DATAEND:  Data end (data counter, SDMMC_DCOUNT, is zero) interrupt
  *            @arg SDMMC_IT_SDIOIT:   SD I/O interrupt received interrupt
  * @retval None
  */
#define __SDMMC_CLEAR_IT(__INSTANCE__, __INTERRUPT__)  ((__INSTANCE__)->ICR = (__INTERRUPT__))

/**
  * @brief  Enable Start the SD I/O Read Wait operation.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_START_READWAIT_ENABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL |= SDMMC_DCTRL_RWSTART)

/**
  * @brief  Disable Start the SD I/O Read Wait operations.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_START_READWAIT_DISABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL &= ~SDMMC_DCTRL_RWSTART)

/**
  * @brief  Enable Start the SD I/O Read Wait operation.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_STOP_READWAIT_ENABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL |= SDMMC_DCTRL_RWSTOP)

/**
  * @brief  Disable Stop the SD I/O Read Wait operations.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_STOP_READWAIT_DISABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL &= ~SDMMC_DCTRL_RWSTOP)

/**
  * @brief  Enable the SD I/O Mode Operation.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_OPERATION_ENABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL |= SDMMC_DCTRL_SDIOEN)

/**
  * @brief  Disable the SD I/O Mode Operation.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_OPERATION_DISABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL &= ~SDMMC_DCTRL_SDIOEN)

/**
  * @brief  Enable the SD I/O Suspend command sending.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_SUSPEND_CMD_ENABLE(__INSTANCE__)  ((__INSTANCE__)->CMD |= SDMMC_CMD_SDIOSUSPEND)

/**
  * @brief  Disable the SD I/O Suspend command sending.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_SUSPEND_CMD_DISABLE(__INSTANCE__)  ((__INSTANCE__)->CMD &= ~SDMMC_CMD_SDIOSUSPEND)


/* Initialization/de-initialization functions  **********************************/
SDMMC_Status  Core_SDMMC_Init(SDMMC_REG *SDMMCx, SDMMC_InitTypeDef Init);

/* I/O operation functions  *****************************************************/
/* Blocking mode: Polling */
uint32_t     Core_SDMMC_ReadFIFO(SDMMC_REG *SDMMCx);
SDMMC_Status Core_SDMMC_WriteFIFO(SDMMC_REG *SDMMCx, uint32_t *pWriteData);

/* Peripheral Control functions  ************************************************/
SDMMC_Status Core_SDMMC_PowerState_ON(SDMMC_REG *SDMMCx);
SDMMC_Status Core_SDMMC_PowerState_OFF(SDMMC_REG *SDMMCx);
uint32_t     Core_SDMMC_GetPowerState(SDMMC_REG *SDMMCx);

/* Command path state machine (CPSM) management functions */
SDMMC_Status Core_SDMMC_SendCommand(SDMMC_REG *SDMMCx, SDMMC_CmdInitTypeDef *Command);
uint32_t     Core_SDMMC_GetResponse(SDMMC_REG *SDMMCx, uint32_t Response);

/* Data path state machine (DPSM) management functions */
SDMMC_Status Core_SDMMC_DataConfig(SDMMC_REG *SDMMCx, SDMMC_DataInitTypeDef* Data);
uint32_t     Core_SDMMC_GetDataCounter(SDMMC_REG *SDMMCx);
uint32_t     Core_SDMMC_GetFIFOCount(SDMMC_REG *SDMMCx);


/* Power on/off switch for up to 16 cards */
#define   Core_SDMMC_SetPWREN(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->PWREN),temreg);
/* Bits used to mask unwanted interrupts */
#define   Core_SDMMC_SetINTMASK(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->INTMASK),temreg);
/* Writes to bits clear status bit */
#define   Core_SDMMC_SetRINTSTS(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->RINTSTS),temreg);
/* Enable global interrupt */
#define   Core_SDMMC_SetCTRL(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->CTRL),temreg);
/* Clock Divider Register*/
#define   Core_SDMMC_SetCLKDIV(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->CLKDIV),temreg);
/* Clock divider source for up to 16 SD cards supported */
#define   Core_SDMMC_SetCLKSRC(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->CLKSRC),temreg);
/* Clock Enable Register */
#define   Core_SDMMC_SetCLKENA(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->CLKENA),temreg);
/* Set value for card data read timeout*/
#define   Core_SDMMC_SetTMOUT(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->TMOUT),temreg);
/* Set card type */
#define   Core_SDMMC_SetCTYPE(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->CTYPE),temreg);
/* FIFO Threshold Watermark Register */
#define   Core_SDMMC_SetFIFOTH(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->FIFOTH),temreg);
/* Bus Mode Register */
#define   Core_SDMMC_SetBMOD(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->BMOD),temreg);
/* Descriptor List Base Address Register */
#define   Core_SDMMC_SetDBADDR(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->DBADDR),temreg);
/* internal DMAC Interrupt Enable Register*/
#define   Core_SDMMC_SetIDINTEN(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->IDINTEN),temreg);
#define   Core_SDMMC_SetUHSREG(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->UHSREG),temreg);
#define   Core_SDMMC_SetBLKSIZ(SDMMCx, temreg)  \
           write_reg32(&(SDMMCx->BLKSIZ),temreg);
#define   Core_SDMMC_SetBYCTNT(SDMMCx, temreg)  \
            write_reg32(&(SDMMCx->BYCTNT),temreg);
#define   Core_SDMMC_SetCMD(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->CMD),temreg);
#define   Core_SDMMC_SetCMDARG(SDMMCx, temreg) \
            write_reg32(&(SDMMCx->CMDARG),temreg);
#define   Core_SDMMC_GetCDETECT(SDMMCx) \
            read_reg32(&(SDMMCx->CDETECT));
#define   Core_SDMMC_GetRINTSTS(SDMMCx) \
            read_reg32(&(SDMMCx->RINTSTS));
#define   Core_SDMMC_GetSTATUS(SDMMCx)  \
            read_reg32(&(SDMMCx->STATUS));
#define   Core_SDMMC_GetCMD(SDMMCx) \
            read_reg32(&(SDMMCx->CMD));
#define   Core_SDMMC_GetRESP0(SDMMCx) \
            read_reg32(&(SDMMCx->RESP0));
#define   Core_SDMMC_GetRESP1(SDMMCx) \
            read_reg32(&(SDMMCx->RESP1));
#define   Core_SDMMC_GetRESP2(SDMMCx) \
            read_reg32(&(SDMMCx->RESP2));
#define   Core_SDMMC_GetRESP3(SDMMCx) \
            read_reg32(&(SDMMCx->RESP3));


#define   Core_SDMMC_WaiteCmdDone(SDMMCx) \
      do { \
        get_val = Core_SDMMC_GetRINTSTS(SDMMCx)  \
        cmd_done = (get_val & SDMMC_RINTSTS_CMD_DONE);  \
      } while (!cmd_done);
#define   Core_SDMMC_WaiteDataOver(SDMMCx) \
      do { \
        get_val = Core_SDMMC_GetRINTSTS(SDMMCx)  \
        data_over = (get_val & SDMMC_RINTSTS_DATA_OVER);  \
      } while (!data_over);
#define   Core_SDMMC_WaiteCardBusy(SDMMCx) \
      do { \
        get_val = Core_SDMMC_GetSTATUS(SDMMCx)  \
        card_busy = (get_val & SDMMC_STATUS_DATA_BUSY);  \
      } while (card_busy);
#define   Core_SDMMC_WaiteCmdStart(SDMMCx) \
      do { \
      get_val = Core_SDMMC_GetCMD(SDMMCx)  \
      cmd_start = (get_val & SDMMC_CMD_START_CMD); \
      } while (cmd_start);
#define   Core_SDMMC_WaiteVoltSwitchInt(SDMMCx)  \
      do { \
        get_val = Core_SDMMC_GetSTATUS(SDMMCx) \
        volt_switch_int = (get_val & SDMMC_RINTSTS_HTO); \
      } while (!volt_switch_int);


/* SDMMC Cards mode management functions */
SDMMC_Status Core_SDMMC_SetSDMMCReadWaitMode(SDMMC_REG *SDMMCx, uint32_t SDMMC_ReadWaitMode);


#ifdef __cplusplus
}
#endif


#endif /* __SD_CORE_H */
