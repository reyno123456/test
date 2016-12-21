/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_gpio.h
Description: this module contains the helper fucntions necessary to control the general
             purpose IO block
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/19
History:
         0.0.1    2016/12/19    The initial version of hal_gpio.h
*****************************************************************************/


#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

typedef enum
{
    HAL_GPIO_PIN_RESET = 0, //gpio output 0
    HAL_GPIO_PIN_SET        //gpio output 1

} ENUM_HAL_GPIO_PinState;

typedef enum
{
    HAL_GPIO_ACTIVE_LOW = 0, //falling-edge or active-low sensitive
    HAL_GPIO_ACTIVE_HIGH     //rising-edge or active-high sensitive

} ENUM_HAL_GPIO_InterrputLevel;

typedef enum
{
    HAL_GPIO_LEVEL_SENUMSITIVE = 0,//level-interrupt
    HAL_GPIO_EDGE_SENUMSITIVE      //edge-interrupt

} ENUM_HAL_GPIO_InterrputPolarity;

typedef enum
{
    GPIO_NUM0 = 0,
    GPIO_NUM1,
    GPIO_NUM2,
    GPIO_NUM3,
    GPIO_NUM4,
    GPIO_NUM5,
    GPIO_NUM6,
    GPIO_NUM7,
    GPIO_NUM8,
    GPIO_NUM9,
    GPIO_NUM10,
    GPIO_NUM11,
    GPIO_NUM12,
    GPIO_NUM13,
    GPIO_NUM14,
    GPIO_NUM15,
    GPIO_NUM16,
    GPIO_NUM17,
    GPIO_NUM18,
    GPIO_NUM19,
    GPIO_NUM20,
    GPIO_NUM21,
    GPIO_NUM22,
    GPIO_NUM23,
    GPIO_NUM24,
    GPIO_NUM25,
    GPIO_NUM26,
    GPIO_NUM27,
    GPIO_NUM28,
    GPIO_NUM29,
    GPIO_NUM30,
    GPIO_NUM31,
    GPIO_NUM32,
    GPIO_NUM33,
    GPIO_NUM34,
    GPIO_NUM35,
    GPIO_NUM36,
    GPIO_NUM37,
    GPIO_NUM38,
    GPIO_NUM39,
    GPIO_NUM40,
    GPIO_NUM41,
    GPIO_NUM42,
    GPIO_NUM43,
    GPIO_NUM44,
    GPIO_NUM45,
    GPIO_NUM46,
    GPIO_NUM47,
    GPIO_NUM48,
    GPIO_NUM49,
    GPIO_NUM50,
    GPIO_NUM51,
    GPIO_NUM52,
    GPIO_NUM53,
    GPIO_NUM54,
    GPIO_NUM55,
    GPIO_NUM56,
    GPIO_NUM57,
    GPIO_NUM58,
    GPIO_NUM59,
    GPIO_NUM60,
    GPIO_NUM61,
    GPIO_NUM62,
    GPIO_NUM63,
    GPIO_NUM64,
    GPIO_NUM65,
    GPIO_NUM66,
    GPIO_NUM67,
    GPIO_NUM68,
    GPIO_NUM69,
    GPIO_NUM70,
    GPIO_NUM71,
    GPIO_NUM72,
    GPIO_NUM73,
    GPIO_NUM74,
    GPIO_NUM75,
    GPIO_NUM76,
    GPIO_NUM77,
    GPIO_NUM78,
    GPIO_NUM79,
    GPIO_NUM80,
    GPIO_NUM81,
    GPIO_NUM82,
    GPIO_NUM83,
    GPIO_NUM84,
    GPIO_NUM85,
    GPIO_NUM86,
    GPIO_NUM87,
    GPIO_NUM88,
    GPIO_NUM89,
    GPIO_NUM90,
    GPIO_NUM91,
    GPIO_NUM92,
    GPIO_NUM93,
    GPIO_NUM94,
    GPIO_NUM95,
    GPIO_NUM96,
    GPIO_NUM97,
    GPIO_NUM98,
    GPIO_NUM99,
    GPIO_NUM100,
    GPIO_NUM101,
    GPIO_NUM102,
    GPIO_NUM103,
    GPIO_NUM104,
    GPIO_NUM105,
    GPIO_NUM106,
    GPIO_NUM107,
    GPIO_NUM108,
    GPIO_NUM109,
    GPIO_NUM110,
    GPIO_NUM111,
    GPIO_NUM112,
    GPIO_NUM113,
    GPIO_NUM114,
    GPIO_NUM115,
    GPIO_NUM116,
    GPIO_NUM117,
    GPIO_NUM118,
    GPIO_NUM119,
    GPIO_NUM120,
    GPIO_NUM121,
    GPIO_NUM122,
    GPIO_NUM123,
    GPIO_NUM124,
    GPIO_NUM125,
    GPIO_NUM126,
    GPIO_NUM127
} ENUM_HAL_GPIO_Num;

