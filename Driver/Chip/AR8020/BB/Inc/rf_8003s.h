#ifndef __RF8003_H__
#define __RF8003_H__

#include "boardParameters.h"

/**
  * @brief : Write 8003 RF register by SPI 
  * @param : addr: 8003 SPI address
  * @param : data: data for 8003
  * @retval  0: sucess   1: FAIL
  */
int RF8003s_SPI_WriteReg(uint8_t u8_addr, uint8_t u8_data);


/**
  * @brief : Read 8003 RF register by SPI 
  * @param : addr: 8003 SPI address
  * @retval  0: sucess   1: FAIL
  */
int RF8003s_SPI_ReadReg(uint8_t u8_addr, uint8_t *pu8_rxValue);


/**
  * @brief : init RF8003s register
  * @param : addr: 8003 SPI address
  * @retval  None
  */
void RF8003s_init(uint8_t *pu8_regs1, uint8_t *pu8_regs2, STRU_BoardCfg *boardCfg, ENUM_BB_MODE en_mode);

void RF8003s_afterCali(ENUM_BB_MODE en_mode, STRU_BoardCfg *boardCfg);

#endif