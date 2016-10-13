#include <string.h>
#include <stdio.h>

#include "config_functions_sel.h"
#include "config_baseband_register.h"
#include "sys_peripheral_init.h"
#include "sys_peripheral_communication.h"
#include "BB_ctrl.h"

PC_FlagTypeDef PCState;
Sys_FlagTypeDef SysState;

uint8_t RXFIFO[RXMSG_SIZE] = {0};      //Temporary FIFO
uint8_t TXFIFO[TXMSG_SIZE] = {0};

uint8_t iRead_Addr_Num  = 0;
uint8_t iWrite_Addr_Num = 0;
uint8_t Txosd_Buffer[Osdpackage] = {0};
uint8_t Rx_Addr_From_Cy7c68013[RxFIFO68013_R][RxFIFO68013_L]={0};
uint8_t Tx_Addr_To_Cy7c68013[TxFIFO68013_R][TxFIFO68013_L]={0};


void Sys_Parm_Init(void)
{
    SysState.GrdIrq=DISABLE;
    SysState.SkyIrq=DISABLE;
    SysState.SkySpecialIrq=DISABLE;
    SysState.GrdGetSnrA=ENABLE;
    SysState.GrdGetSnrB=DISABLE;
    SysState.GrdGetSnrC=DISABLE;
    SysState.GrdGetSnrD=DISABLE;
    SysState.GrdGetSnrE=DISABLE;
    SysState.GrdGetSnrF=DISABLE;
    SysState.GrdGetSnrG=DISABLE;
    SysState.GrdGetSnrH=DISABLE;
    SysState.TxcyregOnly=DISABLE;
    SysState.TxcyosdOnly=DISABLE;
}

void PC_Parm_Init(void)
{
   PCState.CmdsearchID=DISABLE;
   PCState.CmdEXTI = ENABLE;
}


#if 0
/**
  * @brief Software application for AR8003 RF calibration.
  *
  * @illustrate
  *
    Please be noted that the description in this section is only used for sky end with AR8003 as
    RF transceiver. After power on and reset for AR8001, the calibration of AR8003 will be performed
    for only one time. After the calibration, software must read and record the calibration results,
    and, with some calculation, write these calibration results back to the AR8003 related registers.

    (1)Do the RF calibration and TSSI DC calculation.
       1.1. Do TX calibration and TSSI DC value calculation
            Page1: Write 0xA1=0x0F, 0xA2=0x00, 0xA3=0x01, 0xAC= the value of address 0xCE.
       1.2. Software reset
            Toggle 0x00[0], write ??first and then write ??
       1.3. Wait for 200ms. Then the RF calibration values are available.
       
    (2)Read RF calibration values
    (3)Write the calibration results back to the AR8003 related registers (all in page 1)
       ...Omitted here.
    (4)Use force calibration value mode (all in page 1)
       4.1. Write 0xA1=0x00, 0xA3=0x80, 0xA0=0x06
       4.2. Toggle 0x00[0], write ??first and then write ??
*/

uint8_t Read_Calibration_AR8001(uint8_t ADDRESS1,uint8_t ibit)  //Read AR8001 Reg
{
   uint8_t ibData=0;
   uint8_t ibData_temp=0;
   uint8_t iMASK[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};


   ibData = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0, (ibData |PAGE_1));

   ibData_temp=Spi_Baseband_ReadWrite(spiRead, ADDRESS1, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , ibData);

   if(ibData_temp&iMASK[ibit])
   {
    return 1;
   }
   else
   {
    return 0;
   }

}

void Write_Back_AR8003(uint8_t Set_Reset_write, uint8_t ADDRESS2,uint8_t jbit)
{
   uint8_t jbData=0;
   uint8_t jbData_temp=0;

   uint8_t reMASK[]={0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F};
   uint8_t sMASK[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

   jbData = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0, (jbData |PAGE_1));

   jbData_temp=Spi_Baseband_ReadWrite(spiRead, ADDRESS2, 0x00);

   if(Set_Reset_write)
    {
        Spi_Baseband_ReadWrite(spiWrite, ADDRESS2,( jbData_temp|sMASK[jbit] ));//set _ 1
    }
   else
    {
        Spi_Baseband_ReadWrite(spiWrite, ADDRESS2,( jbData_temp & reMASK[jbit] )); //set _ 0
    }
     Spi_Baseband_ReadWrite(spiWrite, FSM_0 , jbData);
}