/**
* @brief    set gpio output mode.
* @param    e_gpioPin: The gpio number, the right number should be 0-127.
* @retval   HAL_OK                means the initializtion output mode is well done.
*           HAL_GPIO_ERR_UNKNOWN  means the gpio number error.
* @note     none
*/
HAL_RET_T HAL_GPIO_OutPut(ENUM_HAL_GPIO_Num e_gpioPin);

/**
* @brief    set gpio input mode
* @param    e_gpioPin: The gpio number, the right number should be 0-127.
* @retval   HAL_OK             means the initializtion input mode is well done.
*           HAL_GPIO_ERR_INIT  means some error happens in the initializtion. 
* @note     none
*/
HAL_RET_T HAL_GPIO_InPut(ENUM_HAL_GPIO_Num e_gpioPin);

/**
* @brief    get gpio value
* @param    e_gpioPin: The gpio number, the right number should be 0-127.
* @retval   gpio state
            0                  means the gpio is low
            1                  means the gpio is high
            HAL_GPIO_ERR_UNKNOWN  means the gpio number error. 
* @note     gpio must be seted input mode otherwise only retrun 0
*/
uint32_t HAL_GPIO_GetPin(ENUM_HAL_GPIO_Num e_gpioPin);

/**
* @brief    set gpio high or low
* @param    e_gpioPin: The gpio number, the right number should be 0-127.
* @param    e_pinState:  GPIO_PIN_RESET set gpio low
                         GPIO_PIN_SET set gpio high
* @retval   HAL_OK                means the set pin state is well done.
*           HAL_GPIO_ERR_UNKNOWN  means the gpio number error. 
* @note     gpio must be seted output mode
*/
HAL_RET_T HAL_GPIO_SetPin(ENUM_HAL_GPIO_Num e_gpioPin, ENUM_HAL_GPIO_PinState e_pinState);

/**
* @brief    set gpio interrupt mode
* @param    e_gpioPin: The gpio number, the right number should be 0-127.
            e_inttype： GPIO_LEVEL_SENUMSITIVE is level-interrupt
                        GPIO_EDGE_SENUMSITIVE is edge-interrupt
            e_polarity: GPIO_ACTIVE_LOW falling-edge or active-low sensitive
                        GPIO_ACTIVE_HIGH rising-edge or active-high sensitive
* @retval   HAL_OK             means the initializtion interrupt mode is well done.
*           HAL_GPIO_ERR_INIT  means some error happens in the initializtion. 
* @note     this function include set gpio input mode and debounce mode and register corresponding interrupt
*/
HAL_RET_T HAL_GPIO_RegisterInterrupt(ENUM_HAL_GPIO_Num e_gpioPin, 
                                     ENUM_HAL_GPIO_InterrputLevel e_inttype, 
                                     ENUM_HAL_GPIO_InterrputPolarity e_polarity,
                                     void *fun_callBack);

/**
* @brief    clear corresponding interrupt bit
* @param    e_gpioPin: The gpio number, the right number should be 0-127.
* @retval   HAL_OK                means that disbale corresponding interrupt.
            HAL_GPIO_ERR_UNKNOWN  means the gpio number error.  
* @note   none
*/
HAL_RET_T HAL_GPIO_DisableNvic(ENUM_HAL_GPIO_Num e_gpioPin);  


#endif
