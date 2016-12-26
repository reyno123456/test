/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_spi.c
Description: The external HAL APIs to use the SPI controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal_spi.c
*****************************************************************************/

#include "hal_spi.h"

/**
* @brief  The SPI initialization function which must be called 
*         before using the SPI controller.
* @param  e_spiComponent    The SPI controller number, the right number 
*                           should be 0-7 and totally 8 SPI controllers 
*                           can be used by application.
*         pst_spiInitInfo   spi init info,the member's value should use 
*                           the enum value.
* @retval HAL_OK            means the initializtion is well done.
*         HAL_SPI_ERR_INIT  means some error happens in the initializtion.
* @note   None.
*       
*/
HAL_RET_T HAL_SPI_MasterInit(ENUM_HAL_SPI_COMPONENT e_spiComponent, 
                             STRU_HAL_SPI_INIT *pst_spiInitInfo)
{
    uint16_t u16_spiDivision;
    // init default value.
    STRU_SPI_InitTypes st_spiInit = 
    {
        .ctrl0   = 0x47,
        .clk_Mhz = 0x09,
        .Tx_Fthr = 0x03,
        .Rx_Ftlr = 0x6,
        .SER     = 0x01
    };
   
    if (e_spiComponent > HAL_SPI_COMPONENT_7)
    {
        return HAL_SPI_ERR_INIT;
    }

    if (NULL == pst_spiInitInfo)
    {
        return HAL_SPI_ERR_INIT;
    }
    // BAUDR
    st_spiInit.clk_Mhz = pst_spiInitInfo->u16_halSpiBaudr;
    
    // SCPOL 
    st_spiInit.ctrl0 &= 0x80;
    st_spiInit.ctrl0 |= (((uint16_t)(pst_spiInitInfo->e_halSpiPolarity)) << 7) & 0x80;

    // SCPH
    st_spiInit.ctrl0 &= 0x40;
    st_spiInit.ctrl0 |= (((uint16_t)(pst_spiInitInfo->e_halSpiPhase)) << 6) & 0x40;
    
    SPI_master_init((ENUM_SPI_COMPONENT)(e_spiComponent), &st_spiInit);

    return HAL_OK;
}

/**
* @brief  The SPI data write function which can be used to send out SPI data 
*         by the SPI controller.
* @param  e_spiComponent          The SPI controller number, the right number 
*                                 should be 0-7 and totally 8 SPI controllers 
*                                 can be used by application.
*         pu8_wrData              The transmit buffer pointer to be sent out 
*                                 by SPI bus. 
*         u16_wrSize              The transmit buffer size in byte. 
* @retval HAL_OK                  means the SPI data write is well done.
*         HAL_SPI_ERR_WRITE_DATA  means some error happens in the SPI data write.
* @note   the SPI controller must work in master mode.
*/
HAL_RET_T HAL_SPI_MasterWriteData(ENUM_HAL_SPI_COMPONENT e_spiComponent, 
                                  uint8_t *pu8_wrData,
                                  uint16_t u16_wrSize)
{
    if (e_spiComponent > HAL_SPI_COMPONENT_7)
    {
        return HAL_UART_ERR_WRITE_DATA;
    }
    if (NULL == pu8_wrData)
    {
        return HAL_UART_ERR_WRITE_DATA;
    }
    if (0 == u16_wrSize)
    {
        return HAL_UART_ERR_WRITE_DATA;
    }
    SPI_write_read((ENUM_SPI_COMPONENT)(e_spiComponent),
		    pu8_wrData, u16_wrSize, 0, 0);
    return HAL_OK;
}

/**
* @brief  The SPI data read function which can be used to read SPI data by the 
*         SPI controller.
* @param  e_spiComponent          The SPI controller number, the right number 
*                                 should be 0-7 and totally 8 SPI controllers 
*                                 can be used by application.
*         pu8_rdData              The receive buffer pointer to hold the data 
*                                 in read operation.
*         u16_rdSize              The receive buffer size in byte.
* @retval HAL_OK                  means the initializtion is well done.
*         HAL_SPI_ERR_READ_DATA   means some error happens in the data read.
* @note   the SPI controller must work in master mode.
*/
HAL_RET_T HAL_SPI_MasterReadData(ENUM_HAL_SPI_COMPONENT e_spiComponent, 
                                 uint8_t *pu8_rdData,
                                 uint16_t u16_rdSize)
{
        
    if (e_spiComponent > HAL_SPI_COMPONENT_7)
    {
        return HAL_UART_ERR_READ_DATA;
    }
    if (NULL == pu8_rdData)
    {
        return HAL_UART_ERR_READ_DATA;
    }
    if (0 == u16_rdSize)
    {
        return HAL_UART_ERR_READ_DATA;
    }
    SPI_write_read((ENUM_SPI_COMPONENT)(e_spiComponent),
		    0, 0, pu8_rdData, u16_rdSize);
    return HAL_OK;
}
