/**
  ******************************************************************************
  * @file    usbd_hid.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                HID Class  Description
  *          =================================================================== 
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick
  *             - Collection : Application 
  *      
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid.h"
#include "usbd_hid_desc.h"
#include "usbd_ctlreq.h"
#include "usbd_core.h"
#include "debuglog.h"
#include "sram.h"
#include "bb_types.h"
#include "sys_event.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_HID 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_HID_Private_Defines
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_HID_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 




/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_HID_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_HID_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_HID_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  *USBD_HID_GetCfgDesc (uint16_t *length);

static uint8_t  *USBD_HID_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t  USBD_HID_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_HID_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  *USBD_HID_GetUsrStrDescriptor(USBD_HandleTypeDef *pdev, uint8_t index, uint16_t *length);

/**
  * @}
  */ 

/** @defgroup USBD_HID_Private_Variables
  * @{
  */ 

USBD_ClassTypeDef  USBD_HID = 
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  NULL, /*EP0_TxSent*/  
  NULL, /*EP0_RxReady*/
  USBD_HID_DataIn, /*DataIn*/
  USBD_HID_DataOut, /*DataOut*/
  NULL, /*SOF */
  NULL,
  NULL,      
  USBD_HID_GetCfgDesc,
  USBD_HID_GetCfgDesc, 
  USBD_HID_GetCfgDesc,
  USBD_HID_GetDeviceQualifierDesc,
#if (USBD_SUPPORT_USER_STRING == 1)
  USBD_HID_GetUsrStrDescriptor,
#endif  
};

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgDesc[USB_HID_CONFIG_DESC_SIZ]  __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x04,         /*bNumInterfaces: 4 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0x80,         /*bmAttributes: bus powered and Support Remote Wake-up */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/
  
  /************** Descriptor of Joystick Mouse interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  HID_COMM_STRING_INTERFACE,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of Mouse endpoint ********************/
  /* 27 */
  /* 34 */
  0x07,
  USB_DESC_TYPE_ENDPOINT,
  HID_EPIN_CTRL_ADDR,
  USBD_EP_TYPE_BULK,
  LOBYTE(HID_EPIN_CTRL_SIZE),
  HIBYTE(HID_EPIN_CTRL_SIZE),
  HID_HS_BINTERVAL,
  /* 41 */
  0x07,
  USB_DESC_TYPE_ENDPOINT,
  HID_EPOUT_ADDR,
  USBD_EP_TYPE_INTR,
  LOBYTE(HID_EPOUT_SIZE),
  HIBYTE(HID_EPOUT_SIZE),
  HID_HS_BINTERVAL,

  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x01,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  HID_VIDEO0_STRING_INTERFACE,            /*iInterface: Index of string descriptor*/

  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,

  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
  HID_EPIN_VIDEO_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  USBD_EP_TYPE_BULK,          /*bmAttributes: BULK endpoint*/
  LOBYTE(HID_EPIN_VIDEO_SIZE), /*wMaxPacketSize: 4 Byte max */
  HIBYTE(HID_EPIN_VIDEO_SIZE),
  HID_HS_BINTERVAL,          /*bInterval: Polling Interval (10 ms)*/

  0x07,
  USB_DESC_TYPE_ENDPOINT,
  HID_EPOUT_VIDEO_ADDR,
  USBD_EP_TYPE_INTR,
  LOBYTE(HID_EPOUT_VIDEO_SIZE),
  HIBYTE(HID_EPOUT_VIDEO_SIZE),
  HID_HS_BINTERVAL,

  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x02,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x01,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  HID_AUDIO_STRING_INTERFACE,            /*iInterface: Index of string descriptor*/

  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,

  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
  HID_EPIN_AUDIO_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  USBD_EP_TYPE_BULK,          /*bmAttributes: BULK endpoint*/
  LOBYTE(HID_EPIN_AUDIO_SIZE), /*wMaxPacketSize: 4 Byte max */
  HIBYTE(HID_EPIN_AUDIO_SIZE),
  HID_HS_BINTERVAL,          /*bInterval: Polling Interval (10 ms)*/

  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x03,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  HID_CUSTOMER_STRING_INTERFACE,            /*iInterface: Index of string descriptor*/

  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,

  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
  HID_CUSTOMER_OUT_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  USBD_EP_TYPE_INTR,          /*bmAttributes: BULK endpoint*/
  LOBYTE(HID_CUSTOMER_OUT_SIZE), /*wMaxPacketSize: 4 Byte max */
  HIBYTE(HID_CUSTOMER_OUT_SIZE),
  HID_HS_BINTERVAL,          /*bInterval: Polling Interval (10 ms)*/

  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
  HID_CUSTOMER_IN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  USBD_EP_TYPE_BULK,          /*bmAttributes: BULK endpoint*/
  LOBYTE(HID_CUSTOMER_IN_SIZE), /*wMaxPacketSize: 4 Byte max */
  HIBYTE(HID_CUSTOMER_IN_SIZE),
  HID_HS_BINTERVAL,          /*bInterval: Polling Interval (10 ms)*/
} ;

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_Desc[USB_HID_DESC_SIZ]  __ALIGN_END  =
{
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC]  __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE]  __ALIGN_END =
{
  0x06,   0x00,
  0xFF,   0x09,
  0x01,   0xA1,
  0x01,   0x19,
  
  0x01,   0x2A,
  0x00,   0x02,
  0x15,   0x00,
  0x26,   0xFF,
  
  0x00,   0x75,
  0x08,   0x96,
  0x00,   0x02,
  0x81,   0x00,
  
  0x19,   0x01,
  0x29,   0x40,
  0x15,   0x00,
  0x26,   0xFF,
  
  0x00,   0x75,
  0x08,   0x95,
  0x40,   0x91,
  0x00,   0xC0,
}; 

