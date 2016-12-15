#ifndef __BB_PROXY_H_
#define __BB_PROXY_H_

#include "bb_types.h"



#define SFR_TRX_MODE_SEL (*(volatile uint32_t *)0x40B00068)
#define SFR_TRX_MODE_GROUND             0x03
#define SFR_TRX_MODE_SKY                0x01

/** 
 * @brief       API for set channel Bandwidth 10M/20M, the function can only be called by cpu0,1
 * @param[in]   en_bw: channel bandwidth setting 10M/20M
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetFreqBandwidthSelection_proxy(ENUM_CH_BW en_bw);



/** 
 * @brief       API for set frequency band (2G/5G) selection mode (ATUO / Manual), the function can only be called by cpu0,1
 * @param[in]   mode: selection mode (ATUO / Manual)
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetFreqBandSelectionMode_proxy(RUN_MODE en_mode);



/** 
 * @brief       API for set frequency band (2G/5G), the function can only be called by cpu0,1
 * @param[in]   band:  manual selection RF band
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetFreqBand_proxy(ENUM_RF_BAND band);




/** 
 * @brief       API for set IT(image transmit) channel selection RUN mode(AUTO/Manual). the function can only be called by cpu0,1
 * @param[in]   en_mode: the modulation QAM mode for image transmit.
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetITChannelSelectionMode_proxy(RUN_MODE en_mode);




/** 
 * @brief       API for set IT(image transmit) channel Number. the function can only be called by cpu0,1
 * @param[in]   channelNum: the current channel number int current Frequency band(2G/5G)
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetITChannel_proxy(uint8_t channelNum);




/** 
 * @brief       API for set MCS(modulation, coderate scheme) mode, the function can only be called by cpu0,1
 * @param[in]   en_mode: auto or manual selection.
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetMCSmode_proxy(RUN_MODE en_mode);




/** 
 * @brief       API for set the image transmit QAM mode, the function can only be called by cpu0,1
 * @param[in]   qam: modulation qam mode
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetITQAM_proxy(ENUM_BB_QAM qam);




/** 
 * @brief       API for set the image transmit LDPC coderate, the function can only be called by cpu0,1
 * @param[in]   ldpc:  ldpc coderate 
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetITLDPC_proxy(ENUM_BB_LDPC ldpc);




/** 
 * @brief       API for set the encoder bitrate control mode, the function can only be called by cpu0,1
 * @param[in]   en_mode: auto or manual selection.
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetEncoderBrcMode_proxy(RUN_MODE en_mode);



/** 
 * @brief       API for set the encoder bitrate Unit:Mbps, the function can only be called by cpu0,1
 * @param[in]   bitrate_Mbps: select the bitrate unit: Mbps
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetEncoderBitrate_proxy(uint8_t bitrate_Mbps);


/** 
 * @brief       API for set board SKY mode or GROUND mode
 * @param[in]   SFR_TRX_MODE_SKY or SFR_TRX_MODE_GROUND
 */
void BB_SetBoardMODE(uint8_t mode);



/** 
 * @brief       API for return board status
 * @retval      SKY or GROUND
 */
int BB_GetBoardMODE(void);

/** 
 *  * @brief       API for set board enter or out debug mode
 *   * @param   mode	0:enter debug mode, 1:out debug mode.
 *    */
int BB_SetBoardDebugMODE(uint8_t mode);


#endif
