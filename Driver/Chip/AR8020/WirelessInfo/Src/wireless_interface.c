#include "wireless_interface.h"
#include "usbd_def.h"
#include "usbd_hid.h"
#include "debuglog.h"
#include "bb_ctrl_proxy.h"
#include "bb_spi.h"
#include "sys_event.h"


STRU_WIRELESS_INFO_DISPLAY             *g_pstWirelessInfoDisplay;        //OSD Info in SRAM
STRU_WIRELESS_INFO_DISPLAY              g_stWirelessInfoSend;            //send OSD to PAD or PC

STRU_WIRELESS_PARAM_CONFIG_MESSAGE      g_stWirelessParamConfig;         //receive from PAD or PC

uint8_t eventFlag = 0;

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
    WIRELESS_INTERFACE_OSD_DISPLAY_Handler,
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
    WIRELESS_INTERFACE_READ_MCU_ID_Handler,       
    WIRELESS_INTERFACE_LOAD_SKY_REG_TABLE_Handler,
    WIRELESS_INTERFACE_LOAD_GRD_REG_TABLE_Handler,
    WIRELESS_INTERFACE_SWITCH_BB_POWER_Handler,   
    WIRELESS_INTERFACE_SKY_ONLY_RX_Handler,       
    WIRELESS_INTERFACE_SWITCH_RF_PWR_0_Handler,   
    WIRELESS_INTERFACE_SWITCH_RF_PWR_1_Handler,   
    WIRELESS_INTERFACE_EXT_ONEKEY_IT_Handler,     
    WIRELESS_INTERFACE_SWITCH_IT_CHAN_Handler,    
    WIRELESS_INTERFACE_SWITCH_RMT_CHAN_Handler,   
    WIRELESS_INTERFACE_SET_PWR_CAL_0_Handler,     
    WIRELESS_INTERFACE_SET_PWR_CAL_1_Handler,     
    WIRELESS_INTERFACE_RST_MCU_Handler,           
    WIRELESS_INTERFACE_RF_PWR_AUTO_Handler,       
    WIRELESS_INTERFACE_SWITCH_DEBUG_MODE_Handler, 
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    PAD_FREQUENCY_BAND_WIDTH_SELECT_Handler,
    PAD_FREQUENCY_BAND_OPERATION_MODE_Handler,
    PAD_FREQUENCY_BAND_SELECT_Handler,
    PAD_FREQUENCY_CHANNEL_OPERATION_MODE_Handler,
    PAD_FREQUENCY_CHANNEL_SELECT_Handler,
    PAD_MCS_OPERATION_MODE_Handler,
    PAD_MCS_MODULATION_MODE_Handler,
    PAD_ENCODER_DYNAMIC_BITRATE_MODE_Handler,
    PAD_ENCODER_DYNAMIC_BITRATE_SELECT_Handler,
    PAD_WIRELESS_OSD_DISPLAY_Handler
};

/* get osd info from shared memory */
void WITELESS_GetOSDInfo(void)
{
    uint8_t                  *sendBuffer;
    g_pstWirelessInfoDisplay  = (STRU_WIRELESS_INFO_DISPLAY *)OSD_STATUS_SHM_ADDR;
    sendBuffer                = (uint8_t *)g_pstWirelessInfoDisplay;

    if (USB_OTG_IS_BIG_ENDIAN())
    {
        convert_endian((void *)sendBuffer, (void *)&g_stWirelessInfoSend, (uint32_t)(sizeof(STRU_WIRELESS_INFO_DISPLAY)));
    }
    else
    {
        memcpy((void *)&g_stWirelessInfoSend, (void *)g_pstWirelessInfoDisplay, sizeof(STRU_WIRELESS_INFO_DISPLAY));
    }

}

