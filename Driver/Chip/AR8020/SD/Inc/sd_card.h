/**
  * @file    sd_card.h
  * @author  Minzhao
  * @version V1.0.0
  * @date    7-10-2016
  * @brief   Header file of sd card.
  *          This file contains:
  *           - Card define handle function and structure
  */

#ifndef __SD_CARD_H
#define __SD_CARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f746xx.h"
#include "sd_core.h"
#include "command.h"

/*
 * DMA data Structure definition
 */
typedef struct
{
	uint32_t DstAddr;         /* DstAddr: Pointer to the buffer that will contain the received data */
	uint32_t SrcAddr;         /* SrcAddr: Address from where data is to be read */
	uint32_t BlockSize;       /* BlockSize: SD card Data block size */
	uint32_t NumberOfBytes;  /*  NumberOfBytes: Number of bytes to be read/write. */
	uint32_t ListBaseAddr;
} SDMMC_DMATransTypeDef;
/*
 * SD Handle Structure definition
 */
typedef struct
{
	SDMMC_REG                    *Instance;        /*!< SDMMC register base address                     */
	uint32_t                     CardType;         /*!< SD card type                                   */
	uint32_t                     RCA;              /*!< SD relative card address                       */
	uint32_t                     CSD[4];           /*!< SD card specific data table                    */
	uint32_t                     CID[4];           /*!< SD card identification number table            */
	uint32_t               		 SdTransferCplt;   /*!< SD transfer complete flag in non blocking mode */
	uint32_t               		 SdTransferErr;    /*!< SD transfer error flag in non blocking mode    */
	uint32_t              		 SdOperation;      /*!< SD transfer operation (read/write)             */
	uint32_t                     Response;         /* SD response */

} SD_HandleTypeDef;


/** @defgroup SD_Exported_Types_Group2 Card Specific Data: CSD Register
  * @{
  */
typedef struct
{
	__IO uint8_t  CSDStruct;            /*!< CSD structure                         */
	__IO uint8_t  SysSpecVersion;       /*!< System specification version          */
	__IO uint8_t  Reserved1;            /*!< Reserved                              */
	__IO uint8_t  TAAC;                 /*!< Data read access time 1               */
	__IO uint8_t  NSAC;                 /*!< Data read access time 2 in CLK cycles */
	__IO uint8_t  MaxDataTransRate;        /*!< Max. bus clock frequency              */
	__IO uint16_t CardComdClasses;      /*!< Card command classes                  */
	__IO uint8_t  RdBlockLen;           /*!< Max. read data block length           */
	__IO uint8_t  PartBlockRead;        /*!< Partial blocks for read allowed       */
	__IO uint8_t  WrBlockMisalign;      /*!< Write block misalignment              */
	__IO uint8_t  RdBlockMisalign;      /*!< Read block misalignment               */
	__IO uint8_t  DSRImpl;              /*!< DSR implemented                       */
	__IO uint8_t  Reserved2;            /*!< Reserved                              */
	__IO uint32_t DeviceSize;           /*!< Device Size                           */
	__IO uint8_t  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min           */
	__IO uint8_t  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max           */
	__IO uint8_t  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min          */
	__IO uint8_t  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max          */
	__IO uint8_t  DeviceSizeMul;        /*!< Device size multiplier                */
	__IO uint8_t  EraseGrSize;          /*!< Erase group size                      */
	__IO uint8_t  EraseGrMul;           /*!< Erase group size multiplier           */
	__IO uint8_t  WrProtectGrSize;      /*!< Write protect group size              */
	__IO uint8_t  WrProtectGrEnable;    /*!< Write protect group enable            */
	__IO uint8_t  ManDeflECC;           /*!< Manufacturer default ECC              */
	__IO uint8_t  WrSpeedFact;          /*!< Write speed factor                    */
	__IO uint8_t  MaxWrBlockLen;        /*!< Max. write data block length          */
	__IO uint8_t  WriteBlockPaPartial;  /*!< Partial blocks for write allowed      */
	__IO uint8_t  Reserved3;            /*!< Reserved                              */
	__IO uint8_t  ContentProtectAppli;  /*!< Content protection application        */
	__IO uint8_t  FileFormatGrouop;     /*!< File format group                     */
	__IO uint8_t  CopyFlag;             /*!< Copy flag (OTP)                       */
	__IO uint8_t  PermWrProtect;        /*!< Permanent write protection            */
	__IO uint8_t  TempWrProtect;        /*!< Temporary write protection            */
	__IO uint8_t  FileFormat;           /*!< File format                           */
	__IO uint8_t  ECC;                  /*!< ECC code                              */
	__IO uint8_t  CSD_CRC;              /*!< CSD CRC                               */
	__IO uint8_t  Reserved4;            /*!< Always 1                              */

} SD_CSDTypedef;
/**
  * @}
  */