#define HID_COMM_STRING_DESC        "ArtosynComm"
#define HID_VIDEO0_STRING_DESC      "ArtosynVideo0"
#define HID_VIDEO1_STRING_DESC      "ArtosynVideo1"
#define HID_AUDIO_STRING_DESC       "ArtosynAudio"
#define HID_CUSTOMER_STRING_DESC    "ArtosynCustomer"

__ALIGN_BEGIN uint8_t HID_USER_INTERFACE_DESC[48]  __ALIGN_END;


USBD_HID_HandleTypeDef        g_usbdHidData;
uint8_t                       g_u32USBDeviceRecv[HID_EPOUT_SIZE];
uint8_t                       g_u8CustomerOut[HID_CUSTOMER_OUT_SIZE];
uint32_t                      g_u32CustomerOutSize;
uint8_t                       g_u8UsbBypassEncoderBuff[HID_EPOUT_VIDEO_SIZE];

/*
  * @}
  */ 

/** @defgroup USBD_HID_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{
    uint8_t ret = 0;
    uint8_t i;

    /* Open EP IN */
    USBD_LL_OpenEP(pdev,
                 HID_EPIN_VIDEO_ADDR,
                 USBD_EP_TYPE_INTR,
                 HID_EPIN_VIDEO_SIZE);

    USBD_LL_OpenEP(pdev,
                 HID_EPOUT_VIDEO_ADDR,
                 USBD_EP_TYPE_INTR,
                 HID_EPOUT_VIDEO_SIZE);

    USBD_LL_OpenEP(pdev,
                 HID_EPIN_AUDIO_ADDR,
                 USBD_EP_TYPE_INTR,
                 HID_EPIN_AUDIO_SIZE);

    USBD_LL_OpenEP(pdev,
                 HID_EPIN_CTRL_ADDR,
                 USBD_EP_TYPE_INTR,
                 HID_EPIN_CTRL_SIZE);

    USBD_LL_OpenEP(pdev,
                 HID_EPOUT_ADDR,
                 USBD_EP_TYPE_INTR,
                 HID_EPOUT_SIZE);

    USBD_LL_OpenEP(pdev,
                 HID_CUSTOMER_OUT_ADDR,
                 USBD_EP_TYPE_INTR,
                 HID_CUSTOMER_OUT_SIZE);

    USBD_LL_OpenEP(pdev,
                 HID_CUSTOMER_IN_ADDR,
                 USBD_EP_TYPE_INTR,
                 HID_CUSTOMER_IN_SIZE);

    pdev->pClassData = &g_usbdHidData;

    if(pdev->pClassData == NULL)
    {
        ret = 1;
    }
    else
    {
        for (i = 1; i <= 6; i++)
        {
            ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state[i] = HID_IDLE;
        }

        USBD_LL_PrepareReceive(pdev, HID_EPOUT_ADDR, g_u32USBDeviceRecv, HID_EPOUT_SIZE);

        USBD_LL_PrepareReceive(pdev, HID_CUSTOMER_OUT_ADDR, g_u8CustomerOut, HID_CUSTOMER_OUT_SIZE);

        USBD_LL_PrepareReceive(pdev, HID_EPOUT_VIDEO_ADDR, g_u8UsbBypassEncoderBuff, HID_EPOUT_VIDEO_SIZE);
    }

    pdev->u8_connState  = 1;    //connect
    pdev->u8_errorCount = 0;    //clear error count

    if (pdev->pUserData)
    {
        if (((USBD_HID_ItfTypeDef *)pdev->pUserData)->userInit)
        {
            ((USBD_HID_ItfTypeDef *)pdev->pUserData)->userInit();
        }
    }

    USBD_AddValidPortNum(pdev->id);

    return ret;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{
    uint8_t  i;

    /* Close HID EPs */
    USBD_LL_CloseEP(pdev,
                  HID_EPIN_VIDEO_ADDR);

    USBD_LL_CloseEP(pdev,
                  HID_EPIN_CTRL_ADDR);

    USBD_LL_CloseEP(pdev,
                  HID_EPOUT_ADDR);

    USBD_LL_CloseEP(pdev,
                  HID_EPIN_AUDIO_ADDR);

    USBD_LL_CloseEP(pdev,
                  HID_CUSTOMER_IN_ADDR);

    USBD_LL_CloseEP(pdev,
                  HID_CUSTOMER_OUT_ADDR);

    /* FRee allocated memory */
    for (i = 1; i <= 6; i++)
    {
        ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state[i] = HID_IDLE;
    }

    pdev->u8_connState  = 0;    //disconnect
    pdev->u8_errorCount = 0;    //clear error count

    USBD_HID_CloseVideoDisplay(pdev);

    if (pdev->pUserData)
    {
        if (((USBD_HID_ItfTypeDef *)pdev->pUserData)->userInit)
        {
            ((USBD_HID_ItfTypeDef *)pdev->pUserData)->userInit();
        }
    }

    USBD_RmvValidPortNum(pdev->id);

    return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_HID_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef*) pdev->pClassData;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
    case HID_REQ_SET_PROTOCOL:
      hhid->Protocol = (uint8_t)(req->wValue);
      break;
      
    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&hhid->Protocol,
                        1);    
      break;
      
    case HID_REQ_SET_IDLE:
      hhid->IdleState = (uint8_t)(req->wValue >> 8);
      break;
      
    case HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&hhid->IdleState,
                        1);        
      break;      
      
    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( req->wValue >> 8 == HID_REPORT_DESC)
      {
        len = MIN(HID_MOUSE_REPORT_DESC_SIZE , req->wLength);
        pbuf = HID_MOUSE_ReportDesc;
      }
      else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {
        pbuf = USBD_HID_Desc;   
        len = MIN(USB_HID_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&hhid->AltSetting,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      hhid->AltSetting = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}


/**
  * @brief  USBD_HID_ErrorDetect 
  *         detect send data error count
  * @param  
  * @retval status
  */
static void USBD_HID_ErrorDetect(USBD_HandleTypeDef *pdev)
{
    STRU_SysEvent_DEV_PLUG_OUT stDevPlugOut;

    pdev->u8_errorCount++;

    if (pdev->u8_errorCount >= 50)
    {
        dlog_error("usb send fail detect");
        pdev->u8_errorCount = 0;
        pdev->u8_connState  = 0;

        stDevPlugOut.otg_port_id = pdev->id;

        SYS_EVENT_Notify(SYS_EVENT_ID_USB_PLUG_OUT, (void *)&stDevPlugOut);
    }
}


/**
  * @brief  USBD_HID_SendReport 
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport(USBD_HandleTypeDef  *pdev, 
                           uint8_t *report,
                           uint16_t len,
                           uint8_t  ep_addr)
{
    USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef*)pdev->pClassData;
    uint8_t                     ret  = USBD_OK;

    if (pdev->dev_state == USBD_STATE_CONFIGURED)
    {
        if(hhid->state[0x7F & ep_addr] == HID_IDLE)
        {
            hhid->state[0x7F & ep_addr]   = HID_BUSY;

            /* temp add for test */
            /* take endian problem for consideration */
            if (((ep_addr | 0x80) == HID_EPIN_VIDEO_ADDR)||
                ((ep_addr | 0x80) == HID_EPIN_AUDIO_ADDR))
            {
                USB_OTG_SetBigEndian(pdev);
            }
            else
            {
                if (USB_OTG_IsBigEndian(pdev))
                {
                    USB_LL_ConvertEndian(report, report, len);
                }
            }

            ret = USBD_LL_Transmit (pdev, ep_addr, report, len);
        }
        else
        {
            ret = USBD_BUSY;

            if (((ep_addr | 0x80) != HID_EPIN_VIDEO_ADDR)&&
                ((ep_addr | 0x80) != HID_EPIN_AUDIO_ADDR))
            {
                USBD_HID_ErrorDetect(pdev);
            }
        }
    }
    else
    {
        dlog_error("usb device not connected: %d", pdev->dev_state);
        ret = USBD_FAIL;
    }

    return ret;
}