void Calculate_Txa_Sin_Ofs(void)
{
  uint8_t cData=0;
  uint8_t i_A7=0;

  uint16_t txa_sin_ofs_Data= 0;
  uint16_t txa_sin_ofs_low = 0;
  uint16_t txa_sin_ofs_hgh = 0;

  cData = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
  Spi_Baseband_ReadWrite(spiWrite, FSM_0, (cData |PAGE_1));
  //1.1 - Do TX calibration and TSSI DCvalue calculation
  txa_sin_ofs_low = Spi_Baseband_ReadWrite(spiRead, CALI_2, 0x00);
  txa_sin_ofs_hgh = Spi_Baseband_ReadWrite(spiRead, CALI_3, 0x00);

  Spi_Baseband_ReadWrite(spiWrite, FSM_0, cData);
  txa_sin_ofs_Data=((txa_sin_ofs_hgh&0x0F)<<8|txa_sin_ofs_low);

  if(txa_sin_ofs_Data<=0x47)
  {
    for(i_A7=0;i_A7<4;i_A7++)
    {
     Write_Back_AR8003( Set_Write ,CA_7,i_A7);
    }
  }
  else if(txa_sin_ofs_Data<=0x60)
  {
    Write_Back_AR8003( Set_Write ,CA_7,3);  //0xE
    Write_Back_AR8003( Set_Write ,CA_7,2);
    Write_Back_AR8003( Set_Write ,CA_7,1);
    Write_Back_AR8003( Reset_Write ,CA_7,0);
  }
  else if(txa_sin_ofs_Data < 0x73)
  {
    Write_Back_AR8003( Set_Write ,CA_7,3);  //0xD
    Write_Back_AR8003( Set_Write ,CA_7,2);
    Write_Back_AR8003(Reset_Write,CA_7,1);
    Write_Back_AR8003( Set_Write ,CA_7,0);
  }
  else if(txa_sin_ofs_Data<0x86)
  {
    Write_Back_AR8003( Set_Write ,CA_7,3);  //0xC
    Write_Back_AR8003( Set_Write ,CA_7,2);
    Write_Back_AR8003(Reset_Write,CA_7,1);
    Write_Back_AR8003(Reset_Write,CA_7,0);
  }
  else if(txa_sin_ofs_Data<0x92)
  {
    Write_Back_AR8003( Set_Write ,CA_7,3);  //0xB
    Write_Back_AR8003(Reset_Write,CA_7,2);
    Write_Back_AR8003( Set_Write ,CA_7,1);
    Write_Back_AR8003( Set_Write ,CA_7,0);
  }
  else if(txa_sin_ofs_Data<0x9F)
  {
    Write_Back_AR8003( Set_Write ,CA_7,3);  //0xA
    Write_Back_AR8003(Reset_Write,CA_7,2);
    Write_Back_AR8003( Set_Write ,CA_7,1);
    Write_Back_AR8003(Reset_Write,CA_7,0);
  }
  else if(txa_sin_ofs_Data<0xAB)
  {
    Write_Back_AR8003( Set_Write ,CA_7,3);  //0x9
    Write_Back_AR8003(Reset_Write,CA_7,2);
    Write_Back_AR8003(Reset_Write,CA_7,1);
    Write_Back_AR8003( Set_Write ,CA_7,0);
  }
  else if(txa_sin_ofs_Data<0xB8)
  {
    Write_Back_AR8003( Set_Write ,CA_7,3);  //0x8
    Write_Back_AR8003(Reset_Write,CA_7,2);
    Write_Back_AR8003(Reset_Write,CA_7,1);
    Write_Back_AR8003(Reset_Write,CA_7,0);
  }
  else if(txa_sin_ofs_Data<0xBE)
  {
    Write_Back_AR8003(Reset_Write,CA_7,3);  //0x7
    Write_Back_AR8003( Set_Write ,CA_7,2);
    Write_Back_AR8003( Set_Write ,CA_7,1);
    Write_Back_AR8003( Set_Write ,CA_7,0);
  }
  else
  {
    Write_Back_AR8003(Reset_Write,CA_7,3);  //0x6
    Write_Back_AR8003( Set_Write ,CA_7,2);
    Write_Back_AR8003( Set_Write ,CA_7,1);
    Write_Back_AR8003(Reset_Write,CA_7,0);
  }
}