/** @defgroup SD_Exported_Types_Group3 Card Identification Data: CID Register
  * @{
  */
typedef struct
{
	__IO uint8_t  ManufacturerID;  /*!< Manufacturer ID       */
	__IO uint16_t OEM_AppliID;     /*!< OEM/Application ID    */
	__IO uint32_t ProdName1;       /*!< Product Name part1    */
	__IO uint8_t  ProdName2;       /*!< Product Name part2    */
	__IO uint8_t  ProdRev;         /*!< Product Revision      */
	__IO uint32_t ProdSN;          /*!< Product Serial Number */
	__IO uint8_t  Reserved1;       /*!< Reserved1             */
	__IO uint16_t ManufactDate;    /*!< Manufacturing Date    */
	__IO uint8_t  CID_CRC;         /*!< CID CRC               */
	__IO uint8_t  Reserved2;       /*!< Always 1              */

} SD_CIDTypedef;

/** @defgroup SD_Exported_Types_Group4 SD Card Status returned by ACMD13
  * @{
  */
typedef struct
{
	__IO uint8_t  DAT_BUS_WIDTH;           /*!< Shows the currently defined data bus width                 */
	__IO uint8_t  SECURED_MODE;            /*!< Card is in secured mode of operation                       */
	__IO uint16_t SD_CARD_TYPE;            /*!< Carries information about card type                        */
	__IO uint32_t SIZE_OF_PROTECTED_AREA;  /*!< Carries information about the capacity of protected area   */
	__IO uint8_t  SPEED_CLASS;             /*!< Carries information about the speed class of the card      */
	__IO uint8_t  PERFORMANCE_MOVE;        /*!< Carries information about the card's performance move      */
	__IO uint8_t  AU_SIZE;                 /*!< Carries information about the card's allocation unit size  */
	__IO uint16_t ERASE_SIZE;              /*!< Determines the number of AUs to be erased in one operation */
	__IO uint8_t  ERASE_TIMEOUT;           /*!< Determines the timeout for any number of AU erase          */
	__IO uint8_t  ERASE_OFFSET;            /*!< Carries information about the erase offset                 */

} SD_CardStatusTypedef;

/** @defgroup SD_Exported_Types_Group5 SD Card information structure
  * @{
  */
typedef struct
{
	SD_CSDTypedef       SD_csd;         /*!< SD card specific data register         */
	SD_CIDTypedef       SD_cid;         /*!< SD card identification number register */
	uint64_t            CardCapacity;   /*!< Card capacity                          */
	uint32_t            CardBlockSize;  /*!< Card block size                        */
	uint32_t            RCA;            /*!< SD relative card address               */
	uint32_t            CardType;       /*!< SD card type                           */

} SD_CardInfoTypedef;

//TODO
#define SDMMC_CMD0TIMEOUT                ((uint32_t)0x00010000)
/** @defgroup SD_Exported_Types_Group6 SD Error status enumeration Structure definition
  * @{
  */
