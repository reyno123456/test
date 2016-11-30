#include "wireless_interface.h"
#include "usbd_def.h"
#include "usbd_hid.h"
#include "debuglog.h"
#include "sys_event.h"


STRU_WIRELESS_INFO_DISPLAY              g_stWirelessInfoDisplay;        //send to PAD or PC

STRU_WIRELESS_PARAM_CONFIG_MESSAGE      g_stWirelessParamConfig;        //receive from PAD or PC

extern USBD_HandleTypeDef               USBD_Device;


WIRELESS_CONFIG_HANDLER g_stWirelessMsgHandler[PAD_WIRELESS_INTERFACE_PID_NUM] = 
{
    NULL,
    WIRELESS_INTERFACE_UPGRADE_Handler,
    WIRELESS_INTERFACE_ENTER_TEST_MODE_Handler,
    WIRELESS_INTERFACE_SYNCHRONIZE_FREQ_CHANNEL_Handler,
    WIRELESS_INTERFACE_AUTO_MODE_Handler,
    WIRELESS_INTERFACE_SELF_ADAPTION_MODE_Handler,
    WIRELESS_INTERFACE_SWITCH_TX_MODE_Handler,
    WIRELESS_INTERFACE_SWITCH_RX_MODE_Handler,
    WIRELESS_INTERFACE_SWITCH_USB1_MODE_Handler,
    WIRELESS_INTERFACE_SWITCH_USB2_MODE_Handler,
    WIRELESS_INTERFACE_ALL_RESET_Handler,
    WIRELESS_INTERFACE_RX_RESET_Handler,
    WIRELESS_INTERFACE_TX_RESET_Handler,
    WIRELESS_INTERFACE_MIMO_1T2R_Handler,
    WIRELESS_INTERFACE_WRITE_BB_REG_Handler,
    WIRELESS_INTERFACE_READ_BB_REG_Handler,
    WIRELESS_INTERFACE_MIMO_2T2R_Handler,
    WIRELESS_INTERFACE_ENABLE_OSD_DISPLAY_Handler,
    NULL,
    WIRELESS_INTERFACE_GET_ID_Handler,
    WIRELESS_INTERFACE_GET_GROUND_TX_PWR_Handler,
    WIRELESS_INTERFACE_GET_SKY_TX_PWR_Handler,
    WIRELESS_INTERFACE_GET_RC_FREQ_CHANNEL_Handler,
    WIRELESS_INTERFACE_GET_VIDEO_FREQ_CHANNEL_Handler,
    WIRELESS_INTERFACE_GET_SOFTWARE_VERSION_Handler,
    WIRELESS_INTERFACE_GET_DEV_INFO_Handler,
    WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_MASK_Handler,
    WIRELESS_INTERFACE_SET_VIDEO_FREQ_CHANNEL_MASK_Handler,
    WIRELESS_INTERFACE_SELECT_VIDEO_FREQ_CHANNEL_Handler,
    WIRELESS_INTERFACE_SWITCH_QAM_MODE_Handler,
    WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_Handler,
    WIRELESS_INTERFACE_SET_TX_PWR_Handler,
    WIRELESS_INTERFACE_SET_VIDEO_TX_ID_Handler,
    WIRELESS_INTERFACE_SET_CTRL_ID_Handler,
    WIRELESS_INTERFACE_SWITCH_WORKING_FREQ_BAND_Handler,
    WIRELESS_INTERFACE_RC_SCAN_ALL_BAND_Handler,
    WIRELESS_INTERFACE_RC_SCAN_WORKING_BAND_Handler,
    WIRELESS_INTERFACE_VIDEO_SCAN_ALL_BAND_Handler,
    WIRELESS_INTERFACE_VIDEO_SCAN_WORKING_BAND_Handler,
    WIRELESS_INTERFACE_RECOVER_TO_FACTORY_Handler,
    WIRELESS_INTERFACE_RC_HOPPING_Handler,
    WIRELESS_INTERFACE_SAVE_CONFIGURE_Handler,
    PAD_FREQUENCY_BAND_WIDTH_SELECT_Handler,
    PAD_FREQUENCY_BAND_OPERATION_MODE_Handler,
    PAD_FREQUENCY_BAND_SELECT_Handler,
    PAD_FREQUENCY_CHANNEL_OPERATION_MODE_Handler,
    PAD_FREQUENCY_CHANNEL_SELECT_Handler,
    PAD_MCS_OPERATION_MODE_Handler,
    PAD_MCS_MODULATION_MODE_Handler,
    PAD_ENCODER_DYNAMIC_BITRATE_MODE_Handler,
    PAD_ENCODER_DYNAMIC_BITRATE_SELECT_Handler,
    PAD_WIRELESS_INFO_DISPLAY_Handler
};




