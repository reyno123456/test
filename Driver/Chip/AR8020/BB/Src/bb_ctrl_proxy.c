#include <stdio.h>
#include <stdint.h>

#include "bb_ctrl_proxy.h"
#include "sys_event.h"


/** 
 * @brief       API for set channel Bandwidth 10M/20M, the function can only be called by cpu0,1
 * @param[in]   en_bw: channel bandwidth setting 10M/20M
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetFreqBandwidthSelection_proxy(ENUM_CH_BW en_bw)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_FREQ_CHANGE;
    cmd.configItem   = FREQ_BAND_WIDTH_SELECT;
    cmd.configValue  = (uint32_t)en_bw;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}



/** 
 * @brief       API for set frequency band (2G/5G) selection mode (ATUO / Manual), the function can only be called by cpu0,1
 * @param[in]   mode: selection mode (ATUO / Manual)
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetFreqBandSelectionMode_proxy(RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_FREQ_CHANGE;
    cmd.configItem   = FREQ_BAND_MODE;
    cmd.configValue  = (uint32_t)en_mode;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}



/** 
 * @brief       API for set frequency band (2G/5G), the function can only be called by cpu0,1
 * @param[in]   band:  manual selection RF band
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetFreqBand_proxy(ENUM_RF_BAND band)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_FREQ_CHANGE;
    cmd.configItem   = FREQ_BAND_SELECT;
    cmd.configValue  = (uint32_t)band;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}



/** 
 * @brief       API for set IT(image transmit) channel selection RUN mode(AUTO/Manual). the function can only be called by cpu0,1
 * @param[in]   en_mode: the modulation QAM mode for image transmit.
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetITChannelSelectionMode_proxy(RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_FREQ_CHANGE;
    cmd.configItem   = FREQ_CHANNEL_MODE;
    cmd.configValue  = (uint32_t)en_mode;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}


/** 
 * @brief       API for set IT(image transmit) channel Number. the function can only be called by cpu0,1
 * @param[in]   channelNum: the current channel number int current Frequency band(2G/5G)
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetITChannel_proxy(uint8_t channelNum)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_FREQ_CHANGE;
    cmd.configItem   = FREQ_CHANNEL_SELECT;
    cmd.configValue  = channelNum;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}



////////////////// handlers for WIRELESS_MCS_CHANGE //////////////////

/** 
 * @brief       API for set MCS(modulation, coderate scheme) mode, the function can only be called by cpu0,1
 * @param[in]   en_mode: auto or manual selection.
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetMCSmode_proxy(RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_MCS_CHANGE;
    cmd.configItem   = MCS_MODE_SELECT;
    cmd.configValue  = (uint32_t)en_mode;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}


/** 
 * @brief       API for set the image transmit QAM mode, the function can only be called by cpu0,1
 * @param[in]   qam: modulation qam mode
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetITQAM_proxy(ENUM_BB_QAM qam)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_MCS_CHANGE;
    cmd.configItem   = MCS_MODULATION_SELECT;
    cmd.configValue  = (uint32_t)qam;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}


/** 
 * @brief       API for set the image transmit LDPC coderate, the function can only be called by cpu0,1
 * @param[in]   ldpc:  ldpc coderate 
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetITLDPC_proxy(ENUM_BB_LDPC ldpc)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.configClass  = WIRELESS_MCS_CHANGE;
    cmd.configItem   = MCS_CODE_RATE_SELECT;
    cmd.configValue  = (uint32_t)ldpc;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}


////////////////// handlers for WIRELESS_ENCODER_CHANGE //////////////////

/** 
 * @brief       API for set the encoder bitrate control mode, the function can only be called by cpu0,1
 * @param[in]   en_mode: auto or manual selection.
 * @retval      TURE, this means command is sent sucessfully. 
 * @retval      FALSE, this means error happens in sending the command
 */
int BB_SetEncoderBrcMode_proxy(RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.configClass  = WIRELESS_ENCODER_CHANGE;
    cmd.configItem   = ENCODER_DYNAMIC_BIT_RATE_MODE;
    cmd.configValue  = (uint32_t)en_mode;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}


/** 
 * @brief       API for set the encoder bitrate Unit:Mbps, the function can only be called by cpu0,1
 * @param[in]   bitrate_Mbps: select the bitrate unit: Mbps
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetEncoderBitrate_proxy(uint8_t bitrate_Mbps)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.configClass  = WIRELESS_ENCODER_CHANGE;
    cmd.configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT;
    cmd.configValue  = bitrate_Mbps;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}


/** 
 * @brief       API for set board SKY mode or GROUND mode
 * @param[in]   SFR_TRX_MODE_SKY or SFR_TRX_MODE_GROUND
 */
void BB_SetBoardMODE(uint8_t mode)
{
    SFR_TRX_MODE_SEL = mode;
}

/** 
 * @brief       API for return board status
 * @retval      SKY or GROUND
 */
int BB_GetBoardMODE(void)
{
    return SFR_TRX_MODE_SEL;
}