typedef enum
{
	/**
	  * @brief  SD specific error defines
	  */
	SD_CMD_CRC_FAIL                    = (1),   /*!< Command response received (but CRC check failed)              */
	SD_DATA_CRC_FAIL                   = (2),   /*!< Data block sent/received (CRC check failed)                   */
	SD_CMD_RSP_TIMEOUT                 = (3),   /*!< Command response timeout                                      */
	SD_DATA_TIMEOUT                    = (4),   /*!< Data timeout                                                  */
	SD_TX_UNDERRUN                     = (5),   /*!< Transmit FIFO underrun                                        */
	SD_RX_OVERRUN                      = (6),   /*!< Receive FIFO overrun                                          */
	SD_START_BIT_ERR                   = (7),   /*!< Start bit not detected on all data signals in wide bus mode   */
	SD_OUT_OF_RANGE                    = (8),   /*!< Command's argument was out of range.                          */
	SD_ADDRESS_ERROR                      = (9),   /*!< Misaligned address                                            */
	SD_BLOCK_LEN_ERROR                 = (10),  /*!< Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
	SD_ERASE_SEQ_ERROR                 = (11),  /*!< An error in the sequence of erase command occurs.            */
	SD_ERASE_PARAM                     = (12),  /*!< An invalid selection for erase groups                        */
	SD_WP_VIOLATION                    = (13),  /*!< Attempt to program a write protect block                     */
	SD_CARD_IS_LOCKED                  = (34),  /*!< When set, signals that the card is locked by the host        */
	SD_LOCK_UNLOCK_FAILED              = (14),  /*!< Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
	SD_COM_CRC_ERROR                   = (15),  /*!< CRC check of the previous command failed                     */
	SD_ILLEGAL_COMMAND                 = (16),  /*!< Command is not legal for the card state                      */
	SD_CARD_ECC_FAILED                 = (17),  /*!< Card internal ECC was applied but failed to correct the data */
	SD_CC_ERROR                        = (18),  /*!< Internal card controller error                               */
	SD_GENERAL_UNKNOWN_ERROR                           = (19),  /*!< General or unknown error                                     */
	SD_STREAM_READ_UNDERRUN            = (20),  /*!< The card could not sustain data transfer in stream read operation. */
	SD_STREAM_WRITE_OVERRUN            = (21),  /*!< The card could not sustain data programming in stream mode   */
	SD_CSD_OVERWRITE                   = (22),  /*!< CSD overwrite error                                      */
	SD_WP_ERASE_SKIP                   = (23),  /*!< Only partial address space was erased                        */
	SD_CARD_ECC_DISABLED               = (24),  /*!< Command has been executed without using internal ECC         */
	SD_ERASE_RESET                     = (25),  /*!< Erase sequence was cleared before executing because an out of erase sequence command was received */
	SD_AKE_SEQ_ERROR                   = (26),  /*!< Error in sequence of authentication.                         */
	SD_INVALID_VOLTRANGE               = (27),
	SD_ADDR_OUT_OF_RANGE               = (28),
	SD_SWITCH_ERROR                    = (29),
	SD_SDMMC_DISABLED                  = (30),
	SD_SDMMC_FUNCTION_BUSY             = (31),
	SD_SDMMC_FUNCTION_FAILED           = (32),
	SD_SDMMC_UNKNOWN_FUNCTION          = (33),

	/**
	  * @brief  Standard error defines
	  */
	SD_INTERNAL_ERROR                  = (34),
	SD_NOT_CONFIGURED                  = (35),
	SD_REQUEST_PENDING                 = (36),
	SD_REQUEST_NOT_APPLICABLE          = (37),
	SD_INVALID_PARAMETER               = (38),
	SD_UNSUPPORTED_FEATURE             = (39),
	SD_UNSUPPORTED_HW                  = (40),
	SD_ERROR                           = (41),
	SD_NOTCARD                         = (42), /* sdcard is not detected */
	SD_UNSUPPORTED_VOLTAGE             = (43),
	SD_OK                              = (0)

} SD_ErrorTypedef;

/** @defgroup SD_Exported_Types_Group7 SD Transfer state enumeration structure
  * @{
  */
typedef enum
{
	SD_TRANSFER_OK    = 0,  /*!< Transfer success      */
	SD_TRANSFER_BUSY  = 1,  /*!< Transfer is occurring */
	SD_TRANSFER_ERROR = 2   /*!< Transfer failed       */

} SD_TransferStateTypedef;

/** @defgroup SD_Exported_Types_Group8 SD Card State enumeration structure
  * @{
  */
typedef enum
{
	SD_CARD_IDLE = 0,
	SD_CARD_READY,
	SD_CARD_IDENTIFICATION,
	SD_CARD_STANDBY,
	SD_CARD_TRANSFER,
	SD_CARD_DATA,
	SD_CARD_RECEIVE,
	SD_CARD_PROGRAMMING,
	SD_CARD_DISCONNECTED,
	SD_CARD_ERROR

} SD_CardStateTypedef;

/** @defgroup SD_Exported_Types_Group9 SD Operation enumeration structure
  * @{
  */