void Calculate_Txb_Sin_Ofs(void)
{
  uint8_t dData=0;
  uint8_t i_A7=0;

  uint16_t txb_sin_ofs_Data=0;
  uint16_t txb_sin_ofs_low=0;
  uint16_t txb_sin_ofs_hgh=0;
  dData = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
  Spi_Baseband_ReadWrite(spiWrite, FSM_0, (dData |PAGE_1));
  //1.1 - Do TX calibration and TSSI DCvalue calculation
  txb_sin_ofs_low = Spi_Baseband_ReadWrite(spiRead, CALI_7, 0x00);
  txb_sin_ofs_hgh = Spi_Baseband_ReadWrite(spiRead, CALI_8, 0x00);

  Spi_Baseband_ReadWrite(spiWrite, FSM_0, dData);
  txb_sin_ofs_Data=((txb_sin_ofs_hgh&0x0F)<<8|txb_sin_ofs_low);

  if(txb_sin_ofs_Data<=0x47)
  {
    for(i_A7=0;i_A7<4;i_A7++)
    {
      Write_Back_AR8003( Set_Write ,CA_A,i_A7);
    }
  }
  else if(txb_sin_ofs_Data<=0x60)
  {
    Write_Back_AR8003( Set_Write ,CA_A,3);  //0xE
    Write_Back_AR8003( Set_Write ,CA_A,2);
    Write_Back_AR8003( Set_Write ,CA_A,1);
    Write_Back_AR8003(Reset_Write,CA_A,0);
  }
  else if(txb_sin_ofs_Data < 0x73)
  {
    Write_Back_AR8003( Set_Write ,CA_A,3);  //0xD
    Write_Back_AR8003( Set_Write ,CA_A,2);
    Write_Back_AR8003(Reset_Write,CA_A,1);
    Write_Back_AR8003( Set_Write ,CA_A,0);
  }
  else if(txb_sin_ofs_Data<0x86)
  {
    Write_Back_AR8003( Set_Write ,CA_A,3);  //0xC
    Write_Back_AR8003( Set_Write ,CA_A,2);
    Write_Back_AR8003(Reset_Write,CA_A,1);
    Write_Back_AR8003(Reset_Write,CA_A,0);
  }
  else if(txb_sin_ofs_Data<0x92)
  {
    Write_Back_AR8003( Set_Write ,CA_A,3);  //0xB
    Write_Back_AR8003(Reset_Write,CA_A,2);
    Write_Back_AR8003( Set_Write ,CA_A,1);
    Write_Back_AR8003( Set_Write ,CA_A,0);
  }
  else if(txb_sin_ofs_Data<0x9F)
  {
    Write_Back_AR8003( Set_Write ,CA_A,3);  //0xA
    Write_Back_AR8003(Reset_Write,CA_A,2);
    Write_Back_AR8003( Set_Write ,CA_A,1);
    Write_Back_AR8003(Reset_Write,CA_A,0);
  }
  else if(txb_sin_ofs_Data<0xAB)
  {
    Write_Back_AR8003( Set_Write ,CA_A,3);  //0x9
    Write_Back_AR8003(Reset_Write,CA_A,2);
    Write_Back_AR8003(Reset_Write,CA_A,1);
    Write_Back_AR8003( Set_Write ,CA_A,0);
  }
  else if(txb_sin_ofs_Data<0xB8)
  {
    Write_Back_AR8003( Set_Write ,CA_A,3);  //0x8
    Write_Back_AR8003(Reset_Write,CA_A,2);
    Write_Back_AR8003(Reset_Write,CA_A,1);
    Write_Back_AR8003(Reset_Write,CA_A,0);
  }
  else if(txb_sin_ofs_Data<0xBE)
  {
    Write_Back_AR8003(Reset_Write,CA_A,3);  //0x7
    Write_Back_AR8003( Set_Write ,CA_A,2);
    Write_Back_AR8003( Set_Write ,CA_A,1);
    Write_Back_AR8003( Set_Write ,CA_A,0);
  }

  else
  {
    Write_Back_AR8003(Reset_Write,CA_A,3);  //0x6
    Write_Back_AR8003( Set_Write ,CA_A,2);
    Write_Back_AR8003( Set_Write ,CA_A,1);
    Write_Back_AR8003(Reset_Write,CA_A,0);
  }
}