/* Send to PAD or PC */
void WIRELESS_SendOSDInfo(ENUM_WIRELESS_TOOL host)
{
    uint32_t                  sendLength;
    sendLength                = (uint32_t)(sizeof(STRU_WIRELESS_INFO_DISPLAY));

    if (host == MCU_TO_PAD)
    {
        g_stWirelessInfoSend.messageId = PAD_WIRELESS_INTERFACE_OSD_DISPLAY;
        g_stWirelessInfoSend.paramLen = sendLength;
        if (USB_OTG_IS_BIG_ENDIAN())
        {
            convert_endian((void *)&g_stWirelessInfoSend, (void *)&g_stWirelessInfoSend, 4);
        }
    }
    else
    {
        g_stWirelessInfoSend.messageId = WIRELESS_INTERFACE_OSD_DISPLAY;
        g_stWirelessInfoSend.paramLen = sendLength;
        if (USB_OTG_IS_BIG_ENDIAN())
        {
            convert_endian((void *)&g_stWirelessInfoSend, (void *)&g_stWirelessInfoSend, 4);
        }
    }

    /* if cpu2 update info, and the info is valid */
    if ((0x0 == g_pstWirelessInfoDisplay->head)
      &&(0xFF == g_pstWirelessInfoDisplay->tail))
    {
        if (USBD_OK != USBD_HID_SendReport(&USBD_Device, (uint8_t *)&g_stWirelessInfoSend, sendLength, HID_EPIN_CTRL_ADDR))
        {
            dlog_error("send wireless info fail\n");
        }
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
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstWirelessParamConfig;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    uint8_t  inDebugFlag = 0;
    WITELESS_GetOSDInfo();
    inDebugFlag = g_stWirelessInfoSend.in_debug;
    pstWirelessParamConfig = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    if (inDebugFlag == 1)
    {
        if (BB_SPI_curPageWriteByte(pstWirelessParamConfig->paramData[0], pstWirelessParamConfig->paramData[1]))
        {
            dlog_error("write fail!\n");
        }
    }
    else
    {   
        // dlog_info("inDebugFlag1 = %x\n",inDebugFlag);
        recvMessage->messageId = 0x0e;
        recvMessage->paramLen = 0;

        if (USB_OTG_IS_BIG_ENDIAN())
        {
            convert_endian((void *)recvMessage, (void *)recvMessage, (uint32_t)(sizeof(recvMessage->messageId) + sizeof(recvMessage->paramLen)));
        }

        if (USBD_OK != USBD_HID_SendReport(&USBD_Device, (uint8_t *)recvMessage, (uint32_t)(sizeof(recvMessage->messageId) + sizeof(recvMessage->paramLen)), HID_EPIN_CTRL_ADDR))
        {
            dlog_error("send fail!\n");
        }
    }

}


void WIRELESS_INTERFACE_READ_BB_REG_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    uint8_t  inDebugFlag = 0;

    WITELESS_GetOSDInfo();
    inDebugFlag = g_stWirelessInfoSend.in_debug;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    if (inDebugFlag == 1)
    {
        recvMessage->messageId = 0x0f;
        recvMessage->paramLen = 2;
        recvMessage->paramData[0] = recvMessage->paramData[0];
        recvMessage->paramData[1] = BB_SPI_curPageReadByte(recvMessage->paramData[0]);

        if (USB_OTG_IS_BIG_ENDIAN())
        {
            convert_endian((void *)recvMessage, (void *)recvMessage, (uint32_t)(sizeof(recvMessage)));
        }

        if (USBD_OK != USBD_HID_SendReport(&USBD_Device, (uint8_t *)recvMessage, sizeof(recvMessage), HID_EPIN_CTRL_ADDR))
        {
            dlog_error("send fail!\n");
        }
    }
    else
    {   
        dlog_info("inDebugFlag1 = %x\n",inDebugFlag);
        recvMessage->messageId = 0x0f;
        recvMessage->paramLen = 0;

        if (USB_OTG_IS_BIG_ENDIAN())
        {
            convert_endian((void *)recvMessage, (void *)recvMessage, (uint32_t)(sizeof(recvMessage->messageId) + sizeof(recvMessage->paramLen)));
        }

        if (USBD_OK != USBD_HID_SendReport(&USBD_Device, (uint8_t *)recvMessage, (uint32_t)(sizeof(recvMessage->messageId) + sizeof(recvMessage->paramLen)), HID_EPIN_CTRL_ADDR))
        {
            dlog_error("send fail!\n");
        }
    }
}


void WIRELESS_INTERFACE_MIMO_2T2R_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_MIMO_1T2R_Handler\n");
}


void WIRELESS_INTERFACE_OSD_DISPLAY_Handler(void *param)
{
    ENUM_WIRELESS_TOOL toolToHost = MCU_TO_PC;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    WITELESS_GetOSDInfo();
    WIRELESS_SendOSDInfo(toolToHost);
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

void WIRELESS_INTERFACE_READ_MCU_ID_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_READ_MCU_ID_Handler\n");   
}

void WIRELESS_INTERFACE_SWITCH_DEBUG_MODE_Handler(void *param)
{
    uint8_t inDebugFlag = 0;

    STRU_WIRELESS_CONFIG_CHANGE           stWirelessConfigChange;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE    *sendMessage,*recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (recvMessage->paramData[0] == 0 && recvMessage->paramData[1] == 0)
    {
        /*enter debug mode */
        if (!eventFlag)
        {
           BB_SetBoardDebugMODE(0);
           eventFlag = 1;
        }
        WITELESS_GetOSDInfo();
        inDebugFlag = g_stWirelessInfoSend.in_debug;

    }
    else if (recvMessage->paramData[0] != 0 && recvMessage->paramData[1] == 0)
    {
        /*exit debug mode */
        if (eventFlag)
        {
          BB_SetBoardDebugMODE(1);  
	      eventFlag = 0;
        }

        WITELESS_GetOSDInfo();
        inDebugFlag = g_stWirelessInfoSend.in_debug;
    }
    
    /*send to PC*/
    recvMessage->paramData[1] = inDebugFlag;
    if (USBD_OK != USBD_HID_SendReport(&USBD_Device, (uint8_t *)recvMessage, sizeof(recvMessage), HID_EPIN_CTRL_ADDR))
    {
        dlog_error("send fail!\n");
    }
    
}