typedef enum
{
	SD_READ_SINGLE_BLOCK    = 0,  /*!< Read single block operation      */
	SD_READ_MULTIPLE_BLOCK  = 1,  /*!< Read multiple blocks operation   */
	SD_WRITE_SINGLE_BLOCK   = 2,  /*!< Write single block operation     */
	SD_WRITE_MULTIPLE_BLOCK = 3   /*!< Write multiple blocks operation  */

} SD_OperationTypedef;

/**
  * @brief SD Commands Index
  */
#define SD_CMD_GO_IDLE_STATE                       ((uint8_t)0)   /*!< Resets the SD memory card.                                                               */
#define SD_CMD_SEND_OP_COND                        ((uint8_t)1)   /*!< Sends host capacity support information and activates the card's initialization process. */
#define SD_CMD_ALL_SEND_CID                        ((uint8_t)2)   /*!< Asks any card connected to the host to send the CID numbers on the CMD line.             */
#define SD_CMD_SEND_RELATIVE_ADDR                  ((uint8_t)3)   /*!< Asks the card to publish a new relative address (RCA).                                   */
#define SD_CMD_SET_DSR                             ((uint8_t)4)   /*!< Programs the DSR of all cards.                                                           */
#define SD_CMD_IO_SEND_OP_COND                     ((uint8_t)5)   /*!< SDIO used to inquire about the voltage range needed by the I/O card*/
#define SD_CMD_HS_SWITCH                           ((uint8_t)6)   /*!< Checks switchable function (mode 0) and switch card function (mode 1).                   */
#define SD_CMD_SEL_DESEL_CARD                      ((uint8_t)7)   /*!< Selects the card by its own relative address and gets deselected by any other address    */
#define SD_CMD_SEND_IF_COND                        ((uint8_t)8)   /*!< Sends SD Memory Card interface condition, which includes host supply voltage information
                                                                       and asks the card whether card supports voltage.                                         */
#define SD_CMD_SEND_CSD                            ((uint8_t)9)   /*!< Addressed card sends its card specific data (CSD) on the CMD line.                       */
#define SD_CMD_SEND_CID                            ((uint8_t)10)  /*!< Addressed card sends its card identification (CID) on the CMD line.                      */
#define SD_CMD_VOLTAGE_SWITCH                      ((uint8_t)11)  /*!< SD card doesn't support it.                                                              */
#define SD_CMD_STOP_TRANSMISSION                   ((uint8_t)12)  /*!< Forces the card to stop transmission.                                                    */
#define SD_CMD_SEND_STATUS                         ((uint8_t)13)  /*!< Addressed card sends its status register.                                                */
#define SD_CMD_HS_BUSTEST_READ                     ((uint8_t)14)
#define SD_CMD_GO_INACTIVE_STATE                   ((uint8_t)15)  /*!< Sends an addressed card into the inactive state.                                         */
#define SD_CMD_SET_BLOCKLEN                        ((uint8_t)16)  /*!< Sets the block length (in bytes for SDSC) for all following block commands 
                                                                       (read, write, lock). Default block length is fixed to 512 Bytes. Not effective 
                                                                       for SDHS and SDXC.                                                                       */
#define SD_CMD_READ_SINGLE_BLOCK                   ((uint8_t)17)  /*!< Reads single block of size selected by SET_BLOCKLEN in case of SDSC, and a block of 
                                                                       fixed 512 bytes in case of SDHC and SDXC.                                                */
#define SD_CMD_READ_MULTIPLE_BLOCK                 ((uint8_t)18)  /*!< Continuously transfers data blocks from card to host until interrupted by 
                                                                       STOP_TRANSMISSION command.                                                               */
#define SD_CMD_SEND_TUNING_PATTERN                 ((uint8_t)19)  /*!< 64 bytes tuning pattern is sent for SDR50 and SDR104.                                    */
#define SD_CMD_SPEED_CLASS_CONTROL                 ((uint8_t)20)  /*!< Speed class control command.                                                             */
#define SD_CMD_SET_BLOCK_COUNT                     ((uint8_t)23)  /*!< Specify block count for CMD18 and CMD25.                                                 */
#define SD_CMD_WRITE_SINGLE_BLOCK                  ((uint8_t)24)  /*!< Writes single block of size selected by SET_BLOCKLEN in case of SDSC, and a block of 
                                                                       fixed 512 bytes in case of SDHC and SDXC.                                                */
