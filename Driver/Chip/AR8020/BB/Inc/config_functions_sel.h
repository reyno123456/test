
#ifndef __CONFIG_FUNCTIONS_SEL_H
#define __CONFIG_FUNCTIONS_SEL_H

//**********************  sel the ground( rf & frq )  **********************

#define  GRD_RF8003_2P4
#define  SKY_RF8003_2P4

//**********************  sel the sky( rf & frq )   **************************
#define  POWER_GATE         (0x40)        //Reg[c5/c6] power #define  POWER_Thresholds.
#define  AAGC_GAIN_FAR      (0x16)
#define  AAGC_GAIN_NEAR     (0x30)

//*****************  define page1 or page2  ********************

 #define  PAGE_1                0x80
 #define  PAGE_2                0x7F
 #define  ID_SEARCH_MAX_TIMES   0x28
 #define  ID_MATCH_MAX_TIMES    0x05
 #define  RC_ID_BIT39_32        0x55     //Gound ID Values in Reg[03..07](high-->low).
 #define  RC_ID_BIT31_24        0xF0
 #define  RC_ID_BIT23_16        0x55
 #define  RC_ID_BIT15_08        0xF0
 #define  RC_ID_BIT07_00        0x55

 #define  SKY_RC_FRQ_INIT    0x01     //Sky Initial Freq Value.
 #define  SKY_ID_BIT39_32    0xFF     //Gound ID Values in Reg[03..07](high-->low).
 #define  SKY_ID_BIT31_24    0xFF
 #define  SKY_ID_BIT23_16    0xFF
 #define  SKY_ID_BIT15_08    0xFF
 #define  SKY_ID_BIT07_00    0xFF

//*****************  Define Snr Array Sizes   ********************

 #define SNRNUM_PER_CYC      0x08
 #define FRQ_SNR_BLOCK_ROWS  0x04
 #define FRQ_SNR_BLOCK_LISTS 0x09

 #define QAM_SNR_BLOCK_ROWS  0x20
 #define QAM_SNR_BLOCK_LISTS 0x09


 #define SPAN_ITUNLK         0x98
 #define SPAN_ALTFRQ         0x20     //span time of image Frq hopping.
 #define LDPC_STATIC_SPAN    0xA0
 #define LDPC_STATIC_NUM     0xA0

//*****************  define QAM mode threshold  ********************

#define BPSK1_2            0x004E              //  QAM mode threshold
#define QPSK1_2            0x0090
#define QPSK2_3            0x00BE
#define QAM16_1_2          0x01FD
#define QAM64_1_2          0x055E
#define QAM64_2_3          0x07EC

#define MCS3_0_THRE        0x009B            //  QAM mode threshold( +3dB )
#define MCS3_1_THRE        0x011E
#define MCS3_2_THRE        0x0179
#define MCS3_3_THRE        0x03F6
#define MCS3_4_THRE        0x0AAA
#define MCS3_5_THRE        0x0FC6


//communication with cy7c68013,sizeof FIFO(rows,lists)
#define RxFIFO68013_R        0x64      //rx from 68013
#define RxFIFO68013_L        0x05
#define TxFIFO68013_R        0x64      //tx to 68013
#define TxFIFO68013_L        0x04
#define RXMSG_SIZE           0x05
#define TXMSG_SIZE           0x04
#define  LINER_CONGRUENCE
// #define  FLASH_STOR_ENABLE
//


//define spi communication data
#define spiWrite             0x0E
#define spiRead              0x0F
#define Write_AR8001         0x01
#define Read_AR8001          0x02
#define Write_AD9363         0x03
#define Read_Ad9363          0x04
#define Write_AR8003         0x05
#define Read_AR8003          0x06
#define Wait                 0x11
#define SEL_SKY              0X00
#define SEL_GRD              0X01
#define Set_Write            0x01
#define Reset_Write          0x00

/**
  ==============================================================================
                    ##### ARLink PC special command define #####
  ==============================================================================
 */

#define PC_HEADOR        0x55    // Header order from pc.

#define PC_REGW          0x0E    // Write Register command from pc.
//#define PC_REGW_CKS
#define PC_REGR          0x0F    // Read Register command from pc.
//#define PC_REGR_CKS
#define PC_SIDEN         0x10    // Enable searching ID command from pc.
//#define PC_SIDEN_CKS   0x9B
#define PC_SIDDIS        0x11    // Disable searching ID command from pc.
//#define PC_SIDDIS_CKS  0x9A
#define PC_ITAFH         0x12    // Image transmission frq auto hopping mode.
#define PC_ITMFH         0x13    // Image transmission frq manual mode.
#define PC_ITMFC         0x14    // Image transmission frq manual channel from pc.
#define PC_RCFHP         0x15    // Remote controller frq hopping pattern sel from pc.
#define PC_RCWM          0x16    // Remote controller working mode sel from pc.
#define PC_ITWM          0x17    // Image transmission working mode sel from pc.
#define PC_EICLOSE       0x18    // Close external interrupt from pc.
//#define PC_EICLOSE_CKS 0x93
#define PC_EIOPEN        0x19    // Open external interrupt from pc.
//#define PC_EIOPEN_CKS  0x92
#define PC_DDCLOSE       0x1A    // Close dynamic display interface.
#define PC_WDOPEN        0x1B    // Open normal working display interface.
#define PC_FTDOPEN       0x1C    // Open field test display interface.
#define PC_UPFADD        0x1D    // Upload flash address.
#define PC_UPFDATA       0x1E    // Upload flash data.
#define PC_UPFDRCK       0x1F    // Upload flash data row check value.
#define PC_UPFDEND       0x20    // Upload flash end flag.
#define PC_UPFDBEG       0x21    // Upload flash begin flag.
#define PC_GSUART        0x22    // Upload grd tx sky through UARST.

#endif

