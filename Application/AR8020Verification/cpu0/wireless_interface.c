#include <string.h>
#include "wireless_interface.h"
#include "hal_usb_device.h"
#include "debuglog.h"
#include "hal_bb.h"
#include "hal_sram.h"
#include "bb_types.h"
#include "cmsis_os.h"
#include "md5.h"
#include "nor_flash.h"

STRU_WIRELESS_INFO_DISPLAY             *g_pstWirelessInfoDisplay;        //OSD Info in SRAM
STRU_WIRELESS_INFO_DISPLAY              g_stWirelessInfoSend;            //send OSD to PAD or PC
STRU_WIRELESS_MESSAGE_BUFF              g_stWirelessParamConfig;     //receive from PAD or PC
STRU_WIRELESS_MESSAGE_BUFF              g_stWirelessReply;     //send to PAD or PC


uint8_t eventFlag = 0;


WIRELESS_CONFIG_HANDLER g_stWirelessMsgHandler[MAX_PID_NUM] = 
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
    WIRELESS_INTERFACE_OPEN_ADAPTION_BIT_STREAM_Handler,
    WIRELESS_INTERFACE_SWITCH_CH1_Handler,
    WIRELESS_INTERFACE_SWITCH_CH2_Handler,
    WIRELESS_INTERFACE_SET_CH1_BIT_RATE_Handler,
    WIRELESS_INTERFACE_SET_CH2_BIT_RATE_Handler,
    WIRELESS_INTERFACE_VIDEO_QAM_Handler,
    WIRELESS_INTERFACE_VIDEO_CODE_RATE_Handler,
    WIRELESS_INTERFACE_RC_QAM_Handler,
    WIRELESS_INTERFACE_RC_CODE_RATE_Handler,
    WIRELESS_INTERFACE_OPEN_VIDEO_Handler,
    WIRELESS_INTERFACE_CLOSE_VIDEO_Handler,
    WIRELESS_INTERFACE_VIDEO_AUTO_HOPPING_Handler,
    WIRELESS_INTERFACE_VIDEO_BAND_WIDTH_Handler,
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


uint8_t WIRELESS_IsInDebugMode(void)
{
    g_pstWirelessInfoDisplay  = (STRU_WIRELESS_INFO_DISPLAY *)OSD_STATUS_SHM_ADDR;

    return g_pstWirelessInfoDisplay->in_debug;
}


/* get osd info from shared memory */
uint8_t WIRELESS_GetOSDInfo(void)
{
    g_pstWirelessInfoDisplay  = (STRU_WIRELESS_INFO_DISPLAY *)OSD_STATUS_SHM_ADDR;

    /* if cpu2 update info, and the info is valid */
    if ((0x0 == g_pstWirelessInfoDisplay->head)
      &&(0xFF == g_pstWirelessInfoDisplay->tail)
      &&(0x0 == g_pstWirelessInfoDisplay->in_debug))
    {
        memcpy((void *)&g_stWirelessInfoSend, (void *)g_pstWirelessInfoDisplay, sizeof(STRU_WIRELESS_INFO_DISPLAY));

        return 0;
    }

    return 1;
}

/* Send to PAD or PC */
void WIRELESS_SendOSDInfo(ENUM_WIRELESS_TOOL host)
{
    uint32_t                                sendLength;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE      recvMessage;

    if (WIRELESS_GetOSDInfo())
    {
        return;
    }

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

    recvMessage.messageId       = g_stWirelessInfoSend.messageId;
    recvMessage.paramLen        = g_stWirelessInfoSend.paramLen;

    Wireless_InsertMsgIntoReplyBuff(&recvMessage);

    return;
}