#define SD_CMD_WRITE_MULTIPLE_BLOCK                ((uint8_t)25)  /*!< Continuously writes blocks of data until a STOP_TRANSMISSION follows.                    */
#define SD_CMD_PROG_CID                            ((uint8_t)26)  /*!< Reserved for manufacturers.                                                              */
#define SD_CMD_PROGRAM_CSD                         ((uint8_t)27)  /*!< Programming of the programmable bits of the CSD.                                         */
#define SD_CMD_SET_WRITE_PROT                      ((uint8_t)28)  /*!< Sets the write protection bit of the addressed group.                                    */
#define SD_CMD_CLR_WRITE_PROT                      ((uint8_t)29)  /*!< Clears the write protection bit of the addressed group.                                  */
#define SD_CMD_SEND_WRITE_PROT                     ((uint8_t)30)  /*!< Asks the card to send the status of the write protection bits.                           */
#define SD_CMD_SD_ERASE_WR_BLK_START               ((uint8_t)32)  /*!< Sets the address of the first write block to be erased. (For SD card only).              */
#define SD_CMD_SD_ERASE_WR_BLK_END                 ((uint8_t)33)  /*!< Sets the address of the last write block of the continuous range to be erased.           */
#define SD_CMD_ERASE_GRP_START                     ((uint8_t)35)  /*!< Sets the address of the first write block to be erased. Reserved for each command 
                                                                       system set by switch function command (CMD6).                                            */
#define SD_CMD_ERASE_GRP_END                       ((uint8_t)36)  /*!< Sets the address of the last write block of the continuous range to be erased. 
                                                                       Reserved for each command system set by switch function command (CMD6).                  */
#define SD_CMD_ERASE                               ((uint8_t)38)  /*!< Reserved for SD security applications.                                                   */
#define SD_CMD_FAST_IO                             ((uint8_t)39)  /*!< SD card doesn't support it (Reserved).                                                   */
#define SD_CMD_GO_IRQ_STATE                        ((uint8_t)40)  /*!< SD card doesn't support it (Reserved).                                                   */
#define SD_CMD_LOCK_UNLOCK                         ((uint8_t)42)  /*!< Sets/resets the password or lock/unlock the card. The size of the data block is set by 
                                                                       the SET_BLOCK_LEN command.                                                               */
#define SD_CMD_APP_CMD                             ((uint8_t)55)  /*!< Indicates to the card that the next command is an application specific command rather 
                                                                       than a standard command.                                                                 */
#define SD_CMD_GEN_CMD                             ((uint8_t)56)  /*!< Used either to transfer a data block to the card or to get a data block from the card 
                                                                       for general purpose/application specific commands.                                       */
#define SD_CMD_NO_CMD                              ((uint8_t)64)

/**
  * @brief Following commands are SD Card Specific commands.
  *        SDMMC_APP_CMD should be sent before sending these commands.
  */
#define SD_CMD_APP_SD_SET_BUSWIDTH                 ((uint8_t)6)   /*!< (ACMD6) Defines the data bus width to be used for data transfer. The allowed data bus 
                                                                       widths are given in SCR register.                                                          */
#define SD_CMD_SD_APP_STATUS                       ((uint8_t)13)  /*!< (ACMD13) Sends the SD status.                                                              */
#define SD_CMD_SD_APP_SEND_NUM_WR_BLOCKS           ((uint8_t)22)  /*!< (ACMD22) Sends the number of the written (without errors) write blocks. Responds with 
                                                                       32bit+CRC data block.                                                                      */
#define SD_CMD_APP_SET_WR_BLK_ERASE_COUNT          ((uint8_t)23)  /*   Set the number of write blocks to be pre-erased before writing*/
#define SD_CMD_APP_SD_SEND_OP_COND                 ((uint8_t)41)  /*!< (ACMD41) Sends host capacity support information (HCS) and asks the accessed card to 
                                                                       send its operating condition register (OCR) content in the response on the CMD line.       */
#define SD_CMD_SD_APP_SET_CLR_CARD_DETECT          ((uint8_t)42)  /*!< (ACMD42) Connects/Disconnects the 50 KOhm pull-up resistor on CD/DAT3 (pin 1) of the card. */
#define SD_CMD_SD_APP_SEND_SCR                     ((uint8_t)51)  /*!< Reads the SD Configuration Register (SCR).                                                 */
#define SD_CMD_SDMMC_RW_DIRECT                     ((uint8_t)52)  /*!< For SD I/O card only, reserved for security specification.                                 */
#define SD_CMD_SDMMC_RW_EXTENDED                   ((uint8_t)53)  /*!< For SD I/O card only, reserved for security specification.                                 */

