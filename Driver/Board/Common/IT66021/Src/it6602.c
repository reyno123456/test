///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <it6602.c>
//   @author Max.Kao@ite.com.tw
//   @date   2014/09/11
//   @fileversion: ITE_MHLRX_SAMPLE_V1.01
//******************************************/

//FIX_ID_001		//Dr. Liu suggestion to enable Auto EQ with Manual EQ to avoid some special HDMI cable issue.
//FIX_ID_002		//Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
//FIX_ID_003		//Add IT6602 Video Output Configure setting
//FIX_ID_004		//Add 100ms calibration for Cbus
//FIX_ID_005		//Add Cbus Event Handler
//FIX_ID_006		//Add P2_0 for switch Exteranl 24c04 EEPROM and Internal IT6602 EDID RAM
//FIX_ID_007		//for debug IT6681 HDCP issue7
//FIX_ID_008		//Add SW reset when HDMI / MHL device un-plug  !!!
//FIX_ID_009		//Verify interrupt event with reg51[0] select port
//FIX_ID_010		//Add JudgeBestEQ to avoid wrong EQ setting
//FIX_ID_011		//Use FW send PATH_EN{Sink}=1
//FIX_ID_012		//For SamSung Galaxy Note wake up fail iss ue
//FIX_ID_013		//For MSC 3D request issue
//FIX_ID_014		//For HDCP Auth Start with EQ Adjust issue
//FIX_ID_015		//Add RCP timeout mechanism for peer device no RCPK or RCPE response
//FIX_ID_016		//Support Dual Pixel Mode for IT66023 Only
//FIX_ID_017		//Disable IPLockChk
//FIX_ID_018		//modify 1K pull-down to 1.033K ohm HDMI Reg1C0[3:2]=2
//FIX_ID_019		//modify ENHYS control for MHL mode
//FIX_ID_020		//Turn off DEQ for HDMI port 1 with 20m DVI Cable
//FIX_ID_021		//To use CP_100ms calibration for CBus and CEC
//FIX_ID_022		//Fixed for CEC capacitor issue
//FIX_ID_023		//Fixed for Audio Channel Status Error with invalid HDMI source
//FIX_ID_024		//Fixed for RCP compliance issue
//FIX_ID_025		//Adjust H/W Mute time
//FIX_ID_026		//Support RB SWAP for TTL output
//FIX_ID_027		//Support RGB limited / Full range convert
//FIX_ID_028		//For Debug Audio error with S2
//FIX_ID_029		//fixed Ulta-2000 HDCP fail issue at Receiver mode
//FIX_ID_030		//fixed video lost at 640x480 timing
//FIX_ID_031		//change CBus pull down 1K after 75ms power 5V stable
//FIX_ID_032		//Support HDCP Repeater function for HDMI Tx device
//FIX_ID_033		//Fine-tune EQ Adjust function for HDCP receiver and repeater mode
//FIX_ID_034		//Add MHL HPD Control by it6602HPDCtrl( )
//FIX_ID_035		//For MTK6592 HDMI to SII MHL TX compliance issue
//FIX_ID_036		//Enable MHL Function for IT68XX
//FIX_ID_037		//Allion MHL compliance issue !!!
//FIX_ID_039		//fix image flick when enable RGB limited / Full range convert
//FIX_ID_041		//Add EDID reset
//FIX_ID_042		//Disable HDCP 1.1 feature to avoid compilance issue from ilegal HDCP 1.1 source device

// 2014-0527 MHL compliance issue !!!
// REG_RX_1C0 : 0x80 [7] PWSB_LV =0 , [3:2] 1K resistance
// REG_RX_1BB : 0x0A [4] Cbus Schimitter trigger =0
//
//
// MHL_RX_01 : 0xD8 [6:4] OSC division = 5 , [3:2]
// MHL_RX_29 : 0x83 [7] Enable Crystall
// MHL_RX_2A : 0x41 [2:1] 5vStableTime2CDsenseGoHigh = 00 75ms
// MHL_RX_2B : 0x1A [2] output HPD when wakeup fail trigger, [0] wkeup fail tigger by FW
//
//
// REG_RX_1C0 = 0x04 	//2014-0527 +10% for W1070
// MHL_RX_2B = 0x1A
//
/*****************************************************************************/
/* Header Files Included *****************************************************/
/*****************************************************************************/

#if 0

#ifndef _MCU_
#define _MCU_
#endif

#endif


#ifndef _ITEHDMI_
#define _ITEHDMI_
#endif

#ifdef _MCU_
#define _CODE code
#include "it6602.h"
#include "it6602_reg.h"
#include "typedef.h"
#include "Utility.h"

#else
//#define _CODE const
#define _CODE
#include "config.h"
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "it_66021.h"
#include "it_define.h"
#include "it6602_reg.h"
#include "it_typedef.h"
#include "Utility.h"
#include "debuglog.h"
#include "i2c.h"
#include "systicks.h"
#include "it6602.h"
#include "debug.h"

#endif

#define IT_DEBUGE

#ifdef IT_DEBUGE
#define DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define DLOG_INFO(...)
#endif


//FIX_ID_016 xxxxx Support Dual Pixel Mode for IT66023 Only
#if defined(_IT66023_)
#pragma message ("defined ENABLE_IT66023")
#endif
//FIX_ID_016 xxxxx

/*****************************************************************************/
/* Local Defines    **********************************************************/
/*****************************************************************************/
//#define DISABLE_HDMI_CSC
#define Enable_Vendor_Specific_packet
//#define EN_DUAL_PIXEL_MODE	//2013-0520


//FIX_ID_003 xxxxx	//Add IT6602 Video Output Configure setting
// 0 eRGB444_SDR=0,
// 1	eYUV444_SDR,
// 2	eRGB444_DDR,
// 3	eYUV444_DDR,
// 4	eYUV422_Emb_Sync_SDR,
// 5	eYUV422_Emb_Sync_DDR,
// 6	eYUV422_Sep_Sync_SDR,
// 7	eYUV422_Sep_Sync_DDR,
// 8	eCCIR656_Emb_Sync_SDR,
// 9	eCCIR656_Emb_Sync_DDR,
// 10 eCCIR656_Sep_Sync_SDR,
// 11 eCCIR656_Sep_Sync_DDR,
// 12 eRGB444_Half_Bus,
// 13 eYUV444_Half_Bus,
// 14 eBTA1004_SDR,
// 15 eBTA1004_DDR
//06-27 disable --> #define HDMIRX_OUTPUT_VID_MODE (F_MODE_EN_UDFILT | F_MODE_RGB444)
#define HDMIRX_OUTPUT_VID_MODE eYUV422_Sep_Sync_SDR
//FIX_ID_003 xxxxx

#define MS_TimeOut(x) (x+1)

//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
#define VSATE_CONFIRM_SCDT_COUNT        MS_TimeOut(100)
//FIX_ID_033 xxxxx

#ifdef _FIX_ID_028_
//xxxxx 2014-0417
//FIX_ID_028 xxxxx //For Debug Audio error with S2
#define AUDIO_READY_TIMEOUT                 	MS_TimeOut(0)	// change 100ms to 0 for speed up audio on
//FIX_ID_028 xxxxx
//xxxxx 2014-0417
#else
//FIX_ID_023 xxxxx		//Fixed for Audio Channel Status Error with invalid HDMI source
#define AUDIO_READY_TIMEOUT                 	MS_TimeOut(200)
//FIX_ID_023 xxxxx
#endif
#define AUDIO_MONITOR_TIMEOUT              MS_TimeOut(150)

#define SCDT_OFF_TIMEOUT              		MS_TimeOut(20)		//100 x MS_LOOP = 5000 ms = 5 sec
#define ECC_TIMEOUT              				MS_TimeOut(20)
#define DESKEW_TIMEOUT            			MS_TimeOut(20)



// Debug Mode
#define EnCBusDbgMode  	FALSE
#define MSCCBusDbgCtrl 	TRUE
#define  DDCCBusDbgCtrl 	FALSE
#define  RCLKFreqSel 	1	//; //0: RING/2 ; 1: RING/4 ; 2: RING/8 ; 3: RING/16
#define GenPktRecType	0x81
#define PPHDCPOpt	TRUE	//2013-0509 MHL 1080p packet pixel mode HDCP

#ifndef IT6811B0
#define PPHDCPOpt2	TRUE	//2013-0509 MHL 1080p packet pixel mode HDCP
#else
#define PPHDCPOpt2	FALSE 	//only for it6811b0
#endif

//FIX_ID_021 xxxxx		//To use CP_100ms for CBus_100ms and CEC_100m
//FIX_ID_004 xxxxx //Add 100ms calibration for Cbus
//#ifdef _SelectExtCrystalForCbus_
#define T10usSrcSel   TRUE	//FALSE: 100ms calibration , TRUR: 27MHz Crystal(only IT6602)
//#else
//#define T10usSrcSel   FALSE	 //FALSE: 100ms calibration , TRUR: 27MHz Crystal(only IT6602)
//#endif
//FIX_ID_004 xxxxx
//FIX_ID_021 xxxxx

#define EnMSCBurstWr	TRUE
#define MSCBurstWrID	TRUE   // TRUE: from MHL5E/MHL5F
#define MSCBurstWrOpt	FALSE  // TRUE: Not write Adopter ID unsigned char o ScratchPad
#define EnPktFIFOBurst	TRUE
// DDC Option
#define EnDDCSendAbort	TRUE  // Send ABORT after segment write with EOF
//CBUS Capability
#define MHLVersion	0x20
#define PLIM	1
#define POW	1
#define DEV_TYPE_SINK	1 //06-26
#define DEV_TYPE	1
#define ADOPTER_ID_H	0x02
#define ADOPTER_ID_L	0x45
#define DEVICE_ID_H		0x68
#define DEVICE_ID_L		0x02
#define AckHigh	0xB
#define AckLow	1
// CBUS INput Option
#define EnCBusDeGlitch	TRUE
//---------------------//
//----- WatchDog -----//
//--------------------//
#define DeltaNum 	1
#define RegBurstWrTOSel	2 // 2	//0: 320ms, 1: 340ms, 2: 360ms (ATC)
#define Reg100msTOAdj	2 // 2	//00: 100ms, 01: 99ms, 10: 101ms (ATC)
#define EnMSCHwRty	FALSE
#define EnHWPathEn	FALSE
#define MSCRxUCP2Nack	TRUE

/////////////////////////////////////////
//Cbus command fire wait time
//Maxmun time for determin CBUS fail
//	CBUSWAITTIME(ms) x CBUSWAITNUM
/////////////////////////////////////////
//FIX_ID_024 xxxxx	//Fixed for RCP compliance issue
#define CBUSWAITTIME    1
#define CBUSWAITNUM     100
//FIX_ID_024	xxxxx



#define  HDCPIntKey   FALSE   //TRUE: Internal HDCP Key, FALSE: SIPROM

#define  VCLK_INV	0
#define  VCLK_DLY	0
#define  EnMultiSeg     TRUE
#define  EnIntEDID      TRUE

//Discovery
#define  CBUSFloatAdj	FALSE
#define EQFAILCNT 2



//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
#define EQRETRYFAILCNT 1	// for EQ interrupt
#define RCLKVALUE 12			// for show TMDS and Pixel Clk
#define TMDSCLKVALUE 160	// for TMDS > 160 then set RS to 00, otherwise set to 3F

#define TMDSCLKVALUE_1080P 160	// for TMDS > 160 then set RS to 00, otherwise set to 3F
#define TMDSCLKVALUE_480P 35
#define TMDSCLKVALUE_MHL_ER1 90
#define JUDGE_ER1_VALUE 90

//FIX_ID_001 xxxxx


//FIX_ID_021 xxxxx		//To use CP_100ms for CBus_100ms and CEC_100m
//FIX_ID_004 xxxxx 		//Add 100ms calibration for Cbus
//#ifndef _SelectExtCrystalForCbus_
#define _RCLK_FREQ_20M  FALSE
//#endif
//FIX_ID_004 xxxxx
//FIX_ID_021 xxxxx


//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
//FIX_ID_005 xxxx	//Wait for video on then read MHL device capability
#define MAX_CBUS_WAITNO 		(300/MS_LOOP)		// 250ms
#define MAX_PATHEN_WAITNO 	(700/MS_LOOP)		// 700ms
#define MAX_BUSY_WAITNO 		(2500/MS_LOOP)		// 150ms
#define MAX_DISCOVERY_WAITNO 	(100/MS_LOOP)		// 100ms
//FIX_ID_005 xxxx
//FIX_ID_033 xxxxx
//FIX_ID_037 xxxxx

//FIX_ID_014 xxxx
#define MAX_TMDS_WAITNO 		(350/MS_LOOP)		// 400ms
#define MAX_HDCP_WAITNO 		(100/MS_LOOP)		// 150ms
//FIX_ID_014 xxxx
//FIX_ID_018	xxxxx	//modify 1K pull-down to 1.033K ohm HDMI Reg1C0[3:2]=2
#define RENEW_WAKEUP		(12000/MS_LOOP)
#define IGNORE_WAKEUP		(1000/MS_LOOP)
#define TOGGLE_WAKEUP		(4000/MS_LOOP)
#define CDSENSE_WAKEUP		(500/MS_LOOP)
//FIX_ID_018	xxxxx

#define DEFAULT_EQVALUE 0x1F

/*****************************************************************************/
/* Private and Local Variables    ********************************************/
/*****************************************************************************/
#if 1

struct it6602_dev_data it6602DEV;

unsigned char  V3D_EntryCnt = 0;
unsigned char  wrburstoff, wrburstnum;
unsigned char  TxWrBstSeq = 0;

//FIX_ID_013	xxxxx	//For Acer MHL Dongle MSC 3D request issue
//unsigned char  EnMSCWrBurst3D  = TRUE;
//unsigned char  EnMHL3DSupport  = FALSE;
//FIX_ID_013	xxxxx






unsigned char  wakeupcnt = 0;
#define MinEQValue	0x03

//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
#ifdef _SUPPORT_EQ_ADJUST_
#define MaxEQIndex 3


unsigned char IT6602EQTable[]={0xFF,0x9F,0x83};


//for EQ state machine handler
//#define	MAXSYNCOFF 		5
#define	MAXECCWAIT 		(10)
#define	EQSTATE_WAIT		(20)
#define	EQSTATE_START		(EQSTATE_WAIT+MAXECCWAIT)
#define	EQSTATE_LOW		(EQSTATE_WAIT+EQSTATE_START+(MAXECCWAIT*1))
#define	EQSTATE_MIDDLE	(EQSTATE_WAIT+EQSTATE_START+(MAXECCWAIT*2))
#define	EQSTATE_HIGH		(EQSTATE_WAIT+EQSTATE_START+(MAXECCWAIT*3))
#define	EQSTATE_END		(255-100)
#define	MINECCFAILCOUNT 	(MAXECCWAIT/2)
#endif

#ifdef _SUPPORT_AUTO_EQ_
unsigned char ucPortAMPOverWrite[2];
unsigned char ucPortAMPValid[2];
unsigned char ucChannelB[2];	// ch0
unsigned char ucChannelG[2];	// ch1
unsigned char ucChannelR[2];	// ch2
unsigned char ucEQMode[2];
#endif

//FIX_ID_035 xxxxx //For MTK6592 HDMI to SII MHL TX compliance issue
//xxxxx 2014-0508 disable -> unsigned char ucEqRetryCnt[2];
//FIX_ID_035 xxxxx
//FIX_ID_001 xxxxx

//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
unsigned char HdmiI2cAddr=IT6602B0_HDMI_ADDR;
//FIX_ID_002 xxxxx




//#ifdef _IT6602_
//unsigned char   DeviceID = IT6602_CHIP;
//#else
//unsigned char   DeviceID = IT66021_CHIP;
//#endif

//for debug video format only
int CurTMDSCLK;
volatile VTiming CurVTiming;
AVI_InfoFrame aviinfoframe;
//int GCP_CD       = CD8BIT; //24 bits per pixel
int InColorMode  = RGB444; //RGB444, YCbCr422, YCbCr444
int OutColorMode = RGB444; //RGB444, YCbCr422, YCbCr444
int OutCD        = OUT8B;
int VIC;
//for debug only


//#ifndef _SUPPORT_EQ_ADJUST_	//20130401
//BYTE OldRegB4=0xAA;
//BYTE OldRegB5=0xAA;
//BYTE OldRegB9=0xAA;
//BYTE OldRegBA=0xAA;
//#endif


#ifdef _FIX_ID_028_
//FIX_ID_028 xxxxx //For Debug Audio error with S2
static unsigned char m_bAudioWaiting=0;
//FIX_ID_028 xxxxx
#else

#ifdef EnableCalFs
//FIX_ID_023 xxxxx		//Fixed for Audio Channel Status Error with invalid HDMI source
static unsigned int m_u16TMDSCLK=0;
static unsigned char m_ForceFsValue=0;
//static unsigned long m_ROSCCLK;
static unsigned char m_AudioChannelStatusErrorCount=0;
#define MAX_AUDIO_CHANNEL_STATUS_ERROR 4
//FIX_ID_023 xxxxx
#endif
#endif

/****************************************************************************/
/*							EDID Argument									*/
/****************************************************************************/
unsigned char  VSDB_Addr;// for EDID RAM function
unsigned char  txphyadr[2], txphyA, txphyB, txphyC, txphyD, txphylevel;	// for CEC function
unsigned char  rxphyadr[2][2];// for EDID RAM function
unsigned char  rxphyA, rxphyB, rxphyC, rxphyD, rxcurport;	// for CEC function

#ifdef FIX_ID_013
//FIX_ID_013	xxxxx	//For MSC 3D request issue
unsigned char uc3DDtd[]={0x00};
struct PARSE3D_STR	st3DParse;
MHL3D_STATE e3DReqState = MHL3D_REQ_DONE;
unsigned char SVD_LIST[16];
	//unsigned char STRUCTURE_3D[16]={1,0,0,0,0,0,2,0,4,0,0,0,0,0,0,0};
//FIX_ID_013	xxxxx
#endif //FIX_ID_013

#endif

/*****************************************************************************/
/* Init, Power, and IO Structures ********************************************/
/*****************************************************************************/
////////////////////////////////////////////////////////////////////
//it6602 chip inital table
//
//
//
////////////////////////////////////////////////////////////////////
static _CODE struct IT6602_REG_INI  IT6602_HDMI_INIT_TABLE[] = {
//static	struct IT6602_REG_INI code IT6602_HDMI_INIT_TABLE[] = {
//port 0
	{REG_RX_00F,	0x03,	0x00},	//change bank 0
	{REG_RX_010,	0xFF,	0x08},	//[3]1: Register reset
	{REG_RX_00F,	0x03,	0x00},	//change bank 0
	{REG_RX_034,	0xFF,	MHL_ADDR+0x01},	//I2C Slave Addresss for MHL block

	{REG_RX_010,	0xFF,	0x17},	//[4]Auto Video Reset [2]Int Reset [1]Audio Reset [0]Video Reset

	{REG_RX_011,	0xFF,	0x1F},	//Port 0�G[4]EQ Reset [3]CLKD5 Reset [2]CDR Reset [1]HDCP Reset [0]All logic Reset
	{REG_RX_018,	0xFF,	0x1F},	//Port 1�G[4]EQ Reset [3]CLKD5 Reset [2]CDR Reset [1]HDCP Reset [0]All logic Reset
	{REG_RX_012,	0xFF,	0xF8},

	{REG_RX_012,	0xFF,	0xF8},	//Port 0�G[7:3] MHL Logic reset

	{REG_RX_010,	0xFF,	0x10},	//[4]Auto Video Reset [2]Int Reset [1]Audio Reset [0]Video Reset

	{REG_RX_011,	0xFF,	0xA0},	//Port 0�G[7] Enable Auto Reset when Clock is not stable [5]Enable Auto Reset
	{REG_RX_018,	0xFF,	0xA0},	//Port 1�G[7] Enable Auto Reset when Clock is not stable [5]Enable Auto Reset

	{REG_RX_012,	0xFF,	0x00},	//Port 0�G[7:3] MHL Logic reset

	{REG_RX_00F,	0x03,	0x01},	//change bank 1	//2013-0430 Andrew suggestion
	{REG_RX_1B0,	0x03,	0x01},	// MHL Port Set HPD = 0 at Power On initial state

//FIX_ID_037 xxxxx //Allion MHL compliance issue debug !!!
//FIX_ID_018 xxxxx 	modify 1K pull-down to 1.033K ohm HDMI Reg1C0[3:2]=2
//2014-0526 MHL compliance issue Debug disable ->	{REG_RX_1C0,	0x8C,	0x08},	//[7] PWSB_LV = 0	//2013-0430 Andrew suggestion
//FIX_ID_018 xxxxx
//FIX_ID_037 xxxxx


	{REG_RX_00F,	0x03,	0x00},	//change bank 0	//2013-0430 Andrew suggestion
	{REG_RX_017,	0xC0,	0x80},	//Port 0�G[7:6] = 10 invert Port 0 input HCLK , CLKD5I	//2013-0430 Andrew suggestion
	{REG_RX_01E,	0xC0,	0x00},	//Port 1�G[7:6] = 00 invert Port 1 input TMDS , CLKD5I	//2013-0430 Andrew suggestion

#ifdef Enable_IT6602_CEC
	{REG_RX_00E,	0xFF,	0xFF},	//for enable CEC Clock
	{REG_RX_086,	0xFF,	(CEC_ADDR|0x01)},	//CEC chip Slave Adr
#endif

//	{0xFE,	0x80,	0x80},	//BUS10B for FPGA

	{REG_RX_016,	0x08,	0x08},	//Port 0�G[3]1: Enable CLKD5 auto power down
	{REG_RX_01D,	0x08,	0x08},	//Port 1�G[3]1: Enable CLKD5 auto power down

//	{0x20,	0x01,	0x30},	//Port 0�GAFE control

	{REG_RX_02B,	0xFF,	0x07},	//FixTek3D
//	{REG_RX_031,	0xFF,	0x2C},	//[7:4]Enable repeater function [3:0] SCL hold time count & Update Ri sel
//FIX_ID_042 xxxxx //Disable HDCP 1.1 feature to avoid compilance issue from ilegal HDCP 1.1 source device
	{REG_RX_031,	0xFF,	0x09},	//[7:4]Enable repeater function [3:0] SCL hold time count & Update Ri sel
	{REG_RX_049,	0xFF,	0x09},	//[7:4]Enable repeater function [3:0] SCL hold time count & Update Ri sel
//FIX_ID_042 xxxxx
//20131129 move to top side->	{REG_RX_034,	0xFF,	MHL_ADDR+0x01},	//I2C Slave Addresss for MHL block
//FIX_ID_017 xxxxx Disable IPLockChk
//FIX_ID_001 xxxxx UseIPLock = 0 for avoid clk change
	{REG_RX_035,	0x1E,	(0x10+(DeltaNum<<2))},	//[3:2] RCLKDeltaSel , [1] UseIPLock = 0
	{REG_RX_04B,	0x1E,	(0x10+(DeltaNum<<2))},	//[3:2] RCLKDeltaSel , [1] UseIPLock = 0
//FIX_ID_001 xxxxx
//FIX_ID_017 xxxxx
	{REG_RX_054,	0xFF,	(1<<4)+RCLKFreqSel},	//[1:0]RCLK frequency select
	{REG_RX_06A,	0xFF,	GenPktRecType},			//Decide which kind of packet to be fully recorded on General PKT register
	{REG_RX_074,	0xFF,	0xA0},	//[7]Enable i2s and SPDIFoutput [5]Disable false DE output
	{REG_RX_050,	0x1F,	0x12},	//[4]1: Invert output DCLK and DCLK DELAY 2 Step
//2013-0606	{REG_RX_050,	0x13,	0x00},	//[4]1: Invert output DCLK and DCLK DELAY 2 Step

//	{REG_RX_065,	0x0C,	0x00},	//[3:2]0=8bits Output color depth
//	{REG_RX_065,	0x0C,	0x04},	//[3:2]1=10bits Output color depth
	{REG_RX_065,	0x0C,	0x08},	//[3:2]2=12bits Output color depth

	{REG_RX_07A,	0x80,	0x80},	//[7]1: enable audio B Frame Swap Interupt
//	{REG_RX_02D,	0x03,	0x03},	//[1:0] 11: Enable HDMI/DVI mode over-write

	{REG_RX_085,	0x02,	0x02},	//[1]1: gating avmute in video detect module

//	{REG_RX_051,	0x80,	0x80},	//[7]1: power down color space conversion logic

#ifdef  _SUPPORT_EDID_RAM_
	{REG_RX_0C0,	0x43,	0x40},	//[0]1:Reg_P0DisableShadow
	{REG_RX_087,	0xFF,	(EDID_ADDR|0x01)},	//[7:1] EDID RAM Slave Adr ,[0]1: Enable access EDID block
#else
	{REG_RX_0C0,	0x03,	0x03},	//[0]1:Reg_P0DisableShadow
	{REG_RX_087,	0xFF,	(0x00)},	//[7:1] EDID RAM Slave Adr ,[0]1: Enable access EDID block
#endif

	{REG_RX_071,	0x08,	0x00},	//Reg71[3] RegEnPPColMode must clear to 0 for andrew suggestion 2013-0502
//FIX_ID_030 xxxxx fixed video lost at 640x480 timing
	{REG_RX_037,	0xFF,	0xA6},	//Reg37 Reg_P0_WCLKValidNum must set to 0xA6 for andrew suggestion 2014-0403
	{REG_RX_04D,	0xFF,	0xA6},	//Reg4D Reg_P1_WCLKValidNum must set to 0xA6 for andrew suggestion 2014-0403
//FIX_ID_030 xxxxx
	{REG_RX_067,	0x80,	0x00},	//Reg67[7] disable HW CSCSel

	{REG_RX_07A,B_CTS_RES,B_CTS_RES},

//FIX_ID_037 xxxxx //Allion MHL compliance issue debug !!!
//FIX_ID_018 xxxxx 	modify 1K pull-down to 1.033K ohm HDMI Reg1C0[3:2]=2
//2014-0526 MHL compliance issue Debug disable ->	{REG_RX_1C0,	0x8C,	0x08},	//[7] PWSB_LV = 0	//2013-0430 Andrew suggestion
// Reg1C0[3:2] = 00 -> 1.08Kohm	0 %
// Reg1C0[3:2] = 01 -> 1.18Kohm	+10 %
// Reg1C0[3:2] = 10 -> 0.98Kohm	-10%
// Reg1C0[3:2] = 11 -> 0.88Kohm	-20%
//FIX_ID_018 xxxxx
#if defined(_IT6602_) || defined(_IT66023_)
	{REG_RX_077, 0x80, 0x00},
	{REG_RX_00F, 0x03, 0x01},	//change bank 1
	{REG_RX_1C0, 0x8C, 0x04},	//FIX_ID_037  2014-0527 +10% for W1070 only
	{REG_RX_00F, 0x03, 0x00},	//change bank 0
#else
	{REG_RX_077, 0x80, 0x80},	 // IT66021 Audio i2s sck and mclk is common pin
	{REG_RX_00F, 0x03, 0x01},	//change bank 1
	{REG_RX_1C0, 0x80, 0x80},
	{REG_RX_00F, 0x03, 0x00},	//change bank 0
#endif
//FIX_ID_037 xxxxx

#ifdef _HBR_I2S_
	{REG_RX_07E,B_HBRSel,0x00},
#else
	{REG_RX_07E,B_HBRSel,B_HBRSel},
#endif

	{REG_RX_052,(B_DisVAutoMute),(B_DisVAutoMute)},				//Reg52[5] = 1 for disable Auto video MUTE
	{REG_RX_053,(B_VDGatting|B_VIOSel|B_TriVDIO|B_TriSYNC),(B_VIOSel|B_TriVDIO|B_TriSYNC)},				//Reg53[7][5] = 01    // for disable B_VDIO_GATTING

	{REG_RX_058,0xFF,0x33},			// Reg58 for 4Kx2K Video output Driving Strength

//	{REG_RX_059,0xFF,0xAA},			// Reg59 for Audio output Driving Strength

//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
//!!!  For Manual Adjust EQ only  !!!
#ifdef _SUPPORT_MANUAL_ADJUST_EQ_
	{REG_RX_03E,0x20,0x20},	// Enable OvWrRsCs
	{REG_RX_026,0x20,0x20},	// Enable OvWrRsCs
#endif
//FIX_ID_001 xxxxx


#ifdef _ONLY_SUPPORT_MANUAL_EQ_ADJUST_
	{REG_RX_026,	0xFF,	0x20},	//Reg26=0x00 disable Auto Trigger
	{REG_RX_03E,	0xFF,	0x20},	//Reg3E=0x00 disable Auto Trigger
#endif

//RS initial valie
// 2013/06/06 added by jau-chih.tseng@ite.com.tw
// Dr. Liu said, reg25/reg3D should set as 0x1F for auto EQ start option.
	{REG_RX_025, 0xFF, DEFAULT_EQVALUE},
	{REG_RX_03D, 0xFF, DEFAULT_EQVALUE},
//~jau-chih.tseng@ite.com.tw
	{REG_RX_027, 0xFF, DEFAULT_EQVALUE},	// B ch
	{REG_RX_028, 0xFF, DEFAULT_EQVALUE},	// G
	{REG_RX_029, 0xFF, DEFAULT_EQVALUE},	// R
	{REG_RX_03F, 0xFF, DEFAULT_EQVALUE},
	{REG_RX_040, 0xFF, DEFAULT_EQVALUE},
	{REG_RX_041, 0xFF, DEFAULT_EQVALUE},

	{REG_RX_00F,	0x03,	0x01},	//change bank 1	//2013-0515 Andrew suggestion	for Auto EQ
	{REG_RX_1BC,	0xFF,	0x06},	//Reg1BC=0x06		//2013-0515 Andrew suggestion	for Auto EQ
//FIX_ID_020 xxxxx		//Turn off DEQ for HDMI port 1 with 20m DVI Cable
	{REG_RX_1CC,	0xFF,	0x00},	//Reg1CC=0x00		for TURN OFF DEQ
	{REG_RX_1C6,      0x07,      0x03},	// [2:0]Reg_P1_ENHYS = 03 for default enable filter to gating output
//FIX_ID_020 xxxxx

	{REG_RX_1B5,	0x03,	0x03},	//Reg1B5[1:0]='11'	for fix Korea K706 MHL pattern Generator	//2013-0515 Andrew suggestion
//FIX_ID_019	xxxxx modify ENHYS control for MHL mode
	{REG_RX_1B8,      0x80,      0x00},	// [7] Reg_HWENHYS = 0
	{REG_RX_1B6,      0x07,      0x03},	// [2:0]Reg_P0_ENHYS = 03 for default enable filter to gating output
//FIX_ID_019	xxxxx


//FIX_ID_029	xxxxx fixed Ulta-2000 HDCP fail issue at Receiver mode
	{REG_RX_128,      0xFF,      0x00},	// Clear KSV LIST
	{REG_RX_129,      0xFF,      0x00},	// Clear KSV LIST
	{REG_RX_12A,      0xFF,      0x00},	// Clear KSV LIST
	{REG_RX_12B,      0xFF,      0x00},	// Clear KSV LIST
	{REG_RX_12C,      0xFF,      0x00},	// Clear KSV LIST
//FIX_ID_029	xxxxx

	{REG_RX_00F,	0x03,	0x00},	//change bank 0	//2013-0515 Andrew suggestion	for Auto EQ

//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
// 	for Auto EQ
#ifdef _SUPPORT_AUTO_EQ_
//0704 disable ->	{REG_RX_022,	0xFF,	0x38},	//Reg22=0x38		//2013-0515 Andrew suggestion	for Auto EQ
//0704 disable ->	{REG_RX_03A,	0xFF,	0x38},	//Reg3A=0x38		//2013-0515 Andrew suggestion	for Auto EQ
	{REG_RX_022,	0xFF,	0x00},	// 07-16 Reg22=0x30	power down auto EQ
	{REG_RX_03A,	0xFF,	0x00},	// 07-16 Reg3A=0x30	power down auto EQ

#ifdef ENABLE_AUTO_TRIGGER
	{REG_RX_026,	0xFF,	0x80},	//Reg26=0x80	enable Auto Trigger
	{REG_RX_03E,	0xFF,	0x80},	//Reg3E=0x80	enable Auto Trigger
#else
	{REG_RX_026,	0xFF,	0x00},	//Reg26=0x00 disable Auto Trigger
	{REG_RX_03E,	0xFF,	0x00},	//Reg3E=0x00 disable Auto Trigger
#endif

#else
	{REG_RX_022,	0xFF,	0x00},	// 07-16 Reg22=0x30	power down auto EQ
	{REG_RX_03A,	0xFF,	0x00},	// 07-16 Reg3A=0x30	power down auto EQ

	{REG_RX_026,	0xFF,	0x00},	// 07-16 Reg26=0x00 disable Auto Trigger
	{REG_RX_03E,	0xFF,	0x00},	// 07-16 Reg3E=0x00 disable Auto Trigger

#endif
//FIX_ID_001 xxxxx

//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
//	if(HdmiI2cAddr==IT6602A0_HDMI_ADDR)
//	{
//	//FIX_ID_007 xxxxx 	//for debug IT6681  HDCP issue
//	{REG_RX_00F,	0x03,	0x01},	//change bank 1
//	{REG_RX_1B1,	0xFF,	0x20},	// enable SW OVER-WRITE
//	{REG_RX_1B2,	0xFF,	0x01},
//	{REG_RX_00F,	0x03,	0x00},	//change bank 0
//	//FIX_ID_007 xxxxx
//	}
//
//
//	{REG_RX_020,	0x7F,	0x00},	//set 0x3F when TMDS < 1.48 GHz , otherwise set 0x00 , [6]=0 for IT6602A0
//	{REG_RX_038,	0x7F,	0x00},	//set 0x3F when TMDS < 1.48 GHz , otherwise set 0x00 , [6]=0 for IT6602A0
//
//FIX_ID_002 xxxxx

//	{REG_RX_014,0xFF,0xFF},		//for enable interrupt output Pin
//	{REG_RX_063,0xFF,0x3F},		//for enable interrupt output Pin MZY 17/5/5
	{REG_RX_073, 0x08, 0x00},		// for HDCPIntKey = false

	{REG_RX_060, 0x40, 0x00},		// disable interrupt mask for NoGenPkt_Rcv

//FIX_ID_017 xxxxx Disable IPLockChk
	{REG_RX_02A, 0x01, 0x00},		// disable PORT 0 EnIPLockChk
	{REG_RX_042, 0x01, 0x00},		// disable PORT 1 EnIPLockChk
	//{REG_RX_035, 0x02, 0x00},		// disable PORT 0 EnIPLockChk
	//{REG_RX_04B, 0x02, 0x00},
//FIX_ID_017 xxxxx


#if defined(_IT66023_)
//FIX_ID_016 xxxxx Support Dual Pixel Mode for IT66023 Only
	{REG_RX_08C, 0x09, 0x09},		// Reg8C[0] = 1	// SPOutMode�G//  for enable IO Mapping for Signal Pixel mode
//FIX_ID_016 xxxxx
#endif


//FIX_ID_025 xxxxx Audio lock method select for HDMI Repeater / splitter application
	{REG_RX_077, 0x0C, 0x08},		// Reg77[3:2] = 01	Audio lock method select
//FIX_ID_025 xxxxx

	{0xFF, 0xFF, 0xFF},
};



//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_
////////////////////////////////////////////////////////////////////
//it6602 MHL inital table
//
//
//
////////////////////////////////////////////////////////////////////
static struct IT6602_REG_INI _CODE IT6602_MHL_INIT_TABLE[] = {

	{MHL_RX_0A,	0xFF,	0x00},	//INT MASK 1: disable corresponding interrupt
	{MHL_RX_08,	0xFF,	0x05},	//INT MASK 1: disable corresponding interrupt , disable TX packet done , Rx Packet Done
	{MHL_RX_09,	0xFF,	0x40},	//INT MASK 1: disable corresponding interrupt, disable DDC Rpd done

	{MHL_RX_52,	0xFF,	0x00},	//0: handle by HW
	{MHL_RX_53,	0xFF,	0x80},	//[7]=1 disable FW mode

	{MHL_RX_32,	0xFF,	0x0C},					//CBUS arbitration Low time
	{MHL_RX_81,	0xFF,	MHLVersion},  			//DCAP00�GMHL_VER_MAJOR and MINOR
	{MHL_RX_82,	0xFF,	(PLIM<<5)+(POW<<4)+DEV_TYPE_SINK},  	//DCAP02�GDEVICE TYPE
	{MHL_RX_83,	0xFF,	ADOPTER_ID_H},  		//DCAP03�GADOPTER_ID
	{MHL_RX_84,	0xFF,	ADOPTER_ID_L},
	{MHL_RX_8B,	0xFF,	DEVICE_ID_H},  			//DCAP0B�GDEVICE_ID
	{MHL_RX_8C,	0xFF,	DEVICE_ID_L},

	{MHL_RX_28,	0x0F,	0x00},	//Port 0�G[0] = 0 RS value of CH1 if req_setEQ=1		//2013-0430 Andrew suggestion

	{MHL_RX_0F,	0x20,	0x20},	//[5]Guard band swap when Pack Mode
//FIX_ID_004 xxxxx //Add 100ms calibration for Cbus
	{MHL_RX_29,	0x83,	(T10usSrcSel<<7)+(0x03)},	//[7] 1: frome 27M crystall ( _SelectExtCrystalForCbus_ )
//FIX_ID_004 xxxxx
	{MHL_RX_39,	0x80,	0x80},	//[7] 1: enable I2C deglitch
	{MHL_RX_00,	0x8F,	(EnCBusDbgMode<<7)+(MSCCBusDbgCtrl<<2)+DDCCBusDbgCtrl},	//Enable Cbus Debug Mode
//FIX_ID_004 xxxxx //Add 100ms calibration for Cbus
	{MHL_RX_01,	0xFC,	(EnCBusDeGlitch<<7)+(Reg100msTOAdj<<2)+0x50},  	//Cbus configure , [6:4] OSCCLK Divide count select = 5 for bit time =1us ( _SelectExtCrystalForCbus_ )
//FIX_ID_004 xxxxx
	{MHL_RX_0C,	0x89,	(PPHDCPOpt<<7)+EnHWPathEn+(PPHDCPOpt2<<3)},   //PATH_EN configure
	{MHL_RX_36,	0xFC,	(AckHigh<<4)+(AckLow<<2)},  //ACK configure
	{MHL_RX_38,	0x20,	EnDDCSendAbort<<5},  //DDC bus configure
	{MHL_RX_5C,	0xFC,	(EnPktFIFOBurst<<7)+(MSCBurstWrID<<6)+(MSCBurstWrOpt<<5)+(EnMSCBurstWr<<4)+(EnMSCHwRty<<3)+(MSCRxUCP2Nack<<2)},  //MSC configure
	{MHL_RX_66,	0x03,	RegBurstWrTOSel},  //Brst WrTOSel

//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
//FIX_ID_031 xxxxx //change CBus pull down 1K after 75ms power 5V  stable .
	{MHL_RX_2A, 0x07, 0x01},		// MHL2A[2:1]=00 for Set Reg5VStableTSel = 75ms, 	[0]=1 enable HW rstddfsm
//xxxxx 2014-0421 disable -> 	{MHL_RX_2A, 0x01, 0x01},		// MHL2A[2:1]=01 for Set Reg5VStableTSel = 100ms, 	[0]=1 enable HW rstddfsm
//FIX_ID_031 xxxxx
//FIX_ID_033 xxxxx

	{MHL_RX_0F, 0x10, 0x00},

// #ifdef SUPPORT_INVALID_WAKEUP
// 	{MHL_RX_28, 0x08, 0x08},		//2013-0515 for fix HTC one-S discovery pulse issue
// #endif


//FIX_ID_018	xxxxx	//modify 1K pull-down to 1.033K ohm HDMI Reg1C0[3:2]=2
//	{MHL_RX_2A, 0x60, 0x20},	// HW adjust wake up fail count
//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
//	{MHL_RX_2B, 0x04, 0x04},	// MHL2B[2] 1 for enable HW wake up fail machanism
//	{MHL_RX_2B, 0xFF, 0x1A},	//xxxxx 2014-0520 --> [2] 0 : for disable HW wake up fail machanism
//FIX_ID_037 xxxxx
//FIX_ID_018	xxxxx

	{0xFF, 0xFF, 0xFF},

};


#ifdef _SUPPORT_RAP_
//                      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
_CODE unsigned char  SuppRAPCode[32] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
                        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};// 1
#endif

#ifdef _SUPPORT_RCP_
_CODE unsigned char  SuppRCPCode[128]= { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 0
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 2
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 3
                        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 4
                        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 5
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 6
                        0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};// 7
#endif

#endif
//FIX_ID_036	xxxxx

static _CODE unsigned char bCSCMtx_RGB2YUV_ITU601_16_235[]=
{
	0x00,		0x80,		0x10,
	0xB2,0x04,	0x65,0x02,	0xE9,0x00,
	0x93,0x3C,	0x18,0x04,	0x55,0x3F,
	0x49,0x3D,	0x9F,0x3E,	0x18,0x04
};

static _CODE unsigned char bCSCMtx_RGB2YUV_ITU601_0_255[]=
{
	0x10,		0x80,		0x10,
	0x09,0x04,	0x0E,0x02,	0xC9,0x00,
	0x0F,0x3D,	0x84,0x03,	0x6D,0x3F,
	0xAB,0x3D,	0xD1,0x3E,	0x84,0x03
};

static _CODE unsigned char bCSCMtx_RGB2YUV_ITU709_16_235[]=
{
	0x00,		0x80,		0x10,
	0xB8,0x05,	0xB4,0x01,	0x94,0x00,
	0x4A,0x3C,	0x17,0x04,	0x9F,0x3F,
	0xD9,0x3C,	0x10,0x3F,	0x17,0x04
};

static _CODE unsigned char bCSCMtx_RGB2YUV_ITU709_0_255[]=
{
	0x10,		0x80,		0x10,
	0xEA,0x04,	0x77,0x01,	0x7F,0x00,
	0xD0,0x3C,	0x83,0x03,	0xAD,0x3F,
	0x4B,0x3D,	0x32,0x3F,	0x83,0x03
};


static _CODE unsigned char bCSCMtx_YUV2RGB_ITU601_16_235[] =
{
	0x00,		0x00,		0x00,
	0x00,0x08,	0x6B,0x3A,	0x50,0x3D,
	0x00,0x08,	0xF5,0x0A,	0x02,0x00,
	0x00,0x08,	0xFD,0x3F,	0xDA,0x0D
} ;

static _CODE unsigned char bCSCMtx_YUV2RGB_ITU601_0_255[] =
{
	0x04,		0x00,		0xA7,
	0x4F,0x09,	0x81,0x39,	0xDD,0x3C,
	0x4F,0x09,	0xC4,0x0C,	0x01,0x00,
	0x4F,0x09,	0xFD,0x3F,	0x1F,0x10
} ;

static _CODE unsigned char bCSCMtx_YUV2RGB_ITU709_16_235[] =
{
	0x00,		0x00,		0x00,
	0x00,0x08,	0x55,0x3C,	0x88,0x3E,
	0x00,0x08,	0x51,0x0C,	0x00,0x00,
	0x00,0x08,	0x00,0x00,	0x84,0x0E
} ;

static _CODE unsigned char bCSCMtx_YUV2RGB_ITU709_0_255[] =
{
	0x04,		0x00,		0xA7,
	0x4F,0x09,	0xBA,0x3B,	0x4B,0x3E,
	0x4F,0x09,	0x57,0x0E,	0x02,0x00,
	0x4F,0x09,	0xFE,0x3F,	0xE8,0x10
} ;

//FIX_ID_027 xxxxx Support Full/Limited Range convert
//full 2 limit
static _CODE unsigned char bCSCMtx_RGB_0_255_RGB_16_235[] =
{
	0x10,		0x10,		0x00,
	0xe0,0x06,	0x00,0x00,	0x00,0x00,
	0x00,0x00,	0xe0,0x06,	0x00,0x00,
	0x00,0x00,	0x00,0x00,	0xe0,0x06,


} ;

//limit 2 full
static _CODE unsigned char bCSCMtx_RGB_16_235_RGB_0_255[] =
{
	0xED,		0xED,		0x00,
	0x50,0x09,	0x00,0x00,	0x00,0x00,
	0x00,0x00,	0x50,0x09,	0x00,0x00,
	0x00,0x00,	0x00,0x00,	0x50,0x09,
} ;
//FIX_ID_027 xxxxx

#ifdef  _SUPPORT_EDID_RAM_
// EDID_SELECT_TABLE
// (0) IT6602 support 4K2k
// (1) IT6602 3D
// (2) Philips monitor for 4kX2k
// (3) AOC monitor without 3D
// (4) Astro 1831 HDMI analyzer
// (5) TI PICO 343X EDID
// (6) IT6602 with 640x480p , 720x480p , 1280x720p , 1920x1080p
// (7) IT6602 with 640x480p , 720x480p , 1280x720p , 1920x1080p, 1440x480i
// (8) artosyn edid 720P60,1080P30
// (9) only 720p 60fs
#define EDID_SELECT_TABLE	(9)

unsigned char _CODE Default_Edid_Block[256] = {
#if (EDID_SELECT_TABLE == 0)
// 0 for IT6602 4K2k  EDID
// 0       1       2       3       4        5       6       7       8       9       A        B       C      D       E       F
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x26, 0x85, 0x02, 0x66, 0x01, 0x01, 0x01, 0x01,	// 00h: 8 ~B  Vendor ID
0x21, 0x17, 0x01, 0x03, 0x80, 0x55, 0x30, 0x78, 0x2A, 0x63, 0xBD, 0xA1, 0x54, 0x52, 0x9E, 0x26,	// 10h:
0x0C, 0x47, 0x4A, 0x20, 0x08, 0x00, 0x81, 0x80, 0xD1, 0xC0, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,	// 20h:
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x74, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80, 0xB0, 0x58,	// 30h: 36 ~ 47 (4Kx2K)
0x8A, 0x00, 0xA2, 0x0B, 0x32, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20,	// 40h: 48 ~ 59 (720p60Hz)
0x6E, 0x28, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x18,
0x4C, 0x1E, 0x53, 0x1E, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC,
0x00, 0x49, 0x54, 0x45, 0x36, 0x38, 0x30, 0x32, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x8B,
0x02, 0x03, 0x25, 0xF1, 0x43, 0x84, 0x10, 0x03, 0x23, 0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00,
0xE2, 0x00, 0x0F, 0xE3, 0x05, 0x03, 0x01, 0x6D, 0x03, 0x0C, 0x00, 0x10, 0x00, 0x38, 0x3C, 0x20,	// 0x38 for support 36bit , 0x3C for support 300MHz TMDS
0x00, 0x60, 0x03, 0x02, 0x01, 0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55,
0x00, 0xA0, 0x5A, 0x00, 0x00, 0x00, 0x1E, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10,
0x3E, 0x96, 0x00, 0xA0, 0x5A, 0x00, 0x00, 0x00, 0x18, 0xF3, 0x39, 0x80, 0x18, 0x71, 0x38, 0x2D,
0x40, 0x58, 0x2C, 0x45, 0x00, 0xE0, 0x0E, 0x11, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15

#elif (EDID_SELECT_TABLE == 1)
// 1 for IT6602 3D EDID
// 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x26, 0x85, 0x02, 0x66, 0x01, 0x00, 0x00, 0x00,
0x0C, 0x14, 0x01, 0x03, 0x80, 0x1C, 0x15, 0x78, 0x0A, 0x1E, 0xAC, 0x98, 0x59, 0x56, 0x85, 0x28,
0x29, 0x52, 0x57, 0x20, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E,
0x96, 0x00, 0xFA, 0xBE, 0x00, 0x00, 0x00, 0x18, 0xD5, 0x09, 0x80, 0xA0, 0x20, 0xE0, 0x2D, 0x10,
0x10, 0x60, 0xA2, 0x00, 0xFA, 0xBE, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x49,
0x54, 0x45, 0x36, 0x38, 0x30, 0x32, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFD,
0x00, 0x17, 0x3D, 0x0D, 0x2E, 0x11, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x1B,

0x02,0x03,0x30,0xF1,0x43,0x84,0x10,0x03,0x23,0x09,0x07,0x07,0x83,
0x01,0x00,0x00,0xE2,0x00,0x0F,0xE3,0x05,0x03,0x01,0x78,0x03,0x0C,0x00,0x10,0x00,
0x88,0x2D,0x20,0xC0,0x0E,0x01,0x00,0x00,0x12,0x18,0x20,0x28,0x20,0x38,0x20,0x58,
0x20,0x68,0x20,0x01,0x1D,0x00,0x72,0x51,0xD0,0x1E,0x20,0x6E,0x28,0x55,0x00,0xA0,
0x5A,0x00,0x00,0x00,0x1E,
0x8C,0x0A,0xD0,0x8A,0x20,0xE0,0x2D,0x10,0x10,
0x3E,0x96,0x00,0xA0,0x5A,0x00,0x00,0x00,0x18,0xF3,0x39,0x80,0x18,0x71,0x38,0x2D,
0x40,0x58,0x2C,0x45,0x00,0xE0,0x0E,0x11,0x00,0x00,0x1E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6B


#elif (EDID_SELECT_TABLE == 2)
//Philips Monitor 4kX2K EDID for test MHL 3D request function
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x41, 0x0C, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
0x21, 0x17, 0x01, 0x03, 0x80, 0x55, 0x30, 0x78, 0x2A, 0x63, 0xBD, 0xA1, 0x54, 0x52, 0x9E, 0x26,
0x0C, 0x47, 0x4A, 0xA3, 0x08, 0x00, 0x81, 0x80, 0xD1, 0xC0, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x74, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80, 0xB0, 0x58,
0x8A, 0x00, 0xA2, 0x0B, 0x32, 0x00, 0x00, 0x1E, 0x1B, 0x21, 0x50, 0xA0, 0x51, 0x00, 0x1E, 0x30,
0x48, 0x88, 0x35, 0x00, 0xA2, 0x0B, 0x32, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x18,
0x4C, 0x1E, 0x53, 0x1E, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC,
0x00, 0x50, 0x68, 0x69, 0x6C, 0x69, 0x70, 0x73, 0x20, 0x54, 0x56, 0x0A, 0x20, 0x20, 0x01, 0xDF,

0x02, 0x03, 0x35, 0xF1, 0x4C, 0x90, 0x1F, 0x21, 0x20, 0x05, 0x14, 0x04, 0x13, 0x03, 0x07, 0x12,
0x16, 0x26, 0x09, 0x07, 0x03, 0x11, 0x06, 0x00, 0x83, 0x01, 0x00, 0x00, 0x78, 0x03, 0x0C, 0x00,
0x30, 0x00, 0x00, 0x3C, 0x20, 0xC8, 0x8A, 0x01, 0x03, 0x02, 0x04, 0x01, 0x41, 0x00, 0xCF, 0x66,
0x68, 0x10, 0x56, 0x58, 0x80, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96,
0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18, 0x8C, 0x0A, 0xA0, 0x14, 0x51, 0xF0, 0x16, 0x00, 0x26,
0x7C, 0x43, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x98, 0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40, 0x31,
0x20, 0x0C, 0x40, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18, 0x8C, 0x0A, 0xA0, 0x20, 0x51,
0x20, 0x18, 0x10, 0x18, 0x7E, 0x23, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x98, 0x00, 0x00, 0x24,

#elif (EDID_SELECT_TABLE == 3)
//AOC monitor EDID (without 3D)
0x00, 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0x00,0x05, 0xE3, 0x69, 0x23,0x66, 0x00, 0x00, 0x00,
0x0D, 0x17, 0x01, 0x03,0x80, 0x33, 0x1D, 0x78,0x2A, 0xE5, 0x95, 0xA6,0x56, 0x52, 0x9D, 0x27,
0x10, 0x50, 0x54, 0xBF,0xEF, 0x00, 0xD1, 0xC0,0xB3, 0x00, 0x95, 0x00,0x81, 0x80, 0x81, 0x40,
0x81, 0xC0, 0x01, 0x01,0x01, 0x01, 0x02, 0x3A,0x80, 0x18, 0x71, 0x38,0x2D, 0x40, 0x58, 0x2C,
0x45, 0x00, 0xFD, 0x1E,0x11, 0x00, 0x00, 0x1E,0x00, 0x00, 0x00, 0xFD,0x00, 0x32, 0x4C, 0x1E,
0x53, 0x11, 0x00, 0x0A,0x20, 0x20, 0x20, 0x20,0x20, 0x20, 0x00, 0x00,0x00, 0xFC, 0x00, 0x32,
0x33, 0x36, 0x39, 0x4D,0x0A, 0x20, 0x20, 0x20,0x20, 0x20, 0x20, 0x20,0x00, 0x00, 0x00, 0xFF,
0x00, 0x42, 0x46, 0x44,0x44, 0x33, 0x39, 0x41,0x30, 0x30, 0x30, 0x31,0x30, 0x32, 0x01, 0x19,

0x02, 0x03, 0x1F, 0xF1,0x4C, 0x10, 0x1F, 0x05,0x14, 0x04, 0x13, 0x03,0x12, 0x02, 0x11, 0x01,
0x22, 0x23, 0x09, 0x07,0x07, 0x83, 0x01, 0x00,0x00, 0x65, 0x03, 0x0C,0x00, 0x10, 0x00, 0x8C,
0x0A, 0xD0, 0x8A, 0x20,0xE0, 0x2D, 0x10, 0x10,0x3E, 0x96, 0x00, 0xFD,0x1E, 0x11, 0x00, 0x00,
0x18, 0x01, 0x1D, 0x00,0x72, 0x51, 0xD0, 0x1E,0x20, 0x6E, 0x28, 0x55,0x00, 0xFD, 0x1E, 0x11,
0x00, 0x00, 0x1E, 0x8C,0x0A, 0xD0, 0x8A, 0x20,0xE0, 0x2D, 0x10, 0x10,0x3E, 0x96, 0x00, 0xFD,
0x1E, 0x11, 0x00, 0x00,0x18, 0x8C, 0x0A, 0xD0,0x90, 0x20, 0x40, 0x31,0x20, 0x0C, 0x40, 0x55,
0x00, 0xFD, 0x1E, 0x11,0x00, 0x00, 0x18, 0x00,0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x59,

#elif (EDID_SELECT_TABLE == 4)
// Astro 1831 EDID
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x06, 0x8F, 0x12, 0xB0, 0x01, 0x00, 0x00, 0x00,
    0x0C, 0x14, 0x01, 0x03, 0x80, 0x1C, 0x15, 0x78, 0x0A, 0x1E, 0xAC, 0x98, 0x59, 0x56, 0x85, 0x28,
    0x29, 0x52, 0x57, 0x20, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E,
    0x96, 0x00, 0xFA, 0xBE, 0x00, 0x00, 0x00, 0x18, 0xD5, 0x09, 0x80, 0xA0, 0x20, 0xE0, 0x2D, 0x10,
    0x10, 0x60, 0xA2, 0x00, 0xFA, 0xBE, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x56,
    0x41, 0x2D, 0x31, 0x38, 0x33, 0x31, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFD,
    0x00, 0x17, 0x3D, 0x0D, 0x2E, 0x11, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0xFA,

    0x02, 0x03, 0x33, 0xF1, 0x46, 0x90, 0x04, 0x05, 0x03, 0x20, 0x22, 0x23, 0x09, 0x07, 0x07, 0x83,
    0x01, 0x00, 0x00, 0xE2, 0x00, 0x0F, 0xE3, 0x05, 0x03, 0x01, 0x78, 0x03, 0x0C, 0x00, 0x10, 0x00,
    0x88, 0x2D, 0x20, 0xC0, 0x0E, 0x01, 0x00, 0x00, 0x12, 0x18, 0x20, 0x28, 0x20, 0x38, 0x20, 0x58,
    0x20, 0x68, 0x20, 0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0xA0,
    0x5A, 0x00, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20, 0x58, 0x2C, 0x25,
    0x00, 0xA0, 0x5A, 0x00, 0x00, 0x00, 0x9E, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10,
    0x3E, 0x96, 0x00, 0xA0, 0x5A, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF4

#elif (EDID_SELECT_TABLE == 5)
// TI PICO 343X EDID(864x480) + IT66021 MONITOR (720P) + gamma 2.6
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,   // address 0x00
0x11, 0x90, 0x50, 0x50, 0x01, 0x00, 0x00, 0x00,   //
0x08, 0x13, 0x01, 0x03, 0x80, 0x34, 0x20, 0xA0,   // address 0x10
0x2A, 0xEE, 0x95, 0xA3, 0x54, 0x4C, 0x99, 0x26,   //
0x0F, 0x50, 0x54, 0x21, 0x08, 0x00, 0x01, 0x01,   // address 0x20
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   //
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x1D,   // address 0x30
0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28,   //
0x55, 0x00, 0x00, 0xD0, 0x52, 0x00, 0x00, 0x1E,   // address 0x40
0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10,   //
0x10, 0x3E, 0x96, 0x00, 0xD0, 0xE0, 0x21, 0x00,   // address 0x50
0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x49,   //
0x54, 0x45, 0x36, 0x38, 0x30, 0x31, 0x20, 0x0A,   // address 0x60
0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFD,   //
0x00, 0x30, 0x7A, 0x1F, 0x41, 0x0F, 0x00, 0x0A,   // address 0x70
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x95,   //
0x02, 0x03, 0x1B, 0x41, 0x83, 0x01, 0x00, 0x00,   // address 0x80
0x65, 0x03, 0x0C, 0x00, 0x10, 0x00, 0x48, 0x84,   //
0x02, 0x03, 0x11, 0x12, 0x13, 0x2A, 0x30, 0x23,   // address 0x90
0x09, 0x07, 0x07, 0x16, 0x0D, 0x60, 0x6A, 0x30,   //
0xE0, 0x5F, 0x10, 0x04, 0x28, 0xFF, 0x07, 0x60,   // address 0xA0
0xE0, 0x31, 0x00, 0x00, 0x1E, 0x44, 0x0C, 0x56,   //
0x36, 0x30, 0xE0, 0x60, 0x10, 0xEA, 0x28, 0x0F,   // address 0xB0
0xC3, 0x56, 0xE0, 0x31, 0x00, 0x00, 0x1E, 0x47,   //
0x09, 0x80, 0x30, 0x20, 0xE0, 0x5F, 0x10, 0xE4,   // address 0xC0
0x28, 0xFF, 0xCF, 0x80, 0xE0, 0x21, 0x00, 0x00,   //
0x3E, 0x9E, 0x20, 0x00, 0x90, 0x51, 0x20, 0x1F,   // address 0xD0
0x30, 0x48, 0x80, 0x36, 0x00, 0x00, 0x20, 0x53,   //
0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00,   // address 0xE0
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // address 0xF0
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69,   //

#elif (EDID_SELECT_TABLE == 6)
// IT6602 with 640x480p , 720x480p , 1280x720p , 1920x1080p
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,   // address 0x00
0x26, 0x85, 0x02, 0x66, 0x21, 0x60, 0x00, 0x00,   //
0x00, 0x17, 0x01, 0x03, 0x80, 0x73, 0x41, 0x78,   // address 0x10
0x2A, 0x7C, 0x11, 0x9E, 0x59, 0x47, 0x9B, 0x27,   //
0x10, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01,   // address 0x20
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   //
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A,   // address 0x30
0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,   //
0x45, 0x00, 0x10, 0x09, 0x00, 0x00, 0x00, 0x1E,   // address 0x40
0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10,   //
0x10, 0x3E, 0x96, 0x00, 0x04, 0x03, 0x00, 0x00,   // address 0x50
0x00, 0x18, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x49,   //
0x54, 0x45, 0x36, 0x38, 0x30, 0x32, 0x0A, 0x20,   // address 0x60
0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFD,   //
0x00, 0x30, 0x7A, 0x0F, 0x50, 0x10, 0x00, 0x0A,   // address 0x70
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0xF3,   //
0x02, 0x03, 0x19, 0x72, 0x46, 0x90, 0x04, 0x13,   // address 0x80
0x01, 0x02, 0x03, 0x23, 0x09, 0x07, 0x07, 0x83,   //
0x01, 0x00, 0x00, 0x65, 0x03, 0x0C, 0x00, 0x10,   // address 0x90
0x00, 0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E,   //
0x20, 0x6E, 0x28, 0x55, 0x00, 0x10, 0x09, 0x00,   // address 0xA0
0x00, 0x00, 0x1E, 0xD6, 0x09, 0x80, 0xA0, 0x20,   //
0xE0, 0x2D, 0x10, 0x10, 0x60, 0xA2, 0x00, 0x04,   // address 0xB0
0x03, 0x00, 0x00, 0x00, 0x18, 0x8C, 0x0A, 0xD0,   //
0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96,   // address 0xC0
0x00, 0x10, 0x09, 0x00, 0x00, 0x00, 0x18, 0x00,   //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // address 0xD0
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // address 0xE0
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // address 0xF0
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B,   //
#elif (EDID_SELECT_TABLE == 7)
// IT6602 with 640x480p , 720x480p , 1280x720p , 1920x1080p , 1440x480i
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,   // address 0x00
0x26, 0x85, 0x02, 0x68, 0x01, 0x68, 0x00, 0x00,   //
0x00, 0x17, 0x01, 0x03, 0x80, 0x73, 0x41, 0x78,   // address 0x10
0x2A, 0x7C, 0x11, 0x9E, 0x59, 0x47, 0x9B, 0x27,   //
0x10, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01,   // address 0x20
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   //
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A,   // address 0x30
0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,   //
0x45, 0x00, 0x10, 0x09, 0x00, 0x00, 0x00, 0x1E,   // address 0x40
0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10,   //
0x10, 0x3E, 0x96, 0x00, 0x04, 0x03, 0x00, 0x00,   // address 0x50
0x00, 0x18, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x49,   //
0x54, 0x45, 0x36, 0x38, 0x30, 0x32, 0x0A, 0x20,   // address 0x60
0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFD,   //
0x00, 0x30, 0x7A, 0x0F, 0x50, 0x10, 0x00, 0x0A,   // address 0x70
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0xF3,   //
0x02, 0x03, 0x1B, 0x72, 0x48, 0x90, 0x04, 0x13,   // address 0x80
0x01, 0x02, 0x03, 0x06, 0x07, 0x23, 0x09, 0x07,   //
0x07, 0x83, 0x01, 0x00, 0x00, 0x65, 0x03, 0x0C,   // address 0x90
0x00, 0x10, 0x00, 0x01, 0x1D, 0x00, 0x72, 0x51,   //
0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0x10,   // address 0xA0
0x09, 0x00, 0x00, 0x00, 0x1E, 0xD6, 0x09, 0x80,   //
0xA0, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x60, 0xA2,   // address 0xB0
0x00, 0x04, 0x03, 0x00, 0x00, 0x00, 0x18, 0x8C,   //
0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10,   // address 0xC0
0x3E, 0x96, 0x00, 0x10, 0x09, 0x00, 0x00, 0x00,   //
0x18, 0x8C, 0x0A, 0xA0, 0x14, 0x51, 0xF0, 0x16,   // address 0xD0
0x00, 0x26, 0x7C, 0x43, 0x00, 0xD0, 0xE0, 0x21,   //
0x00, 0x00, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00,   // address 0xE0
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // address 0xF0
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B,   //
#elif (EDID_SELECT_TABLE == 8)
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x06, 0x8F, 0x07, 0x11, 0x01, 0x00, 0x00, 0x00, 
0x17, 0x11, 0x01, 0x03, 0x80, 0x0C, 0x09, 0x78, 0x0A, 0x1E, 0xAC, 0x98, 0x59, 0x56, 0x85, 0x28, 
0x29, 0x52, 0x57, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 
0x96, 0x00, 0x81, 0x60, 0x00, 0x00, 0x00, 0x18, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20, 
0x58, 0x2C, 0x25, 0x00, 0x81, 0x49, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x56, 
0x41, 0x2D, 0x31, 0x38, 0x30, 0x39, 0x41, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFD, 
0x00, 0x17, 0x3D, 0x0D, 0x2E, 0x11, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x9C, 
0x02, 0x03, 0x0F, 0x10, 0x42, 0x04, 0x22, 0x67, 0x03, 0x0C, 0x00, 0x10, 0x00, 0x88, 0x2D, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x39

#elif (EDID_SELECT_TABLE == 9)
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x05, 0xE3, 0x79, 0x24, 0x18, 0x05, 0x00, 0x00, 
0x2B, 0x19, 0x01, 0x03, 0x80, 0x34, 0x1D, 0x78, 0x2A, 0xEE, 0xD5, 0xA5, 0x55, 0x48, 0x9B, 0x26, 
0x12, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
0x81, 0xC0, 0x01, 0x01, 0x01, 0x01, 0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 
0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x32, 0x4C, 0x1E, 
0x53, 0x11, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x32, 
0x34, 0x37, 0x39, 0x57, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFF, 
0x00, 0x4A, 0x4A, 0x52, 0x46, 0x41, 0x4A, 0x41, 0x30, 0x30, 0x31, 0x33, 0x30, 0x34, 0x01, 0xEC, 
0x02, 0x03, 0x0C, 0x30, 0x41, 0x04, 0x65, 0x03, 0x0C, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x72, 
0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2B, 
#endif
};
#endif

// disable ->static struct IT6602_VIDEO_CONFIGURE_REG _CODE IT6602_OUTPUT_VIDEO_MODE[] =
// disable ->{
// disable ->
// disable ->// { Reg51 , Reg65}
// disable ->//
// disable ->// Reg51 [6] Half PCLK DDR , [5] Half Bus DDR , [2] CCIR656 mode
// disable ->// Reg65 [7] BTA1004Fmt , [6] SyncEmb , [5:4] output color 0x00 RGB, 0x10 YUV422, 0x20 YUV444
// disable ->//
// disable ->// 1. RGB/YUV422/YUV444
// disable ->// 2. SDR/HalfPCLKDDR/HalfBusDDR
// disable ->// 3. EmbSync/SepSync
// disable ->//
// disable ->{0x00,0x00},  //RGB444_SDR=0,
// disable ->{0x00,0x10},  //YUV422_SDR
// disable ->{0x00,0x20},  //YUV444_SDR
// disable ->
// disable ->{0x00,0x50},  //YUV422_SDR_Emb_Sync
// disable ->{0x04,0x50},  //CCIR656_SDR_Emb_Sync
// disable ->{0x04,0x10},  //CCIR656_SDR_Sep_Sync
// disable ->
// disable ->{0x40,0x00},  //RGB444_HalfPCLKDDR
// disable ->{0x40,0x20},  //YUV444_HalfPCLKDDR
// disable ->{0x40,0x50},  //YUV422_HalfPCLKDDR_Emb_Sync
// disable ->{0x40,0x10},  //YUV422_HalfPCLKDDR_Sep_Sync
// disable ->
// disable ->{0x44,0x10},  //CCIR656_HalfPCLKDDR_Sep_Sync
// disable ->
// disable ->{0x20,0x00},  //RGB444_HalfBusDDR
// disable ->{0x20,0x20},  //YUV444_HalfBusDDR
// disable ->
// disable ->{0x00,0xD0},  //BTA1004_SDR
// disable ->{0x40,0xD0},  //BTA1004_DDR
// disable ->};


_CODE char *VStateStr[] = {
	"VSTATE_Off",
	"VSTATE_TerminationOff",
	"VSTATE_TerminationOn",
	"VSTATE_5VOff",
	"VSTATE_SyncWait",
	"VSTATE_SWReset",
	"VSTATE_SyncChecking",
	"VSTATE_HDCPSet",
	"VSTATE_HDCP_Reset",
	"VSTATE_ModeDetecting",
	"VSTATE_VideoOn",
	"VSTATE_ColorDetectReset",
	"VSTATE_HDMI_OFF",
	"VSTATE_Reserved",
    };

_CODE char  *AStateStr[] = {
	"ASTATE_AudioOff",
	"ASTATE_RequestAudio",
	"ASTATE_ResetAudio",
	"ASTATE_WaitForReady",
	"ASTATE_AudioOn",
	"ASTATE_Reserved"
};


_CODE char  *VModeStateStr[] = {
	"0 eRGB444_SDR",
	"1 eYUV444_SDR",
	"2 eRGB444_DDR",
	"3 eYUV444_DDR",
	"4 eYUV422_Emb_Sync_SDR",
	"5 eYUV422_Emb_Sync_DDR",
	"6 eYUV422_Sep_Sync_SDR",
	"7 eYUV422_Sep_Sync_DDR",
	"8 eCCIR656_Emb_Sync_SDR",
	"9 eCCIR656_Emb_Sync_DDR",
	"10 eCCIR656_Sep_Sync_SDR",
	"11 eCCIR656_Sep_Sync_DDR",
	"12 eRGB444_Half_Bus",
	"13 eYUV444_Half_Bus",
	"14 eBTA1004_SDR",
	"15 eBTA1004_DDR",
};


#ifdef _SUPPORT_HDCP_REPEATER_
//FIX_ID_032 xxxxx	//Support HDCP Repeater function for HDMI Tx device

#define _PRINT_HDMI_RX_HDCP_
#define HDMIRX_MAX_KSV 8
#define rol(x,y) (((x) << (y)) | (((unsigned long)x) >> (32-y)))

#define RXHDCP_DEBUG_PRINT(x) printf x


unsigned char	SHABuff[64];
unsigned char	Vr[20];
unsigned long	w[80];
unsigned char	m_bHDCPrepeater=FALSE;
unsigned char    m_RxHDCPstatus=0;

#ifdef _PSEUDO_HDCP_REPEATER_TEST_
static  _CODE BYTE TX_KSVList[]=
{
0x2E,0x17,0x6A,0x79,0x35,
0x0F,0xE2,0x71,0x8E,0x47,
};

static _CODE BYTE TX_BKSV[]=
{
0xA6,0x97,0x53,0xE8,0x74,
};

static _CODE WORD TX_BSTATUS = 0x102;
#endif
//FIX_ID_032 xxxxx
#endif

//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
unsigned char	m_MHLabortID = 0x00;

// default: do nothing
// 1 test retry HPD with MSC abort command
// 2 test retry Path_EN with MSC abort command
// 3 test retry DCap Ready with MSC abort command
// 4 test retry Read DCap register with MSC abort command
#define MHL_ABORT_ID	(0x00)

//FIX_ID_037 xxxxx

/*****************************************************************************/
/*  Function Definitions    **************************************************/
/*****************************************************************************/
//#define VideoTimeOutCheck(x) TimeOutCheck(it6602->m_VideoCountingTimer, (x))
//#define AudioTimeOutCheck(x) TimeOutCheck(it6602->m_AudioCountingTimer, (x))
//#define AssignSWResetVirtualTime() { it6602->m_SWResetTimeOut     = GetCurrentVirtualTime(); }
//#define AssignVideoVirtualTime()   { it6602->m_VideoCountingTimer = GetCurrentVirtualTime(); }
//#define AssignAudioVirtualTime()   { it6602->m_AudioCountingTimer = GetCurrentVirtualTime();}
//#define AssignVideoTimerTimeout(TimeOut) {it6602->m_VideoCountingTimer = (TimeOut);}
//#define AssignAudioTimerTimeout(TimeOut) {it6602->m_AudioCountingTimer = (TimeOut);}



/*****************************************************************************/
/*  Function Prototypes    **************************************************/
/*****************************************************************************/

/* ITEHDMI IO Functions   ***********************************************************/
static SYS_STATUS EDID_RAM_Write(unsigned char offset,unsigned char byteno,_CODE unsigned char *p_data );
static unsigned char EDID_RAM_Read(unsigned char offset);
static unsigned char hdmirxrd( unsigned char RegAddr);
static unsigned char hdmirxwr( unsigned char RegAddr,unsigned char DataIn);
static unsigned char  hdmirxset( unsigned char  offset, unsigned char  mask, unsigned char  ucdata );
static void hdmirxbwr( unsigned char offset, unsigned char byteno, _CODE unsigned char *rddata );

//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_
static unsigned char mhlrxrd( unsigned char offset );
static unsigned char mhlrxwr( unsigned char offset, unsigned char ucdata );
static unsigned char mhlrxset( unsigned char offset, unsigned char mask, unsigned char ucdata );
static void mhlrxbrd( unsigned char offset, unsigned char byteno, unsigned char *rddata );
static void mhlrxbwr( unsigned char offset, unsigned char byteno, unsigned char *rddata );
static void mhlrx_write_init(struct IT6602_REG_INI _CODE *tdata);
#endif
//FIX_ID_036	xxxxx

/* ITEHDMI Configuration and Initialization ***********************************/
struct it6602_dev_data* get_it6602_dev_data(void);
static void hdimrx_write_init(struct IT6602_REG_INI _CODE *tdata);
//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
//static void mhlrx_write_init(struct IT6602_REG_INI _CODE *tdata);
//FIX_ID_036	xxxxx

//FIX_ID_003 xxxxx	//Add IT6602 Video Output Configure setting
static void IT6602_VideoOutputConfigure_Init(struct it6602_dev_data *it6602,Video_Output_Configure eVidOutConfig);
//FIX_ID_003 xxxxx

static void hdmirx_Var_init(struct it6602_dev_data *it6602);
static void IT6602_Rst( struct it6602_dev_data *it6602 );
//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
static void IT6602_Identify_Chip(void);
//FIX_ID_002 xxxxx
//void IT6602_fsm_init(void);
//FIX_ID_016 xxxxx Support Dual Pixel Mode for IT66023 Only
#if defined(_IT66023_)
void IT66023OutputPixelModeSet(unsigned char bSignalPixelMode);
void IT66023JudgeOutputMode(void);
#endif
//FIX_ID_016 xxxxx

/* HDMI RX functions   *********************************************************/
static void chgbank(int bank);
static unsigned char CheckSCDT(struct it6602_dev_data *it6602);
static void WaitingForSCDT(struct it6602_dev_data *it6602);
static unsigned char CLKCheck(unsigned char ucPortSel);

//FIX_ID_009 xxxxx	//verify interrupt event with reg51[0] select port
static unsigned char  IT6602_IsSelectedPort(unsigned char ucPortSel);
//FIX_ID_009 xxxxx

//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
#ifdef _SUPPORT_EQ_ADJUST_
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void HDMIStartEQDetect(struct it6602_eq_data *ucEQPort);
static void HDMISetEQValue(struct it6602_eq_data *ucEQPort,unsigned char ucIndex);
static void HDMISwitchEQstate(struct it6602_eq_data *ucEQPort,unsigned char state);
static void HDMICheckSCDTon(struct it6602_eq_data *ucEQPort);
static void HDMIPollingErrorCount(struct it6602_eq_data *ucEQPort);
static void HDMIJudgeECCvalue(struct it6602_eq_data *ucEQPort);
static void HDMIAdjustEQ(struct it6602_eq_data *ucEQPort);
//FIX_ID_010 xxxxx 	//Add JudgeBestEQ to avoid wrong EQ setting
void JudgeBestEQ(struct it6602_eq_data *ucEQPort);
static void StoreEccCount(struct it6602_eq_data *ucEQPort);
//FIX_ID_010 xxxxx
static void IT6602VideoCountClr(void);
//-------------------------------------------------------------------------------------------------------
#endif

#ifdef _SUPPORT_AUTO_EQ_
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void DisableOverWriteRS (unsigned char ucPortSel);
static void AmpValidCheck (unsigned char ucPortSel);
static void TogglePolarity (unsigned char ucPortSel);
static void TMDSCheck(unsigned char ucPortSel);
static void OverWriteAmpValue2EQ (unsigned char ucPortSel);
//-------------------------------------------------------------------------------------------------------
#endif
//FIX_ID_001 xxxxx
static unsigned char CheckAVMute(void);
//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
static unsigned char CheckPlg5VPwr(unsigned char ucPortSel);
//FIX_ID_037 xxxxx
static unsigned char IsHDMIMode(void);
static void GetAVIInfoFrame(struct it6602_dev_data *it6602);
static void SetVideoInputFormatWithInfoFrame(struct it6602_dev_data *it6602);
static void SetColorimetryByInfoFrame(struct it6602_dev_data *it6602);
static void SetCSCBYPASS(struct it6602_dev_data *it6602);
static void SetColorSpaceConvert(struct it6602_dev_data *it6602);
static void SetNewInfoVideoOutput(struct it6602_dev_data *it6602);
static void SetVideoInputFormatWithoutInfoFrame(struct it6602_dev_data *it6602,unsigned char bInMode);
static void SetColorimetryByMode(struct it6602_dev_data *it6602);
static void SetDVIVideoOutput(struct it6602_dev_data *it6602);

//FIX_ID_003 xxxxx	//Add IT6602 Video Output Configure setting
static void IT6602_VideoOutputModeSet(struct it6602_dev_data *it6602);
//FIX_ID_003 xxxxx

static void IT6602VideoOutputConfigure(struct it6602_dev_data *it6602);
static void SetVideoOutputColorFormat(struct it6602_dev_data *it6602);
//void it6602PortSelect(unsigned char ucPortSel);

static void hdmirx_ECCTimingOut(unsigned char ucport);

/* HDMI Audio function    *********************************************************/
static void aud_fiforst( void );
static void IT6602AudioOutputEnable(unsigned char bEnable);
static void hdmirx_ResetAudio(void);
static void hdmirx_SetHWMuteClrMode(void);
static void hdmirx_SetHWMuteClr(void);
static void hdmirx_ClearHWMuteClr(void);
static void getHDMIRXInputAudio(AUDIO_CAPS *pAudioCaps);
static void IT6602SwitchAudioState(struct it6602_dev_data *it6602,Audio_State_Type state);
#ifdef _FIX_ID_028_
//FIX_ID_028 xxxxx //For Debug Audio error with S2
// remove --> static void IT6602AudioHandler(struct it6602_dev_data *it6602);
//FIX_ID_028 xxxxx
#else
static void IT6602AudioHandler(struct it6602_dev_data *it6602);
#endif

#ifdef EnableCalFs
//FIX_ID_023 xxxxx		//Fixed for Audio Channel Status Error with invalid HDMI source
static void AudioFsCal(void);
static unsigned int PCLKGet(void);
static void TMDSGet(void);
void DumpNCTSReg(void);
//FIX_ID_023 xxxxx
#endif

/* HDMI Video function    *********************************************************/
static void IT6602_AFE_Rst( void );

//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
//xxxxx 2014-0529 //Content On/Off
static void IT6602_HDCP_ContentOff(unsigned char ucPort , unsigned char bOff);
static void IT6602_RAPContentOff(unsigned char bOff);
//xxxxx 2014-0529
//FIX_ID_037 xxxxx

static void IT6602_SetVideoMute(struct it6602_dev_data *it6602,unsigned char bMute);
//static void IT6602VideoOutputCDSet(void);
static void IT6602VideoOutputEnable(unsigned char bEnableOutput);
static void IT6602VideoCountClr(void);
static void IT6602SwitchVideoState(struct it6602_dev_data *it6602,Video_State_Type  eNewVState);
static void IT6602VideoHandler(struct it6602_dev_data *it6602);


/* HDMI Interrupt function    *********************************************************/
static void hdmirx_INT_5V_Pwr_Chg(struct it6602_dev_data *it6602,unsigned char ucport);
static void hdmirx_INT_P0_ECC(struct it6602_dev_data *it6602);
static void hdmirx_INT_P1_ECC(struct it6602_dev_data *it6602);
static void hdmirx_INT_P0_Deskew(struct it6602_dev_data *it6602);
static void hdmirx_INT_P1_Deskew(struct it6602_dev_data *it6602);
//FIX_ID_009 xxxxx	//verify interrupt event with reg51[0] select port
static void hdmirx_INT_HDMIMode_Chg(struct it6602_dev_data *it6602,unsigned char ucport);
//FIX_ID_009 xxxxx
static void hdmirx_INT_SCDT_Chg(struct it6602_dev_data *it6602);

//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
#ifdef _SUPPORT_AUTO_EQ_
static void hdmirx_INT_EQ_FAIL(struct it6602_dev_data *it6602,unsigned char ucPortSel);
#endif
//FIX_ID_001 xxxxx


/* MHL HDCP functions    *********************************************************/
// disable -> static void hdcpsts( void );

/* MHLRX functions    *********************************************************/
//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_

#ifdef _SUPPORT_RCP_
static void parse_rcpkey( unsigned char rcpkey );
static void mhl_parse_RCPkey(struct it6602_dev_data *it6602);
#endif

#ifdef _SUPPORT_RAP_
static void mhl_parse_RAPkey(struct it6602_dev_data *it6602);
#endif

#ifdef _SUPPORT_RAP_
static void ucp_report_event( unsigned char key);
static void mhl_parse_UCPkey(struct it6602_dev_data *it6602);
#endif

static void mhl_read_mscmsg( struct it6602_dev_data *it6602 );
static int mscWait( void );
static int mscFire( int offset, int wdata );
static int cbus_send_mscmsg( struct it6602_dev_data *it6602 );

/*  MHL interrupt    *******************************************************/
static void parse_devcap(unsigned char *devcap );
static int read_devcap_hw( struct it6602_dev_data *it6602 );
static int set_mhlint( unsigned char offset, unsigned char field );
static int set_mhlsts( unsigned char offset, unsigned char status );
static void write_burst(struct it6602_dev_data *it6602, int offset, int byteno );
static void set_wrburst_data(struct it6602_dev_data *it6602, int offset, int byteno );

/* MHL 3D functions    *******************************************************/
//static void v3d_burst1st(struct it6602_dev_data *it6602);
//static void v3d_burst2nd(struct it6602_dev_data *it6602);
//static void v3d_burst3rd(struct it6602_dev_data *it6602);
//static void v3d_burst4th(struct it6602_dev_data *it6602);
//static void v3d_unsupport1st(struct it6602_dev_data *it6602);
//static void v3d_unsupport2nd(struct it6602_dev_data *it6602);

#ifdef FIX_ID_013_
//FIX_ID_013	xxxxx	//For MSC 3D request issue
#ifdef  _SUPPORT_EDID_RAM_
static struct PARSE3D_STR* get_EDID_VSDB_3Ddata(void);
static void EDID_ParseVSDB_3Dblock(struct PARSE3D_STR *pstParse3D);
#endif
static void Msc_3DProcess(MHL3D_STATE *e3DReqState);
static MHL3D_STATE MSC_3DInforSend(unsigned char b3dDtd);
static unsigned char Msc_WriteBurstDataFill(unsigned char ucOffset, unsigned char ucByteNo, unsigned char *pucData);
//FIX_ID_013	xxxxx
#else
static void v3d_unsupport1st(struct it6602_dev_data *it6602);
static void v3d_unsupport2nd(struct it6602_dev_data *it6602);
#endif //FIX_ID_013



#if 1
//FIX_ID_021 xxxxx		//To use CP_100ms for CBus_100ms and CEC_100m
//FIX_ID_004 xxxxx 		//Add 100ms calibration for Cbus
//#ifndef _SelectExtCrystalForCbus_
static unsigned long m_ROSCCLK;
static void Cal_oclk( void );
//#endif
//FIX_ID_004
#define CP_MEAN_VALUE		(48696)
#define CP_MAX_DEVIATION	(CP_MEAN_VALUE*15/100)
//#define CP_DEFAULT_VALUE	(CP_MEAN_VALUE*100)
static unsigned char OSCvalueCompare(unsigned long *calibrationValue);
static unsigned long CP_OCLK( void );
//FIX_ID_021 xxxxx
#endif



/* RCP functions    *******************************************************/
#ifdef _SUPPORT_RCP_
static void RCPinitQ(struct it6602_dev_data *it6602);
static int RCPKeyPop(struct it6602_dev_data *it6602);
static void SwitchRCPResult(struct it6602_dev_data *it6602,RCPResult_Type RCPResult);
static void SwitchRCPStatus(struct it6602_dev_data *it6602,RCPState_Type RCPState);
static void RCPManager(struct it6602_dev_data *it6602);

//FIX_ID_005 xxxxx	//Add Cbus Event Handler
static void IT6602CbusEventManager(struct it6602_dev_data *it6602);
//FIX_ID_005 xxxxx


//FIX_ID_014 xxxxx
static void IT6602HDMIEventManager(struct it6602_dev_data *it6602);
//FIX_ID_014 xxxxx

//void RCPKeyPush(unsigned char ucKey);

//FIX_ID_018	xxxxx	//modify 1K pull-down to 1.033K ohm HDMI Reg1C0[3:2]=2
static void IT6602_WakeupProcess(void);
//FIX_ID_018 xxxxx



//FIX_ID_024 xxxxx		//Fixed for RCP compliance issue
static void WaitRCPresponse(struct it6602_dev_data *it6602);
//FIX_ID_024 xxxxx
#endif

/* Driver State Machine Process **********************************************/
static void IT6602MHLInterruptHandler(struct it6602_dev_data *it6602);

#endif
//FIX_ID_036	xxxxx

/* EDID RAM  functions    *******************************************************/
#ifdef _SUPPORT_EDID_RAM_
//static unsigned char UpdateEDIDRAM(_CODE unsigned char *pEDID,unsigned char BlockNUM);
static unsigned char UpdateEDIDRAM(unsigned char *pEDID,unsigned char BlockNUM);
static void EnableEDIDupdata(void);
static void DisableEDIDupdata(void);
//static void EDIDRAMInitial(_CODE unsigned char *pIT6602EDID);
static void EDIDRAMInitial(unsigned char *pIT6602EDID);
//static unsigned char Find_Phyaddress_Location(_CODE unsigned char *pEDID,unsigned char Block_Number);
static unsigned char Find_Phyaddress_Location(unsigned char *pEDID,unsigned char Block_Number);
static void UpdateEDIDReg(unsigned char u8_VSDB_Addr, unsigned char CEC_AB, unsigned char CEC_CD, unsigned char Block1_CheckSum);
static void PhyAdrSet(void);
#endif






/* Driver State Machine Process **********************************************/
//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
//static void IT6602MHLInterruptHandler(struct it6602_dev_data *it6602);
//FIX_ID_036	xxxxx
static void IT6602HDMIInterruptHandler(struct it6602_dev_data *it6602);
//void IT6602_fsm(void);


#ifndef Enable_IR
static void it6602AutoPortSelect(struct it6602_dev_data *it6602);
#endif
// disable -> static void it6602ShowErrorCount(void);
//void show_vid_info( void );
//void get_vid_info( void );

#ifdef Enable_Vendor_Specific_packet
static void Dump3DReg(void);
static unsigned char IT6602_DE3DFrame(unsigned char ena_de3d);
#endif

//
//#ifdef MEGAWIN82516
//static void it6602AutoPortSelect(void);
//#endif

/*****************************************************************************/
/*  IO Functions   ***********************************************************/
/*****************************************************************************/

static SYS_STATUS EDID_RAM_Write(unsigned char offset,unsigned char byteno, _CODE unsigned char *p_data )
{
    
    IT_66021_WriteBytes(RX_I2C_EDID_MAP_ADDR, offset, byteno, p_data);
}

static unsigned char EDID_RAM_Read(unsigned char offset)
{
    return IT_66021_ReadByte(RX_I2C_EDID_MAP_ADDR,offset);
}

//===============================================================================//
static unsigned char IT6602VersionRead( unsigned char RegAddr)
{
    return IT_66021_ReadByte(HdmiI2cAddr,RegAddr);
}


static unsigned char hdmirxrd( unsigned char RegAddr)
{
    return IT_66021_ReadByte(HdmiI2cAddr,RegAddr);
}

static unsigned char hdmirxwr( unsigned char RegAddr,unsigned char DataIn)
{    
    return IT_66021_WriteByte(HdmiI2cAddr, RegAddr, DataIn);
}

static unsigned char  hdmirxset( unsigned char  offset, unsigned char  mask, unsigned char  ucdata )
{
    unsigned char  temp;
    temp = hdmirxrd(offset);
    temp = (temp&((~mask)&0xFF))+(mask&ucdata);
    return hdmirxwr(offset, temp);
}

static void hdmirxbwr( unsigned char offset, unsigned char byteno, _CODE unsigned char *rddata )
{
    IT_66021_WriteBytes(HdmiI2cAddr, offset, byteno, rddata);
}

static unsigned char mhlrxrd( unsigned char offset )
{
	return IT_66021_ReadByte(MHL_ADDR,offset);	
}

static unsigned char mhlrxwr( unsigned char offset, unsigned char ucdata )
{
	return IT_66021_WriteByte(MHL_ADDR, offset, ucdata);
}

//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_
//===============================================================================//
static unsigned char mhlrxrd( unsigned char offset )
{
	unsigned char	mDataIn;
	unsigned char	FLAG;
	FLAG=i2c_read_byte(MHL_ADDR, offset, 1, &mDataIn, HDMI_DEV);
	if(FLAG==0)
	{
	DLOG_INFO("====================================\n");
	DLOG_INFO("MHL I2C ERROR !!!");
	DLOG_INFO("=====  read Reg0x%X=%X =====  \n",(int)offset,(int)mDataIn);
	DLOG_INFO("====================================\n");
	}
	return mDataIn;
}

static unsigned char mhlrxwr( unsigned char offset, unsigned char ucdata )
{
	unsigned char  flag;
	flag= i2c_write_byte(MHL_ADDR, offset, 1, &ucdata, HDMI_DEV);
	if(flag==0)
	{
	DLOG_INFO("====================================\n");
	DLOG_INFO("MHL I2C ERROR !!!");
	DLOG_INFO("=====  Write Reg0x%X=%X =====  \n",(int)offset,(int)ucdata);
	DLOG_INFO("====================================\n");
	}
	return !flag;
}

static unsigned char mhlrxset( unsigned char offset, unsigned char mask, unsigned char ucdata )
{
	unsigned char temp;
	temp = mhlrxrd(offset);
	temp = (temp&((~mask)&0xFF))+(mask&ucdata);
	return mhlrxwr(offset, temp);
}

static void mhlrxbrd( unsigned char offset, unsigned char byteno, unsigned char *rddata )
{
	if( byteno>0 )
	i2c_read_byte(MHL_ADDR, offset, byteno, rddata, HDMI_DEV);
}

static void mhlrxbwr( unsigned char offset, unsigned char byteno, unsigned char *rddata )
{
	if( byteno>0 )
	i2c_write_byte(MHL_ADDR, offset, byteno, rddata, HDMI_DEV);
}
#endif
//FIX_ID_036	xxxxx

/*****************************************************************************/
/* ITEHDMI Configuration and Initialization ***********************************/
/*****************************************************************************/
#ifdef _ITEHDMI_
struct it6602_dev_data* get_it6602_dev_data(void)
{
	return &it6602DEV;
}


static void hdimrx_write_init(struct IT6602_REG_INI _CODE *tdata)
{
	int cnt = 0;
	while(tdata[cnt].ucAddr != 0xFF)
	{
		//printf(" Cnt = %d, addr = %02X,andmask = %02X,ucValue=%02X\n", cnt,(int)tdata[cnt].ucAddr,(int)tdata[cnt].andmask,(int)tdata[cnt].ucValue);
		hdmirxset(tdata[cnt].ucAddr,tdata[cnt].andmask,tdata[cnt].ucValue);
		cnt++;

	}
}

//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_
static void mhlrx_write_init(struct IT6602_REG_INI _CODE *tdata)
{
	int cnt = 0;
	while(tdata[cnt].ucAddr != 0xFF)
	{
		//printf(" Cnt = %d, addr = %02X,andmask = %02X,ucValue=%02X\n", cnt,(int)tdata[cnt].ucAddr,(int)tdata[cnt].andmask,(int)tdata[cnt].ucValue);
		mhlrxset(tdata[cnt].ucAddr,tdata[cnt].andmask,tdata[cnt].ucValue);
		cnt++;
	}
}
#endif
//FIX_ID_036	xxxxx

//FIX_ID_003 xxxxx	//Add IT6602 Video Output Configure setting
static void IT6602_VideoOutputConfigure_Init(struct it6602_dev_data *it6602,Video_Output_Configure eVidOutConfig)
{
	it6602->m_VidOutConfigMode=eVidOutConfig;

	switch(eVidOutConfig)
	{
		case eRGB444_SDR:
			it6602->m_bOutputVideoMode = F_MODE_RGB444;
//FIX_ID_027 xxxxx Support Full/Limited Range convert
			it6602->m_bOutputVideoMode = F_MODE_RGB444|F_MODE_0_255;      // ITEHDMI output RGB Full Range
			// it6602->m_bOutputVideoMode = F_MODE_RGB444|F_MODE_16_235;    // ITEHDMI output RGB limited Range
//FIX_ID_027 xxxxx
			it6602->m_VidOutDataTrgger=eSDR;
			it6602->m_VidOutSyncMode=eSepSync;
			break;

		case eYUV444_SDR:
			it6602->m_bOutputVideoMode=F_MODE_YUV444;
			it6602->m_VidOutDataTrgger=eSDR;
			it6602->m_VidOutSyncMode=eSepSync;
			break;

		case eRGB444_DDR:
			it6602->m_bOutputVideoMode=F_MODE_RGB444;
			it6602->m_VidOutDataTrgger=eHalfPCLKDDR;
			it6602->m_VidOutSyncMode=eSepSync;
			break;

		case eYUV444_DDR:
			it6602->m_bOutputVideoMode=F_MODE_YUV444;
			it6602->m_VidOutDataTrgger=eHalfPCLKDDR;
			it6602->m_VidOutSyncMode=eSepSync;
			break;

		case eYUV422_Emb_Sync_SDR:
			it6602->m_bOutputVideoMode=F_MODE_YUV422;
			it6602->m_VidOutDataTrgger=eSDR;
			it6602->m_VidOutSyncMode=eEmbSync;
			break;

		case eYUV422_Emb_Sync_DDR:
			it6602->m_bOutputVideoMode=F_MODE_YUV422;
			it6602->m_VidOutDataTrgger=eHalfPCLKDDR;
			it6602->m_VidOutSyncMode=eEmbSync;
			break;

		case eYUV422_Sep_Sync_SDR:
			it6602->m_bOutputVideoMode=F_MODE_YUV422;
			it6602->m_VidOutDataTrgger=eSDR;
			it6602->m_VidOutSyncMode=eSepSync;
			break;

		case eYUV422_Sep_Sync_DDR:
			it6602->m_bOutputVideoMode=F_MODE_YUV422;
			it6602->m_VidOutDataTrgger=eHalfPCLKDDR;
			it6602->m_VidOutSyncMode=eSepSync;
			break;

		case eCCIR656_Emb_Sync_SDR:
			it6602->m_bOutputVideoMode=F_MODE_YUV422;
			it6602->m_VidOutDataTrgger=eSDR;
			it6602->m_VidOutSyncMode=eCCIR656EmbSync;
			break;

		case eCCIR656_Emb_Sync_DDR:
			it6602->m_bOutputVideoMode=F_MODE_YUV422;
			it6602->m_VidOutDataTrgger=eHalfPCLKDDR;
			it6602->m_VidOutSyncMode=eCCIR656EmbSync;
			break;

		case eCCIR656_Sep_Sync_SDR:
			it6602->m_bOutputVideoMode=F_MODE_YUV422;
			it6602->m_VidOutDataTrgger=eSDR;
			it6602->m_VidOutSyncMode=eCCIR656SepSync;
			break;

		case eCCIR656_Sep_Sync_DDR:
			it6602->m_bOutputVideoMode=F_MODE_YUV422;
			it6602->m_VidOutDataTrgger=eHalfPCLKDDR;
			it6602->m_VidOutSyncMode=eCCIR656SepSync;
			break;

		case eRGB444_Half_Bus:
			it6602->m_bOutputVideoMode=F_MODE_RGB444;
			it6602->m_VidOutDataTrgger=eHalfBusDDR;
			it6602->m_VidOutSyncMode=eSepSync;
			break;

		case eYUV444_Half_Bus:
			it6602->m_bOutputVideoMode=F_MODE_YUV444;
			it6602->m_VidOutDataTrgger=eHalfBusDDR;
			it6602->m_VidOutSyncMode=eSepSync;
			break;

		case eBTA1004_SDR:	//BTA1004_SDR_Emb_Sync
			it6602->m_bOutputVideoMode=F_MODE_YUV422;
			it6602->m_VidOutDataTrgger=eSDR_BTA1004;
			it6602->m_VidOutSyncMode=eEmbSync;
			break;

		case eBTA1004_DDR:  //BTA1004_DDR_Emb_Sync
			it6602->m_bOutputVideoMode=F_MODE_YUV422;
			it6602->m_VidOutDataTrgger=eDDR_BTA1004;		// eHalfPCLKDDR
			it6602->m_VidOutSyncMode=eEmbSync;
			break;

	}

	// only for debug use !!!
	//IT6602_VideoOutputModeSet(it6602);

}
//FIX_ID_003 xxxxx

////////////////////////////////////////////////////////////////////
//int hdmirx_Var_init( void )
//
//
//
////////////////////////////////////////////////////////////////////
static void hdmirx_Var_init(struct it6602_dev_data *it6602)
{

    // it6602->m_ucCurrentHDMIPort=0xFF;
    //it6602->m_ucDVISCDToffCNT=0;
    it6602->m_ucSCDTOffCount=0;
    it6602->m_ucEccCount_P0=0;
    it6602->m_ucEccCount_P1=0;
    it6602->m_ucDeskew_P0=0;
    it6602->m_ucDeskew_P1=0;

    it6602->m_VState=VSTATE_Off;
    it6602->m_AState=ASTATE_AudioOff;
    it6602->m_RxHDCPState=RxHDCP_PwrOff;

    it6602->m_SWResetTimeOut=0;
    it6602->m_VideoCountingTimer=0;
    it6602->m_AudioCountingTimer=0;

    it6602->m_bVideoOnCountFlag=FALSE;

    it6602->m_MuteAutoOff=FALSE;
    it6602->m_bUpHDMIMode=FALSE;
    it6602->m_bUpHDCPMode=FALSE;
    it6602->m_NewAVIInfoFrameF=FALSE;
    it6602->m_NewAUDInfoFrameF=FALSE;
    it6602->m_HDCPRepeater=FALSE;

//06-27 disable -->     it6602->m_bOutputVideoMode=HDMIRX_OUTPUT_VID_MODE ;

//FIX_ID_003 xxxxx	//Add IT6602 Video Output Configure Table
    IT6602_VideoOutputConfigure_Init(it6602,HDMIRX_OUTPUT_VID_MODE);
//FIX_ID_003 xxxxx


    it6602->m_bRxAVmute=FALSE;

//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
    #ifdef _SUPPORT_EQ_ADJUST_
    it6602->EQPort[0].ucEQState=0xFF;
    it6602->EQPort[0].ucAuthR0=0;
    it6602->EQPort[0].ucECCvalue=0;
    it6602->EQPort[0].ucECCfailCount=0;
    it6602->EQPort[0].ucPkt_Err=0;	//Pkt_Err
    it6602->EQPort[0].ucPortID=F_PORT_SEL_0;

    it6602->EQPort[1].ucEQState=0xFF;
    it6602->EQPort[1].ucAuthR0=0;
    it6602->EQPort[1].ucECCvalue=0;
    it6602->EQPort[1].ucECCfailCount=0;
    it6602->EQPort[1].ucPkt_Err=0;
    it6602->EQPort[1].ucPortID=F_PORT_SEL_1;

    it6602->EQPort[0].f_manualEQadjust=FALSE;
    it6602->EQPort[1].f_manualEQadjust=FALSE;

    #endif

    #ifdef _SUPPORT_AUTO_EQ_

    ucPortAMPOverWrite[1]=0;	//2013-0801
    ucPortAMPValid[1]=0;
    ucChannelB[1]=0;
    ucChannelG[1]=0;
    ucChannelR[1]=0;

    ucPortAMPOverWrite[0]=0;	//2013-0801
    ucPortAMPValid[0]=0;
    ucChannelB[0]=0;
    ucChannelG[0]=0;
    ucChannelR[0]=0;
    #endif
//FIX_ID_001 xxxxx

//FIX_ID_005 xxxxx 	//Add Cbus Event Handler
	it6602->CBusIntEvent=0;
	it6602->CBusSeqNo=0;
	it6602->CBusWaitNo=0x00;
//FIX_ID_005 xxxxx

//FIX_ID_005 xxxxx 	//Add Cbus Event Handler
	it6602->HDMIIntEvent=0;
	it6602->HDMIWaitNo[0]=0;
	it6602->HDMIWaitNo[1]=0;
//FIX_ID_005 xxxxx


    #ifdef _IT6607_GeNPacket_Usage_
    it6602->m_PollingPacket=0;
    it6602->m_PacketState=0;
    it6602->m_ACPState=0;
    it6602->m_GamutPacketRequest=FALSE;
    it6602->m_GeneralRecPackType=0x00;
    #endif
	it6602->m_ucCurrentHDMIPort = 0xff;

//FIX_ID_034 xxxxx //Add MHL HPD Control by it6602HPDCtrl( )
    it6602->m_DiscoveryDone = 0;
//FIX_ID_034 xxxxx

//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
//xxxxx 2014-0529 //Manual Content On/Off
   it6602->m_RAP_ContentOff = 0;
   it6602->m_HDCP_ContentOff = 0;
//xxxxx 2014-0529
//FIX_ID_037 xxxxx

}



////////////////////////////////////////////////////////////////////
//void hdmitx_rst( void )
//
//\
//
////////////////////////////////////////////////////////////////////
static void IT6602_Rst( struct it6602_dev_data *it6602 )
{
	hdmirx_Var_init(it6602);
	hdimrx_write_init(IT6602_HDMI_INIT_TABLE);
}

//=========================================================================//
static void IT6602_Identify_Chip(void)
{

//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
#if 1
//FIX_ID_002 xxxxx

	unsigned char i,j;
	unsigned char VenID;
	unsigned char Result=0;
	unsigned char acIT6602A0Version[4]={0x54,0x49,0x02,0x68};
	unsigned char I2cAddr[2]={IT6602B0_HDMI_ADDR,IT6602A0_HDMI_ADDR};	// it6602A0 i2c addr = 0x94 , but it6602B0 i2c addr = 0x90 !!!

	do
	{
		for(j=0;j<2;j++)
		{
			HdmiI2cAddr	=I2cAddr[j];

			for(i=0;i<4;i++)
			{
				VenID = hdmirxrd(i);
				DLOG_INFO("VenID[%X]= %X !!!\r\n",(int)i,(int)VenID);
				if(VenID==acIT6602A0Version[i])
				{
					Result=1;
				}
				else
				{
					Result=0;
					DLOG_INFO("I2C Addr %X Error: Can not find IT6602 Chip !!!\r\n",(int)HdmiI2cAddr);
					break;
				}
			}
			if(Result==1)
			{
				DLOG_INFO("OK , Find IT6602 Chip I2C Addr %X !!!\r\n",(int)HdmiI2cAddr);
				break;
			}
		}
	}
	while(Result==0);
#endif
}

//FIX_ID_016 xxxxx Support Dual Pixel Mode for IT66023 Only
#if defined(_IT6602_) || defined(_IT66023_)
void IT66023OutputPixelModeSet(unsigned char bSignalPixelMode)
{
	chgbank(0);
	if(bSignalPixelMode)

	{
		DLOG_INFO("IT66023OutputPixelModeSet( ) 111111111111 Signal Pixel Mode\r\n");
		// 1 .	Signal Pixel Mode
		// Reg0D[3] = 0    // EnPHFClk�G0 for disable PHFCLK for signal pixel mode
		// Reg8C[0] = 1	// SPOutMode�G//  for enable IO Mapping for Signal Pixel mode
		// Reg8C[3] = 1    // VDIO3en�G//  for enable QA IO
		// Reg8B[0] = 0    // EnDualPixel�G0 for disable Dual Pixel mode
		// Reg8B[7] = 1    // PWDDownDualData�G//  for gating dual pix data output
		hdmirxset(REG_RX_00D, 0x08, 0x00);
		hdmirxset(REG_RX_08C, 0x09, 0x09);
		hdmirxset(REG_RX_08B, 0x81, 0x80);

	}
	else
	{
		DLOG_INFO("IT66023OutputPixelModeSet( ) 00000000000000 Dual Pixel Mode\r\n");
		// 2.	Dual Pixel Mode
		// Reg0D[3] = 1    // EnPHFClk�G//  for enable PHFCLK for Dual pixel mode
		// Reg8C[0] = 0    // SPOutMode�G0 for Disable IO Mapping for Dual Pixel mode
		// Reg8C[3] = 1    // VDIO3en�G//  for enable QA IO
		// Reg8B[0] = 1    // EnDualPixel�G//  for enable Dual Pixel mode
		// Reg8B[7] = 0    // PWDDownDualData�G0 for PWD UP DUAL DATA
		hdmirxset(REG_RX_00D, 0x08, 0x08);
		hdmirxset(REG_RX_08C, 0x09, 0x08);
		hdmirxset(REG_RX_08B, 0x81, 0x01);
	}
}

void IT66023JudgeOutputMode(void)
{
	unsigned char rddata;
	int PCLK;	//, sump;


	rddata = hdmirxrd(0x9A);
	PCLK=(124*255/rddata)/10;

	DLOG_INFO("IT66023JudgeOutputMode( ) PCLK = %d \r\n",(int) PCLK);

	if(PCLK>160)
	{
		#if defined(_IT66023_)
		IT66023OutputPixelModeSet(FALSE);	// FALSE output Dual pixel mode
		#endif
	}
	else
	{
		#if defined(_IT66023_)
		IT66023OutputPixelModeSet(TRUE);		// TRUE output signal pixel mode
		#endif
	}
}
#endif
//FIX_ID_016 xxxxx


//=========================================================================//
void IT6602_fsm_init(void)
{
	struct it6602_dev_data *it6602data = get_it6602_dev_data();

	DLOG_INFO("IT6602_fsm_init( ) \n");

//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
	IT6602_Identify_Chip();
//FIX_ID_002 xxxxx

	
    IT6602_Rst(it6602data);

//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
// disable -> 	#ifdef _SUPPORT_MANUAL_ADJUST_EQ_
// disable -> 	//!!!  For Manual Adjust EQ only  !!!
// disable -> 	HDMI_EQ_DefaultSet(&(it6602data->EQPort[0]));
// disable -> 	HDMI_EQ_DefaultSet(&(it6602data->EQPort[1]));
// disable -> 	#endif
//FIX_ID_001 xxxxx



//FIX_ID_006 xxxxx 	//Add P2_0 for switch Exteranl 24c04 EEPROM and Internal IT6602 EDID RAM
#ifndef MEGAWIN82516
/*	P2_0=1;
	if(P2_0==0)
	{
		#ifdef FIX_ID_013_
		//printf("!!!Use External EEPROM 24c04 EDID !!!\r\n");
		st3DParse.bVSDBspport3D = 0;
		st3DParse.ucVicCnt = 0;
		st3DParse.ucDtdCnt = 0;
		#endif

		// for Disable EDID RAM function !!!
		hdmirxset(REG_RX_0C0, 0x03, 0x03);
//		hdmirxset(REG_RX_087, 0xFF, 0x00);
	}
	else*/
#endif
	{


		#ifdef  _SUPPORT_EDID_RAM_
			//printf("!!!Use it6602 EDID RAM !!!");

#ifdef _PSEUDO_HDCP_REPEATER_TEST_
			ITEHDMI_RxHDCPmodeChangeRequest(TRUE);	// configure ITEHDMI to HDMI repeater mode
#else
			EDIDRAMInitial(&Default_Edid_Block[0]);
#endif

#ifdef FIX_ID_013_
//FIX_ID_013	xxxxx	//For MSC 3D request issue
			EDID_ParseVSDB_3Dblock(&st3DParse);
//FIX_ID_013	xxxxx
#endif //FIX_ID_013

//FIX_ID_041 xxxxx Add EDID reset
	// fo IT6803 EDID fail issue
	hdmirxset(REG_RX_0C0, 0x20, 0x20);	//xxxxx 2014-0731 [5] 1 for  reset edid
	IT_Delay(1);
	hdmirxset(REG_RX_0C0, 0x20, 0x00);
	// fo IT6803 EDID fail issue
//FIX_ID_041 xxxxx

		#else

			st3DParse.bVSDBspport3D = 0;
			st3DParse.ucVicCnt = 0;
			st3DParse.ucDtdCnt = 0;

			// for Disable EDID RAM function !!!
			hdmirxset(REG_RX_0C0, 0x03, 0x03);
//			hdmirxset(REG_RX_087, 0xFF, 0x00);

		#endif
	}

//xxxxx 2013-0801 disable HPD Control
#if 0
		//set port 0 HPD=1
		chgbank(1);
		hdmirxset(REG_RX_1B0, 0x03, 0x03);
		chgbank(0);

		HotPlug(1);	//set port 1 HPD=1
#endif
//xxxxx
//FIX_ID_006 xxxxx


	it6602PortSelect(0);	// select port 0
}

#endif


/*****************************************************************************/
/* HDMIRX functions    *******************************************************/
/*****************************************************************************/
#ifdef _ITEHDMI_

static void chgbank( int bank )
{
	switch( bank ) {
	case 0 :
		 hdmirxset(0x0F, 0x03, 0x00);
		 break;
	case 1 :
		 hdmirxset(0x0F, 0x03, 0x01);
		 break;
	case 2 :
		 hdmirxset(0x0F, 0x03, 0x02);
		 break;
	case 3:
		 hdmirxset(0x0F, 0x03, 0x03);
		 break;
	default :
		 break;
	}
}


static unsigned char CheckSCDT(struct it6602_dev_data *it6602)
{
	unsigned char ucPortSel;
	unsigned char sys_state_P0;

	ucPortSel = hdmirxrd(REG_RX_051) & B_PORT_SEL;
	sys_state_P0=hdmirxrd(REG_RX_P0_SYS_STATUS);

	if(ucPortSel == it6602->m_ucCurrentHDMIPort) {

		if(sys_state_P0 & B_P0_SCDT)
		{
			//SCDT on
			//it6602->m_ucSCDTOffCount=0;
			return TRUE;
		}
		else
		{
			//SCDT off
			return FALSE;
		}

	}
	return FALSE;
}


static void WaitingForSCDT(struct it6602_dev_data *it6602)
{
	unsigned char sys_state_P0;
	unsigned char sys_state_P1;
	unsigned char ucPortSel;
//	unsigned char ucTMDSClk ;

	sys_state_P0=hdmirxrd(REG_RX_P0_SYS_STATUS) & (B_P0_SCDT|B_P0_PWR5V_DET|B_P0_RXCK_VALID);
	sys_state_P1=hdmirxrd(REG_RX_P1_SYS_STATUS) & (B_P1_SCDT|B_P1_PWR5V_DET|B_P1_RXCK_VALID);
	ucPortSel = hdmirxrd(REG_RX_051) & B_PORT_SEL;

	if(sys_state_P0 & B_P0_SCDT)
	{
		IT6602SwitchVideoState(it6602,VSTATE_SyncChecking);	//2013-0520
		return;
	}
	else
	{
		#ifdef _SUPPORT_EQ_ADJUST_
		if(it6602->EQPort[ucPortSel].f_manualEQadjust==TRUE)		// ignore SCDT off when manual EQ adjust !!!
		{
			return;
		}
		#endif

		if(ucPortSel == F_PORT_SEL_0)
		{

			if((sys_state_P0 & (B_P0_PWR5V_DET|B_P0_RXCK_VALID)) == (B_P0_PWR5V_DET|B_P0_RXCK_VALID))
			{
				it6602->m_ucSCDTOffCount++;
					DLOG_INFO(" SCDT off count = %X \r\n",(int)it6602->m_ucSCDTOffCount);
					DLOG_INFO(" sys_state_P0 = %X \r\n",(int)hdmirxrd(REG_RX_P0_SYS_STATUS));

			}
		}
		else
		{
			if((sys_state_P1 & (B_P1_PWR5V_DET|B_P1_RXCK_VALID)) == (B_P1_PWR5V_DET|B_P1_RXCK_VALID))
			{
				it6602->m_ucSCDTOffCount++;
					DLOG_INFO(" SCDT off count = %X \r\n",(int)it6602->m_ucSCDTOffCount);
					DLOG_INFO(" sys_state_P1 = %X \r\n",(int)hdmirxrd(REG_RX_P1_SYS_STATUS));

			}
		}

		if((it6602->m_ucSCDTOffCount)>SCDT_OFF_TIMEOUT)
		{
			it6602->m_ucSCDTOffCount=0;


//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
//FIX_ID_035	xxxxx //For MTK6592 HDMI to SII MHL TX compliance issue
					DLOG_INFO(" WaitingForSCDT( ) CDR reset !!! \r\n");
					hdmirx_ECCTimingOut(ucPortSel);

 					#ifdef _SUPPORT_AUTO_EQ_
 					//xxxxx
 					DisableOverWriteRS(ucPortSel);
 					TMDSCheck(ucPortSel);
 					//xxxxx
 					#endif

//FIX_ID_035	xxxxx
//FIX_ID_033 xxxxx


					// disable 0502 -> if(ucPortSel == F_PORT_SEL_0)
					// disable 0502 -> {
					// disable 0502 -> //xxxxx 2014-0502 for fix SII MHL TX issue
					// disable 0502 -> hdmirx_ECCTimingOut(0);
					// disable 0502 ->
					// disable 0502 -> DLOG_INFO(" WaitingForSCDT( ) Port 0 CDR reset !!! \r\n");
					// disable 0502 ->
					// disable 0502 -> #ifdef _SUPPORT_AUTO_EQ_
					// disable 0502 -> //xxxxx
					// disable 0502 -> DisableOverWriteRS(0);
					// disable 0502 -> TMDSCheck(0);
					// disable 0502 -> //xxxxx
					// disable 0502 -> #endif
					// disable 0502 -> //xxxxx 2014-0502 for fix SII MHL TX issue
					// disable 0502 ->
					// disable 0502 -> }
					// disable 0502 -> else
					// disable 0502 -> {
					// disable 0502 -> hdmirxset(REG_RX_018,(B_P1_DCLKRST|B_P1_CDRRST),(B_P1_DCLKRST|B_P1_CDRRST|B_P1_SWRST));
					// disable 0502 -> hdmirxset(REG_RX_018,(B_P1_DCLKRST|B_P1_CDRRST),0x00);
					// disable 0502 -> DLOG_INFO(" WaitingForSCDT( ) Port 1 CDR reset !!! \r\n");
					// disable 0502 ->
					// disable 0502 -> //xxxxx
					// disable 0502 -> DisableOverWriteRS(1);
					// disable 0502 -> TMDSCheck(1);
					// disable 0502 -> //xxxxx
					// disable 0502 ->
					// disable 0502 -> }

		}

	}
}

static unsigned char CLKCheck(unsigned char ucPortSel)
{
	unsigned char sys_state;
	if(ucPortSel == F_PORT_SEL_1)
	{
	sys_state = hdmirxrd(REG_RX_P1_SYS_STATUS) & (B_P1_RXCK_VALID);
	}
	else
	{
	sys_state = hdmirxrd(REG_RX_P0_SYS_STATUS) & (B_P0_RXCK_VALID);
	}
	if(sys_state == B_P0_RXCK_VALID)
		return TRUE;
	else
		return FALSE;
}


//---------------------------------------------------------------------------------------------------
//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
#ifdef _SUPPORT_AUTO_EQ_
static void DisableOverWriteRS (unsigned char ucPortSel)
{

//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_
	unsigned char uc;
#endif
//FIX_ID_036	xxxxx

	struct it6602_dev_data *it6602data = get_it6602_dev_data();

	if(ucPortSel == F_PORT_SEL_1)
	{
#ifdef _SUPPORT_AUTO_EQ_
		ucPortAMPOverWrite[F_PORT_SEL_1]=0;	//2013-0801
		ucPortAMPValid[F_PORT_SEL_1]=0;
//FIX_ID_035 xxxxx //For MTK6592 HDMI to SII MHL TX compliance issue
//xxxxx 2014-0508 disable -> 		ucEqRetryCnt[F_PORT_SEL_1]=0;
//FIX_ID_035 xxxxx
		ucEQMode[F_PORT_SEL_1] = 0; // 0 for Auto Mode
		hdmirxset(REG_RX_03A, 0xFF, 0x00);	// 07-16 Reg3A=0x30	power down auto EQ
		hdmirxset(REG_RX_03E,0x20,0x00);		//Manually set RS Value
		DLOG_INFO(" ############# DisableOverWriteRS( ) port 1 ###############\n");
#endif
		#ifdef _SUPPORT_EQ_ADJUST_
		it6602data->EQPort[1].f_manualEQadjust=FALSE;
		it6602data->EQPort[F_PORT_SEL_1].ucEQState=0xFF;
		#endif
		it6602data->m_ucDeskew_P1=0;
		it6602data->m_ucEccCount_P1=0;


		//FIX_ID_014 xxxxx
			it6602data->HDMIIntEvent &=0x0F;;
			it6602data->HDMIWaitNo[F_PORT_SEL_1]=0;
		//FIX_ID_014 xxxxx

	}
	else
	{
#ifdef _SUPPORT_AUTO_EQ_
		ucPortAMPOverWrite[F_PORT_SEL_0]=0;	//2013-0801
		ucPortAMPValid[F_PORT_SEL_0]=0;
//FIX_ID_035 xxxxx //For MTK6592 HDMI to SII MHL TX compliance issue
//xxxxx 2014-0508 disable ->		ucEqRetryCnt[F_PORT_SEL_0]=0;
//FIX_ID_035 xxxxx
		ucEQMode[F_PORT_SEL_0] = 0; // 0 for Auto Mode
		hdmirxset(REG_RX_022, 0xFF, 0x00);	// 07-16 Reg22=0x30	power down auto EQ
		hdmirxset(REG_RX_026,0x20,0x00);		//Manually set RS Value
		DLOG_INFO(" ############# DisableOverWriteRS( ) port 0 ###############\n");
#endif

		#ifdef _SUPPORT_EQ_ADJUST_
		it6602data->EQPort[0].f_manualEQadjust=FALSE;
		it6602data->EQPort[F_PORT_SEL_0].ucEQState=0xFF;
		#endif
		it6602data->m_ucDeskew_P0=0;
		it6602data->m_ucEccCount_P0=0;

		//FIX_ID_014 xxxxx
			it6602data->HDMIIntEvent &=0xF0;;
			it6602data->HDMIWaitNo[F_PORT_SEL_0]=0;
		//FIX_ID_014 xxxxx

//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_

		wakeupcnt = 0;	//07-23

		//FIX_ID_005 xxxxx 	//Add Cbus Event Handler
			it6602data->CBusIntEvent=0;
			it6602data->CBusSeqNo=0;
			it6602data->CBusWaitNo=0x00;
		//FIX_ID_005 xxxxx

		//FIX_ID_019	xxxxx modify ENHYS control for MHL mode
		chgbank(1);
		hdmirxset(REG_RX_1B8,0x80,0x00);	// [7] Reg_HWENHYS = 0
		hdmirxset(REG_RX_1B6,0x07,0x03);	// [2:0]Reg_P0_ENHYS = 03  [2:0]Reg_P0_ENHYS = 03 for default enable filter to gating output
		chgbank(0);
		uc = mhlrxrd(0x05);
		mhlrxwr(0x05,uc);
		//FIX_ID_019	xxxxx

		#if 1
		//FIX_ID_024 xxxxx	//Fixed for RCP compliance issue
			it6602data->m_bRCPTimeOut = FALSE;
			it6602data->m_bRCPError = FALSE;
		//FIX_ID_024 xxxxx
		#endif

		//FIX_ID_034 xxxxx //Add MHL HPD Control by it6602HPDCtrl( )
		it6602data->m_DiscoveryDone = 0;
		//FIX_ID_034 xxxxx

		//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
		mhlrxset(MHL_RX_2B, 0x04,0x00);	// MHL2B[2] 0 for disable HW wake up fail machanism
		m_MHLabortID = 0; 	// test MSC Abort command only !!!
		chgbank(1);
		hdmirxset(REG_RX_1C0, 0x8C, 0x04);	//FIX_ID_037  2014-0527 +10% for W1070 only
		DLOG_INFO("Reset 1k pull down to +10 percent for W1070 only \r\n");
		chgbank(0);
		//xxxxx 2014-0529 //Manual Content On/Off
		it6602data->m_RAP_ContentOff = 0;
		it6602data->m_HDCP_ContentOff = 0;
		//xxxxx 2014-0529
		//FIX_ID_037 xxxxx
#endif
//FIX_ID_036	xxxxx

	}


	//	ucCurrentPort = hdmirxrd(REG_RX_051)&B_PORT_SEL;
	//
	//	if(ucCurrentPort == ucPortSel)
	//	{
	//		//FIX_ID_005 xxxxx 	//Add Cbus Event Handler
	//			it6602data->CBusIntEvent=0;
	//			it6602data->CBusSeqNo=0;
	//			it6602data->CBusWaitNo=0x00;
	//		//FIX_ID_005 xxxxx
	//
	//		//FIX_ID_005 xxxxx 	//Add Cbus Event Handler
	//			it6602data->HDMIIntEvent=0;
	//			it6602data->HDMIWaitNo[0]=0;
	//			it6602data->HDMIWaitNo[1]=0;
	//		//FIX_ID_005 xxxxx
	//
	//	}
}

static void AmpValidCheck (unsigned char ucPortSel)
{

	struct it6602_dev_data *it6602data = get_it6602_dev_data();

#ifdef _SUPPORT_AUTO_EQ_
	unsigned char uc;

	if(ucPortSel == F_PORT_SEL_1)
	{
		chgbank(1);
		uc = hdmirxrd(REG_RX_1D8);
		DLOG_INFO(" ############# AmpValidCheck( ) port 1 ###############\n");
		DLOG_INFO(" ############# Reg1D8 = %X ###############\n",(int)uc);
		DLOG_INFO(" ############# Reg1DC = %X ###############\n",(int)hdmirxrd(REG_RX_1DC));


		if((uc&0x03)==0x03)
		{
			ucChannelB[1] = hdmirxrd(REG_RX_1DD);
			ucPortAMPValid[1]|=0x03;
			DLOG_INFO(" ############# B AMP VALID port 1 Reg1DD = 0x%X  ###############\n",(int)ucChannelB[1]);
		}

		if((uc&0x0C)==0x0C)
		{
			ucChannelG[1]= hdmirxrd(REG_RX_1DE);
			ucPortAMPValid[1]|=0x0C;
			DLOG_INFO(" ############# G AMP VALID port 1 Reg1DD = 0x%X  ###############\n",(int)ucChannelG[1]);
		}

		if((uc&0x30)==0x30)
		{
			ucChannelR[1]= hdmirxrd(REG_RX_1DF);
			ucPortAMPValid[1]|=0x30;
			DLOG_INFO(" ############# R AMP VALID port 1 Reg1DD = 0x%X  ###############\n",(int)ucChannelR[1]);
		}
		chgbank(0);


		//		if((ucPortAMPValid[1]&0x3F)==0x3F)
		//		{
		//			ucPortAMPOverWrite[1]=1;	//2013-0801
		//			DLOG_INFO("#### REG_RX_03E = 0x%X ####\n",hdmirxrd(REG_RX_03E));
		//			hdmirxset(REG_RX_03E,0x20,0x20);	//Manually set RS Value
		//			DLOG_INFO("#### REG_RX_03E = 0x%X ####\n",hdmirxrd(REG_RX_03E));
		//
		//			hdmirxset(REG_RX_03F,0xFF,(ucChannelB[1] & 0x7F));
		//			hdmirxset(REG_RX_040,0xFF,(ucChannelG[1] & 0x7F));
		//			hdmirxset(REG_RX_041,0xFF,(ucChannelR[1] & 0x7F));
		//			DLOG_INFO(" ############# Over-Write port 1 EQ###############\n");
		//			DLOG_INFO(" ############# B port 1 Reg03F = 0x%X  ###############\n",hdmirxrd(REG_RX_03F));
		//			DLOG_INFO(" ############# G port 1 Reg040 = 0x%X  ###############\n",hdmirxrd(REG_RX_040));
		//			DLOG_INFO(" ############# R port 1 Reg041 = 0x%X  ###############\n",hdmirxrd(REG_RX_041));
		//
		//			HDMICheckErrorCount(&(it6602data->EQPort[F_PORT_SEL_1]));	//07-04 for port 1
		//
		//			hdmirxset(REG_RX_03A, 0xFF, 0x00);	//07-08 [3] power down	?????
		//
		//
		//			//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
		//			#ifdef _SUPPORT_EQ_ADJUST_
		//			HDMIStartEQDetect(&(it6602data->EQPort[F_PORT_SEL_1]));
		//			#endif
		//			//FIX_ID_001 xxxxx
		//
		//
		//		}
		//
		//		DLOG_INFO("REG_RX_038 = 0x%X  \n",hdmirxrd(REG_RX_038));
		//		DLOG_INFO("REG_RX_03A = 0x%X  \n",hdmirxrd(REG_RX_03A)));	// ??
		//		DLOG_INFO("REG_RX_03C = 0x%X  \n",hdmirxrd(REG_RX_03C));
		//		DLOG_INFO("REG_RX_03E = 0x%X  \n",hdmirxrd(REG_RX_03E));
		//		DLOG_INFO("REG_RX_03F = 0x%X  \n",hdmirxrd(REG_RX_03F));

//FIX_ID_010 xxxxx 	//Add JudgeBestEQ to avoid wrong EQ setting
		if((ucPortAMPValid[1]&0x3F)==0x3F)
		{
			OverWriteAmpValue2EQ(F_PORT_SEL_1);

			//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
			#ifdef _SUPPORT_EQ_ADJUST_
			HDMIStartEQDetect(&(it6602data->EQPort[F_PORT_SEL_1]));
			#endif
			//FIX_ID_001 xxxxx
		}
//FIX_ID_010 xxxxx

	}
	else
	{
		chgbank(1);
		uc = hdmirxrd(REG_RX_1D0);
		DLOG_INFO(" ############# AmpValidCheck( ) port 0 ###############\n");
		DLOG_INFO(" ############# REG_RX_1D0 = %X ###############\n",(int) uc);
		DLOG_INFO(" ############# Reg1D4 = %X ###############\n",(int) hdmirxrd(REG_RX_1D4));

		if((uc&0x03)==0x03)
		{
			ucChannelB[0] = hdmirxrd(REG_RX_1D5);
			ucPortAMPValid[0]|=0x03;
			DLOG_INFO(" ############# B AMP VALID port 0 Reg1D5 = 0x%X  ###############\n",(int) ucChannelB[0]);
		}

		if((uc&0x0C)==0x0C)
		{
			ucChannelG[0]= hdmirxrd(REG_RX_1D6);
			ucPortAMPValid[0]|=0x0C;
			DLOG_INFO(" ############# G AMP VALID port 0 Reg1D6 = 0x%X  ###############\n",(int) ucChannelG[0]);
		}

		if((uc&0x30)==0x30)
		{
			ucChannelR[0]= hdmirxrd(REG_RX_1D7);
			ucPortAMPValid[0]|=0x30;
			DLOG_INFO(" ############# R AMP VALID port 0 Reg1D7 = 0x%X  ###############\n",(int) ucChannelR[0]);
		}
		chgbank(0);

	//07-08
	if(hdmirxrd(REG_RX_P0_SYS_STATUS) & (B_P0_MHL_MODE))
	{
		if((ucPortAMPValid[0]&0x03)==0x03)
		{
			//			ucPortAMPOverWrite[0]=1;	//2013-0801
			//			DLOG_INFO("#### REG_RX_026 = 0x%X ####\n",hdmirxrd(REG_RX_026));
			//			hdmirxset(REG_RX_026,0x20,0x20);	//Manually set RS Value
			//			DLOG_INFO("#### REG_RX_026 = 0x%X ####\n",hdmirxrd(REG_RX_026));
			//
			//			hdmirxset(REG_RX_027,0xFF,(ucChannelB[0] & 0x7F));
			//			hdmirxset(REG_RX_028,0xFF,(ucChannelB[0] & 0x7F));	//07-08 using B channal to over-write G and R channel
			//			hdmirxset(REG_RX_029,0xFF,(ucChannelB[0] & 0x7F));
			//			DLOG_INFO(" ############# Over-Write port 0 EQ###############\n");
			//			DLOG_INFO(" ############# B port 0 REG_RX_027 = 0x%X  ###############\n",hdmirxrd(REG_RX_027));
			//			DLOG_INFO(" ############# G port 0 REG_RX_028 = 0x%X  ###############\n",hdmirxrd(REG_RX_028));
			//			DLOG_INFO(" ############# R port 0 REG_RX_029 = 0x%X  ###############\n",hdmirxrd(REG_RX_029));
			//
			//			HDMICheckErrorCount(&(it6602data->EQPort[F_PORT_SEL_0]));	//07-04 for port 1
			//
			//			hdmirxset(REG_RX_022, 0xFF, 0x00);	//07-08 [3] power down

//FIX_ID_010 xxxxx 	//Add JudgeBestEQ to avoid wrong EQ setting
			OverWriteAmpValue2EQ(F_PORT_SEL_0);
//FIX_ID_010 xxxxx

			//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
			#ifdef _SUPPORT_EQ_ADJUST_
			HDMIStartEQDetect(&(it6602data->EQPort[F_PORT_SEL_0]));
			#endif
			//FIX_ID_001 xxxxx


		}

	}
	else
	{
		if((ucPortAMPValid[0]&0x3F)==0x3F)
		{
			//			ucPortAMPOverWrite[0]=1;	//2013-0801
			//			DLOG_INFO("#### REG_RX_026 = 0x%X ####\n",hdmirxrd(REG_RX_026));
			//			hdmirxset(REG_RX_026,0x20,0x20);	//Manually set RS Value
			//			DLOG_INFO("#### REG_RX_026 = 0x%X ####\n",hdmirxrd(REG_RX_026));
			//
			//			hdmirxset(REG_RX_027,0xFF,(ucChannelB[0] & 0x7F));
			//			hdmirxset(REG_RX_028,0xFF,(ucChannelG[0] & 0x7F));
			//			hdmirxset(REG_RX_029,0xFF,(ucChannelR[0] & 0x7F));
			//			DLOG_INFO(" ############# Over-Write port 0 EQ###############\n");
			//			DLOG_INFO(" ############# B port 0 REG_RX_027 = 0x%X  ###############\n",hdmirxrd(REG_RX_027));
			//			DLOG_INFO(" ############# G port 0 REG_RX_028 = 0x%X  ###############\n",hdmirxrd(REG_RX_028));
			//			DLOG_INFO(" ############# R port 0 REG_RX_029 = 0x%X  ###############\n",hdmirxrd(REG_RX_029));
			//
			//			HDMICheckErrorCount(&(it6602data->EQPort[F_PORT_SEL_0]));	//07-04 for port 1
			//
			//			hdmirxset(REG_RX_022, 0xFF, 0x00);	//07-08 [3] power down

//FIX_ID_010 xxxxx 	//Add JudgeBestEQ to avoid wrong EQ setting
			OverWriteAmpValue2EQ(F_PORT_SEL_0);
//FIX_ID_010 xxxxx

			//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
			#ifdef _SUPPORT_EQ_ADJUST_
			HDMIStartEQDetect(&(it6602data->EQPort[F_PORT_SEL_0]));
			#endif
			//FIX_ID_001 xxxxx

		}
	}

	}
#endif
}
static void TogglePolarity (unsigned char ucPortSel)
{
#ifdef _SUPPORT_AUTO_EQ_


	//xxxxx only for IT6602A0 Version
	unsigned char ucPortSelCurrent;
	ucPortSelCurrent = hdmirxrd(REG_RX_051)&B_PORT_SEL;

#ifdef _ONLY_SUPPORT_MANUAL_EQ_ADJUST_
	return;
#endif

	//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
	if(HdmiI2cAddr==IT6602A0_HDMI_ADDR)
	//FIX_ID_002 xxxxx
	{

		if(ucPortSelCurrent !=ucPortSel)
			return;
	}
	//xxxxx


	if(ucPortSel == F_PORT_SEL_1)
	{
		DLOG_INFO(" ############# TogglePolarity Port 1###############\n");
		chgbank(1);

		hdmirxset(REG_RX_1C5, 0x10, 0x00);

		//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
		if(HdmiI2cAddr==IT6602A0_HDMI_ADDR)
		{
			//xxxxx only for IT6602A0 Version
			if( (hdmirxrd(REG_RX_1B9)&0x80)>>7 )
			{
			hdmirxset(REG_RX_1B9, 0x80, 0x00);	// Change Polarity
			}
			else
			{
			hdmirxset(REG_RX_1B9, 0x80, 0x80);	// Change Polarity
			}
			//xxxxx
		}
		else
		{
			if( (hdmirxrd(REG_RX_1C9)&0x80)>>7 )
			{
			hdmirxset(REG_RX_1C9, 0x80, 0x00);	// Change Polarity
			}
			else
			{
			hdmirxset(REG_RX_1C9, 0x80, 0x80);	// Change Polarity
			}
		}
		//FIX_ID_002 xxxxx

		hdmirxset(REG_RX_1C5, 0x10, 0x10);

		chgbank(0);


		DLOG_INFO(" ############# TogglePolarity Trigger Port 1 EQ ###############\n");
		//hdmirxset(0x3E, 0x80, 0x00);
		//#ifdef 	ENABLE_AUTO_TRIGGER
		//hdmirxset(0x3E, 0x80, 0x80);
		//#endif
		hdmirxset(REG_RX_03A, 0xFF, 0x38);
		hdmirxset(REG_RX_03A, 0x04, 0x04);
		hdmirxset(REG_RX_03A, 0x04, 0x00);

	}
	else
	{

		DLOG_INFO(" ############# TogglePolarity Port 0###############\n");
		chgbank(1);
		hdmirxset(REG_RX_1B5, 0x10, 0x00);

		//xxxxx only for IT6602A0 Version
		if( (hdmirxrd(REG_RX_1B9)&0x80)>>7 )
		{
		hdmirxset(REG_RX_1B9, 0x80, 0x00);	// Change Polarity
		}
		else
		{
		hdmirxset(REG_RX_1B9, 0x80, 0x80);	// Change Polarity
		}
		//xxxxx

		hdmirxset(REG_RX_1B5, 0x10, 0x10);
		chgbank(0);


		DLOG_INFO(" ############# TogglePolarity Trigger Port 0 EQ ###############\n");
		//hdmirxset(0x26, 0x80, 0x00);
		//#ifdef 	ENABLE_AUTO_TRIGGER
		//hdmirxset(0x26, 0x80, 0x80);
		//#endif
//		hdmirxset(REG_RX_022, 0x04, 0x00);
		hdmirxset(REG_RX_022, 0xFF, 0x38);	//07-04
		hdmirxset(REG_RX_022, 0x04, 0x04);
		hdmirxset(REG_RX_022, 0x04, 0x00);
	}
#endif
}

static void TMDSCheck(unsigned char ucPortSel)
{
#ifdef _SUPPORT_AUTO_EQ_
	unsigned int ucTMDSClk ;
	unsigned char rddata ;
	unsigned char ucClk ;

//FIX_ID_009 xxxxx	//verify interrupt event with reg51[0] select port
	struct it6602_dev_data *it6602data = get_it6602_dev_data();
//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
#if 0  //xxxxx 2014-0421 disable
	if(IT6602_IsSelectedPort(ucPortSel) == TRUE)
	{

		//FIX_ID_017 xxxxx for Allion ATC 8-5 Min Swing chanage to Max Swing ,
		//2013-1128 for Allion ATC 8-5 Min Swing chanage to Max Swing ,
		//there are no SCDT off or SCDT on interrupt
		//only CKOn interrupt happen !!!
		IT6602SwitchVideoState(it6602data,VSTATE_SyncWait);
		//FIX_ID_017 xxxxx

		//2013-1217 disable --> IT6602VideoOutputEnable(FALSE);

		//xxxxx 2013-0812 @@@@@
		it6602data->m_ucSCDTOffCount=0;
		//xxxxx 2013-0812



	}
#endif //xxxxx 2014-0421 disable
//FIX_ID_009 xxxxx
//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode

	DLOG_INFO("TMDSCheck() !!!\n");


	if(ucPortSel == F_PORT_SEL_1)
	{
		ucClk = hdmirxrd(REG_RX_092) ;
		rddata = hdmirxrd(0x90);

		if(ucClk != 0)
		{

		if( rddata&0x04 )
			ucTMDSClk=2*RCLKVALUE*256/ucClk;
		else if( rddata&0x08 )
			ucTMDSClk=4*RCLKVALUE*256/ucClk;
		else
			ucTMDSClk=RCLKVALUE*256/ucClk;


		DLOG_INFO(" Port 1 TMDS CLK  = %d \r\n",(int) ucTMDSClk);
		}


//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
	if(HdmiI2cAddr==IT6602A0_HDMI_ADDR)
	{
		if(ucTMDSClk<TMDSCLKVALUE_480P || ucTMDSClk > TMDSCLKVALUE_1080P )
			hdmirxwr(REG_RX_038,0x00);	// Dr. Liu suggestion to 0x00
		else
			hdmirxwr(REG_RX_038,0x3F);	// Dr. Liu suggestion to 0x3F
	}
//FIX_ID_002 xxxxx


		DLOG_INFO(" HDMI Reg038  = %X \r\n",(int) hdmirxrd(REG_RX_038));

		chgbank(1);
		DLOG_INFO(" HDMI Reg1C1  = %X ,Reg1C2  = %X\r\n",hdmirxrd(REG_RX_1C1),hdmirxrd(REG_RX_1C2));
		chgbank(0);

		//hdmirxset(0x3E, 0x80, 0x00);
		//#ifdef 	ENABLE_AUTO_TRIGGER
		//hdmirxset(0x3E, 0x80, 0x80);
		//#endif

		if(ucPortAMPOverWrite[1]==0)	// 2013-0801
		{
			//FIX_ID_001 xxxxx check State of AutoEQ
			chgbank(1);
			rddata=hdmirxrd(REG_RX_1DC);
			chgbank(0);
			if(rddata==0)
			//FIX_ID_001 xxxxx
			{
				DLOG_INFO(" ############# Trigger Port 1 EQ ###############\n");
				hdmirxset(REG_RX_03A, 0xFF, 0x38);	//07-04
				hdmirxset(REG_RX_03A, 0x04, 0x04);
				hdmirxset(REG_RX_03A, 0x04, 0x00);
			}
		}
		else
		{
				DLOG_INFO(" ############# B_PORT1_TimingChgEvent###############\n");
				it6602data->HDMIIntEvent |= (B_PORT1_Waiting);
				it6602data->HDMIIntEvent |= (B_PORT1_TimingChgEvent);
				it6602data->HDMIWaitNo[1]=MAX_TMDS_WAITNO;
		}


	}
	else
	{
		DLOG_INFO(" HDMI Reg90  = %X ,Reg91  = %X\r\n",(int) hdmirxrd(0x90),(int) hdmirxrd(0x91));
		ucClk = hdmirxrd(REG_RX_091) ;
		rddata = hdmirxrd(0x90);

		if(ucClk != 0)
		{
		if( rddata&0x01 )
			ucTMDSClk=2*RCLKVALUE*256/ucClk;
		else if( rddata&0x02 )
			ucTMDSClk=4*RCLKVALUE*256/ucClk;
		else
			ucTMDSClk=RCLKVALUE*256/ucClk;

		DLOG_INFO(" Port 0 TMDS CLK  = %X \r\n",(int) ucTMDSClk);
		}

//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
	if(HdmiI2cAddr==IT6602A0_HDMI_ADDR)
	{

		//FIX_ID_007 xxxxx 	//for debug IT6681  HDCP issue
			if(hdmirxrd(REG_RX_P0_SYS_STATUS) & (B_P0_MHL_MODE))
			{
				chgbank(1);
				hdmirxset(REG_RX_1B1,0x20,0x20);//Reg1b1[5] = 1 for enable over-write
				hdmirxset(REG_RX_1B2,0x07,0x01);	// default 0x04 , change to 0x01
				DLOG_INFO(" HDMI Reg1B1  = %X ,Reg1B2  = %X\r\n",(int) hdmirxrd(REG_RX_1B1),(int) hdmirxrd(REG_RX_1B2));
				chgbank(0);
			}
			else
			{
				chgbank(1);
				hdmirxset(REG_RX_1B1,0x20,0x00);//Reg1b1[5] = 0 for disable over-write
				hdmirxset(REG_RX_1B2,0x07,0x04);	// default 0x04
				DLOG_INFO(" HDMI Reg1B1  = %X ,Reg1B2  = %X\r\n",(int) hdmirxrd(REG_RX_1B1),(int) hdmirxrd(REG_RX_1B2));
				chgbank(0);
			}
		//FIX_ID_007 xxxxx
	}
//FIX_ID_002 xxxxx




//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
	if(HdmiI2cAddr==IT6602A0_HDMI_ADDR)
	{
		if(ucTMDSClk<TMDSCLKVALUE_480P || ucTMDSClk > TMDSCLKVALUE_1080P )
			hdmirxwr(REG_RX_020,0x00);	// Dr. Liu suggestion to 0x00
		else
			hdmirxwr(REG_RX_020,0x3F);	// Dr. Liu suggestion to 0x3F
	}
//FIX_ID_002 xxxxx

		DLOG_INFO(" HDMI Reg020  = %X \r\n",(int) hdmirxrd(REG_RX_020));

		//hdmirxset(0x26, 0x80, 0x00);
		//#ifdef 	ENABLE_AUTO_TRIGGER
		//hdmirxset(0x26, 0x80, 0x80);
		//#endif


		//FIX_ID_019	xxxxx modify ENHYS control for MHL mode
		if(hdmirxrd(REG_RX_P0_SYS_STATUS) & (B_P0_MHL_MODE))
		{
			chgbank(1);
			hdmirxset(REG_RX_1B8,0x80,0x00);	// [7] Reg_HWENHYS = 0
			hdmirxset(REG_RX_1B6,0x07,0x00);	// [2:0]Reg_P0_ENHYS = 00 for MHL mode only  [2:0]Reg_P0_ENHYS = 00 for disable ENHYS
			chgbank(0);
		}
		//FIX_ID_019	xxxxx


		if(ucPortAMPOverWrite[0]==0)	// 2013-0801
		{
			//FIX_ID_001 xxxxx check State of AutoEQ
			chgbank(1);
			rddata=hdmirxrd(REG_RX_1D4);
			chgbank(0);

//FIX_ID_032 xxxxx	//Support HDCP Repeater function for HDMI Tx device
#ifdef _ONLY_SUPPORT_MANUAL_EQ_ADJUST_
		#ifndef _SUPPORT_HDCP_REPEATER_
			//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
			#ifdef _SUPPORT_EQ_ADJUST_
			HDMIStartEQDetect(&(it6602data->EQPort[F_PORT_SEL_0]));
			#endif
			//FIX_ID_001 xxxxx
		#endif
#else
			if(rddata==0)
			//FIX_ID_001 xxxxx
			{
				DLOG_INFO(" ############# Trigger Port 0 EQ ###############\n");
				hdmirxset(REG_RX_022, 0xFF, 0x38);	//07-04
				hdmirxset(REG_RX_022, 0x04, 0x04);
				hdmirxset(REG_RX_022, 0x04, 0x00);
			}
#endif
//FIX_ID_032 xxxxx


//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
			// if Authentication start interrupt with CKon interrupt then do TMDSCheck() at first.
			it6602data->HDMIIntEvent &= ~(B_PORT0_TMDSEvent|B_PORT0_Waiting|B_PORT0_TimingChgEvent);
//FIX_ID_033 xxxxx
		}
		else
		{
				DLOG_INFO(" ############# B_PORT0_TimingChgEvent###############\n");
				it6602data->HDMIIntEvent |= (B_PORT0_Waiting);
				it6602data->HDMIIntEvent |= (B_PORT0_TimingChgEvent);
				it6602data->HDMIWaitNo[0]=MAX_TMDS_WAITNO;
		}


	}

#endif
}




static void OverWriteAmpValue2EQ (unsigned char ucPortSel)
{
	struct it6602_dev_data *it6602data = get_it6602_dev_data();

	DLOG_INFO(" 111111111111111111 OverWriteAmpValue2EQ 111111111111111111111111111111\r\n");
	DLOG_INFO(" 111111111111111111 OverWriteAmpValue2EQ 111111111111111111111111111111\r\n");
	DLOG_INFO(" 111111111111111111 OverWriteAmpValue2EQ 111111111111111111111111111111\r\n");
	DLOG_INFO(" 111111111111111111 OverWriteAmpValue2EQ 111111111111111111111111111111\r\n");
	DLOG_INFO(" 111111111111111111 OverWriteAmpValue2EQ 111111111111111111111111111111\r\n");
	DLOG_INFO(" 111111111111111111 OverWriteAmpValue2EQ 111111111111111111111111111111\r\n");


	if(ucPortSel == F_PORT_SEL_1)
	{
		if((ucPortAMPValid[1]&0x3F)==0x3F)
		{
			ucPortAMPOverWrite[F_PORT_SEL_1]=1;	//2013-0801
			ucEQMode[F_PORT_SEL_1] = 0; // 0 for Auto Mode
			DLOG_INFO("#### REG_RX_03E = 0x%X ####\r\n",(int) hdmirxrd(REG_RX_03E));
			hdmirxset(REG_RX_03E,0x20,0x20);	//Manually set RS Value
			DLOG_INFO("#### REG_RX_03E = 0x%X ####\r\n",(int) hdmirxrd(REG_RX_03E));

//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
			if(ucChannelB[F_PORT_SEL_1]  < MinEQValue )
			{
			hdmirxwr(REG_RX_03F,MinEQValue);
			}
			else
			{
			hdmirxwr(REG_RX_03F,(ucChannelB[F_PORT_SEL_1] & 0x7F));
			}

			if(ucChannelG[F_PORT_SEL_1]  < MinEQValue )
			{
			hdmirxwr(REG_RX_040,MinEQValue);
			}
			else
			{
			hdmirxwr(REG_RX_040,(ucChannelG[F_PORT_SEL_1] & 0x7F));
			}

			if(ucChannelR[F_PORT_SEL_1]  < MinEQValue )
			{
			hdmirxwr(REG_RX_041,MinEQValue);
			}
			else
			{
			hdmirxwr(REG_RX_041,(ucChannelR[F_PORT_SEL_1] & 0x7F));
			}

			//xxxxx 2014-0421 disable ->			hdmirxset(REG_RX_03F,0xFF,(ucChannelB[F_PORT_SEL_1] & 0x7F));
			//xxxxx 2014-0421 disable ->			hdmirxset(REG_RX_040,0xFF,(ucChannelG[F_PORT_SEL_1] & 0x7F));
			//xxxxx 2014-0421 disable ->			hdmirxset(REG_RX_041,0xFF,(ucChannelR[F_PORT_SEL_1] & 0x7F));

			//if Auto EQ done  interrupt then clear HDMI Event !!!
			it6602data->HDMIIntEvent &= ~(B_PORT1_TMDSEvent|B_PORT1_Waiting|B_PORT1_TimingChgEvent);


			DLOG_INFO(" ############# Over-Write port 1 EQ###############\r\n");
			DLOG_INFO(" ############# B port 1 Reg03F = 0x%X  ###############\r\n",(int) hdmirxrd(REG_RX_03F));
			DLOG_INFO(" ############# G port 1 Reg040 = 0x%X  ###############\r\n",(int) hdmirxrd(REG_RX_040));
			DLOG_INFO(" ############# R port 1 Reg041 = 0x%X  ###############\r\n",(int) hdmirxrd(REG_RX_041));

			//	HDMICheckErrorCount(&(it6602data->EQPort[F_PORT_SEL_1]));	//07-04 for port 1

			//hdmirxset(REG_RX_03A, 0xFF, 0x00);	//07-08 [3] power down	?????
			hdmirxwr(REG_RX_03A, 0x00);	// power down auto EQ
			hdmirxwr(0xD0, 0xC0);
//FIX_ID_033 xxxxx

		}
	}
	else
	{

	//07-08
	if(hdmirxrd(REG_RX_P0_SYS_STATUS) & (B_P0_MHL_MODE))
	{
		if((ucPortAMPValid[F_PORT_SEL_0]&0x03)==0x03)
		{
			ucPortAMPOverWrite[F_PORT_SEL_0]=1;	//2013-0801
			ucEQMode[F_PORT_SEL_0] = 0; // 0 for Auto Mode
			DLOG_INFO("#### REG_RX_026 = 0x%X ####\r\n",(int) hdmirxrd(REG_RX_026));

			hdmirxset(REG_RX_026,0x20,0x20);	//Manually set RS Value
			DLOG_INFO("#### REG_RX_026 = 0x%X ####\r\n",(int) hdmirxrd(REG_RX_026));

//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
			if((ucChannelB[F_PORT_SEL_0] ) < MinEQValue)
			{
			hdmirxwr(REG_RX_027,(MinEQValue));
			hdmirxwr(REG_RX_028,(MinEQValue));	//07-08 using B channal to over-write G and R channel
			hdmirxwr(REG_RX_029,(MinEQValue));
			}
			else
			{
			hdmirxwr(REG_RX_027,(ucChannelB[F_PORT_SEL_0] & 0x7F));
			hdmirxwr(REG_RX_028,(ucChannelB[F_PORT_SEL_0] & 0x7F));	//07-08 using B channal to over-write G and R channel
			hdmirxwr(REG_RX_029,(ucChannelB[F_PORT_SEL_0] & 0x7F));
			}

			DLOG_INFO(" ############# Over-Write port 0 MHL EQ###############\r\n");
			DLOG_INFO(" ############# B port 0 REG_RX_027 = 0x%X  ###############\r\n",(int) hdmirxrd(REG_RX_027));
			DLOG_INFO(" ############# G port 0 REG_RX_028 = 0x%X  ###############\r\n",(int) hdmirxrd(REG_RX_028));
			DLOG_INFO(" ############# R port 0 REG_RX_029 = 0x%X  ###############\r\n",(int) hdmirxrd(REG_RX_029));

			//	HDMICheckErrorCount(&(it6602data->EQPort[F_PORT_SEL_0]));	//07-04 for port 1

			hdmirxwr(REG_RX_022, 0x00);	// power down auto EQ
		hdmirxwr(0xD0, 0x30);
//FIX_ID_033 xxxxx
		}

	}
	else
	{
		if((ucPortAMPValid[F_PORT_SEL_0]&0x3F)==0x3F)
		{
			ucPortAMPOverWrite[F_PORT_SEL_0]=1;	//2013-0801
			ucEQMode[F_PORT_SEL_0] = 0; // 0 for Auto Mode
			DLOG_INFO("#### REG_RX_026 = 0x%X ####\r\n",(int) hdmirxrd(REG_RX_026));
			hdmirxset(REG_RX_026,0x20,0x20);	//Manually set RS Value
			DLOG_INFO("#### REG_RX_026 = 0x%X ####\r\n",(int) hdmirxrd(REG_RX_026));

//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
			if(ucChannelB[F_PORT_SEL_0]  < MinEQValue )
			{
			hdmirxwr(REG_RX_027,MinEQValue);
			}
			else
			{
			hdmirxwr(REG_RX_027,(ucChannelB[F_PORT_SEL_0] & 0x7F));
			}

			if(ucChannelG[F_PORT_SEL_0]  < MinEQValue )
			{
			hdmirxwr(REG_RX_028,MinEQValue);
			}
			else
			{
			hdmirxwr(REG_RX_028,(ucChannelG[F_PORT_SEL_0] & 0x7F));
			}

			if(ucChannelR[F_PORT_SEL_0]  < MinEQValue )
			{
			hdmirxwr(REG_RX_029,MinEQValue);
			}
			else
			{
			hdmirxwr(REG_RX_029,(ucChannelR[F_PORT_SEL_0] & 0x7F));
			}

//xxxxx 2014-0421 disable -> 			hdmirxset(REG_RX_027,0xFF,(ucChannelB[F_PORT_SEL_0] & 0x7F));
//xxxxx 2014-0421 disable -> 			hdmirxset(REG_RX_028,0xFF,(ucChannelG[F_PORT_SEL_0] & 0x7F));
//xxxxx 2014-0421 disable -> 			hdmirxset(REG_RX_029,0xFF,(ucChannelR[F_PORT_SEL_0] & 0x7F));

			//if Auto EQ done  interrupt then clear HDMI Event !!!
			it6602data->HDMIIntEvent &= ~(B_PORT0_TMDSEvent|B_PORT0_Waiting|B_PORT0_TimingChgEvent);

			DLOG_INFO(" ############# Over-Write port 0 EQ###############\r\n");
			DLOG_INFO(" ############# B port 0 REG_RX_027 = 0x%X  ###############\r\n",(int) hdmirxrd(REG_RX_027));
			DLOG_INFO(" ############# G port 0 REG_RX_028 = 0x%X  ###############\r\n",(int) hdmirxrd(REG_RX_028));
			DLOG_INFO(" ############# R port 0 REG_RX_029 = 0x%X  ###############\r\n",(int) hdmirxrd(REG_RX_029));

			//	HDMICheckErrorCount(&(it6602data->EQPort[F_PORT_SEL_0]));	//07-04 for port 1

			//hdmirxset(REG_RX_022, 0xFF, 0x00);	//07-08 [3] power down
			hdmirxwr(REG_RX_022, 0x00);	// power down auto EQ
		hdmirxwr(0xD0, 0x30);
//FIX_ID_033 xxxxx
		}
	}

	}
}
//-------------------------------------------------------------------------------------------------------
#endif



//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
#ifdef _SUPPORT_EQ_ADJUST_

static void HDMIStartEQDetect(struct it6602_eq_data *ucEQPort)
/*
 * This is the HDMIRX Start EQ Detect
 * @param it6602_eq_data
 * @return void
 */
{
	unsigned char ucPortSel;


//FIX_ID_035 xxxxx //For MTK6592 HDMI to SII MHL TX compliance issue
if( ucEQPort->ucPortID == F_PORT_SEL_0)
{
	if(hdmirxrd(REG_RX_P0_SYS_STATUS) & (B_P0_MHL_MODE))
	{
		// for MHL mode , there are no need to adjust EQ for long cable.
		return;
	}
}
//FIX_ID_035 xxxxx

	if(ucEQPort->ucEQState==0xFF)
	{
		ucPortSel = hdmirxrd(REG_RX_051) & B_PORT_SEL;

		if(ucPortSel == ucEQPort->ucPortID)
		{
		HDMISwitchEQstate(ucEQPort,0);	// for SCDT off state
		}
		else
		{
		HDMISwitchEQstate(ucEQPort,EQSTATE_WAIT+1); 	//for SCDT on state
		}

		ucEQPort->f_manualEQadjust=TRUE;
		HDMIAdjustEQ(ucEQPort);

//FIX_ID_010 xxxxx 	//Add JudgeBestEQ to avoid wrong EQ setting
		ucEQPort->ErrorCount[0] = MAXECCWAIT;
		ucEQPort->ErrorCount[1] = MAXECCWAIT;
		ucEQPort->ErrorCount[2] = MAXECCWAIT;
//FIX_ID_010 xxxxx
	}
}

static void HDMISetEQValue(struct it6602_eq_data *ucEQPort,unsigned char ucIndex)
/*
 * This is the HDMIRX Set Manual EQ value
 * @param it6602_eq_data
 * @return void
 */
{
	if(ucIndex<MaxEQIndex)
	{
		if(ucEQPort->ucPortID==F_PORT_SEL_0)
		{
#ifdef _SUPPORT_AUTO_EQ_
			ucEQMode[F_PORT_SEL_0] = 1; // 1 for Manual Mode
#endif
			hdmirxset(REG_RX_026, 0x20, 0x20);	//07-04 add for adjust EQ
			hdmirxwr(REG_RX_027,IT6602EQTable[ucIndex]);
			DLOG_INFO("Port=%X ,ucIndex = %X ,HDMISetEQValue Reg027 = %X \r\n",(int) ucEQPort->ucPortID,(int) ucIndex,(int) hdmirxrd(REG_RX_027));
		}
		else
		{
#ifdef _SUPPORT_AUTO_EQ_
			ucEQMode[F_PORT_SEL_1] = 1; // 1 for Manual Mode
#endif
			hdmirxset(REG_RX_03E, 0x20, 0x20);	//07-04 add for adjust EQ
			hdmirxwr(REG_RX_03F,IT6602EQTable[ucIndex]);
			DLOG_INFO("Port=%X ,ucIndex = %X ,HDMISetEQValue Reg03F = %X \r\n",(int) ucEQPort->ucPortID,(int) ucIndex,(int) hdmirxrd(REG_RX_03F));
		}

	}

}


static void HDMISwitchEQstate(struct it6602_eq_data *ucEQPort,unsigned char state)
/*
 * This is the HDMIRX Switch EQ State
 * @param it6602_eq_data
 * @return void
 */
{

	ucEQPort->ucEQState=state;

	DLOG_INFO("!!! Port=%X ,HDMISwitchEQstate %X \r\n",(int) ucEQPort->ucPortID,(int) ucEQPort->ucEQState);

	switch(ucEQPort->ucEQState)
	{
		case EQSTATE_START:
			HDMISetEQValue(ucEQPort,0);
			break;
		case EQSTATE_LOW:
			HDMISetEQValue(ucEQPort,1);
			break;
		case EQSTATE_MIDDLE:
			HDMISetEQValue(ucEQPort,2);
			break;
		case EQSTATE_HIGH:
			HDMISetEQValue(ucEQPort,3);
			break;

		default:
			//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
			//xxxxx 2014-0529 //HDCP Content On/Off
			IT6602_HDCP_ContentOff(ucEQPort->ucPortID, 0);
			//xxxxx 2014-0529
			//FIX_ID_037 xxxxx

			HDMISetEQValue(ucEQPort,0xff);	//dont care
			break;

	}

	// !!! re-start the error count !!!

	ucEQPort->ucPkt_Err=0;
	ucEQPort->ucECCvalue=0;
	ucEQPort->ucECCfailCount=0;

}

static void HDMICheckSCDTon(struct it6602_eq_data *ucEQPort)
/*
 * This is the HDMIRX SCDT on
 * @param it6602_eq_data
 * @return void
 */
{
	unsigned char ucResult = 0;
	unsigned char Receive_Err;
	unsigned char ucStatus;
	unsigned char ucCurrentPort;
	unsigned char ucHDCP;

	ucCurrentPort = hdmirxrd(REG_RX_051) & B_PORT_SEL;

	if(ucEQPort->ucPortID != ucCurrentPort)
		return;


	if(ucEQPort->ucPortID==F_PORT_SEL_1)
	{
		ucStatus = hdmirxrd(REG_RX_P1_SYS_STATUS);
		// !!! check ECC error register  !!!
		Receive_Err = hdmirxrd(REG_RX_0B7);
		hdmirxwr(REG_RX_0B7,Receive_Err);

		ucHDCP = hdmirxrd(REG_RX_095);
	}
	else
	{
		ucStatus = hdmirxrd(REG_RX_P0_SYS_STATUS);
		// !!! check ECC error register  !!!
		Receive_Err = hdmirxrd(REG_RX_0B2);
		hdmirxwr(REG_RX_0B2,Receive_Err);

		ucHDCP = hdmirxrd(REG_RX_093);
	}


	if((ucStatus & (B_P0_SCDT|B_P0_PWR5V_DET|B_P0_RXCK_VALID)) == (B_P0_PWR5V_DET|B_P0_RXCK_VALID))
	{
			ucEQPort->ucECCfailCount++;
	}

	DLOG_INFO("Port=%d, CheckSCDTon=%d, Receive_Err=%X, ucECCfailCount=%X, SCDT=%X, HDCP=%X \r\n",
				(int) ucEQPort->ucPortID,(int) ucEQPort->ucEQState,(int) Receive_Err,(int)ucEQPort->ucECCfailCount,(int) ucStatus,(int) ucHDCP);

//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
	if((Receive_Err & 0xC0) != 0x00)
	{
			ucEQPort->ucECCvalue++;

			//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
			//xxxxx 2014-0529 //Manual Content On/Off
			IT6602_HDCP_ContentOff(ucEQPort->ucPortID, 1);
			//xxxxx 2014-0529
			//FIX_ID_037 xxxxx

			//xxxxx 2014-0421
			//if((Receive_Err & 0xC0) == 0xC0)
			if(ucEQPort->ucECCvalue > ((MINECCFAILCOUNT/2)))
			//xxxxx 2014-0421
			{
				ucEQPort->ucECCvalue = 0;
				DLOG_INFO("HDMICheckSCDTon() for ECC / Deskew issue !!!");

				if(ucEQPort->ucPortID==F_PORT_SEL_1)
				{
					if(hdmirxrd(REG_RX_038) == 0x00)
						hdmirxwr(REG_RX_038,0x3F);	// Dr. Liu suggestion to 0x00
					//else
					//	hdmirxwr(REG_RX_038,0x00);	// Dr. Liu suggestion to 0x3F

					DLOG_INFO("Port 1 Reg38=%X !!!\n",(int) hdmirxrd(REG_RX_038));
				}
				else
				{
					if(hdmirxrd(REG_RX_020) == 0x00)
					{
						hdmirxwr(REG_RX_020,0x3F);	// Dr. Liu suggestion to 0x00
					//else
					//	hdmirxwr(REG_RX_020,0x00);	// Dr. Liu suggestion to 0x3F

					DLOG_INFO("Port 0 Reg20=%X !!!\n",(int) hdmirxrd(REG_RX_020));
					}
				}
			}
	}
//FIX_ID_033 xxxxx

	if(ucEQPort->ucEQState == EQSTATE_WAIT-1)
	{

		//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
			DLOG_INFO("Port=%d, CheckSCDTon=%d, Receive_Err=%X, ucECCfailCount=%X, SCDT=%X, HDCP=%X \r\n",
					(int) ucEQPort->ucPortID,(int) ucEQPort->ucEQState,(int) Receive_Err,(int)ucEQPort->ucECCfailCount,(int) ucStatus,(int) ucHDCP);

			if((Receive_Err & 0xC0) == 0xC0)
			{
				DLOG_INFO("HDMICheckSCDTon() CDR reset for Port %d ECC_TIMEOUT !!!\n",ucCurrentPort);
				hdmirx_ECCTimingOut(ucCurrentPort);

				HDMISwitchEQstate(ucEQPort,EQSTATE_END);
				return;
			}
		//FIX_ID_033 xxxxx

#ifdef _SUPPORT_AUTO_EQ_
		if((ucEQPort->ucECCfailCount)==0)
		{


			if(ucEQPort->ucPortID==F_PORT_SEL_1)
			{
				if(ucEQMode[F_PORT_SEL_1] == 0)	// verfiy Auto EQ Value wehn auto EQ finish
				{

					if(	((ucChannelB[F_PORT_SEL_1] & 0x7F)<0x0F) ||
						((ucChannelG[F_PORT_SEL_1] & 0x7F)<0x0F) ||
						((ucChannelR[F_PORT_SEL_1] & 0x7F)<0x0F) )

					{
						ucResult	= 1;	// 1 for EQ start
					}

				}
			}
			else
			{
				if(ucEQMode[F_PORT_SEL_0] == 0)	// verfiy Auto EQ Value when auto EQ finish
				{
					if(hdmirxrd(REG_RX_P0_SYS_STATUS) & (B_P0_MHL_MODE))
					{
						if((ucChannelB[F_PORT_SEL_0] & 0x7F)<0x0F)
						{
							ucResult	= 1;	// 1 for EQ start
						}
					}
					else
					{
						if(	((ucChannelB[F_PORT_SEL_0] & 0x7F)<0x0F) ||
							((ucChannelG[F_PORT_SEL_0] & 0x7F)<0x0F) ||
							((ucChannelR[F_PORT_SEL_0] & 0x7F)<0x0F) )

						{
							ucResult	= 1;	// 1 for EQ start
						}

					}
				}
			}

			if( ucResult == 0)	// no need to do manual EQ adjust when SCDT always On !!!
			{
				HDMISwitchEQstate(ucEQPort,EQSTATE_END);
				return;
			}

		}
#endif

		HDMISwitchEQstate(ucEQPort,EQSTATE_WAIT);
	}
}

static void HDMIPollingErrorCount(struct it6602_eq_data *ucEQPort)
/*
 * This is the HDMIPollingErrorCount
 * @param
 * @return void
 */
{
	unsigned char Receive_Err;
	unsigned char Video_Err;
	unsigned char Code_Err;
	unsigned char Pkt_Err;
	unsigned char CrtErr;
	unsigned char ucHDCP;
	unsigned char ucStatus;

	unsigned char ucCurrentPort;
	ucCurrentPort = hdmirxrd(REG_RX_051)&B_PORT_SEL;



	if(ucEQPort->ucPortID==F_PORT_SEL_1)
	{
		ucStatus = hdmirxrd(REG_RX_P1_SYS_STATUS);
		// !!! check ECC error register  !!!
		Receive_Err = hdmirxrd(REG_RX_0B7);
		Video_Err = hdmirxrd(REG_RX_0B8)&0xE0;
		Code_Err = hdmirxrd(REG_RX_0B9);
		Pkt_Err = hdmirxrd(REG_RX_0BA);
		CrtErr = hdmirxrd(REG_RX_0BB);

		hdmirxwr(REG_RX_0B7,Receive_Err);
		hdmirxwr(REG_RX_0B8,Video_Err);
		hdmirxwr(REG_RX_0B9,Code_Err);
		hdmirxwr(REG_RX_0BA,Pkt_Err);
		hdmirxwr(REG_RX_0BB,CrtErr);

		ucHDCP = hdmirxrd(REG_RX_095);
	}
	else
	{
		ucStatus = hdmirxrd(REG_RX_P0_SYS_STATUS);
		// !!! check ECC error register  !!!
		Receive_Err = hdmirxrd(REG_RX_0B2);
		Video_Err = hdmirxrd(REG_RX_0B3)&0xE0;
		Code_Err = hdmirxrd(REG_RX_0B4);
		Pkt_Err = hdmirxrd(REG_RX_0B5);
		CrtErr = hdmirxrd(REG_RX_0B6);

		hdmirxwr(REG_RX_0B2,Receive_Err);
		hdmirxwr(REG_RX_0B3,Video_Err);
		hdmirxwr(REG_RX_0B4,Code_Err);
		hdmirxwr(REG_RX_0B5,Pkt_Err);
		hdmirxwr(REG_RX_0B6,CrtErr);


		ucHDCP = hdmirxrd(REG_RX_093);
	}

	//xxxxx 2013-0903
	if(ucCurrentPort == ucEQPort->ucPortID)
	{
		if((ucStatus & B_P0_SCDT) == 0x00)
		{
			Receive_Err = 0xFF;

		//xxxxx 2013-0812  ++++
		ucEQPort->ucECCfailCount |= 0x80;
		//xxxxx 2013-0812

		}
	}
	//xxxxx 2013-0903

	DLOG_INFO("Port=%d ,EQState2No=%d, Receive_Err=%X, HDCP=%X \r\n",
					(int) ucEQPort->ucPortID,(int) ucEQPort->ucEQState,(int) Receive_Err,(int) ucHDCP);

#if 1
//FIX_ID_007 xxxxx 	//07-18 xxxxx for ATC 8-7 Jitter Tolerance
	if(Pkt_Err==0xFF ||Code_Err==0xFF)
	{
		ucEQPort->ucPkt_Err++;	// judge whether CDR reset
	}
	else
	{
		ucEQPort->ucPkt_Err=0;
	}

	if(ucEQPort->ucPkt_Err > (MINECCFAILCOUNT-2))
	{

		if( ucEQPort->ucEQState > EQSTATE_START)
		{

//FIX_ID_020 xxxxx		//Turn off DEQ for HDMI port 1 with 20m DVI Cable
			DLOG_INFO("1111111111111111111111111111111111111111111111111111111111111111111111111\r\n");

			if(ucEQPort->ucPortID==F_PORT_SEL_1)
			{
				Code_Err = hdmirxrd(REG_RX_0B9);
				hdmirxwr(REG_RX_0B9,Code_Err);

				if(Code_Err == 0xFF)
				{
					if(hdmirxrd(REG_RX_038) == 0x00)
						hdmirxwr(REG_RX_038,0x3F);	// Dr. Liu suggestion to 0x00
					else
						hdmirxwr(REG_RX_038,0x00);	// Dr. Liu suggestion to 0x3F
					DLOG_INFO("Port 1 Reg38=%X !!!\n",(int) hdmirxrd(REG_RX_038));
				}
			}
			else
			{
				Code_Err = hdmirxrd(REG_RX_0B4);
				hdmirxwr(REG_RX_0B4,Code_Err);

				if(Code_Err == 0xFF)
				{
					if(hdmirxrd(REG_RX_020) == 0x00)
						hdmirxwr(REG_RX_020,0x3F);	// Dr. Liu suggestion to 0x00
					else
						hdmirxwr(REG_RX_020,0x00);	// Dr. Liu suggestion to 0x3F

					DLOG_INFO("Port 0 Reg20=%X !!!\n",(int) hdmirxrd(REG_RX_020));
				}
			}
			DLOG_INFO("1111111111111111111111111111111111111111111111111111111111111111111111111\r\n");
//FIX_ID_020 xxxxx

			if(ucEQPort->ucPortID==F_PORT_SEL_0)
			{

				hdmirxset(REG_RX_011,(B_P0_DCLKRST|B_P0_CDRRST),(B_P0_DCLKRST|B_P0_CDRRST/*|B_P0_SWRST*/));
				hdmirxset(REG_RX_011,(B_P0_DCLKRST|B_P0_CDRRST),0x00);
				DLOG_INFO(" HDMIPollingErrorCount( ) Port 0 CDR reset !!!!!!!!!!!!!!!!!! \r\n");
			}
			else
			{
				hdmirxset(REG_RX_018,(B_P1_DCLKRST|B_P1_CDRRST),(B_P1_DCLKRST|B_P1_CDRRST/*|B_P1_SWRST*/));
				hdmirxset(REG_RX_018,(B_P1_DCLKRST|B_P1_CDRRST),0x00);
				DLOG_INFO(" HDMIPollingErrorCount( ) Port 1 CDR reset !!!!!!!!!!!!!!!!!! \r\n");
			}
		}
		ucEQPort->ucPkt_Err=0;

//xxxxx 2013-0812  ++++
		ucEQPort->ucECCfailCount |= 0x40;
		ucEQPort->ucECCfailCount &= 0xF0;
//xxxxx 2013-0812

	}
	//07-18 xxxxx
//FIX_ID_007 xxxxx
#endif


//	if(Receive_Err>32 )

//xxxxx 2013-0812  ++++
	if(Receive_Err != 0 )
//xxxxx 2013-0812
	{
		DLOG_INFO("Video_Err = %X \r\n",(int) Video_Err);
		DLOG_INFO("Code_Err = %X \r\n",(int) Code_Err);
		DLOG_INFO("Pkt_Err = %X \r\n",(int) Pkt_Err);
		DLOG_INFO("CrtErr = %X \r\n",(int) CrtErr);

		ucEQPort->ucECCvalue++;
		ucEQPort->ucECCfailCount++;
	}
	else
	{
		ucEQPort->ucECCfailCount=0;
	}

//	DLOG_INFO("ucEQPort->ucECCvalue = %X 666666666666666666666666\r\n",ucEQPort->ucECCvalue);
//xxxxx 2013-0812  ++++
#if 1
	if((ucEQPort->ucECCfailCount & 0x7F) < (0x40) ) 	// before CDR reset , dont care pkt_error and code_error
	{

		if(Pkt_Err==0xFF ||Code_Err==0xFF)
			return;
	}
#endif
//xxxxx 2013-0812

//	if((ucEQPort->ucECCfailCount & 0x7F) > (0x40 + MINECCFAILCOUNT-2))

//xxxxx 2013-0812  ++++
	if((ucEQPort->ucECCfailCount & 0x0F) > (MINECCFAILCOUNT-2))
	{

		ucEQPort->ucECCvalue=MAXECCWAIT;

		ucCurrentPort = hdmirxrd(REG_RX_051)&B_PORT_SEL;

		if(ucEQPort->ucPortID==F_PORT_SEL_1)
		{
			ucStatus = hdmirxrd(REG_RX_P1_SYS_STATUS);
		}
		else
		{
			ucStatus = hdmirxrd(REG_RX_P0_SYS_STATUS);
		}

		if(ucCurrentPort == ucEQPort->ucPortID)
		{
			if(((ucStatus & B_P0_SCDT) == 0x00) || ((ucEQPort->ucECCfailCount & 0x80) != 0x00))
			{
				ucEQPort->ucECCvalue=MAXECCWAIT | 0x80; 	// 0x80 for Identify SCDT off with Ecc error
			}
		}

		StoreEccCount(ucEQPort);	// abnormal judge ucECCvalue mode


		if(ucEQPort->ucEQState<EQSTATE_START)
			HDMISwitchEQstate(ucEQPort,EQSTATE_START);
		else if(ucEQPort->ucEQState<EQSTATE_LOW)
			HDMISwitchEQstate(ucEQPort,EQSTATE_LOW);
		else if(ucEQPort->ucEQState<EQSTATE_MIDDLE)
			HDMISwitchEQstate(ucEQPort,EQSTATE_MIDDLE);
		else if(ucEQPort->ucEQState<=EQSTATE_HIGH)
			HDMISwitchEQstate(ucEQPort,EQSTATE_HIGH);
	}
//xxxxx 2013-0812

}
// disable ->
// disable -> static unsigned char CheckErrorCode(struct it6602_eq_data *ucEQPort)
// disable -> {
// disable -> 	unsigned char Receive_Err;
// disable -> 	unsigned char Video_Err;
// disable -> 	unsigned char Code_Err;
// disable -> 	unsigned char Pkt_Err;
// disable -> 	unsigned char CrtErr;
// disable -> //	unsigned char ucPortSel;
// disable ->
// disable -> 	DLOG_INFO(" CheckErrorCode( ) !!! \r\n");
// disable ->
// disable ->
// disable -> 	if(ucEQPort->ucPortID==F_PORT_SEL_1)
// disable -> 	{
// disable -> 		// !!! check ECC error register  !!!
// disable -> 		Receive_Err = hdmirxrd(REG_RX_0B7);
// disable -> 		Video_Err = hdmirxrd(REG_RX_0B8)&0xE0;
// disable -> 		Code_Err = hdmirxrd(REG_RX_0B9);
// disable -> 		Pkt_Err = hdmirxrd(REG_RX_0BA);
// disable -> 		CrtErr = hdmirxrd(REG_RX_0BB);
// disable -> 		DLOG_INFO("!!! Port 1 !!! \r\n");
// disable -> 	}
// disable -> 	else
// disable -> 	{
// disable -> 		// !!! check ECC error register  !!!
// disable -> 		Receive_Err = hdmirxrd(REG_RX_0B2);
// disable -> 		Video_Err = hdmirxrd(REG_RX_0B3)&0xE0;
// disable -> 		Code_Err = hdmirxrd(REG_RX_0B4);
// disable -> 		Pkt_Err = hdmirxrd(REG_RX_0B5);
// disable -> 		CrtErr = hdmirxrd(REG_RX_0B6);
// disable -> 		DLOG_INFO("!!! Port 0 !!! \r\n");
// disable -> 	}
// disable ->
// disable -> 	DLOG_INFO("Video_Err = %X \r\n",(int) Video_Err);
// disable -> 	DLOG_INFO("Code_Err = %X \r\n",(int) Code_Err);
// disable -> 	DLOG_INFO("Pkt_Err = %X \r\n",(int) Pkt_Err);
// disable -> 	DLOG_INFO("CrtErr = %X \r\n",(int) CrtErr);
// disable ->
// disable -> 	if(ucEQPort->ucPortID==F_PORT_SEL_1)
// disable -> 	{
// disable -> 		// !!! write one clear ECC error register  !!!
// disable -> 		hdmirxwr(REG_RX_0B7,0xFF);
// disable -> 		hdmirxwr(REG_RX_0B8,0xF0);
// disable -> 		hdmirxwr(REG_RX_0B9,0xFF);
// disable -> 		hdmirxwr(REG_RX_0BA,0xFF);
// disable -> 		hdmirxwr(REG_RX_0BB,0xFF);
// disable -> 		DLOG_INFO("!!! write on clear Port 1 !!! \r\n");
// disable -> 	}
// disable -> 	else
// disable -> 	{
// disable -> 		// !!! write one clear ECC error register  !!!
// disable -> 		hdmirxwr(REG_RX_0B2,0xFF);
// disable -> 		hdmirxwr(REG_RX_0B3,0xF0);
// disable -> 		hdmirxwr(REG_RX_0B4,0xFF);
// disable -> 		hdmirxwr(REG_RX_0B5,0xFF);
// disable -> 		hdmirxwr(REG_RX_0B6,0xFF);
// disable -> 		DLOG_INFO("!!! write on clear Port 0 !!! \r\n");
// disable -> 	}
// disable ->
// disable ->
// disable -> 	if(Pkt_Err==0xFF ||Code_Err==0xFF)
// disable -> 	{
// disable -> 		return TRUE;
// disable -> 	}
// disable -> 		return FALSE;
// disable -> }

static void HDMIJudgeECCvalue(struct it6602_eq_data *ucEQPort)
/*
 * This is the HDMIJudgeECCvalue
 * @param it6602_eq_data
 * @return void
 */
{

	//unsigned char uc;

	DLOG_INFO("!!! HDMI Judge ECCvalue( ) %X!!! \r\n",(int) ucEQPort->ucECCvalue);

	StoreEccCount(ucEQPort);	// normal judge ucECCvalue mode

	if((ucEQPort->ucECCvalue) > (MAXECCWAIT/2))
	{
		//uc = CheckErrorCode(ucEQPort);

		//if(CheckErrorCode()==FALSE)
		//if(uc == FALSE)
		{

			if(ucEQPort->ucEQState==EQSTATE_START)
				HDMISwitchEQstate(ucEQPort,EQSTATE_START);
			else if(ucEQPort->ucEQState==EQSTATE_LOW)
				HDMISwitchEQstate(ucEQPort,EQSTATE_LOW);
			else if(ucEQPort->ucEQState==EQSTATE_MIDDLE)
				HDMISwitchEQstate(ucEQPort,EQSTATE_MIDDLE);
			else if(ucEQPort->ucEQState==EQSTATE_HIGH)
				HDMISwitchEQstate(ucEQPort,EQSTATE_HIGH);
		}
	}
	else
	{
		HDMISwitchEQstate(ucEQPort,EQSTATE_END);	// quit EQadjust( )
	}


	ucEQPort->ucPkt_Err=0;
	ucEQPort->ucECCvalue=0;
	ucEQPort->ucECCfailCount=0;

}


static void HDMIAdjustEQ(struct it6602_eq_data *ucEQPort)
/*
 * This is the HDMIAdjustEQ
 * @param it6602_eq_data
 * @return void
 */
{
	unsigned char ucCurrentPort;
	ucCurrentPort = hdmirxrd(REG_RX_051)&B_PORT_SEL;

	switch(ucEQPort->ucEQState)
	{
 		case EQSTATE_WAIT:
			break;
 		case EQSTATE_START:
		case EQSTATE_LOW:
		case EQSTATE_MIDDLE:
			HDMIJudgeECCvalue(ucEQPort);
			break;

		case EQSTATE_HIGH:
			HDMIJudgeECCvalue(ucEQPort);
 			ucEQPort->ucEQState=EQSTATE_END;
			break;

		case EQSTATE_HIGH+1:
		case EQSTATE_END+1:

//xxxxx 2013-0904 for pc debug only
		ucEQPort->f_manualEQadjust = FALSE;
		ucEQPort->ucEQState	= 0xFF;
//xxxxx 2013-0904 for pc debug only

	//if(ucEQPort->ucPortID==F_PORT_SEL_0)
	//{
	//DLOG_INFO("Port=%X ,HDMIAdjustEQ END REG_RX_011 = %X \r\n",ucEQPort->ucPortID,hdmirxrd(REG_RX_011));
	//DLOG_INFO("Port=%X ,HDMIAdjustEQ END REG_RX_026 = %X \r\n",ucEQPort->ucPortID,hdmirxrd(REG_RX_026));
	//DLOG_INFO("Port=%X ,HDMIAdjustEQ END REG_RX_027 = %X \r\n",ucEQPort->ucPortID,hdmirxrd(REG_RX_027));
	//DLOG_INFO("Port=%X ,HDMIAdjustEQ END REG_RX_028 = %X \r\n",ucEQPort->ucPortID,hdmirxrd(REG_RX_028));
	//DLOG_INFO("Port=%X ,HDMIAdjustEQ END REG_RX_029 = %X \r\n",ucEQPort->ucPortID,hdmirxrd(REG_RX_029));
	//chgbank(1);
	//DLOG_INFO(" P0_Rec_B_CS=%02X  P0_Rec_B_RS=%02X  \n",(hdmirxrd(REG_RX_1D5)&0x80)>>7 , (hdmirxrd(REG_RX_1D5)&0x7F));
	//DLOG_INFO(" P0_Rec_G_CS=%02X  P0_Rec_G_RS=%02X  \n",(hdmirxrd(REG_RX_1D6)&0x80)>>7 , (hdmirxrd(REG_RX_1D6)&0x7F));
	//DLOG_INFO(" P0_Rec_R_CS=%02X  P0_Rec_R_RS=%02X  \n",(hdmirxrd(REG_RX_1D7)&0x80)>>7 , (hdmirxrd(REG_RX_1D7)&0x7F));
	//chgbank(0);
	//
	//}
	//else
	//{
	//DLOG_INFO("Port=%X ,HDMIAdjustEQ END REG_RX_018 = %X \r\n",ucEQPort->ucPortID,hdmirxrd(REG_RX_018));
	//DLOG_INFO("Port=%X ,HDMIAdjustEQ END REG_RX_03E = %X \r\n",ucEQPort->ucPortID,hdmirxrd(REG_RX_03E));
	//DLOG_INFO("Port=%X ,HDMIAdjustEQ END REG_RX_03F = %X \r\n",ucEQPort->ucPortID,hdmirxrd(REG_RX_03F));
	//DLOG_INFO("Port=%X ,HDMIAdjustEQ END REG_RX_040 = %X \r\n",ucEQPort->ucPortID,hdmirxrd(REG_RX_040));
	//DLOG_INFO("Port=%X ,HDMIAdjustEQ END REG_RX_040 = %X \r\n",ucEQPort->ucPortID,hdmirxrd(REG_RX_041));
	//chgbank(1);
	//DLOG_INFO(" P1_Rec_B_CS=%02X  P1_Rec_B_RS=%02X  \n",(hdmirxrd(REG_RX_1DD)&0x80)>>7 , (hdmirxrd(REG_RX_1DD)&0x7F));
	//DLOG_INFO(" P1_Rec_G_CS=%02X  P1_Rec_G_RS=%02X  \n",(hdmirxrd(REG_RX_1DE)&0x80)>>7 , (hdmirxrd(REG_RX_1DE)&0x7F));
	//DLOG_INFO(" P1_Rec_R_CS=%02X  P1_Rec_R_RS=%02X  \n",(hdmirxrd(REG_RX_1DF)&0x80)>>7 , (hdmirxrd(REG_RX_1DF)&0x7F));
	//chgbank(0);
	//
	//
	//}
	//DLOG_INFO(" PCLK = %X , REG_RX_011 = %X ,REG_RX_02A = %X, REG_RX_017 = %X \r\n",hdmirxrd(REG_RX_09A),hdmirxrd(REG_RX_011),hdmirxrd(REG_RX_02A),hdmirxrd(REG_RX_017));

	// disable -> get_vid_info();
	// disable -> show_vid_info();
//			DLOG_INFO("============================================================= \r\n");


	if(ucEQPort->ucPortID==ucCurrentPort)
		IT6602VideoCountClr();

 			break;
		case 0xff:
//			DLOG_INFO("====================== f_manualEQadjust = FALSE ====================== \r\n");
//			DLOG_INFO("====================== f_manualEQadjust = FALSE ====================== \r\n");
	// disable ->

		default:
			break;
	}

	if(ucEQPort->ucEQState != 0xFF)
	{

		if(ucEQPort->ucEQState < EQSTATE_WAIT)			//20120410
			HDMICheckSCDTon(ucEQPort);
		else if(ucEQPort->ucEQState < EQSTATE_HIGH)
			HDMIPollingErrorCount(ucEQPort);
//		else
//			HDMICheckErrorCount(ucEQPort);

		ucEQPort->ucEQState++;
	}
	else
	{
		ucEQPort->f_manualEQadjust = FALSE;
	}
}
// disable ->
// disable ->
// disable ->
// disable -> static void HDMI_EQ_DefaultSet(struct it6602_eq_data *ucEQPort)
// disable -> /*
// disable ->  * This is the HDMIRX auto EQ parameter reset
// disable ->  * @param it6602_eq_data
// disable ->  * @return void
// disable ->  */
// disable -> {
// disable -> 	HDMISetEQValue(ucEQPort,0);
// disable ->
// disable -> 	if(ucEQPort->ucPortID==F_PORT_SEL_1)
// disable -> 	{
// disable -> 		hdmirxset(REG_RX_03E,0x20,0x20);		//Manually set RS Value
// disable -> 		DLOG_INFO("Port=%X ,HDMI_EQ_DefaultSet REG_RX_03E = %X \r\n",(int) ucEQPort->ucPortID,(int) hdmirxrd(REG_RX_03E));
// disable -> 	}
// disable -> 	else
// disable -> 	{
// disable -> 		hdmirxset(REG_RX_026,0x20,0x20);		//Manually set RS Value
// disable -> 		DLOG_INFO("Port=%X ,HDMI_EQ_DefaultSet REG_RX_026 = %X \r\n",(int) ucEQPort->ucPortID,(int) hdmirxrd(REG_RX_026));
// disable -> 	}
// disable ->
// disable ->
// disable -> }
// disable ->
// disable -> static void HDMICheckErrorCount(struct it6602_eq_data *ucEQPort)
// disable -> /*
// disable ->  * This is the HDMIRX Check Error Count
// disable ->  * @param it6602_eq_data
// disable ->  * @return void
// disable ->  */
// disable -> {
// disable -> 	unsigned char Receive_Err;
// disable -> 	unsigned char Video_Err;
// disable -> 	unsigned char Code_Err;
// disable -> 	unsigned char Pkt_Err;
// disable -> 	unsigned char CrtErr;
// disable ->
// disable -> 	if(ucEQPort->ucPortID==F_PORT_SEL_1)
// disable -> 	{
// disable -> 		// !!! check ECC error register  !!!
// disable -> 		Receive_Err = hdmirxrd(REG_RX_0B7);
// disable -> 		Video_Err = hdmirxrd(REG_RX_0B8)&0xE0;
// disable -> 		Code_Err = hdmirxrd(REG_RX_0B9);
// disable -> 		Pkt_Err = hdmirxrd(REG_RX_0BA);
// disable -> 		CrtErr = hdmirxrd(REG_RX_0BB);
// disable -> 		DLOG_INFO("!!! Port 1 !!! \r\n");
// disable -> 	}
// disable -> 	else
// disable -> 	{
// disable -> 		// !!! check ECC error register  !!!
// disable -> 		Receive_Err = hdmirxrd(REG_RX_0B2);
// disable -> 		Video_Err = hdmirxrd(REG_RX_0B3)&0xE0;
// disable -> 		Code_Err = hdmirxrd(REG_RX_0B4);
// disable -> 		Pkt_Err = hdmirxrd(REG_RX_0B5);
// disable -> 		CrtErr = hdmirxrd(REG_RX_0B6);
// disable -> 		DLOG_INFO("!!! Port 0 !!! \r\n");
// disable -> 	}
// disable ->
// disable -> 	// !!! dont use code error [7~6] bit !!!
// disable -> 	//uc2&=0x3F;
// disable ->
// disable -> 	//if((uc1>0 && uc1<10) && (uc2>0 && uc2<10))	//20130410
// disable -> 	{
// disable -> 	DLOG_INFO("Port= %X ,HDMICheckErrorCount Receive_Err = %X \r\n",(int) ucEQPort->ucPortID,(int) Receive_Err);
// disable -> 	DLOG_INFO("Video_Err = %X \r\n",(int) Video_Err);
// disable -> 	DLOG_INFO("Code_Err = %X \r\n",(int) Code_Err);
// disable -> 	DLOG_INFO("Pkt_Err = %X \r\n",(int) Pkt_Err);
// disable -> 	DLOG_INFO("CrtErr = %X \r\n",(int) CrtErr);
// disable -> 	}
// disable ->
// disable -> 	if(ucEQPort->ucPortID==F_PORT_SEL_1)
// disable -> 	{
// disable -> 		// !!! write one clear ECC error register  !!!
// disable -> 		hdmirxwr(REG_RX_0B7,0xFF);
// disable -> 		hdmirxwr(REG_RX_0B8,0xF0);
// disable -> 		hdmirxwr(REG_RX_0B9,0xFF);
// disable -> 		hdmirxwr(REG_RX_0BA,0xFF);
// disable -> 		hdmirxwr(REG_RX_0BB,0xFF);
// disable -> 		DLOG_INFO("!!! write on clear Port 1 !!! \r\n");
// disable -> 	}
// disable -> 	else
// disable -> 	{
// disable -> 		// !!! write one clear ECC error register  !!!
// disable -> 		hdmirxwr(REG_RX_0B2,0xFF);
// disable -> 		hdmirxwr(REG_RX_0B3,0xF0);
// disable -> 		hdmirxwr(REG_RX_0B4,0xFF);
// disable -> 		hdmirxwr(REG_RX_0B5,0xFF);
// disable -> 		hdmirxwr(REG_RX_0B6,0xFF);
// disable -> 		DLOG_INFO("!!! write on clear Port 0 !!! \r\n");
// disable -> 	}
// disable ->
// disable ->
// disable -> }

//FIX_ID_010 xxxxx 	//Add JudgeBestEQ to avoid wrong EQ setting
static void StoreEccCount(struct it6602_eq_data *ucEQPort)
{

	DLOG_INFO("StoreEccCount() ucEQPort->ucECCvalue = %02X \r\n",(int) ucEQPort->ucECCvalue);

	if(ucEQPort->ucEQState <= EQSTATE_LOW)
		ucEQPort->ErrorCount[0] = ucEQPort->ucECCvalue ;
	else if(ucEQPort->ucEQState <= EQSTATE_MIDDLE)
		ucEQPort->ErrorCount[1] = ucEQPort->ucECCvalue ;
	else if(ucEQPort->ucEQState <= EQSTATE_HIGH)
	{
		ucEQPort->ErrorCount[2] = ucEQPort->ucECCvalue ;
		JudgeBestEQ(ucEQPort);
	}

}



void JudgeBestEQ(struct it6602_eq_data *ucEQPort)
{
	unsigned char i,j,Result;

		j=0;
		Result=ucEQPort->ErrorCount[0];

		for(i=1;i<MaxEQIndex;i++)
		{
//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
// use Min value to be best EQ !			if(Result>ucEQPort->ErrorCount[i])
// use Max value to be best EQ !			if(Result>=ucEQPort->ErrorCount[i])
			if(Result>=ucEQPort->ErrorCount[i])
//FIX_ID_033 xxxxx
			{
				Result=ucEQPort->ErrorCount[i];
				j=i;
			}
		}

		DLOG_INFO(" Best IT6602EQTable ErrorCount[%X]=%X !!! IT6602EQTable Value=%X !!!\n",(int) j,(int) Result,(int) IT6602EQTable[j]);

		//if(j==0 && Result==0)
//2014-0102 bug xxxxx !!!!!
		if(ucEQPort->ucPortID==F_PORT_SEL_0)
		{
#ifdef _SUPPORT_AUTO_EQ_
				if(hdmirxrd(REG_RX_027) & 0x80 == 0)
				{
					//printf(" Use Auto EQ Value \r\n",j,Result,IT6602EQTable[j]);

		//			AmpValidCheck(ucEQPort->ucPortID);
					OverWriteAmpValue2EQ(ucEQPort->ucPortID);
				}
				else
#endif
				{
					hdmirxset(REG_RX_026, 0x20, 0x20);	//07-04 add for adjust EQ
					hdmirxwr(REG_RX_027,IT6602EQTable[j]);
					DLOG_INFO("Port=%X ,ucIndex = %X ,JudgeBestEQ Reg027 = %X \r\n",(int) ucEQPort->ucPortID,(int) j,(int) hdmirxrd(REG_RX_027));
				}

		}
//#ifdef ENABLE_INTPUT_PORT1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		else
		{
#ifdef _SUPPORT_AUTO_EQ_
				if(hdmirxrd(REG_RX_03F) & 0x80 == 0)
				{
					//printf(" Use Auto EQ Value \r\n",j,Result,IT6602EQTable[j]);

		//			AmpValidCheck(ucEQPort->ucPortID);
					OverWriteAmpValue2EQ(ucEQPort->ucPortID);
				}
				else
#endif
				{

					hdmirxset(REG_RX_03E, 0x20, 0x20);	//07-04 add for adjust EQ
					hdmirxwr(REG_RX_03F,IT6602EQTable[j]);
					DLOG_INFO("Port=%X ,ucIndex = %X ,JudgeBestEQ Reg03F = %X \r\n",(int) ucEQPort->ucPortID,(int) j,(int) hdmirxrd(REG_RX_03F));

				}
		}
//-------------------------------------------------------------------------------------------------------
//#endif
//2014-0102 bug xxxxx



}

//---------------------------------------------------------------------------------------------------
static void IT6602VideoCountClr(void)
{
	struct it6602_dev_data *it6602data = get_it6602_dev_data();
	it6602data->m_VideoCountingTimer = 1;
}
#endif
//FIX_ID_001 xxxxx
//---------------------------------------------------------------------------------------------------



static unsigned char CheckAVMute(void)
{

	unsigned char ucAVMute;
	unsigned char ucPortSel;

	ucAVMute=hdmirxrd(REG_RX_0A8) & (B_P0_AVMUTE|B_P1_AVMUTE);
	ucPortSel = hdmirxrd(REG_RX_051)&B_PORT_SEL;

	if(((ucAVMute & B_P0_AVMUTE)&& (ucPortSel == F_PORT_SEL_0 ))||
	((ucAVMute & B_P1_AVMUTE)&& (ucPortSel == F_PORT_SEL_1 )))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}




//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
static unsigned char CheckPlg5VPwr(unsigned char ucPortSel)
{
	unsigned char sys_state_P0;
	unsigned char sys_state_P1;

	if(ucPortSel==0)
	{
		sys_state_P0 = hdmirxrd(REG_RX_P0_SYS_STATUS) & B_P0_PWR5V_DET;
		if((sys_state_P0 & B_P0_PWR5V_DET))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		sys_state_P1 = hdmirxrd(REG_RX_P1_SYS_STATUS) & B_P1_PWR5V_DET;
		if((sys_state_P1 & B_P1_PWR5V_DET))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}

	}
}
//FIX_ID_037 xxxxx

// ---------------------------------------------------------------------------
static unsigned char IsHDMIMode(void)
{

	unsigned char sys_state_P0;
	unsigned char sys_state_P1;
	unsigned char ucPortSel;

	sys_state_P0=hdmirxrd(REG_RX_P0_SYS_STATUS) & B_P0_HDMI_MODE;
	sys_state_P1=hdmirxrd(REG_RX_P1_SYS_STATUS) & B_P1_HDMI_MODE;
	ucPortSel = hdmirxrd(REG_RX_051) & B_PORT_SEL;

	if(((sys_state_P0 & B_P0_HDMI_MODE)&& (ucPortSel == F_PORT_SEL_0 ))||
	((sys_state_P1 & B_P1_HDMI_MODE)&& (ucPortSel == F_PORT_SEL_1 )))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

// ---------------------------------------------------------------------------
unsigned char IsVideoOn(void)
{

	struct it6602_dev_data *it6602data = get_it6602_dev_data();

	if(it6602data->m_VState == VSTATE_VideoOn )

	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}
// ---------------------------------------------------------------------------
static void GetAVIInfoFrame(struct it6602_dev_data *it6602)
{
	chgbank(2);
	it6602->ColorMode = ((hdmirxrd(REG_RX_AVI_DB1)&0x60)>>5);
	it6602->Colorimetry = ((hdmirxrd(REG_RX_AVI_DB2)&0xC0)>>6);
	it6602->ExtendedColorimetry = ((hdmirxrd(REG_RX_AVI_DB3)&0x70)>>4);
	it6602->RGBQuantizationRange = ((hdmirxrd(REG_RX_AVI_DB3)&0x0C)>>2);
	it6602->VIC = ((hdmirxrd(REG_RX_AVI_DB4)&0x7F));
	it6602->YCCQuantizationRange = ((hdmirxrd(REG_RX_AVI_DB5)&0xC0)>>6);
	chgbank(0);

//FIX_ID_027 xxxxx Support RGB limited / Full range convert
	if(it6602->RGBQuantizationRange == 0 )
	{
	if( it6602->VIC >=2 )
	{
	// CE Mode
	it6602->RGBQuantizationRange = 1 ; // limited range
	}
	else
	{
	// IT mode
	it6602->RGBQuantizationRange = 2 ; // Full range
	}
	}
//FIX_ID_027	 xxxxx

	DLOG_INFO("AVI ColorMode = %X \r\n",(int) it6602->ColorMode);
	DLOG_INFO("AVI Colorimetry = %X \r\n",(int) it6602->Colorimetry);
	DLOG_INFO("AVI ExtendedColorimetry = %X \r\n",(int) it6602->ExtendedColorimetry);
	DLOG_INFO("AVI RGBQuantizationRange = %X \r\n",(int) it6602->RGBQuantizationRange);
	DLOG_INFO("AVI VIC = %X \r\n",(int) it6602->VIC);
	DLOG_INFO("AVI YCCQuantizationRange = %X \r\n",(int) it6602->YCCQuantizationRange);
}


// ---------------------------------------------------------------------------
static void SetVideoInputFormatWithInfoFrame(struct it6602_dev_data *it6602)
{
	unsigned char i;
	unsigned char bAVIColorModeIndicated=FALSE;
//	unsigned char bOldInputVideoMode=it6602->m_bInputVideoMode;

	chgbank(2);
	i=hdmirxrd(REG_RX_215);	//REG_RX_AVI_DB1
	chgbank(0);
	it6602->m_bInputVideoMode &=~F_MODE_CLRMOD_MASK;


	switch((i>>O_AVI_COLOR_MODE)&M_AVI_COLOR_MASK)
	{
		case B_AVI_COLOR_YUV444:
			DLOG_INFO("input YUV444 mode ");
			it6602->m_bInputVideoMode |=F_MODE_YUV444;
			break;
		case B_AVI_COLOR_YUV422:
			DLOG_INFO("input YUV422 mode ");
			it6602->m_bInputVideoMode |=F_MODE_YUV422;
			break;
		case B_AVI_COLOR_RGB24:
			DLOG_INFO("input RGB24 mode ");
			it6602->m_bInputVideoMode |=F_MODE_RGB24;
			break;
		default:
			return;
	}


	DLOG_INFO("SetVideoInputFormatWithInfoFrame - RegAE=%X it6602->m_bInputVideoMode=%X\n",(int) i,(int) it6602->m_bInputVideoMode);
	i=hdmirxrd(REG_RX_IN_CSC_CTRL);
	i &=~B_IN_FORCE_COLOR_MODE;
	hdmirxwr(REG_RX_IN_CSC_CTRL,i);
}

// ---------------------------------------------------------------------------
static void SetColorimetryByInfoFrame(struct it6602_dev_data *it6602)
{
    unsigned char i;


//    if(it6602->m_NewAVIInfoFrameF)
    {
	chgbank(2);
	i=hdmirxrd(REG_RX_216);	//REG_RX_AVI_DB2
	chgbank(0);
        i &=M_AVI_CLRMET_MASK<<O_AVI_CLRMET;

        if(i==(B_AVI_CLRMET_ITU601<<O_AVI_CLRMET))
        {
            it6602->m_bInputVideoMode &=~F_MODE_ITU709;
        }
        else if(i==(B_AVI_CLRMET_ITU709<<O_AVI_CLRMET))
        {
            it6602->m_bInputVideoMode |=F_MODE_ITU709;

        }
    }

}
// ---------------------------------------------------------------------------
static void SetCSCBYPASS(struct it6602_dev_data *it6602)
{

    it6602->m_bOutputVideoMode=it6602->m_bInputVideoMode;

    switch(it6602->m_bInputVideoMode&F_MODE_CLRMOD_MASK) {
    case F_MODE_RGB24 :
	hdmirxset(REG_RX_OUT_CSC_CTRL,(M_OUTPUT_COLOR_MASK),B_OUTPUT_RGB24);
#ifdef _SUPPORT_RBSWAP_
	hdmirxset(REG_RX_064, 0x18, 0x08);
#endif
	break;
    case F_MODE_YUV422 :
	hdmirxset(REG_RX_OUT_CSC_CTRL,(M_OUTPUT_COLOR_MASK),B_OUTPUT_YUV422);
#ifdef _SUPPORT_RBSWAP_
	hdmirxset(REG_RX_064, 0x18, 0x10);
#endif
	break;
    case F_MODE_YUV444 :
	hdmirxset(REG_RX_OUT_CSC_CTRL,(M_OUTPUT_COLOR_MASK),B_OUTPUT_YUV444);
#ifdef _SUPPORT_RBSWAP_
	hdmirxset(REG_RX_064, 0x18, 0x08);
#endif
	break;
    }

}


#if 0



void DumpCSCReg(void)
{
    ushort	i,j ;
    BYTE ucData ;

    printf("\r\n       ") ;
    for(j = 0 ; j < 16 ; j++)
    {
        printf(" %02X",(int) j) ;
        if((j == 3)||(j==7)||(j==11))
        {
                printf(" :") ;
        }
    }
   printf("\r\n") ;

    chgbank(1);

    for(i = 0x70 ; i < 0x90 ; i+=16)
    {
        printf("[%03X]  ",i) ;
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = hdmirxrd((BYTE)((i+j)&0xFF)) ;
            printf(" %02X",(int) ucData) ;
            if((j == 3)||(j==7)||(j==11))
            {
                printf(" :") ;
            }
        }
        printf("\r\n") ;

    }

   printf("\n        =====================================================\r\n") ;

    chgbank(0);
    for(i = 0x60 ; i < 0x70 ; i+=16)
    {
        printf("[%03X]  ",i) ;
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = hdmirxrd((BYTE)((i+j)&0xFF)) ;
            printf(" %02X",(int) ucData) ;
            if((j == 3)||(j==7)||(j==11))
            {
                printf(" :") ;
            }
        }
        printf("\r\n") ;

    }

}

void IT6602_UpdateCSC(unsigned char *cAVIInfoFrame,unsigned char *cHDMI_DVI_mode,unsigned char *cHDMI_Input_CSC,unsigned char *cRGBQuantData)
{

	unsigned char ucInputYUV;
	unsigned char ucITU709;
	unsigned char ucInputFullRange;
	//unsigned char ucOutputYUV;

	struct it6602_dev_data *it6602data = get_it6602_dev_data();

	GetAVIInfoFrame(it6602data);

	*cHDMI_Input_CSC = it6602data->ColorMode;

	if((it6602data->ColorMode == 1) || (it6602data->ColorMode == 2))
	{
		ucInputYUV=1;

		if((it6602data->YCCQuantizationRange == 1))
		{
			ucInputFullRange = 1;
		}
		else
		{
			ucInputFullRange = 0;
		}

		if((it6602data->Colorimetry == 2))
		{
			ucITU709 = 1;
			*cHDMI_Input_CSC = 2;	//Receiver_YUV_709;
		}
		else
		{
			ucITU709 = 0;
			*cHDMI_Input_CSC = 1;	//Receiver_YUV_601;
		}

	}
	else
	{
		ucInputYUV=0;
		*cHDMI_Input_CSC = 0;	//Receiver_RGB;

		if((it6602data->RGBQuantizationRange == 2))
			ucInputFullRange = 1;
		else
			ucInputFullRange = 0;
	}

	if(ucInputFullRange == 1)
	{
		*cRGBQuantData = 2;
	}
	else
	{
		*cRGBQuantData = 1;
	}

	if(IsHDMIMode()==TRUE)
	{
		*cAVIInfoFrame = TRUE;
		*cHDMI_DVI_mode  = TRUE;
	}
	else
	{
		*cAVIInfoFrame = FALSE;
		*cHDMI_DVI_mode  = FALSE;
	}

}


void SetCSCbyOSD(unsigned char ucInputYUV, unsigned char ucITU709, unsigned char ucFullRange, unsigned char ucOutputYUV)
{
	//struct it6602_dev_data *it6602data = get_it6602_dev_data();

	if(ucOutputYUV ==0)
	{
		//Convert input RGB / YUV To output RGB444
		// 0. RGB / YUV
		// 1. ITU601 / ITU709
		// 2. Data Range ( Limit / Full )
		pCSCLookup = CSCOutputRGB[ucInputYUV][ucITU709][ucFullRange];
		pCSCLookupConfig = &IT6602_OUTPUT_RGB_CONFIG_REG[ucInputYUV][ucFullRange];
	}
	else
	{
		//Convert input RGB / YUV To output YUV444
		// 0. RGB / YUV
		// 1. ITU601 / ITU709
		// 2. Data Range ( Limit / Full )
		pCSCLookup = CSCOutputYUV[ucInputYUV][ucITU709][ucFullRange];
		pCSCLookupConfig = &IT6602_OUTPUT_YUV_CONFIG_REG[ucInputYUV][ucFullRange];
	}


	chgbank(1);
	hdmirxbwr(REG_RX_170,21,pCSCLookup);
	chgbank(0);
	hdmirxset(REG_RX_065,0x33, pCSCLookupConfig->ucReg65);
	hdmirxwr(REG_RX_067,pCSCLookupConfig->ucReg67);
	hdmirxwr(REG_RX_068,pCSCLookupConfig->ucReg68);

	if(ucOutputYUV == 1)
	{
		if(ucInputYUV == 0)
		{
			hdmirxset(REG_RX_065,0x30, 0x10);	// Input RGB Outptu YUV422 with CSC
		}
		else
		{
			hdmirxset(REG_RX_065,0x33, 0x10);	// Input YUV Outptu YUV422 with bypass CSC
		}

	}

	DumpCSCReg();
}
//xxxxx
#endif
// ---------------------------------------------------------------------------
static void SetColorSpaceConvert(struct it6602_dev_data *it6602)
{
	unsigned char csc ;
	//    unsigned char uc ;
	unsigned char filter = 0 ; // filter is for Video CTRL DN_FREE_GO, EN_DITHER, and ENUDFILT

#ifdef DISABLE_HDMI_CSC
	DLOG_INFO("ITEHDMI - HDMI Color Space Convert is disabled \r\n");

	csc = B_CSC_BYPASS ;
	it6602->m_bOutputVideoMode = it6602->m_bInputVideoMode;

#else
	DLOG_INFO("\n!!! SetColorSpaceConvert( ) !!!\n");

//FIX_ID_039 xxxxx fix image flick when enable RGB limited / Full range convert
//xxxxx FIX_ID_039 disable --> //FIX_ID_027 xxxxx Support Full/Limited Range convert
//xxxxx FIX_ID_039 disable --> 	//default to turn off CSC offset
//xxxxx FIX_ID_039 disable --> 	hdmirxset(REG_RX_067,0x78,0x00);
//xxxxx FIX_ID_039 disable --> 	hdmirxwr(REG_RX_068,0x00);
//xxxxx FIX_ID_039 disable --> //FIX_ID_027 xxxxx
//FIX_ID_039 xxxxx

//FIX_ID_039 xxxxx fix image flick when enable RGB limited / Full range convert
#ifdef _AVOID_REDUNDANCE_CSC_
	if((it6602->m_Backup_OutputVideoMode == it6602->m_bOutputVideoMode) &&  (it6602->m_Backup_InputVideoMode == it6602->m_bInputVideoMode))
	{
		DLOG_INFO("I/P and O/P color without change , No need to setup CSC convert again \n");
		return;
	}
#endif
//FIX_ID_039 xxxxx

    //DLOG_INFO("Input mode is YUV444 ");
    switch(it6602->m_bOutputVideoMode&F_MODE_CLRMOD_MASK)
    {
    #if defined(SUPPORT_OUTPUTYUV444)
    case F_MODE_YUV444:
         DLOG_INFO("Output mode is YUV444\n");
	    switch(it6602->m_bInputVideoMode&F_MODE_CLRMOD_MASK)
	    {
	    case F_MODE_YUV444:
             DLOG_INFO("Input mode is YUV444\n");
	        csc = B_CSC_BYPASS ;
	        break ;
	    case F_MODE_YUV422:
             DLOG_INFO("Input mode is YUV422\n");
            csc = B_CSC_BYPASS ;
            if(it6602->m_bOutputVideoMode & F_MODE_EN_UDFILT)// RGB24 to YUV422 need up/dn filter.
            {
                filter |= B_RX_EN_UDFILTER ;
            }

            if(it6602->m_bOutputVideoMode & F_MODE_EN_DITHER)// RGB24 to YUV422 need up/dn filter.
            {
                filter |= B_RX_EN_UDFILTER | B_RX_DNFREE_GO ;
            }

            break ;
	    case F_MODE_RGB24:
             DLOG_INFO("Input mode is RGB444\n");
            csc = B_CSC_RGB2YUV ;
            break ;
	    }
        break ;
    #endif

    #if defined(SUPPORT_OUTPUTYUV422)

    case F_MODE_YUV422:
	    switch(it6602->m_bInputVideoMode&F_MODE_CLRMOD_MASK)
	    {
	    case F_MODE_YUV444:
             DLOG_INFO("Input mode is YUV444\n");
	        if(it6602->m_bOutputVideoMode & F_MODE_EN_UDFILT)
	        {
	            filter |= B_RX_EN_UDFILTER ;
	        }
	        csc = B_CSC_BYPASS ;
	        break ;
	    case F_MODE_YUV422:
             DLOG_INFO("Input mode is YUV422\n");
            csc = B_CSC_BYPASS ;

            // if output is YUV422 and 16 bit or 565, then the dither is possible when
            // the input is YUV422 with 24bit input, however, the dither should be selected
            // by customer, thus the requirement should set in ROM, no need to check
            // the register value .
            if(it6602->m_bOutputVideoMode & F_MODE_EN_DITHER)// RGB24 to YUV422 need up/dn filter.
            {
                filter |= B_RX_EN_UDFILTER | B_RX_DNFREE_GO ;
            }
	    	break ;
	    case F_MODE_RGB24:
             DLOG_INFO("Input mode is RGB444\n");
            if(it6602->m_bOutputVideoMode & F_MODE_EN_UDFILT)// RGB24 to YUV422 need up/dn filter.
            {
                filter |= B_RX_EN_UDFILTER ;
            }
            csc = B_CSC_RGB2YUV ;
	    	break ;
	    }
	    break ;
    #endif

    #if defined(SUPPORT_OUTPUTRGB)
    case F_MODE_RGB24:
         DLOG_INFO("Output mode is RGB24\n");
	    switch(it6602->m_bInputVideoMode&F_MODE_CLRMOD_MASK)
	    {
	    case F_MODE_YUV444:
             DLOG_INFO("Input mode is YUV444\n");
	        csc = B_CSC_YUV2RGB ;
	        break ;
	    case F_MODE_YUV422:
             DLOG_INFO("Input mode is YUV422\n");
            csc = B_CSC_YUV2RGB ;
            if(it6602->m_bOutputVideoMode & F_MODE_EN_UDFILT)// RGB24 to YUV422 need up/dn filter.
            {
                filter |= B_RX_EN_UDFILTER ;
            }
            if(it6602->m_bOutputVideoMode & F_MODE_EN_DITHER)// RGB24 to YUV422 need up/dn filter.
            {
                filter |= B_RX_EN_UDFILTER | B_RX_DNFREE_GO ;
            }
	    	break ;
	    case F_MODE_RGB24:
             DLOG_INFO("Input mode is RGB444\n");
            csc = B_CSC_BYPASS ;
	    	break ;
	    }
	    break ;
    #endif
    }


    #if defined(SUPPORT_OUTPUTYUV)
    // set the CSC associated registers
    if(csc == B_CSC_RGB2YUV)
    {
        // DLOG_INFO("CSC = RGB2YUV ");
	//FIX_ID_039 xxxxx fix image flick when enable RGB limited / Full range convert
		//default to turn off CSC offset
		hdmirxset(REG_RX_067,0x78,0x00);
		hdmirxwr(REG_RX_068,0x00);
		DLOG_INFO(" Clear Reg67 and Reg68 ... \r\n");
	//FIX_ID_039 xxxxx
        if(it6602->m_bInputVideoMode & F_MODE_ITU709)
        {
            DLOG_INFO("ITU709 ");

            if(it6602->m_bInputVideoMode & F_MODE_16_235)
            {
                DLOG_INFO(" 16-235\n");
		chgbank(1);	//for CSC setting Reg170 ~ Reg184 !!!!
                hdmirxbwr(REG_RX_170,sizeof(bCSCMtx_RGB2YUV_ITU709_16_235),&bCSCMtx_RGB2YUV_ITU709_16_235[0]);
            }
            else
            {
                DLOG_INFO(" 0-255\n");
		chgbank(1);	//for CSC setting Reg170 ~ Reg184 !!!!
                hdmirxbwr(REG_RX_170,sizeof(bCSCMtx_RGB2YUV_ITU709_0_255),&bCSCMtx_RGB2YUV_ITU709_0_255[0]);
            }
        }
        else
        {
            DLOG_INFO("ITU601 ");
            if(it6602->m_bInputVideoMode & F_MODE_16_235)
            {
		chgbank(1);	//for CSC setting Reg170 ~ Reg184 !!!!
                hdmirxbwr(REG_RX_170,sizeof(bCSCMtx_RGB2YUV_ITU601_16_235),&bCSCMtx_RGB2YUV_ITU601_16_235[0]);
                DLOG_INFO(" 16-235\n");
            }
            else
            {
		chgbank(1);	//for CSC setting Reg170 ~ Reg184 !!!!
                hdmirxbwr(REG_RX_170,sizeof(bCSCMtx_RGB2YUV_ITU601_0_255),&bCSCMtx_RGB2YUV_ITU601_0_255[0]);
                DLOG_INFO(" 0-255\n");
            }
        }
    }
    #endif
    #if defined(SUPPORT_OUTPUTRGB)
	if(csc == B_CSC_YUV2RGB)
    {
        DLOG_INFO("CSC = YUV2RGB ");
	//FIX_ID_039 xxxxx fix image flick when enable RGB limited / Full range convert
		//default to turn off CSC offset
		hdmirxset(REG_RX_067,0x78,0x00);
		hdmirxwr(REG_RX_068,0x00);
		DLOG_INFO(" Clear Reg67 and Reg68 ... \r\n");
	//FIX_ID_039 xxxxx
        if(it6602->m_bInputVideoMode & F_MODE_ITU709)
        {
            DLOG_INFO("ITU709 ");
            if(it6602->m_bOutputVideoMode & F_MODE_16_235)
            {
                DLOG_INFO("16-235\n");
		chgbank(1);	//for CSC setting Reg170 ~ Reg184 !!!!
                hdmirxbwr(REG_RX_170,sizeof(bCSCMtx_YUV2RGB_ITU709_16_235),&bCSCMtx_YUV2RGB_ITU709_16_235[0]);
            }
            else
            {
                DLOG_INFO("0-255\n");
		chgbank(1);	//for CSC setting Reg170 ~ Reg184 !!!!
                hdmirxbwr(REG_RX_170,sizeof(bCSCMtx_YUV2RGB_ITU709_0_255),&bCSCMtx_YUV2RGB_ITU709_0_255[0]);
            }
        }
        else
        {
            DLOG_INFO("ITU601 ");
            if(it6602->m_bOutputVideoMode & F_MODE_16_235)
            {
                DLOG_INFO("16-235\n");
		chgbank(1);	//for CSC setting Reg170 ~ Reg184 !!!!
                hdmirxbwr(REG_RX_170,sizeof(bCSCMtx_YUV2RGB_ITU601_16_235),&bCSCMtx_YUV2RGB_ITU601_16_235[0]);
            }
            else
            {
                DLOG_INFO("0-255\n");
		chgbank(1);	//for CSC setting Reg170 ~ Reg184 !!!!
                hdmirxbwr(REG_RX_170,sizeof(bCSCMtx_YUV2RGB_ITU601_0_255),&bCSCMtx_YUV2RGB_ITU601_0_255[0]);

            }
        }

    }

//FIX_ID_027 xxxxx Support Full/Limited Range convert
	if(csc == B_CSC_BYPASS)
	{

		if((it6602->m_bInputVideoMode&F_MODE_CLRMOD_MASK)==F_MODE_RGB24)
		{
			if(it6602->RGBQuantizationRange == 1)	// Limited range from HDMI source
			{
				 if((it6602->m_bOutputVideoMode & F_MODE_16_235)!=F_MODE_16_235)	// Full range to back-end device
				{
					// RedText;
					DLOG_INFO(" bCSCMtx_RGB_16_235_RGB_0_255 \r\n");
					// printf("pccmd w 65 02 90;\r\n");
					// printf("pccmd w 67 78 90;\r\n");
					// printf("pccmd w 68 ED 90;\r\n");
					// WhileText;
					csc = B_CSC_RGB2YUV;
					chgbank(1);	//for CSC setting Reg170 ~ Reg184 !!!!
					hdmirxbwr(REG_RX_170,sizeof(bCSCMtx_RGB_16_235_RGB_0_255),&bCSCMtx_RGB_16_235_RGB_0_255[0]);
					chgbank(0);
					//hdmirxset(REG_RX_065,0x03,0x02);	// B_CSC_RGB2YUV
					hdmirxset(REG_RX_067,0x78,0x78);
					hdmirxwr(REG_RX_068,0xED);
				 }
			}
			else if(it6602->RGBQuantizationRange == 2) //Full range from HDMI source
			{
				 if((it6602->m_bOutputVideoMode & F_MODE_16_235)==F_MODE_16_235)	// Limited range to back-end device
				{
					// RedText;
					DLOG_INFO(" bCSCMtx_RGB_0_255_RGB_16_235 \r\n");
					// printf("pccmd w 65 02 90;\r\n");
					// printf("pccmd w 67 40 90;\r\n");
					// printf("pccmd w 68 10 90;\r\n");
					// WhileText;
					csc = B_CSC_RGB2YUV;
					chgbank(1);	//for CSC setting Reg170 ~ Reg184 !!!!
					hdmirxbwr(REG_RX_170,sizeof(bCSCMtx_RGB_0_255_RGB_16_235),&bCSCMtx_RGB_0_255_RGB_16_235[0]);
					chgbank(0);
					//hdmirxset(REG_RX_065,0x03,0x02);	// B_CSC_RGB2YUV
					hdmirxset(REG_RX_067,0x78,0x40);
					hdmirxwr(REG_RX_068,0x10);
				 }
			}
		}
	}
//FIX_ID_027 xxxxx

	#endif // SUPPORT_OUTPUTRGB

#endif	//end of DISABLE_HDMI_CSC

	chgbank(0);
    	hdmirxset(REG_RX_OUT_CSC_CTRL,(M_CSC_SEL_MASK),csc);

    // set output Up/Down Filter, Dither control
	hdmirxset(REG_RX_VIDEO_CTRL1,(B_RX_DNFREE_GO|B_RX_EN_DITHER|B_RX_EN_UDFILTER),filter);
//FIX_ID_039 xxxxx fix image flick when enable RGB limited / Full range convert
	if(csc == B_CSC_BYPASS)
	{
		//default to turn off CSC offset
		hdmirxset(REG_RX_067,0x78,0x00);
		hdmirxwr(REG_RX_068,0x00);
	}

#ifdef _AVOID_REDUNDANCE_CSC_
	it6602->m_Backup_OutputVideoMode = it6602->m_bOutputVideoMode;
	it6602->m_Backup_InputVideoMode = it6602->m_bInputVideoMode;
#endif
//FIX_ID_039 xxxxx
}

// ---------------------------------------------------------------------------
static void SetNewInfoVideoOutput(struct it6602_dev_data *it6602)
{

	DLOG_INFO("SetNewInfoVideoOutput() \n");

	SetVideoInputFormatWithInfoFrame(it6602);
	SetColorimetryByInfoFrame(it6602);
	SetColorSpaceConvert(it6602);

	SetVideoOutputColorFormat(it6602);	//2013-0502

//	get_vid_info();
//	show_vid_info();

}

// ---------------------------------------------------------------------------
static void SetVideoInputFormatWithoutInfoFrame(struct it6602_dev_data *it6602,unsigned char bInMode)
{
    unsigned char i;

    i=hdmirxrd(REG_RX_IN_CSC_CTRL);
    i |=B_IN_FORCE_COLOR_MODE;

    i &=(~M_INPUT_COLOR_MASK);
    it6602->m_bInputVideoMode &=~F_MODE_CLRMOD_MASK;

    switch(bInMode)
    {
    case F_MODE_YUV444:
	i |=B_INPUT_YUV444;
	it6602->m_bInputVideoMode |=F_MODE_YUV444;
	break;
    case F_MODE_YUV422:
	i |=B_INPUT_YUV422;
        it6602->m_bInputVideoMode |=F_MODE_YUV422;
        break;
    case F_MODE_RGB24:
	i |=B_INPUT_RGB24;
        it6602->m_bInputVideoMode |=F_MODE_RGB24;
        break;
    default:
        return;
    }
    hdmirxwr(REG_RX_IN_CSC_CTRL,i);

}
// ---------------------------------------------------------------------------
static void SetColorimetryByMode(struct it6602_dev_data *it6602)
{
    unsigned char  RxClkXCNT;
    RxClkXCNT=hdmirxrd(REG_RX_PIXCLK_SPEED);

   DLOG_INFO(" SetColorimetryByMode() REG_RX_PIXCLK_SPEED=%X \n", (int) RxClkXCNT);

    it6602->m_bInputVideoMode &=~F_MODE_ITU709;

     if(RxClkXCNT <0x34)
    {

        it6602->m_bInputVideoMode |=F_MODE_ITU709;
    }
    else
    {

        it6602->m_bInputVideoMode &=~F_MODE_ITU709;
    }

}
// ---------------------------------------------------------------------------
static void SetDVIVideoOutput(struct it6602_dev_data *it6602)
{
	DLOG_INFO("SetDVIVideoOutput() \n");

	SetVideoInputFormatWithoutInfoFrame(it6602,F_MODE_RGB24);
	SetColorimetryByMode(it6602);
	SetColorSpaceConvert(it6602);

	SetVideoOutputColorFormat(it6602);	//2013-0502
}



//FIX_ID_003 xxxxx	//Add IT6602 Video Output Configure setting
static void IT6602_VideoOutputModeSet(struct it6602_dev_data *it6602)
{
	unsigned char ucReg51;
	unsigned char ucReg65;

	DLOG_INFO("IT6602_VideoOutputModeSet() \r\n");

	
	DLOG_INFO("+++ %s",VModeStateStr[(unsigned char)it6602->m_VidOutConfigMode]);


	ucReg51 = hdmirxrd(REG_RX_051)&0x9B;	// Reg51 [6] Half PCLK DDR , [5] Half Bus DDR , [2] CCIR656 mode
	ucReg65 = hdmirxrd(REG_RX_065)&0x0F;	// Reg65 [7] BTA1004Fmt , [6] SyncEmb , [5:4] output color 0x00 RGB, 0x10 YUV422, 0x20 YUV444

	switch((it6602->m_bOutputVideoMode&F_MODE_CLRMOD_MASK))
	{
		case F_MODE_RGB444:
			ucReg65|=(B_OUTPUT_RGB24);		// 0x00 B_OUTPUT_RGB24
			#ifdef _SUPPORT_RBSWAP_
			hdmirxset(REG_RX_064, 0x18, 0x08);
			#endif
			break;
		case F_MODE_YUV422:
			ucReg65|=(B_OUTPUT_YUV422);		// 0x10 B_OUTPUT_YUV422
			#ifdef _SUPPORT_RBSWAP_
			hdmirxset(REG_RX_064, 0x18, 0x10);
			#endif
			break;
		case F_MODE_YUV444:
			ucReg65|=(B_OUTPUT_YUV444);		// 0x20 B_OUTPUT_YUV444
			#ifdef _SUPPORT_RBSWAP_
			hdmirxset(REG_RX_064, 0x18, 0x10);
			#endif
			break;
	}


	switch(it6602->m_VidOutDataTrgger)
	{
		case eSDR:
			break;

		case eHalfPCLKDDR:
			ucReg51|=(B_HALF_PCLKC);			// 0x40 half PCLK
			break;

		case eHalfBusDDR:
			ucReg51|=(B_OUT_DDR);				// 0x20 half bus
			break;

		case eSDR_BTA1004:
			ucReg65|=(B_BTA1004Fmt|B_SyncEmb) ;	// 0x80 BTA1004 + 0x40 SyncEmb
			break;

		case eDDR_BTA1004:
			ucReg51|=(B_HALF_PCLKC);			// 0x40 half PCLK
			ucReg65|=(B_BTA1004Fmt|B_SyncEmb) ;	// 0x80 BTA1004 + 0x40 SyncEmb
			break;

	}

	switch(it6602->m_VidOutSyncMode)
	{
		case eSepSync:
			break;

		case eEmbSync:
			ucReg65|=(B_SyncEmb) ;	// 0x40 SyncEmb
			break;

		case eCCIR656SepSync:
			ucReg51|=(B_CCIR656);	// 0x04 CCIR656
			break;

		case eCCIR656EmbSync:
			ucReg51|=(B_CCIR656);	// 0x04 CCIR656
			ucReg65|=(B_SyncEmb) ;	// 0x40 SyncEmb
			break;
	}


	DLOG_INFO("Reg51 = %X ",(int) ucReg51);
	DLOG_INFO("Reg65 = %X\r\n",(int) ucReg65);

	hdmirxwr(REG_RX_051,ucReg51);
	hdmirxwr(REG_RX_065,ucReg65);


}
//FIX_ID_003 xxxxx


static void IT6602VideoOutputConfigure(struct it6602_dev_data *it6602)
{

	// Configure Output color space convert

//06-27 disable -->	#ifndef DISABLE_HDMI_CSC
//06-27 disable --> 	it6602->m_bOutputVideoMode = HDMIRX_OUTPUT_VID_MODE ;
//06-27 disable -->	#endif

	it6602->m_bUpHDMIMode =IsHDMIMode();
	if(it6602->m_bUpHDMIMode==FALSE)
	{
	SetDVIVideoOutput(it6602);
	}
	else
	{
//FIX_ID_027 xxxxx		//Support RGB limited / Full range convert
		GetAVIInfoFrame(it6602);
		SetNewInfoVideoOutput(it6602);
//FIX_ID_027 xxxxx
	}
	it6602->m_NewAVIInfoFrameF=FALSE;

	// Configure Output Color Depth

	it6602->GCP_CD = ((hdmirxrd(0x99)&0xF0)>>4);
	switch( it6602->GCP_CD ) {
	case 5 :
	DLOG_INFO( "\n Output ColorDepth = 30 bits per pixel\r\n");
	hdmirxset(0x65, 0x0C, 0x04);
	break;
	case 6 :
	DLOG_INFO( "\n Output ColorDepth = 36 bits per pixel\r\n");
	hdmirxset(0x65, 0x0C, 0x08);
	break;
	default :
	DLOG_INFO( "\n Output ColorDepth = 24 bits per pixel\r\n");
	hdmirxset(0x65, 0x0C, 0x00);
	break;
	}

	// Configure TTL Video Output mode
	IT6602_VideoOutputModeSet(it6602);

}

// ---------------------------------------------------------------------------
static void SetVideoOutputColorFormat(struct it6602_dev_data *it6602)
{
	switch(it6602->m_bOutputVideoMode&F_MODE_CLRMOD_MASK) {
		case F_MODE_RGB24 :
			hdmirxset(REG_RX_OUT_CSC_CTRL,(M_OUTPUT_COLOR_MASK),B_OUTPUT_RGB24);
			#ifdef _SUPPORT_RBSWAP_
				hdmirxset(REG_RX_064, 0x18, 0x08);
			#endif

			break;
		case F_MODE_YUV422 :
			hdmirxset(REG_RX_OUT_CSC_CTRL,(M_OUTPUT_COLOR_MASK),B_OUTPUT_YUV422);
			#ifdef _SUPPORT_RBSWAP_
				hdmirxset(REG_RX_064, 0x18, 0x10);
			#endif
			break;
		case F_MODE_YUV444 :
			hdmirxset(REG_RX_OUT_CSC_CTRL,(M_OUTPUT_COLOR_MASK),B_OUTPUT_YUV444);
			#ifdef _SUPPORT_RBSWAP_
				hdmirxset(REG_RX_064, 0x18, 0x08);
			#endif
			break;
    }
}

void it6602PortSelect(unsigned char ucPortSel)
{
	struct it6602_dev_data *it6602data = get_it6602_dev_data();

#if defined(_IT6602_) || defined(_IT66023_)
	if(ucPortSel == F_PORT_SEL_0 )
		hdmirxset(REG_RX_051, B_PORT_SEL, F_PORT_SEL_0); //select port 0
	else
		hdmirxset(REG_RX_051, B_PORT_SEL, F_PORT_SEL_1); //select port 1
#else
	hdmirxset(REG_RX_051, B_PORT_SEL, F_PORT_SEL_0); //select port 0
	it6602data->m_ucCurrentHDMIPort = F_PORT_SEL_0;
#endif

	if(it6602data->m_ucCurrentHDMIPort != ucPortSel)
	{

//xxxxx 2013-0801 disable HPD Control
#if 0
		if(ucPortSel == F_PORT_SEL_0)
		{
			//set port 0 HPD=0
			chgbank(1);
			hdmirxset(REG_RX_1B0, 0x03, 0x01); //clear port 0 HPD=1 for EDID update
			chgbank(0);

			IT_Delay(300);

			//set port 0 HPD=1
			chgbank(1);
			hdmirxset(REG_RX_1B0, 0x03, 0x03);
			chgbank(0);


		}
		else
		{
			HotPlug(0);

			IT_Delay(300);

			//set port 1 HPD=1
			HotPlug(1);
		}
#endif
//xxxxx



		IT6602SwitchVideoState(it6602data,VSTATE_SyncWait);
		it6602data->m_ucCurrentHDMIPort = ucPortSel;
		DLOG_INFO("it6602PortSelect = %X \r\n",(int) ucPortSel);
	}

}

void it6602HPDCtrl(unsigned char ucport,unsigned char ucEnable)
{

	struct it6602_dev_data *it6602data = get_it6602_dev_data();

	if(ucport == 0)
	{
		if(ucEnable == 0)
		{

			// Disable HDMI DDC Bus to access ITEHDMI EDID RAM
			//hdmirxset(REG_RX_0C0, 0x01, 0x01);                            // HDMI RegC0[1:0]=11 for disable HDMI DDC bus to access EDID RAM

			DLOG_INFO("Port 0 HPD HDMI 00000 \r\n");
			chgbank(1);
			hdmirxset(REG_RX_1B0, 0x03, 0x01); //clear port 0 HPD=1 for EDID update
			chgbank(0);

		}
		else
		{


			if((hdmirxrd(REG_RX_P0_SYS_STATUS) & B_P0_PWR5V_DET))
			{


				// Enable HDMI DDC bus to access ITEHDMI EDID RAM
				//hdmirxset(REG_RX_0C0, 0x01, 0x00);                        // HDMI RegC0[1:0]=00 for enable HDMI DDC bus to access EDID RAM

//FIX_ID_036	xxxxx
				DLOG_INFO("Port 0 HPD HDMI 11111 \r\n");
				chgbank(1);
				hdmirxset(REG_RX_1B0, 0x03, 0x03); //set port 0 HPD=1
				chgbank(0);
			}
		}
	}
#if defined(_IT6602_) || defined(_IT66023_)
	else
	{
		 if(ucEnable)
		 {
			// Enable HDMI DDC bus to access ITEHDMI EDID RAM
			//hdmirxset(REG_RX_0C0, 0x02, 0x00);                        // HDMI RegC0[1:0]=00 for enable HDMI DDC bus to access EDID RAM

			DLOG_INFO("Port 1 HPD 11111 \r\n");
		 	gpHPD0 = HPDON;
		 }
		 else
		 {
			// Disable HDMI DDC Bus to access ITEHDMI EDID RAM
			//hdmirxset(REG_RX_0C0, 0x02, 0x02);                            // HDMI RegC0[1:0]=11 for disable HDMI DDC bus to access EDID RAM

			DLOG_INFO("Port 1 HPD 00000 \r\n");
		 	gpHPD0 = HPDOFF;
		 }
	}
#endif
}






static void hdmirx_ECCTimingOut(unsigned char ucport)
{
	DLOG_INFO("CDR reset for hdmirx_ECCTimingOut()  \r\n");

	if(ucport == F_PORT_SEL_0)
	{
		//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
		//FIX_ID_035	xxxxx //For MTK6592 HDMI to SII MHL TX compliance issue
		// disable -> 		if((hdmirxrd(0x0A)&0x40))
		// disable -> 		//if( MHL_Mode )
		// disable -> 		{
		// disable -> 		mhlrxset(MHL_RX_28,0x40,0x40);
		// disable -> 		//it6602HPDCtrl(0,1);
		// disable -> 		IT_Delay(100);
		// disable -> 		//it6602HPDCtrl(0,0);
		// disable -> 		mhlrxset(MHL_RX_28,0x40,0x00);
		// disable ->
		// disable -> 		}
		// disable -> 		else
		// disable -> 		{
		// disable -> 		it6602HPDCtrl(0,0);	// MHL port , set HPD = 0
		// disable -> 		}
		//FIX_ID_035	xxxxx
		//FIX_ID_033 xxxxx


		it6602HPDCtrl(0,0);	// MHL port , set HPD = 0

		hdmirxset(REG_RX_011,(B_P0_DCLKRST|B_P0_CDRRST|B_P0_HDCPRST|B_P0_SWRST),(B_P0_DCLKRST|B_P0_CDRRST|B_P0_HDCPRST|B_P0_SWRST));
		IT_Delay(300);
		hdmirxset(REG_RX_011,(B_P0_DCLKRST|B_P0_CDRRST|B_P0_HDCPRST|B_P0_SWRST),0x00);

		//set port 0 HPD=1
		it6602HPDCtrl(0,1);	// MHL port , set HPD = 1

	}
	else
	{
		//set port 1 HPD=0
		it6602HPDCtrl(1,0);	// HDMI port , set HPD = 0


		hdmirxset(REG_RX_018,(B_P1_DCLKRST|B_P1_CDRRST|B_P1_HDCPRST|B_P1_SWRST),(B_P1_DCLKRST|B_P1_CDRRST|B_P1_HDCPRST|B_P1_SWRST));
		IT_Delay(300);
		hdmirxset(REG_RX_018,(B_P1_DCLKRST|B_P1_CDRRST|B_P1_HDCPRST|B_P1_SWRST),0x00);

		//set port 1 HPD=1
		it6602HPDCtrl(1,1);	// HDMI port , set HPD = 1
	}
}

#endif

#ifdef _SUPPORT_HDCP_REPEATER_
//FIX_ID_032 xxxxx	//Support HDCP Repeater function for HDMI Tx device

// ---------------------------------------------------------------------------
// ITEHDMI callback function
// ---------------------------------------------------------------------------
_CODE ITEHDMICallbackList_t  ITEHDMICallbackList[]=
{
	ITEHDMI_AVMute_Set_Notify,						//	eAVMUTE_SET_NOTIFY
	ITEHDMI_AVMute_Clear_Notify,						//	eAVMUTE_CLEAR_NOTIFY,
	ITEHDMI_Aksv_Written_Notify,						//	eAKSV_WRITTEN_NOTIFY,
	ITEHDMI_Authentication_Complete_Notify,			//	eAUTHENTICATION_COMPLETE_NOTIFY,
	ITEHDMI_Authentication_5_Sec_Timeout_Notify,		//	eAUTHENTICATION_5_SEC_TIMEOUT_NOTIFY,
	//	    eINVALID_ITEHDMI_Notify
};

// ---------------------------------------------------------------------------
// ITEHDMI callback function
// ---------------------------------------------------------------------------
void ITEHDMI_AVMute_Set_Notify(void)
{
	// ITEHDMI receive the AV mute set then HDMI Tx need to do something in here.
}

void ITEHDMI_AVMute_Clear_Notify(void)
{
	// ITEHDMI receive the AV mute clear then HDMI Tx need to do something in here.
}

void ITEHDMI_Aksv_Written_Notify(void)
{
	//ITEHDMI receive the Part 1 of authentication then Trigger HDMI TX to start part 1 of authentication.

	//disable ->  bEnableAuth = TRUE ;
	//disable ->  Tx_SwitchHDCPState(TxHDCP_Off);

}

void ITEHDMI_Authentication_Complete_Notify(void)
{

}

void ITEHDMI_Authentication_5_Sec_Timeout_Notify(void)
{

}


void ITEHDMI_Event_Notify_Callback(eITEHDMIEventNotify event)
{

	struct it6602_dev_data *it6602data = get_it6602_dev_data();

	switch(event)
	{
		case eAVMUTE_SET_NOTIFY:
			it6602data->pCallbackFunctionsList->callback_avmute_set_notify();
	            break;

		case eAVMUTE_CLEAR_NOTIFY:
			it6602data->pCallbackFunctionsList->callback_avmute_clear_notify();
	            break;

		case eAKSV_WRITTEN_NOTIFY:
			it6602data->pCallbackFunctionsList->callback_aksv_written_notify();
	            break;

		case eAUTHENTICATION_COMPLETE_NOTIFY:
			it6602data->pCallbackFunctionsList->callback_authentication_complete_notify();
	            break;

		case eAUTHENTICATION_5_SEC_TIMEOUT_NOTIFY:
			it6602data->pCallbackFunctionsList->callback_authentication_5_sec_timeout_notify();
	            break;

	}

}




// ---------------------------------------------------------------------------
// ite6802 register callback function
// ---------------------------------------------------------------------------
void ITEHDMI_CallbackRegister( struct it6602_dev_data *it6602)
{
     if(ITEHDMICallbackList == NULL) return;

      it6602->pCallbackFunctionsList = ITEHDMICallbackList;

//      printf("\n####====>>>>>Executed func %s Successfully \n\n\n", __func__);

}


// ***************************************************************************
// HDCP Repeater function
// ***************************************************************************

void SHATransform(unsigned long * h)
{
	unsigned long    t;
	unsigned long 	tmp;

    for (t=16; t < 80; t++) {
        tmp=w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16];
        w[t]=rol(tmp,1);
    }

    h[0]=0x67452301;
    h[1]=0xefcdab89;
    h[2]=0x98badcfe;
    h[3]=0x10325476;
    h[4]=0xc3d2e1f0;

    for (t=0; t < 20; t++) {
        tmp=rol(h[0],5) + ((h[1] & h[2]) | (h[3] & ~h[1])) + h[4] + w[t] + 0x5a827999;
        h[4]=h[3];
        h[3]=h[2];
        h[2]=rol(h[1],30);
        h[1]=h[0];
        h[0]=tmp;

    }
    for (t=20; t < 40; t++) {
        tmp=rol(h[0],5) + (h[1] ^ h[2] ^ h[3]) + h[4] + w[t] + 0x6ed9eba1;
        h[4]=h[3];
        h[3]=h[2];
        h[2]=rol(h[1],30);
        h[1]=h[0];
        h[0]=tmp;
    }
    for (t=40; t < 60; t++) {
        tmp=rol(h[0],5) + ((h[1] & h[2]) | (h[1] & h[3]) | (h[2] & h[3])) + h[4] + w[t] + 0x8f1bbcdc;
        h[4]=h[3];
        h[3]=h[2];
        h[2]=rol(h[1],30);
        h[1]=h[0];
        h[0]=tmp;
    }
    for (t=60; t < 80; t++)
    {
        tmp=rol(h[0],5) + (h[1] ^ h[2] ^ h[3]) + h[4] + w[t] + 0xca62c1d6;
        h[4]=h[3];
        h[3]=h[2];
        h[2]=rol(h[1],30);
        h[1]=h[0];
        h[0]=tmp;
    }
    h[0] +=0x67452301;
    h[1] +=0xefcdab89;
    h[2] +=0x98badcfe;
    h[3] +=0x10325476;
    h[4] +=0xc3d2e1f0;

}

void SHA_Simple(void *p,unsigned int len,unsigned char *output)
{
	unsigned int 	i,t;
	unsigned long	c;
	unsigned long	sha[5];
	unsigned char 	*pBuff=p;


    for(i=0;i < len;i++)
    {
        t=i/4;
        if(i%4==0)
        {
            w[t]=0;
        }
        c=pBuff[i];
        c <<=(3-(i%4))*8;
        w[t] |=c;
    }
    t=i/4;
    if(i%4==0)
    {
        w[t]=0;
    }
    //c=0x80 << ((3-i%4)*24);
    c=0x80;
    c <<=((3-i%4)*8);
    w[t]|=c;t++;
    for(; t < 15;t++)
    {
        w[t]=0;
    }
    w[15]=len*8;

    SHATransform(sha);

    for(i=0;i < 5;i++)
    {
        output[i*4]=(BYTE)((sha[i]>>24)&0xFF);
        output[i*4+1]=(BYTE)((sha[i]>>16)&0xFF);
        output[i*4+2]=(BYTE)((sha[i]>>8)&0xFF);
        output[i*4+3]=(BYTE)(sha[i]&0xFF);
    }
}


// ---------------------------------------------------------------------------
void ITEHDMI_RxHDCPRepeaterCapabilitySet(unsigned char uc)
{
	hdmirxwr(REG_RX_P0_BCAPS,hdmirxrd(REG_RX_P0_BCAPS)|uc);
#ifdef _PRINT_HDMI_RX_HDCP_
	RXHDCP_DEBUG_PRINT(("ITEHDMI_RxHDCPRepeaterCapabilitySet=%X\n",(int) hdmirxrd(REG_RX_P0_BCAPS)));
#endif
}

// ---------------------------------------------------------------------------
void ITEHDMI_RxHDCPRepeaterCapabilityClear(unsigned char  uc)
{
	hdmirxwr(REG_RX_P0_BCAPS,hdmirxrd(REG_RX_P0_BCAPS)&(~uc));
#ifdef _PRINT_HDMI_RX_HDCP_
	RXHDCP_DEBUG_PRINT(("ITEHDMI_RxHDCPRepeaterCapabilityClear=%X\n",(int) hdmirxrd(REG_RX_P0_BCAPS)));
#endif
}

// ---------------------------------------------------------------------------
void ITEHDMI_RxKSVListMergeTxBksv(unsigned char ucTxDownStream,unsigned char *TxBksv,unsigned char *RxKSVList)
{
    unsigned int i;
#ifdef _PRINT_HDMI_RX_HDCP_
    RXHDCP_DEBUG_PRINT((" ucTxDownStream =%X ",(int) ucTxDownStream));
    RXHDCP_DEBUG_PRINT((" Merge TxBksv to RxKSV List = \r\n"));
#endif
    for(i=0;i<5;i++)
    {
        RxKSVList[ucTxDownStream*5+i]=TxBksv[i];
#ifdef _PRINT_HDMI_RX_HDCP_
	RXHDCP_DEBUG_PRINT(("RxKSVList[%X] =%X , TxBksv[%X] = %X \r\n",(int) ucTxDownStream*5+i,(int) RxKSVList[ucTxDownStream*5+i] ,(int) i, (int) TxBksv[i] ));
#endif
    }
}

// ---------------------------------------------------------------------------
void ITEHDMI_RxKSVListSet(unsigned char *pRxKSVList)
{
    unsigned char i=0;

    if(!pRxKSVList)
    {
        return	;	//ER_FAIL;
    }

	hdmirxwr(REG_RX_BLOCK_SEL, 0x01);


    for(;i<40;i++)
    {
        hdmirxwr(REG_RX_KSV_FIFO00+i,*(pRxKSVList+i));
    }

	hdmirxwr(REG_RX_BLOCK_SEL, 0x00);

    return ;	//ER_SUCCESS;
}

// ---------------------------------------------------------------------------
void ITEHDMI_RxBstatusSet(unsigned int RxBstatus)
{
    hdmirxwr(REG_RX_BLOCK_SEL,1);
    hdmirxwr(REG_RX_BSTATUSH,(unsigned char)((RxBstatus>>8) & 0x0F));
    hdmirxwr(REG_RX_BSTATUSL,(unsigned char)(RxBstatus & 0xFF));
    hdmirxwr(REG_RX_BLOCK_SEL,0);
}

// ---------------------------------------------------------------------------
void ITEHDMI_RxHDCPGenVr(unsigned char *pRx_KsvList, unsigned int Rx_Bstatus)
{
	unsigned char i,n;
	unsigned char ucRxDownStream;
	unsigned char ucM0[8];


	hdmirxwr(REG_RX_BLOCK_SEL,2);
	for(i=0; i < 8; i ++)
	{
	    ucM0[i]=hdmirxrd(REG_RX_M0_B0 + i);
	}
	hdmirxwr(REG_RX_BLOCK_SEL,0);

	ucRxDownStream = (unsigned char) (Rx_Bstatus & 0x7F);

	#ifdef _PRINT_HDMI_RX_HDCP_
		RXHDCP_DEBUG_PRINT(("ITEHDMI HDCP_GenVR()  \n"));
		RXHDCP_DEBUG_PRINT(("DS=%X \n",(int) ucRxDownStream));
		RXHDCP_DEBUG_PRINT(("Rx_Bstatus=%X \n",(int) Rx_Bstatus));

		RXHDCP_DEBUG_PRINT(("pRx_KsvList[]=\n"));
		for(n=0;n<ucRxDownStream;n++)
		{
		    for( i=0; i < 5; i++ )
		    {
		        RXHDCP_DEBUG_PRINT(("%X ",(int) pRx_KsvList[n*5+i]));
		    }
		    RXHDCP_DEBUG_PRINT(("\n"));
		}
	#endif

	#ifdef _PRINT_HDMI_RX_HDCP_
		RXHDCP_DEBUG_PRINT(("Rx HDCP ucM0[]\n "));
		for(i=0;i<8;i++)
		{
		    RXHDCP_DEBUG_PRINT(("%X ",(int) ucM0[i]));
		}
	#endif


	for(i=0; i < ucRxDownStream * 5; i ++)
	{
	    SHABuff[i]=pRx_KsvList[i];
	}

	SHABuff[i ++]=Rx_Bstatus & 0xFF;
	SHABuff[i ++]=(Rx_Bstatus>>8) & 0xFF;

	for(n=0; n < 8; n++,i++)
	{
	    SHABuff[i]=ucM0[n];
	}

	n=i;
    //    SHABuff[i++]=0x80;// end mask

	for(; i < 64; i ++)
	{
	    SHABuff[i]=0;
	}

    #ifdef _PRINT_HDMI_RX_HDCP_
        for( i=0; i < 64; i++ )
        {

            if( (i % 5 )==0 ) RXHDCP_DEBUG_PRINT(("\nSHABuff[%2X] ",(int) i ));
            RXHDCP_DEBUG_PRINT(("%02X ",(int) SHABuff[i]));
        }
        RXHDCP_DEBUG_PRINT(("\r\n"));
    #endif

    SHA_Simple(SHABuff,n,Vr);

    //    RXHDCP_DEBUG_PRINT(("n=%2X\n",n));

}

// ---------------------------------------------------------------------------
void ITEHDMI_RxHDCPVrSet(unsigned char *pVr)
{
    BYTE i,j,k;

    if(!pVr)
    {
        return ;	//ER_FAIL;
    }

    hdmirxwr(REG_RX_BLOCK_SEL,1);

    i=0;
    for(j=0; j < 5; j ++)
    {
        for(k=4;k>0;k--)
        {
            hdmirxwr(REG_RX_SHA1_H00+i,*(pVr+(j*4)+(k-1)));
            #ifdef _PRINT_HDMI_RX_HDCP_
            if(!(i%5))RXHDCP_DEBUG_PRINT(("\r\n"));
            RXHDCP_DEBUG_PRINT(("SHA1[%02X]=%02X,",(int) i,(int) *(pVr+(j*4)+(k-1))));
            #endif
            i++;
        }
    }


#ifdef _PRINT_HDMI_RX_HDCP_
	RXHDCP_DEBUG_PRINT(("\r\n"));
#endif

    hdmirxwr(REG_RX_BLOCK_SEL,0);


    return ;	//ER_SUCCESS;
}


// ---------------------------------------------------------------------------
void ITEHDMI_RxHDCPmodeChangeRequest(unsigned char bHDCPmode)
{
// bHDCPmode = FALSE for set ITEHDMI to HDCP receiver mode
// bHDCPmode = TRUE for set ITEHDMI to HDCP Repeater mode

	// 	const unsigned char aeHDCPMode[] =
	// 	{
	// 	0,	//default , HDCP receiver mode
	// 	1,	//HDCP Repeater mode
	// 	};

 	// Force HPD off
	it6602HPDCtrl(0,0);	// MHL port , set HPD = 1

	// Disable HDMI DDC Bus to access ITEHDMI EDID RAM
	hdmirxset(REG_RX_0C0, 0x01, 0x01);                            // HDMI RegC0[1:0]=11 for disable HDMI DDC bus to access EDID RAM

	IT_Delay(100);
//bug !!!	hdmirxwr(REG_RX_P0_BCAPS,0x00);		// Clear Rx Bcaps
	hdmirxwr(REG_RX_P0_BCAPS,B_BCAPS_7B);	// set Bcaps bit 7 HDMI_RESERVED = 1

	if(bHDCPmode == 0 )
	{
		m_bHDCPrepeater=FALSE;
		m_RxHDCPstatus = 0;
		ITEHDMI_RxHDCPRepeaterCapabilityClear(B_ENABLE_REPEATER);
	}
	else
	{
		m_bHDCPrepeater=TRUE;
		m_RxHDCPstatus = 1;
		ITEHDMI_RxHDCPRepeaterCapabilitySet(B_ENABLE_REPEATER);
	}


	// Update EDID Table to ITEHDMI EDID RAM
	EDIDRAMInitial(&Default_Edid_Block[0]);



	// Enable HDMI DDC bus to access ITEHDMI EDID RAM
	hdmirxset(REG_RX_0C0, 0x01, 0x00);                        // HDMI RegC0[1:0]=00 for enable HDMI DDC bus to access EDID RAM

 	// Force HPD on
	it6602HPDCtrl(0,1);	// MHL port , set HPD = 1
}


void ITEHDMI_RxHDCP2ndAuthenticationRequest(unsigned char *pKsvList, unsigned char *pTxBksv, unsigned int Tx_Bstatus)
{
// pKsvList : HDMI Tx need to read from DDC bus that access I2C address =0x74 (HDCP) offset=0x43(KSV FIFO) , size = DEVICE_COUNT *5
// pTxBksv : HDMI Tx need to read from DDC bus that access I2C address =0x74 (HDCP) offset=0x00(Bksv) , size =5
// Tx_Bstatus: HDMI Tx need to read from DDC bus that access I2C address =0x74 (HDCP) offset=0x41(Bstatus) , size =2

	unsigned char ucTxDevices_Count=0;
	unsigned char ucDepth=0;
	unsigned char bMax_Devs_Exceeded=0;
	unsigned char bMax_Cascade_Exceeded=0;

	unsigned int Rx_Bstatus=0;
	unsigned char i,j;


	RXHDCP_DEBUG_PRINT(("ITEHDMI_RxHDCP2ndAuthenticationRequest\n"));
	RXHDCP_DEBUG_PRINT((" Tx_Bstatus=%X \n",Tx_Bstatus));


	ucTxDevices_Count = (unsigned char) Tx_Bstatus & 0x7F;

	if(ucTxDevices_Count == 0)
	{
	    Tx_Bstatus=0;        // down stream is receiver
	}

	ucDepth = (unsigned char) ((Tx_Bstatus & 0x700)>>8);
	bMax_Devs_Exceeded = (unsigned char) ((Tx_Bstatus & 0x80)>>7);
	bMax_Cascade_Exceeded = (unsigned char) ((Tx_Bstatus & 0x800)>>11);


	for(j=0;j<ucTxDevices_Count;j++)
	{
		RXHDCP_DEBUG_PRINT(("Tx KSV List[%X]=",(int) j));
		for( i=0; i < 5; i++ )
		{
			RXHDCP_DEBUG_PRINT(("%X ",(int) pKsvList[j*5+i]));
		}
		RXHDCP_DEBUG_PRINT(("\n"));
	}

	//Increase devices count and depth for Rx Bstatus
	Rx_Bstatus |= (ucTxDevices_Count+1);
	Rx_Bstatus |= (((ucDepth&0x07)+1)<<8);


	if(bMax_Devs_Exceeded != 0)
	{
		Rx_Bstatus |= B_DOWNSTREAM_OVER;
	}

	if(bMax_Cascade_Exceeded != 0)
	{
		Rx_Bstatus |= B_MAX_CASCADE_EXCEEDED;
	}

	//Verfiy Rx Bstatus Downstream with ITEHDMI MAX KSV
	if((Rx_Bstatus & 0x7F) >  HDMIRX_MAX_KSV)
	{
		Rx_Bstatus|=B_DOWNSTREAM_OVER;
		RXHDCP_DEBUG_PRINT((" RX_Bstatus Downstream over %X \n",HDMIRX_MAX_KSV));
	}

	Rx_Bstatus |= IsHDMIMode() ? B_CAP_HDMI_MODE:0;

	RXHDCP_DEBUG_PRINT((" RX_Bstatus=%X \n",Rx_Bstatus));

	ITEHDMI_RxKSVListMergeTxBksv(ucTxDevices_Count,pTxBksv,pKsvList);
	ITEHDMI_RxKSVListSet(pKsvList);
	ITEHDMI_RxBstatusSet(Rx_Bstatus);
	ITEHDMI_RxHDCPGenVr(pKsvList,Rx_Bstatus);
	ITEHDMI_RxHDCPVrSet(Vr);
	ITEHDMI_RxHDCPRepeaterCapabilitySet(B_KSV_READY);

	m_RxHDCPstatus = 3;
}
//FIX_ID_032 xxxxx
#endif

#ifdef _ITEHDMI_
// ***************************************************************************
// Audio function
// ***************************************************************************
static void aud_fiforst( void )
{
    unsigned char uc ;
//FIX_ID_033 xxxxx for fix audio noise issue

#ifndef _FIX_ID_028_
//FIX_ID_025 xxxxx		//Adjust H/W Mute time
	hdmirxset(REG_RX_074,0x0c,0x0c);	// enable Mute i2s and ws	and s/pdif
	//IT_Delay(100);
	hdmirxset(REG_RX_074,0x0c,0x00);	// disable Mute i2s and ws	and s/pdif
//FIX_ID_025 xxxxx
#endif

//xxxxx 2014-0421
//FIX_ID_033 xxxxx

	hdmirxset(REG_RX_010, 0x02, 0x02);
	hdmirxset(REG_RX_010, 0x02, 0x00);

	uc = hdmirxrd(REG_RX_07B) ;
	hdmirxwr(REG_RX_07B,uc) ;
	hdmirxwr(REG_RX_07B,uc) ;
	hdmirxwr(REG_RX_07B,uc) ;
	hdmirxwr(REG_RX_07B,uc) ; // HOPE said, after FIFO reset, four valid update will active the AUDIO FIFO.
}


#ifdef _FIX_ID_028_
// ---------------------------------------------------------------------------
//FIX_ID_028 xxxxx //For Debug Audio error with S2
static void IT6602AudioOutputEnable(unsigned char bEnable)
{
	struct it6602_dev_data *it6602data = get_it6602_dev_data();

	m_bAudioWaiting=FALSE;


	hdmirxset(REG_RX_HWMuteCtrl,(B_HWMuteClr),(B_HWMuteClr));
	hdmirxset(REG_RX_HWMuteCtrl,(B_HWAudMuteClrMode),(B_HWAudMuteClrMode));
	hdmirxset(REG_RX_HWMuteCtrl,(B_HWMuteClr),(0));
	hdmirxset(REG_RX_HWMuteCtrl,(B_HWAudMuteClrMode),(0));
	aud_fiforst();

	if(bEnable==TRUE)
	{
		hdmirxset(REG_RX_052,(B_TriI2SIO|B_TriSPDIF),0x00);
		it6602data->m_AState  = ASTATE_AudioOn;


		DLOG_INFO(" === IT6602AudioOutputEnable 11111111111 ==== \r\n");
	}
	else
	{
		hdmirxset(REG_RX_052,(B_TriI2SIO|B_TriSPDIF),(B_TriI2SIO|B_TriSPDIF));
		it6602data->m_AState  = ASTATE_AudioOff;

		DLOG_INFO(" === IT6602AudioOutputEnable 00000000000 ==== \r\n");
	}
}
//FIX_ID_028 xxxxx

#else

// ---------------------------------------------------------------------------
static void IT6602AudioOutputEnable(unsigned char bEnable)
{
	if(bEnable==TRUE)
	{
		hdmirxset(REG_RX_052,(B_TriI2SIO|B_TriSPDIF),0x00);
	}
	else
	{
#ifdef EnableCalFs
//FIX_ID_023 xxxxx		//Fixed for Audio Channel Status Error with invalid HDMI source
		m_u16TMDSCLK = 0;
		m_AudioChannelStatusErrorCount = 0;
		hdmirxset(REG_RX_074,0x40,0x00);	// reg74[6]=0 disable Force FS mode
//FIX_ID_023 xxxxx
#endif

		hdmirxset(REG_RX_052,(B_TriI2SIO|B_TriSPDIF),(B_TriI2SIO|B_TriSPDIF));
	}
}
#endif
// ---------------------------------------------------------------------------

static void hdmirx_ResetAudio(void)
{
    unsigned char uc ;
	hdmirxset(REG_RX_RST_CTRL,B_AUDRST,B_AUDRST);
	hdmirxset(REG_RX_RST_CTRL,B_AUDRST,0x00);

	uc = hdmirxrd(REG_RX_07B) ;
	hdmirxwr(REG_RX_07B,uc) ;
	hdmirxwr(REG_RX_07B,uc) ;
	hdmirxwr(REG_RX_07B,uc) ;
	hdmirxwr(REG_RX_07B,uc) ; // HOPE said, after FIFO reset, four valid update will active the AUDIO FIFO.

}

// ---------------------------------------------------------------------------
static void hdmirx_SetHWMuteClrMode(void)
{
    hdmirxset(REG_RX_HWMuteCtrl,(B_HWAudMuteClrMode),(B_HWAudMuteClrMode));
}
// ---------------------------------------------------------------------------
static void hdmirx_SetHWMuteClr(void)
{
    hdmirxset(REG_RX_HWMuteCtrl,(B_HWMuteClr),(B_HWMuteClr));
}
// ---------------------------------------------------------------------------
static void hdmirx_ClearHWMuteClr(void)
{
    hdmirxset(REG_RX_HWMuteCtrl,(B_HWMuteClr),0);
}



// ---------------------------------------------------------------------------
static void getHDMIRXInputAudio(AUDIO_CAPS *pAudioCaps)
{

    unsigned char uc;

    uc = hdmirxrd(REG_RX_0AE);	// REG_RX_AUD_CHSTAT3
    pAudioCaps->SampleFreq = uc & M_FS;

    uc = hdmirxrd(REG_RX_0AA);	//REG_RX_AUDIO_CH_STAT
    pAudioCaps->AudioFlag = uc & 0xF0;
    pAudioCaps->AudSrcEnable=uc&M_AUDIO_CH;
    pAudioCaps->AudSrcEnable|=hdmirxrd(REG_RX_0AA)&M_AUDIO_CH;

    if( (uc & (B_HBRAUDIO|B_DSDAUDIO)) == 0)
    {
        uc = hdmirxrd(REG_RX_0AB);	//REG_RX_AUD_CHSTAT0

        if( (uc & B_NLPCM ) == 0 )
        {
            pAudioCaps->AudioFlag |= B_CAP_LPCM;
        }
    }

#ifdef EnableCalFs
//FIX_ID_023 xxxxx		//Fixed for Audio Channel Status Error with invalid HDMI source
	if( hdmirxrd(REG_RX_074) & 0x40)
	{
		AudioFsCal();
	}
//FIX_ID_023 xxxxx
#endif
}


#ifdef EnableCalFs
//FIX_ID_023 xxxxx		//Fixed for Audio Channel Status Error with invalid HDMI source
// ---------------------------------------------------------------------------
void DumpNCTSReg(void)
{
#if 0
    ushort	i,j ;
    BYTE ucData ;

    DLOG_INFO("\r\n       "));
    for(j = 0 ; j < 16 ; j++)
    {
        DLOG_INFO(" %02X",(int) j));
        if((j == 3)||(j==7)||(j==11))
        {
                DLOG_INFO(" :"));
        }
    }
   DLOG_INFO("\r\n"));

    chgbank(2);

    for(i = 0xB0 ; i < 0xD0 ; i+=16)
    {
        DLOG_INFO("[%03X]  ",i));
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = hdmirxrd((BYTE)((i+j)&0xFF)) ;
            DLOG_INFO(" %02X",(int) ucData));
            if((j == 3)||(j==7)||(j==11))
            {
                DLOG_INFO(" :"));
            }
        }
        DLOG_INFO("\r\n"));

    }

   DLOG_INFO("\n        =====================================================\r\n"));

    chgbank(0);
#endif
}

// ---------------------------------------------------------------------------
static unsigned int PCLKGet(void)
{
	unsigned char rddata;
	unsigned int PCLK;	//, sump;


	rddata = hdmirxrd(0x9A);
	PCLK = (m_ROSCCLK * 255/rddata)/1000;
	//DLOG_INFO("PCLKGet( ) PCLK = %ld \r\n",(int) PCLK);

	return PCLK;

}

// ---------------------------------------------------------------------------
static void TMDSGet(void)
{
	unsigned char ucCurrentPort ;
	unsigned int ucTMDSClk=0 ;
	unsigned char rddata ;
	unsigned char ucClk ;


	ucCurrentPort = hdmirxrd(REG_RX_051) & B_PORT_SEL;

	if(ucCurrentPort == F_PORT_SEL_1)
	{
		ucClk = hdmirxrd(REG_RX_092) ;
		rddata = hdmirxrd(0x90);

		if(ucClk != 0)
		{

			if( rddata&0x04 )
				ucTMDSClk=2*RCLKVALUE*256/ucClk;
			else if( rddata&0x08 )
				ucTMDSClk=4*RCLKVALUE*256/ucClk;
			else
				ucTMDSClk=RCLKVALUE*256/ucClk;


			DLOG_INFO(" TMDSGet() Port 1 TMDS org  = %d \r\n",(int) m_u16TMDSCLK);
		}
	}
	else
	{

		if(hdmirxrd(REG_RX_P0_SYS_STATUS) & (B_P0_MHL_MODE))	// judge MHL mode
		{
			ucTMDSClk = PCLKGet();
			//DLOG_INFO("MHL use Pclk to calculate FS %d \r\n",(int) ucTMDSClk);
		}
		else		//else HDMI mode
		{
		ucClk = hdmirxrd(REG_RX_091) ;
		rddata = hdmirxrd(0x90);

		if(ucClk != 0)
		{
			if( rddata&0x01 )
				ucTMDSClk=2*RCLKVALUE*256/ucClk;
				else if( rddata&0x02 )
					ucTMDSClk=4*RCLKVALUE*256/ucClk;
				else
					ucTMDSClk=RCLKVALUE*256/ucClk;

				//DLOG_INFO("HDMI use TMDS to calculate FS %d \r\n",(int) ucTMDSClk);
			}
		}

		DLOG_INFO(" TMDSGet() Port 0 TMDS org  = %d \r\n",(int) m_u16TMDSCLK);
	}

	if(m_u16TMDSCLK==0)
	{
	m_u16TMDSCLK = ucTMDSClk;
	}
	else
	{
	m_u16TMDSCLK = (( ucTMDSClk+m_u16TMDSCLK ));
	m_u16TMDSCLK /=2;
	}
}


// ---------------------------------------------------------------------------
static void AudioFsCal(void)
{

	unsigned long u32_N;
	unsigned long u32_CTS;
	unsigned char u8_FS;
	unsigned char uc;
	unsigned long sum=0;

	hdmirxset(REG_RX_077,0x01,0x01);
	chgbank(2);
	u32_N = (unsigned long)(hdmirxrd(REG_RX_2C0) & 0x0F);
	u32_N += (unsigned long)hdmirxrd(REG_RX_2BF)<<4;
	u32_N += (unsigned long)hdmirxrd(REG_RX_2BE)<<12;


	u32_CTS = (unsigned long)((hdmirxrd(REG_RX_2C0) & 0xF0)>>4);
	u32_CTS += (unsigned long)hdmirxrd(REG_RX_2C2)<<4;
	u32_CTS += (unsigned long)hdmirxrd(REG_RX_2C1)<<12;
	chgbank(0);


	DumpNCTSReg();
	DLOG_INFO(" u32_N %ld \r\n",(unsigned long) u32_N);
	DLOG_INFO(" u32_CTS %ld \r\n",(unsigned long) u32_CTS);

	if((u32_N == 0) || (u32_CTS == 0))
		return;

	TMDSGet();

//	u8_FS=(unsigned char ) (((m_u16TMDSCLK*1000)*u32_N)/(128*u32_CTS));
	sum = (u32_N*1000*m_u16TMDSCLK);
	u8_FS=(unsigned char ) (sum/(128*u32_CTS));

#if 0
	u8_FS = m_u16TMDSCLK;
	DLOG_INFO("m_u16TMDSCLK %d \r\n",(int) u8_FS);
	u8_FS = (((u32_N*m_u16TMDSCLK*78)/u32_CTS)/10);
#endif
	DLOG_INFO("u8_FS %d \r\n",(int) u8_FS);

	//Judge FS by FS calulate
	if(u8_FS>25 && u8_FS<=38)
	{
		// FS=32k , Calu Value = 29k~36k
		m_ForceFsValue = (B_32K);
	}
	else if(u8_FS>38 && u8_FS<=44)
	{
		// FS=44k , Calu Value = 41k~46k
		m_ForceFsValue = (B_44P1K);
	}
	else if(u8_FS>44 && u8_FS<=58)
	{
		// FS=48k , Calu Value = 47k~51k
		m_ForceFsValue = (B_48K);
	}
	else if(u8_FS>78 && u8_FS<=92)
	{
		// FS=88k , Calu Value = 85k~91k
		m_ForceFsValue = (B_88P2K);
	}
	else if(u8_FS>92 && u8_FS<=106)
	{
		// FS=96k , Calu Value = 93k~99k
		m_ForceFsValue = (B_96K);
	}
	else if(u8_FS>166 && u8_FS<=182)
	{
		// FS=176k , Calu Value = 173k~179k
		m_ForceFsValue = (B_176P4K);
	}
	else if(u8_FS>182 && u8_FS<=202)
	{
		// FS=192k , Calu Value = 188k~195k
		m_ForceFsValue = (B_192K);
	}


	uc = hdmirxrd(REG_RX_0AE);	// REG_RX_AUD_CHSTAT3
	DLOG_INFO("REG_RX_0AE %x ,",(int) ( uc & M_FS) );
	DLOG_INFO("m_ForceFsValue %x \r\n",(int) ( m_ForceFsValue) );
	if(( uc & M_FS) == ( m_ForceFsValue ))
	{
		m_AudioChannelStatusErrorCount=0;
		// no need to enable Force FS mode
		DLOG_INFO("CHS_FS %x , !!!No need !!! to enable Force FS mode \r\n",(int) ( uc & M_FS) );
		hdmirxset(REG_RX_074,0x40,0x00);	// reg74[6]=0 disable Force FS mode
		return;
	}

	if(++m_AudioChannelStatusErrorCount>MAX_AUDIO_CHANNEL_STATUS_ERROR)
	{
	m_AudioChannelStatusErrorCount=0;
	// a. if find Audio Error in a period timers,assue the FS message is wrong,then try to force FS setting.
	// b. set Reg0x74[6]=1=> select Force FS mode.
	hdmirxset(REG_RX_074,0x40,0x40);	// reg74[6]=1
	// f. set Reg0x7e[3:0]=0 (at leasst three times)=> force FS value
	// g. if Audio still Error,then repeat b~f setps.(on f setp,set another FS value
	// 0:44,1K,2: 48K,3:32K,8:88.2K,A:96K,C:176.4K,E:192K)
	hdmirxwr(REG_RX_07B,m_ForceFsValue);
	hdmirxwr(REG_RX_07B,m_ForceFsValue);
	hdmirxwr(REG_RX_07B,m_ForceFsValue);
	hdmirxwr(REG_RX_07B,m_ForceFsValue);
	DLOG_INFO("CHS_FS %x , !!! Enable Force FS mode !!!\r\n",(int) ( uc & M_FS) );
	}

}
//FIX_ID_023 xxxxx
#endif


// ---------------------------------------------------------------------------

static void IT6602SwitchAudioState(struct it6602_dev_data *it6602,Audio_State_Type state)
{
//	unsigned char uc;

	if( it6602->m_AState == state )
	{
	    return ;
	}

	
	DLOG_INFO("+++ %s\n",AStateStr[(unsigned char)state]);

	it6602->m_AState=state;
	//AssignAudioVirtualTime();

	switch(it6602->m_AState)
	{
	case ASTATE_AudioOff:
		hdmirxset(REG_RX_RST_CTRL, B_AUDRST, B_AUDRST);
		IT6602AudioOutputEnable(OFF);

		break;
	case ASTATE_RequestAudio:
		IT6602AudioOutputEnable(OFF);

	    break;

	case ASTATE_WaitForReady:
		hdmirx_SetHWMuteClr();
		hdmirx_ClearHWMuteClr();
		it6602->m_AudioCountingTimer = AUDIO_READY_TIMEOUT;
	    break;

	case ASTATE_AudioOn:
#ifdef EnableCalFs
//FIX_ID_023 xxxxx		//Fixed for Audio Channel Status Error with invalid HDMI source
		// AudioFsCal();
//FIX_ID_023 xxxxx
#endif
		IT6602AudioOutputEnable(ON);

		DLOG_INFO("Cat6023 Audio--> Audio flag=%02X,Ch No=%02X,Fs=%02X ... \n",
                        		(int)it6602->m_RxAudioCaps.AudioFlag,
                        		(int)it6602->m_RxAudioCaps.AudSrcEnable,
                        		(int)it6602->m_RxAudioCaps.SampleFreq);
    	break;
	}
}


#ifdef _FIX_ID_028_
//FIX_ID_028 xxxxx //For Debug Audio error with S2
//remove --> static void IT6602AudioHandler(struct it6602_dev_data *it6602)
//remove --> {
//remove --> }
//FIX_ID_028 xxxxx
#else
static void IT6602AudioHandler(struct it6602_dev_data *it6602)
{
//    unsigned char uc;

   if(it6602->m_AudioCountingTimer > MS_LOOP)
   {
   it6602->m_AudioCountingTimer -= MS_LOOP;
   }
   else
   {
   it6602->m_AudioCountingTimer = 0;
   }


   if(it6602->m_RxHDCPState==RxHDCP_ModeCheck)
   	return;

    switch(it6602->m_AState)
    {
    case ASTATE_RequestAudio:

        getHDMIRXInputAudio(&(it6602->m_RxAudioCaps));

        if(it6602->m_RxAudioCaps.AudioFlag & B_CAP_AUDIO_ON)
        {

		hdmirxset(REG_RX_MCLK_CTRL,M_MCLKSel,B_256FS);

            if(it6602->m_RxAudioCaps.AudioFlag& B_CAP_HBR_AUDIO)
            {
		DLOG_INFO("+++++++++++ B_CAP_HBR_AUDIO +++++++++++++++++\n");

		hdmirxset(REG_RX_MCLK_CTRL,M_MCLKSel,B_128FS);	// MCLK = 128fs only for HBR audio

		hdmirx_SetHWMuteClrMode();
		hdmirx_ResetAudio();
            }
            else if(it6602->m_RxAudioCaps.AudioFlag& B_CAP_DSD_AUDIO )
            {

                hdmirx_SetHWMuteClrMode();
                hdmirx_ResetAudio();
            }
            else
            {

		hdmirxset(REG_RX_HWMuteCtrl,B_HWMuteClr,0x00);
		hdmirx_SetHWMuteClrMode();
		hdmirx_ResetAudio();

            }

            IT6602SwitchAudioState(it6602,ASTATE_WaitForReady);

        }
        break;

    case ASTATE_WaitForReady:

	//if(AudioTimeOutCheck(AUDIO_READY_TIMEOUT))
#ifdef EnableCalFs
//FIX_ID_023 xxxxx		//Fixed for Audio Channel Status Error with invalid HDMI source
	TMDSGet();
//FIX_ID_023 xxxxx
#endif
	if(it6602->m_AudioCountingTimer==0)
        {
            IT6602SwitchAudioState(it6602,ASTATE_AudioOn);
        }
        break;

    case ASTATE_AudioOn:
	//if(AudioTimeOutCheck(AUDIO_MONITOR_TIMEOUT)==TRUE)
	if(it6602->m_AudioCountingTimer==0)
	{
		AUDIO_CAPS CurAudioCaps;
		//it6602->m_AudioCountingTimer = GetCurrentVirtualTime();
		//AssignAudioTimerTimeout(AUDIO_MONITOR_TIMEOUT);
		it6602->m_AudioCountingTimer = AUDIO_MONITOR_TIMEOUT;

		getHDMIRXInputAudio(&CurAudioCaps);

	        if(it6602->m_RxAudioCaps.AudioFlag != CurAudioCaps.AudioFlag
	           || it6602->m_RxAudioCaps.AudSrcEnable != CurAudioCaps.AudSrcEnable
	           || it6602->m_RxAudioCaps.SampleFreq != CurAudioCaps.SampleFreq)
	        {
			//it6602->m_ucHDMIAudioErrorCount=0;
			IT6602SwitchAudioState(it6602,ASTATE_RequestAudio);
	        }
	}
        break;
    }
}

#endif

#endif
#ifdef _ITEHDMI_
// ***************************************************************************
// Video function
// ***************************************************************************


static void IT6602_AFE_Rst( void )
{

//FIX_ID_008 xxxxx	//Add SW reset when HDMI / MHL device un-plug  !!!

	unsigned char Reg51h;

	struct it6602_dev_data *it6602data = get_it6602_dev_data();		//2013-0814

	chgbank(0);
	Reg51h = hdmirxrd(0x51);
	if( Reg51h&0x01 )
	{
		DLOG_INFO("=== port 1 IT6602_AFE_Rst() === \r\n");
		hdmirxset(REG_RX_018, 0x01, 0x01);
		IT_Delay(1);
		hdmirxset(REG_RX_018, 0x01, 0x00);
		#ifdef _SUPPORT_AUTO_EQ_
		DisableOverWriteRS(1);	//2013-1129
		#endif
	}
	else
	{
		DLOG_INFO("=== port 0 IT6602_AFE_Rst() === \r\n");
		hdmirxset(REG_RX_011, 0x01, 0x01);
		IT_Delay(1);
		hdmirxset(REG_RX_011, 0x01, 0x00);
		#ifdef _SUPPORT_AUTO_EQ_
		DisableOverWriteRS(0);	//2013-1129 for MHL unplug detected
		#endif


		// move to DisableOverWriteRS( )
		////FIX_ID_019	xxxxx modify ENHYS control for MHL mode
		//chgbank(1);
		//hdmirxset(REG_RX_1B8,0x80,0x00);	// [7] Reg_HWENHYS = 0
		//hdmirxset(REG_RX_1B6,0x07,0x03);	// [2:0]Reg_P0_ENHYS = 03
		//chgbank(0);
		////FIX_ID_019	xxxxx

	}

	it6602data->m_ucSCDTOffCount=0;	//2013-0814

//FIX_ID_008 xxxxx
}



//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
//xxxxx 2014-0529 //HDCP Content On/Off
static void IT6602_ManualVideoTristate(unsigned char bOff)
{
    if(bOff)
    {
	hdmirxset(REG_RX_053,(B_VDGatting|B_VIOSel),(B_VDGatting|B_VIOSel));	//Reg53[7][5] = 11    // for enable B_VDIO_GATTING and VIO_SEL
	hdmirxset(REG_RX_052,(B_DisVAutoMute),(B_DisVAutoMute));				//Reg52[5] = 1 for disable Auto video MUTE
	hdmirxset(REG_RX_053,(B_TriVDIO),(0x00));								//Reg53[2:0] = 000;         // 0 for enable video io data output
	DLOG_INFO("+++++++++++ Manual Video / Audio off  +++++++++++++++++\n");
    }
    else
    {
	hdmirxset(REG_RX_053,(B_TriSYNC),(0x00));								//Reg53[0] = 0;                 // for enable video sync
	hdmirxset(REG_RX_053,(B_TriVDIO),(0x00));								//Reg53[3:1] = 000;         // 0 for enable video io data output
	hdmirxset(REG_RX_053,(B_TriVDIO),(B_TriVDIO));							//Reg53[2:0] = 111;         // 1 for enable tri-state of video io data
	hdmirxset(REG_RX_053,(B_TriVDIO),(0x00));								//Reg53[2:0] = 000;         // 0 for enable video io data output
	hdmirxset(REG_RX_053,(B_VDGatting|B_VIOSel),(B_VDGatting|B_VIOSel));	//Reg53[7][5] = 11    // for enable B_VDIO_GATTING and VIO_SEL
	hdmirxset(REG_RX_053,(B_VDGatting|B_VIOSel),(B_VIOSel));				//Reg53[7][5] = 01    // for disable B_VDIO_GATTING
	DLOG_INFO("+++++++++++ Manual Video on  +++++++++++++++++\n");
    }
}
static void IT6602_HDCP_ContentOff(unsigned char ucPort , unsigned char bOff)
{
	struct it6602_dev_data *it6602data = get_it6602_dev_data();		//2013-0814

	if(IT6602_IsSelectedPort(ucPort)==FALSE)
	return;

	if(bOff != 0)
	{
		//******** Content Off ********//
		IT6602_ManualVideoTristate(1);
		it6602data->m_HDCP_ContentOff = 1;
		DLOG_INFO("+++++++++++ HDCP Content Off   +++++++++++++++++\n");
	}
	else
	{
		if(it6602data->m_VState == VSTATE_VideoOn)
		{
			if(it6602data->m_HDCP_ContentOff == 1 )
			{
				//******** Content On ********//
				IT6602_ManualVideoTristate(0);
				DLOG_INFO("+++++++++++ HDCP Content On   +++++++++++++++++\n");
			}
		}

		it6602data->m_HDCP_ContentOff = 0;
	}

}



static void IT6602_RAPContentOff(unsigned char bOff)
{
	struct it6602_dev_data *it6602data = get_it6602_dev_data();		//2013-0814

	if(IT6602_IsSelectedPort(0)==FALSE)
	return;

	if(bOff != 0)
	{
		//******** RAP Content Off ********//
		IT6602_ManualVideoTristate(1);
		it6602data->m_RAP_ContentOff = 1;
		DLOG_INFO("+++++++++++ RAP Content Off   +++++++++++++++++\n");

		//xxxxx 2014-0603 for RAP Content off
		IT6602AudioOutputEnable(0);
		//xxxxx 2014-0603
	}
	else
	{
	    if(it6602data->m_VState == VSTATE_VideoOn)
	    {
	    	if(it6602data->m_RAP_ContentOff == 1 )
		{
			//******** RAP Content On ********//
			IT6602_ManualVideoTristate(0);
			DLOG_INFO("+++++++++++ RAP Content On   +++++++++++++++++\n");

			#ifndef _FIX_ID_028_
			//FIX_ID_028 xxxxx //For Debug Audio error with S2
			//xxxxx 2014-0603 for RAP Content On
			IT6602SwitchAudioState(it6602data,ASTATE_RequestAudio);
			//xxxxx 2014-0603
			//FIX_ID_028 xxxxx //For Debug Audio error with S2
			#endif

		}
	    }
		it6602data->m_RAP_ContentOff = 0;
	}
}
//xxxxx 2014-0529
//FIX_ID_037 xxxxx

static void IT6602_SetVideoMute(struct it6602_dev_data *it6602,unsigned char bMute)
{

    if(bMute)
    {
	//******** AV Mute -> ON ********//
	hdmirxset(REG_RX_053,(B_VDGatting|B_VIOSel),(B_VDGatting|B_VIOSel));	//Reg53[7][5] = 11    // for enable B_VDIO_GATTING and VIO_SEL
	hdmirxset(REG_RX_052,(B_DisVAutoMute),(B_DisVAutoMute));				//Reg52[5] = 1 for disable Auto video MUTE
	hdmirxset(REG_RX_053,(B_TriVDIO),(0x00));								//Reg53[2:0] = 000;         // 0 for enable video io data output

	DLOG_INFO("+++++++++++ IT6602_SetVideoMute -> On +++++++++++++++++\n");
    }
    else
    {
        if(it6602->m_VState == VSTATE_VideoOn)
        {
		//******** AV Mute -> OFF ********//
		hdmirxset(REG_RX_053,(B_TriSYNC),(0x00));								//Reg53[0] = 0;                 // for enable video sync
		hdmirxset(REG_RX_053,(B_TriVDIO),(0x00));								//Reg53[3:1] = 000;         // 0 for enable video io data output

            if(CheckAVMute()==TRUE)
            {
		hdmirxset(REG_RX_052,(B_DisVAutoMute),(B_DisVAutoMute));				//Reg52[5] = 1 for disable Auto video MUTE
            }
            else
            {

		hdmirxset(REG_RX_053,(B_TriVDIO),(B_TriVDIO));							//Reg53[2:0] = 111;         // 1 for enable tri-state of video io data
		hdmirxset(REG_RX_053,(B_TriVDIO),(0x00));								//Reg53[2:0] = 000;         // 0 for enable video io data output

		hdmirxset(REG_RX_053,(B_VDGatting|B_VIOSel),(B_VDGatting|B_VIOSel));	//Reg53[7][5] = 11    // for enable B_VDIO_GATTING and VIO_SEL
		hdmirxset(REG_RX_053,(B_VDGatting|B_VIOSel),(B_VIOSel));				//Reg53[7][5] = 01    // for disable B_VDIO_GATTING

		DLOG_INFO("+++++++++++  IT6602_SetVideoMute -> Off +++++++++++++++++\n");

//		get_vid_info();
//		show_vid_info();

            }

        }

    }

}




//static void IT6602VideoOutputCDSet(void)
//{
//
//	unsigned char GCP_CD;
//	GCP_CD = ((hdmirxrd(0x99)&0xF0)>>4);
//
//	switch( GCP_CD ) {
//	case 5 :
//	printf("\n Output ColorDepth = 30 bits per pixel\r\n");
//	hdmirxset(0x65, 0x0C, 0x04);
//	break;
//	case 6 :
//	printf( "\n Output ColorDepth = 36 bits per pixel\r\n");
//	hdmirxset(0x65, 0x0C, 0x08);
//	break;
//	default :
//	printf( "\n Output ColorDepth = 24 bits per pixel\r\n");
//	hdmirxset(0x65, 0x0C, 0x00);
//	break;
//	}
//}


static void IT6602VideoOutputEnable(unsigned char bEnableOutput)
{
//	struct it6602_dev_data *it6602data = get_it6602_dev_data();
	if(bEnableOutput)
	{
		// enable output
		hdmirxset(REG_RX_053,(B_TriSYNC|B_TriVDIO),(0x00));
		DLOG_INFO("---------------- IT6602VideoOutputEnable -> On ----------------\n");
//		IT6602VideoOutputCDSet();

		//FIX_ID_016 xxxxx Support Dual Pixel Mode for IT66023 Only
		#if defined(_IT66023_)
			IT66023JudgeOutputMode();
		#endif
		//FIX_ID_016 xxxxx

	}
	else
	{
		// disable output
		hdmirxset(REG_RX_053,(B_TriSYNC|B_TriVDIO),(B_TriSYNC|B_TriVDIO));
		DLOG_INFO("---------------- IT6602VideoOutputEnable -> Off ----------------\n");

		//FIX_ID_016 xxxxx Support Dual Pixel Mode for IT66023 Only
		#if defined(_IT66023_)
			hdmirxset(REG_RX_08C, 0x08, 0x00);		// Reg8C[3] = 0    // VDIO3en�G//  for disable QA IO
		#endif
		//FIX_ID_016 xxxxx


	}
}


static void IT6602SwitchVideoState(struct it6602_dev_data *it6602,Video_State_Type  eNewVState)
{

	if(it6602->m_VState==eNewVState)
		return;

	DLOG_INFO("+++ %s\n",VStateStr[(unsigned char)eNewVState]);


	it6602->m_VState=eNewVState;
//	it6602->m_VideoCountingTimer = GetCurrentVirtualTime(); // get current time tick, and the next tick judge in the polling handler.

	switch(it6602->m_VState)
	{

		case VSTATE_SWReset:
			{
				IT6602VideoOutputEnable(FALSE);
            //FIX_ID_039 xxxxx fix image flick when enable RGB limited / Full range convert
            #ifdef _AVOID_REDUNDANCE_CSC_
				it6602->m_Backup_OutputVideoMode = 0xFF;
				it6602->m_Backup_InputVideoMode = 0xFF;
            #endif
            //FIX_ID_039 xxxxx				IT6602_AFE_Rst();
            #ifdef Enable_IT6602_CEC
            //xxxxx FIX_ID_022		//Fixed for CEC capacitor issue
				IT6602_ResetCEC();
            //xxxxx
            #endif
			}
			break;

		case VSTATE_SyncWait: {
				// 1. SCDT off interrupt
				// 2. VideoMode_Chg interrupt
				IT6602VideoOutputEnable(FALSE);
                //FIX_ID_039 xxxxx fix image flick when enable RGB limited / Full range convert
                #ifdef _AVOID_REDUNDANCE_CSC_
                	it6602->m_Backup_OutputVideoMode = 0xFF;
                	it6602->m_Backup_InputVideoMode = 0xFF;
                #endif
                //FIX_ID_039 xxxxx
				it6602->m_NewAVIInfoFrameF=FALSE;
				it6602->m_ucDeskew_P0=0;
				it6602->m_ucDeskew_P1=0;
				//it6602->m_ucSCDTOffCount=0;

				#ifdef Enable_Vendor_Specific_packet
				it6602->f_de3dframe_hdmi = FALSE;
				hdmirxwr(REG_RX_06A, 0x82);
				#endif

			}
			break;

		case VSTATE_SyncChecking: {
				// 1. SCDT on interrupt
				//AssignVideoVirtualTime(VSATE_CONFIRM_SCDT_COUNT);
				//AssignVideoTimerTimeout(VSATE_CONFIRM_SCDT_COUNT);

				it6602->m_VideoCountingTimer = VSATE_CONFIRM_SCDT_COUNT;

				#ifdef Enable_Vendor_Specific_packet
				hdmirxwr(REG_RX_06A, 0x82);
				#endif

			}
			break;

		case VSTATE_VideoOn: {
				IT6602VideoOutputConfigure(it6602);
				IT6602VideoOutputEnable(TRUE);
				IT6602SwitchAudioState(it6602,ASTATE_RequestAudio);

				get_vid_info();
				show_vid_info();
				dlog_output(1000);
				hdmirxwr(0x84, 0x8F);	//2011/06/17 xxxxx, for enable Rx Chip count

				#ifdef Enable_Vendor_Specific_packet
				hdmirxwr(REG_RX_06A, 0x81);
				#endif


				//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
				//	#ifdef _SUPPORT_EQ_ADJUST_
				//	HDMIStartEQDetect(&(it6602->EQPort[it6602->m_ucCurrentHDMIPort]));
				//	#endif
				//FIX_ID_001 xxxxx

				//xxxxx 2013-0812 @@@@@
					it6602->m_ucSCDTOffCount=0;
				//xxxxx 2013-0812

//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
#ifdef _SUPPORT_HDCP_REPEATER_
			#ifdef _PSEUDO_HDCP_REPEATER_TEST_
				// TX_BSTATUS = 0x102;
				if(m_RxHDCPstatus == 2)
				ITEHDMI_RxHDCP2ndAuthenticationRequest(TX_KSVList, TX_BKSV, TX_BSTATUS);
			#endif
#endif
//FIX_ID_033 xxxxx

			}
			break;
	}

}

// ---------------------------------------------------------------------------
static void IT6602VideoHandler(struct it6602_dev_data *it6602)
{
//	unsigned char uc;
	IT_Delay(500);
	if(it6602->m_VideoCountingTimer > MS_LOOP)
	{
	it6602->m_VideoCountingTimer -= MS_LOOP;
	}
	else
	{
	it6602->m_VideoCountingTimer = 0;
	}



	switch(it6602->m_VState)
	{

		case VSTATE_SyncWait: {
				//Waiting for SCDT on interrupt !!!
				//if(VideoCountingTimer==0)

				WaitingForSCDT(it6602);

#if 0
				if(TimeOutCheck(eVideoCountingTimer)==TRUE) {
					DLOG_INFO("------------SyncWait time out -----------\n");
					SWReset_HDMIRX();
					return;
				}
#endif
			}
			break;

		case VSTATE_SyncChecking:{
			        //if(VideoTimeOutCheck(VSATE_CONFIRM_SCDT_COUNT))
			        if(it6602->m_VideoCountingTimer == 0)
			        {
						IT6602SwitchVideoState(it6602,VSTATE_VideoOn);
			        }
			}
			break;

		case VSTATE_VideoOn: {
#ifdef _SUPPORT_HDCP_REPEATER_
			#ifdef _PSEUDO_HDCP_REPEATER_TEST_
				// TX_BSTATUS = 0x102;
				if(m_RxHDCPstatus == 2)
				ITEHDMI_RxHDCP2ndAuthenticationRequest(TX_KSVList, TX_BKSV, TX_BSTATUS);
			#endif
#endif
				if(it6602->m_NewAVIInfoFrameF==TRUE)
				{
					if(it6602->m_RxHDCPState != RxHDCP_ModeCheck)
					{
						IT6602VideoOutputConfigure(it6602);
						it6602->m_NewAVIInfoFrameF=FALSE;
					}

				}

				if(hdmirxrd(REG_RX_053)&B_VDGatting)
				{
					//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
					//xxxxx 2014-0529 //Manual Content On/Off
					//if(IT6602_IsSelectedPort(0)
					{
						if((it6602->m_RAP_ContentOff == 0) && (it6602->m_HDCP_ContentOff == 0))
						{
							if(CheckAVMute()==FALSE)
							{
							IT6602_SetVideoMute(it6602,OFF);
							}
						}
					}
					//xxxxx 2014-0529
					//FIX_ID_037 xxxxx
				}

#ifdef _FIX_ID_028_
//FIX_ID_028 xxxxx //For Debug Audio error with S2
				if( hdmirxrd(REG_RX_0AA) & 0x80)
				{
					//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
					if(it6602->m_RAP_ContentOff == 0) 	//xxxxx 2014-0529 //Manual Content On/Off
					{
						if(it6602->m_AState  != ASTATE_AudioOn)
						{
							it6602->m_AudioCountingTimer = AUDIO_READY_TIMEOUT;
							it6602->m_AState = ASTATE_AudioOn;
							m_bAudioWaiting = TRUE;
						}
						else
						{

						   if(it6602->m_AudioCountingTimer > MS_LOOP)
						   {
						   it6602->m_AudioCountingTimer -= MS_LOOP;
						   }
						   else
						   {
						   	it6602->m_AudioCountingTimer = 0;
						   	if(m_bAudioWaiting == TRUE)
							{
							IT6602AudioOutputEnable(TRUE);
							}
						   }

						}
					}		//xxxxx 2014-0529
					//FIX_ID_037 xxxxx
				}
				else
				{
					if(it6602->m_AState  == ASTATE_AudioOn)
					IT6602AudioOutputEnable(FALSE);
				}
//FIX_ID_028 xxxxx
#endif
//				#ifdef Enable_Vendor_Specific_packet
//					if(it6602->f_de3dframe_hdmi == FALSE)
//					{
//					it6602->f_de3dframe_hdmi = IT6602_DE3DFrame(TRUE);
//					}
//				#endif

			}
			break;
	}
}

#endif

#ifdef _ITEHDMI_
// ***************************************************************************
// Interrupt function
// ***************************************************************************
static void hdmirx_INT_5V_Pwr_Chg(struct it6602_dev_data *it6602,unsigned char ucport)
{

	unsigned char ucPortSel;
	ucPortSel = hdmirxrd(REG_RX_051)&B_PORT_SEL;

//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
	if(ucPortSel == ucport)
	{
		if(CheckPlg5VPwr(ucport)==TRUE)
		{
			DLOG_INFO("#### Power 5V ON ####\r\n");
			IT6602SwitchVideoState(it6602,VSTATE_SyncWait);
			it6602HPDCtrl(ucport,1);	// set ucport's HPD = 1
		}
		else
		{
			DLOG_INFO("#### Power 5V OFF ####\r\n");
			IT6602SwitchVideoState(it6602,VSTATE_SWReset);
			it6602HPDCtrl(ucport,0);	// clear ucport's HPD = 0
		}
	}
	else
	{
		if(CheckPlg5VPwr(ucport)==FALSE)
		{
			#ifdef _SUPPORT_AUTO_EQ_
			DisableOverWriteRS(ucport);
			#endif
			it6602HPDCtrl(ucport,0);	// clear ucport's HPD = 0
		}
		else
		{
			it6602HPDCtrl(ucport,1);	// set ucport's HPD = 1
		}
	}
//FIX_ID_037 xxxxx
}
// ---------------------------------------------------------------------------
static void hdmirx_INT_P0_ECC(struct it6602_dev_data *it6602)
{
	struct it6602_dev_data *it6602data = get_it6602_dev_data();


	if((it6602->m_ucEccCount_P0++) > ECC_TIMEOUT)
	{

		#ifdef _SUPPORT_EQ_ADJUST_
		if(it6602->EQPort[F_PORT_SEL_0].f_manualEQadjust==TRUE)	// ignore ECC interrupt when manual EQ adjust !!!
		return;
		#endif

		it6602->m_ucEccCount_P0=0;

		DLOG_INFO("CDR reset for Port0 ECC_TIMEOUT !!!\r\n");

		hdmirx_ECCTimingOut(F_PORT_SEL_0);
		// disable ->if((hdmirxrd(0x0A)&0x40))
		// disable ->//if( MHL_Mode )
		// disable ->{
		// disable ->mhlrxset(MHL_RX_28,0x40,0x40);
		// disable ->//it6602HPDCtrl(0,1);
		// disable ->IT_Delay(100);
		// disable ->//it6602HPDCtrl(0,0);
		// disable ->mhlrxset(MHL_RX_28,0x40,0x00);
		// disable ->
		// disable ->}
		// disable ->else
		// disable ->{
		// disable ->it6602HPDCtrl(0,0);	// MHL port , set HPD = 0
		// disable ->}
		// disable ->
		// disable ->hdmirxset(REG_RX_011,(B_P0_DCLKRST|B_P0_CDRRST|B_P0_SWRST),(B_P0_DCLKRST|B_P0_CDRRST|B_P0_SWRST));
		// disable ->IT_Delay(300);
		// disable ->hdmirxset(REG_RX_011,(B_P0_DCLKRST|B_P0_CDRRST|B_P0_SWRST),0x00);
		// disable ->
		// disable ->
		// disable ->
		// disable ->//set port 0 HPD=1
		// disable ->it6602HPDCtrl(0,1);	// MHL port , set HPD = 1
		//IT6602SwitchVideoState(it6602,VSTATE_SyncWait);

	}
}

// ---------------------------------------------------------------------------
static void hdmirx_INT_P1_ECC(struct it6602_dev_data *it6602)
{


	if((it6602->m_ucEccCount_P1++) > ECC_TIMEOUT)
	{
		#ifdef _SUPPORT_EQ_ADJUST_
		if(it6602->EQPort[F_PORT_SEL_1].f_manualEQadjust==TRUE)	// ignore ECC interrupt when manual EQ adjust !!!
		return;
		#endif

		it6602->m_ucEccCount_P1=0;

		DLOG_INFO("CDR reset for Port1 ECC_TIMEOUT !!!\r\n");

		hdmirx_ECCTimingOut(F_PORT_SEL_1);
		// disable ->//set port 1 HPD=0
		// disable ->//HotPlug(0);
		// disable ->//xxxxx 2013-0801
		// disable ->it6602HPDCtrl(1,0);	// HDMI port , set HPD = 0
		// disable ->//xxxxx
		// disable ->
		// disable ->hdmirxset(REG_RX_018,(B_P1_DCLKRST|B_P1_CDRRST|B_P1_SWRST),(B_P1_DCLKRST|B_P1_CDRRST|B_P1_SWRST));
		// disable ->IT_Delay(200);
		// disable ->hdmirxset(REG_RX_018,(B_P1_DCLKRST|B_P1_CDRRST|B_P1_SWRST),0x00);
		// disable ->
		// disable ->//set port 1 HPD=1
		// disable ->//HotPlug(1);
		// disable ->//xxxxx 2013-0801
		// disable ->it6602HPDCtrl(1,1);	// HDMI port , set HPD = 1
		// disable ->//xxxxx


	}
}

// ---------------------------------------------------------------------------
static void hdmirx_INT_P0_Deskew(struct it6602_dev_data *it6602)
{
	if((it6602->m_ucDeskew_P0++) > DESKEW_TIMEOUT)
	{
		#ifdef _SUPPORT_EQ_ADJUST_
		if(it6602->EQPort[F_PORT_SEL_0].f_manualEQadjust==TRUE)	// ignore ECC interrupt when manual EQ adjust !!!
		return;
		#endif
		it6602->m_ucDeskew_P0=0;

		DLOG_INFO("CDR reset for Port0 DESKEW_TIMEOUT !!!\r\n");
//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
// 07-16 disable -> 	#ifdef _SUPPORT_EQ_ADJUST_
// 07-16 disable -> 	if(it6602->f_manualEQadjust[0]==TRUE)
// 07-16 disable -> 		HDMISwitchEQstate(&it6602->EQPort[0],EQSTATE_WAIT+1); 	//for SCDT on state
// 07-16 disable -> 	#endif
//FIX_ID_001 xxxxx
//		hdmirxset(REG_RX_011,(B_P0_DCLKRST|B_P0_CDRRST),(B_P0_DCLKRST|B_P0_CDRRST));
//		hdmirxset(REG_RX_011,(B_P0_DCLKRST|B_P0_CDRRST),0x00);

		if(hdmirxrd(REG_RX_020) == 0x00)
			hdmirxwr(REG_RX_020,0x3F);	// Dr. Liu suggestion to 0x00
		else
			hdmirxwr(REG_RX_020,0x00);	// Dr. Liu suggestion to 0x3F

	}
}

// ---------------------------------------------------------------------------
static void hdmirx_INT_P1_Deskew(struct it6602_dev_data *it6602)
{
	if((it6602->m_ucDeskew_P1++) > DESKEW_TIMEOUT)
	{
		#ifdef _SUPPORT_EQ_ADJUST_
		if(it6602->EQPort[F_PORT_SEL_1].f_manualEQadjust==TRUE)	// ignore ECC interrupt when manual EQ adjust !!!
		return;
		#endif

		it6602->m_ucDeskew_P1=0;

		DLOG_INFO("CDR reset for Port1 DESKEW_TIMEOUT !!!\r\n");
//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
// 07-16 disable -> 	#ifdef _SUPPORT_EQ_ADJUST_
// 07-16 disable -> 	if(it6602->f_manualEQadjust[1]==TRUE)
// 07-16 disable -> 		HDMISwitchEQstate(&it6602->EQPort[1],EQSTATE_WAIT+1); 	//for SCDT on state
// 07-16 disable -> 	#endif
//FIX_ID_001 xxxxx
//		hdmirxset(REG_RX_018,(B_P1_DCLKRST|B_P1_CDRRST),(B_P1_DCLKRST|B_P1_CDRRST));
//		hdmirxset(REG_RX_018,(B_P1_DCLKRST|B_P1_CDRRST),0x00);



		//it6602->EQPort[F_PORT_SEL_1].f_manualEQadjust=TRUE;
		//HDMISwitchEQstate(&it6602->EQPort[1],EQSTATE_END); 	//for SCDT on state
		//HDMICheckECC(&it6602->EQPort[1]);

		if(hdmirxrd(REG_RX_038) == 0x00)
			hdmirxwr(REG_RX_038,0x3F);	// Dr. Liu suggestion to 0x00
		else
			hdmirxwr(REG_RX_038,0x00);	// Dr. Liu suggestion to 0x3F
	}
}


// ---------------------------------------------------------------------------
//FIX_ID_009 xxxxx	//verify interrupt event with reg51[0] select port
static void hdmirx_INT_HDMIMode_Chg(struct it6602_dev_data *it6602,unsigned char ucport)
{
	unsigned char ucPortSel;
	ucPortSel = hdmirxrd(REG_RX_051)&B_PORT_SEL;

	if(ucPortSel != ucport)
		return;
//FIX_ID_009 xxxxx

	if(IsHDMIMode()){
	    	if(it6602->m_VState==VSTATE_VideoOn)
	        {
			    IT6602SwitchAudioState(it6602,ASTATE_RequestAudio);
		}
		it6602->m_bUpHDMIMode = TRUE ;
		DLOG_INFO("#### HDMI/DVI Mode : HDMI ####\r\n");
	}
	else
	{
		IT6602SwitchAudioState(it6602,ASTATE_AudioOff);
		it6602->m_NewAVIInfoFrameF=FALSE;
	    	if(it6602->m_VState==VSTATE_VideoOn)
	    	{
	    		SetDVIVideoOutput(it6602);
	    	}
		it6602->m_bUpHDMIMode = FALSE ;
		DLOG_INFO("#### HDMI/DVI Mode : DVI ####\r\n");
	}
}

// ---------------------------------------------------------------------------
static void hdmirx_INT_SCDT_Chg(struct it6602_dev_data *it6602)
{
	if(CheckSCDT(it6602) == TRUE){
		DLOG_INFO("#### SCDT ON ####\r\n");
		IT6602SwitchVideoState(it6602,VSTATE_SyncChecking);
	}
	else{
		DLOG_INFO("#### SCDT OFF ####\r\n");
		IT6602SwitchVideoState(it6602,VSTATE_SyncWait);
		IT6602SwitchAudioState(it6602,ASTATE_AudioOff);
//		TMDSCheck(it6602->m_ucCurrentHDMIPort);
//		TogglePolarity (it6602->m_ucCurrentHDMIPort);


	}
}


//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
#ifdef _SUPPORT_AUTO_EQ_
static void hdmirx_INT_EQ_FAIL(struct it6602_dev_data *it6602,unsigned char ucPortSel)
{
	if(ucPortSel>F_PORT_SEL_1)
		return;

#ifdef _SUPPORT_EQ_ADJUST_
	if(it6602->EQPort[ucPortSel].f_manualEQadjust==FALSE)	// ignore EQ fail interrupt when manual EQ adjust !!!
#endif
	{
		if(CheckPlg5VPwr(ucPortSel))
		{

			//07-08
			if( ucPortSel	== 0)
			{
				if((it6602->HDMIIntEvent & (B_PORT0_TMDSEvent)))
				{
					DLOG_INFO("#### hdmirx_INT_EQ_FAIL not yet !!! ####\r\n");

//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
					if((it6602->HDMIIntEvent & (B_PORT0_Waiting))==0)
					{
						hdmirxwr(REG_RX_022, 0x00);	// power down auto EQ
						it6602->HDMIIntEvent |= (B_PORT0_Waiting);
						it6602->HDMIIntEvent |= (B_PORT0_TMDSEvent);
						it6602->HDMIWaitNo[0]=MAX_TMDS_WAITNO;
					}
					else if((it6602->HDMIIntEvent & (B_PORT0_TMDSEvent)))
					{
						it6602->HDMIIntEvent |= (B_PORT0_Waiting);
						it6602->HDMIWaitNo[0] += MAX_HDCP_WAITNO;
					}

//FIX_ID_033 xxxxx

					return;
				}

				if(hdmirxrd(REG_RX_P0_SYS_STATUS) & (B_P0_MHL_MODE))
				{
					if((ucPortAMPValid[0]&0x03)!=0x03)
						AmpValidCheck(ucPortSel);

				}
				else
				{
					if((ucPortAMPValid[ucPortSel]&0x3F)!=0x3F)
						AmpValidCheck(ucPortSel);

				}
			}
			else
			{
				if((it6602->HDMIIntEvent & (B_PORT1_TMDSEvent)))
				{
					DLOG_INFO("#### hdmirx_INT_EQ_FAIL not yet !!! ####\r\n");

//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
					if((it6602->HDMIIntEvent & (B_PORT1_Waiting))==0)
					{
						hdmirxwr(REG_RX_03A, 0x00);	// power down auto EQ
						it6602->HDMIIntEvent |= (B_PORT1_Waiting);
						it6602->HDMIIntEvent |= (B_PORT1_TMDSEvent);
						it6602->HDMIWaitNo[1]=MAX_TMDS_WAITNO;
					}
					else if((it6602->HDMIIntEvent & (B_PORT1_TMDSEvent)))
					{
						it6602->HDMIIntEvent |= (B_PORT1_Waiting);
						it6602->HDMIWaitNo[1] += MAX_HDCP_WAITNO;
					}
//FIX_ID_033 xxxxx

					return;
				}

				if((ucPortAMPValid[ucPortSel]&0x3F)!=0x3F)
					AmpValidCheck(ucPortSel);
			}

//FIX_ID_035 xxxxx //For MTK6592 HDMI to SII MHL TX compliance issue
//xxxxx 2014-0508 disable ->			ucEqRetryCnt[ucPortSel]++;
//xxxxx 2014-0508 disable ->			if( ucEqRetryCnt[ucPortSel]>=EQRETRYFAILCNT)
//FIX_ID_035 xxxxx
			{

				//07-08
				if( ucPortSel	== 0)
				{
					if(hdmirxrd(REG_RX_P0_SYS_STATUS) & (B_P0_MHL_MODE))
					{
						if((ucPortAMPValid[0]&0x03)!=0x03)
							TogglePolarity (ucPortSel);

					}
					else
					{
						if((ucPortAMPValid[ucPortSel]&0x3F)!=0x3F)
							TogglePolarity (ucPortSel);

					}
				}
				else
				{
					if((ucPortAMPValid[ucPortSel]&0x3F)!=0x3F)
						TogglePolarity (ucPortSel);
				}
//FIX_ID_035 xxxxx //For MTK6592 HDMI to SII MHL TX compliance issue
//xxxxx 2014-0508 disable ->				ucEqRetryCnt[ucPortSel]=0;
//FIX_ID_035 xxxxx
			}
		}
	}

}
#endif
//FIX_ID_001 xxxxx
#endif


#ifdef _ITEHDMI_
// disable -> /*****************************************************************************/
// disable -> /* HDCP functions    *********************************************************/
// disable -> /*****************************************************************************/
// disable ->
// disable -> static void hdcpsts( void )
// disable -> {
// disable ->
// disable -> 	int AKSV1, AKSV2, AKSV3, AKSV4, AKSV5;
// disable -> 	int BKSV1, BKSV2, BKSV3, BKSV4, BKSV5;
// disable -> 	int BMi1,BMi2, BMi3, BMi4, BMi5, BMi6, BMi7, BMi8;
// disable -> 	int Mi1, Mi2, Mi3, Mi4, Mi5, Mi6, Mi7, Mi8;
// disable ->
// disable ->     chgbank(2);
// disable ->
// disable -> 	BKSV1 = hdmirxrd(0x9F);
// disable -> 	BKSV2 = hdmirxrd(0xA0);
// disable -> 	BKSV3 = hdmirxrd(0xA1);
// disable -> 	BKSV4 = hdmirxrd(0xA2);
// disable -> 	BKSV5 = hdmirxrd(0xA3);
// disable ->
// disable -> 	AKSV1 = hdmirxrd(0x9A);
// disable -> 	AKSV2 = hdmirxrd(0x9B);
// disable -> 	AKSV3 = hdmirxrd(0x9C);
// disable -> 	AKSV4 = hdmirxrd(0x9D);
// disable -> 	AKSV5 = hdmirxrd(0x9E);
// disable ->
// disable -> 	BMi1 = hdmirxrd(0xA4);
// disable -> 	BMi2 = hdmirxrd(0xA5);
// disable -> 	BMi3 = hdmirxrd(0xA6);
// disable -> 	BMi4 = hdmirxrd(0xA7);
// disable -> 	BMi5 = hdmirxrd(0xA8);
// disable -> 	BMi6 = hdmirxrd(0xA9);
// disable -> 	BMi7 = hdmirxrd(0xAA);
// disable -> 	BMi8 = hdmirxrd(0xAB);
// disable ->
// disable -> 	Mi1 = (~BMi1)&0xFF;
// disable -> 	Mi2 = (~BMi2)&0xFF;
// disable -> 	Mi3 = (~BMi3)&0xFF;
// disable -> 	Mi4 = (~BMi4)&0xFF;
// disable -> 	Mi5 = (~BMi5)&0xFF;
// disable -> 	Mi6 = (~BMi6)&0xFF;
// disable -> 	Mi7 = (~BMi7)&0xFF;
// disable -> 	Mi8 = (~BMi8)&0xFF;
// disable ->
// disable -> 	DLOG_INFO("AKSV = 0x%X%X%X%X%X%X%X%X%X%X \n", (int) AKSV5>>4,(int)  AKSV5&0x0,
// disable -> 		                                      (int)  AKSV4>>4, (int) AKSV4&0x0F,
// disable -> 											   (int) AKSV3>>4, (int) AKSV3&0x0F,
// disable -> 											   (int) AKSV2>>4, (int) AKSV2&0x0F,
// disable -> 											   (int) AKSV1>>4, (int) AKSV1&0x0F));
// disable ->
// disable -> 	DLOG_INFO("BKSV = 0x%X%X%X%X%X%X%X%X%X%X \n", (int) BKSV5>>4,(int)  BKSV5&0x0,
// disable -> 		                                       (int) BKSV4>>4, (int) BKSV4&0x0F,
// disable -> 											   (int) BKSV3>>4, (int) BKSV3&0x0F,
// disable -> 											   (int) BKSV2>>4, (int) BKSV2&0x0F,
// disable -> 											   (int) BKSV1>>4, (int) BKSV1&0x0F));
// disable ->
// disable -> 	DLOG_INFO("M0   = 0x%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X \n", (int) Mi8>>4,(int)  Mi8&0x0,
// disable -> 		                                                  (int)  Mi7>>4,(int)  Mi7&0x0F,
// disable -> 														 (int)   Mi6>>4, (int) Mi6&0x0F,
// disable -> 														  (int)  Mi5>>4, (int) Mi5&0x0F,
// disable -> 														   (int) Mi4>>4, (int) Mi4&0x0F,
// disable -> 														   (int) Mi3>>4, (int) Mi3&0x0F,
// disable -> 														   (int) Mi2>>4, (int) Mi2&0x0F,
// disable -> 														   (int) Mi1>>4, (int) Mi1&0x0F));
// disable -> 	chgbank(0);
// disable ->
// disable -> }
#endif



//#ifdef _ITEHDMI_
//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_
/*****************************************************************************/
/* MHLRX functions    *********************************************************/
/*****************************************************************************/
#ifdef _SUPPORT_RCP_
////////////////////////////////////////////////////////////////////
//void parse_rcpkey( unsigned char rcpkey )
//
//
//
////////////////////////////////////////////////////////////////////
static void parse_rcpkey( unsigned char rcpkey )
{
     unsigned char suppkey;

     suppkey = TRUE;

     DLOG_INFO("KeyCode=%X ==>",(int)  rcpkey&0x7F);

     switch( rcpkey&0x7F ) {
     case 0x00: DLOG_INFO("Select"); break;
     case 0x01: DLOG_INFO("Up"); break;
     case 0x02: DLOG_INFO("Down"); break;
     case 0x03: DLOG_INFO("Left"); break;
     case 0x04: DLOG_INFO("Right"); break;
     case 0x05: DLOG_INFO("Right-Up"); break;
     case 0x06: DLOG_INFO("Right-Down"); break;
     case 0x07: DLOG_INFO("Left-Up"); break;
     case 0x08: DLOG_INFO("Left-Down"); break;
     case 0x09: DLOG_INFO("Root Menu"); break;
     case 0x0A: DLOG_INFO("Setup Menu"); break;
     case 0x0B: DLOG_INFO("Contents Menu"); break;
     case 0x0C: DLOG_INFO("Favorite Menu"); break;
     case 0x0D: DLOG_INFO("Exit"); break;

     case 0x20: DLOG_INFO("Numeric 0"); break;
     case 0x21: DLOG_INFO("Numeric 1"); break;
     case 0x22: DLOG_INFO("Numeric 2"); break;
     case 0x23: DLOG_INFO("Numeric 3"); break;
     case 0x24: DLOG_INFO("Numeric 4"); break;
     case 0x25: DLOG_INFO("Numeric 5"); break;
     case 0x26: DLOG_INFO("Numeric 6"); break;
     case 0x27: DLOG_INFO("Numeric 7"); break;
     case 0x28: DLOG_INFO("Numeric 8"); break;
     case 0x29: DLOG_INFO("Numeric 9"); break;
     case 0x2A: DLOG_INFO("Dot"); break;
     case 0x2B: DLOG_INFO("Enter"); break;
     case 0x2C: DLOG_INFO("Clear"); break;

     case 0x30: DLOG_INFO("Channel Up"); break;
     case 0x31: DLOG_INFO("Channel Down"); break;
     case 0x32: DLOG_INFO("Previous Channel"); break;
     case 0x33: DLOG_INFO("Sound Select"); break;
     case 0x34: DLOG_INFO("Input Select"); break;
     case 0x35: DLOG_INFO("Show Information"); break;
     case 0x36: DLOG_INFO("Help"); break;
     case 0x37: DLOG_INFO("Page Up"); break;
     case 0x38: DLOG_INFO("Page Down"); break;

     case 0x41: DLOG_INFO("Volume Up"); break;
     case 0x42: DLOG_INFO("Volume Down"); break;
     case 0x43: DLOG_INFO("Mute"); break;
     case 0x44: DLOG_INFO("Play"); break;
     case 0x45: DLOG_INFO("Stop"); break;
     case 0x46: DLOG_INFO("Pause"); break;
     case 0x47: DLOG_INFO("Record"); break;
     case 0x48: DLOG_INFO("Rewind"); break;
     case 0x49: DLOG_INFO("Fast Forward"); break;
     case 0x4A: DLOG_INFO("Eject"); break;
     case 0x4B: DLOG_INFO("Forward"); break;
     case 0x4C: DLOG_INFO("Backward"); break;

     case 0x50: DLOG_INFO("Angle"); break;
     case 0x51: DLOG_INFO("Subpicture"); break;

     case 0x60: DLOG_INFO("Play_Function"); break;
     case 0x61: DLOG_INFO("Pause_Play_Function"); break;
     case 0x62: DLOG_INFO("Record_Function"); break;
     case 0x63: DLOG_INFO("Pause_Record_Function"); break;
     case 0x64: DLOG_INFO("Stop"); break;
     case 0x65: DLOG_INFO("Mute"); break;
     case 0x66: DLOG_INFO("Restore_Volume_Function"); break;
     case 0x67: DLOG_INFO("Tune_Function"); break;
     case 0x68: DLOG_INFO("Select_Media_Function"); break;

     case 0x71: DLOG_INFO("F1 (Blue)"); break;
     case 0x72: DLOG_INFO("F2 (Red)"); break;
     case 0x73: DLOG_INFO("F3 (Green)"); break;
     case 0x74: DLOG_INFO("F4 (Yellow)"); break;
     case 0x75: DLOG_INFO("F5"); break;
     case 0x7E: DLOG_INFO("Vendor_Specific"); break;

     default  : DLOG_INFO("ERROR: Reserved RCP sub-command code !!!\n"); suppkey = FALSE;
     }

     if( suppkey ) {
         if( rcpkey&0x80 )
             DLOG_INFO(" Key Release\n");
         else
             DLOG_INFO(" Key Press\n");
     }
}

////////////////////////////////////////////////////////////////////
//static void mhl_parse_RCPkey(struct it6602_dev_data *it6602)
//
//
//
////////////////////////////////////////////////////////////////////
static void mhl_parse_RCPkey(struct it6602_dev_data *it6602)
{
	parse_rcpkey( it6602->rxmsgdata[1]);

	if( SuppRCPCode[it6602->rxmsgdata[1]] ){

		it6602->txmsgdata[0] = MSG_RCPK;
		it6602->txmsgdata[1] = it6602->rxmsgdata[1];
		DLOG_INFO("Send a RCPK with action code = 0x%02X\n", (int) it6602->txmsgdata[1]);

		//not yet !!!rcp_report_event(it6602->rxmsgdata[1]);

	}
	else {
		it6602->txmsgdata[0] = MSG_RCPE;
		it6602->txmsgdata[1] = 0x01;

		DLOG_INFO("Send a RCPE with status code = 0x%02X\n",(int)  it6602->txmsgdata[1]);
	}

	cbus_send_mscmsg(it6602);
	SwitchRCPResult(it6602,RCP_Result_Finish);

}
#endif


#ifdef _SUPPORT_RAP_
static void mhl_parse_RAPkey(struct it6602_dev_data *it6602)
{
	//parse_rapkey( it6602->rxmsgdata[1]);

	it6602->txmsgdata[0] = MSG_RAPK;

	if( SuppRAPCode[it6602->rxmsgdata[1]]) {
		it6602->txmsgdata[1] = 0x00;
	}
	else{
		it6602->txmsgdata[1] = 0x02;
	}

	switch( it6602->rxmsgdata[1] ) {
		case 0x00:
			DLOG_INFO("Poll\n");
			break;
		case 0x10:
			DLOG_INFO("Change to CONTENT_ON state\n");
			//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
			//xxxxx 2014-0529 //Manual Content On/Off
			IT6602_RAPContentOff(0);	//(1);	//xxxxx 2014-0520 For Turn on Video output
			//xxxxx 2014-0529
			//FIX_ID_037 xxxxx
			break;
		case 0x11:
			DLOG_INFO("Change to CONTENT_OFF state\n");
			//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
			//xxxxx 2014-0529 //Manual Content On/Off
			IT6602_RAPContentOff(1);	//xxxxx 2014-0520 For Turn off Video output
			//xxxxx 2014-0529
			//FIX_ID_037 xxxxx
			break;
		default  :

		it6602->txmsgdata[1] = 0x01;
		DLOG_INFO("ERROR: Unknown RAP action code 0x%02X !!!\n",(int)  it6602->rxmsgdata[1]);
		DLOG_INFO("Send a RAPK with status code = 0x%02X\n", (int) it6602->txmsgdata[1]);
	 }

         if( it6602->rxmsgdata[1]==0x00 || it6602->rxmsgdata[1]==0x10 || it6602->rxmsgdata[1]==0x11 ) {
             it6602->txmsgdata[0] = MSG_RAPK;
//             if( RAPBusyCnt!=0 )
//                 txmsgdata[1] = 0x03;
//         else
		{
                 if( SuppRAPCode[it6602->rxmsgdata[1]] ) {
                     it6602->txmsgdata[1] = 0x00;
                     //RAPBusyCnt = RAPBUSYNUM;
                 }
                 else
                     it6602->txmsgdata[1] = 0x02;
             }
//             printf("Send a RAPK with status code = 0x%02X (RxSeqNum=%X) \n", txmsgdata[1], RxRapMsgSeq++);
//             send_mscmsg();
         }
         else {
             it6602->txmsgdata[0] = MSG_RAPK;
             it6602->txmsgdata[1] = 0x01;
//             printf("Send a RAPK with status code = 0x%02X (RxSeqNum=%X) \n", txmsgdata[1], RxRapMsgSeq++);
//             send_mscmsg();
//             TxMSGIdle = TRUE;
         }

	cbus_send_mscmsg(it6602);
}

#endif

#ifdef _SUPPORT_UCP_
static void ucp_report_event( unsigned char key)
{
   // struct it6602_dev_data *it6602data = get_it6602_dev_data();

	//DLOG_INFO("ucp_report_event key: %X\n", (int) key);
	//input_report_key(it6602data->ucp_input, (unsigned int)key+1, 1);
	//input_report_key(it6602data->ucp_input, (unsigned int)key+1, 0);
	//input_sync(it6602data->ucp_input);

}
static void mhl_parse_UCPkey(struct it6602_dev_data *it6602)
{

	//parse_ucpkey( it6602->rxmsgdata[1] );

	if( (it6602->rxmsgdata[1]&0x80)==0x00 ) {
		it6602->txmsgdata[0] = MSG_UCPK;
		it6602->txmsgdata[1] = it6602->rxmsgdata[1];
		ucp_report_event(it6602->rxmsgdata[1]);
	}
	else {
		it6602->txmsgdata[0] = MSG_UCPE;
		it6602->txmsgdata[1] = 0x01;
	}

	cbus_send_mscmsg(it6602);
}
#endif


////////////////////////////////////////////////////////////////////
//static void mhl_read_mscmsg( struct it6602_dev_data *it6602 )
//
//
//
////////////////////////////////////////////////////////////////////
static void mhl_read_mscmsg( struct it6602_dev_data *it6602 )
{
//FIX_ID_024	xxxxx	//Fixed for RCP compliance issue
	unsigned int cbuswaitcnt=0;
	unsigned char MHL04;
//FIX_ID_024	xxxxx

    //mhltxbrd(0x60, 2, &it6602->rxmsgdata[0]);
	it6602->rxmsgdata[0] = mhlrxrd(0x60);
	it6602->rxmsgdata[1] = mhlrxrd(0x61);

    switch( it6602->rxmsgdata[0] ) {
		case MSG_MSGE :
			DLOG_INFO("RX MSGE => ");
			switch( it6602->rxmsgdata[1] ) {
				case 0x00:
					DLOG_INFO("No Error\n");
					break;
				case 0x01:
					DLOG_INFO("ERROR: Invalid sub-command code !!!\n");
					break;
				default  :
					DLOG_INFO("ERROR: Unknown MSC_MSG status code 0x%02X !!!\n", (int) it6602->rxmsgdata[1]);
			}
			 break;
#ifdef _SUPPORT_RCP_
		case MSG_RCP  :
			mhl_parse_RCPkey(it6602);
			break;
		case MSG_RCPK :
			 DLOG_INFO("RX RCPK  => ");
			parse_rcpkey( it6602->rxmsgdata[1]);
//FIX_ID_024	xxxxx	//Fixed for RCP compliance issue
			it6602->m_bRCPTimeOut = FALSE;
//FIX_ID_024	xxxxx
			SwitchRCPResult(it6602,RCP_Result_Finish);
			 break;
		case MSG_RCPE :
			 switch( it6602->rxmsgdata[1] ){
				case 0x00: DLOG_INFO("No Error\n"); break;
				case 0x01: DLOG_INFO("ERROR: Ineffective RCP Error Code !!!\n"); break;
				case 0x02: DLOG_INFO("Responder Busy ...\n"); break;
				default  : DLOG_INFO("ERROR: Unknown RCP status code !!!\n");
			 }
//FIX_ID_024 xxxxx	//Fixed for RCP compliance issue
			{

					it6602->m_bRCPError = TRUE;
					it6602->m_bRCPTimeOut = TRUE;
					do
					{
						cbuswaitcnt++;
						//DLOG_INFO("IT6602-MSG_RCPE then waiting RCPK ...\n");

						MHL04 = mhlrxrd(MHL_RX_04);
						if( MHL04&0x10 )
						{
							mhlrxwr(MHL_RX_04,0x10);

							it6602->rxmsgdata[0] = mhlrxrd(0x60);
							it6602->rxmsgdata[1] = mhlrxrd(0x61);

							if(it6602->rxmsgdata[0] == MSG_RCPK)
							{
								parse_rcpkey( it6602->rxmsgdata[1]);
								it6602->m_bRCPTimeOut=FALSE;
							}

							break;
						}

						//DLOG_INFO(" mhlrxrd(0x04)= 0x%02X !!!\n",(int)   MHL04);
						IT_Delay(1);
					}
					while(cbuswaitcnt<CBUSWAITNUM);

					DLOG_INFO(" mhlrxrd(0x04)= 0x%02X !!!\n",(int)   mhlrxrd(0x04));
					DLOG_INFO(" mhlrxrd(0x05)= 0x%02X !!!\n",(int)   mhlrxrd(0x05));
					DLOG_INFO(" mhlrxrd(0x06)= 0x%02X !!!\n",(int)   mhlrxrd(0x06));
					DLOG_INFO(" it6602->rxmsgdata[0]= 0x%02X !!!\n",(int)   it6602->rxmsgdata[0]);
					DLOG_INFO(" it6602->rxmsgdata[1]= 0x%02X !!!\n",(int)   it6602->rxmsgdata[1]);
			}
//FIX_ID_024	xxxxx
			SwitchRCPResult(it6602,RCP_Result_Finish);

			 break;
#endif
#ifdef _SUPPORT_RAP_
		case MSG_RAP  :
			mhl_parse_RAPkey(it6602);
			break;
		case MSG_RAPK :
			 DLOG_INFO("RX RAPK  => ");
			 switch( it6602->rxmsgdata[1] ) {
				case 0x00: DLOG_INFO("No Error\n"); break;
				case 0x01: DLOG_INFO("ERROR: Unrecognized Action Code !!!\n"); break;
				case 0x02: DLOG_INFO("ERROR: Unsupported Action Code !!!\n"); break;
				case 0x03: DLOG_INFO("Responder Busy ...\n"); break;
				default  : DLOG_INFO("ERROR: Unknown RAP status code 0x%02X !!!\n",(int)  it6602->rxmsgdata[1]);
			}
			break;
#endif
#ifdef _SUPPORT_UCP_
		case MSG_UCP  :
			mhl_parse_UCPkey(it6602);
			break;
		case MSG_UCPK :
			 break;
		case MSG_UCPE :
			 switch( it6602->rxmsgdata[1] ){
				case 0x00: DLOG_INFO("No Error\n"); break;
				case 0x01: DLOG_INFO("ERROR: Ineffective UCP Key Code !!!\n"); break;
				default  : DLOG_INFO("ERROR: Unknown UCP status code !!!\n");
			 }
			 break;
#endif
		default :
			DLOG_INFO("ERROR: Unknown MSC_MSG sub-command code 0x%02X !!!\n",(int)  it6602->rxmsgdata[0]);
			it6602->txmsgdata[0] = MSG_MSGE;
			it6602->txmsgdata[1] = 0x01;
			cbus_send_mscmsg(it6602);
			SwitchRCPResult(it6602,RCP_Result_Finish);
	}
}

////////////////////////////////////////////////////////////////////
//void mscWait( void )
//
//
//
////////////////////////////////////////////////////////////////////
static int mscWait( void )
{

//FIX_ID_005 xxxxx	//Add Cbus Event Handler

	int cbuswaitcnt;
	//unsigned char MHL04, MHL05, rddata[2];
	unsigned char MHL05, rddata[2];

	struct it6602_dev_data *it6602data = get_it6602_dev_data();

	cbuswaitcnt = 0;

	do
	{
		cbuswaitcnt++;
		IT_Delay(CBUSWAITTIME);
		MHL05 = mhlrxrd(MHL_RX_05);
		//printf("MSC Wait %X !!!\n",MHL05);
		if(MHL05 & 0x03) break;
	}
	while(cbuswaitcnt<CBUSWAITNUM);
//	while( (mhlrxrd(0x1C)&0x06)!=0  && cbuswaitcnt<CBUSWAITNUM);
//    while( (mhlrxrd(0x1C)&0x02)==0x02 && cbuswaitcnt<CBUSWAITNUM   );

	if( MHL05&0x01 )
	{
	mhlrxwr(MHL_RX_05, 0x01);
	//printf("msc Wait Req Done ...Ok %X !!!\n",MHL05);
	}

     MHL05 = mhlrxrd(0x05);
     if( (cbuswaitcnt==CBUSWAITNUM) || MHL05&0x02 )
     {
         if( cbuswaitcnt==CBUSWAITNUM )
		DLOG_INFO("ERROR: MSC Wait TimeOut !!!\n");


         if( MHL05&0x02 ) {
             DLOG_INFO("MSC Req Fail Interrupt ...Fail\n");
             mhlrxbrd(0x18, 2, &rddata[0]);
             mhlrxwr(MHL_RX_05, 0x02);

             if( rddata[0]&0x01 ) {
                 DLOG_INFO("Incomplete Packet !!!\n");
                 mhlrxwr(0x18, 0x01);
             }
             if( rddata[0]&0x02 ) {
                 DLOG_INFO("MSC 100ms TimeOut !!!\n");
                 mhlrxwr(0x18, 0x02);
             }
             if( rddata[0]&0x04 ) {
                 DLOG_INFO("Protocol Error !!!\n");
                 mhlrxwr(0x18, 0x04);
             }
             if( rddata[0]&0x08 ) {
                 DLOG_INFO("Retry > 32 times !!!\n");
                 mhlrxwr(0x18, 0x08);
             }
             if( rddata[0]&0x10 ) {
                 DLOG_INFO("Receive ABORT Packet !!!\n");
                 mhlrxwr(0x18, 0x10);
//                 return RCVABORT;
             }
             if( rddata[0]&0x20 ) {
                // MSGNackCnt++;
                // MSCFailCnt--;
                 DLOG_INFO("MSC_MSG Requester Receive NACK Packet !!! \n");
                 mhlrxwr(0x18, 0x20);
//                 return RCVNACK;
             }
             if( rddata[0]&0x40 ) {
                 DLOG_INFO("Disable HW Retry and MSC Requester Arbitration Lose at 1st Packet !!! \n");
                 mhlrxwr(0x18, 0x40);

 //                if( enarblosetrg )
 //                    printf("Trgger !!!");

//                 return ARBLOSE;
             }
            if( rddata[0]&0x80 ) {
                 DLOG_INFO("Disable HW Retry and MSC Requester Arbitration Lose before 1st Packet !!! \n");

                 mhlrxwr(0x18, 0x80);
//                 return ARBLOSE;
             }

             if( rddata[1]&0x01 ) {
                 DLOG_INFO("TX FW Fail in the middle of the command sequence !!!\n");
                 mhlrxwr(0x19, 0x01);
//                 return FWTXFAIL;
             }
             if( rddata[1]&0x02 ) {
                 DLOG_INFO("TX Fail because FW mode RxPktFIFO not empty !!!\n");
                 mhlrxwr(0x19, 0x02);
//                 return FWRXPKT;
             }
         }
         else
             DLOG_INFO("Unknown Issue !!!\n");

     DLOG_INFO("\n\n");


	DLOG_INFO("!!! mscWait Fail !!!\r\n");

#if 0
	printf("MHL04 %X \r\n",mhlrxrd(0x04));
	printf("MHL05 %X \r\n",mhlrxrd(0x05));
	printf("MHL06 %X \r\n\n",mhlrxrd(0x06));

	printf("MHL15 %X \r\n",mhlrxrd(0x15));
	printf("MHL16 %X \r\n",mhlrxrd(0x16));
	printf("MHL17 %X \r\n",mhlrxrd(0x17));
	printf("MHL18 %X \r\n",mhlrxrd(0x18));
	printf("MHL19 %X \r\n",mhlrxrd(0x19));
	printf("MHL1A %X \r\n",mhlrxrd(0x1A));
	printf("MHL1B %X \r\n",mhlrxrd(0x1B));
	printf("MHL1C %X \r\n\n",mhlrxrd(0x1C));
#endif

 	return FAIL;
     }


//FIX_ID_005 xxxxx

	return SUCCESS;

}




//FIX_ID_037 xxxxx //Allion MHL compliance issue debug !!!
////////////////////////////////////////////////////////////////////
//void mscCheckResult( void )
//
//
//
////////////////////////////////////////////////////////////////////
static int mscCheckResult(void)
{
//FIX_ID_005 xxxxx	//Add Cbus Event Handler
// disable -> 	int  fwmodeflag = FALSE;
// disable -> 	int  wrburstflag = FALSE;
	int mscreqsts=FAIL;
	int result=SUCCESS;
	int ucMHL1C=0;

	struct it6602_dev_data *it6602data = get_it6602_dev_data();

	//TickCountPrint();

//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
	if(it6602data->CBusWaitNo !=0)
		return FAIL;
//FIX_ID_037 xxxxx

// disable -> 	if( offset==0x51)
// disable -> 	{
// disable -> 		if(wdata==0x80 )
// disable -> 			fwmodeflag  = TRUE;
// disable -> 		if(wdata==0x01 )
// disable -> 			wrburstflag = TRUE;
// disable -> 	}


	ucMHL1C = mhlrxrd(MHL_RX_1C);

	//for debug only !!!
//RedText;
	//DLOG_INFO("IT6602-MSC check() ucMHL1C = %X  \n",(int) ucMHL1C);
	DLOG_INFO("IT6602-mscCheckResult () \r\n");
//WhiteText;
	//for debug only !!!

	if((ucMHL1C & 0x07)==0)
	{
		//Allion MHL compliance issue debug !!! disable -> mhlrxwr((unsigned char)offset, (unsigned char)wdata);
		mscreqsts = mscWait();
		//DLOG_INFO("IT6602-mscCheckResult () %X  \n",(int) mscreqsts);

		if(mscreqsts != SUCCESS)
		{
			DLOG_INFO("mscreqsts = FAIL \r\n");
//2014-0526 disable MHL compliance issue ->
			result = FAIL;
		}

	}
	else
	{
		DLOG_INFO("ucMHL1C %X \r\n",(int) ucMHL1C);
//2014-0526 disable MHL compliance issue ->
		result = FAIL;
	}

//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
#if 1
//GreenText;
	if(m_MHLabortID == MHL_ABORT_ID)
	{
		it6602data->CBusIntEvent |= (B_MSC_Waiting);
		it6602data->CBusWaitNo=MAX_BUSY_WAITNO;	//100ms
		result = FAIL;
		DLOG_INFO("m_MHLabortID == MHL_ABORT_ID \r\n");
		m_MHLabortID++;
	}
	else
	{
		DLOG_INFO("m_MHLabortID =%d \r\n",(int) m_MHLabortID);
	}
//WhiteText;
#endif
//FIX_ID_037 xxxxx

	if(result==FAIL)
	{
		it6602data->CBusIntEvent |= (B_MSC_Waiting);
		it6602data->CBusWaitNo=MAX_BUSY_WAITNO;	//100ms
	}

	return (result==SUCCESS)?SUCCESS:FAIL;

//FIX_ID_005 xxxxx
}
//FIX_ID_037 xxxxx
//FIX_ID_037 xxxxx //Allion MHL compliance issue debug !!!

////////////////////////////////////////////////////////////////////
//void mscFire( void )
//
//
//
////////////////////////////////////////////////////////////////////
static int mscFire( int offset, int wdata )
{
//FIX_ID_005 xxxxx	//Add Cbus Event Handler
// disable -> 	int  fwmodeflag = FALSE;
// disable -> 	int  wrburstflag = FALSE;
	int mscreqsts=FAIL;
	int result=SUCCESS;
	int ucMHL1C=0;

	struct it6602_dev_data *it6602data = get_it6602_dev_data();

	// TickCountPrint();

//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
	if(it6602data->CBusWaitNo !=0)
		return FAIL;
//FIX_ID_037 xxxxx

// disable -> 	if( offset==0x51)
// disable -> 	{
// disable -> 		if(wdata==0x80 )
// disable -> 			fwmodeflag  = TRUE;
// disable -> 		if(wdata==0x01 )
// disable -> 			wrburstflag = TRUE;
// disable -> 	}


	ucMHL1C = mhlrxrd(MHL_RX_1C);

	//for debug only !!!
//RedText;
	//DLOG_INFO("IT6602-MSC FIRE() ucMHL1C = %X  \n",(int) ucMHL1C);
	DLOG_INFO("IT6602-MSC FIRE() offset = %X wdata = %X  \n",(int) offset , (int) wdata);
//WhiteText;
	//for debug only !!!

	if((ucMHL1C & 0x07)==0)
	{
		mhlrxwr((unsigned char)offset, (unsigned char)wdata);
		mscreqsts = mscWait();
		//DLOG_INFO("IT6602-MSC FIRE() %X  \n",(int) mscreqsts);

		if(mscreqsts != SUCCESS)
		{
			DLOG_INFO("mscreqsts = FAIL \r\n");
//2014-0526 disable MHL compliance issue ->
			result = FAIL;
		}

	}
	else
	{
		DLOG_INFO("ucMHL1C %X \r\n",(int) ucMHL1C);
//2014-0526 disable MHL compliance issue ->
		result = FAIL;
	}

//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
#if 1
// GreenText;
	if(m_MHLabortID == MHL_ABORT_ID)
	{
		it6602data->CBusIntEvent |= (B_MSC_Waiting);
		it6602data->CBusWaitNo=MAX_BUSY_WAITNO;	//100ms
		result = FAIL;
		DLOG_INFO("m_MHLabortID == MHL_ABORT_ID \r\n");
		m_MHLabortID++;
	}
	else
	{
		DLOG_INFO("m_MHLabortID =%d \r\n",(int) m_MHLabortID);
	}
//  WhiteText;
#endif
//FIX_ID_037 xxxxx

	if(result==FAIL)
	{
		it6602data->CBusIntEvent |= (B_MSC_Waiting);
		it6602data->CBusWaitNo=MAX_BUSY_WAITNO;	//100ms
	}

	return (result==SUCCESS)?SUCCESS:FAIL;

//FIX_ID_005 xxxxx
}
//FIX_ID_037 xxxxx


static int cbus_send_mscmsg( struct it6602_dev_data *it6602 )
{
	int uc;

	mhlrxwr(0x54, it6602->txmsgdata[0]);
	mhlrxwr(0x55, it6602->txmsgdata[1]);
	uc = mscFire(MHL_MSC_CtrlPacket1, B_MSC_MSG);

	return (uc==SUCCESS)?SUCCESS:FAIL;
}

#if 1
//== MHL interrupt ===
//
//
//
//
static void parse_devcap(unsigned char *devcap )
{
    DLOG_INFO("\nParsing Device Capability Register ...\n\n");
    DLOG_INFO("DEV_STATE=%02X\n", (int) devcap[0]);

    DLOG_INFO("MHL_VER_MAJOR/MINOR=%02X\n", (int) devcap[1]);

    DLOG_INFO("DEV_TYPE=");
    switch(devcap[2]&0x0F){
        case 0: DLOG_INFO("ERROR: DEV_TYPE at least one bit must be set !!!\n"); break;
        case 1: DLOG_INFO("DEV_TYPE = Sink, "); break;
        case 2: DLOG_INFO("DEV_TYPE = Source, "); break;
        case 3: DLOG_INFO("DEV_TYPE = Dongle, "); break;
        default: DLOG_INFO("ERROR: Reserved for future use !!! "); break;
    }
    DLOG_INFO("POW = %02X\n", (int) (devcap[2]&0x10)>>4);
    DLOG_INFO("PLIM = %02X\n", (int) (devcap[2]&0x60)>>5);

    DLOG_INFO("ADOPTER_ID_H=0x%02X, ADOPTER_ID_L=0x%02X\n",(int)  devcap[3], (int) devcap[4]);

    DLOG_INFO("VID_LINK_MODE:\n");
    DLOG_INFO("    SUPP_RGB444   = %02X\n", (int) (devcap[5]&0x01)>>0);
    DLOG_INFO("    SUPP_YCBCR444 = %02X\n", (int) (devcap[5]&0x02)>>1);
    DLOG_INFO("    SUPP_YCBCR422 = %02X\n", (int) (devcap[5]&0x04)>>2);
    DLOG_INFO("    SUPP_PPIXEL   = %02X\n", (int) (devcap[5]&0x08)>>3);
    DLOG_INFO("    SUPP_ISLANDS  = %02X\n", (int) (devcap[5]&0x10)>>4);
    DLOG_INFO("    SUPP_VGA      = %02X\n",(int)  (devcap[5]&0x20)>>5);

    DLOG_INFO("AUD_LINK_MODE:\n");
    DLOG_INFO("    SUPP_2CH  = %02X\n", (int) (devcap[6]&0x01)>>0);
    DLOG_INFO("    SUPP_8CH  = %02X\n", (int) (devcap[6]&0x02)>>1);

    if( devcap[7]&0x80 ) {
        DLOG_INFO("VIDEO_TYPE: \n");
        DLOG_INFO("    VT_GRAPHICS = %02X\n",(int)  (devcap[7]&0x01)>>0);
        DLOG_INFO("    VT_PHOTO    = %02X\n", (int) (devcap[7]&0x02)>>1);
        DLOG_INFO("    VT_CINEMA   = %02X\n", (int) (devcap[7]&0x04)>>2);
        DLOG_INFO("    VT_GAME     = %02X\n", (int) (devcap[7]&0x08)>>3);
    }
    else
        DLOG_INFO("Not Support VIDEO_TYPE !!!\n");

    DLOG_INFO("LOG_DEV_MAP:\n");
    DLOG_INFO("    LD_DISPLAY  = %02X\n",(int)  (devcap[8]&0x01)>>0);
    DLOG_INFO("    LD_VIDEO    = %02X\n",(int)  (devcap[8]&0x02)>>1);
    DLOG_INFO("    LD_AUDIO    = %02X\n", (int) (devcap[8]&0x04)>>2);
    DLOG_INFO("    LD_MEDIA    = %02X\n",(int)  (devcap[8]&0x08)>>3);
    DLOG_INFO("    LD_TUNER    = %02X\n", (int) (devcap[8]&0x10)>>4);
    DLOG_INFO("    LD_RECORD   = %02X\n",(int)  (devcap[8]&0x20)>>5);
    DLOG_INFO("    LD_SPEAKER  = %02X\n",(int)  (devcap[8]&0x40)>>6);
    DLOG_INFO("    LD_GUI      = %02X\n", (int) (devcap[8]&0x80)>>7);

    DLOG_INFO("BANDWIDTH = %XMHz\n", (int) devcap[9]*5);

    DLOG_INFO("FEATURE_FLAG:\n");
    DLOG_INFO("         RCP_SUPPORT = %02X\n", (int) (devcap[10]&0x01)>>0);
    DLOG_INFO("         RAP_SUPPORT = %02X\n", (int) (devcap[10]&0x02)>>1);
    DLOG_INFO("         SP_SUPPORT  = %02X\n", (int) (devcap[10]&0x04)>>2);
    DLOG_INFO("    UCP_SEND_SUPPORT = %02X\n", (int) (devcap[10]&0x08)>>3);
    DLOG_INFO("    UCP_RECV_SUPPORT = %02X\n", (int) (devcap[10]&0x10)>>4);

    DLOG_INFO("DEVICE_ID_H=0x%02X, DEVICE_ID_L=0x%02X\n", (int) devcap[11], (int) devcap[12]);

    if( devcap[10]&0x04 )
        DLOG_INFO("SCRATCHPAD_SIZE = %02X Bytes\n", (int) devcap[13]);

    DLOG_INFO("INT_SIZE  = %02X Bytes\n", (int) (devcap[14]&0x0F)+1);
    DLOG_INFO("STAT_SIZE = %02X Bytes\n",(int)  ((devcap[14]&0xF0)>>4)+1);

}

////////////////////////////////////////////////////////////////////
//void read_devcap_hw( void )
//
//
//
////////////////////////////////////////////////////////////////////
static int read_devcap_hw( struct it6602_dev_data *it6602 )
{
	unsigned char offset;

	DLOG_INFO("IT6602-\nRead Device Capability Start ...\n");

	for(offset=0; offset<0x10; offset++) {

		mhlrxwr(0x54, offset);

		if( mscFire(MHL_MSC_CtrlPacket0, B_READ_DEVCAP) == SUCCESS )
			it6602->Mhl_devcap[offset] = mhlrxrd(0x56);
		else
			return -1;


		DLOG_INFO("IT6602-DevCap[%X]=%X\n", (int)offset, (int)it6602->Mhl_devcap[offset]);
	}

	DLOG_INFO("IT6602-Read Device Capability End...\n");

	parse_devcap(&it6602->Mhl_devcap[0]);

	return 0;

}


static int set_mhlint( unsigned char offset, unsigned char field )
{
    int uc;
    mhlrxwr(0x54, offset);
    mhlrxwr(0x55, field);
    uc = mscFire(MHL_MSC_CtrlPacket0, B_WRITE_STAT_SET_INT);

	return (uc==SUCCESS)?SUCCESS:FAIL;

}

static int set_mhlsts( unsigned char offset, unsigned char status )
{
    int uc;
    mhlrxwr(0x54, offset);
    mhlrxwr(0x55, status);
    uc = mscFire(MHL_MSC_CtrlPacket0, B_WRITE_STAT_SET_INT);

	if(uc==SUCCESS)
		DLOG_INFO("uc==SUCCESS \r\n");
	else
		DLOG_INFO("uc = FAIL \r\n");


	return (uc==SUCCESS)?SUCCESS:FAIL;
}

// disable -> static void write_burst(struct it6602_dev_data *it6602, int offset, int byteno )
// disable -> {
// disable ->     set_wrburst_data(it6602, offset, byteno);
// disable -> //    DLOG_INFO("Fire WRITE_BURST (WrData=0x%X) ...\n", it6602->txscrpad[3]);
// disable -> //  enarblosetrg = TRUE;
// disable ->     mscFire(MHL_MSC_CtrlPacket1, B_WRITE_BURST);        // fire WRITE_BURST
// disable -> //  enarblosetrg = FALSE;
// disable ->     mhlrxset(0x00, 0x80, EnCBusDbgMode<<7);  // Restore setting
// disable -> }
// disable ->
// disable -> static void set_wrburst_data(struct it6602_dev_data *it6602, int offset, int byteno )
// disable -> {
// disable ->     int i;
// disable ->
// disable ->     if( byteno<=2 || (offset+byteno)>16 ) {
// disable ->         DLOG_INFO("ERROR: Set Burst Write Data Fail !!!\n");
// disable ->         return;
// disable ->     }
// disable ->
// disable -> //    mhlrxbwr(0x5E, 2, &it6602->txscrpad[0]);
// disable ->     mhlrxwr(0x54, 0x40+offset);
// disable ->
// disable ->     if( EnMSCBurstWr ) {
// disable ->         mhlrxset(0x00, 0x80, 0x00); // Disable CBUS Debug Mode when using Burst Write
// disable ->
// disable ->
// disable ->         if( MSCBurstWrID )
// disable ->             i = 2;
// disable ->         else
// disable ->             i = 0;
// disable ->
// disable -> 		for(; i<byteno; i++)
// disable ->             mhlrxwr(0x59, it6602->txscrpad[i]);
// disable ->
// disable ->     }
// disable ->     else
// disable ->         mhlrxwr(0x55, it6602->txscrpad[2]);
// disable -> }


#endif

//#if 0
//static void v3d_burst1st(struct it6602_dev_data *it6602)
//{
//     it6602->txscrpad[0]  = 0x00;
//	 it6602->txscrpad[1]  = 0x11;
//	 it6602->txscrpad[2]  = 0xC1;
//	 it6602->txscrpad[3]  = 0x05;
//	 it6602->txscrpad[4]  = 0x01;
//	 it6602->txscrpad[5]  = 0x05;
//	 it6602->txscrpad[6]  = 0x00;
//	 it6602->txscrpad[7]  = 0x07;
//	 it6602->txscrpad[8]  = 0x00;
//	 it6602->txscrpad[9]  = 0x07;
//	 it6602->txscrpad[10] = 0x00;
//	 it6602->txscrpad[11] = 0x07;
//	 it6602->txscrpad[12] = 0x00;
//	 it6602->txscrpad[13] = 0x07;
//	 it6602->txscrpad[14] = 0x00;
//	 it6602->txscrpad[15] = 0x07;
//
//	 mhlrxwr(0x5E, it6602->txscrpad[0]);
//	 mhlrxwr(0x5F, it6602->txscrpad[1]);
//	 wrburstoff = 0;
//	 wrburstnum = 16;
//	 write_burst(it6602,wrburstoff, wrburstnum);
//	 DLOG_INFO("Set DSCR_CHG to 1 (TxSeqNum=%X) ...\n", TxWrBstSeq++);
//	 set_mhlint(MHLInt00B, DSCR_CHG);
//	V3D_EntryCnt++;
//	 set_mhlint(MHLInt00B, REQ_WRT);
//}
//
//static void v3d_burst2nd(struct it6602_dev_data *it6602)
//{
//	 it6602->txscrpad[0]  = 0x00;
//	 it6602->txscrpad[1]  = 0x10;
//	 it6602->txscrpad[2]  = 0xB8;
//	 it6602->txscrpad[3]  = 0x0F;
//	 it6602->txscrpad[4]  = 0x01;
//	 it6602->txscrpad[5]  = 0x05;
//	 it6602->txscrpad[6]  = 0x00;
//	 it6602->txscrpad[7]  = 0x07;
//	 it6602->txscrpad[8]  = 0x00;
//	 it6602->txscrpad[9]  = 0x07;
//	 it6602->txscrpad[10] = 0x00;
//	 it6602->txscrpad[11] = 0x07;
//	 it6602->txscrpad[12] = 0x00;
//	 it6602->txscrpad[13] = 0x07;
//	 it6602->txscrpad[14] = 0x00;
//	 it6602->txscrpad[15] = 0x07;
//
//	 mhlrxwr(0x5E, it6602->txscrpad[0]);
//	 mhlrxwr(0x5F, it6602->txscrpad[1]);
//	 wrburstoff = 0;
//	 wrburstnum = 16;
//	 write_burst(it6602,wrburstoff, wrburstnum);
//	 DLOG_INFO("Set DSCR_CHG to 1 (TxSeqNum=%X) ...\n", TxWrBstSeq++);
//	 set_mhlint(MHLInt00B, DSCR_CHG);
//	V3D_EntryCnt++;
//	 set_mhlint(MHLInt00B, REQ_WRT);
//}
//
//static void v3d_burst3rd(struct it6602_dev_data *it6602)
//{
//	 it6602->txscrpad[0]  = 0x00;
//	 it6602->txscrpad[1]  = 0x10;
//	 it6602->txscrpad[2]  = 0xB7;
//	 it6602->txscrpad[3]  = 0x0F;
//	 it6602->txscrpad[4]  = 0x02;
//	 it6602->txscrpad[5]  = 0x05;
//	 it6602->txscrpad[6]  = 0x00;
//	 it6602->txscrpad[7]  = 0x07;
//	 it6602->txscrpad[8]  = 0x00;
//	 it6602->txscrpad[9]  = 0x07;
//	 it6602->txscrpad[10] = 0x00;
//	 it6602->txscrpad[11] = 0x07;
//	 it6602->txscrpad[12] = 0x00;
//	 it6602->txscrpad[13] = 0x07;
//	 it6602->txscrpad[14] = 0x00;
//	 it6602->txscrpad[15] = 0x07;
//
//	 mhlrxwr(0x5E, it6602->txscrpad[0]);
//	 mhlrxwr(0x5F, it6602->txscrpad[1]);
//	 wrburstoff = 0;
//	 wrburstnum = 16;
//	 write_burst(it6602,wrburstoff, wrburstnum);
//	 DLOG_INFO("Set DSCR_CHG to 1 (TxSeqNum=%X) ...\n", TxWrBstSeq++);
//	 set_mhlint(MHLInt00B, DSCR_CHG);
//	V3D_EntryCnt++;
//	 set_mhlint(MHLInt00B, REQ_WRT);
//}
//
//static void v3d_burst4th(struct it6602_dev_data *it6602)
//{
//	 it6602->txscrpad[0]  = 0x00;
//	 it6602->txscrpad[1]  = 0x10;
//	 it6602->txscrpad[2]  = 0xB6;
//	 it6602->txscrpad[3]  = 0x0F;
//	 it6602->txscrpad[4]  = 0x03;
//	 it6602->txscrpad[5]  = 0x05;
//	 it6602->txscrpad[6]  = 0x00;
//	 it6602->txscrpad[7]  = 0x07;
//	 it6602->txscrpad[8]  = 0x00;
//	 it6602->txscrpad[9]  = 0x07;
//	 it6602->txscrpad[10] = 0x00;
//	 it6602->txscrpad[11] = 0x07;
//	 it6602->txscrpad[12] = 0x00;
//	 it6602->txscrpad[13] = 0x07;
//	 it6602->txscrpad[14] = 0x00;
//	 it6602->txscrpad[15] = 0x07;
//
//	 mhlrxwr(0x5E, it6602->txscrpad[0]);
//	 mhlrxwr(0x5F, it6602->txscrpad[1]);
//	 wrburstoff = 0;
//	 wrburstnum = 16;
//	 write_burst(it6602,wrburstoff, wrburstnum);
//	 DLOG_INFO("Set DSCR_CHG to 1 (TxSeqNum=%X) ...\n", TxWrBstSeq++);
//	 set_mhlint(MHLInt00B, DSCR_CHG);
//	V3D_EntryCnt++;
//}
//
//
//
//static void v3d_unsupport1st(struct it6602_dev_data *it6602)
//{
////FIX_ID_013	xxxxx	//For Acer MHL Dongle MSC 3D request issue
//	 it6602->txscrpad[0]  = 0x00;
//	 it6602->txscrpad[1]  = 0x10;
//	 it6602->txscrpad[2]  = 0xF0;
//	 it6602->txscrpad[3]  = 0x00;
////FIX_ID_013	xxxxx
//
//
//	 mhlrxwr(0x5E, it6602->txscrpad[0]);
//	 mhlrxwr(0x5F, it6602->txscrpad[1]);
//	 wrburstoff = 0;
//	 wrburstnum = 4;
//	 write_burst(it6602,wrburstoff, wrburstnum);
//
//	 DLOG_INFO("Set DSCR_CHG to 1 (TxSeqNum=%X) ...\n", TxWrBstSeq++);
//	 set_mhlint(MHLInt00B, DSCR_CHG);
//	V3D_EntryCnt++;
// 	 set_mhlint(MHLInt00B, REQ_WRT);
//}
//
//static void v3d_unsupport2nd(struct it6602_dev_data *it6602)
//{
////FIX_ID_013	xxxxx	//For Acer MHL Dongle MSC 3D request issue
//	 it6602->txscrpad[0]  = 0x00;
//	 it6602->txscrpad[1]  = 0x11;
//	 it6602->txscrpad[2]  = 0xEF;
//	 it6602->txscrpad[3]  = 0x00;
////FIX_ID_013	xxxxx
//
//	 mhlrxwr(0x5E, it6602->txscrpad[0]);
//	 mhlrxwr(0x5F, it6602->txscrpad[1]);
//	 wrburstoff = 0;
//	 wrburstnum = 4;
//	 write_burst(it6602,wrburstoff, wrburstnum);
//	 DLOG_INFO("Set DSCR_CHG to 1 (TxSeqNum=%X) ...\n", TxWrBstSeq++);
//	 set_mhlint(MHLInt00B, DSCR_CHG);
//	V3D_EntryCnt++;
//}
//
//#endif
/* MHL 3D functions    *******************************************************/

#ifdef FIX_ID_013_
//FIX_ID_013	xxxxx	//For MSC 3D request issue
static unsigned char Msc_WriteBurstDataFill(unsigned char ucOffset, unsigned char ucByteNo, unsigned char *pucData)
{
	unsigned char	ucTemp;

	if(ucByteNo<=2 || (ucOffset+ucByteNo)>16){
		DLOG_INFO("ERROR: Set Burst Write Data Fail\n");
		return FALSE;
	}else{
		mhlrxwr(0x5E, pucData[0]);
		mhlrxwr(0x5F, pucData[1]);
		mhlrxwr(0x54, 0x40+ucOffset);
		if(MSCBurstWrID)
			ucTemp = 2;
		else
			ucTemp = 0;

		//disable ->	DLOG_INFO("Msc_WriteBurstDataFill =");
		for(; ucTemp < ucByteNo;)
		{
			//disable ->	DLOG_INFO(" 0x%x\n ", (int)pucData[ucTemp]);
			mhlrxwr(0x59, pucData[ucTemp++]);
		}
		//disable ->	DLOG_INFO("\n");
		return TRUE;
	}
}



static MHL3D_STATE MSC_3DInforSend(unsigned char b3dDtd)
{
	unsigned char	ucTemp, uc3DInforLen;
	unsigned char	ucWBData[16];
	unsigned char	uc3DTempCnt;
	MHL3D_STATE eRet3dState;

	uc3DTempCnt = st3DParse.uc3DTempCnt & 0x7F;

	if(b3dDtd){
		ucWBData[0] = MSC_3D_DTD >> 8;
		ucWBData[1] = MSC_3D_DTD & 0xff;
		ucWBData[3] = st3DParse.ucDtdCnt;
	}else{
		ucWBData[0] = MSC_3D_VIC >> 8;
		ucWBData[1] = MSC_3D_VIC & 0xff;
		ucWBData[3] = st3DParse.ucVicCnt;
	}
	ucWBData[2] = 0;
	ucWBData[4] = (uc3DTempCnt / 5) + 1;

	ucWBData[5] = ucWBData[3] - uc3DTempCnt;
	if(ucWBData[5] > 5){
		ucWBData[5] = 5;
		eRet3dState = MHL3D_REQ_WRT;
		st3DParse.uc3DTempCnt += 5;
		DLOG_INFO("*** MSC_3DInforSend MHL3D_REQ_WRT ***\r\n");
	}else{
		if(b3dDtd){
			st3DParse.uc3DTempCnt = 0;
			eRet3dState = MHL3D_REQ_WRT;
			DLOG_INFO("*** MSC_3DInforSend DTD Done ***\r\n");
		}else{
			st3DParse.uc3DTempCnt = 0x80;
			eRet3dState = MHL3D_REQ_DONE;
			DLOG_INFO("*** MSC_3DInforSend VIC Done ***\r\n");
		}
	}
	uc3DInforLen = 6 + (ucWBData[5] * 2);
	for(ucTemp = 6; ucTemp < uc3DInforLen; ){
		ucWBData[ucTemp++] = 0x00;
		if(b3dDtd){
			ucWBData[ucTemp++] = uc3DDtd[uc3DTempCnt++];
		}else{
			ucWBData[ucTemp++] = st3DParse.uc3DInfor[uc3DTempCnt++];
		}
	}
	do{
		if(--ucTemp != 2){
			ucWBData[2] -= ucWBData[ucTemp];
		}
	}while(ucTemp);


	DLOG_INFO("txscrpad --> ");
	for(ucTemp=0;ucTemp <= uc3DInforLen;ucTemp++)
	{
	DLOG_INFO("%x ",(int)ucWBData[ucTemp]);
	}
	DLOG_INFO("\r\n");

	if(TRUE == Msc_WriteBurstDataFill(0x00, uc3DInforLen, ucWBData))
	{
		mscFire(MHL_MSC_CtrlPacket1, B_WRITE_BURST);        // fire WRITE_BURST
		set_mhlint(MHLInt00B, DSCR_CHG);
	}
	// disable -> DLOG_INFO("*** MSC_3DInforSend eRet3dState = 0x%x ***\n", (int)eRet3dState);
	return eRet3dState;
}


static void Msc_3DProcess(MHL3D_STATE *e3DReqState)
{
	unsigned char	ucTemp;
	unsigned char	uc;

	DLOG_INFO("***Msc_3DProcess***\r\n");
	switch(*e3DReqState){
		case	MHL3D_REQ_DONE:
				//DLOG_INFO("***Msc_3DProcess*** MHL3D_REQ_DONE\n");
			break;
		case	MHL3D_REQ_START:
				DLOG_INFO("***Msc_3DProcess*** MHL3D_REQ_START\r\n");
				for(ucTemp=0;ucTemp < 16;ucTemp++)
				{
					uc =st3DParse.uc3DInfor[ucTemp];
					DLOG_INFO("%x ",(int)uc);
				}
				DLOG_INFO("\n***Msc_3DProcess*** MHL3D_REQ_START\r\n");

				ucTemp = sizeof(uc3DDtd);
				DLOG_INFO("\n uc3DDtd = %X \r\n",(int) uc3DDtd[0]);
				if((ucTemp == 1) && (uc3DDtd[0]==0))
				ucTemp = 0;
				st3DParse.ucDtdCnt = ucTemp;
				st3DParse.uc3DTempCnt = 0x80;


				set_mhlint(MHLInt00B, REQ_WRT);
				*e3DReqState = MHL3D_REQ_WRT;


			break;
		case	MHL3D_REQ_WRT:
					DLOG_INFO("***Msc_3DProcess*** MHL3D_REQ_WRT\r\n");
					*e3DReqState = MHL3D_GNT_WRT;
			break;
		case	MHL3D_GNT_WRT:
				DLOG_INFO("***Msc_3DProcess*** MHL3D_GNT_WRT\r\n");
				if(st3DParse.uc3DTempCnt & 0x80){
					*e3DReqState = MSC_3DInforSend(TRUE);
					//DLOG_INFO("***Msc_3DProcess*** MSC_3DInforSend(TRUE)\r\n");
				}else{
					*e3DReqState = MSC_3DInforSend(FALSE);
					//DLOG_INFO("***Msc_3DProcess*** MSC_3DInforSend(FALSE)\r\n");
				}
					if(*e3DReqState == MHL3D_REQ_DONE){
						st3DParse.uc3DTempCnt = 0x80;
					DLOG_INFO("***Msc_3DProcess*** MHL3D_REQ_DONE\r\n");
				}else{

					set_mhlint(MHLInt00B, REQ_WRT);
					*e3DReqState = MHL3D_REQ_WRT;
					DLOG_INFO("***Msc_3DProcess*** MHL3D_REQ_WRT\r\n");
				}
			break;
	default:
		break;

	}

}
//FIX_ID_013	xxxxx
#else
static void write_burst(struct it6602_dev_data *it6602, int offset, int byteno )
{
    set_wrburst_data(it6602,offset, byteno);
//    DLOG_INFO("Fire WRITE_BURST (WrData=0x%X) ...\n", it6602data->txscrpad[3]);
//  enarblosetrg = TRUE;
    mscFire(MHL_MSC_CtrlPacket1, B_WRITE_BURST);        // fire WRITE_BURST
//  enarblosetrg = FALSE;
    mhlrxset(0x00, 0x80, EnCBusDbgMode<<7);  // Restore setting
}

static void set_wrburst_data(struct it6602_dev_data *it6602, int offset, int byteno )
{
    int i;
// disable ->
    if( byteno<=2 || (offset+byteno)>16 )
   {
        DLOG_INFO("ERROR: Set Burst Write Data Fail !!!\n");
        return;
    }
// disable ->
//    mhlrxbwr(0x5E, 2, &it6602data->txscrpad[0]);
    mhlrxwr(0x54, 0x40+offset);
// disable ->
    if( EnMSCBurstWr )
  {
        mhlrxset(0x00, 0x80, 0x00); // Disable CBUS Debug Mode when using Burst Write
// disable ->
// disable ->
        if( MSCBurstWrID )
            i = 2;
        else
            i = 0;
// disable ->
		for(; i<byteno; i++)
            mhlrxwr(0x59, it6602->txscrpad[i]);
// disable ->
    }
    else
        mhlrxwr(0x55, it6602->txscrpad[2]);
}


static void v3d_unsupport1st(struct it6602_dev_data *it6602)
{
//FIX_ID_013	xxxxx	//For MSC 3D request issue
	 it6602->txscrpad[0]  = 0x00;
	 it6602->txscrpad[1]  = 0x10;
	 it6602->txscrpad[2]  = 0xF0;
	 it6602->txscrpad[3]  = 0x00;
//FIX_ID_013	xxxxx

     mhlrxwr(0x5E, it6602->txscrpad[0]);
     mhlrxwr(0x5F, it6602->txscrpad[1]);
     wrburstoff = 0;
     wrburstnum = 4;
     write_burst(it6602,wrburstoff, wrburstnum);

	// disable ->      #ifdef DEBUG_MODE
	// disable ->      dbmsg_ftrace(DBM_DPATH,"Set DSCR_CHG to 1 (TxSeqNum=%d) ...\r\n", TxWrBstSeq++);
	// disable ->      #endif
     set_mhlint(MHLInt00B, DSCR_CHG);
     V3D_EntryCnt++;
     set_mhlint(MHLInt00B, REQ_WRT);
}

static void v3d_unsupport2nd(struct it6602_dev_data *it6602)
{
//FIX_ID_013	xxxxx	//For MSC 3D request issue
	 it6602->txscrpad[0]  = 0x00;
	 it6602->txscrpad[1]  = 0x11;
	 it6602->txscrpad[2]  = 0xEF;
	 it6602->txscrpad[3]  = 0x00;
//FIX_ID_013	xxxxx

     mhlrxwr(0x5E, it6602->txscrpad[0]);
     mhlrxwr(0x5F, it6602->txscrpad[1]);
     wrburstoff = 0;
     wrburstnum = 4;
     write_burst(it6602,wrburstoff, wrburstnum);
	// disable ->      #ifdef DEBUG_MODE
	// disable ->      dbmsg_ftrace(DBM_DPATH,"Set DSCR_CHG to 1 (TxSeqNum=%d) ...\r\n", TxWrBstSeq++);
	// disable ->      #endif
     set_mhlint(MHLInt00B, DSCR_CHG);
     V3D_EntryCnt++;
}

#endif //FIX_ID_013	xxxxx


#if 1
//FIX_ID_021 xxxxx		//To use CP_100ms for CBus_100ms and CEC_100ms
////////////////////////////////////////////////////////////////////
//static unsigned char OSCvalueCompare(unsigned long *calibrationValue)
//
//
//
////////////////////////////////////////////////////////////////////
static unsigned char OSCvalueCompare(unsigned long *calibrationValue)
{
	unsigned long diff_cp;
	unsigned long cp_cal_mean = CP_MEAN_VALUE;

	if(*calibrationValue != 0)
	{
	*calibrationValue /= 100;
	}

	if( *calibrationValue >= cp_cal_mean )
	{
	diff_cp = (*calibrationValue-cp_cal_mean);
	DLOG_INFO("diff_cp=%ld \n", diff_cp);
	}
	else
	{
	diff_cp = (cp_cal_mean-*calibrationValue);
	DLOG_INFO("diff_cp=%ld \n", diff_cp);
	}

	if( diff_cp>CP_MAX_DEVIATION )
	{
	DLOG_INFO("The Calibration Value Error \n");
	return FALSE;
	}

	DLOG_INFO("calibrationValue = %ld MHz\n", *calibrationValue);

	return TRUE;
}


////////////////////////////////////////////////////////////////////
//static unsigned long CP_OCLK( void )
//
//
//
////////////////////////////////////////////////////////////////////
static unsigned long CP_OCLK( void )
{
	unsigned char  SIP_00B;
	unsigned char SIP_01B;
	unsigned char SIP_02B;
	unsigned char CP_100ms_00B;
	unsigned char CP_100ms_01B;
	unsigned char CP_100ms_02B;
	unsigned long rddata;

	hdmirxwr(0x0F, 0x00);
	hdmirxwr(0x73, 0x05);
	hdmirxwr(0x0F, 0x01);
	hdmirxwr(0x57, 0x12);
	hdmirxwr(0x57, 0x02);
	hdmirxwr(0x56, 0x01);

	// set Read Address and enable
	hdmirxwr(0x50, 0x00);
	hdmirxwr(0x51, 0x00);
	hdmirxwr(0x53, 0x04);
	SIP_00B=hdmirxrd(0x61);
	SIP_01B=hdmirxrd(0x61);
	SIP_02B=hdmirxrd(0x61);

	DLOG_INFO("SIP_00B=%X \n",(int)SIP_00B);
	DLOG_INFO("SIP_01B=%X \n",(int)SIP_01B);
	DLOG_INFO("SIP_02B=%X \n",(int)SIP_02B);

	if( SIP_00B==0x02 && SIP_01B==0x02 && SIP_02B==0x02 )
	{
		hdmirxwr(0x50, 0x01);
		hdmirxwr(0x51, 0x60);
		hdmirxwr(0x53, 0x04);
		CP_100ms_00B=hdmirxrd(0x61);
		CP_100ms_01B=hdmirxrd(0x61);
		CP_100ms_02B=hdmirxrd(0x61);

		// disable -> if(CP_100ms_02B > 0x60 || CP_100ms_02B < 0x40)
		// disable -> {
		// disable -> DLOG_INFO("SIPROM CP 100MS Error \n");
		// disable -> rddata = 0;
		// disable -> }
		// disable -> else
		{
			rddata =(unsigned long) (CP_100ms_00B) + (CP_100ms_01B<<8) + (CP_100ms_02B<<16);
			DLOG_INFO("SIPROM CP 100MS %x \r\n",rddata);
		}
	}
	else if( SIP_00B==0xFF && SIP_01B==0x00 && SIP_02B==0xFF )
	{
		hdmirxwr(0x50, 0x03);
		hdmirxwr(0x51, 0x60);
		hdmirxwr(0x53, 0x04);
		CP_100ms_00B=hdmirxrd(0x61);
		CP_100ms_01B=hdmirxrd(0x61);
		CP_100ms_02B=hdmirxrd(0x61);
		// disable -> if(CP_100ms_02B > 0x60 || CP_100ms_02B < 0x40)
		// disable -> {
		// disable -> DLOG_INFO("SIPROM CP 100MS Error \n");
		// disable -> rddata = 0;
		// disable -> }
		// disable -> else
		{
			rddata =(unsigned long) (CP_100ms_00B) + (CP_100ms_01B<<8) + (CP_100ms_02B<<16);
			DLOG_INFO("SIPROM CP 100MS %x \r\n",rddata);
		}
	}
	else
	{
		DLOG_INFO("SIPROM CP 100MS Error \n");
		rddata = 0;
	}

	hdmirxwr(0x0F, 0x00);
	hdmirxwr(0x73, 0x00);

	DLOG_INFO("CP_100ms_00B=%X \n",(int)CP_100ms_00B);
	DLOG_INFO("CP_100ms_01B=%X \n",(int)CP_100ms_01B);
	DLOG_INFO("CP_100ms_02B=%X \n",(int)CP_100ms_02B);

	return rddata;
}

//FIX_ID_004 xxxxx //Add 100ms calibration for Cbus
//#ifndef _SelectExtCrystalForCbus_
////////////////////////////////////////////////////////////////////
//void cal_oclk( void )
//
//
//
////////////////////////////////////////////////////////////////////
static void Cal_oclk( void )
{
#if 0 // disable Cbus Calibration of MCU
	unsigned char i;
	unsigned long rddata;
#endif
	unsigned char oscdiv;
	unsigned long OSCCLK;
	unsigned long u32_CEC;
	unsigned long sum=0;

	//float m _ROSCCLK;
	unsigned int t10usint, t10usflt;

#if 0 // disable Cbus Calibration of MCU
	// 100ms OSC calibartion by MCU
	for(i=0; i<1; i++)
	{
//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
		mhlrxset(MHL_RX_01,0x01, 0x01);
		IT_Delay(99);
		mhlrxset(MHL_RX_01,0x01, 0x00);
//FIX_ID_037 xxxxx

		rddata = (unsigned long)mhlrxrd(MHL_RX_12);
		rddata += (unsigned long)mhlrxrd(MHL_RX_13)<<8;
		rddata += (unsigned long)mhlrxrd(MHL_RX_14)<<16;

		sum += rddata;
	}
	DLOG_INFO("loop=%d, cnt=%ld  sum =%ld \r\n",(int) i, rddata,sum);

	sum /= i;


	OSCCLK = (unsigned long) sum;
	DLOG_INFO("MCU calculate 100ms = %ld MHz \r\n", OSCCLK);
	if(OSCvalueCompare(&OSCCLK)==FALSE)
#endif

	{
		OSCCLK = CP_OCLK();
		DLOG_INFO("CP_OSCCLK = %ld MHz \r\n", OSCCLK);
		if(OSCvalueCompare(&OSCCLK)==FALSE)
		{
			OSCCLK = CP_MEAN_VALUE;
			//OSCCLK = (unsigned long) sum;
			//OSCCLK /= 100;
			DLOG_INFO("Use CP_MEAN_VALUE = %ld MHz \r\n", OSCCLK);
		}
		else
		{
			DLOG_INFO("Use CP Calibration Value \r\n");
		}
	}
#if 0 // disable Cbus Calibration of MCU
	else
	{
		DLOG_INFO("Use MCU calculate OSC Value = %ld MHz\n", OSCCLK);
	}
#endif

	DLOG_INFO("OSCCLK=%ld MHz \r\n", OSCCLK/1000);
	oscdiv = (unsigned char) (OSCCLK/1000/10);

	if( ((OSCCLK/1000/oscdiv)-10)>(10-(OSCCLK/1000/(oscdiv+1))) )
	{
		oscdiv++;
	}
//GreenText;
	DLOG_INFO("!!! oscdiv = %d !!!\r\n", (int) oscdiv);
//WhiteText;
	mhlrxset(MHL_RX_01, 0x70, oscdiv<<4);


	DLOG_INFO("OCLK=%ld MHz \r\n", OSCCLK/1000/oscdiv);

	u32_CEC = OSCCLK*100;
	u32_CEC = u32_CEC>>8;
	u32_CEC /= 0xFB;
	DLOG_INFO("u32_CEC = %X \r\n",(int) u32_CEC);


	mhlrxset(MHL_RX_29,0x83,0x83);	//[7] 1: use 27M crystall ( _SelectExtCrystalForCbus_ )

	if( RCLKFreqSel==0 )
	m_ROSCCLK = OSCCLK/2;   // 20 MHz
	else if( RCLKFreqSel==1 )
	m_ROSCCLK = OSCCLK/4;   // 10 MHz
	else if( RCLKFreqSel==2 )
	m_ROSCCLK = OSCCLK/8;
	else
	m_ROSCCLK = OSCCLK/16;

	//RCLK*=1.1;    // add for CTS (RCLK*=1.1)

	t10usint = m_ROSCCLK/100;
	t10usflt = (m_ROSCCLK/100 - t10usint)*128;

	DLOG_INFO("RCLK=%ld MHz\n", m_ROSCCLK/1000);
	DLOG_INFO("T10usInt=0x%X, T10usFlt=0x%X \r\n",(int) t10usint,(int) t10usflt);
	mhlrxwr(0x02, t10usint&0xFF);
	mhlrxwr(0x03, ((t10usint&0x100)>>1)+t10usflt);
	DLOG_INFO("MHL reg02=%X, reg03=%X \r\n",(int)mhlrxrd(0x02),(int) mhlrxrd(0x03));
	//     RCLK/=1.1;    // add for CTS (RCLK*=1.1)
	DLOG_INFO("\n");


}
//FIX_ID_021 xxxxx
//#endif
#endif


#endif


#ifdef _SUPPORT_EDID_RAM_
/*****************************************************************************/
/* EDID RAM  functions    *******************************************************/
/*****************************************************************************/

//static unsigned char UpdateEDIDRAM(_CODE unsigned char *pEDID,unsigned char BlockNUM)
static unsigned char UpdateEDIDRAM(unsigned char *pEDID,unsigned char BlockNUM)
{
	unsigned char  i,offset,sum =0;

	if ( BlockNUM == 0x02 )
		offset = 0x00+128*0x01;
	else
		offset = 0x00+128*BlockNUM;

	DLOG_INFO("block No =%02X offset = %02X \n",(int) BlockNUM,(int) offset);


	for(i=0;i<0x7F;i++)
	{
		EDID_RAM_Write(offset,1 ,(pEDID+offset));

		DLOG_INFO("%02X ",(int) *(pEDID+offset));
		sum += *(pEDID+offset);
		offset ++;
//		pEDID++;
/*		MZY 17/5/5
	        if((i % 16) == 15)
	        {
	        DLOG_INFO("\r\n");
	        }

*/
	}
	sum = 0x00- sum;
//    EDID_RAM_Write(offset,1,&sum);
	return 	sum;
}

static void EnableEDIDupdata(void)
{
	DLOG_INFO("EnableEDIDupdata() \n");

//	HotPlug(0);	//clear port 1 HPD=0 for Enable EDID update

//	chgbank(1);
//	hdmirxset(REG_RX_1B0, 0x03, 0x01); //clear port 0 HPD=1 for EDID update
//	chgbank(0);

	it6602HPDCtrl(0,0);	// HDMI/MHL port 0, set HPD = 0
	it6602HPDCtrl(1,0);	// HDMI port 1, set HPD = 0
}

static void DisableEDIDupdata(void)
{
	DLOG_INFO("DisableEDIDupdata() \n");

//xxxxx 2013-0801 disable HPD Control
#if 0
	HotPlug(1);	//set port 1 HPD=1

	chgbank(1);
	hdmirxset(REG_RX_1B0, 0x03, 0x03); //set port 0 HPD=1
	chgbank(0);
#endif
//xxxxx
}


//static void EDIDRAMInitial(_CODE unsigned char *pIT6602EDID)
static void EDIDRAMInitial(unsigned char *pIT6602EDID)
{

	unsigned char Block0_CheckSum;
	unsigned char Block1_CheckSum;
	unsigned char u8_VSDB_Addr;
	unsigned char BlockNo;

	u8_VSDB_Addr=0;

	EnableEDIDupdata();

	for(BlockNo=0;BlockNo<2;BlockNo++){

	DLOG_INFO("IT6602 EDIDRAMInitial = %02X\n", (int) BlockNo);

		if(BlockNo==0)
		{
			Block0_CheckSum =  UpdateEDIDRAM(pIT6602EDID,0);
			hdmirxwr(REG_RX_0C4,Block0_CheckSum);		//Port 0 Bank 0 CheckSum
			hdmirxwr(REG_RX_0C8,Block0_CheckSum);		//Port 1 Bank 0 CheckSum

			DLOG_INFO(" Block0_CheckSum = %02X\n", (int) Block0_CheckSum);
		}
		else
		{
			Block1_CheckSum =  UpdateEDIDRAM(pIT6602EDID,1);
			DLOG_INFO(" Block1_CheckSum = %02X\n", (int) Block1_CheckSum);
			u8_VSDB_Addr=Find_Phyaddress_Location(pIT6602EDID,1);

			DLOG_INFO("u8_VSDB_Addr = %02X\n", (int) u8_VSDB_Addr);
			PhyAdrSet();

			if(u8_VSDB_Addr!=0)
			{

				UpdateEDIDReg(u8_VSDB_Addr, pIT6602EDID[u8_VSDB_Addr],pIT6602EDID[u8_VSDB_Addr+1], Block1_CheckSum);
				DLOG_INFO("EDID Parsing OK\n");
			}
		}
	}

	DisableEDIDupdata();
}


//static unsigned char Find_Phyaddress_Location(_CODE unsigned char *pEDID,unsigned char Block_Number)
static unsigned char Find_Phyaddress_Location(unsigned char *pEDID,unsigned char Block_Number)
{
	unsigned char AddStart;
	unsigned char tag, count;
	unsigned char offset,End;
	unsigned char u8_VSDB_Addr;

#ifdef FIX_ID_013_
//FIX_ID_013	xxxxx	//For MSC 3D request issue
	unsigned char u8_3DPresent_Addr;
	unsigned char ucTemp;
	struct PARSE3D_STR *pstParse3D = get_EDID_VSDB_3Ddata();
//FIX_ID_013	xxxxx
#endif //FIX_ID_013

	if ( Block_Number == 0x02 )
		AddStart = 0x00+128*0x01;
	else
		AddStart = 0x00+128*Block_Number;

	if((*(pEDID+AddStart))!=0x2 || (*(pEDID+AddStart+1))!=0x3)
		return 0;
	End = (*(pEDID+AddStart+2));
	u8_VSDB_Addr=0;

#ifdef FIX_ID_013_
//FIX_ID_013	xxxxx	//For MSC 3D request issue
	// initial value then check with SVD and VSDB block to find the SVD of 3D support timing
	pstParse3D->bVSDBspport3D = 0x00;
	pstParse3D->ucVicCnt=0;
//FIX_ID_013	xxxxx
#endif //FIX_ID_013

	for(offset=(AddStart+0x04);offset<(AddStart+End); )
	{


		tag=(*(pEDID+offset))>>5;
		count=(*(pEDID+offset)) & 0x1f;

		//#ifdef printf_EDID
		DLOG_INFO("offset = %X , Tag = %X , count =%X \n", (int) offset,(int)  tag, (int) count);
		//#endif

		offset++;
	if(tag==0x03)	// HDMI VSDB Block of EDID
        {
        	//#ifdef printf_EDID
	        	DLOG_INFO("HDMI VSDB Block address = %X\n",(int)  offset);
        	//#endif

			if(	(*(pEDID+offset  ))==0x03 &&
				(*(pEDID+offset+1))==0x0C &&
				(*(pEDID+offset+2))==0x0    )
			{
				u8_VSDB_Addr=offset+3;
				txphyadr[0]=(*(pEDID+offset+3));
				txphyadr[1]=(*(pEDID+offset+4));
				//#ifdef printf_EDID
				DLOG_INFO("txphyadr[0] = %X\n",(int)  txphyadr[0]);
				DLOG_INFO("txphyadr[1] = %X\n",(int)  txphyadr[1]);
				//#endif

#ifdef FIX_ID_013_
//FIX_ID_013	xxxxx	//For MSC 3D request issue

				if(count < 7)		// no 3D support !!!
					return u8_VSDB_Addr;

				u8_3DPresent_Addr = offset+7;

				ucTemp = *(pEDID+offset+7);

				if(ucTemp & 0x80)				// Video and Audio Latency present
					u8_3DPresent_Addr += 2;

				if(ucTemp & 0x40) 			// Interlaced Video and Audio Latency present
					u8_3DPresent_Addr += 2;

				if(ucTemp & 0x20)				// HDMI additional video format present
				{
					 u8_3DPresent_Addr++;
				}
				pstParse3D->uc3DEdidStart = u8_3DPresent_Addr;

				pstParse3D->uc3DBlock = Block_Number;

				pstParse3D->bVSDBspport3D = 0x01;		// for identify the HDMI VSDB 3D support
//FIX_ID_013	xxxxx
#endif //FIX_ID_013
				return u8_VSDB_Addr;
			}
		}

#ifdef FIX_ID_013_
//FIX_ID_013	xxxxx	//For MSC 3D request issue

	        if(tag==0x02)	// Short Video Descriptor of EDID
       	 {
	        	//#ifdef printf_EDID
	        	DLOG_INFO("Short Video Descriptor Address = %X, VIC count = %X \r\n",(int)  offset,(int) count);
	        	//#endif

			// get the SVD size
			pstParse3D->ucVicCnt = count;

			for(ucTemp=0;ucTemp<count;ucTemp++)
			{
				u8_3DPresent_Addr = (*(pEDID+offset+ucTemp) ) & 0x7F;
				SVD_LIST[ucTemp] = u8_3DPresent_Addr;
				DLOG_INFO("SVD[%X] = %X\n", ucTemp,u8_3DPresent_Addr);
			}
		 }
//FIX_ID_013	xxxxx
#endif //FIX_ID_013

		offset=offset+count;
	}
    return 0;
}



static void UpdateEDIDReg(unsigned char u8_VSDB_Addr, unsigned char CEC_AB, unsigned char CEC_CD, unsigned char Block1_CheckSum)
{

    unsigned char  A_Addr_AB, A_Addr_CD, A_Block1_CheckSum;
    unsigned char  B_Addr_AB, B_Addr_CD, B_Block1_CheckSum;

	A_Addr_AB=rxphyadr[0][0];
	A_Addr_CD=rxphyadr[0][1];

	B_Addr_AB=rxphyadr[1][0];
	B_Addr_CD=rxphyadr[1][1];


	A_Block1_CheckSum=(Block1_CheckSum+CEC_AB+CEC_CD -A_Addr_AB-A_Addr_CD)%0x100;
	B_Block1_CheckSum=(Block1_CheckSum+CEC_AB+CEC_CD -B_Addr_AB-B_Addr_CD)%0x100;


	hdmirxwr(REG_RX_0C1,u8_VSDB_Addr);			//VSDB Start Address
	hdmirxwr(REG_RX_0C2,A_Addr_AB);					//Port 0 AB
	hdmirxwr(REG_RX_0C3,A_Addr_CD);				//Port 0 CD
	hdmirxwr(REG_RX_0C5,A_Block1_CheckSum);		//Port 0 Bank 1 CheckSum

	hdmirxwr(REG_RX_0C6,B_Addr_AB);					//Port 1 AB
	hdmirxwr(REG_RX_0C7,B_Addr_CD);				//Port 1 CD
	hdmirxwr(REG_RX_0C9,B_Block1_CheckSum);		//Port 1 Bank 1 CheckSum



}
static void PhyAdrSet(void)
{

#if 0
//only for HDMI switch
	txphyA = (txphyadr[0]&0xF0)>>4;
	txphyB = (txphyadr[0]&0x0F);
	txphyC = (txphyadr[1]&0xF0)>>4;
	txphyD = (txphyadr[1]&0x0F);

	if( txphyA==0 && txphyB==0 && txphyC==0 && txphyD==0 )
		txphylevel = 0;
	else if( txphyB==0 && txphyC==0 && txphyD==0 )
		txphylevel = 1;
	else if( txphyC==0 && txphyD==0 )
		txphylevel = 2;
	else if( txphyD==0 )
		txphylevel = 3;
	else
		txphylevel = 4;

	rxcurport = 0;
	switch( txphylevel )
	{
		case 0:
			rxphyadr[0][0] = 0x10;
			rxphyadr[0][1] = 0x00;
			rxphyadr[1][0] = 0x20;
			rxphyadr[1][1] = 0x00;
			break;
		case 1:
			rxphyadr[0][0] = (txphyA<<4)+0x01;
			rxphyadr[0][1] = 0x00;
			rxphyadr[1][0] = (txphyA<<4)+0x02;
			rxphyadr[1][1] = 0x00;
			break;
		case 2:
			rxphyadr[0][0] = txphyadr[0];
			rxphyadr[0][1] = 0x10;
			rxphyadr[1][0] = txphyadr[0];
			rxphyadr[1][1] = 0x20;
			break;
		case 3:
			rxphyadr[0][0] = txphyadr[0];
			rxphyadr[0][1] = (txphyC<<4)+0x01;
			rxphyadr[1][0] = txphyadr[0];
			rxphyadr[1][1] = (txphyC<<4)+0x02;
			break;
		default:
			rxphyadr[0][0] = 0xFF;
			rxphyadr[0][1] = 0xFF;
			rxphyadr[1][0] = 0xFF;
			rxphyadr[1][1] = 0xFF;
			break;
	}
	DLOG_INFO("\nDnStrm PhyAdr = %X, %X, %X, %X\n", (int) txphyA,(int)  txphyB, (int) txphyC, (int) txphyD);

	rxphyA = (rxphyadr[0][0]&0xF0)>>4;
	rxphyB = (rxphyadr[0][0]&0x0F);
	rxphyC = (rxphyadr[0][1]&0xF0)>>4;
	rxphyD = (rxphyadr[0][1]&0x0F);
	DLOG_INFO(" PortA PhyAdr = %X, %X, %X, %X\n",(int)  rxphyA, (int) rxphyB, (int) rxphyC,(int)  rxphyD);

	rxphyA = (rxphyadr[1][0]&0xF0)>>4;
	rxphyB = (rxphyadr[1][0]&0x0F);
	rxphyC = (rxphyadr[1][1]&0xF0)>>4;
	rxphyD = (rxphyadr[1][1]&0x0F);
	DLOG_INFO(" PortB PhyAdr = %X, %X, %X, %X\n", (int) rxphyA,(int)  rxphyB,(int)  rxphyC,(int)  rxphyD);
#endif

//#ifdef Enable_IT6602_CEC
//	if(Myself_LogicAdr==DEVICE_ID_TV)
//#endif
	{
		rxphyadr[0][0] = 0x10;
		rxphyadr[0][1] = 0x00;
		rxphyadr[1][0] = 0x20;
		rxphyadr[1][1] = 0x00;

	}



}

#ifdef FIX_ID_013_
//FIX_ID_013	xxxxx	//For MSC 3D request issue
#ifdef  _SUPPORT_EDID_RAM_
static struct PARSE3D_STR* get_EDID_VSDB_3Ddata(void)
{
	return &st3DParse;
}

static void EDID_ParseVSDB_3Dblock(struct PARSE3D_STR *pstParse3D)
{

	unsigned char	ucTemp;
	unsigned char	uc3DMulti;
	unsigned char	uc3DEdidEnd = 0xFF;
	unsigned char	ucRdPtr = pstParse3D->uc3DEdidStart;

	PARSE3D_STA	e3DEdidState = PARSE3D_START;


	//check with HDMI VSDB block of EDID
	if(pstParse3D->bVSDBspport3D == 0x00)
	{
		pstParse3D->ucVicCnt=0;
		return;
	}


	// Re-initial bVSDBspport3D =0 then check with 3D_Structure and 3D_MASK at HDMI VSDB block of EDID
	pstParse3D->bVSDBspport3D = 0x00;


	DLOG_INFO("***   EDID_ParseVSDB_3Dblock   *** \r\n");
	DLOG_INFO("MHL 3D [2]LR [1]TB [0]FS \r\n");

	if(ucRdPtr ==0)
		return;

		for(;ucRdPtr <= uc3DEdidEnd;){

			// disable -> 	DLOG_INFO("Default_Edid_Block[%x]=0x%x\n",(int) ucRdPtr,(int) Default_Edid_Block[ucRdPtr]);

			// disable -> 	for(ucTemp=0; ucTemp<16; ucTemp++)
			// disable -> 	{
			// disable -> 		DLOG_INFO("%x ", (int)pstParse3D->uc3DInfor[ucTemp]);
			// disable -> 	}

			switch(e3DEdidState){
					case	PARSE3D_START:
							uc3DMulti = Default_Edid_Block[ucRdPtr++];

							if(uc3DMulti & 0x80){
								uc3DMulti &= 0x60;
								e3DEdidState = PARSE3D_LEN;
							}else{
								return;
							}
						break;
					case	PARSE3D_LEN:
							uc3DEdidEnd = (Default_Edid_Block[ucRdPtr] >> 5) + (Default_Edid_Block[ucRdPtr] & 0x1F) +ucRdPtr;
							ucRdPtr += (Default_Edid_Block[ucRdPtr] >> 5) + 1;
							e3DEdidState = PARSE3D_STRUCT_H;
						break;
					case	PARSE3D_STRUCT_H:
							switch(uc3DMulti){
								case	0x20:
								case	0x40:
										if(Default_Edid_Block[ucRdPtr++] & 0x01){
											uc3DMulti |= 0x04;
										}
										e3DEdidState = PARSE3D_STRUCT_L;
									break;
								default:
										e3DEdidState = PARSE3D_VIC;
									break;
							}
						break;
					case	PARSE3D_STRUCT_L:
							ucTemp = Default_Edid_Block[ucRdPtr++];
							if(ucTemp & 0x40)
								uc3DMulti |= 0x02;
							if(ucTemp & 0x01)
								uc3DMulti |= 0x01;


							if((uc3DMulti & 0x60) == 0x20){
								e3DEdidState = PARSE3D_VIC;
								uc3DMulti &= 7;

								for(ucTemp=0; ucTemp<16; ucTemp++){
									pstParse3D->uc3DInfor[ucTemp] = uc3DMulti;
									DLOG_INFO("VSD[%d]=0x%x \r\n", (int)ucTemp,(int) uc3DMulti);
								}

							}else{
								e3DEdidState = PARSE3D_MASK_H;
								uc3DMulti &= 7;
							}
						break;
					case	PARSE3D_MASK_H:

							if(Default_Edid_Block[ucRdPtr])
								pstParse3D->bVSDBspport3D = 0x01;	//for identify 3D_MASK have Short Video Descriptor (SVD) support 3D format

							for(ucTemp=0; ucTemp<8; ucTemp++){
								if(Default_Edid_Block[ucRdPtr] & (1<<ucTemp)){
									pstParse3D->uc3DInfor[ucTemp+8] = uc3DMulti;
									DLOG_INFO("VSD[%d]=0x%x \r\n",(int) ucTemp+8,(int) uc3DMulti);
								}else{
									pstParse3D->uc3DInfor[ucTemp+8] = 0;
								}
							}
							ucRdPtr++;
							e3DEdidState = PARSE3D_MASK_L;
						break;
					case	PARSE3D_MASK_L:

							if(Default_Edid_Block[ucRdPtr])
								pstParse3D->bVSDBspport3D = 0x01;	//for identify 3D_MASK have SVD support 3D format

							for(ucTemp=0; ucTemp<8; ucTemp++){
								if(Default_Edid_Block[ucRdPtr] & (1<<ucTemp)){
									pstParse3D->uc3DInfor[ucTemp] = uc3DMulti;
									DLOG_INFO("VSD[%d]=0x%x \r\n", (int)ucTemp, (int)uc3DMulti);
								}else{
									pstParse3D->uc3DInfor[ucTemp] = 0;
								}
							}
							ucRdPtr++;
							e3DEdidState = PARSE3D_VIC;
						break;
					case	PARSE3D_VIC:
// disable ->							ucTemp = Default_Edid_Block[ucRdPtr]>>4;
// disable ->							if(pstParse3D->ucVicCnt > ucTemp){
// disable ->								pstParse3D->uc3DInfor[ucTemp] |= STRUCTURE_3D[Default_Edid_Block[ucRdPtr] & 0xF];
// disable ->								DLOG_INFO("Vic[%d]=0x%x\n", ucTemp, STRUCTURE_3D[Default_Edid_Block[ucRdPtr] & 0xF]);
// disable ->							}
// disable ->							if(Default_Edid_Block[ucRdPtr] & 0x8 )
// disable ->							ucRdPtr+=2;
// disable ->							else
								ucRdPtr+=1;
						break;
					default:
						break;
				}
		}
}
#endif
//FIX_ID_013	xxxxx
#endif //FIX_ID_013


#endif


//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_

#ifdef _SUPPORT_RCP_
/*****************************************************************************/
/* RCP functions    *******************************************************/
/*****************************************************************************/

static void RCPinitQ(struct it6602_dev_data *it6602)
{
	it6602->RCPhead = 0;
	it6602->RCPtail = 0;
	it6602->RCPResult = RCP_Result_Finish;
}

void RCPKeyPush(unsigned char ucKey)
{
	unsigned char	i;

	struct it6602_dev_data *it6602 = get_it6602_dev_data();

	// IF buffer is full , can't put data
	if( ( it6602->RCPhead % MAXRCPINDEX ) == ( ( it6602->RCPtail+1 ) % MAXRCPINDEX ) )
	{
		DLOG_INFO("RCPKeyPush Full !!! \r\n");
		SwitchRCPStatus(it6602,RCP_Transfer);
		return;
	}

	// put in buffer
	it6602->RCPtail += 1;
	i=it6602->RCPtail % MAXRCPINDEX;
	it6602->RCPTxArray[i]=ucKey;			// push press
//FIX_ID_024	xxxxx	//Fixed for RCP compliance issue
	if((it6602->m_bRCPTimeOut )==FALSE)
	{
	// put in buffer
	it6602->RCPtail += 1;
	i=it6602->RCPtail % MAXRCPINDEX;
	it6602->RCPTxArray[i]=ucKey|0x80;		// push release
	}
//FIX_ID_024	xxxxx

	SwitchRCPStatus(it6602,RCP_Transfer);

}

static int RCPKeyPop(struct it6602_dev_data *it6602)
{

	// is Empty
	if( (it6602->RCPhead % MAXRCPINDEX) == (it6602->RCPtail % MAXRCPINDEX) )
	{
		DLOG_INFO("RCPKeyPop Empty !!! \r\n");
		return FAIL;
	}

	it6602->RCPhead += 1;

	it6602->txmsgdata[0]=MSG_RCP;
	it6602->txmsgdata[1]=it6602->RCPTxArray[ it6602->RCPhead % MAXRCPINDEX ];	//tx new key to peer

	DLOG_INFO("RCPKeyPop() key = %X \n",(int) it6602->txmsgdata[1]);

//FIX_ID_024	xxxxx	//Fixed for RCP compliance issue
	if(((it6602->m_bRCPTimeOut )==TRUE) || ((it6602->m_bRCPError )==TRUE))
	{
		if(it6602->txmsgdata[1]&0x80)	// since SamSung Note did not check bit7 press/release bit. so skip release key for SamSung Note
		{
			it6602->m_bRCPError = FALSE;
			DLOG_INFO("### skip release key for SamSung Note  ###\r\n");
			return SUCCESS;
		}
	}
//FIX_ID_024	xxxxx
	cbus_send_mscmsg(it6602);
	SwitchRCPResult(it6602,RCP_Result_Transfer);

//FIX_ID_024	xxxxx	//Fixed for RCP compliance issue
	WaitRCPresponse(it6602);
//FIX_ID_024	xxxxx
	return SUCCESS;
}


//FIX_ID_024	xxxxx	//Fixed for RCP compliance issue
static void WaitRCPresponse(struct it6602_dev_data *it6602)
{
	unsigned char cbuswaitcnt=0;
	unsigned char MHL04;

	//if(it6602->RCPResult == RCP_Result_Transfer)
	{
		do
		{
			cbuswaitcnt++;
			// DLOG_INFO("IT6602-SwitchRCPStatus() MSC RX MSC_MSG Interrupt ...\n");
			IT_Delay(CBUSWAITTIME);
			MHL04 = mhlrxrd(MHL_RX_04);
			if( MHL04&0x10 )
			{
				mhlrxwr(MHL_RX_04,0x10);
				mhl_read_mscmsg(it6602 );
				break;
			}
		}
		while(cbuswaitcnt<CBUSWAITNUM);
	}
}
//FIX_ID_024	xxxxx

static void SwitchRCPResult(struct it6602_dev_data *it6602,RCPResult_Type RCPResult)
{
	it6602->RCPResult = RCPResult;
	//FIX_ID_015	xxxxx peer device no response
	it6602->RCPCheckResponse=0;	//xxxxx 2013-0806
	//FIX_ID_015	xxxxx
	switch(RCPResult)
	{
		case RCP_Result_OK:		DLOG_INFO("RCP_Result_OK \n");   break;
		case RCP_Result_FAIL:		DLOG_INFO("RCP_Result_FAIL \n");   break;
		case RCP_Result_ABORT:	DLOG_INFO("RCP_Result_ABORT \n");   break;
		case RCP_Result_Transfer:	DLOG_INFO("RCP_Result_Transfer \n");   break;
		case RCP_Result_Finish:	DLOG_INFO("RCP_Result_Finish \n");   break;
		case RCP_Result_Unknown:	break;

	}
}

static void SwitchRCPStatus(struct it6602_dev_data *it6602,RCPState_Type RCPState)
{
	it6602->RCPState = RCPState;
}

static void RCPManager(struct it6602_dev_data *it6602)
{

	switch(it6602->RCPState)
	{

		case RCP_Transfer:	//Send New RCP key
			{
				if(it6602->RCPResult == RCP_Result_Finish)
				{
					if(RCPKeyPop(it6602)==FAIL)
						SwitchRCPStatus(it6602,RCP_Empty);
				}
				//FIX_ID_015	xxxxx peer device no response
				else
				{
					//xxxxx 2013-0806 peer device no response
					if(it6602->RCPCheckResponse++>6)	// 6 x 50ms = 300ms --> MHL spec
					{
//FIX_ID_024	xxxxx	//Fixed for RCP compliance issue
						it6602->m_bRCPTimeOut = TRUE;
//FIX_ID_024	xxxxx
						DLOG_INFO("\n\n### RCP Timeout, Peer device no RCPE or RCPK response  ###\r\n");
						//FIX_ID_024 xxxxx disable -> cbus_send_mscmsg(it6602);
						SwitchRCPResult(it6602,RCP_Result_Finish);
					}

				}
				//FIX_ID_015	xxxxx
			}
			break;

		default :
			break;
	}
}

#endif

//FIX_ID_005 xxxxx	//Add Cbus Event Handler
static void IT6602CbusEventManager(struct it6602_dev_data *it6602)
{
	unsigned char CbusStatus;

	//for debug only !!!
	//DLOG_INFO(" it6602->CBusIntEvent ...%X...%X \n",it6602->CBusIntEvent,it6602->CBusSeqNo);
	//for debug only !!!

	if(it6602->CBusIntEvent !=0)
	{

		CbusStatus = mhlrxrd(MHL_RX_1C);
		if((CbusStatus & 0x07)==0)
		{

		//for debug only !!!
		//TickCountPrint();
		//DLOG_INFO(" it6602->CBusIntEvent ...%X...%X \n",it6602->CBusIntEvent,it6602->CBusSeqNo);
		//for debug only !!!

//============================================================================

			if((it6602->CBusIntEvent & B_3DSupporpt) == B_3DSupporpt)
			{


			       // DLOG_INFO("MHL GNT_WRT Interrupt (TxSeqNum=%d) ...\n", TxWrBstSeq);
				// disable --> if( EnMHL3DSupport==TRUE ) {
				// disable -->     if( V3D_EntryCnt==0 )
				// disable -->         v3d_burst1st(it6602);
				// disable -->     else if( V3D_EntryCnt==1 )
				// disable -->		      v3d_burst2nd(it6602);
				// disable -->     else if( V3D_EntryCnt==2 )
				// disable -->		      v3d_burst3rd(it6602);
				// disable -->	 else if( V3D_EntryCnt==3 ) {
				// disable -->		      v3d_burst4th(it6602);
				// disable -->	 		  DLOG_INFO(" ### 3D supporpt Write_Burst End ###\n");
				// disable -->	 }
				// disable --> }
				// disable --> else {
				// disable -->	 if( V3D_EntryCnt==0 )
				// disable -->		 v3d_unsupport1st(it6602);
				// disable -->	 else if( V3D_EntryCnt==1 ) {
				// disable -->		      v3d_unsupport2nd(it6602);
				// disable -->		      DLOG_INFO(" ### 3D Un-Support Write_Burst End ###\n");
				// disable -->	 }
				// disable -->
				// disable --> }
				// disable --> it6602->CBusIntEvent &= ~(B_3DSupporpt);	// finish MSC
				//FIX_ID_037 xxxxx //2014-0522 MHL compliance issue !!!
				it6602->CBusWaitNo =0 ;
				//FIX_ID_037 xxxxx
#ifdef FIX_ID_013_
//FIX_ID_013	xxxxx	//For MSC 3D request issue
				if(e3DReqState != MHL3D_REQ_DONE)
				{
					Msc_3DProcess(&e3DReqState);
				}
				else
				{
					it6602->CBusIntEvent &= ~(B_3DSupporpt);	// finish MSC
				}
//FIX_ID_013	xxxxx	//For MSC 3D request issue
#else
				if( V3D_EntryCnt==0 )
					v3d_unsupport1st(it6602);
					else if( V3D_EntryCnt==1 ) {
					v3d_unsupport2nd(it6602);
					it6602->CBusIntEvent &= ~(B_3DSupporpt);	// finish MSC
					DLOG_INFO(" ### 3D Un-Support Write_Burst End ###\n");
				}
#endif //FIX_ID_013

			}
//============================================================================
			if((it6602->CBusIntEvent & B_MSC_Waiting) == B_MSC_Waiting)
			{

				if(it6602->CBusWaitNo == 0)
				{
					it6602->CBusIntEvent &= ~(B_MSC_Waiting);
					DLOG_INFO("waiting B_MSC_Waiting  OK ...\n");
				}
				else
				{
					it6602->CBusWaitNo--;
					DLOG_INFO("waiting B_MSC_Waiting  %X ...\n",(int) it6602->CBusWaitNo);
				}
				return;
			}

//============================================================================
			if((it6602->CBusIntEvent & B_DiscoveryDone) == B_DiscoveryDone)
			{
				switch(it6602->CBusSeqNo)
				{
					case 0:	CbusStatus = mscFire(MHL_MSC_CtrlPacket0, B_SET_HPD);
							if(CbusStatus != SUCCESS)
							{
								DLOG_INFO("Set B_SET_HPD Need to retry ...\n");
								return ;
							}
							it6602->CBusSeqNo=1;
							// break;

							//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
							m_MHLabortID++;
							//FIX_ID_037 xxxxx
					case 1:
					//FIX_ID_011 xxxx	//Use FW send PATH_EN{Sink}=1
							mhlrxset(MHL_RX_0C, 0x02, 0x02);	 //MHL0C[1] FW send PATH_EN{Sink}=1
							//2013-0905 change PATH_EN method ->CbusStatus = set_mhlsts(MHLSts01B, PATH_EN);
							//2013-0905 change PATH_EN method ->if(CbusStatus != SUCCESS)
							//2013-0905 change PATH_EN method ->{
							//2013-0905 change PATH_EN method ->DLOG_INFO("Set PATH_EN Need to retry ...\n");
							//2013-0905 change PATH_EN method ->return ;
							//2013-0905 change PATH_EN method ->}

							//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
							CbusStatus = mscCheckResult();
							if(CbusStatus != SUCCESS)
							{
								// GreenText;
								DLOG_INFO("Set PATH_EN Need to retry ...\n");
								// WhiteText;
								return ;
							}
							//FIX_ID_037 xxxxx

							it6602->CBusSeqNo=2;
							//break;

							//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
							m_MHLabortID++;
							//FIX_ID_037 xxxxx

					//FIX_ID_011 xxxx
					case 2:	CbusStatus = set_mhlsts(MHLSts00B, DCAP_RDY);
							if(CbusStatus != SUCCESS)
							{
								DLOG_INFO("Set DCAP_RDY Need to retry ...\n");
								return ;
							}
							it6602->CBusIntEvent &= ~(B_DiscoveryDone);
							it6602->CBusSeqNo=0x00;
							DLOG_INFO("DiscoveryDone Finish OK ...\n");

							//FIX_ID_034 xxxxx //Add MHL HPD Control by it6602HPDCtrl( )
							    it6602->m_DiscoveryDone = 1;
							//FIX_ID_034 xxxxx

							//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
							m_MHLabortID++;
							//FIX_ID_037 xxxxx

							break;
				}

				return;
			}



//============================================================================
			if((it6602->CBusIntEvent & B_DevCapChange) == B_DevCapChange	|| (it6602->CBusIntEvent & B_ReadDevCap) == B_ReadDevCap)
			{
				if(mhlrxrd(0xB0)&0x01 )
				{
					mhlrxwr(0x54, it6602->CBusSeqNo);
					CbusStatus = mscFire(MHL_MSC_CtrlPacket0, B_READ_DEVCAP);
					if(CbusStatus != SUCCESS)
					{
						DLOG_INFO("Set B_READ_DEVCAP Need to retry ...%X\n",(int)it6602->CBusSeqNo);
						return ;
					}

					it6602->Mhl_devcap[it6602->CBusSeqNo] = mhlrxrd(0x56);
					DLOG_INFO("IT6602-DevCap[%X]=%X\n", (int)it6602->CBusSeqNo, (int)it6602->Mhl_devcap[it6602->CBusSeqNo]);
					it6602->CBusSeqNo++;

					if(it6602->CBusSeqNo < 0x10)
					{
						return;
					}

					it6602->CBusIntEvent &= ~(B_ReadDevCap|B_DevCapChange);	// finish MSC	// finish B_PATH_EN
					DLOG_INFO("IT6602-Read Device Capability End...\n");
					parse_devcap(&it6602->Mhl_devcap[0]);

				}
				else
				{
					DLOG_INFO("IT6602-DCapRdy Change from '1' to '0'\r\n");
				}
				return;

			}

//============================================================================
		}
	}
}

//FIX_ID_005 xxxxx


//FIX_ID_014 xxxxx


//FIX_ID_018	xxxxx	//modify 1K pull-down to 1.033K ohm HDMI Reg1C0[3:2]=2
static void IT6602_WakeupProcess(void)
{
	if(wakeupcnt==0)
	{
		return;
	}

	wakeupcnt++;
	//DLOG_INFO("WakeUp Interrupt %d \n",wakeupcnt);

//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
	 mhlrxset(MHL_RX_2B, 0x04,0x04);	// MHL2B[2] 1 for enable HW wake up fail machanism

	if(( wakeupcnt%CDSENSE_WAKEUP)==(CDSENSE_WAKEUP-1))
	{
		chgbank(1);
		if((hdmirxrd(REG_RX_1C0)&0x0C) != 0x08)
		{
			hdmirxset(REG_RX_1C0, 0x8C, 0x08);
			DLOG_INFO("1k pull down  -10 percent \r\n");
		}
		else
		{
			// disable -> hdmirxset(REG_RX_1C0, 0x8C, 0x0C);
			// disable -> DLOG_INFO("1k pull down  -20 percent \r\n");
			hdmirxset(REG_RX_1C0, 0x8C, 0x04);
			DLOG_INFO("1k pull down  +10 percent for W1070 only \r\n");

		}
		chgbank(0);
	}
//FIX_ID_037 xxxxx

	if( wakeupcnt==IGNORE_WAKEUP)
	{
		//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify
		{
			if( mhlrxrd(MHL_RX_28)&0x08 )
			{
				mhlrxset(MHL_RX_28, 0x08, 0x00);
			}
			else
			{
				mhlrxset(MHL_RX_28, 0x08, 0x08);
			}

		}
		//FIX_ID_002 xxxxx
	}
	 else if( wakeupcnt==TOGGLE_WAKEUP ) 	//07-23 for SamSung Galaxy Note
	 {
		if((hdmirxrd(REG_RX_019)&0x80) != 0x80)	// Reg19[7] = 1 it is D0 chip
		{
			DLOG_INFO("WakeUp Interrupt %d \n",(int) wakeupcnt);
			mhlrxset(MHL_RX_28,0x40,0x40);
			//it6602HPDCtrl(0,1);
			IT_Delay(200);
			//it6602HPDCtrl(0,0);
			mhlrxset(MHL_RX_28,0x40,0x00);
		}
	 }
	 else if( wakeupcnt==RENEW_WAKEUP ) 	//07-23 for SamSung Galaxy Note
	 {
		wakeupcnt=0;
	 }



}

#endif
//FIX_ID_036	xxxxx

//FIX_ID_014 xxxxx
static void IT6602HDMIEventManager(struct it6602_dev_data *it6602)
{
	if(it6602->HDMIIntEvent !=0)
	{
//============================================================================

			if((it6602->HDMIIntEvent & B_PORT0_Waiting) == B_PORT0_Waiting)
			{
				if(it6602->HDMIWaitNo[0] == 0)
				{
					it6602->HDMIIntEvent &= ~(B_PORT0_Waiting);
					DLOG_INFO("B_PORT0_Waiting  OK ...\n");
				}
				else
				{
					it6602->HDMIWaitNo[0]--;
					DLOG_INFO("B_PORT0_Waiting  %X ...Event=%X ...Reg93=%X \n",
							(int) it6602->HDMIWaitNo[0],(int) it6602->HDMIIntEvent,(int) hdmirxrd(0x93));
				}
			}
			else
			{
			 	if((it6602->HDMIIntEvent & B_PORT0_TMDSEvent) == B_PORT0_TMDSEvent)
			 	{
					if(CLKCheck(F_PORT_SEL_0))
					{
				             DLOG_INFO("TMDSEvent &&&&& Port 0 Rx CKOn Detect &&&&&\r\n");
						#ifdef _SUPPORT_AUTO_EQ_
						TMDSCheck(F_PORT_SEL_0);
						#else
						//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
						#ifdef _SUPPORT_EQ_ADJUST_
						HDMIStartEQDetect(&(it6602->EQPort[F_PORT_SEL_0]));
						#endif
						//FIX_ID_001 xxxxx
						#endif
						it6602->HDMIIntEvent &= ~(B_PORT0_TMDSEvent);	// finish MSC
					}
				}
				else if((it6602->HDMIIntEvent & B_PORT0_TimingChgEvent) == B_PORT0_TimingChgEvent)
				{
					if(CLKCheck(F_PORT_SEL_0))
					{
				             DLOG_INFO("TimingChgEvent &&&&& Port 0 Rx CKOn Detect &&&&&\r\n");
						//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
						#ifdef _SUPPORT_EQ_ADJUST_
						HDMIStartEQDetect(&(it6602->EQPort[F_PORT_SEL_0]));
						#endif
						//FIX_ID_001 xxxxx

						it6602->HDMIIntEvent &= ~(B_PORT0_TimingChgEvent);	// finish MSC
					}
				}

			}
//============================================================================

//============================================================================

			if((it6602->HDMIIntEvent & B_PORT1_Waiting) == B_PORT1_Waiting)
			{
				if(it6602->HDMIWaitNo[1] == 0)
				{
					it6602->HDMIIntEvent &= ~(B_PORT1_Waiting);
					DLOG_INFO("B_PORT1_Waiting  OK ...\n");
				}
				else
				{
					it6602->HDMIWaitNo[1]--;
					DLOG_INFO("B_PORT1_Waiting  %X ...\n",(int) it6602->HDMIWaitNo[1]);
				}
			}
			else
			{
			 	if((it6602->HDMIIntEvent & B_PORT1_TMDSEvent) == B_PORT1_TMDSEvent)
			 	{
					if(CLKCheck(F_PORT_SEL_1))
					{
				             DLOG_INFO("TMDSEvent &&&&& Port 1 Rx CKOn Detect &&&&&\r\n");
						#ifdef _SUPPORT_AUTO_EQ_
						TMDSCheck(F_PORT_SEL_1);
						#else
						//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
						#ifdef _SUPPORT_EQ_ADJUST_
						HDMIStartEQDetect(&(it6602->EQPort[F_PORT_SEL_1]));
						#endif
						//FIX_ID_001 xxxxx
						#endif
						it6602->HDMIIntEvent &= ~(B_PORT1_TMDSEvent);	// finish MSC
					}
				}
				else if((it6602->HDMIIntEvent & B_PORT1_TimingChgEvent) == B_PORT1_TimingChgEvent)
				{
					if(CLKCheck(F_PORT_SEL_1))
					{
						DLOG_INFO("TimingChgEvent &&&&& Port 1 Rx CKOn Detect &&&&&\r\n");
						#ifdef _SUPPORT_EQ_ADJUST_
						HDMIStartEQDetect(&(it6602->EQPort[F_PORT_SEL_1]));
						#endif
						it6602->HDMIIntEvent &= ~(B_PORT1_TimingChgEvent);	// finish MSC
					}
				}

			}
//============================================================================
//============================================================================
//============================================================================

	}

}
// disable -> //FIX_ID_014 xxxxx
// disable ->
// disable ->
// disable -> //FIX_ID_018	xxxxx	//modify 1K pull-down to 1.033K ohm HDMI Reg1C0[3:2]=2
// disable -> static void IT6602_WakeupProcess(void)
// disable -> {
// disable -> 	if(wakeupcnt==0)
// disable -> 	{
// disable -> 		return;
// disable -> 	}
// disable ->
// disable -> 	wakeupcnt++;
// disable -> 	//DLOG_INFO("WakeUp Interrupt %d \n",wakeupcnt);
// disable ->
// disable -> 	 mhlrxset(MHL_RX_2B, 0x04,0x04);
// disable -> 	if(( wakeupcnt%CDSENSE_WAKEUP)==(CDSENSE_WAKEUP-1))
// disable -> 	{
// disable -> 		chgbank(1);
// disable -> 		if((hdmirxrd(REG_RX_1C0)&0x0C) == 0x0C)
// disable -> 		{
// disable -> 			hdmirxset(REG_RX_1C0, 0x8C, 0x08);
// disable -> 			DLOG_INFO("1k pull down  -10 percent \r\n");
// disable -> 		}
// disable -> 		else
// disable -> 		{
// disable -> 			hdmirxset(REG_RX_1C0, 0x8C, 0x0C);
// disable -> 			DLOG_INFO("1k pull down  -20 percent \r\n");
// disable -> 		}
// disable -> 		chgbank(0);
// disable -> 	}
// disable ->
// disable ->
// disable -> 	if( wakeupcnt==IGNORE_WAKEUP)
// disable -> 	{
// disable -> 		//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify
// disable -> 		{
// disable -> 			if( mhlrxrd(MHL_RX_28)&0x08 )
// disable -> 			{
// disable -> 				mhlrxset(MHL_RX_28, 0x08, 0x00);
// disable -> 			}
// disable -> 			else
// disable -> 			{
// disable -> 				mhlrxset(MHL_RX_28, 0x08, 0x08);
// disable -> 			}
// disable ->
// disable -> 		}
// disable -> 		//FIX_ID_002 xxxxx
// disable -> 	}
// disable -> 	 else if( wakeupcnt==TOGGLE_WAKEUP ) 	//07-23 for SamSung Galaxy Note
// disable -> 	 {
// disable -> 		if((hdmirxrd(REG_RX_019)&0x80) != 0x80)	// Reg19[7] = 1 it is D0 chip
// disable -> 		{
// disable -> 			DLOG_INFO("WakeUp Interrupt %d \n",(int) wakeupcnt);
// disable -> 			mhlrxset(MHL_RX_28,0x40,0x40);
// disable -> 			//it6602HPDCtrl(0,1);
// disable -> 			IT_Delay(200);
// disable -> 			//it6602HPDCtrl(0,0);
// disable -> 			mhlrxset(MHL_RX_28,0x40,0x00);
// disable -> 		}
// disable -> 	 }
// disable -> 	 else if( wakeupcnt==RENEW_WAKEUP ) 	//07-23 for SamSung Galaxy Note
// disable -> 	 {
// disable -> 		wakeupcnt=0;
// disable -> 	 }
// disable ->
// disable ->
// disable ->
// disable -> }
//FIX_ID_018	xxxxx


//FIX_ID_009 xxxxx	//verify interrupt event with reg51[0] select port
static unsigned char  IT6602_IsSelectedPort(unsigned char ucPortSel)
{
	unsigned char ucCurrentPort ;

	struct it6602_dev_data *it6602data = get_it6602_dev_data();

	ucCurrentPort = hdmirxrd(REG_RX_051) & B_PORT_SEL;

	if(ucCurrentPort == ucPortSel)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
//FIX_ID_009 xxxxx

/*****************************************************************************/
/* Driver State Machine Process **********************************************/
/*****************************************************************************/
#ifdef _ITEHDMI_

//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_
static void IT6602MHLInterruptHandler(struct it6602_dev_data *it6602)
{
	unsigned char MHL04, MHL05, MHL06;
	unsigned char MHLA0, MHLA1, MHLA2, MHLA3;
	unsigned char i,rddata;

	MHL04 = 0x00;
	MHL05 = 0x00;
	MHL06 = 0x00;
	MHLA0 = 0x00;
	MHLA1 = 0x00;
	MHLA2 = 0x00;
	MHLA3 = 0x00;

	MHL04 = mhlrxrd(0x04);
	MHL05 = mhlrxrd(0x05);
	MHL06 = mhlrxrd(0x06);

	mhlrxwr(0x04,MHL04);
	mhlrxwr(0x05,MHL05);
	mhlrxwr(0x06,MHL06);

	MHLA0 = mhlrxrd(0xA0);
	MHLA1 = mhlrxrd(0xA1);
	MHLA2 = mhlrxrd(0xA2);
	MHLA3 = mhlrxrd(0xA3);

	mhlrxwr(0xA0,MHLA0);
	mhlrxwr(0xA1,MHLA1);
	mhlrxwr(0xA2,MHLA2);
	mhlrxwr(0xA3,MHLA3);

//FIX_ID_018	xxxxx	//modify 1K pull-down to 1.033K ohm HDMI Reg1C0[3:2]=2
	IT6602_WakeupProcess();
//FIX_ID_018	xxxxx

	if( MHL04&0x01 ) {
		DLOG_INFO("IT6602-CBUS Link Layer TX Packet Done Interrupt ...\n");
	}

	if( MHL04&0x02 ) {

		DLOG_INFO("IT6602-ERROR: CBUS Link Layer TX Packet Fail Interrupt ... \n");
		DLOG_INFO("IT6602- TX Packet error Status reg15=%X\n", (int)mhlrxrd(0x15));

		rddata = mhlrxrd(0x15);

		mhlrxwr(0x15, rddata&0xF0);

	}

	if( MHL04&0x04 ) {
		DLOG_INFO("IT6602-CBUS Link Layer RX Packet Done Interrupt ...\n");
	}

	if( MHL04&0x08 ) {

		 DLOG_INFO("IT6602-ERROR: CBUS Link Layer RX Packet Fail Interrupt ... \n");

		 DLOG_INFO("IT6602- TX Packet error Status reg15=%X\n", (int)mhlrxrd(0x15));

		 rddata = mhlrxrd(0x15);

		 mhlrxwr(0x15, rddata&0x0F);

//FIX_ID_008 xxxxx	//Add SW reset when HDMI / MHL device un-plug  !!!
		rddata=hdmirxrd(REG_RX_P0_SYS_STATUS);
		DLOG_INFO("IT6602- Port 0 MHL unplug detect = %X !!! \n",(int) rddata);
		//IT_Delay(10);
		//if((rddata & (B_P0_SCDT|B_P0_RXCK_VALID))== 0)
		if((rddata & (B_P0_SCDT))== 0)
		{
			//FIX_ID_009 xxxxx	//verify interrupt event with reg51[0] select port
			if(IT6602_IsSelectedPort(0) == TRUE)
			//FIX_ID_009 xxxxx
			{
				IT6602SwitchVideoState(it6602,VSTATE_SWReset);
			}
			else
			{
				hdmirxset(REG_RX_011, 0x01, 0x01);
				IT_Delay(1);
				hdmirxset(REG_RX_011, 0x01, 0x00);
#ifdef _SUPPORT_AUTO_EQ_
				DisableOverWriteRS(0);	//2013-1129 for MHL unplug detected		ss
#endif
			}
		}
//FIX_ID_008 xxxxx




	}

	if( MHL04&0x10 ) {
		DLOG_INFO("IT6602-MSC RX MSC_MSG Interrupt ...\n");
	    	mhl_read_mscmsg(it6602 );

//FIX_ID_005 xxxxx	//for waiting RAP content on
//2013-1003 not yet ->		it6602->CBusIntEvent |= (B_MSC_Waiting);
//2013-1003 not yet ->		it6602->CBusWaitNo=MAX_PATHEN_WAITNO;
//FIX_ID_005 xxxxx

	}

	if( MHL04&0x20 ) {
		 DLOG_INFO("IT6602-MSC RX WRITE_STAT Interrupt ...\n");
	}

	if( MHL04&0x40 ) {

		DLOG_INFO("IT6602-MSC RX WRITE_BURST Interrupt  ...\n");
	}


	if( MHL05&0x01 ) {

		DLOG_INFO("IT6602-MSC Req Done Interrupt ...\n");
	}


	if( MHL05&0x02 )
	 {

		DLOG_INFO("IT6602-MSC Req Fail Interrupt (Unexpected) ...\n");
		DLOG_INFO("IT6602-MSC Req Fail reg18= %X \n",(int)mhlrxrd(0x18));

		rddata = mhlrxrd(0x18);
		mhlrxwr(0x18, rddata);

		rddata = mhlrxrd(0x19);

		if( rddata&0x01 )
			DLOG_INFO("IT6602-ERROR: TX FW Fail in the middle of the command sequence !!!\n");
		if( rddata&0x02 )
			DLOG_INFO("IT6602-ERROR: TX Fail because FW mode RxPktFIFO not empty !!!\n");

		mhlrxwr(0x19, rddata);


	}

	if( MHL05&0x04 ) {
		 mhlrxwr(0x05, 0x04);

		 DLOG_INFO("IT6602-MSC Rpd Done Interrupt ...\n");
	}

	if( MHL05&0x08 ) {
		DLOG_INFO("IT6602-MSC Rpd Fail Interrupt ...\n" );
		DLOG_INFO("IT6602-MSC Rpd Fail status reg1A=%X reg1B=%X\n", (int)mhlrxrd(0x1A),(int)mhlrxrd(0x1B));

		 rddata = mhlrxrd(0x1A);

		mhlrxwr(0x1A, rddata);

		 rddata = mhlrxrd(0x1B);
		 if( rddata&0x01 )
			 DLOG_INFO("IT6602-ERROR: Retry > 32 times !!!\n");
		 if( rddata&0x02 ) {
			 DLOG_INFO("IT6602-ERROR: Receive ABORT Packet !!!\n");
			 //get_msc_errcode();
		 }

		 mhlrxwr(0x1B, rddata);


	}

	if( MHL05&0x10 ) {

	         mhlrxwr(0x05, 0x10);

		 if( (mhlrxrd(0xB1)&0x07)==2 ) {
			DLOG_INFO("MHL Clock Mode : PackPixel Mode ...\r\n");
		 }
		 else {
			DLOG_INFO("MHL Clock Mode : 24 bits Mode ...\r\n");
		 }

	}

	if( MHL05&0x20 ) {

		DLOG_INFO("IT6602-DDC Req Fail Interrupt (Hardware) ... \r\n");

	}

	if( MHL05&0x40 ) {

		DLOG_INFO("IT6602-DDC Rpd Done Interrupt ...\r\n");
	}

	if( MHL05&0x80 ) {

		DLOG_INFO("IT6602-DDC Rpd Fail Interrupt ...\r\n");

         rddata  = mhlrxrd(0x16);
         if( rddata&0x01 )
             DLOG_INFO("RxState=IDLE, receive non-SOF packet !!!\r\n");
         if( rddata&0x02 )
             DLOG_INFO("RxState/=IDLE, receive unexpected packet !!!\r\n");
         if( rddata&0x04 )
             DLOG_INFO("100ms timeout !!!\r\n");
         if( rddata&0x08 )
             DLOG_INFO("100ms timeout caused by link layer error !!!\r\n");
         if( rddata&0x10 )
             DLOG_INFO("Receive unexpected STOP !!!\r\n");
         if( rddata&0x20 )
             DLOG_INFO("Transmit packet failed !!!\r\n");
         if( rddata&0x40 )
             DLOG_INFO(" DDC bus hang !!!\r\n");
         if( rddata&0x80 )
             DLOG_INFO("TxState/=IDLE, receive new packet !!!\r\n");

         mhlrxwr(0x16, rddata);

         rddata = mhlrxrd(0x17);
         if( rddata&0x01 )
             DLOG_INFO("Receive TxDDCArbLose !!!\r\n");

         mhlrxwr(0x17, rddata);

	}

	 if( MHL06&0x01 ) {
		 mhlrxwr(0x06, 0x01);
		DLOG_INFO("CBUS disvovery wakeup interrupt %d  [MHL06&0x01] \r\n",(int)wakeupcnt);
		DLOG_INFO("MHL_RX_2A = %X \r\n",(int)mhlrxrd(MHL_RX_2A));
		wakeupcnt = 0;

		//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
		//FIX_ID_018 xxxxx 	modify 1K pull-down to 1.033K ohm HDMI Reg1C0[3:2]=2
		chgbank(1);
		// FIX_ID_037 , disable -> hdmirxset(REG_RX_1C0, 0x8C, 0x08);
		// FIX_ID_037 , disable -> DLOG_INFO("1K pull-down -10 percent \r\n");
		hdmirxset(REG_RX_1C0, 0x8C, 0x00);	//xxxxx 2014-0604 set to default value when discovery Done !!!
		DLOG_INFO("set 1K pull-down to default value \r\n");
		chgbank(0);
 		//FIX_ID_018 xxxxx
		//FIX_ID_037 xxxxx

		//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify
		//if(HdmiI2cAddr==IT6602A0_HDMI_ADDR)
		{
			mhlrxset(MHL_RX_28, 0x08, 0x00);//MHL28[3] RegCBUSFloatAdj [Discovery related pulse width select]
		}
		//FIX_ID_002 xxxxx

		//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
			 mhlrxset(MHL_RX_2B, 0x04,0x00);	// MHL2B[2] 0 for disable HW wake up fail machanism
		//FIX_ID_037 xxxxx

	 }

	 if( MHL06&0x02 ) {
		 mhlrxwr(0x06, 0x02);


		//FIX_ID_012	xxxxx	//For SamSung Galaxy Note wake up fail issue
		//FIX_ID_018	xxxxx	//modify 1K pull-down to 1.033K ohm HDMI Reg1C0[3:2]=2
		if(wakeupcnt==0)
		{
			wakeupcnt=1;
		}
		//FIX_ID_018	xxxxx
		//xxxxx FIX_ID_012

	}

	if( MHL06&0x04 )
	{

		DLOG_INFO("it6602-CBUS Discovery Done Interrupt ...\n");
		DLOG_INFO("CBUS Discovery Done Interrupt ...\n");
		wakeupcnt = 0;
		TxWrBstSeq = 0;

//FIX_ID_005 xxxxx	//Add Cbus Event Handler
	it6602->CBusIntEvent=0;
	it6602->CBusSeqNo=0;

	it6602->CBusWaitNo=0x00;
	it6602->CBusIntEvent |= B_DiscoveryDone;


//FIX_ID_012 xxxxx		//For SamSung Galaxy Note wake up fail issue
	it6602->CBusIntEvent |= (B_MSC_Waiting);
	it6602->CBusWaitNo=MAX_DISCOVERY_WAITNO;	//100ms
//FIX_ID_012 xxxxx

//FIX_ID_024	xxxxx	//Fixed for RCP compliance issue
	it6602->m_bRCPTimeOut = FALSE;
	it6602->m_bRCPError = FALSE;
//FIX_ID_024	xxxxx

//FIX_ID_034 xxxxx //Add MHL HPD Control by it6602HPDCtrl( )
	it6602->m_DiscoveryDone = 0;
//FIX_ID_034 xxxxx

//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
	m_MHLabortID=1;
//FIX_ID_037 xxxxx
#if 0
// disable ->		if( EnHWPathEn==FALSE )
// disable ->		{
// disable ->
// disable ->			// DEVCAP initialization here
// disable ->			DLOG_INFO("Set DCAP_RDY to 1 ...\n");
// disable ->			set_mhlsts(MHLSts00B, DCAP_RDY);
// disable ->
// disable ->			DLOG_INFO("Set HPD to 1 ...\n");
// disable ->//			mhlrxwr(MHL_MSC_CtrlPacket0, B_SET_HPD);	// SET_HPD
// disable ->			 mscFire(MHL_MSC_CtrlPacket0, B_SET_HPD);
// disable ->
// disable ->
// disable ->			DLOG_INFO("Set PATH_EN to 1 ...\n");
// disable ->			set_mhlsts(MHLSts01B, PATH_EN);
// disable ->		}
#endif
//FIX_ID_005 xxxxx

	}

	//if( MHL06&0x08 ) {
	////		DLOG_INFO("it6602-CBUS Discovery Fail Interrupt ... ==> %dth Fail\n",(int) it6602->DisvFailCnt);
	//}

	if( MHL06&0x10 ) {

		DLOG_INFO("IT6602-CBUS PATH_EN Change Interrupt ...\n");

//FIX_ID_005 xxxxx 	//Add Cbus Event Handler
		it6602->CBusIntEvent |= B_MSC_Waiting;
		it6602->CBusWaitNo = MAX_PATHEN_WAITNO;
//FIX_ID_005 xxxxx


//07-01		if( EnHWPathEn )
//		mhlrxwr(MHL_MSC_CtrlPacket0, B_SET_HPD);	// SET_HPD
//07-01		mscFire(MHL_MSC_CtrlPacket0, B_SET_HPD);

#if 0	// disable trigger EQ
		hdmirxset(0x26, 0x80, 0x00);
		IT_Delay(1);
		hdmirxset(0x26, 0x80, 0x80);
		hdmirxset(0x22, 0x04, 0x04);
#endif


//		hdmirxset(0x22, 0x04, 0x04); //trigger AutoEQ

	}

	if( MHL06&0x20 )
	{

		DLOG_INFO("it6602-CBUS MUTE Change Interrupt ...\n");
		DLOG_INFO("it6602-Current CBUS MUTE Status = %X \n", (int)(mhlrxrd(0xB1)&0x10)>>4);
	}

	if( MHL06&0x40 )
	{

		DLOG_INFO("IT6602-CBUS DCapRdy Change Interrupt ...\n");

//FIX_ID_005 xxxxx	//Add Cbus Event Handler

	it6602->CBusIntEvent |= B_ReadDevCap;
//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
	if(it6602->m_DiscoveryDone != 0)
	{
		it6602->CBusSeqNo = 0x00;
	}
//FIX_ID_037 xxxxx

#if 0
// disable ->		if( mhlrxrd(0xB0)&0x01 )
// disable ->		{
// disable ->
// disable ->			read_devcap_hw(it6602);  // READ_DEVCAP hardware mode
// disable ->
// disable ->			DLOG_INFO("IT6602-Set DCAP_RDY to 1 ...\n");
// disable ->
// disable ->			set_mhlsts(MHLSts00B, DCAP_RDY);
// disable ->
// disable ->		}
// disable ->		else
// disable ->		{
// disable ->			DLOG_INFO("IT6602-DCapRdy Change from '1' to '0'\n");
// disable ->		}
#endif
//FIX_ID_005 xxxxx
	}

	if( MHL06&0x80 )
	{

		DLOG_INFO("IT6602-VBUS Status Change Interrupt ...\n");
		DLOG_INFO("IT6602-Current VBUS Status = %X\n",(int) (mhlrxrd(0x10)&0x08)>>3);

	}


	if( MHLA0&0x01 ) {
		 DLOG_INFO("IT6602-MHL Device Capability Change Interrupt ...\n");

//FIX_ID_005 xxxxx	//Add Cbus Event Handler
	it6602->CBusIntEvent |= B_DevCapChange;
//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
	if(it6602->m_DiscoveryDone != 0)
	{
		it6602->CBusSeqNo = 0x00;
	}
//FIX_ID_037 xxxxx

#if 0
// disable ->			if( mhlrxrd(0xB0)&0x01 ){
// disable ->				read_devcap_hw(it6602);	  // READ_DEVCAP HardWare mode
// disable ->			}
// disable ->			else
// disable ->			{
// disable ->				DLOG_INFO("IT6602-MHL Device Capability is still not Ready !!! \n");
// disable ->			}
#endif
//FIX_ID_005 xxxxx
	}

	if( MHLA0&0x02 ) {
		DLOG_INFO("IT6602-MHL DSCR_CHG Interrupt ......\n");

         mhlrxbrd(0xC0, 16, &(it6602->rxscrpad[0]));

         rddata = mhlrxrd(0x64);

         for(i=0; i<rddata; i++)
             DLOG_INFO("RX Scratch Pad [%02d] = 0x%X\n",(int)  i, (int) it6602->rxscrpad[i]);

         if( rddata>16 )
             DLOG_INFO("ERROR: Receive Scratch Pad Data > 16 bytes !!!\n");

         if( it6602->rxscrpad[0]!=mhlrxrd(0x83) || it6602->rxscrpad[1]!=mhlrxrd(0x84) ) {
             DLOG_INFO("Adopter ID = 0x%X %X\n", (int) mhlrxrd(0x83), (int) mhlrxrd(0x84));
             DLOG_INFO("ERROR: Adopter ID Mismatch !!!\n");
         }


	}

	if( MHLA0&0x04 ) {

		DLOG_INFO("IT6602-MHL REQ_WRT Interrupt  ...\n");

		DLOG_INFO("IT6602-Set GRT_WRT to 1  ...\n");

		set_mhlint(MHLInt00B, GRT_WRT);

         rddata = mhlrxrd(0x64);

         for(i=0; i<rddata; i++)
             DLOG_INFO("RX Scratch Pad [%02d] = 0x%X\n",(int)  i,(int)  mhlrxrd(0xc0+i));


	}

	if( MHLA0&0x08 )
	{

		DLOG_INFO("IT6602-[**]MHL GNT_WRT Interrupt  ...\n");

#ifdef FIX_ID_013_
//FIX_ID_013	xxxxx	//For MSC 3D request issue
		//FIX_ID_005 xxxxx	//Add Cbus Event Handler
		// disable --> 	it6602->CBusIntEvent |= B_3DSupporpt;
		if(e3DReqState == MHL3D_REQ_WRT)
		{
			e3DReqState = MHL3D_GNT_WRT;
		}
		//FIX_ID_005 xxxxx
//FIX_ID_013	xxxxx
#else
		//FIX_ID_037 xxxxx //2014-0522 MHL compliance issue !!!
		it6602->CBusWaitNo =0 ;
		//FIX_ID_037 xxxxx
		it6602->CBusIntEvent |= B_3DSupporpt;
				if( V3D_EntryCnt==0 )
					v3d_unsupport1st(it6602);
					else if( V3D_EntryCnt==1 ) {
					v3d_unsupport2nd(it6602);
					it6602->CBusIntEvent &= ~(B_3DSupporpt);	// finish MSC
					DLOG_INFO(" ### 3D Un-Support Write_Burst End ###\n");
				}

#endif //FIX_ID_013	xxxxx
	}



     if( MHLA0&0x10 ) {
         mhlrxwr(0xA0, 0x10);

         DLOG_INFO("### 3D Request Interrupt ...### \n");

#ifdef FIX_ID_013_
//FIX_ID_013	xxxxx	//For MSC 3D request issue
	// disable -->		 if( EnMSCWrBurst3D==TRUE ) {
	// disable -->			V3D_EntryCnt = 0;
	// disable -->			set_mhlint(MHLInt00B, REQ_WRT);
	// disable -->		 }

	if(e3DReqState == MHL3D_REQ_DONE)
	{
		it6602->CBusIntEvent |= B_3DSupporpt;
		e3DReqState = MHL3D_REQ_START;
	}
//FIX_ID_013	xxxxx
#else
	//FIX_ID_037 xxxxx //2014-0522 MHL compliance issue !!!
	it6602->CBusWaitNo =0 ;
	//FIX_ID_037 xxxxx
	V3D_EntryCnt = 0;
	set_mhlint(MHLInt00B, REQ_WRT);
#endif //FIX_ID_013

     }

	if( MHLA1&0x02 ) {
		DLOG_INFO("IT6602-MHL EDID Change Interrupt ...\r\n");
	}

}

#endif
//FIX_ID_036	xxxxx

static void IT6602HDMIInterruptHandler(struct it6602_dev_data *it6602)
{
	volatile unsigned char Reg05h;
	volatile unsigned char Reg06h;
	volatile unsigned char Reg07h;
	volatile unsigned char Reg08h;
	volatile unsigned char Reg09h;
	volatile unsigned char Reg0Ah;
//	volatile unsigned char Reg0Bh;
	volatile unsigned char RegD0h;

	unsigned char MHL04;
	unsigned char MHL05;
	unsigned char MHL06;
	unsigned char MHLA0;
	unsigned char MHLA1;
	unsigned char MHLA2;
	unsigned char MHLA3;

	chgbank(0);
/*	
	for (unsigned int i = 0; i < 0xffffff; i++)
	{
		;
	}*/


	Reg05h = hdmirxrd(REG_RX_005);
	Reg06h = hdmirxrd(REG_RX_006);
	Reg07h = hdmirxrd(REG_RX_007);
	Reg08h = hdmirxrd(REG_RX_008);
	Reg09h = hdmirxrd(REG_RX_009);

	Reg0Ah = hdmirxrd(REG_RX_P0_SYS_STATUS);
//	Reg0Bh = hdmirxrd(REG_RX_P1_SYS_STATUS);
	RegD0h = hdmirxrd(REG_RX_0D0);


	hdmirxwr(REG_RX_005, Reg05h);
	hdmirxwr(REG_RX_006, Reg06h);
	hdmirxwr(REG_RX_007, Reg07h);
	hdmirxwr(REG_RX_008, Reg08h);
	hdmirxwr(REG_RX_009, Reg09h);
//2013-0606 disable ==>
	hdmirxwr(REG_RX_0D0, RegD0h&0x0F);
	DLOG_INFO("ite interrupt");
	DLOG_INFO("Reg05 = %X",(int) Reg05h);
	DLOG_INFO("Reg06 = %X",(int) Reg06h);
	DLOG_INFO("Reg07 = %X",(int) Reg07h);
	DLOG_INFO("Reg08 = %X",(int) Reg08h);
	DLOG_INFO("Reg09 = %X",(int) Reg09h);
	DLOG_INFO("Reg0A = %X",(int) Reg0Ah);
	DLOG_INFO("RegD0 = %X",(int) RegD0h);
	MHL04 = mhlrxrd(0x04);
	MHL05 = mhlrxrd(0x05);
	MHL06 = mhlrxrd(0x06);

	mhlrxwr(0x04,MHL04);
	mhlrxwr(0x05,MHL05);
	mhlrxwr(0x06,MHL06);

	MHLA0 = mhlrxrd(0xA0);
	MHLA1 = mhlrxrd(0xA1);
	MHLA2 = mhlrxrd(0xA2);
	MHLA3 = mhlrxrd(0xA3);

	mhlrxwr(0xA0,MHLA0);
	mhlrxwr(0xA1,MHLA1);
	mhlrxwr(0xA2,MHLA2);
	mhlrxwr(0xA3,MHLA3);

	unsigned char ClearReg05h = hdmirxrd(REG_RX_005);
	unsigned char ClearReg06h = hdmirxrd(REG_RX_006);
	unsigned char ClearReg07h = hdmirxrd(REG_RX_007);
	unsigned char ClearReg08h = hdmirxrd(REG_RX_008);
	unsigned char ClearReg09h = hdmirxrd(REG_RX_009);

	unsigned char ClearReg0Ah = hdmirxrd(REG_RX_P0_SYS_STATUS);
//	Reg0Bh = hdmirxrd(REG_RX_P1_SYS_STATUS);
	unsigned char ClearRegD0h = hdmirxrd(REG_RX_0D0);

	DLOG_INFO("ite clear interrupt");
	DLOG_INFO("ClearReg05 = %X",(int) ClearReg05h);
	DLOG_INFO("ClearReg06 = %X",(int) ClearReg06h);
	DLOG_INFO("ClearReg07 = %X",(int) ClearReg07h);
	DLOG_INFO("ClearReg08 = %X",(int) ClearReg08h);
	DLOG_INFO("ClearReg09 = %X",(int) ClearReg09h);
	DLOG_INFO("ClearReg0A = %X",(int) ClearReg0Ah);
	DLOG_INFO("ClearRegD0 = %X",(int) ClearRegD0h);


//	DLOG_INFO("111111111111111111111111 STATUS 111111111111111111111= %X \r\n",hdmirxrd(REG_RX_P0_SYS_STATUS));
     if( Reg05h!=0x00 )
	{

		DLOG_INFO("Reg05 = %X \r\n",(int) Reg05h);
		//dlog_info("Reg05 = %X",(int) Reg05h);
		 if( Reg05h&0x80 ) {
			 DLOG_INFO("#### Port 0 HDCP Off Detected ###\r\n");
			it6602->m_ucEccCount_P0=0;
		 }

		 if( Reg05h&0x40 ) {
			 DLOG_INFO("#### Port 0 ECC Error %X ####\r\n",(int) (it6602->m_ucEccCount_P0));
//			HDMICheckErrorCount(&(it6602->EQPort[F_PORT_SEL_0]));	//07-04 for port 0
			hdmirx_INT_P0_ECC(it6602);
		 }

		 if( Reg05h&0x20 ) {

		     DLOG_INFO("#### Port 0 HDMI/DVI Mode change ####\r\n");
//FIX_ID_009 xxxxx	//verify interrupt event with reg51[0] select port
			if(CLKCheck(0))
			hdmirx_INT_HDMIMode_Chg(it6602,0);
//FIX_ID_009 xxxxx

		 }

		 if( Reg05h&0x08 )
		 {
			 DLOG_INFO("#### Port 0 HDCP Authentication Start ####\r\n");
			it6602->m_ucEccCount_P0=0;
//			get_vid_info();
//			show_vid_info();

#ifdef _SUPPORT_HDCP_REPEATER_
//FIX_ID_032 xxxxx	//Support HDCP Repeater function for HDMI Tx device
			if(m_bHDCPrepeater== TRUE)
			{
	    			ITEHDMI_RxHDCPRepeaterCapabilityClear(B_KSV_READY);
				ITEHDMI_Event_Notify_Callback(eAKSV_WRITTEN_NOTIFY);
				//RxAuthStartInt();
				//bEnableAuth = TRUE ;
			}
//FIX_ID_032 xxxxx
#endif

#ifdef _SUPPORT_AUTO_EQ_
//FIX_ID_014 xxxxx
			if( ucPortAMPOverWrite[F_PORT_SEL_0] == 0)
			{
				if((it6602->HDMIIntEvent & (B_PORT0_Waiting))==0)
				{
					hdmirxwr(REG_RX_022, 0x00);	// power down auto EQ

					it6602->HDMIIntEvent |= (B_PORT0_Waiting);
					it6602->HDMIIntEvent |= (B_PORT0_TMDSEvent);
					it6602->HDMIWaitNo[0]=MAX_TMDS_WAITNO;
				}
				else if((it6602->HDMIIntEvent & (B_PORT0_TMDSEvent)))
				{
					it6602->HDMIIntEvent |= (B_PORT0_Waiting);
					it6602->HDMIWaitNo[0] += MAX_HDCP_WAITNO;
				}
			}
			else
			{
				if((it6602->HDMIIntEvent & (B_PORT0_TMDSEvent)))
				{
					it6602->HDMIIntEvent |= (B_PORT0_Waiting);
					it6602->HDMIWaitNo[0] += MAX_HDCP_WAITNO;
				}
			}
//FIX_ID_014 xxxxx
#endif

//FIX_ID_005 xxxxx	//for waiting RAP content on
			if( (Reg0Ah&0x40))
			{
				it6602->CBusIntEvent |= (B_MSC_Waiting);
				it6602->CBusWaitNo=MAX_CBUS_WAITNO;
			}
//FIX_ID_005 xxxxx


		 }

		 if( Reg05h&0x10 ) {
			 DLOG_INFO("#### Port 0 HDCP Authentication Done ####\r\n");

//FIX_ID_005 xxxxx	//for waiting RAP content on
		if( (Reg0Ah&0x40))
		{
			it6602->CBusIntEvent |= (B_MSC_Waiting);
			it6602->CBusWaitNo=MAX_CBUS_WAITNO;
		}
//FIX_ID_005 xxxxx

#ifdef _SUPPORT_HDCP_REPEATER_
//FIX_ID_032 xxxxx	//Support HDCP Repeater function for HDMI Tx device

			if(m_bHDCPrepeater == FALSE)
				m_RxHDCPstatus = 0;
			else
				m_RxHDCPstatus = 2;

			#ifdef _PSEUDO_HDCP_REPEATER_TEST_
				// TX_BSTATUS = 0x102;
				//ITEHDMI_RxHDCP2ndAuthenticationRequest(TX_KSVList, TX_BKSV, TX_BSTATUS);
			#else
				//    For debug HDCP receiver mode Only !!!
				//	m_bHDCPrepeater=FALSE;
				//	m_RxHDCPstatus = 0;
				//	ITEHDMI_RxHDCPRepeaterCapabilityClear(B_ENABLE_REPEATER);
				//	ITEHDMI_RxBstatusSet(0x00);
				//	ITEHDMI_RxHDCPRepeaterCapabilitySet(B_KSV_READY);
			#endif


			#ifdef _SUPPORT_AUTO_EQ_
			//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
				if(ucPortAMPOverWrite[0]==0)	// 2013-0801
				{
					it6602->HDMIIntEvent &= ~(B_PORT0_Waiting);
					it6602->HDMIWaitNo[0]= 0;
					it6602->HDMIIntEvent |= B_PORT0_TMDSEvent;
					//return;
				}
			//FIX_ID_033 xxxxx
			#endif

#else

			#ifdef _SUPPORT_AUTO_EQ_
			//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
			//FIX_ID_032 xxxxx	//Support HDCP Repeater function for HDMI Tx device
			//xxxxx 2014-0421
			//FIX_ID_014 xxxxx
				if(ucPortAMPOverWrite[0]==0)	// 2013-0801
				{
					it6602->HDMIIntEvent &= ~(B_PORT0_Waiting);
					it6602->HDMIWaitNo[0]= 0;
					it6602->HDMIIntEvent |= B_PORT0_TMDSEvent;
					//return;
				}
			//FIX_ID_014 xxxxx
			//xxxxx 2014-0421
			//FIX_ID_032 xxxxx
			//FIX_ID_033 xxxxx
			#endif

//FIX_ID_032 xxxxx
#endif
		 }

		 if( Reg05h&0x04 )
		 {
			 DLOG_INFO("#### Port 0 Input Clock Change Detect ####\r\n");
		 }

		 if( Reg05h&0x02 )
		 {

			it6602->m_ucEccCount_P0=0;
			it6602->m_ucDeskew_P0=0;
			//it6602->m_ucDeskew_P1=0;
			//it6602->m_ucEccCount_P1=0;

			DLOG_INFO("#### Port 0 Rx CKOn Detect ####\r\n");

//#ifdef _SUPPORT_HDCP_REPEATER_
#if 1
//FIX_ID_032 xxxxx	//Support HDCP Repeater function for HDMI Tx device
			if(CLKCheck(F_PORT_SEL_0))
			{
				#ifdef _SUPPORT_AUTO_EQ_
				TMDSCheck(F_PORT_SEL_0);
				#else
				//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
				#ifdef _SUPPORT_EQ_ADJUST_
				HDMIStartEQDetect(&(it6602->EQPort[F_PORT_SEL_0]));
				#endif
				//FIX_ID_001 xxxxx
				#endif
			}
#else

			// NO --> Authentication Start 	&& 	Input Clock Change Detect 	&&	 B_PORT1_TMDSEvent
			if(( Reg05h&0x08 )==0 && ( Reg05h&0x04 )==0  &&  (it6602->HDMIIntEvent & (B_PORT0_TMDSEvent))==0)
			{
					if(CLKCheck(F_PORT_SEL_0))
					{
						#ifdef _SUPPORT_AUTO_EQ_
						TMDSCheck(F_PORT_SEL_0);
						#endif
					}
			}
			else
			{
				if(( Reg05h&0x10 ) == 0)
				{
					if((it6602->HDMIIntEvent & (B_PORT0_Waiting))==0)
					{
						hdmirxwr(REG_RX_022, 0x00);	// power down auto EQ

						//FIX_ID_019 xxxxx modify ENHYS control for MHL mode
						if(hdmirxrd(REG_RX_P0_SYS_STATUS) & B_P0_RXCK_VALID)
						{
							it6602->HDMIIntEvent |= (B_PORT0_Waiting);
							it6602->HDMIIntEvent |= (B_PORT0_TMDSEvent);
							it6602->HDMIWaitNo[0]=MAX_TMDS_WAITNO;
						}
						//FIX_ID_019 xxxxx
					}
				}
				else
				{
					if(CLKCheck(F_PORT_SEL_0))
					{
						#ifdef _SUPPORT_AUTO_EQ_
						TMDSCheck(F_PORT_SEL_0);
						#endif
					}
				}
			}
//FIX_ID_032 xxxxx
#endif
		 }

		 if( Reg05h&0x01 ) {
			DLOG_INFO("#### Port 0 Power 5V change ####\r\n");
			hdmirx_INT_5V_Pwr_Chg(it6602,0);


			// disable ->// disable ->FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
			// disable ->if(CheckPlg5VPwr(F_PORT_SEL_0)==FALSE)
			// disable ->{
			// disable ->
			// disable ->	#ifdef _SUPPORT_EQ_ADJUST_
			// disable ->	DisableOverWriteRS(F_PORT_SEL_0);
			// disable ->	#endif
			// disable ->}
			// disable ->// disable ->FIX_ID_001 xxxxx

		 }
	 }

     if( Reg06h!=0x00 )
	 {
		 //dlog_info("Reg06h = %X",(int) Reg06h);
		 if( Reg06h&0x80 ) {
			DLOG_INFO("#### Port 1 HDCP Off Detected ###\r\n");
			it6602->m_ucEccCount_P1=0;

		 }

		 if( Reg06h&0x40 ) {
			 DLOG_INFO("#### Port 1 ECC Error ####\r\n");
			hdmirx_INT_P1_ECC(it6602);
		 }

		 if( Reg06h&0x20 )
		 {
		     DLOG_INFO("#### Port 1 HDMI/DVI Mode change ####\r\n");
//FIX_ID_009 xxxxx	//verify interrupt event with reg51[0] select port
			if(CLKCheck(1))
			hdmirx_INT_HDMIMode_Chg(it6602,1);
//FIX_ID_009 xxxxx
		 }

//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
//xxxxx 2014-0421 disabe-> 		 if( Reg06h&0x10 )
//xxxxx 2014-0421 disabe-> 		 {
//xxxxx 2014-0421 disabe-> 			 DLOG_INFO("#### Port 1 HDCP Authentication Done ####\n");
//xxxxx 2014-0421 disabe-> //FIX_ID_014 xxxxx
//xxxxx 2014-0421 disabe-> 			if((it6602->HDMIIntEvent & (B_PORT1_Waiting)))
//xxxxx 2014-0421 disabe-> 			{
//xxxxx 2014-0421 disabe-> 				it6602->HDMIWaitNo[1] = 0;
//xxxxx 2014-0421 disabe-> 			}
//xxxxx 2014-0421 disabe-> //FIX_ID_014 xxxxx
//xxxxx 2014-0421 disabe->
//xxxxx 2014-0421 disabe-> 		 }
//FIX_ID_033 xxxxx


		 if( Reg06h&0x08 )
		 {
			 DLOG_INFO("#### Port 1 HDCP Authentication Start ####\r\n");
			it6602->m_ucEccCount_P1=0;

#ifdef _SUPPORT_AUTO_EQ_
//FIX_ID_014 xxxxx
			if( ucPortAMPOverWrite[F_PORT_SEL_1] == 0)
			{
				if((it6602->HDMIIntEvent & (B_PORT1_Waiting))==0)
				{
					DLOG_INFO(" power down auto EQ of PORT 1\r\n");
					hdmirxwr(REG_RX_03A, 0x00);	// power down auto EQ

					it6602->HDMIIntEvent |= (B_PORT1_Waiting);
					it6602->HDMIIntEvent |= (B_PORT1_TMDSEvent);
					it6602->HDMIWaitNo[1]=MAX_TMDS_WAITNO;
				}
				else if((it6602->HDMIIntEvent & (B_PORT1_TMDSEvent)))
				{
					it6602->HDMIIntEvent |= (B_PORT1_Waiting);
					it6602->HDMIWaitNo[1] += MAX_HDCP_WAITNO;
				}
			}
			else
			{
				if((it6602->HDMIIntEvent & (B_PORT1_TMDSEvent)))
				{
					it6602->HDMIIntEvent |= (B_PORT1_Waiting);
					it6602->HDMIWaitNo[1] += MAX_HDCP_WAITNO;
				}
			}
//FIX_ID_014 xxxxx
#endif

		 }

		 if( Reg06h&0x10 )
		 {
			 DLOG_INFO("#### Port 1 HDCP Authentication Done ####\r\n");
//FIX_ID_014 xxxxx
			if((it6602->HDMIIntEvent & (B_PORT1_Waiting)))
			{
				it6602->HDMIWaitNo[1] = 0;
			}
//FIX_ID_014 xxxxx


#ifdef _SUPPORT_AUTO_EQ_
//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
// disable ->	if( ucPortAMPValid[F_PORT_SEL_1] == 0)
			if(ucPortAMPOverWrite[1]==0)	// 2013-0801
			{
				it6602->HDMIIntEvent &= ~(B_PORT1_Waiting);
				it6602->HDMIWaitNo[1]= 0;
				it6602->HDMIIntEvent |= B_PORT1_TMDSEvent;
			}
//FIX_ID_033 xxxxx
#endif


		 }



		 if( Reg06h&0x04 )
		 {
			 DLOG_INFO("#### Port 1 Input Clock Change Detect ####\r\n");
		 }

		 if( Reg06h&0x02 )
		 {
			DLOG_INFO("#### Port 1 Rx CKOn Detect ####\r\n");
			//it6602->m_ucEccCount_P0=0;
			//it6602->m_ucDeskew_P0=0;
			it6602->m_ucDeskew_P1=0;
			it6602->m_ucEccCount_P1=0;

//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
//FIX_ID_032 xxxxx	//Support HDCP Repeater function for HDMI Tx device
#if 0

			if(CLKCheck(F_PORT_SEL_1))
			{
			TMDSCheck(F_PORT_SEL_1);
			}
#else
			// NO --> Authentication Start 	&& 	Input Clock Change Detect 	&&	 B_PORT1_TMDSEvent
			if(( Reg06h&0x08 )==0 && ( Reg06h&0x04 )==0  &&  (it6602->HDMIIntEvent & (B_PORT1_TMDSEvent))==0)
			{
					if(CLKCheck(F_PORT_SEL_1))
					{
						#ifdef _SUPPORT_AUTO_EQ_
						TMDSCheck(F_PORT_SEL_1);
						#endif
					}
			}
			else
			{
				if(( Reg06h&0x10 ) == 0)
				{
					if((it6602->HDMIIntEvent & (B_PORT1_Waiting))==0)
					{
						hdmirxwr(REG_RX_03A, 0x00);	// power down auto EQ
						it6602->HDMIIntEvent |= (B_PORT1_Waiting);
						it6602->HDMIIntEvent |= (B_PORT1_TMDSEvent);
						it6602->HDMIWaitNo[1]=MAX_TMDS_WAITNO;
					}
				}
				else
				{
					if(CLKCheck(F_PORT_SEL_1))
					{
						#ifdef _SUPPORT_AUTO_EQ_
						TMDSCheck(F_PORT_SEL_1);
						#else
						//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
						#ifdef _SUPPORT_EQ_ADJUST_
						HDMIStartEQDetect(&(it6602->EQPort[F_PORT_SEL_1]));
						#endif
						//FIX_ID_001 xxxxx
						#endif
					}
				}
			}
#endif
//FIX_ID_032 xxxxx
//FIX_ID_033 xxxxx


		}



		 if( Reg06h&0x01 )
		 {
			DLOG_INFO("#### Port 1 Power 5V change ####\r\n");
			hdmirx_INT_5V_Pwr_Chg(it6602,1);
			// disable ->//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
			// disable ->if(CheckPlg5VPwr(F_PORT_SEL_1)==FALSE)
			// disable ->{
			// disable ->	#ifdef _SUPPORT_EQ_ADJUST_
			// disable ->	DisableOverWriteRS(F_PORT_SEL_1);
			// disable ->	#endif
			// disable ->}
			// disable ->//FIX_ID_001 xxxxx
	 	}

     	}

	 if( Reg07h!=0x00)
	 {
		 //dlog_info("Reg07h = %X",(int) Reg07h);
		 if( Reg07h&0x80 ) {
			 DLOG_INFO("#### Audio FIFO Error ####\r\n");
			 aud_fiforst();
#ifdef EnableCalFs
//FIX_ID_023 xxxxx		//Fixed for Audio Channel Status Error with invalid HDMI source
			AudioFsCal();
//FIX_ID_023 xxxxx
#endif
		 }

		 if( Reg07h&0x40 ) {
			 DLOG_INFO("#### Audio Auto Mute ####\r\n");
		 }

		 if( Reg07h&0x20 ) {
			 DLOG_INFO("#### Packet Left Mute ####\r\n");
			 IT6602_SetVideoMute(it6602,OFF);
		 }

		 if( Reg07h&0x10 ) {
			 DLOG_INFO("#### Set Mute Packet Received ####\r\n");

			 IT6602_SetVideoMute(it6602,ON);
		 }

		 if( Reg07h&0x08 ) {
			 DLOG_INFO("#### Timer Counter Tntterrupt ####\r\n");
			//if(it6602->m_VState == VSTATE_VideoOn)
			//	hdmirxset(0x84,0x80,0x80);	//2011/06/17 xxxxx, for enable Rx Chip count

		 }

		 if( Reg07h&0x04 ) {
			 DLOG_INFO("#### Video Mode Changed ####\r\n");
		 }

		 if( Reg07h&0x02 ) {
			hdmirx_INT_SCDT_Chg(it6602);
		 }

		 if( Reg07h&0x01 ) {
			 if( (Reg0Ah&0x40)>>6 )
			 {
				 DLOG_INFO("#### Port 0 Bus Mode : MHL ####\r\n");

				//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
				if(HdmiI2cAddr==IT6602A0_HDMI_ADDR)
				{
					chgbank(1);
					hdmirxset(REG_RX_1B6,0x07,0x00);
					//FIX_ID_007 xxxxx 	//for debug IT6681  HDCP issue
					hdmirxset(REG_RX_1B1,0x20,0x20);//Reg1b1[5] = 1 for enable over-write
					hdmirxset(REG_RX_1B2,0x07,0x01);	// default 0x04 , change to 0x01
					DLOG_INFO(" Port 0 Bus Mode Reg1B1  = %X ,Reg1B2  = %X\r\n",(int) hdmirxrd(REG_RX_1B1),(int) hdmirxrd(REG_RX_1B2));
					//FIX_ID_007 xxxxx
					chgbank(0);
				}
				//FIX_ID_002 xxxxx

//FIX_ID_037 xxxxx //Allion MHL compliance issue !!!
					//xxxxx 2014-0522 disable for Allion MHL debug !!! --> 	it6602HPDCtrl(0,1);	// MHL port , set HPD = 1
//FIX_ID_037 xxxxx

			 }
			 else
			 {
				DLOG_INFO("#### Port 0 Bus Mode : HDMI ####\r\n");
				//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
					if(HdmiI2cAddr==IT6602A0_HDMI_ADDR)
					{
						chgbank(1);
						hdmirxset(REG_RX_1B6,0x07,0x03);
						////FIX_ID_007 xxxxx 	//for debug IT6681  HDCP issue
						hdmirxset(REG_RX_1B1,0x20,0x00);//Reg1b1[5] = 0 for disable over-write
						hdmirxset(REG_RX_1B2,0x07,0x04);	// default 0x04 , change to 0x01
						DLOG_INFO(" Port 0 Bus Mode Reg1B1  = %X ,Reg1B2  = %X\r\n",(int) hdmirxrd(REG_RX_1B1),(int) hdmirxrd(REG_RX_1B2));
						////FIX_ID_007 xxxxx
						chgbank(0);
					}
				//FIX_ID_002 xxxxx

			 }

		 };
	 }

	 if( Reg08h!=0x00)
	 {
		 //dlog_info("Reg08h = %X",(int) Reg08h);
		 if( Reg08h&0x80 ) {
			//			 DLOG_INFO("#### No General Packet 2 Received ####\n");
		 }

		 if( Reg08h&0x40 ) {
			//			 DLOG_INFO("#### No General Packet Received ####\n");
		 }

		 if( Reg08h&0x20 ) {
			 DLOG_INFO("#### No Audio InfoFrame Received ####\r\n");
		 }

		 if( Reg08h&0x10) {
			 DLOG_INFO("#### No AVI InfoFrame Received ####\r\n");
		 }

		 if( Reg08h&0x08 ) {
			 DLOG_INFO("#### CD Detect ####\r\n");

		 }

		 if( Reg08h&0x04 ) {
			//			 DLOG_INFO("#### Gen Pkt Detect ####\n");
			 DLOG_INFO("#### 3D InfoFrame Detect ####\r\n");

				#ifdef Enable_Vendor_Specific_packet
					if(it6602->f_de3dframe_hdmi == FALSE)
					{
					it6602->f_de3dframe_hdmi = IT6602_DE3DFrame(TRUE);
					}
				#endif

		 }

		 if( Reg08h&0x02 ) {
			 DLOG_INFO("#### ISRC2 Detect ####\r\n");
		 }

		 if( Reg08h&0x01 ) {
			 DLOG_INFO("#### ISRC1 Detect ####\r\n");
		 }
	 }

	 if( Reg09h!=0x00 )
	 {
        	 //dlog_info("Reg09h = %X",(int) Reg09h);
        	 if( Reg09h&0x80 )
		{
			 DLOG_INFO("#### H2V Buffer Skew Fail ####\r\n");
		 }

		 if( Reg09h&0x40 )
		 {

			//FIX_ID_002 xxxxx 	Check IT6602 chip version Identify for TogglePolarity and Port 1 Deskew
			if(HdmiI2cAddr==IT6602A0_HDMI_ADDR)
			{
				hdmirxwr(0x09, 0x20); //bug ~ need to update by Andrew
			}
			else
			{
				hdmirxwr(0x09, 0x40);
			}
			//FIX_ID_002 xxxxx
			DLOG_INFO("#### Port 1 Deskew Error ####\r\n");
			hdmirx_INT_P1_Deskew(it6602);
		 }

		 if( Reg09h&0x20 ) {
			 hdmirxwr(0x09, 0x20);
			 DLOG_INFO("#### Port 0 Deskew Error ####\r\n");
			hdmirx_INT_P0_Deskew(it6602);
		 }

		 if( Reg09h&0x10 ) {
			 DLOG_INFO("#### New Audio Packet Received ####\r\n");
		 }

		 if( Reg09h&0x08 ) {
			 DLOG_INFO("#### New ACP Packet Received ####\r\n");
		 }

		 if( Reg09h&0x04 ) {
			 DLOG_INFO("#### New SPD Packet Received ####\r\n");
		 }

		 if( Reg09h&0x02) {
			 DLOG_INFO("#### New MPEG InfoFrame Received ####\r\n");
		 }

		 if( Reg09h&0x01) {
			 DLOG_INFO("#### New AVI InfoFrame Received ####\r\n");
			//IT6602VideoOutputConfigure();
			it6602->m_NewAVIInfoFrameF=TRUE;
		 }

	 }


	if( RegD0h!=0x00 )
	{
// disable		if( RegD0h&0x08)
// disable		{
// disable			DLOG_INFO("#### Port 1 Rx Clock change detect Interrupt ####\n");
// disable		}
// disable
// disable		if( RegD0h&0x04)
// disable		{
// disable			DLOG_INFO("#### Port 0 Rx Clock change detect Interrupt ####\n");
// disable		}
	 //dlog_info("RegD0h = %X",(int) RegD0h);
	 if( RegD0h&0x10 )
	 {

		hdmirxwr(0xD0, 0x30);
		RegD0h&=0x30;
//FIX_ID_035 xxxxx //For MTK6592 HDMI to SII MHL TX compliance issue
		//xxxxx 2014-0508 disable ->		ucEqRetryCnt[0]=0;
//FIX_ID_035 xxxxx
		 DLOG_INFO("#### Port 0 EQ done interrupt ####\r\n");

		#ifdef _SUPPORT_AUTO_EQ_
		//2013-0923 disable ->	ucPortAMPOverWrite[0]=1;	//2013-0801
		AmpValidCheck(0);	//2013-0801
		#endif

//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
	#ifdef _SUPPORT_EQ_ADJUST_
	HDMIStartEQDetect(&(it6602->EQPort[F_PORT_SEL_0]));
	#endif
//FIX_ID_001 xxxxx

 }

	 if( RegD0h&0x40 )
	 {

	hdmirxwr(0xD0, 0xC0);
	RegD0h&=0xC0;
//FIX_ID_035 xxxxx //For MTK6592 HDMI to SII MHL TX compliance issue
	//xxxxx 2014-0508 disable ->	ucEqRetryCnt[1]=0;
//FIX_ID_035 xxxxx
	 DLOG_INFO("#### Port 1 EQ done interrupt ####\r\n");


	#ifdef _SUPPORT_AUTO_EQ_
	//2013-0923 disable ->	ucPortAMPOverWrite[1]=1;	//2013-0801
	AmpValidCheck(1);	//2013-0801
	#endif

	// disable -> //FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
	// disable -> 	#ifdef _SUPPORT_EQ_ADJUST_
	// disable -> 	HDMIStartEQDetect(&(it6602->EQPort[F_PORT_SEL_1]));
	// disable -> 	#endif
	// disable -> //FIX_ID_001 xxxxx
 }

	if( RegD0h&0x20)
	{

	hdmirxwr(0xD0, 0x20);
	DLOG_INFO("#### Port 0 EQ Fail Interrupt ####\r\n");
//	HDMICheckErrorCount(&(it6602->EQPort[F_PORT_SEL_0]));	//07-04 for port 0
//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
	#ifdef _SUPPORT_AUTO_EQ_
	hdmirx_INT_EQ_FAIL(it6602,F_PORT_SEL_0);
	#endif
//FIX_ID_001 xxxxx
}

	if( RegD0h&0x80)
		{

	hdmirxwr(0xD0, 0x80);
	DLOG_INFO("#### Port 1 EQ Fail Interrupt ####\r\n");
//	HDMICheckErrorCount(&(it6602->EQPort[F_PORT_SEL_1]));	//07-04 for port 0
//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
	#ifdef _SUPPORT_AUTO_EQ_
	hdmirx_INT_EQ_FAIL(it6602,F_PORT_SEL_1);
	#endif
//FIX_ID_001 xxxxx
}



	}

}
void IT6602_Interrupt(void)
{
	struct it6602_dev_data *it6602data = get_it6602_dev_data();
//	dlog_output(1000);
	IT6602HDMIInterruptHandler(it6602data);
//	dlog_output(1000);
}
void IT6602_fsm(void)
{
	struct it6602_dev_data *it6602data = get_it6602_dev_data();

#if defined(_IT6602_) || defined(_IT66023_)
	#ifdef SUPPORT_UART_CMD
	if(m_UartCmd == 0)
	#endif
	{

		#ifndef Enable_IR
		it6602AutoPortSelect(it6602data);
		#endif
	}
#endif

//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_
	IT6602MHLInterruptHandler(it6602data);
#endif
//FIX_ID_036	xxxxx

	//IT6602HDMIInterruptHandler(it6602data); MZY 17/5/5
	IT6602VideoHandler(it6602data);
#ifndef _FIX_ID_028_
//FIX_ID_028 xxxxx //For Debug Audio error with S2
	IT6602AudioHandler(it6602data);
//FIX_ID_028 xxxxx
#endif

//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_
	RCPManager(it6602data);
#endif
//FIX_ID_036	xxxxx

//FIX_ID_001 xxxxx Add Auto EQ with Manual EQ
        #ifdef _SUPPORT_EQ_ADJUST_
		if(it6602data->EQPort[F_PORT_SEL_0].f_manualEQadjust==TRUE)
			HDMIAdjustEQ(&(it6602data->EQPort[F_PORT_SEL_0]));	// for port 0

		if(it6602data->EQPort[F_PORT_SEL_1].f_manualEQadjust==TRUE)
			HDMIAdjustEQ(&(it6602data->EQPort[F_PORT_SEL_1]));	// for port 1
        #endif
//FIX_ID_001 xxxxx

//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_
//FIX_ID_005 xxxxx	//Add Cbus Event Handler
	IT6602CbusEventManager(it6602data);
//FIX_ID_005 xxxxx
#endif
//FIX_ID_036	xxxxx

//FIX_ID_033 xxxxx  //Fine-tune EQ Adjust function for HDCP receiver and repeater mode
//xxxxx 2014-0421 disable -> #ifndef _SUPPORT_HDCP_REPEATER_
//FIX_ID_032 xxxxx	//Support HDCP Repeater function for HDMI Tx device
//FIX_ID_014 xxxxx	//Add HDMI Event Handler
	IT6602HDMIEventManager(it6602data);
//FIX_ID_014 xxxxx
//FIX_ID_032 xxxxx
//xxxxx 2014-0421 disable -> #endif
//FIX_ID_033 xxxxx


	#ifdef Enable_IT6602_CEC
	CECManager();				// detect CEC for IT6602_CEC
	#endif
}
// disable ->
// disable -> static void it6602ShowErrorCount(void)
// disable -> {
// disable -> #ifndef _SUPPORT_EQ_ADJUST_	//20130401
// disable -> //	BYTE RegB4;
// disable -> //	BYTE RegB5;
// disable -> //	BYTE RegB9;
// disable -> //	BYTE RegBA;
// disable -> //
// disable -> //	RegB4 = hdmirxrd(REG_RX_0B4);
// disable -> //	RegB5 = hdmirxrd(REG_RX_0B5);
// disable -> //	RegB9 = hdmirxrd(REG_RX_0B9);
// disable -> //	RegBA = hdmirxrd(REG_RX_0BA);
// disable -> //
// disable -> //	if(OldRegB4!=RegB4)
// disable -> //	{
// disable -> //	DLOG_INFO("11111111 RegB4=%X \n",RegB4);
// disable -> //	OldRegB4=RegB4;
// disable -> //	}
// disable -> //
// disable -> //	if(OldRegB5!=RegB5)
// disable -> //	{
// disable -> //
// disable -> //	DLOG_INFO("22222222 RegB5=%X \n",RegB5);
// disable -> //	OldRegB5=RegB5;
// disable -> //	}
// disable -> //
// disable -> //	if(OldRegB9!=RegB9)
// disable -> //	{
// disable -> //	DLOG_INFO("33333333 RegB9=%X \n",RegB9);
// disable -> //	OldRegB9=RegB9;
// disable -> //	}
// disable -> //
// disable -> //	if(OldRegBA!=RegBA)
// disable -> //	{
// disable -> //	DLOG_INFO("44444444 RegBA=%X \n",RegBA);
// disable -> //	OldRegBA=RegBA;
// disable -> //	}
// disable -> //
// disable -> #endif
// disable -> }

#if defined(_IT6602_) || defined(_IT66023_)
static void it6602AutoPortSelect(struct it6602_dev_data *it6602)
{

	if(SEL_PORT_1==1)
	{
		if(it6602->m_ucCurrentHDMIPort != 0)
		{
			it6602PortSelect(0);
		}
	}
	else
		{
			if(it6602->m_ucCurrentHDMIPort == 0)
			{
				it6602PortSelect(1);
			}

		}

}
#endif

void get_vid_info( void )
{
#if 1
	int HSyncPol, VSyncPol, InterLaced;
	int HTotal, HActive, HFP, HSYNCW;
	int VTotal, VActive, VFP, VSYNCW;
//	int rddata;
//	int i;
//	unsigned long PCLK, sump;
	unsigned int ucTMDSClk;	//, sumt;
	unsigned char ucPortSel;
	unsigned char rddata;
	unsigned char ucClk;
	int PCLK;	//, sump;



	rddata = hdmirxrd(0x9A);
	PCLK=(124*255/rddata)/10;


	ucPortSel = hdmirxrd(REG_RX_051) & B_PORT_SEL;
	rddata = hdmirxrd(0x90);

	if(ucPortSel == F_PORT_SEL_1)
	{

			DLOG_INFO("Reg51[0] = 1 Active Port HDMI \r\n");
			ucClk = hdmirxrd(REG_RX_092) ;
			if(ucClk != 0 )
			{

				if( rddata&0x04 )
					ucTMDSClk=2*RCLKVALUE*256/ucClk;
				else if( rddata&0x08 )
					ucTMDSClk=4*RCLKVALUE*256/ucClk;
				else
					ucTMDSClk=RCLKVALUE*256/ucClk;

				DLOG_INFO(" Port 1 TMDS CLK  = %d \r\n",(int)ucTMDSClk);
			}
			//DLOG_INFO(" HDMI Reg92  = %X \r\n",(int) hdmirxrd(0x92));
			//DLOG_INFO(" HDMI Reg38  = %X \r\n",(int) hdmirxrd(0x38));
	}
	else
	{
			DLOG_INFO("Reg51[0] = 0 Active Port MHL \r\n");
			ucClk = hdmirxrd(REG_RX_091) ;
			if(ucClk != 0 )
			{
				if( rddata&0x01 )
					ucTMDSClk=2*RCLKVALUE*256/ucClk;
				else if( rddata&0x02 )
					ucTMDSClk=4*RCLKVALUE*256/ucClk;
				else
					ucTMDSClk=RCLKVALUE*256/ucClk;

				DLOG_INFO("Port 0 TMDS CLK  = %d \r\n",(int)ucTMDSClk);
			}
			//DLOG_INFO(" HDMI Reg91  = %X \r\n",(int) hdmirxrd(0x91));
			//DLOG_INFO(" HDMI Reg20  = %X \r\n",(int) hdmirxrd(0x20));

	}


	InterLaced = (hdmirxrd(0x99)&0x02)>>1;

	HTotal   = ((hdmirxrd(0x9D)&0x3F)<<8) + hdmirxrd(0x9C);
	HActive  = ((hdmirxrd(0x9F)&0x3F)<<8) + hdmirxrd(0x9E);
	HFP      = ((hdmirxrd(0xA1)&0xF0)<<4) + hdmirxrd(0xA2);
	HSYNCW   = ((hdmirxrd(0xA1)&0x01)<<8) + hdmirxrd(0xA0);
	HSyncPol = hdmirxrd(0xA8)&0x04>>2;

	VTotal   = ((hdmirxrd(0xA4)&0x0F)<<8) + hdmirxrd(0xA3);
	VActive  = ((hdmirxrd(0xA4)&0xF0)<<4) + hdmirxrd(0xA5);
	VFP      = hdmirxrd(0xA7)&0x3F;
	VSYNCW   = hdmirxrd(0xA6)&0x1F;
	VSyncPol = (hdmirxrd(0xA8)&0x08)>>3;

//	CurVTiming.TMDSCLK     = (int)TMDSCLK;
	CurTMDSCLK             = (int)ucTMDSClk;
	CurVTiming.PCLK        = (int)PCLK;
	CurVTiming.HActive     = HActive;
	CurVTiming.HTotal      = HTotal;
	CurVTiming.HFrontPorch = HFP;
	CurVTiming.HSyncWidth  = HSYNCW;
	CurVTiming.HBackPorch  = HTotal - HActive - HFP - HSYNCW;
	CurVTiming.VActive     = VActive;
	CurVTiming.VTotal      = VTotal;
	CurVTiming.VFrontPorch = VFP;
	CurVTiming.VSyncWidth  = VSYNCW;
	CurVTiming.VBackPorch  = VTotal - VActive - VFP - VSYNCW;
	CurVTiming.ScanMode    = (InterLaced)&0x01;
	CurVTiming.VPolarity   = (VSyncPol)&0x01;
	CurVTiming.HPolarity   = (HSyncPol)&0x01;
#endif
}



void show_vid_info( void )
{
#if 1
	int InBPC, InBPP;
	int MHL_Mode;
	int MHL_CLK_Mode;
	int GCP_CD       = CD8BIT; //24 bits per pixel

	unsigned long FrameRate;

	GCP_CD = ((hdmirxrd(0x99)&0xF0)>>4);

	switch( GCP_CD ) {
	case 5 :
	DLOG_INFO("I/P ColorDepth = 30 bits per pixel \r\n");
	InBPC=10;
	hdmirxset(0x65, 0x0C, 0x04);
	OutCD = OUT10B;
	break;
	case 6 :
	DLOG_INFO("I/P ColorDepth = 36 bits per pixel \r\n");
	InBPC=12;
	hdmirxset(0x65, 0x0C, 0x08);
	OutCD = OUT12B;
	break;
	default :
	DLOG_INFO("I/P ColorDepth = 24 bits per pixel \r\n");
	InBPC=8;
	hdmirxset(0x65, 0x0C, 0x00);
	OutCD = OUT8B;
	break;
	}

	switch( OutCD ) {
	case 1 :
	DLOG_INFO("O/P ColorDepth = 30 bits per pixel \r\n");
	break;
	case 2 :
	DLOG_INFO("O/P ColorDepth = 36 bits per pixel \r\n");
	break;
	default :
	DLOG_INFO("O/P ColorDepth = 24 bits per pixel \r\n");
	break;
	}

	chgbank(2);
	InColorMode = (hdmirxrd(0x15)&0x60)>>5;
	chgbank(0);

	if( InColorMode==1 ) { //YCbCr422
	InBPP = InBPC*2;
	}
	else
	{
	InBPP = InBPC*3;
	}

	switch( InColorMode ) {
	case 0 :
	DLOG_INFO("Input Color Mode = RGB444 \n");
	//		 hdmirxset(0xAE, 0x01, 0x01);
	//		 defaultrgb();
	break;
	case 1 :
	DLOG_INFO("Input Color Mode = YCbCr422\n");
	//		 hdmirxset(0xAE, 0x01, 0x00);
	//		 yuv422torgb();
	break;
	case 2 :
	DLOG_INFO("Input Color Mode = YCbCr444\n");
	//		 hdmirxset(0xAE, 0x01, 0x00);
	//		 yuv444torgb();
	break;
	default :
	DLOG_INFO("Input Color Mode = Reserved !!!\n");
	break;
	}


	OutColorMode = (hdmirxrd(0x65)&0x30)>>4;
	switch( OutColorMode ) {
	case 0 :
	DLOG_INFO("Output Color Mode = RGB444\n");
	//		 hdmirxset(0x65, 0x30, 0x00);
	break;
	case 1 :
	DLOG_INFO("Output Color Mode = YCbCr422\n");
	//		 hdmirxset(0x65, 0x30, 0x10);
	break;
	case 2 :
	DLOG_INFO("Output Color Mode = YCbCr444\n");
	//		 hdmirxset(0x65, 0x30, 0x20);
	break;
	default :
	DLOG_INFO("Output Color Mode = Reserved !!!\n");
	break;
	}


	//    DLOG_INFO("Video Input Timing: %s\n", s_VMTable[VIC].format);
	//    DLOG_INFO("TMDSCLK = %3.3fMHz\n", (unsigned long)(CurTMDSCLK)/1000);
	//    DLOG_INFO("PCLK = %3.3fMHz\n", (unsigned long)(CurVTiming.PCLK)/1000);

	DLOG_INFO("HFrontPorch = %d\n", CurVTiming.HFrontPorch);
	DLOG_INFO("HSyncWidth = %d\n", CurVTiming.HSyncWidth);
	DLOG_INFO("HBackPorch = %d\n", CurVTiming.HBackPorch);
	DLOG_INFO("VFrontPorch = %d\n", CurVTiming.VFrontPorch);
	DLOG_INFO("VSyncWidth = %d\n", CurVTiming.VSyncWidth);
	DLOG_INFO("VBackPorch = %d\n", CurVTiming.VBackPorch);

	FrameRate = (unsigned long)(CurVTiming.PCLK)*1000*1000;
	FrameRate /= CurVTiming.HTotal;
	FrameRate /= CurVTiming.VTotal;
	DLOG_INFO("FrameRate = %ld Hz\n", FrameRate);
	if( CurVTiming.ScanMode==0 ) {
	DLOG_INFO("ScanMode = Progressive\n");
	}
	else {
	DLOG_INFO("ScanMode = InterLaced\n");
	}

	if( CurVTiming.VPolarity==1 ) {
	DLOG_INFO("VSyncPol = Positive\n");
	}
	else {
	DLOG_INFO("VSyncPol = Negative\n");
	}

	if( CurVTiming.HPolarity==1 ) {
	DLOG_INFO("HSyncPol = Positive\n");
	}
	else {
	DLOG_INFO("HSyncPol = Negative\n");
	}

	if(((hdmirxrd(0x51)&0x01)))
	{
		DLOG_INFO("Port= 1 ,Reg18=%X ,",(int)hdmirxrd(REG_RX_018));
		DLOG_INFO("Reg38=%X, ",(int)hdmirxrd(REG_RX_038));
		DLOG_INFO("Reg3E=%X, ",(int)hdmirxrd(REG_RX_03E));
		DLOG_INFO("Reg3F=%X, ",(int)hdmirxrd(REG_RX_03F));
		DLOG_INFO("Reg40=%X \r\n",(int)hdmirxrd(REG_RX_040));
		DLOG_INFO("Reg41=%X \r\n",(int)hdmirxrd(REG_RX_041));
		chgbank(1);
		DLOG_INFO("Rec_B_CS=%X  ",(int)(hdmirxrd(REG_RX_1DD)&0x80)>>7);
		DLOG_INFO("Rec_G_CS=%X  ",(int)(hdmirxrd(REG_RX_1DE)&0x80)>>7);
		DLOG_INFO("Rec_R_CS=%X  \n",(int)(hdmirxrd(REG_RX_1DF)&0x80)>>7);
		DLOG_INFO("Rec_B_RS=%X  ",(int)(hdmirxrd(REG_RX_1DD)&0x7F));
		DLOG_INFO("Rec_G_RS=%X  ",(int)(hdmirxrd(REG_RX_1DE)&0x7F));
		DLOG_INFO("Rec_R_RS=%X  \n",(int)(hdmirxrd(REG_RX_1DF)&0x7F));
		DLOG_INFO(" Reg1C1  = %X , Reg1C2  = %X\r\n",(int)hdmirxrd(REG_RX_1C1),(int)hdmirxrd(REG_RX_1C2));
		chgbank(0);
	}
	else
	{

		DLOG_INFO("Port= 0 ,Reg11=%X ,",(int)hdmirxrd(REG_RX_011));
		DLOG_INFO("Reg20=%X, ",(int)hdmirxrd(REG_RX_020));
		DLOG_INFO("Reg26=%X, ",(int)hdmirxrd(REG_RX_026));
		DLOG_INFO("Reg27=%X, ",(int)hdmirxrd(REG_RX_027));
		DLOG_INFO("Reg28=%X, ",(int)hdmirxrd(REG_RX_028));
		DLOG_INFO("Reg29=%X \r\n",(int)hdmirxrd(REG_RX_029));
		chgbank(1);
		DLOG_INFO("Rec_B_CS=%X  ",(int)(hdmirxrd(REG_RX_1D5)&0x80)>>7);
		 DLOG_INFO("Rec_G_CS=%X  ",(int)(hdmirxrd(REG_RX_1D6)&0x80)>>7);
		 DLOG_INFO("Rec_R_CS=%X  \n",(int)(hdmirxrd(REG_RX_1D7)&0x80)>>7);

		 DLOG_INFO("Rec_B_RS=%X  ",(int)(hdmirxrd(REG_RX_1D5)&0x7F));
		 DLOG_INFO("Rec_G_RS=%X  ",(int)(hdmirxrd(REG_RX_1D6)&0x7F));
		 DLOG_INFO("Rec_R_RS=%X  \n",(int)(hdmirxrd(REG_RX_1D7)&0x7F));
		DLOG_INFO("REG_RX_1B1 = %X ,  REG_RX_1B2 = %X\r\n",(int)hdmirxrd(REG_RX_1B1),(int)hdmirxrd(REG_RX_1B2));


		chgbank(0);
	}

	DLOG_INFO("TMDSCLK = %d MHz\n", (int)(CurTMDSCLK));
	DLOG_INFO("PCLK = %d MHz\n", (int)(CurVTiming.PCLK));
	DLOG_INFO("HActive = %d\n", CurVTiming.HActive);
	DLOG_INFO("VActive = %d\n", CurVTiming.VActive);
	DLOG_INFO("HTotal = %d\n", CurVTiming.HTotal);
	DLOG_INFO("VTotal = %d\n", CurVTiming.VTotal);

//FIX_ID_036	xxxxx //Enable MHL Function for IT68XX
#ifdef _ENABLE_IT68XX_MHL_FUNCTION_
	MHL_Mode     = ((hdmirxrd(0x0A)&0x40)>>6);
	MHL_CLK_Mode = ((mhlrxrd(0xB1)&0x07));
#else
	MHL_Mode = 0;
#endif
//FIX_ID_036	xxxxx
	if( MHL_Mode )
	{
		if( MHL_CLK_Mode==0x02 )
		DLOG_INFO("BUS MODE : MHL PackPixel Mode\n");
		else
		DLOG_INFO("BUS MODE : MHL 24 bits Mode\n");
	}
	if(IsHDMIMode())
	{
		DLOG_INFO("HDMI/DVI Mode : HDMI \n");
	}
	else
	{
		DLOG_INFO("HDMI/DVI Mode : DVI \n");
	}
#endif

}

#endif


/*****************************************************************************/
/* Power Control Functions  **************************************************/
/*****************************************************************************/


/*********************************************************************************/
/* End of ITEHDMI.c ***************************************************************/
/*********************************************************************************/


#ifdef Enable_Vendor_Specific_packet

#define HDMI_3DFORMAT_PRESENT           0x40
#define HDMI_3DFORMAT_OFF               0x00
#define FRAME_PACKING                   0x00
#define TOP_AND_BOTTOM                  0x60
#define SIDE_BY_SIDE                    0x80


SET_DE3D_FRAME t_3d_syncgen[] =
{
    //640x480      //524   //559   //514   //526
    {0x01      ,0x020C  ,0x022F  ,0x0202  ,0x020E,	480}, // 60Hz
    //480p      //524   //560   //515   //530
    {0x02      ,0x020C  ,0x0230  ,0x0203  ,0x0212,	480}, // 60Hz
    {0x03      ,0x020C  ,0x0230  ,0x0203  ,0x0212,	480}, // 60Hz
    //576p      //624   //668   //619   //629
    {0x11      ,0x0270  ,0x029C  ,0x026B  ,0x0275,	576}, // 50Hz
    {0x12      ,0x0270  ,0x029C  ,0x026B  ,0x0275,	576}, // 50Hz
    //720p      //749   //774   //744   //754
    {0x3c      ,0x02ED  ,0x0306  ,0x02E8  ,0x02F2,	720}, // 24Hz
    {0x3d      ,0x02ED  ,0x0306  ,0x02E8  ,0x02F2,	720}, // 25Hz
    {0x3e      ,0x02ED  ,0x0306  ,0x02E8  ,0x02F2,	720}, // 30Hz
    {0x13      ,0x02ED  ,0x0306  ,0x02E8  ,0x02F2,	720}, // 50Hz
    {0x04      ,0x02ED  ,0x0306  ,0x02E8  ,0x02F2,	720}, // 60Hz

//disable -> 1080i     //1124   //1165   //1120   //1129
//disable ->     {0x05      ,0x0464  ,0x048D  ,0x0460  ,0x0469,	540}, // 50Hz
//disable ->     {0x14      ,0x0464  ,0x048D  ,0x0460  ,0x0469,	540}, // 60Hz
//disable -> 1080i     //1124   //1165   //1120   //1129
//disable ->     {0x20      ,0x0464  ,0x048D  ,0x0460  ,0x0469,	540}, // 24Hz
//disable ->     {0x22      ,0x0464  ,0x048D  ,0x0460  ,0x0469,	540}, // 30Hz
//disable ->     {0x1f      ,0x0464  ,0x048D  ,0x0460  ,0x0469,	540}, // 50Hz
//disable ->     {0x10      ,0x0464  ,0x048D  ,0x0460  ,0x0469,	540}, // 60Hz

    //1080p    //1124   //1165   //1120   //1129
    {0x20      ,0x0464  ,0x048D  ,0x0460  ,0x0469,	1080}, // 24Hz
    {0x21      ,0x0464  ,0x048D  ,0x0460  ,0x0469,	1080}, // 25Hz
    {0x22      ,0x0464  ,0x048D  ,0x0460  ,0x0469,	1080}, // 30Hz

    //default
    {0xFF      ,0x0000  ,0x0000  ,0x0000  ,0x0000,0x0000}
};
//Reg_PGVTotal	749		// 0x2ED
//Reg_PGVActst 	774		// 0x306
//Reg_PGVActEd	744		// 0x2E8
//Reg_PGVsyncEd	754		// 0x2F2
#define Reg_PGVTotal_19D	0x9D	//[11:4]	0x19D[7:0]
#define Reg_PGVTotal_19C	0x9C	//[3:0]		0x19C[7:4]
#define Reg_PGVActSt_192	0x92	//[7:0]		0x192[7:0]
#define Reg_PGVActSt_193	0x93	//[11:8]	0x193[3:0]
#define Reg_PGVActEd_193	0x93	//[3:0]		0x193[7:4]
#define Reg_PGVActEd_194	0x94	//[11:4]	0x194[7:0]
#define Reg_PGVSyncEd_19F	0x9F	//[3:0]		0x19F[7:4]
#define Reg_PGVSyncSt_19F	0x9F	//[11:8]	0x19F[3:0]
#define Reg_PGVSyncSt_19E	0x9E	//[7:0]		0x19E[7:0]

#define Reg_PG3DRSt_18F		0x8F	//[7:0]		0x190[11:8] 0x18F[7:0]
#define Reg_PG3DRStEd_190	0x90	//[7:0]		0x191[3:0] 0x18F[11:8]
#define Reg_PG3DREd_191	0x91	//[11:4]		0x191[11:4] 0x190[3:0]

#define REG_RX_066_4_DE3DFrame	0x66	//[4] 1: 3D frame-packet mode to sequence mode
#define REG_RX_085_5_En3DROut 		0x85	//[5] 1: Enable 3DR output
//
//pccmd w 0f 01 94
//pccmd w 8f 86 94
//pccmd w 90 41 94
//pccmd w 91 47 94
//pccmd w 92 06 94
//pccmd w 93 83 94
//pccmd w 94 2E 94
//pccmd w 9c d0 94
//pccmd w 9d 2e 94
//pccmd w 9f 22 94
//pccmd w 0f 00 94
//pccmd w 66 58 94

static void Dump3DReg(void)
{
    ushort	i,j ;
    BYTE ucData ;

    DLOG_INFO("\r\n       ");
    for(j = 0 ; j < 16 ; j++)
    {
        DLOG_INFO(" %02X",(int) j);
        if((j == 3)||(j==7)||(j==11))
        {
                DLOG_INFO(" :");
        }
    }
   DLOG_INFO("\r\n");

    chgbank(1);

    for(i = 0x80 ; i < 0xa0 ; i+=16)
    {
        DLOG_INFO("[%03X]  ",i);
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = hdmirxrd((BYTE)((i+j)&0xFF)) ;
            DLOG_INFO(" %02X",(int) ucData);
            if((j == 3)||(j==7)||(j==11))
            {
                DLOG_INFO(" :");
            }
        }
        DLOG_INFO("\r\n");

    }

   DLOG_INFO("\n        =====================================================\r\n");

    chgbank(0);
}

static unsigned char IT6602_DE3DFrame(unsigned char ena_de3d)
/*
 * This function configures the HDMI DE3DFrame
 * @param uunsigned char ena_de3d
 * @return      TRUE
 *              FALSE
 */
{
	unsigned char i, uc;
	unsigned int v_total;
	unsigned int v_act_start;
	unsigned int v_act_end;
	unsigned int v_sync_end;
	unsigned int v_act_bspace;
	unsigned int v_2d_Vtotal;
	unsigned int HActive;
	unsigned int LR_3D_Start;
	unsigned int LR_3D_End;

    #ifdef DEBUG_MODE
    //dbmsg_trace(DBM_DPATH,"ITEHDMI - HDMI_DE3DFrame \r\n");
    #endif

	struct it6602_dev_data *it6602data = get_it6602_dev_data();

    if(ena_de3d  == TRUE)
    {

 	chgbank(2);
	uc=hdmirxrd(REG_RX_224);
 	chgbank(0);
        if(uc == 0x81)	 // 3D InfoFrame Packet Type is valid
        {

 	chgbank(2);
	it6602data->s_Current3DFr.VIC=hdmirxrd(REG_RX_218);	//AVI INFO PB4
	it6602data->s_Current3DFr.HB0=hdmirxrd(REG_RX_224);	// General Packet Header Byte 0
	it6602data->s_Current3DFr.HB1=hdmirxrd(REG_RX_225);
	it6602data->s_Current3DFr.HB2=hdmirxrd(REG_RX_226);
	it6602data->s_Current3DFr.PB0=hdmirxrd(REG_RX_227);	// General Packet Data Byte 0
	it6602data->s_Current3DFr.PB1=hdmirxrd(REG_RX_228);
	it6602data->s_Current3DFr.PB2=hdmirxrd(REG_RX_229);
	it6602data->s_Current3DFr.PB3=hdmirxrd(REG_RX_22A);
	it6602data->s_Current3DFr.PB4=hdmirxrd(REG_RX_22B);
	it6602data->s_Current3DFr.PB5=hdmirxrd(REG_RX_22C);
	it6602data->s_Current3DFr.PB6=hdmirxrd(REG_RX_22D);
	it6602data->s_Current3DFr.PB7=hdmirxrd(REG_RX_22E);
 	chgbank(0);

//#ifdef DEBUG_MODE_3D
    DLOG_INFO("\r\nIT653x - HDMI_DumpDE3DFrameInfo: \r\n");
    DLOG_INFO("        HDMI VIC = 0x%X \r\n",it6602data->s_Current3DFr.VIC);
    DLOG_INFO("        Record HDMI vender specific inforframe HB0 = 0x%X \r\n",(int) it6602data->s_Current3DFr.HB0);
    DLOG_INFO("        Record HDMI vender specific inforframe HB1 = 0x%X \r\n",(int) it6602data->s_Current3DFr.HB1);
    DLOG_INFO("        Record HDMI vender specific inforframe HB2 = 0x%X \r\n",(int) it6602data->s_Current3DFr.HB2);
    DLOG_INFO("        Record HDMI vender specific inforframe PB0 = 0x%X \r\n",(int) it6602data->s_Current3DFr.PB0);
    DLOG_INFO("        Record HDMI vender specific inforframe PB1 = 0x%X \r\n",(int) it6602data->s_Current3DFr.PB1);
    DLOG_INFO("        Record HDMI vender specific inforframe PB2 = 0x%X \r\n",(int) it6602data->s_Current3DFr.PB2);
    DLOG_INFO("        Record HDMI vender specific inforframe PB3 = 0x%X \r\n",(int) it6602data->s_Current3DFr.PB3);
    DLOG_INFO("        Record HDMI vender specific inforframe PB4 = 0x%X \r\n",(int) it6602data->s_Current3DFr.PB4);
    DLOG_INFO("        Record HDMI vender specific inforframe PB5 = 0x%X \r\n",(int) it6602data->s_Current3DFr.PB5);
    DLOG_INFO("        Record HDMI vender specific inforframe PB6 = 0x%X \r\n",(int) it6602data->s_Current3DFr.PB6);
    DLOG_INFO("        Record HDMI vender specific inforframe PB7 = 0x%X \r\n",(int) it6602data->s_Current3DFr.PB7);
//#endif



            /******************************  3D integration  *************************************/

            it6602data->de3dframe_config.LR_Reference             =  2; // Source of the 3D L/R reference.
            it6602data->de3dframe_config.FrameDominance           =  0; // Left or Right Eye is first in L/R image pair.
            it6602data->de3dframe_config.LR_Encoding              =  1; // Type of 3D L/R encoding
            it6602data->de3dframe_config.TB_Reference             =  2; // Top/Bottom reference for vertically sub-sampled sources
            it6602data->de3dframe_config.OE_Reference             =  2; // Odd/Even reference for horizontally sub-sampled sources

            it6602data->de3dframe_config.NumActiveBlankLines      =  0; // Number of lines separating vertically packed L/R data to be removed (cropped)before being displayed
            it6602data->de3dframe_config.NumberOfEncodedLines     =  0; // Number of encoded lines in one L/R eye frame of the display data to be blanked out with "Blanking Color".
            it6602data->de3dframe_config.LeftEncodedLineLocation  = -1; // Active line number of 1st encoded line in one Left eye frame of the display data (-1=unknown).
            it6602data->de3dframe_config.RightEncodedLineLocation = -1; // Active line number of 1st encoded line in one Right eye frame of the display data (-1=unknown).
            it6602data->de3dframe_config.BlankingColor            =  7; // Color to use when blanking (or masking off) any embedded L/R encoding

            if(((it6602data->s_Current3DFr.PB4&0xE0) == HDMI_3DFORMAT_PRESENT) && ((it6602data->s_Current3DFr.PB5&0xF0) == FRAME_PACKING))
            {
                i =0;

                while(t_3d_syncgen[i].Vic != 0xFF)
                {
                    if(t_3d_syncgen[i].Vic == it6602data->s_Current3DFr.VIC)
                    {
                        break;
                    }
                    i++;
                }
		v_total     = t_3d_syncgen[i].V_total;
		v_act_start = t_3d_syncgen[i].V_act_start;
		v_act_end   = t_3d_syncgen[i].V_act_end;
		v_sync_end  = t_3d_syncgen[i].V_sync_end;
		v_2d_Vtotal = t_3d_syncgen[i].V_2D_active_total;
		chgbank(1);
		hdmirxset(Reg_PGVTotal_19D, 0xFF, (unsigned char) ((v_total & 0xFF0)>>4));			 //pccmd w 9d 2e
		hdmirxset(Reg_PGVTotal_19C, 0xF0, (unsigned char) ((v_total & 0x00F)<<4));			//pccmd w 9c d0
		hdmirxset(Reg_PGVActSt_192, 0xFF, (unsigned char) ((v_act_start & 0x0FF)));			//pccmd w 92 06
		hdmirxset(Reg_PGVActSt_193, 0x0F, (unsigned char) ((v_act_start & 0xF00)>>8));		//pccmd w 93 83
		hdmirxset(Reg_PGVActEd_193, 0xF0, (unsigned char) ((v_act_end & 0x00F)<<4));		//pccmd w 93 83
		hdmirxset(Reg_PGVActEd_194, 0xFF, (unsigned char) ((v_act_end & 0xFF0)>>4));		//pccmd w 94 2E
		hdmirxset(Reg_PGVSyncEd_19F, 0xF0, (unsigned char) ((v_sync_end & 0x00F)<<4));	//pccmd w 9f 22


#if 1
		LR_3D_Start = (v_act_start - (v_2d_Vtotal/2));
		LR_3D_End =(v_act_start + (v_2d_Vtotal/2));
#else
		LR_3D_Start = ((v_total/2));
		LR_3D_End =(LR_3D_Start*3);
#endif

		hdmirxset(Reg_PG3DRSt_18F, 0xFF, (unsigned char) ((LR_3D_Start & 0x0FF)));
		hdmirxset(Reg_PG3DRStEd_190, 0x0F, (unsigned char) ((LR_3D_Start & 0xF00)>>8));
		hdmirxset(Reg_PG3DRStEd_190, 0xF0, (unsigned char) ((LR_3D_End & 0x00F)<<4));
		hdmirxset(Reg_PG3DREd_191, 0xFF, (unsigned char) ((LR_3D_End & 0xFF0)>>4));

		DLOG_INFO("\nv_total = %X or %d \r\n",(int)  (v_total), (int)  (v_total));
		DLOG_INFO("Reg_PGVTotal_19D = %X \r\n",(int)  (hdmirxrd(Reg_PGVTotal_19D)));
		DLOG_INFO("Reg_PGVTotal_19C = %X \r\n",(int)  (hdmirxrd(Reg_PGVTotal_19C)));
		DLOG_INFO("\nv_act_start = %X or %d \r\n",(int)  (v_act_start),(int)  (v_act_start));
		DLOG_INFO("Reg_PGVActSt_192 = %X \r\n",(int)  (hdmirxrd(Reg_PGVActSt_192)));
		DLOG_INFO("Reg_PGVActSt_193 = %X \r\n",(int)  (hdmirxrd(Reg_PGVActSt_193)));
		DLOG_INFO("\nv_act_end = %X or %d \r\n",(int)  (v_act_end),(int)  (v_act_end));
		DLOG_INFO("Reg_PGVActEd_193 = %X \r\n",(int)  (hdmirxrd(Reg_PGVActEd_193)));
		DLOG_INFO("Reg_PGVActEd_194 = %X \r\n",(int)  (hdmirxrd(Reg_PGVActEd_194)));
		DLOG_INFO("\nv_sync_end = %X or %d \r\n",(int)  (v_sync_end),(int)  (v_sync_end));
		DLOG_INFO("Reg_PGVSyncEd_19F = %X \r\n",(int)  (hdmirxrd(Reg_PGVSyncEd_19F)));

		DLOG_INFO("LR_3D_Start = %X or %d  \r\n",(int)  (LR_3D_Start),(int)  (LR_3D_Start));
		DLOG_INFO("Reg_PG3DRSt_18F = %X \r\n",(int)  (hdmirxrd(Reg_PG3DRSt_18F)));
		DLOG_INFO("Reg_PG3DRStEd_190 = %X \r\n",(int)  (hdmirxrd(Reg_PG3DRStEd_190)));
		DLOG_INFO("Reg_PG3DREd_191 = %X \r\n",(int)  (hdmirxrd(Reg_PG3DREd_191)));
		DLOG_INFO("LR_3D_End = %X or %d  \r\n",(int)  (LR_3D_End),(int)  (LR_3D_End));

		DLOG_INFO("\n\nv_total = %X or %d \r\n",(int)  (v_total), (int)  (v_total));
		DLOG_INFO("v_act_start = %X or %d \r\n",(int)  (v_act_start),(int)  (v_act_start));
		DLOG_INFO("v_act_end = %X or %d \r\n",(int)  (v_act_end),(int)  (v_act_end));
		DLOG_INFO("v_sync_end = %X or %d \r\n",(int)  (v_sync_end),(int)  (v_sync_end));
		DLOG_INFO("LR_3D_Start = %X or %d  \r\n",(int)  (LR_3D_Start),(int)  (LR_3D_Start));
		DLOG_INFO("LR_3D_End = %X or %d  \r\n",(int)  (LR_3D_End),(int)  (LR_3D_End));

		chgbank(0);
		hdmirxset(REG_RX_066_4_DE3DFrame, 0x10, 0x10);		// Reg66[4] = 1 for enable 3D FP2FS
		hdmirxset(REG_RX_085_5_En3DROut, 0x20, 0x20);			// Reg85[5] = 1 for enable 3DR output


		Dump3DReg();


		// enable output
		HActive  = ((hdmirxrd(0x9F)&0x3F)<<8) + hdmirxrd(0x9E);
		//ChangePicoResolution(HActive,v_2d_Vtotal);
                v_act_bspace = v_act_start - v_act_end;
            }

            if(((it6602data->s_Current3DFr.PB4&0xE0) == HDMI_3DFORMAT_PRESENT) && (!it6602data->DE3DFormat_HDMIFlag))
            {
                it6602data->DE3DFormat_HDMIFlag = TRUE;
            }

            if(((it6602data->s_Current3DFr.PB4&0xE0) == HDMI_3DFORMAT_PRESENT) && (it6602data->DE3DFormat_HDMIFlag))
            {
                if(((it6602data->s_Current3DFr.PB5&0xF0) == FRAME_PACKING) && (!it6602data->FramePacking_Flag))
                {
                    it6602data->FramePacking_Flag   = TRUE;
                    it6602data->TopAndBottom_Flag   = FALSE;
                    it6602data->SideBySide_Flag     = FALSE;
                    it6602data->oldVIC              = 0;
                }

                if(((it6602data->s_Current3DFr.PB5&0xF0) == FRAME_PACKING) && (it6602data->FramePacking_Flag))
                {
                    it6602data->newVIC = it6602data->s_Current3DFr.VIC;
                    if(it6602data->newVIC != it6602data->oldVIC)
                    {
                        if((it6602data->s_Current3DFr.VIC == 0x3c) || (it6602data->s_Current3DFr.VIC ==0x3e) || (it6602data->s_Current3DFr.VIC == 0x13) ||
                           (it6602data->s_Current3DFr.VIC == 0x04) ||(it6602data->s_Current3DFr.VIC ==0x20) || (it6602data->s_Current3DFr.VIC == 0x22))
                           //(it6602data->s_Current3DFr.VIC == 0x05) ||(it6602data->s_Current3DFr.VIC == 0x14) // 1080i@50&60Hz not supported for frame packing
                        {
                            it6602data->de3dframe_config.NumActiveBlankLines  =  (unsigned char)v_act_bspace;
                            it6602data->de3dframe_config.Format = VERT_PACKED_FULL; // Type of 3D source format is FRAME_PACKING(VERT_PACKED_FULL)

                            #ifdef DEBUG_MODE_3D
                            dbmsg_trace(DBM_3D,"ITEHDMI - HDMI_3DFORMAT is FRAME_PACKING \r\n");
				   #else
				DLOG_INFO("ITEHDMI - HDMI_3DFORMAT is FRAME_PACKING \r\n");

                            #endif
                        }
                        else
                        {
                            it6602data->de3dframe_config.Format    =  6; // Type of 3D source format is UNDEFINED_FORMAT

                            #ifdef DEBUG_MODE_3D
                            dbmsg_trace(DBM_3D,"ITEHDMI - HDMI_3DFORMAT is UNDEFINED_FORMAT \r\n");
                            #endif
                        }
                            #ifdef DEBUG_MODE_3D
						dbmsg_trace(DBM_3D,"ITEHDMI - HDMI_3DFORMAT is FRAME_PACKING call detect3D_Port_3D_On( ) \r\n");
				#endif
                        //detect3D_Port_3D_On(&it6602data->de3dframe_config);  //ralph
                        //HDMI_DumpDE3DFrameInfo(&it6602data->s_Current3DFr);
                        it6602data->oldVIC = it6602data->newVIC;
                    }
                }

                if(((it6602data->s_Current3DFr.PB5&0xF0) == TOP_AND_BOTTOM) && (!it6602data->TopAndBottom_Flag))
                {
                    if((it6602data->s_Current3DFr.VIC == 0x3c) || (it6602data->s_Current3DFr.VIC ==0x3e) || (it6602data->s_Current3DFr.VIC == 0x13) || (it6602data->s_Current3DFr.VIC == 0x04) || (it6602data->s_Current3DFr.VIC == 0x05) ||
                       (it6602data->s_Current3DFr.VIC == 0x14) || (it6602data->s_Current3DFr.VIC ==0x20) || (it6602data->s_Current3DFr.VIC == 0x22) || (it6602data->s_Current3DFr.VIC == 0x1f) || (it6602data->s_Current3DFr.VIC == 0x10))
                    {
                        it6602data->de3dframe_config.Format   =  VERT_PACKED_HALF; // Type of 3D source format is TOP_AND_BOTTOM(VERT_PACKED_HALF)

                        #ifdef DEBUG_MODE_3D
                        dbmsg_trace(DBM_3D,"ITEHDMI - HDMI_3DFORMAT is TOP_AND_BOTTOM \r\n");
			   #else
				DLOG_INFO("ITEHDMI - HDMI_3DFORMAT is TOP_AND_BOTTOM \r\n");
                        #endif
                    }
                    else
                    {
                        it6602data->de3dframe_config.Format   =  6; // Type of 3D source format is UNDEFINED_FORMAT

                        #ifdef DEBUG_MODE_3D
                        dbmsg_trace(DBM_3D,"ITEHDMI - HDMI_3DFORMAT is UNDEFINED_FORMAT \r\n");
                        #endif
                    }

                    //detect3D_Port_3D_On(&it6602data->de3dframe_config);  //ralph
                    //HDMI_DumpDE3DFrameInfo(&it6602data->s_Current3DFr);

                    it6602data->FramePacking_Flag   = FALSE;
                    it6602data->TopAndBottom_Flag   = TRUE;
                    it6602data->SideBySide_Flag     = FALSE;
                }

                if(((it6602data->s_Current3DFr.PB5&0xF0) == SIDE_BY_SIDE) && (!it6602data->SideBySide_Flag))
                {
                    if((it6602data->s_Current3DFr.VIC == 0x3c) || (it6602data->s_Current3DFr.VIC ==0x3e) || (it6602data->s_Current3DFr.VIC == 0x13) || (it6602data->s_Current3DFr.VIC == 0x04) || (it6602data->s_Current3DFr.VIC == 0x05) ||
                       (it6602data->s_Current3DFr.VIC == 0x14) || (it6602data->s_Current3DFr.VIC ==0x20) || (it6602data->s_Current3DFr.VIC == 0x22) || (it6602data->s_Current3DFr.VIC == 0x1f) || (it6602data->s_Current3DFr.VIC == 0x10))
                    {
                        it6602data->de3dframe_config.Format   =  HORIZ_PACKED_HALF; // Type of 3D source format is SIDE_BY_SIDE(HORIZ_PACKED_HALF)

                        #ifdef DEBUG_MODE_3D
                        dbmsg_trace(DBM_3D,"ITEHDMI - HDMI_3DFORMAT is SIDE_BY_SIDE \r\n");
			   #else
				DLOG_INFO("ITEHDMI - HDMI_3DFORMAT is SIDE_BY_SIDE \r\n");
                        #endif
                    }
                    else
                    {
                        it6602data->de3dframe_config.Format   =  6; // Type of 3D source format is UNDEFINED_FORMAT

                        #ifdef DEBUG_MODE_3D
                        dbmsg_trace(DBM_3D,"ITEHDMI - HDMI_3DFORMAT is UNDEFINED_FORMAT \r\n");
                        #endif
                    }

                    //detect3D_Port_3D_On(&it6602data->de3dframe_config);  //ralph
                    //HDMI_DumpDE3DFrameInfo(&it6602data->s_Current3DFr);

                    it6602data->FramePacking_Flag   = FALSE;
                    it6602data->TopAndBottom_Flag   = FALSE;
                    it6602data->SideBySide_Flag     = TRUE;
                }

                #ifdef DEBUG_MODE_3D
                dbmsg_trace(DBM_3D,"\r\nITEHDMI - HDMI_3D_SourceConfiguration: \r\n");
                dbmsg_ftrace(DBM_3D,"        Format                   = %X \r\n",(int) it6602data->de3dframe_config.Format);
                dbmsg_ftrace(DBM_3D,"        LR_Reference             = %X \r\n",(int) it6602data->de3dframe_config.LR_Reference);
                dbmsg_ftrace(DBM_3D,"        FrameDominance           = %X \r\n",(int) it6602data->de3dframe_config.FrameDominance);
                dbmsg_ftrace(DBM_3D,"        LR_Encoding              = %X \r\n",(int) it6602data->de3dframe_config.LR_Encoding);
                dbmsg_ftrace(DBM_3D,"        TB_Reference             = %X \r\n",(int) it6602data->de3dframe_config.TB_Reference);
                dbmsg_ftrace(DBM_3D,"        OE_Reference             = %X \r\n",(int) it6602data->de3dframe_config.OE_Reference);
                dbmsg_ftrace(DBM_3D,"        NumActiveBlankLines      = %X \r\n",(int) it6602data->de3dframe_config.NumActiveBlankLines);
                dbmsg_ftrace(DBM_3D,"        NumberOfEncodedLines     = %X \r\n",(int) it6602data->de3dframe_config.NumberOfEncodedLines);
                dbmsg_ftrace(DBM_3D,"        LeftEncodedLineLocation  = %X \r\n",(int) it6602data->de3dframe_config.LeftEncodedLineLocation);
                dbmsg_ftrace(DBM_3D,"        RightEncodedLineLocation = %X \r\n",(int) it6602data->de3dframe_config.RightEncodedLineLocation);
                dbmsg_ftrace(DBM_3D,"        BlankingColor            = %X \r\n",(int) it6602data->de3dframe_config.BlankingColor );

		#else
			DLOG_INFO("\r\nITEHDMI - HDMI_3D_SourceConfiguration: \r\n");
			DLOG_INFO("        Format                   = %X \r\n",(int) it6602data->de3dframe_config.Format);
			DLOG_INFO("        LR_Reference             = %X \r\n",(int) it6602data->de3dframe_config.LR_Reference);
			DLOG_INFO("        FrameDominance           = %X \r\n",(int) it6602data->de3dframe_config.FrameDominance);
			DLOG_INFO("        LR_Encoding              = %X \r\n",(int) it6602data->de3dframe_config.LR_Encoding);
			DLOG_INFO("        TB_Reference             = %X \r\n",(int) it6602data->de3dframe_config.TB_Reference);
			DLOG_INFO("        OE_Reference             = %X \r\n",(int) it6602data->de3dframe_config.OE_Reference);
			DLOG_INFO("        NumActiveBlankLines      = %X \r\n",(int) it6602data->de3dframe_config.NumActiveBlankLines);
			DLOG_INFO("        NumberOfEncodedLines     = %X \r\n",(int) it6602data->de3dframe_config.NumberOfEncodedLines);
			DLOG_INFO("        LeftEncodedLineLocation  = %X \r\n",(int) it6602data->de3dframe_config.LeftEncodedLineLocation);
			DLOG_INFO("        RightEncodedLineLocation = %X \r\n",(int) it6602data->de3dframe_config.RightEncodedLineLocation);
			DLOG_INFO("        BlankingColor            = %X \r\n",(int) it6602data->de3dframe_config.BlankingColor );
                #endif

                return TRUE;
            }
        }

        if(it6602data->DE3DFormat_HDMIFlag)// 3D InfoFrame Packet Type is not valid
        {
            #ifdef DEBUG_MODE_3D
            dbmsg_trace(DBM_3D,"ITEHDMI - HDMI_3DFORMAT is OFF \r\n");
            #endif

//ralph
//detect3D_Port_3D_Off();
//mbSend( detect3DMbxID, D3DMSG_STATE_PORT_2D, -1, 0, FALSE, 0);
//dbmsg_ftrace( DBM_3D, "detect3D_Port_3D_Off: Current state=%s\r\n", detect3DStateStringTable[detect3DState]);

            it6602data->DE3DFormat_HDMIFlag = FALSE;
            it6602data->FramePacking_Flag   = FALSE;
            it6602data->TopAndBottom_Flag   = FALSE;
            it6602data->SideBySide_Flag     = FALSE;
        }

        /******************************  3D integration  *************************************/
    }
	else
	{

	//it6602data->f_de3dframe_hdmi = FALSE;
	hdmirxwr(REG_RX_06A, 0x82);
	hdmirxset(REG_RX_066_4_DE3DFrame, 0x10, 0x00);		// Reg66[4] = 0 for disable 3D FP2FS
	hdmirxset(REG_RX_085_5_En3DROut, 0x20, 0x00);			// Reg85[5] = 0 for disable 3DR output

	}
    return FALSE;
}
#endif

//FIX_ID_003 xxxxx	//Add IT6602 Video Output Configure Table
void IT6602ChangeTTLVideoOutputMode(void)
{
	//for test video output format  only !!!
	unsigned char i;
	struct it6602_dev_data *it6602data = get_it6602_dev_data();


	if(it6602data->m_VidOutConfigMode<eVOMreserve)
	it6602data->m_VidOutConfigMode++;
	else
	it6602data->m_VidOutConfigMode=0;

	i=it6602data->m_VidOutConfigMode;


	IT6602_VideoOutputConfigure_Init(it6602data,i);
	IT6602_VideoOutputModeSet(it6602data);
//FIX_ID_003 xxxxx
}



// disable -> //FIX_ID_005 xxxxx	//Add Cbus Event Handler
// disable -> void IT6602MSCCmdTest(unsigned char ucCmd)
// disable -> {
// disable ->
// disable -> 	struct it6602_dev_data *it6602data = get_it6602_dev_data();
// disable ->
// disable -> 	switch(ucCmd)
// disable -> 	{
// disable ->
// disable -> //	case B_Set_DCAP_RDY:			// 	0x80 // bit5 B_3DSupporpt
// disable -> //	case B_PATH_EN:				// 	0x40 // bit5 B_3DSupporpt
// disable -> //	case B_HPD:					// 	0x20 // bit4 B_3DSupporpt
// disable -> 	case B_DevCapChange:		// 	0x10 // bit3 B_3DSupporpt
// disable -> 	case B_3DSupporpt:			// 	0x04 // bit2 B_3DSupporpt
// disable -> 	case B_ReadDevCap:			// 	0x02 // bit1 B_ReadDevCap
// disable -> 	case B_DiscoveryDone:		// 	0x01 // bit0 B_DiscoveryDone
// disable -> 			it6602data ->CBusIntEvent |= ucCmd;
// disable -> 			break;
// disable -> 	}
// disable -> //FIX_ID_005 xxxxx
// disable -> }







//=============================================================================
void  Dump_ITEHDMIReg(void)//max7088
{

#if 1
    ushort	i,j ;
    //BYTE reg ;
    //BYTE bank ;
    BYTE ucData ;

//    printf(" ITEHDMI \r\n");
//    printf("\n 11111:") ;
//    printf("\n        ===================================================\r") ;
    printf("\r\n       ") ;
    for(j = 0 ; j < 16 ; j++)
    {
        printf(" %02X",(int) j) ;
        if((j == 3)||(j==7)||(j==11))
        {
                printf(" :") ;
        }
    }
    printf("\n        =====================================================\r\n") ;

chgbank(0);

    for(i = 0 ; i < 0x100 ; i+=16)
    {
        printf("[%03X]  ",i) ;
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = hdmirxrd((BYTE)((i+j)&0xFF)) ;
            printf(" %02X",(int) ucData) ;
            if((j == 3)||(j==7)||(j==11))
            {
                printf(" :") ;
            }
        }
        printf("\r\n") ;
        if((i % 0x40) == 0x30)
        {
    printf("\n        =====================================================\r\n") ;
        }
    }

chgbank(1);
    for(i = 0xb0 ; i < 0xd0 ; i+=16)
    {
        printf("[%03X]  ",i+0x100) ;
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = hdmirxrd((BYTE)((i+j)&0xFF)) ;
            printf(" %02X",(int) ucData) ;
            if((j == 3)||(j==7)||(j==11))
            {
                printf(" :") ;
            }
        }
        printf("\r\n") ;
//        if((i % 0x40) == 0x30)
//        {
//    printf("\n        =====================================================\r\n") ;
//        }
    }
chgbank(0);
#endif
}
// disable ->
// disable -> void debugxxx(void)
// disable -> {
// disable -> #if 0
// disable ->
// disable -> #else
// disable ->
// disable -> 	unsigned char i;
// disable -> 	unsigned char Receive_Err;
// disable -> 	unsigned char Video_Err;
// disable -> 	unsigned char Code_Err;
// disable -> 	unsigned char Pkt_Err;
// disable -> 	unsigned char CrtErr;
// disable -> 	unsigned char ucStatus;
// disable ->
// disable -> 	struct it6602_dev_data *it6602data = get_it6602_dev_data();
// disable ->
// disable ->
// disable -> 	IT6602_AFE_Rst();
// disable ->
// disable -> 	if(it6602data->m_ucCurrentHDMIPort == F_PORT_SEL_1)
// disable -> 	{
// disable -> 		hdmirxwr(REG_RX_0B7,Receive_Err);
// disable -> 		hdmirxwr(REG_RX_0B8,Video_Err);
// disable -> 		hdmirxwr(REG_RX_0B9,Code_Err);
// disable -> 		hdmirxwr(REG_RX_0BA,Pkt_Err);
// disable -> 		hdmirxwr(REG_RX_0BB,CrtErr);
// disable -> 	}
// disable -> 	else
// disable -> 	{
// disable -> 		hdmirxwr(REG_RX_0B2,Receive_Err);
// disable -> 		hdmirxwr(REG_RX_0B3,Video_Err);
// disable -> 		hdmirxwr(REG_RX_0B4,Code_Err);
// disable -> 		hdmirxwr(REG_RX_0B5,Pkt_Err);
// disable -> 		hdmirxwr(REG_RX_0B6,CrtErr);
// disable ->
// disable -> 	}
// disable ->
// disable -> 	DLOG_INFO("        [Status] ");
// disable -> 	DLOG_INFO("[Receive_Err] ");
// disable -> 	DLOG_INFO("[Video_Err] ");
// disable -> 	DLOG_INFO("[Code_Err] ");
// disable -> 	DLOG_INFO("[Pkt_Err] ");
// disable -> 	DLOG_INFO("[CrtErr] \r\n");
// disable ->
// disable ->
// disable -> 	for(i=0;i<30;i++)
// disable -> 	{
// disable ->
// disable -> 		if(it6602data->m_ucCurrentHDMIPort == F_PORT_SEL_1)
// disable -> 		{
// disable -> 			ucStatus = hdmirxrd(REG_RX_P1_SYS_STATUS);
// disable -> 			// !!! check ECC error register  !!!
// disable -> 			Receive_Err = hdmirxrd(REG_RX_0B7);
// disable -> 			Video_Err = hdmirxrd(REG_RX_0B8)&0xE0;
// disable -> 			Code_Err = hdmirxrd(REG_RX_0B9);
// disable -> 			Pkt_Err = hdmirxrd(REG_RX_0BA);
// disable -> 			CrtErr = hdmirxrd(REG_RX_0BB);
// disable ->
// disable -> 			hdmirxwr(REG_RX_0B7,Receive_Err);
// disable -> 			hdmirxwr(REG_RX_0B8,Video_Err);
// disable -> 			hdmirxwr(REG_RX_0B9,Code_Err);
// disable -> 			hdmirxwr(REG_RX_0BA,Pkt_Err);
// disable -> 			hdmirxwr(REG_RX_0BB,CrtErr);
// disable ->
// disable -> 		}
// disable -> 		else
// disable -> 		{
// disable -> 			ucStatus = hdmirxrd(REG_RX_P0_SYS_STATUS);
// disable -> 			// !!! check ECC error register  !!!
// disable -> 			Receive_Err = hdmirxrd(REG_RX_0B2);
// disable -> 			Video_Err = hdmirxrd(REG_RX_0B3)&0xE0;
// disable -> 			Code_Err = hdmirxrd(REG_RX_0B4);
// disable -> 			Pkt_Err = hdmirxrd(REG_RX_0B5);
// disable -> 			CrtErr = hdmirxrd(REG_RX_0B6);
// disable ->
// disable -> 			hdmirxwr(REG_RX_0B2,Receive_Err);
// disable -> 			hdmirxwr(REG_RX_0B3,Video_Err);
// disable -> 			hdmirxwr(REG_RX_0B4,Code_Err);
// disable -> 			hdmirxwr(REG_RX_0B5,Pkt_Err);
// disable -> 			hdmirxwr(REG_RX_0B6,CrtErr);
// disable ->
// disable -> 		}
// disable ->
// disable ->
// disable -> 		DLOG_INFO(" [%02X] ",(int) ucStatus));		DLOG_INFO("      ");
// disable -> 		DLOG_INFO(" [%02X] ",(int) Receive_Err));	DLOG_INFO("      ");
// disable -> 		DLOG_INFO(" [%02X] ",(int) Video_Err));		DLOG_INFO("      ");
// disable -> 		DLOG_INFO(" [%02X] ",(int) Code_Err));		DLOG_INFO("      ");
// disable -> 		DLOG_INFO(" [%02X] ",(int) Pkt_Err));		DLOG_INFO("  ");
// disable -> 		DLOG_INFO(" [%02X]  \r\n",(int) CrtErr);
// disable ->
// disable ->
// disable -> 	}
// disable -> 	IT_Delay(500);
// disable -> #endif
// disable -> }
// disable ->
// disable ->
// disable -> void debugEQ(unsigned char ucCmd)
// disable -> {
// disable ->
// disable -> 	unsigned char i;
// disable -> 	unsigned char j=0;
// disable -> 	unsigned char EQTable[8]={0xFF,0x9F,0x8F,0x0F,0x07,0x03,0x01,0x00};
// disable -> 	struct it6602_dev_data *it6602data = get_it6602_dev_data();
// disable ->
// disable -> 	switch(ucCmd)
// disable -> 	{
// disable -> 		case 0:
// disable -> 			{
// disable -> //				DisableOverWriteRS(0);
// disable -> //				DisableOverWriteRS(1);
// disable ->
// disable -> 				#ifdef _SUPPORT_EQ_ADJUST_
// disable ->
// disable -> 				if(it6602data->m_ucCurrentHDMIPort == F_PORT_SEL_0)
// disable -> 					HDMIStartEQDetect(&(it6602data->EQPort[F_PORT_SEL_0]));
// disable -> 				else
// disable -> 					HDMIStartEQDetect(&(it6602data->EQPort[F_PORT_SEL_1]));
// disable -> 				#endif
// disable ->
// disable -> 				break;
// disable -> 			}
// disable ->
// disable -> 		case 1:
// disable -> 			{
// disable -> 			        #ifdef _SUPPORT_EQ_ADJUST_
// disable -> 					if(it6602data->EQPort[F_PORT_SEL_0].f_manualEQadjust==TRUE)
// disable -> 					{
// disable -> 						//TickCountPrint();
// disable -> 						HDMIAdjustEQ(&(it6602data->EQPort[F_PORT_SEL_0]));	// for port 0
// disable -> 					}
// disable ->
// disable -> 					if(it6602data->EQPort[F_PORT_SEL_1].f_manualEQadjust==TRUE)
// disable -> 						HDMIAdjustEQ(&(it6602data->EQPort[F_PORT_SEL_1]));	// for port 1
// disable -> 			        #endif
// disable ->
// disable -> 				break;
// disable -> 			}
// disable ->
// disable -> 		case 2:
// disable -> 			{
// disable -> 				for(i=0;i<8;i++)
// disable -> 				{
// disable ->
// disable -> 					if(it6602data->m_ucCurrentHDMIPort == F_PORT_SEL_0)
// disable -> 					{
// disable -> 						hdmirxset(REG_RX_026, 0x20, 0x20);	//07-04 add for adjust EQ
// disable -> 						hdmirxwr(REG_RX_027,EQTable[i]);
// disable -> 						DLOG_INFO("Port=%X ,EQValue Reg027 = %X \r\n",(int) it6602data->m_ucCurrentHDMIPort,(int) hdmirxrd(REG_RX_027));
// disable -> 					}
// disable -> 					else
// disable -> 					{
// disable -> 						hdmirxset(REG_RX_03E, 0x20, 0x20);	//07-04 add for adjust EQ
// disable -> 						hdmirxwr(REG_RX_03F,EQTable[i]);
// disable -> 						DLOG_INFO("Port=%X ,EQValue Reg03F = %X \r\n",(int) it6602data->m_ucCurrentHDMIPort,(int) hdmirxrd(REG_RX_03F));
// disable -> 					}
// disable ->
// disable ->  					debugxxx();
// disable ->
// disable -> 					//if (kbhit())
// disable -> 					break;
// disable -> 				}
// disable ->
// disable -> 				get_vid_info();
// disable -> 				show_vid_info();
// disable ->
// disable -> 				if(it6602data->m_ucCurrentHDMIPort == F_PORT_SEL_0)
// disable -> 				{
// disable -> #ifdef _SUPPORT_AUTO_EQ_
// disable -> //xxxxx
// disable -> 					DisableOverWriteRS(0);
// disable -> 					TMDSCheck(0);
// disable -> //xxxxx
// disable -> #endif
// disable -> 				}
// disable -> 				else
// disable -> 				{
// disable -> #ifdef _SUPPORT_AUTO_EQ_
// disable -> //xxxxx
// disable -> 					DisableOverWriteRS(1);
// disable -> 					TMDSCheck(1);
// disable -> //xxxxx
// disable -> #endif
// disable -> 				}
// disable -> 				break;
// disable -> 			}
// disable ->
// disable ->
// disable -> 	}
// disable -> }

void it66021_init(void)
{

    it6602HPDCtrl(1,0); // HDMI port , set HPD = 0
    IT_Delay(10); //for power sequence
    IT6602_fsm_init();
    it6602HPDCtrl(1,1);
	
	mhlrxwr(MHL_RX_0A,0xFF);
	mhlrxwr(MHL_RX_08,0xFF);
	mhlrxwr(MHL_RX_09,0xFF);

    hdmirxset(REG_RX_063,0xFF,0x3F);
    hdmirxset(REG_RX_012,0xFF,0xF8);

    dlog_info("REG_RX_012=%2x",hdmirxrd(REG_RX_012));
    dlog_info("REG_RX_063=%2x",hdmirxrd(REG_RX_063));
    dlog_info("MHL_RX_0A=%2x", mhlrxrd(MHL_RX_0A));
    dlog_info("MHL_RX_08=%2x", mhlrxrd(MHL_RX_08));
    dlog_info("MHL_RX_09=%2x", mhlrxrd(MHL_RX_09));
    //IT_Delay(1000); //for power sequence
}