void UpgradeFirmwareFromPCTool(void *upgradeData)
{
    uint8_t             u8_replyToHost;
    uint8_t             u8_finalPacket;
    uint16_t            u16_packetLen;
    static uint8_t     *u8_sdramAddr = HID_UPGRADE_BASE_ADDR;
    uint32_t            u32_ImageSize;
    uint8_t            *u8_imageRawData;
    uint8_t             u8_md5sum[16];
    static uint32_t     u32_RecCount = 0;       // image size count
    uint32_t            u32_RecCountImage;      // image raw data size, exclude header
    uint8_t             u8_i;
    uint8_t            *u8_temp;

    u8_replyToHost      = *(uint8_t *)upgradeData;
    u8_finalPacket      = *((uint8_t *)upgradeData + 1);
    u16_packetLen       = *((uint16_t *)((uint8_t *)upgradeData + 2));

    if (u8_replyToHost != WIRELESS_INTERFACE_UPGRADE)
    {
        dlog_error("it is not a upgrade packet");

        return;
    }

    /* print this message at the first time */
    if (HID_UPGRADE_BASE_ADDR == u8_sdramAddr)
    {
        dlog_info("receiving image");
    }

    /* copy to SDRAM */
    memcpy(u8_sdramAddr,
           ((uint8_t *)upgradeData + 4),
           u16_packetLen);

    u8_sdramAddr       += u16_packetLen;
    u32_RecCount       += u16_packetLen;

    /* app image all received in SDRAM, start to upgrade app from SDRAM */
    if (u8_finalPacket)
    {
        dlog_info("CRC checking");

        /* 1-1. pointer to the image raw data */
        u8_imageRawData     = (HID_UPGRADE_BASE_ADDR + 34);

        /* 1-2. image size exclude header */
        u32_RecCountImage   = (u32_RecCount - 34);

        /* 1-3. get md5 sum from image header */
        u8_temp             = (HID_UPGRADE_BASE_ADDR + 18);
        for(u8_i = 0; u8_i < 16; u8_i++)
        {
            u8_md5sum[u8_i] = *(u8_temp + u8_i);
        }

        /* 1-4. get image size from image header */
        u8_temp         = (HID_UPGRADE_BASE_ADDR + 14);
        u32_ImageSize   = (((uint32_t)(*u8_temp)) | \
                           (((uint32_t)(*(u8_temp + 1))) << 8) | \
                           (((uint32_t)(*(u8_temp + 2))) << 16) | \
                           (((uint32_t)(*(u8_temp + 3))) << 24));

        if (u32_ImageSize != u32_RecCount)
        {
            dlog_error("received length incorrect");
        }

        /* 1-5. check MD5 */
        MD5Check(u8_imageRawData,
                 u32_RecCountImage,
                 u8_md5sum);

        /* 2-1. burn to norflash */
        for(u8_i = 0; u8_i < (u32_RecCount/HID_UPGRADE_FLASH_SECTOR_SIZE); u8_i++)
        {
            NOR_FLASH_EraseSector(HID_UPGRADE_APP_ADDR_IN_NOR + HID_UPGRADE_FLASH_SECTOR_SIZE*u8_i);
            NOR_FLASH_WriteByteBuffer((HID_UPGRADE_APP_ADDR_IN_NOR + HID_UPGRADE_FLASH_SECTOR_SIZE*u8_i),
                                      (HID_UPGRADE_BASE_ADDR + HID_UPGRADE_FLASH_SECTOR_SIZE*u8_i),
                                      HID_UPGRADE_FLASH_SECTOR_SIZE);
        }

        if(0 != (u32_RecCount%HID_UPGRADE_FLASH_SECTOR_SIZE))
        {
            NOR_FLASH_EraseSector(HID_UPGRADE_APP_ADDR_IN_NOR + HID_UPGRADE_FLASH_SECTOR_SIZE*u8_i);
            NOR_FLASH_WriteByteBuffer((HID_UPGRADE_APP_ADDR_IN_NOR + HID_UPGRADE_FLASH_SECTOR_SIZE*u8_i),
                                      (HID_UPGRADE_BASE_ADDR + HID_UPGRADE_FLASH_SECTOR_SIZE*u8_i),
                                      HID_UPGRADE_FLASH_SECTOR_SIZE);
        }

        /* re-check image MD5 in Norflash */
        /* 3-1. pointer to the image raw data */
        u8_temp             = (HID_UPGRADE_FLASH_BASE_ADDR + HID_UPGRADE_APP_ADDR_IN_NOR);
        u8_imageRawData     = (u8_temp + 34);

        /* 3-2. get md5 sum from image header */
        u8_temp             = ((HID_UPGRADE_FLASH_BASE_ADDR + HID_UPGRADE_APP_ADDR_IN_NOR) + 18);
        for(u8_i = 0; u8_i < 16; u8_i++)
        {
            u8_md5sum[u8_i] = *((uint8_t*)(u8_temp + u8_i));
        }

        /* 3-3. get image size from image header */
        u8_temp         = ((HID_UPGRADE_FLASH_BASE_ADDR + HID_UPGRADE_APP_ADDR_IN_NOR) + 14);
        u32_ImageSize   = (((uint32_t)(*u8_temp)) | \
                           (((uint32_t)(*(u8_temp + 1))) << 8) | \
                           (((uint32_t)(*(u8_temp + 2))) << 16) | \
                           (((uint32_t)(*(u8_temp + 3))) << 24));

        u32_RecCountImage   = (u32_ImageSize - 34);

        /* 3-4. check MD5 again */
        MD5Check(u8_imageRawData,
                 u32_RecCountImage,
                 u8_md5sum);

        dlog_info("upgrade app success");
    }

    while(HAL_OK != (HAL_USB_DeviceSendCtrl((uint8_t *)upgradeData, 4)))
    {
        dlog_error("upgrade reply to host fail");
    }

}
 


