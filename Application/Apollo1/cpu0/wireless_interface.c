#include <string.h>
#include "wireless_interface.h"
#include "hal_usb_device.h"
#include "debuglog.h"
#include "hal_bb.h"
#include "bb_types.h"
#include "cmsis_os.h"

STRU_WIRELESS_INFO_DISPLAY             *g_pstWirelessInfoDisplay;        //OSD Info in SRAM
STRU_WIRELESS_INFO_DISPLAY              g_stWirelessInfoSend;            //send OSD to PAD or PC
STRU_WIRELESS_MESSAGE_BUFF              g_stWirelessParamConfig;     //receive from PAD or PC


uint8_t eventFlag = 0;


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
    WIRELESS_INTERFACE_READ_RF_REG_Handler,
    WIRELESS_INTERFACE_WRITE_RF_REG_Handler,
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

    memcpy((void *)&g_stWirelessInfoSend, (void *)g_pstWirelessInfoDisplay, sizeof(STRU_WIRELESS_INFO_DISPLAY));

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
    }
    else
    {
        g_stWirelessInfoSend.messageId = WIRELESS_INTERFACE_OSD_DISPLAY;
        g_stWirelessInfoSend.paramLen = sendLength;
    }

    /* if cpu2 update info, and the info is valid */
    if ((0x0 == g_pstWirelessInfoDisplay->head)
      &&(0xFF == g_pstWirelessInfoDisplay->tail)
      &&(0x0 == g_pstWirelessInfoDisplay->in_debug))
    {
        if (HAL_OK != HAL_USB_DeviceSendCtrl((uint8_t *)&g_stWirelessInfoSend, sendLength))
        {
            dlog_error("send wireless info fail");
        }
    }

    return;
}