/**
  * @brief  USBD_HID_GetPollingInterval 
  *         return polling interval from endpoint descriptor
  * @param  pdev: device instance
  * @retval polling interval
  */
uint32_t USBD_HID_GetPollingInterval (USBD_HandleTypeDef *pdev)
{
  uint32_t polling_interval = 0;

  /* HIGH-speed endpoints */
  if(pdev->dev_speed == USBD_SPEED_HIGH)
  {
   /* Sets the data transfer polling interval for high speed transfers. 
    Values between 1..16 are allowed. Values correspond to interval 
    of 2 ^ (bInterval-1). This option (8 ms, corresponds to HID_HS_BINTERVAL */
    polling_interval = (((1 <<(HID_HS_BINTERVAL - 1)))/8);
  }
  else   /* LOW and FULL-speed endpoints */
  {
    /* Sets the data transfer polling interval for low and full 
    speed transfers */
    polling_interval =  HID_FS_BINTERVAL;
  }
  
  return ((uint32_t)(polling_interval));
}

/**
  * @brief  USBD_HID_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}


/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_DataIn (USBD_HandleTypeDef *pdev, 
                              uint8_t epnum)
{
    /* Ensure that the FIFO is empty before a new transfer, this condition could 
          be caused by  a new transfer before the end of the previous transfer */
    ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state[epnum & 0x7F] = HID_IDLE;

    pdev->u8_errorCount = 0;

    #ifdef ARCAST
    if ((epnum | 0x80) == HID_EPIN_VIDEO_ADDR)
    #else
    if (((epnum | 0x80) == HID_EPIN_VIDEO_ADDR)||
        ((epnum | 0x80) == HID_EPIN_AUDIO_ADDR))
    #endif
    {
        if (pdev->u8_connState== 1)
        {
            pdev->u8_connState  = 2;
        }

        if (1 == sramReady0)
        {
            SRAM_Ready0Confirm();
        }
        else if(1 == sramReady1)
        {
            SRAM_Ready1Confirm();
        }
    }

    return USBD_OK;
}