/**
  * @brief Following commands are SD Card Specific security commands.
  *        SD_CMD_APP_CMD should be sent before sending these commands.
  */
#define SD_CMD_SD_APP_GET_MKB                      ((uint8_t)43)  /*!< For SD card only */
#define SD_CMD_SD_APP_GET_MID                      ((uint8_t)44)  /*!< For SD card only */
#define SD_CMD_SD_APP_SET_CER_RN1                  ((uint8_t)45)  /*!< For SD card only */
#define SD_CMD_SD_APP_GET_CER_RN2                  ((uint8_t)46)  /*!< For SD card only */
#define SD_CMD_SD_APP_SET_CER_RES2                 ((uint8_t)47)  /*!< For SD card only */
#define SD_CMD_SD_APP_GET_CER_RES1                 ((uint8_t)48)  /*!< For SD card only */
#define SD_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK   ((uint8_t)18)  /*!< For SD card only */
#define SD_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  ((uint8_t)25)  /*!< For SD card only */
#define SD_CMD_SD_APP_SECURE_ERASE                 ((uint8_t)38)  /*!< For SD card only */
#define SD_CMD_SD_APP_CHANGE_SECURE_AREA           ((uint8_t)49)  /*!< For SD card only */
#define SD_CMD_SD_APP_SECURE_WRITE_MKB             ((uint8_t)48)  /*!< For SD card only */

/**
  * @brief Supported SD Memory Cards
  */
#define STD_CAPACITY_SD_CARD_V1_1             ((uint32_t)0x00000000)
#define STD_CAPACITY_SD_CARD_V2_0             ((uint32_t)0x00000001)
#define HIGH_CAPACITY_SD_CARD                 ((uint32_t)0x00000002)
#define MULTIMEDIA_CARD                       ((uint32_t)0x00000003)
#define SECURE_DIGITAL_IO_CARD                ((uint32_t)0x00000004)
#define HIGH_SPEED_MULTIMEDIA_CARD            ((uint32_t)0x00000005)
#define SECURE_DIGITAL_IO_COMBO_CARD          ((uint32_t)0x00000006)
#define HIGH_CAPACITY_MMC_CARD                ((uint32_t)0x00000007)

/* 512 bytes */
#define DATA_BLOCK_LEN          ((uint32_t)0x00000040)
#define DATA_BYTE_CNT			 ((uint32_t)0x00000040)
/**
  * @brief  Mask for ACMD41 Argument
  */
#define SD_ACMD41_BUSY                      BIT(31)
#define SD_ACMD41_HCS                       BIT(30)
#define SD_ACMD41_XPC                       BIT(28)
#define SD_ACMD41_S18R                      BIT(24)
/**
  * @brief  Mask for errors Card Status R3 (OCR Register)
  */
#define SD_R3_BUSY                          BIT(31)
#define SD_R3_CCS                           BIT(30)
#define SD_R3_S18A                          BIT(24)

/**TODO
  * @brief  Masks for R6 Response
  */
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)

#define SD_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY                ((uint32_t)0x40000000)
#define SD_STD_CAPACITY                 ((uint32_t)0x00000000)
#define SD_CHECK_PATTERN                ((uint32_t)0x000001AA)

#define SD_MAX_VOLT_TRIAL               ((uint32_t)0x0000FFFF)
#define SD_ALLZERO                      ((uint32_t)0x00000000)

#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)

#define SD_DATATIMEOUT                  ((uint32_t)0xFFFFFFFF)
#define SD_0TO7BITS                     ((uint32_t)0x000000FF)
#define SD_8TO15BITS                    ((uint32_t)0x0000FF00)
#define SD_16TO23BITS                   ((uint32_t)0x00FF0000)
#define SD_24TO31BITS                   ((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFF)

#define SD_HALFFIFO                     ((uint32_t)0x00000008)
#define SD_HALFFIFOBYTES                ((uint32_t)0x00000020)

/**
  * @brief  Command Class Supported
  */
#define SD_CCCC_LOCK_UNLOCK             ((uint32_t)0x00000080)
#define SD_CCCC_WRITE_PROT              ((uint32_t)0x00000040)
#define SD_CCCC_ERASE                   ((uint32_t)0x00000020)