void WIRELESS_INTERFACE_LOAD_SKY_REG_TABLE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_LOAD_SKY_REG_TABLE_Handler\n");
}
void WIRELESS_INTERFACE_LOAD_GRD_REG_TABLE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_LOAD_GRD_REG_TABLE_Handler\n");
}
void WIRELESS_INTERFACE_SWITCH_BB_POWER_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_BB_POWER_Handler\n");
}
void WIRELESS_INTERFACE_SKY_ONLY_RX_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SKY_ONLY_RX_Handler\n");
}
void WIRELESS_INTERFACE_SWITCH_RF_PWR_0_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_RF_PWR_0_Handler\n");
}
void WIRELESS_INTERFACE_SWITCH_RF_PWR_1_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_RF_PWR_1_Handler\n");
}
void WIRELESS_INTERFACE_EXT_ONEKEY_IT_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_EXT_ONEKEY_IT_Handler\n");
}
void WIRELESS_INTERFACE_SWITCH_IT_CHAN_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_IT_CHAN_Handler\n");
}
void WIRELESS_INTERFACE_SWITCH_RMT_CHAN_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_RMT_CHAN_Handler\n");
}
void WIRELESS_INTERFACE_SET_PWR_CAL_0_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_PWR_CAL_0_Handler\n");
}
void WIRELESS_INTERFACE_SET_PWR_CAL_1_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_PWR_CAL_1_Handler\n");
}
void WIRELESS_INTERFACE_RST_MCU_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RST_MCU_Handler\n");
}
void WIRELESS_INTERFACE_RF_PWR_AUTO_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RF_PWR_AUTO_Handler\n");
}

void PAD_FREQUENCY_BAND_WIDTH_SELECT_Handler(void *param)
{	
	BB_SetFreqBandwidthSelection_proxy( (ENUM_CH_BW)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));
}


void PAD_FREQUENCY_BAND_OPERATION_MODE_Handler(void *param)
{
	BB_SetFreqBandSelectionMode_proxy( (RUN_MODE)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));
}


void PAD_FREQUENCY_BAND_SELECT_Handler(void *param)
{
	BB_SetFreqBand_proxy( (ENUM_RF_BAND)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));
}


void PAD_FREQUENCY_CHANNEL_OPERATION_MODE_Handler(void *param)
{
	BB_SetITChannelSelectionMode_proxy( (RUN_MODE)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));
}


void PAD_FREQUENCY_CHANNEL_SELECT_Handler(void *param)
{	
	BB_SetITChannel_proxy( (uint8_t)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));
}


void PAD_MCS_OPERATION_MODE_Handler(void *param)
{
	BB_SetMCSmode_proxy( (RUN_MODE)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));
}


void PAD_MCS_MODULATION_MODE_Handler(void *param)
{
	BB_SetITQAM_proxy( (ENUM_BB_QAM)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));
}


void PAD_ENCODER_DYNAMIC_BITRATE_MODE_Handler(void *param)
{
    BB_SetEncoderBrcMode_proxy( (RUN_MODE)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));
}


void PAD_ENCODER_DYNAMIC_BITRATE_SELECT_Handler(void *param)
{
    BB_SetEncoderBitrate_proxy( (uint8_t)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));
}


void PAD_WIRELESS_OSD_DISPLAY_Handler(void *param)
{
    ENUM_WIRELESS_TOOL toolToHost = MCU_TO_PAD;
    dlog_info("PAD_WIRELESS_OSD_DISPLAY_Handler\n");
    WITELESS_GetOSDInfo();
    WIRELESS_SendOSDInfo(toolToHost);
}


void WIRELESS_ParseParamConfig(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstWirelessParamConfig;
    STRU_WIRELESS_CONFIG_CHANGE             stWirelessConfigChange;
    uint8_t                                 messageId;

    if (USB_OTG_IS_BIG_ENDIAN())
    {
        convert_endian(param, param, (uint32_t)sizeof(STRU_WIRELESS_PARAM_CONFIG_MESSAGE));
    }

	pstWirelessParamConfig          = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    messageId                       = pstWirelessParamConfig->messageId;

    if (messageId >= PAD_WIRELESS_INTERFACE_PID_NUM)
    {
        dlog_error("no this message\n");
        return;
    }

    (g_stWirelessMsgHandler[messageId])(param);

    return;
}


void convert_endian(void *src_data, void *dst_data, uint32_t dataLen)
{
    uint8_t                 temp;
    uint32_t                i;
    uint8_t                *source;
    uint8_t                *dest;

    source                  = (uint8_t *)src_data;
    dest                    = (uint8_t *)dst_data;

    if (source == dest)
    {
        for (i = 0; i < (dataLen - 3); i += 4)
        {
            temp            = source[i];
            source[i]       = source[i+3];
            source[i+3]     = temp;

            temp            = source[i+1];
            source[i+1]     = source[i+2];
            source[i+2]     = temp;
        }
    }
    else
    {
        for (i = 0; i < (dataLen - 3); i += 4)
        {
            dest[i]         = source[i+3];
            dest[i+1]       = source[i+2];
            dest[i+2]       = source[i+1];
            dest[i+3]       = source[i];
        }
    }

    return;
}



