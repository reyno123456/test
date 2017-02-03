#ifndef __HAL_USB_OTG_H__
#define __HAL_USB_OTG_H__


typedef enum
{
    HAL_USB_PORT_0 = 0,
    HAL_USB_PORT_1,
} ENUM_HAL_USB_PORT;


/**
* @brief    initiate the USB Port
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_InitOTG(ENUM_HAL_USB_PORT e_usbPort);


#endif