void Baseband_Calibration_Load(void)
{
  uint8_t i_92=0;
//  uint8_t i_97=0;
//  uint8_t i_9A=0;
  uint8_t iaData = 0;
  uint8_t iData_temp = 0;
  uint8_t ibData = 0;
  uint8_t icData = 0;
//uint8_t idData = 0;
  iaData = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
  Spi_Baseband_ReadWrite(spiWrite, FSM_0, (iaData |PAGE_1));

  Spi_Baseband_ReadWrite(spiWrite, CA_1, 0x0F);    //1.1 - Do TX calibration and TSSI DC value calculation
  Spi_Baseband_ReadWrite(spiWrite, CA_2, 0x00);
  Spi_Baseband_ReadWrite(spiWrite, CA_3, 0x01);
  iData_temp=Spi_Baseband_ReadWrite(spiRead, AGC4_e, 0x00);
  Spi_Baseband_ReadWrite(spiWrite, CA_C, iData_temp);

  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iaData);

  Baseband_Soft_Reset();    //1.2 -  Software reset
  HAL_Delay(200);         //1.3 -  Wait for 200ms. Then the RF calibration values are available.

  // 2
  //Read RF calibration values
  //

  // 3.1
  // Write data in address 0x90[7] to address 0xA5[7].
  // Write data in address 0x90[5:2] to address 0xA6[3:0].

  if(Read_Calibration_AR8001(CALI_0, 7))
   {Write_Back_AR8003( Set_Write ,CA_5, 7);}
  else {Write_Back_AR8003(Reset_Write,CA_5, 7);}

  if(Read_Calibration_AR8001(CALI_0, 5)){Write_Back_AR8003( Set_Write ,CA_6, 3);}
  else {Write_Back_AR8003(Reset_Write,CA_6, 3);}

  if(Read_Calibration_AR8001(CALI_0, 4)){Write_Back_AR8003( Set_Write ,CA_6, 2);}
  else {Write_Back_AR8003(Reset_Write,CA_6, 2);}

  if(Read_Calibration_AR8001(CALI_0, 3)){Write_Back_AR8003( Set_Write ,CA_6, 1);}
  else {Write_Back_AR8003(Reset_Write,CA_6, 1);}

  if(Read_Calibration_AR8001(CALI_0, 2)){Write_Back_AR8003( Set_Write ,CA_6, 0);}
  else {Write_Back_AR8003(Reset_Write,CA_6, 0);}

  // 3.2
  // Write data in address 0x91[7] to address 0xA5[6].
  // Write data in address 0x91[5:2] to address 0xA6[7:4].

  if(Read_Calibration_AR8001(CALI_1, 7)){Write_Back_AR8003( Set_Write ,CA_5, 6);}
  else {Write_Back_AR8003(Reset_Write,CA_5, 6);}

  if(Read_Calibration_AR8001(CALI_1, 5)){Write_Back_AR8003( Set_Write ,CA_6, 7);}
  else {Write_Back_AR8003(Reset_Write,CA_6, 7);}

  if(Read_Calibration_AR8001(CALI_1, 4)){Write_Back_AR8003( Set_Write ,CA_6, 6);}
  else {Write_Back_AR8003(Reset_Write,CA_6, 6);}

  if(Read_Calibration_AR8001(CALI_1, 3)){Write_Back_AR8003( Set_Write ,CA_6, 5);}
  else {Write_Back_AR8003(Reset_Write,CA_6, 5);}

  if(Read_Calibration_AR8001(CALI_1, 2)){Write_Back_AR8003( Set_Write ,CA_6, 4);}
  else {Write_Back_AR8003(Reset_Write,CA_6, 4);}

  // 3.3
  // Write data in address 0x94[7] to address 0xA5[5].
  // Write data in address 0x94[5:2] to address 0xA7[7:4].

  if(Read_Calibration_AR8001(CALI_4, 7)){Write_Back_AR8003( Set_Write ,CA_5, 5);}
  else {Write_Back_AR8003(Reset_Write,CA_5, 5);}

  if(Read_Calibration_AR8001(CALI_4, 5)){Write_Back_AR8003( Set_Write ,CA_7, 7);}
  else {Write_Back_AR8003(Reset_Write,CA_7, 7);}

  if(Read_Calibration_AR8001(CALI_4, 4)){Write_Back_AR8003( Set_Write ,CA_7, 6);}
  else {Write_Back_AR8003(Reset_Write,CA_7, 6);}

  if(Read_Calibration_AR8001(CALI_4, 3)){Write_Back_AR8003( Set_Write ,CA_7, 5);}
  else {Write_Back_AR8003(Reset_Write,CA_7, 5);}

  if(Read_Calibration_AR8001(CALI_4, 2)){Write_Back_AR8003( Set_Write ,CA_7, 4);}
  else {Write_Back_AR8003(Reset_Write,CA_7, 4);}

  // 3.4
  // Write data in address 0x93[3] to address 0xA5[4].
  // Write data in address 0x92[7:0] toaddress 0xA8[7:0].

  if(Read_Calibration_AR8001(CALI_3, 3)){Write_Back_AR8003( Set_Write ,CA_5, 4);}
  else {Write_Back_AR8003(Reset_Write,CA_5, 4);}
  for(i_92=0;i_92<8;i_92++)
  {
   if(Read_Calibration_AR8001(CALI_2, i_92)){Write_Back_AR8003( Set_Write ,CA_8, i_92);}
   else {Write_Back_AR8003(Reset_Write,CA_8, i_92);}
  }

  // 3.5
  // Calculate the absolute value of txa_sin_ofs
  // (the absolute data value in address{0x93[3:0],0x92[7:0]}).

  Calculate_Txa_Sin_Ofs();