/**
  * @def check Command Index
  */
#define IS_SDMMC_CMD_INDEX(INDEX)       ((INDEX) < 0x40)
/**
  * check the response is long or short
  */
#define IS_SDMMC_RESPONSE(RESPONSE)     (((RESPONSE) == SDMMC_RESPONSE_NO)    || \
                                        ((RESPONSE)  == SDMMC_RESPONSE_R1)  || \
                                        ((RESPONSE)  == SDMMC_RESPONSE_R2)   || \
                                        ((RESPONSE)  == SDMMC_RESPONSE_R3)   || \
                                        ((RESPONSE)  == SDMMC_RESPONSE_R6)   || \
                                        ((RESPONSE)  == SDMMC_RESPONSE_R7))


#define IS_SDMMC_RESP(RESP)             (((RESP) == SDMMC_RESP0) || \
		                                ((RESP) == SDMMC_RESP1)  || \
		                                ((RESP) == SDMMC_RESP2)  || \
		                                ((RESP) == SDMMC_RESP3))


/** @defgroup SD_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */
SD_ErrorTypedef Card_SD_Init(SD_HandleTypeDef *hsd, SD_CardInfoTypedef *SDCardInfo);
SD_ErrorTypedef Card_SD_DeInit (SD_HandleTypeDef *hsd);


/** @defgroup SD_Exported_Functions_Group2 Input and Output operation functions
  * @{
  */
/* Blocking mode: Polling */
SD_ErrorTypedef Card_SD_ReadBlocks(SD_HandleTypeDef *hsd, uint32_t *pReadBuffer, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumberOfBlocks);
SD_ErrorTypedef Card_SD_WriteBlocks(SD_HandleTypeDef *hsd, uint32_t *pWriteBuffer, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumberOfBlocks);
SD_ErrorTypedef Card_SD_Erase(SD_HandleTypeDef *hsd, uint32_t startaddr, uint32_t endaddr);
/* Non-Blocking mode: Interrupt */
void Card_SD_IRQHandler(SD_HandleTypeDef *hsd);

/* Callback in non blocking modes (DMA) */
/*void Card_SD_DMA_RxCpltCallback(DMA_HandleTypeDef *hdma);
void Card_SD_DMA_RxErrorCallback(DMA_HandleTypeDef *hdma);
void Card_SD_DMA_TxCpltCallback(DMA_HandleTypeDef *hdma);
void Card_SD_DMA_TxErrorCallback(DMA_HandleTypeDef *hdma);*/
void Card_SD_XferCpltCallback(SD_HandleTypeDef *hsd);
void Card_SD_XferErrorCallback(SD_HandleTypeDef *hsd);

/* Non-Blocking mode: DMA */
SD_ErrorTypedef Card_SD_ReadBlocks_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma);
SD_ErrorTypedef Card_SD_WriteBlocks_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma);
SD_ErrorTypedef Card_SD_CheckWriteOperation(SD_HandleTypeDef *hsd, uint32_t Timeout);
SD_ErrorTypedef Card_SD_CheckReadOperation(SD_HandleTypeDef *hsd, uint32_t Timeout);

/** @defgroup SD_Exported_Functions_Group3 Peripheral Control functions
  * @{
  */
SD_ErrorTypedef Card_SD_Get_CardInfo(SD_HandleTypeDef *hsd, SD_CardInfoTypedef *pCardInfo);
SD_ErrorTypedef Card_SD_WideBusOperation_Config(SD_HandleTypeDef *hsd, uint32_t WideMode);
SD_ErrorTypedef Card_SD_StopTransfer(SD_HandleTypeDef *hsd);
SD_ErrorTypedef Card_SD_HighSpeed (SD_HandleTypeDef *hsd);

/* Peripheral State functions  ************************************************/
/** @defgroup SD_Exported_Functions_Group4 Peripheral State functions
  * @{
  */
SD_ErrorTypedef Card_SD_SendSDStatus(SD_HandleTypeDef *hsd, uint32_t *pSDstatus);
//SD_ErrorTypedef Card_SD_GetCardStatus(SD_HandleTypeDef *hsd, HAL_SD_CardStatusTypedef *pCardStatus);
SD_TransferStateTypedef Card_SD_GetStatus(SD_HandleTypeDef *hsd);



#ifdef __cplusplus
extern
}
#endif
#endif