/* Send to PAD or PC */
void WIRELESS_SendDisplayInfo(void)
{
    uint8_t            *sendBuffer;
    uint8_t             sendLength;

    //g_stWirelessInfoDisplay.messageId   = PAD_WIRELESS_INFO_DISPLAY;

    sendBuffer          = (uint8_t *)&g_stWirelessInfoDisplay;
    sendLength          = (uint8_t)sizeof(g_stWirelessInfoDisplay);

    if (USBD_OK != USBD_HID_SendReport(&USBD_Device, sendBuffer, sendLength, HID_CTRL))
    {
        dlog_error("send wireless info fail\n");
    }

    return;
}


void WIRELESS_INTERFACE_UPGRADE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_UPGRADE_Handler\n");
}


void WIRELESS_INTERFACE_ENTER_TEST_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_ENTER_TEST_MODE_Handler\n");
}


void WIRELESS_INTERFACE_SYNCHRONIZE_FREQ_CHANNEL_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SYNCHRONIZE_FREQ_CHANNEL_Handler\n");
}


void WIRELESS_INTERFACE_AUTO_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_AUTO_MODE_Handler\n");
}


void WIRELESS_INTERFACE_SELF_ADAPTION_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SELF_ADAPTION_MODE_Handler\n");
}


void WIRELESS_INTERFACE_SWITCH_TX_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_TX_MODE_Handler\n");
}


void WIRELESS_INTERFACE_SWITCH_RX_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_RX_MODE_Handler\n");
}


void WIRELESS_INTERFACE_SWITCH_USB1_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_USB1_MODE_Handler\n");
}


void WIRELESS_INTERFACE_SWITCH_USB2_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_USB2_MODE_Handler\n");
}


void WIRELESS_INTERFACE_ALL_RESET_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_ALL_RESET_Handler\n");
}


void WIRELESS_INTERFACE_RX_RESET_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RX_RESET_Handler\n");
}


void WIRELESS_INTERFACE_TX_RESET_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_TX_RESET_Handler\n");
}


void WIRELESS_INTERFACE_MIMO_1T2R_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_MIMO_1T2R_Handler\n");
}


void WIRELESS_INTERFACE_WRITE_BB_REG_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_WRITE_BB_REG_Handler\n");
}


void WIRELESS_INTERFACE_READ_BB_REG_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_READ_BB_REG_Handler\n");
}


void WIRELESS_INTERFACE_MIMO_2T2R_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_MIMO_1T2R_Handler\n");
}


void WIRELESS_INTERFACE_ENABLE_OSD_DISPLAY_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_ENABLE_OSD_DISPLAY_Handler\n");
}


void WIRELESS_INTERFACE_GET_ID_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_ID_Handler\n");
}


void WIRELESS_INTERFACE_GET_GROUND_TX_PWR_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_GROUND_TX_PWR_Handler\n");
}


void WIRELESS_INTERFACE_GET_SKY_TX_PWR_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_SKY_TX_PWR_Handler\n");
}


void WIRELESS_INTERFACE_GET_RC_FREQ_CHANNEL_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_RC_FREQ_CHANNEL_Handler\n");
}


void WIRELESS_INTERFACE_GET_VIDEO_FREQ_CHANNEL_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_VIDEO_FREQ_CHANNEL_Handler\n");
}


void WIRELESS_INTERFACE_GET_SOFTWARE_VERSION_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_SOFTWARE_VERSION_Handler\n");
}


void WIRELESS_INTERFACE_GET_DEV_INFO_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_DEV_INFO_Handler\n");
}


void WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_MASK_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_MASK_Handler\n");
}


void WIRELESS_INTERFACE_SET_VIDEO_FREQ_CHANNEL_MASK_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_VIDEO_FREQ_CHANNEL_MASK_Handler\n");
}