uint8_t WIRELESS_INTERFACE_UPGRADE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_UPGRADE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_ENTER_TEST_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_ENTER_TEST_MODE_Handler\n"); 

    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    WITELESS_GetOSDInfo();
    if (g_stWirelessInfoSend.in_debug == 1)
    {
        HAL_BB_writeByte(PAGE2, 0x02, 0x06);
    }
    else
    {
        HAL_BB_SetItOnlyFreqProxy(1);
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_SYNCHRONIZE_FREQ_CHANNEL_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SYNCHRONIZE_FREQ_CHANNEL_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_AUTO_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_AUTO_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SELF_ADAPTION_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SELF_ADAPTION_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SWITCH_TX_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_TX_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SWITCH_RX_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_RX_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SWITCH_USB1_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_USB1_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SWITCH_USB2_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_USB2_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_ALL_RESET_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_ALL_RESET_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_RX_RESET_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RX_RESET_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_TX_RESET_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_TX_RESET_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_MIMO_1T2R_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_MIMO_1T2R_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_WRITE_BB_REG_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    WITELESS_GetOSDInfo();
    if (g_stWirelessInfoSend.in_debug == 1)
    {
        if (HAL_BB_curPageWriteByte(recvMessage->paramData[0], recvMessage->paramData[1]))
        {
            dlog_error("write fail!\n");

            return 1;
        }
    }
    else
    {
        // dlog_info("inDebugFlag1 = %x\n",inDebugFlag);
        recvMessage->messageId = 0x0e;
        recvMessage->paramLen = 0;

        if (HAL_OK != HAL_USB_DeviceSendCtrl((uint8_t *)recvMessage, (uint32_t)(sizeof(recvMessage->messageId) + sizeof(recvMessage->paramLen))))
        {
            dlog_error("send fail!\n");

            return 1;
        }
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_READ_BB_REG_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    
    WITELESS_GetOSDInfo();

    if (g_stWirelessInfoSend.in_debug == 1)
    {
        recvMessage->messageId = 0x0f;
        recvMessage->paramLen = 2;
        recvMessage->paramData[0] = recvMessage->paramData[0];
        HAL_BB_curPageReadByte(recvMessage->paramData[0], &recvMessage->paramData[1]);

        if (HAL_OK != HAL_USB_DeviceSendCtrl((uint8_t *)recvMessage, sizeof(recvMessage)))
        {
            dlog_error("send fail!\n");

            return 1;
        }
    }
    else
    {   
        recvMessage->messageId = 0x0f;
        recvMessage->paramLen = 0;

        if (HAL_OK != HAL_USB_DeviceSendCtrl((uint8_t *)recvMessage, (uint32_t)(sizeof(recvMessage->messageId) + sizeof(recvMessage->paramLen))))
        {
            dlog_error("send fail!\n");

            return 1;
        }
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_MIMO_2T2R_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_MIMO_1T2R_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_OSD_DISPLAY_Handler(void *param)
{
    ENUM_WIRELESS_TOOL toolToHost = MCU_TO_PC;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    WITELESS_GetOSDInfo();
    WIRELESS_SendOSDInfo(toolToHost);

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_ID_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_ID_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_GROUND_TX_PWR_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_GROUND_TX_PWR_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_SKY_TX_PWR_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_SKY_TX_PWR_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_RC_FREQ_CHANNEL_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_RC_FREQ_CHANNEL_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_VIDEO_FREQ_CHANNEL_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_VIDEO_FREQ_CHANNEL_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_SOFTWARE_VERSION_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_SOFTWARE_VERSION_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_DEV_INFO_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_GET_DEV_INFO_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_MASK_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_MASK_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SET_VIDEO_FREQ_CHANNEL_MASK_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_VIDEO_FREQ_CHANNEL_MASK_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SELECT_VIDEO_FREQ_CHANNEL_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    uint8_t u8_index;
    uint32_t u32_freValue = 0;
    WITELESS_GetOSDInfo();
    if (g_stWirelessInfoSend.in_debug == 1)
    {
        for (u8_index = 0; u8_index < recvMessage->paramData[2]; ++u8_index)
        {
            HAL_BB_writeByte(recvMessage->paramData[0],recvMessage->paramData[1],recvMessage->paramData[u8_index+3]);
            recvMessage->paramData[1] += 1;
        }
    }
    else
    {   
        for (u8_index = 0; u8_index < recvMessage->paramData[2]; ++u8_index)
        {
            u32_freValue = (u32_freValue << 8) + recvMessage->paramData[u8_index+3];
        }
        HAL_BB_SetItFreqProxy(u32_freValue);
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_SWITCH_QAM_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_QAM_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    uint8_t u8_index;
    uint32_t u32_freValue = 0;
    WITELESS_GetOSDInfo();
    if (g_stWirelessInfoSend.in_debug == 1)
    {
        for (u8_index = 0; u8_index < recvMessage->paramData[2]; ++u8_index)
        {
            HAL_BB_writeByte(recvMessage->paramData[0],recvMessage->paramData[1],recvMessage->paramData[u8_index+3]);
            recvMessage->paramData[1] += 1;
        }
    }
    else
    {   
        for (u8_index = 0; u8_index < recvMessage->paramData[2]; ++u8_index)
        {
            u32_freValue = (u32_freValue << 8) + recvMessage->paramData[u8_index+3];
        }
        HAL_BB_SetRcFreqProxy(u32_freValue);
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_SET_TX_PWR_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_TX_PWR_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SET_VIDEO_TX_ID_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_VIDEO_TX_ID_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SET_CTRL_ID_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_CTRL_ID_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SWITCH_WORKING_FREQ_BAND_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_WORKING_FREQ_BAND_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_RC_SCAN_ALL_BAND_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RC_SCAN_ALL_BAND_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_RC_SCAN_WORKING_BAND_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RC_SCAN_WORKING_BAND_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_VIDEO_SCAN_ALL_BAND_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_VIDEO_SCAN_ALL_BAND_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_VIDEO_SCAN_WORKING_BAND_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_VIDEO_SCAN_WORKING_BAND_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_RECOVER_TO_FACTORY_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RECOVER_TO_FACTORY_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_RC_HOPPING_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RC_HOPPING_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SAVE_CONFIGURE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SAVE_CONFIGURE_Handler\n");

    return 0;
}

uint8_t WIRELESS_INTERFACE_READ_MCU_ID_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_READ_MCU_ID_Handler\n");

    return 0;
}

uint8_t WIRELESS_INTERFACE_SWITCH_DEBUG_MODE_Handler(void *param)
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
           HAL_BB_SetBoardDebugModeProxy(0);
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
            HAL_BB_SetBoardDebugModeProxy(1);  
	        eventFlag = 0;
        }

        WITELESS_GetOSDInfo();
        inDebugFlag = g_stWirelessInfoSend.in_debug;
    }
    
    /*send to PC*/
    recvMessage->paramData[1] = inDebugFlag;
    if (HAL_OK != HAL_USB_DeviceSendCtrl((uint8_t *)recvMessage, sizeof(recvMessage)))
    {
        dlog_error("send fail!\n");

        return 1;
    }

    return 0;
}

uint8_t WIRELESS_INTERFACE_WRITE_RF_REG_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    WITELESS_GetOSDInfo();
    dlog_info("ID = %x\n",recvMessage->messageId);
    if (g_stWirelessInfoSend.in_debug == 1)
    {
        if (HAL_RF8003s_writeReg(recvMessage->paramData[0], recvMessage->paramData[1]))
        {
            dlog_error("write fail!\n");

            return 1;
        }
    }
    else
    {   
        // dlog_info("inDebugFlag1 = %x\n",inDebugFlag);
        recvMessage->messageId = 0x0e;
        recvMessage->paramLen = 0;

        if (HAL_OK != HAL_USB_DeviceSendCtrl((uint8_t *)recvMessage, (uint32_t)(sizeof(recvMessage->messageId) + sizeof(recvMessage->paramLen))))
        {
            dlog_error("send fail!\n");

            return 1;
        }
    }

    return 0;
}

