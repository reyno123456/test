#ifndef __BB_API_
#define __BB_API_

#include "bb_types.h"



/** 
 * @brief       API for Baseband initialize.
 * @param[in]   en_mode: brief @ENUM_BB_MODE
 */
void BB_init(ENUM_BB_MODE en_mode);


/** 
 * @brief       API for set frequency band (2G/5G) selection mode (ATUO / Manual)
 * @param[in]   en_mode: selection mode (ATUO / Manual)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetFreqBandSelectionMode(ENUM_RUN_MODE en_mode);

/** 
 * @brief       API for set frequency band (2G/5G)
 * @param[in]   en_mode: selection mode (ATUO / Manual)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetFreqBand(ENUM_RF_BAND band);


/** 
 * @brief       API for set IT(image transmit) channel selection RUN mode(AUTO/Manual).
 * @param[in]   qam: the modulation QAM mode for image transmit.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITChannelSelectionMode(ENUM_RUN_MODE en_mode);


/** 
 * @brief       API for set IT(image transmit) channel Number.
 * @param[in]   channelNum: the current channel number int current Frequency band(2G/5G)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITChannel(uint8_t channelNum);


/** 
 * @brief       API for set MCS(modulation, coderate scheme) mode
 * @param[in]   en_mode: auto or manual selection.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetMCSmode(ENUM_RUN_MODE en_mode);


/** 
 * @brief       API for set the image transmit QAM mode
 * @param[in]   qam: modulation qam mode
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITQAM(ENUM_BB_QAM qam);



/** 
 * @brief       API for set the image transmit LDPC coderate
 * @param[in]   ldpc:  ldpc coderate 
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITLDPC(ENUM_BB_LDPC ldpc);



/** 
 * @brief       API for set the encoder bitrate control mode
 * @param[in]   en_mode: auto or manual selection.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetEncoderBrcMode(ENUM_RUN_MODE en_mode);



/** 
 * @brief       API for set the encoder bitrate Unit:Mbps
 * @param[in]   bitrate_Mbps: select the bitrate unit: Mbps
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetEncoderBitrate(uint8_t bitrate_Mbps);



/** 
 * @brief       API for set the encoder bitrate Unit:Mbps
 * @param[in]   bitrate_Mbps: select the bitrate unit: Mbps
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
void BB_uart10_spi_sel(uint32_t sel_dat);

#endif