void WIRELESS_INTERFACE_SELECT_VIDEO_FREQ_CHANNEL_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SELECT_VIDEO_FREQ_CHANNEL_Handler\n");
}


void WIRELESS_INTERFACE_SWITCH_QAM_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_QAM_MODE_Handler\n");
}


void WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_Handler\n");
}


void WIRELESS_INTERFACE_SET_TX_PWR_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_TX_PWR_Handler\n");
}


void WIRELESS_INTERFACE_SET_VIDEO_TX_ID_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_VIDEO_TX_ID_Handler\n");
}


void WIRELESS_INTERFACE_SET_CTRL_ID_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_CTRL_ID_Handler\n");
}


void WIRELESS_INTERFACE_SWITCH_WORKING_FREQ_BAND_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_WORKING_FREQ_BAND_Handler\n");
}


void WIRELESS_INTERFACE_RC_SCAN_ALL_BAND_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RC_SCAN_ALL_BAND_Handler\n");
}


void WIRELESS_INTERFACE_RC_SCAN_WORKING_BAND_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RC_SCAN_WORKING_BAND_Handler\n");
}


void WIRELESS_INTERFACE_VIDEO_SCAN_ALL_BAND_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_VIDEO_SCAN_ALL_BAND_Handler\n");
}


void WIRELESS_INTERFACE_VIDEO_SCAN_WORKING_BAND_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_VIDEO_SCAN_WORKING_BAND_Handler\n");
}


void WIRELESS_INTERFACE_RECOVER_TO_FACTORY_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RECOVER_TO_FACTORY_Handler\n");
}


void WIRELESS_INTERFACE_RC_HOPPING_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RC_HOPPING_Handler\n");
}


void WIRELESS_INTERFACE_SAVE_CONFIGURE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SAVE_CONFIGURE_Handler\n");
}


void PAD_FREQUENCY_BAND_WIDTH_SELECT_Handler(void *param)
{
    STRU_WIRELESS_CONFIG_CHANGE             stWirelessConfigChange;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstConfigMessage;

    dlog_info("PAD_FREQUENCY_BAND_WIDTH_SELECT_Handler\n");

    pstConfigMessage                    = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    stWirelessConfigChange.configClass  = WIRELESS_FREQ_CHANGE;
    stWirelessConfigChange.configItem   = FREQ_BAND_WIDTH_SELECT;
    stWirelessConfigChange.configValue  = pstConfigMessage->paramData[0];

    SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&stWirelessConfigChange);
}


void PAD_FREQUENCY_BAND_OPERATION_MODE_Handler(void *param)
{
    STRU_WIRELESS_CONFIG_CHANGE             stWirelessConfigChange;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstConfigMessage;

    dlog_info("PAD_FREQUENCY_BAND_OPERATION_MODE_Handler\n");

    pstConfigMessage                    = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    stWirelessConfigChange.configClass  = WIRELESS_FREQ_CHANGE;
    stWirelessConfigChange.configItem   = FREQ_BAND_MODE;
    stWirelessConfigChange.configValue  = pstConfigMessage->paramData[0];

    SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&stWirelessConfigChange);
}


void PAD_FREQUENCY_BAND_SELECT_Handler(void *param)
{
    STRU_WIRELESS_CONFIG_CHANGE             stWirelessConfigChange;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstConfigMessage;

    dlog_info("PAD_FREQUENCY_BAND_SELECT_Handler\n");

    pstConfigMessage                    = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    stWirelessConfigChange.configClass  = WIRELESS_FREQ_CHANGE;
    stWirelessConfigChange.configItem   = FREQ_BAND_SELECT;
    stWirelessConfigChange.configValue  = pstConfigMessage->paramData[0];

    SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&stWirelessConfigChange);
}


void PAD_FREQUENCY_CHANNEL_OPERATION_MODE_Handler(void *param)
{
    STRU_WIRELESS_CONFIG_CHANGE             stWirelessConfigChange;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstConfigMessage;

    pstConfigMessage                    = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    dlog_info("PAD_FREQUENCY_CHANNEL_OPERATION_MODE_Handler\n");

    stWirelessConfigChange.configClass  = WIRELESS_FREQ_CHANGE;
    stWirelessConfigChange.configItem   = FREQ_CHANNEL_MODE;
    stWirelessConfigChange.configValue  = pstConfigMessage->paramData[0];

    SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&stWirelessConfigChange);
}