uint8_t WIRELESS_INTERFACE_READ_RF_REG_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    dlog_info("ID = %x\n",recvMessage->messageId);
    WITELESS_GetOSDInfo();
    if (g_stWirelessInfoSend.in_debug == 1)
    {
        recvMessage->messageId = 0x3A;
        recvMessage->paramLen = 2;
        recvMessage->paramData[0] = recvMessage->paramData[0];
        if (HAL_RF8003s_readByte(recvMessage->paramData[0],&recvMessage->paramData[1]))
        {
            dlog_error("read rf8003s error\n");

            return 1;
        }

        if (HAL_OK != HAL_USB_DeviceSendCtrl((uint8_t *)recvMessage, sizeof(recvMessage)))
        {
            dlog_error("send fail!\n");

            return 1;
        }
    }
    else
    {   
        recvMessage->messageId = 0x0f;
        recvMessage->paramLen = 0;

        if (HAL_OK != HAL_USB_DeviceSendCtrl((uint8_t *)recvMessage, (uint32_t)(sizeof(recvMessage->messageId) + sizeof(recvMessage->paramLen))))
        {
            dlog_error("send fail!\n");

            return 1;
        }
    }

    return 0;
}
uint8_t WIRELESS_INTERFACE_LOAD_SKY_REG_TABLE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_LOAD_SKY_REG_TABLE_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_LOAD_GRD_REG_TABLE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_LOAD_GRD_REG_TABLE_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SWITCH_BB_POWER_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_BB_POWER_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SKY_ONLY_RX_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SKY_ONLY_RX_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SWITCH_RF_PWR_0_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_RF_PWR_0_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SWITCH_RF_PWR_1_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_RF_PWR_1_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_EXT_ONEKEY_IT_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_EXT_ONEKEY_IT_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SWITCH_IT_CHAN_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_IT_CHAN_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SWITCH_RMT_CHAN_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_RMT_CHAN_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SET_PWR_CAL_0_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_PWR_CAL_0_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SET_PWR_CAL_1_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SET_PWR_CAL_1_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_RST_MCU_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RST_MCU_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_RF_PWR_AUTO_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_RF_PWR_AUTO_Handler\n");

    return 0;
}