/*
  // 3.6
  // Write data in address 0x95[7] to address 0xA5[3].
  // Write data in address 0x95[5:2] to address 0xA9[3:0].

  if(Read_Calibration_AR8001(CALI_5, 7)){Write_Back_AR8003( Set_Write ,CA_5, 3);}
  else {Write_Back_AR8003(Reset_Write,CA_5, 3);}

  if(Read_Calibration_AR8001(CALI_5, 5)){Write_Back_AR8003( Set_Write ,CA_9, 3);}
  else {Write_Back_AR8003(Reset_Write,CA_9, 3);}

  if(Read_Calibration_AR8001(CALI_5, 4)){Write_Back_AR8003( Set_Write ,CA_9, 2);}
  else {Write_Back_AR8003(Reset_Write,CA_9, 2);}

  if(Read_Calibration_AR8001(CALI_5, 3)){Write_Back_AR8003( Set_Write ,CA_9, 1);}
  else {Write_Back_AR8003(Reset_Write,CA_9, 1);}

  if(Read_Calibration_AR8001(CALI_5, 2)){Write_Back_AR8003( Set_Write ,CA_9, 0);}
  else {Write_Back_AR8003(Reset_Write,CA_9, 0);}

  // 3.7
  // Write data in address 0x96[7] to address 0xA5[2].
  // Write data in address 0x96[5:2] to address 0xA9[7:4].

  if(Read_Calibration_AR8001(CALI_6, 7)){Write_Back_AR8003( Set_Write ,CA_5, 2);}
  else {Write_Back_AR8003(Reset_Write,CA_5, 2);}

  if(Read_Calibration_AR8001(CALI_6, 5)){Write_Back_AR8003( Set_Write ,CA_9, 7);}
  else {Write_Back_AR8003(Reset_Write,CA_9, 7);}

  if(Read_Calibration_AR8001(CALI_6, 4)){Write_Back_AR8003( Set_Write ,CA_9, 6);}
  else {Write_Back_AR8003(Reset_Write,CA_9, 6);}

  if(Read_Calibration_AR8001(CALI_6, 3)){Write_Back_AR8003( Set_Write ,CA_9, 5);}
  else {Write_Back_AR8003(Reset_Write,CA_9, 5);}

  if(Read_Calibration_AR8001(CALI_6, 2)){Write_Back_AR8003( Set_Write ,CA_9, 4);}
  else {Write_Back_AR8003(Reset_Write,CA_9, 4);}

  // 3.8
  // Write data in address 0x99[7] to address 0xA5[1].
  // Write data in address 0x99[5:2] toaddress 0xAA[7:4].

  if(Read_Calibration_AR8001(CALI_9, 7)){Write_Back_AR8003( Set_Write ,CA_5, 1);}
  else {Write_Back_AR8003(Reset_Write,CA_5, 1);}

  if(Read_Calibration_AR8001(CALI_9, 5)){Write_Back_AR8003( Set_Write ,CA_A, 7);}
  else {Write_Back_AR8003(Reset_Write,CA_A, 7);}

  if(Read_Calibration_AR8001(CALI_9, 4)){Write_Back_AR8003( Set_Write ,CA_A, 6);}
  else {Write_Back_AR8003(Reset_Write,CA_A, 6);}

  if(Read_Calibration_AR8001(CALI_9, 3)){Write_Back_AR8003( Set_Write ,CA_A, 5);}
  else {Write_Back_AR8003(Reset_Write,CA_A, 5);}

  if(Read_Calibration_AR8001(CALI_9, 2)){Write_Back_AR8003( Set_Write ,CA_A, 4);}
  else {Write_Back_AR8003(Reset_Write,CA_A, 4);}

  // 3.9
  // Write data in address 0x98[3] to address 0xA5[0].
  // Write data in address 0x97[7:0] to address 0xAB[7:0].

  if(Read_Calibration_AR8001(CALI_8, 3)){Write_Back_AR8003( Set_Write ,CA_5, 0);}
  else {Write_Back_AR8003(Reset_Write,CA_5, 0);}
  for(i_97=0;i_97<8;i_97++)
  {
   if(Read_Calibration_AR8001(CALI_7, i_97)){Write_Back_AR8003( Set_Write ,CA_B, i_97);}
   else {Write_Back_AR8003(Reset_Write,CA_B, i_97);}
  }

  // 3.10
  // Calculate the absolute value of txb_sin_ofs
  //  (the absolute data value in address{0x98[3:0],0x97[7:0]}).

  Calculate_Txb_Sin_Ofs();

  // 3.11
  // Write data in address 0x9A[7:0] to address 0xAE[7:0].

  for(i_9A=0;i_9A<8;i_9A++)
  {
   if(Read_Calibration_AR8001(CALI_A, i_97)){Write_Back_AR8003( Set_Write ,CA_E, i_97);}
   else {Write_Back_AR8003(Reset_Write,CA_E, i_97);}
  }

*/
  ibData = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
  Spi_Baseband_ReadWrite(spiWrite, FSM_0, (ibData |PAGE_1));

  Spi_Baseband_ReadWrite(spiWrite, CA_1, 0x00);    //4.1 - Use force calibration value mode (all in page 1)
  Spi_Baseband_ReadWrite(spiWrite, CA_3, 0x80);
  Spi_Baseband_ReadWrite(spiWrite, CA_0, 0x06);
  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , ibData);
  Baseband_Soft_Reset();   //4.2  Toggle 0x00[0], write ??first and then write ??

  icData = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
  Spi_Baseband_ReadWrite(spiWrite, FSM_0, (icData |PAGE_1));

  Spi_Baseband_ReadWrite(spiWrite, CA_1, 0x00);    //close calibration
  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iaData);

  // HAL_Delay(10);
  // Spi_Baseband_ReadWrite(spiWrite, 0x00, bData);
}

void Rf_Calibration_Load(void)
{
  Rf_Config(Config_RF_RC_Tune_Data, sizeof(Config_RF_RC_Tune_Data)/4 );
}
void Rf_PA_NO_CURRENT_During_TX_Calibration(void)
{
  Rf_Config(Config_RF_NO_Current_Data, sizeof(Config_RF_NO_Current_Data)/4 );
}

#endif