void PAD_FREQUENCY_CHANNEL_SELECT_Handler(void *param)
{
    STRU_WIRELESS_CONFIG_CHANGE             stWirelessConfigChange;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstConfigMessage;

    pstConfigMessage                    = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    dlog_info("PAD_FREQUENCY_CHANNEL_SELECT_Handler\n");

    stWirelessConfigChange.configClass  = WIRELESS_FREQ_CHANGE;
    stWirelessConfigChange.configItem   = FREQ_CHANNEL_SELECT;
    stWirelessConfigChange.configValue  = pstConfigMessage->paramData[0];

    SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&stWirelessConfigChange);
}


void PAD_MCS_OPERATION_MODE_Handler(void *param)
{
    STRU_WIRELESS_CONFIG_CHANGE             stWirelessConfigChange;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstConfigMessage;

    pstConfigMessage                    = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    dlog_info("PAD_MCS_OPERATION_MODE_Handler\n");

    stWirelessConfigChange.configClass  = WIRELESS_MCS_CHANGE;
    stWirelessConfigChange.configItem   = MCS_MODE_SELECT;
    stWirelessConfigChange.configValue  = pstConfigMessage->paramData[0];

    SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&stWirelessConfigChange);
}


void PAD_MCS_MODULATION_MODE_Handler(void *param)
{
    STRU_WIRELESS_CONFIG_CHANGE             stWirelessConfigChange;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstConfigMessage;

    pstConfigMessage                    = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    dlog_info("PAD_MCS_MODULATION_MODE_Handler\n");

    stWirelessConfigChange.configClass  = WIRELESS_MCS_CHANGE;
    stWirelessConfigChange.configItem   = MCS_MODULATION_SELECT;
    stWirelessConfigChange.configValue  = pstConfigMessage->paramData[0];

    SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&stWirelessConfigChange);
}


void PAD_ENCODER_DYNAMIC_BITRATE_MODE_Handler(void *param)
{
    STRU_WIRELESS_CONFIG_CHANGE             stWirelessConfigChange;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstConfigMessage;

    dlog_info("PAD_ENCODER_DYNAMIC_BITRATE_MODE_Handler\n");

    pstConfigMessage                    = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    stWirelessConfigChange.configClass  = WIRELESS_ENCODER_CHANGE;
    stWirelessConfigChange.configItem   = ENCODER_DYNAMIC_BIT_RATE_MODE;
    stWirelessConfigChange.configValue  = pstConfigMessage->paramData[0];

    SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&stWirelessConfigChange);
}


void PAD_ENCODER_DYNAMIC_BITRATE_SELECT_Handler(void *param)
{
    STRU_WIRELESS_CONFIG_CHANGE             stWirelessConfigChange;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstConfigMessage;

    dlog_info("PAD_ENCODER_DYNAMIC_BITRATE_SELECT_Handler\n");

    pstConfigMessage                    = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    stWirelessConfigChange.configClass  = WIRELESS_ENCODER_CHANGE;
    stWirelessConfigChange.configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT;
    stWirelessConfigChange.configValue  = pstConfigMessage->paramData[0];

    SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&stWirelessConfigChange);
}


void PAD_WIRELESS_INFO_DISPLAY_Handler(void *param)
{
    dlog_info("PAD_WIRELESS_INFO_DISPLAY_Handler\n");

    WIRELESS_SendDisplayInfo();
}


void WIRELESS_ParseParamConfig(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstWirelessParamConfig;
    STRU_WIRELESS_CONFIG_CHANGE             stWirelessConfigChange;
    uint8_t                                 messageId;

	pstWirelessParamConfig          = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    messageId                       = pstWirelessParamConfig->messageId;

    if (messageId >= PAD_WIRELESS_INTERFACE_PID_NUM)
    {
        dlog_error("no this message\n");
        return;
    }

    dlog_info("WIRELESS_ParseParamConfig, handler: 0x%08x\n", g_stWirelessMsgHandler[messageId]);

    (g_stWirelessMsgHandler[messageId])(param);

    return;
}


