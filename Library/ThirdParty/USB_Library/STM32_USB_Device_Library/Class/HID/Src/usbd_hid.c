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
};

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgDesc[USB_HID_CONFIG_DESC_SIZ]  __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
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
  0,            /*iInterface: Index of string descriptor*/
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
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
  
  HID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  LOBYTE(HID_EPIN_SIZE), /*wMaxPacketSize: 4 Byte max */
  HIBYTE(HID_EPIN_SIZE),
  HID_HS_BINTERVAL,          /*bInterval: Polling Interval (10 ms)*/
  /* 34 */
  0x07,
  USB_DESC_TYPE_ENDPOINT,
  HID_EPOUT_ADDR,
  0x03,
  LOBYTE(HID_EPOUT_SIZE),
  LOBYTE(HID_EPOUT_SIZE),
  HID_HS_BINTERVAL,
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

USBD_HID_HandleTypeDef g_usbdHidData;


/**
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
 
  /* Open EP IN */
  USBD_LL_OpenEP(pdev,
                 HID_EPIN_ADDR,
                 USBD_EP_TYPE_INTR,
                 HID_EPIN_SIZE);  

  USBD_LL_OpenEP(pdev,
                 HID_EPOUT_ADDR,
                 USBD_EP_TYPE_INTR,
                 HID_EPOUT_SIZE);

//  pdev->pClassData = malloc(sizeof (USBD_HID_HandleTypeDef));
  pdev->pClassData = &g_usbdHidData;

  if(pdev->pClassData == NULL)
  {
    ret = 1; 
  }
  else
  {
    ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;
  }
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
  /* Close HID EPs */
  USBD_LL_CloseEP(pdev,
                  HID_EPIN_ADDR);
  
  USBD_LL_CloseEP(pdev,
                  HID_EPOUT_ADDR);

  /* FRee allocated memory */
  if(pdev->pClassData != NULL)
  {
    //free(pdev->pClassData);
    pdev->pClassData = NULL;
  } 
  
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

        /* resolve some bug unknown */
        pbuf[32] = 0x00;
        pbuf[33] = 0x75;
        pbuf[34] = 0x08;
        pbuf[35] = 0x95;
        pbuf[36] = 0x40;
        pbuf[37] = 0x91;
        pbuf[38] = 0x00;
        pbuf[39] = 0xC0;

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
  * @brief  USBD_HID_SendReport 
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport     (USBD_HandleTypeDef  *pdev, 
                                 uint8_t *report,
                                 uint16_t len)
{
  USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef*)pdev->pClassData;
  
  if (pdev->dev_state == USBD_STATE_CONFIGURED )
  {
    if(hhid->state == HID_IDLE)
    {
      hhid->state = HID_BUSY;
      USBD_LL_Transmit (pdev, 
                        HID_EPIN_ADDR,                                      
                        report,
                        len);
    }
  }
  return USBD_OK;
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
volatile uint32_t   sendFinish;

static uint8_t  USBD_HID_DataIn (USBD_HandleTypeDef *pdev, 
                              uint8_t epnum)
{
  
  /* Ensure that the FIFO is empty before a new transfer, this condition could 
  be caused by  a new transfer before the end of the previous transfer */
  ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;

  /* temp add for finish */
  uint32_t  *regAddr = 0x40b00074;
  *regAddr   &= 0xFFFFFFFD;

  sendFinish = 1;

  return USBD_OK;
}