static uint8_t USBD_HID_DataOut (USBD_HandleTypeDef *pdev,
                                uint8_t epnum)
{
    USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef*)pdev->pClassData;

    if (HID_EPOUT_ADDR == epnum)
    {
        if (USB_OTG_IsBigEndian(pdev))
        {
            USB_LL_ConvertEndian(g_u32USBDeviceRecv, g_u32USBDeviceRecv, (uint32_t)sizeof(g_u32USBDeviceRecv));
        }

        if (pdev->pUserData)
        {
            if (((USBD_HID_ItfTypeDef *)pdev->pUserData)->dataOut)
            {
                ((USBD_HID_ItfTypeDef *)pdev->pUserData)->dataOut(g_u32USBDeviceRecv, pdev->id);
            }
        }

        USBD_LL_PrepareReceive(pdev, HID_EPOUT_ADDR, g_u32USBDeviceRecv, HID_EPOUT_SIZE);
    }
    else if (HID_CUSTOMER_OUT_ADDR == epnum)
    {
        if (USB_OTG_IsBigEndian(pdev))
        {
            USB_LL_ConvertEndian(g_u8CustomerOut, g_u8CustomerOut, (uint32_t)sizeof(g_u8CustomerOut));
        }

        if (pdev->pUserData)
        {
            if (((USBD_HID_ItfTypeDef *)pdev->pUserData)->customerOut)
            {
                g_u32CustomerOutSize = USBD_LL_GetRxDataSize(pdev, epnum);

                ((USBD_HID_ItfTypeDef *)pdev->pUserData)->customerOut(g_u8CustomerOut, g_u32CustomerOutSize, pdev->id);
            }
        }

        USBD_LL_PrepareReceive(pdev, HID_CUSTOMER_OUT_ADDR, g_u8CustomerOut, HID_CUSTOMER_OUT_SIZE);
    }
    else if (HID_EPOUT_VIDEO_ADDR == epnum)
    {
        if (USB_OTG_IsBigEndian(pdev))
        {
            USB_LL_ConvertEndian(g_u8UsbBypassEncoderBuff, g_u8UsbBypassEncoderBuff, (uint32_t)sizeof(g_u8UsbBypassEncoderBuff));
        }

        if (pdev->pUserData)
        {
            if (((USBD_HID_ItfTypeDef *)pdev->pUserData)->recvVideoStream)
            {
                ((USBD_HID_ItfTypeDef *)pdev->pUserData)->recvVideoStream(g_u8UsbBypassEncoderBuff, USBD_LL_GetRxDataSize(pdev, epnum), pdev->id);
            }
        }

        USBD_LL_PrepareReceive(pdev, HID_EPOUT_VIDEO_ADDR, g_u8UsbBypassEncoderBuff, HID_EPOUT_VIDEO_SIZE);
    }

    return USBD_OK;
}