uint8_t WIRELESS_INTERFACE_UPGRADE_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE   st_replyMessage;

    dlog_info("enter upgrade mode");

    HAL_USB_RegisterUserProcess(UpgradeFirmwareFromPCTool);

    st_replyMessage.messageId   = WIRELESS_INTERFACE_UPGRADE;
    st_replyMessage.paramLen    = 0;

    HAL_USB_DeviceSendCtrl((uint8_t *)&st_replyMessage, sizeof(STRU_WIRELESS_PARAM_CONFIG_MESSAGE));

    return 0;
}


uint8_t WIRELESS_INTERFACE_ENTER_TEST_MODE_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_ENTER_TEST_MODE_Handler\n"); 

    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (WIRELESS_IsInDebugMode() == 1)
    {
        HAL_BB_WriteByte(PAGE2, 0x02, 0x06);
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

    if (WIRELESS_IsInDebugMode() == 1)
    {
        if (HAL_BB_CurPageWriteByte(recvMessage->paramData[0], recvMessage->paramData[1]))
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

        Wireless_InsertMsgIntoReplyBuff(recvMessage);
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_READ_BB_REG_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (WIRELESS_IsInDebugMode() == 1)
    {
        recvMessage->messageId = 0x0f;
        recvMessage->paramLen = 2;
        recvMessage->paramData[0] = recvMessage->paramData[0];
        HAL_BB_CurPageReadByte(recvMessage->paramData[0], &recvMessage->paramData[1]);

        Wireless_InsertMsgIntoReplyBuff(recvMessage);
    }
    else
    {   
        recvMessage->messageId = 0x0f;
        recvMessage->paramLen = 0;

        Wireless_InsertMsgIntoReplyBuff(recvMessage);
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
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE        *stDeviceInfo;

    stDeviceInfo            = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)DEVICE_INFO_SHM_ADDR;

    stDeviceInfo->messageId = WIRELESS_INTERFACE_GET_DEV_INFO;
    stDeviceInfo->paramLen  = 9;

    dlog_info("WIRELESS_INTERFACE_GET_DEV_INFO_Handler\n");

    Wireless_InsertMsgIntoReplyBuff(stDeviceInfo);

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

    if (WIRELESS_IsInDebugMode() == 1)
    {
        for (u8_index = 0; u8_index < recvMessage->paramData[2]; ++u8_index)
        {
            HAL_BB_WriteByte(recvMessage->paramData[0],recvMessage->paramData[1],recvMessage->paramData[u8_index+3]);
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

    if (WIRELESS_IsInDebugMode() == 1)
    {
        for (u8_index = 0; u8_index < recvMessage->paramData[2]; ++u8_index)
        {
            HAL_BB_WriteByte(recvMessage->paramData[0],recvMessage->paramData[1],recvMessage->paramData[u8_index+3]);
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
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_RF_BAND                         enRfBand;

    recvMessage                 = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (recvMessage->paramData[0] == 0)
    {
        dlog_info("2.4 G\n");
        enRfBand = RF_2G;
    }
    else
    {
        dlog_info("5.8 G\n");
        enRfBand = RF_5G;
    }

    HAL_BB_SetFreqBandProxy(enRfBand);

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
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_RUN_MODE                        e_mode;

    recvMessage     = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (recvMessage->paramData[0] == 1)
    {
        e_mode      = AUTO;

        dlog_info("RC HOPPING MODE\n");
    }
    else
    {
        e_mode      = MANUAL;

        dlog_info("RC SELECT MODE\n");
    }

    HAL_BB_SetItChannelSelectionModeProxy(e_mode);

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

        inDebugFlag = 1;
    }
    else if (recvMessage->paramData[0] != 0 && recvMessage->paramData[1] == 0)
    {
        /*exit debug mode */
        if (eventFlag)
        {
            HAL_BB_SetBoardDebugModeProxy(1);  
	        eventFlag = 0;
        }

        inDebugFlag = 0;
    }

    /*send to PC*/
    recvMessage->paramData[1] = inDebugFlag;

    Wireless_InsertMsgIntoReplyBuff(recvMessage);

    return 0;
}

uint8_t WIRELESS_INTERFACE_WRITE_RF_REG_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    dlog_info("ID = %x\n",recvMessage->messageId);

    if (WIRELESS_IsInDebugMode() == 1)
    {
        if (HAL_RF8003S_WriteReg(recvMessage->paramData[0], recvMessage->paramData[1]))
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

        Wireless_InsertMsgIntoReplyBuff(recvMessage);
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_OPEN_VIDEO_Handler(void *param)
{
    HAL_SRAM_OpenVideo();

    return 0;
}


uint8_t WIRELESS_INTERFACE_CLOSE_VIDEO_Handler(void *param)
{
    HAL_SRAM_CloseVideo();

    return 0;
}


uint8_t WIRELESS_INTERFACE_READ_RF_REG_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    dlog_info("ID = %x\n",recvMessage->messageId);

    if (WIRELESS_IsInDebugMode() == 1)
    {
        recvMessage->messageId = 0x3A;
        recvMessage->paramLen = 2;
        recvMessage->paramData[0] = recvMessage->paramData[0];
        if (HAL_RF8003S_ReadByte(recvMessage->paramData[0],&recvMessage->paramData[1]))
        {
            dlog_error("read rf8003s error\n");

            return 1;
        }

        Wireless_InsertMsgIntoReplyBuff(recvMessage);
    }
    else
    {
        recvMessage->messageId = 0x0f;
        recvMessage->paramLen = 0;

        Wireless_InsertMsgIntoReplyBuff(recvMessage);
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


uint8_t WIRELESS_INTERFACE_OPEN_ADAPTION_BIT_STREAM_Handler(void *param)
{
    ENUM_RUN_MODE                        enRunMode;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;

    recvMessage             = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (recvMessage->paramData[0] == 1)
    {
        enRunMode           = AUTO;

        dlog_info("auto\n");
    }
    else
    {
        enRunMode           = MANUAL;

        dlog_info("manual\n");
    }

    HAL_BB_SetEncoderBrcModeProxy(enRunMode);

    return 0;
}

uint8_t WIRELESS_INTERFACE_SWITCH_CH1_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_CH1_Handler");

    return 0;
}

uint8_t WIRELESS_INTERFACE_SWITCH_CH2_Handler(void *param)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_CH2_Handler");

    return 0;
}

uint8_t WIRELESS_INTERFACE_SET_CH1_BIT_RATE_Handler(void *param)
{
    dlog_info("set ch1 bit rate");

    HAL_BB_SetEncoderBitrateProxyCh1( (uint8_t)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}

uint8_t WIRELESS_INTERFACE_SET_CH2_BIT_RATE_Handler(void *param)
{
    dlog_info("set ch2 bit rate");

    HAL_BB_SetEncoderBitrateProxyCh2( (uint8_t)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}

uint8_t WIRELESS_INTERFACE_VIDEO_QAM_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_BB_QAM                          enBBQAM;

    recvMessage         = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    switch (recvMessage->paramData[0])
    {
        dlog_info("QAM : %d", recvMessage->paramData[0]);
        case 0:
            enBBQAM         = MOD_BPSK;
            break;

        case 1:
            enBBQAM         = MOD_4QAM;
            break;

        case 2:
            enBBQAM         = MOD_16QAM;
            break;

        case 3:
            enBBQAM         = MOD_64QAM;
            break;

        case 4:
            enBBQAM         = MOD_QPSK;
            break;

        default:
            enBBQAM         = MOD_BPSK;
            break;
    }

	HAL_BB_SetItQamProxy(enBBQAM);

    return 0;
}

uint8_t WIRELESS_INTERFACE_VIDEO_CODE_RATE_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_BB_LDPC                         enldpc;

    recvMessage         = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    switch (recvMessage->paramData[0])
    {
        dlog_info("ldpc: %d", recvMessage->paramData[0]);

        case 0:
            enldpc  = LDPC_1_2;
            break;

        case 1:
            enldpc  = LDPC_2_3;
            break;

        case 2:
            enldpc  = LDPC_3_4;
            break;

        case 3:
            enldpc  = LDPC_5_6;
            break;

        default:
            enldpc  = LDPC_1_2;
            break;
    }

    HAL_BB_SetItLdpcProxy(enldpc);

    return 0;
}

uint8_t WIRELESS_INTERFACE_RC_QAM_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_BB_QAM                          enBBQAM;

    recvMessage         = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    dlog_info("RC QAM : %d", recvMessage->paramData[0]);

    switch (recvMessage->paramData[0])
    {
        case 0:
            enBBQAM         = MOD_BPSK;
            break;

        case 4:
            enBBQAM         = MOD_QPSK;
            break;

        default:
            enBBQAM         = MOD_BPSK;
            break;
    }

    HAL_BB_SetItQamProxy(enBBQAM);

    return 0;
}

uint8_t WIRELESS_INTERFACE_RC_CODE_RATE_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_BB_LDPC                         e_ldpc;

    recvMessage         = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    dlog_info("RC CODE RATE: %d", recvMessage->paramData[0]);

    switch (recvMessage->paramData[0])
    {
        case 1:
            e_ldpc      = LDPC_1_2;
            break;

        case 2:
            e_ldpc      = LDPC_2_3;        
            break;

        default:
            e_ldpc      = LDPC_1_2;
            break;
    }

    HAL_BB_SetItLdpcProxy(e_ldpc);

    return 0;
}

uint8_t WIRELESS_INTERFACE_VIDEO_AUTO_HOPPING_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_RUN_MODE                        e_mode;

    recvMessage     = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (recvMessage->paramData[0] == 1)
    {
        e_mode      = AUTO;

        dlog_info("VIDEO HOPPING MODE\n");
    }
    else
    {
        e_mode      = MANUAL;

        dlog_info("VIDEO SELECT MODE\n");
    }

    HAL_BB_SetItChannelSelectionModeProxy(e_mode);

    return 0;
}

uint8_t WIRELESS_INTERFACE_VIDEO_BAND_WIDTH_Handler(void *param)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_CH_BW                           enBandWidth;

    recvMessage                 = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (recvMessage->paramData[0] == 2)
    {
        dlog_info("10M Bandwidth\n");
        enBandWidth             = BW_10M;
    }
    else
    {
        dlog_info("20M Bandwidth\n");
        enBandWidth             = BW_20M;
    }

	HAL_BB_SetFreqBandwidthSelectionProxy(enBandWidth);

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
    HAL_BB_SetEncoderBitrateProxyCh1( (uint8_t)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_WIRELESS_OSD_DISPLAY_Handler(void *param)
{
    ENUM_WIRELESS_TOOL toolToHost = MCU_TO_PAD;

    dlog_info("PAD_WIRELESS_OSD_DISPLAY_Handler\n");

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
    uint8_t                                *u8_sendBuff;
    uint32_t                                u32_sendLength;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstWirelessParamConfig;

    dlog_info("wireless task entry");

    while (1)
    {
        if (g_stWirelessReply.u8_buffTail != g_stWirelessReply.u8_buffHead)
        {
            pstWirelessParamConfig = &g_stWirelessReply.stMsgPool[g_stWirelessReply.u8_buffHead];

            messageId              = pstWirelessParamConfig->messageId;

            if ((messageId == PAD_WIRELESS_INTERFACE_OSD_DISPLAY)||
                (messageId == WIRELESS_INTERFACE_OSD_DISPLAY))
            {
                u8_sendBuff         = (uint8_t *)&g_stWirelessInfoSend;
                u32_sendLength      = g_stWirelessInfoSend.paramLen;
            }
            else
            {
                u8_sendBuff         = (uint8_t *)pstWirelessParamConfig;
                u32_sendLength      = (uint32_t)sizeof(STRU_WIRELESS_PARAM_CONFIG_MESSAGE);
            }

            if (HAL_OK != HAL_USB_DeviceSendCtrl(u8_sendBuff, u32_sendLength))
            {
                if ((messageId == PAD_WIRELESS_INTERFACE_OSD_DISPLAY)||
                    (messageId == WIRELESS_INTERFACE_OSD_DISPLAY))
                {
                    g_stWirelessReply.u16_sendfailCount = 0;
                    g_stWirelessReply.u8_buffHead++;
                    g_stWirelessReply.u8_buffHead &= (WIRELESS_INTERFACE_MAX_MESSAGE_NUM - 1);
                }
                else
                {
                    g_stWirelessReply.u16_sendfailCount++;

                    if (g_stWirelessReply.u16_sendfailCount >= WIRELESS_INTERFACE_SEND_FAIL_RETRY)
                    {
                        g_stWirelessParamConfig.u16_sendfailCount = 0;

                        dlog_error("fail counter exceed threshold");

                        HAL_USB_ResetDevice(NULL);

                        osDelay(500);
                    }
                }
            }
            else
            {
                g_stWirelessReply.u16_sendfailCount = 0;

                g_stWirelessReply.u8_buffHead++;
                g_stWirelessReply.u8_buffHead &= (WIRELESS_INTERFACE_MAX_MESSAGE_NUM - 1);
            }
        }

        if (g_stWirelessParamConfig.u8_buffTail != g_stWirelessParamConfig.u8_buffHead)
        {
            // get the head node from the buffer
            pstWirelessParamConfig = &g_stWirelessParamConfig.stMsgPool[g_stWirelessParamConfig.u8_buffHead];

            messageId = pstWirelessParamConfig->messageId;

            if (messageId < MAX_PID_NUM)
            {
                if (g_stWirelessMsgHandler[messageId])
                {
                    (g_stWirelessMsgHandler[messageId])(pstWirelessParamConfig);
                }
                else
                {
                    dlog_error("no this message handler,%d", messageId);
                }
            }

            g_stWirelessParamConfig.u8_buffHead++;
            g_stWirelessParamConfig.u8_buffHead &= (WIRELESS_INTERFACE_MAX_MESSAGE_NUM - 1);
        }

        osDelay(10);
    }
}


void Wireless_TaskInit(void)
{
    memset((void *)&g_stWirelessParamConfig, 0, sizeof(STRU_WIRELESS_MESSAGE_BUFF));
    memset((void *)OSD_STATUS_SHM_ADDR, 0, 512);

    HAL_USB_RegisterUserProcess(WIRELESS_ParseParamConfig);

    osThreadDef(WIRELESS_TASK, Wireless_Task, osPriorityNormal, 0, 4 * 128);
    osThreadCreate(osThread(WIRELESS_TASK), NULL);
}


static void Wireless_InsertMsgIntoReplyBuff(STRU_WIRELESS_PARAM_CONFIG_MESSAGE *pstMessage)
{
    // insert message to the buffer tail
    memcpy((void *)&g_stWirelessReply.stMsgPool[g_stWirelessReply.u8_buffTail],
           (void *)pstMessage,
           sizeof(STRU_WIRELESS_PARAM_CONFIG_MESSAGE));

    g_stWirelessReply.u8_buffTail++;
    g_stWirelessReply.u8_buffTail &= (WIRELESS_INTERFACE_MAX_MESSAGE_NUM - 1);

    if (g_stWirelessReply.u8_buffTail == g_stWirelessReply.u8_buffHead)
    {
        dlog_error("reply buff is full");
        g_stWirelessReply.u8_buffHead = 0;
        g_stWirelessReply.u8_buffHead = 0;
    }

    return;
}