uint8_t PAD_FREQUENCY_BAND_WIDTH_SELECT_Handler(void *param)
{	
	HAL_BB_SetFreqBandwidthSelectionProxy( (ENUM_CH_BW)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_FREQUENCY_BAND_OPERATION_MODE_Handler(void *param)
{
	HAL_BB_SetFreqBandSelectionModeProxy( (ENUM_RUN_MODE)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_FREQUENCY_BAND_SELECT_Handler(void *param)
{
	HAL_BB_SetFreqBandProxy( (ENUM_RF_BAND)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_FREQUENCY_CHANNEL_OPERATION_MODE_Handler(void *param)
{
	HAL_BB_SetItChannelSelectionModeProxy( (ENUM_RUN_MODE)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_FREQUENCY_CHANNEL_SELECT_Handler(void *param)
{	
	HAL_BB_SetItChannelProxy( (uint8_t)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_MCS_OPERATION_MODE_Handler(void *param)
{
	HAL_BB_SetMcsModeProxy( (ENUM_RUN_MODE)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_MCS_MODULATION_MODE_Handler(void *param)
{
	HAL_BB_SetItQamProxy( (ENUM_BB_QAM)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_ENCODER_DYNAMIC_BITRATE_MODE_Handler(void *param)
{
    HAL_BB_SetEncoderBrcModeProxy( (ENUM_RUN_MODE)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_ENCODER_DYNAMIC_BITRATE_SELECT_Handler(void *param)
{
    HAL_BB_SetEncoderBitrateProxy( (uint8_t)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_WIRELESS_OSD_DISPLAY_Handler(void *param)
{
    ENUM_WIRELESS_TOOL toolToHost = MCU_TO_PAD;
    dlog_info("PAD_WIRELESS_OSD_DISPLAY_Handler\n");
    WITELESS_GetOSDInfo();
    WIRELESS_SendOSDInfo(toolToHost);

    return 0;
}

void WIRELESS_ParseParamConfig(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstWirelessParamConfig;

	pstWirelessParamConfig          = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    // insert message to the buffer tail
    memcpy((void *)&g_stWirelessParamConfig.stMsgPool[g_stWirelessParamConfig.u8_buffTail],
           (void *)pstWirelessParamConfig,
           sizeof(STRU_WIRELESS_PARAM_CONFIG_MESSAGE));

    g_stWirelessParamConfig.u8_buffTail++;
    g_stWirelessParamConfig.u8_buffTail &= (WIRELESS_INTERFACE_MAX_MESSAGE_NUM - 1);

    if (g_stWirelessParamConfig.u8_buffTail == g_stWirelessParamConfig.u8_buffHead)
    {
        dlog_error("wireless buff is full");
    }

    return;
}


static void Wireless_Task(void const *argument)
{
    uint8_t                                 messageId;
    uint8_t                                 u8_handlerRet;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstWirelessParamConfig;

    dlog_info("wireless task entry");

    while (1)
    {
        if (g_stWirelessParamConfig.u8_buffTail != g_stWirelessParamConfig.u8_buffHead)
        {
            // get the head node from the buffer
            pstWirelessParamConfig = &g_stWirelessParamConfig.stMsgPool[g_stWirelessParamConfig.u8_buffHead];

            messageId = pstWirelessParamConfig->messageId;

            if (g_stWirelessMsgHandler[messageId])
            {
                u8_handlerRet = (g_stWirelessMsgHandler[messageId])(pstWirelessParamConfig);

                if (0 != u8_handlerRet)
                {
                    g_stWirelessParamConfig.u16_sendfailCount++;

                    if (g_stWirelessParamConfig.u16_sendfailCount >= WIRELESS_INTERFACE_SEND_FAIL_RETRY)
                    {
                        g_stWirelessParamConfig.u16_sendfailCount = 0;

                        dlog_error("fail counter exceed 50, restart usb");

                        HAL_USB_ResetDevice(NULL);

                        osDelay(1000);
                    }
                }
                else
                {
                    g_stWirelessParamConfig.u16_sendfailCount = 0;

                    g_stWirelessParamConfig.u8_buffHead++;
                    g_stWirelessParamConfig.u8_buffHead &= (WIRELESS_INTERFACE_MAX_MESSAGE_NUM - 1);
                }
            }
            else
            {
                dlog_error("no this message handler");

                g_stWirelessParamConfig.u8_buffHead++;
                g_stWirelessParamConfig.u8_buffHead &= (WIRELESS_INTERFACE_MAX_MESSAGE_NUM - 1);
            }

        }

        osDelay(10);
    }
}


void Wireless_TaskInit(void)
{
    memset((void *)&g_stWirelessParamConfig, 0, sizeof(STRU_WIRELESS_MESSAGE_BUFF));

    HAL_USB_RegisterUserProcess(WIRELESS_ParseParamConfig);

    osThreadDef(WIRELESS_TASK, Wireless_Task, osPriorityNormal, 0, 4 * 128);
    osThreadCreate(osThread(WIRELESS_TASK), NULL);
}