/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_HID_GetDeviceQualifierDesc (uint16_t *length)
{
  *length = sizeof (USBD_HID_DeviceQualifierDesc);
  return USBD_HID_DeviceQualifierDesc;
}


uint8_t USBD_HID_RegisterInterface(USBD_HandleTypeDef *pdev,
                             USBD_HID_ItfTypeDef *fops)
{
    uint8_t  ret = USBD_FAIL;

    if(fops != NULL)
    {
        pdev->pUserData= fops;
        ret = USBD_OK;
    }

    return ret;
}



static uint8_t  *USBD_HID_GetUsrStrDescriptor(USBD_HandleTypeDef *pdev, uint8_t index, uint16_t *length)
{
    switch(index)
    {
    case HID_COMM_STRING_INTERFACE:
        USBD_GetString((uint8_t *)HID_COMM_STRING_DESC, HID_USER_INTERFACE_DESC, length);
        break;

    case HID_VIDEO0_STRING_INTERFACE:
        USBD_GetString((uint8_t *)HID_VIDEO0_STRING_DESC, HID_USER_INTERFACE_DESC, length);
        break;

    case HID_AUDIO_STRING_INTERFACE:
        USBD_GetString((uint8_t *)HID_AUDIO_STRING_DESC, HID_USER_INTERFACE_DESC, length);
        break;

    case HID_CUSTOMER_STRING_INTERFACE:
        USBD_GetString((uint8_t *)HID_CUSTOMER_STRING_DESC, HID_USER_INTERFACE_DESC, length);
        break;

    default:
        USBD_GetString((uint8_t *)HID_COMM_STRING_DESC, HID_USER_INTERFACE_DESC, length);
        break;
    }

    return HID_USER_INTERFACE_DESC;
}


void USBD_HID_OpenVideoDisplay(USBD_HandleTypeDef *pdev)
{
    pdev->u8_videoDisplay   = 1;
}


void USBD_HID_CloseVideoDisplay(USBD_HandleTypeDef *pdev)
{
    pdev->u8_videoDisplay   = 0;
}





/**
  * @}
  */ 


/**
  * @}
  */ 


/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