static uint8_t USBD_HID_DataOut (USBD_HandleTypeDef *pdev,
                              uint8_t epnum)
{
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


extern void USBD_HID_InitGlobal(void)
{

  USBD_HID.Init                          = USBD_HID_Init;
  USBD_HID.DeInit                        = USBD_HID_DeInit;
  USBD_HID.Setup                         = USBD_HID_Setup;
  USBD_HID.EP0_TxSent                    = NULL;
  USBD_HID.EP0_RxReady                   = NULL;
  USBD_HID.DataIn                        = USBD_HID_DataIn;
  USBD_HID.DataOut                       = USBD_HID_DataOut;
  USBD_HID.SOF                           = NULL;
  USBD_HID.IsoINIncomplete               = NULL;
  USBD_HID.IsoOUTIncomplete              = NULL;
  USBD_HID.GetHSConfigDescriptor         = USBD_HID_GetCfgDesc;
  USBD_HID.GetFSConfigDescriptor         = USBD_HID_GetCfgDesc;
  USBD_HID.GetOtherSpeedConfigDescriptor = USBD_HID_GetCfgDesc;
  USBD_HID.GetDeviceQualifierDescriptor  = USBD_HID_GetDeviceQualifierDesc;


  USBD_HID_CfgDesc[0]    = 0x09;
  USBD_HID_CfgDesc[1]    = USB_DESC_TYPE_CONFIGURATION;
  USBD_HID_CfgDesc[2]    = USB_HID_CONFIG_DESC_SIZ;
  USBD_HID_CfgDesc[3]    = 0x00;
  USBD_HID_CfgDesc[4]    = 0x01;
  USBD_HID_CfgDesc[5]    = 0x01;
  USBD_HID_CfgDesc[6]    = 0x00;
  USBD_HID_CfgDesc[7]    = 0x80;
  USBD_HID_CfgDesc[8]    = 0x32;
  USBD_HID_CfgDesc[9]    = 0x09;
  USBD_HID_CfgDesc[10]   = USB_DESC_TYPE_INTERFACE;
  USBD_HID_CfgDesc[11]   = 0x00;
  USBD_HID_CfgDesc[12]   = 0x00;
  USBD_HID_CfgDesc[13]   = 0x02;
  USBD_HID_CfgDesc[14]   = 0x03;
  USBD_HID_CfgDesc[15]   = 0x00;
  USBD_HID_CfgDesc[16]   = 0x00;
  USBD_HID_CfgDesc[17]   = 0;
  USBD_HID_CfgDesc[18]   = 0x09;
  USBD_HID_CfgDesc[19]   = HID_DESCRIPTOR_TYPE;
  USBD_HID_CfgDesc[20]   = 0x11;
  USBD_HID_CfgDesc[21]   = 0x01;
  USBD_HID_CfgDesc[22]   = 0x00;
  USBD_HID_CfgDesc[23]   = 0x01;
  USBD_HID_CfgDesc[24]   = 0x22;
  USBD_HID_CfgDesc[25]   = HID_MOUSE_REPORT_DESC_SIZE;
  USBD_HID_CfgDesc[26]   = 0x00;
  USBD_HID_CfgDesc[27]   = 0x07;
  USBD_HID_CfgDesc[28]   = USB_DESC_TYPE_ENDPOINT;
  USBD_HID_CfgDesc[29]   = HID_EPIN_ADDR;
  USBD_HID_CfgDesc[30]   = 0x03;
  USBD_HID_CfgDesc[31]   = LOBYTE(HID_EPIN_SIZE);
  USBD_HID_CfgDesc[32]   = HIBYTE(HID_EPIN_SIZE);
  USBD_HID_CfgDesc[33]   = HID_HS_BINTERVAL;
  USBD_HID_CfgDesc[34]   = 0x07;
  USBD_HID_CfgDesc[35]   = USB_DESC_TYPE_ENDPOINT;
  USBD_HID_CfgDesc[36]   = HID_EPOUT_ADDR;
  USBD_HID_CfgDesc[37]   = 0x03;
  USBD_HID_CfgDesc[38]   = LOBYTE(HID_EPOUT_SIZE);
  USBD_HID_CfgDesc[39]   = HIBYTE(HID_EPOUT_SIZE);
  USBD_HID_CfgDesc[40]   = HID_HS_BINTERVAL;

  USBD_HID_Desc[0]  = 0x09;
  USBD_HID_Desc[1]  = HID_DESCRIPTOR_TYPE;
  USBD_HID_Desc[2]  = 0x11;
  USBD_HID_Desc[3]  = 0x01;
  USBD_HID_Desc[4]  = 0x00;
  USBD_HID_Desc[5]  = 0x01;
  USBD_HID_Desc[6]  = 0x22;
  USBD_HID_Desc[7]  = HID_MOUSE_REPORT_DESC_SIZE;
  USBD_HID_Desc[8]  = 0x00;

  USBD_HID_DeviceQualifierDesc[0]  = USB_LEN_DEV_QUALIFIER_DESC;
  USBD_HID_DeviceQualifierDesc[1]  = USB_DESC_TYPE_DEVICE_QUALIFIER;
  USBD_HID_DeviceQualifierDesc[2]  = 0x00;
  USBD_HID_DeviceQualifierDesc[3]  = 0x02;
  USBD_HID_DeviceQualifierDesc[4]  = 0x00;
  USBD_HID_DeviceQualifierDesc[5]  = 0x00;
  USBD_HID_DeviceQualifierDesc[6]  = 0x00;
  USBD_HID_DeviceQualifierDesc[7]  = 0x40;
  USBD_HID_DeviceQualifierDesc[8]  = 0x01;
  USBD_HID_DeviceQualifierDesc[9]  = 0x00;

  HID_MOUSE_ReportDesc[0]  = 0x06;
  HID_MOUSE_ReportDesc[1]  = 0x00;
  HID_MOUSE_ReportDesc[2]  = 0xFF;
  HID_MOUSE_ReportDesc[3]  = 0x09;
  HID_MOUSE_ReportDesc[4]  = 0x01;
  HID_MOUSE_ReportDesc[5]  = 0xA1;
  HID_MOUSE_ReportDesc[6]  = 0x01;
  HID_MOUSE_ReportDesc[7]  = 0x19;
  HID_MOUSE_ReportDesc[8]  = 0x01;
  HID_MOUSE_ReportDesc[9]  = 0x2A;
  HID_MOUSE_ReportDesc[10] = 0x00;
  HID_MOUSE_ReportDesc[11] = 0x02;
  HID_MOUSE_ReportDesc[12] = 0x15;
  HID_MOUSE_ReportDesc[13] = 0x00;
  HID_MOUSE_ReportDesc[14] = 0x26;
  HID_MOUSE_ReportDesc[15] = 0xFF;
  HID_MOUSE_ReportDesc[16] = 0x00;
  HID_MOUSE_ReportDesc[17] = 0x75;
  HID_MOUSE_ReportDesc[18] = 0x08;
  HID_MOUSE_ReportDesc[19] = 0x96;
  HID_MOUSE_ReportDesc[20] = 0x00;
  HID_MOUSE_ReportDesc[21] = 0x02;
  HID_MOUSE_ReportDesc[22] = 0x81;
  HID_MOUSE_ReportDesc[23] = 0x00;
  HID_MOUSE_ReportDesc[24] = 0x19;
  HID_MOUSE_ReportDesc[25] = 0x01;
  HID_MOUSE_ReportDesc[26] = 0x29;
  HID_MOUSE_ReportDesc[27] = 0x40;
  HID_MOUSE_ReportDesc[28] = 0x15;
  HID_MOUSE_ReportDesc[29] = 0x00;
  HID_MOUSE_ReportDesc[30] = 0x26;
  HID_MOUSE_ReportDesc[31] = 0xFF;
  HID_MOUSE_ReportDesc[32] = 0x00;
  HID_MOUSE_ReportDesc[33] = 0x75;
  HID_MOUSE_ReportDesc[34] = 0x08;
  HID_MOUSE_ReportDesc[35] = 0x95;
  HID_MOUSE_ReportDesc[36] = 0x40;
  HID_MOUSE_ReportDesc[37] = 0x91;
  HID_MOUSE_ReportDesc[38] = 0x00;
  HID_MOUSE_ReportDesc[39] = 0xC0;

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
