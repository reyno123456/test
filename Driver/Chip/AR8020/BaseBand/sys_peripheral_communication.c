#include <string.h>
#include "config_baseband_register.h"
#include "sys_peripheral_init.h"
#include "sys_peripheral_communication.h"


#if defined(GRD_RF8003_2P3) || defined(GRD_RF8003_2P4)

   #include"config_baseband_2p4_8003_grd.h"
   #include"config_rf_2p4_8003_grd.h"

#endif

#if defined(SKY_RF8003_2P3) || defined(SKY_RF8003_2P4)

   #include"config_baseband_2p4_8003_sky.h"
   #include"config_rf_2p4_8003_sky.h"

#endif

#if defined(GRD_RF9363_2P4) || defined(GRD_RF9363_2P4) || defined(GRD_RF9363_2P5)

   #include"config_baseband_2p4_9363_grd.h"
   #include"config_rf_2p4_9363_grd.h"

#endif

#if defined(SKY_RF9363_2P3) || defined(SKY_RF9363_2P4) || defined(SKY_RF9363_2P5)

   #include"config_baseband_2p4_9363_sky.h"
   #include"config_rf_2p4_9363_sky.h"

#endif

#if defined(GRD_RF9361_3P5) || defined(GRD_RF9361_3P6)

   #include"config_baseband_3p5_9361_grd.h"
   #include"config_rf_3p5_9361_grd.h"

#endif

#if defined(SKY_RF9361_3P5) || defined(SKY_RF9361_3P6)

   #include"config_baseband_3p5_9361_sky.h"
   #include"config_rf_3p5_9361_sky.h"

#endif

PC_FlagTypeDef PCState;
Sys_FlagTypeDef SysState;

extern UART_HandleTypeDef Uart1Handle;
extern UART_HandleTypeDef Uart3Handle;

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
/**
  ==============================================================================
              #####  Interface initial between MCU and Baseband #####
  ==============================================================================
      * @brief  SPI slave for MCU to configure AR8001.
      *
      * @illustrate
      *
         The SPI bus provides the mechanism for all digital control of the AR8001.Each
         SPI register is 8-bit wide, and each register contains control bits,status
         monitors, or other settings thatcontrol all functions of the device. There
         are four SPI bus signals which are described in the following sections.

        (+) FPGA_CS
           FPGA_CS is the bus enable signal driven from the MCU to the AR8001. FPGA_CS is driven low
           before the first FPGA_SCK rising edge and id normally driven high after the last
           FPGA_SCK falling edge. The AR8001 ignores the clock and data signals while FPGA_CS
           is high.
        (+)FPGA_SCK
           FPGA_SCK is the interface reference clock driven from the MCU to the AR8001. It is only
           active while FPGA_CS is low. The maximum FPGA_SCK frequency is 8MHz.
        (+)FPGA_MOSI and FPGA_MISO
           The SPI utilizes two data signals – FPGA_MOSI and FPGA_MISO. FPGA_MOSI is the data
           inputline driven from the MCU to the AR8001 and FPGA_MISO is the data output from the
           AR8001 to the MCU in the configuration.

      * @{
      *
      * @}
        @protocol
        The AR8001 SPI supports a 24-bit transfer format. The 24-bit data field contains the
        following information.
            |--------------------------------------------------------|
            |     D23:D16    |       D15:D8         |    D7:D0       |
            |----------------|----------------------|----------------|
            |      WR/RD     |      ADDR<7:0>       |    DATA<7:0>   |
            |--------------------------------------------------------|
         (+)D23:D16 – Bits<23:16> of the data determines whether a read or write transfer occurs
                       after the data byte write. 0x0E indicates a write operation; 0x0F indicates
                       a read operation.
         (+)D15:D8  – Bits<15:8> specify the starting byte address for the data transfer.
         (+)D7:D0   – Bits<7:0> specify the data to be written or read.
      */

void Spi_Cs_Reset(void)   { SPI_NSS_GPIO_PORT->BSRR = (uint32_t)SPI_NSS << 16;}
void Spi_Cs_Set(void)     { SPI_NSS_GPIO_PORT->BSRR = SPI_NSS;}
void Spi_Sclk_Reset(void) { SPI_SCK_GPIO_PORT->BSRR = (uint32_t)SPI_SCK << 16;}
void Spi_Sclk_Set(void)   { SPI_SCK_GPIO_PORT->BSRR = SPI_SCK;}
void Spi_Mosi_Reset(void) { SPI_MOSI_GPIO_PORT->BSRR = (uint32_t)SPI_MOSI << 16;}
void Spi_Mosi_Set(void)   { SPI_MOSI_GPIO_PORT->BSRR = SPI_MOSI;}
uint8_t Spi_Baseband_ReadWrite( uint8_t spiRW, uint8_t spiAdrress, uint8_t spiData)
{
    uint8_t i;
    uint8_t spiRead_temp;
    uint8_t MASK[] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

    Spi_Sclk_Reset();
    Spi_Cs_Reset();
    for(i=0; i<8; i++)           //Read or Writer
    {
        if(spiRW & MASK[i]) Spi_Mosi_Set();
        else Spi_Mosi_Reset();
        Spi_Sclk_Set();
        Spi_Sclk_Reset();
    }

    for(i=0; i<8; i++)           //Address
    {
       if(spiAdrress & MASK[i]) Spi_Mosi_Set();
       else Spi_Mosi_Reset();
       Spi_Sclk_Set();
       Spi_Sclk_Reset();
    }

    for(i=0; i<8; i++)           //Data
    {
        if(spiData & MASK[i])  Spi_Mosi_Set();
        else  Spi_Mosi_Reset();
        Spi_Sclk_Set();
        Spi_Sclk_Reset();
        spiRead_temp = spiRead_temp << 1 ;
        if((SPI_MISO_GPIO_PORT->IDR & SPI_MISO) !=  (uint32_t)GPIO_PIN_RESET)
        {
            spiRead_temp = spiRead_temp | 0x01;
        }

    }
    Spi_Cs_Set();
    if(spiRW == spiRead)
     {
        return spiRead_temp;
     }
    else
     {
        return 0 ;
     }

}

void Baseband_Config(const uint32_t * pConfig , uint32_t iLength)
{
    uint8_t  *tData;
    uint8_t  spiAddr;
    uint8_t  spiData;
    uint32_t i;

    tData = (uint8_t * )pConfig;
    for(i = 0; i<iLength; i++)
    {
        spiData = *tData;       //
        tData++;
        spiAddr = *tData;
        tData++;
        //spiOrder = *tData;
        tData++;
        //spiClass = *tData;
        tData++;
        Spi_Baseband_ReadWrite(spiWrite, spiAddr, spiData);
    }
}

void Baseband_Load()
{
     Baseband_Config(Config_Baseband_Data, sizeof(Config_Baseband_Data)/4 );
}

// Baseband software reset in Reg[00] bit[7].
void Baseband_Soft_Reset(void)
{
    uint8_t bData=0;
    bData = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
    Spi_Baseband_ReadWrite(spiWrite, 0x00, bData | 0x01);
    Spi_Baseband_ReadWrite(spiWrite, 0x00, bData & 0xFE);
}
 /**
      * @brief SPI master for AR8001 to configure AR8003/AD9363.
      *
      * @illustrate
      *
        The SPI bus provides the mechanism for all digital control of the AD9363/AR8003.
        Each SPI register is 8-bit wide, and each register contains control bits, status
        monitors, or other settings that control all functions of the device.There are four
        SPI bus signals which are described in the following sections.
        (+)SPI_ENB
           SPI_ENB is the bus enable signal driven from the AR8001 to the AD9363/AR8003.
           SPI_ENB is driven low before the first SPI_CLK rising edge and id normally driven
           high after the last SPI_CLK falling edge. The AD9363/AR8003 ignores the clock and
           data signals while SCS is high.
        (+)SPI_CLK
           SPI_CLK is the interface reference clock driven by the AR8001 to the AD9363/AR8003.
           It is only active while SPI_ENB is low. The maximum SPI_CLK frequency is 33MHz.
        (+)SPI_DI and SPI_DO
           The SPI utilizes two data signals – SPI_DI and SPI_DO. SPI_DI is the data input line
           driven from the AD9363/AR8003 to the AR8001 and SPI_DO is the data output from the
           AR8001 to the AD9363/AR8003 in the configuration.

      * @{
      *
      * @}
        @protocol
        The AR8001 SPI supports a 24-bit transfer format. The 24-bit data field contains the
        following information.
            |----------------------------------------------------------|
            |   MSB   |    D22:D15   |    D14:D9   |  D8  |    D7:D0   |
            |---------|--------------|-------- ----|------|------------|
            |  WR/RD  |   00000000   |   ADDR<5:0> |   0  |  DATA<7:0> |
            |----------------------------------------------------------|

         (+)WR/RD – Bit 23 of the data determines whether a read or write transfer occurs after
                     the data byte write. Logic high indicates a write operation; logic low indicates
                     a read operation.
         (+)D22:D15 – Bits<22:15> of the data are unused.
         (+)D14:D9  – Bits<14:9> specify the starting byte address for the data transfer.
         (+)D8      – Bits<8> of the data are unused.
         (+)D7:D0   – Bits<7:0> specify the data to be written or read.
      */

uint8_t Spi_Rf_ReadWrite( uint8_t spiRW, uint8_t spiAdrress, uint8_t spiData)
{
    uint8_t i;
    uint8_t spiRead_temp;
    uint8_t MASK[] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

    Spi_Sclk_Reset();
    Spi_Cs_Reset();

    for(i=0; i<8; i++)                      //comand
    {
       if(spiRW & MASK[i]) Spi_Mosi_Set();
       else Spi_Mosi_Reset();
       Spi_Sclk_Set();
       Spi_Sclk_Reset();
    }

    for(i=0; i<8; i++)                      //address
    {
       if(spiAdrress & MASK[i])  Spi_Mosi_Set();
       else Spi_Mosi_Reset();
       Spi_Sclk_Set();
       Spi_Sclk_Reset();
    }
    for(i=0 ; i<8 ; i++)
    {
       if(spiData & MASK[i]) Spi_Mosi_Set();
       else Spi_Mosi_Reset();
       Spi_Sclk_Set();
       Spi_Sclk_Reset();

       spiRead_temp = spiRead_temp << 1 ;   //read data

       if((SPI_MISO_GPIO_PORT->IDR & SPI_MISO) !=  (uint32_t)GPIO_PIN_RESET)
       {
         spiRead_temp =spiRead_temp | 0x01;
       }

    }
    Spi_Cs_Set();
    if( 0x00 == (spiRW & 0x80))
     {
        return spiRead_temp;
     }
    else
     {
        return 0 ;
     }

}

uint8_t Rf_Handler(uint8_t iClass, uint8_t iOrder, uint8_t iAddr, uint8_t iData)
{
    uint8_t bData = 0;
    uint32_t bTime = 0;

    if(iClass == Write_AR8001)
    {
        Spi_Baseband_ReadWrite(iOrder,iAddr,iData);
    }
    else if(iClass == Write_AR8003)
    {
        Spi_Rf_ReadWrite(iOrder, iAddr, iData);
    }
    else if(iClass == Read_Ad9363)
    {
        bData = Spi_Rf_ReadWrite(iOrder,iAddr,iData);
        if(iData == 0xFF)
        {
          HAL_Delay(1);
        }
        else
        {
            switch(iData)
            {
                case 0x00 :  if (bData & 0x01) { HAL_Delay(1); break; }
                                else  { HAL_Delay(1); return 1; }
                case 0x01 :  if (bData & 0x02) { HAL_Delay(1); break; }
                                else  { HAL_Delay(1); return 1; }
                case 0x02 :  if (bData & 0x04) { HAL_Delay(1);break; }
                                else  {HAL_Delay(1); return 1; }
                case 0x03 :  if (bData & 0x08) { HAL_Delay(1);break; }
                                else  {HAL_Delay(1);return 1; }
                case 0x04 :  if (bData & 0x10) { HAL_Delay(1);break; }
                                else  { HAL_Delay(1);return 1; }
                case 0x05 :  if (bData & 0x20) { HAL_Delay(1); break; }
                                else  { HAL_Delay(1);return 1; }
                case 0x06 :  if (bData & 0x40) {HAL_Delay(1);break; }
                                else  {HAL_Delay(1);return 1; }
                case 0x07 :  if (bData & 0x80) {HAL_Delay(1); break; }
                                else  { HAL_Delay(1);return 1; }

                case 0x08 :
                           {
                              while (0x00 == (Spi_Rf_ReadWrite(iOrder,iAddr,iData)&0x80))
                              {
                                 break;
                              }

                           }break;
                case 0x09 :
                           {
                              while (0x00 == (Spi_Rf_ReadWrite(iOrder,iAddr,iData)&0x40))
                              {
                                 break;
                              }

                           }break;
                 case 0x10 :
                           {
                              while (0x00 == (Spi_Rf_ReadWrite(iOrder,iAddr,iData)&0x01))
                              {
                                 break;
                              }

                           }break;
                 case 0x11 :
                           {
                              while (0x00 == (Spi_Rf_ReadWrite(iOrder,iAddr,iData)&0x02))
                              {
                                 break;
                              }

                           }break;
                  case 0x12 :
                           {
                              while (0x00 == (Spi_Rf_ReadWrite(iOrder,iAddr,iData)&0x10))
                              {
                                 break;
                              }

                           }break;

                 default :  break;
            }
        }
    }
    else if(iClass == Wait)
    {
        bTime = iAddr;
        bTime = (bTime << 8) | iData;
        HAL_Delay(bTime);
    }
    return 0;
}
uint8_t Rf_Config(const uint32_t * pConfig, uint32_t iLength)
{
    uint32_t i;
    uint8_t  *tData;

    uint8_t  spiClass;
    uint8_t  spiOrder;
    uint8_t  spiAddr;
    uint8_t  spiData;
    tData = (uint8_t * )pConfig;
    for(i=0; i < iLength; i++)
    {
        spiData = *tData;
        tData++;
        spiAddr = *tData;
        tData++;
        spiOrder = *tData;
        tData++;
        spiClass = *tData;
        tData++;

        if(Rf_Handler(spiClass, spiOrder, spiAddr, spiData))
        {
            return 1;
        }
    }

    return 0;
}
void Rf_Load(void)
{
   Rf_Config(Config_RF_Data, sizeof(Config_RF_Data)/4 );
}

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
                Toggle 0x00[0], write ‘1’ first and then write ‘0’.
           1.3. Wait for 200ms. Then the RF calibration values are available.
        (2)Read RF calibration values
        (3)Write the calibration results back to the AR8003 related registers (all in page 1)
           ...Omitted here.
        (4)Use force calibration value mode (all in page 1)
           4.1. Write 0xA1=0x00, 0xA3=0x80, 0xA0=0x06
           4.2. Toggle 0x00[0], write ‘1’ first and then write ‘0’.
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
		Write_Back_AR8003(Set_Write ,CA_7,3);  //0x9
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
		Write_Back_AR8003(Set_Write ,CA_A,2);
		Write_Back_AR8003(Set_Write ,CA_A,1);
		Write_Back_AR8003(Set_Write ,CA_A,0);
	}
	else
	{
		Write_Back_AR8003(Reset_Write,CA_A,3);  //0x6
		Write_Back_AR8003(Set_Write ,CA_A,2);
		Write_Back_AR8003(Set_Write ,CA_A,1);
		Write_Back_AR8003(Reset_Write,CA_A,0);
	}
}


void Baseband_Calibration_Load(void)
{
  uint8_t i_92=0;
  uint8_t iaData = 0;
  uint8_t iData_temp = 0;
  uint8_t ibData = 0;
  uint8_t icData = 0;

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
  Baseband_Soft_Reset();   //4.2  Toggle 0x00[0], write ‘1’ first and then write ‘0’

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

void Baseband_Power_Up(void)
{
	do
	{
		Spi_Baseband_ReadWrite(spiWrite,FSM_0,0x80);
	}while(0x02 != Spi_Baseband_ReadWrite(spiRead,TX_A, 0x00));
}

//*********************** Sky_Ground_Sel **********************

void Sky_Grd_Sel(uint8_t SEL)
{
	GPIO_PinState pinState =  (SEL == SEL_SKY) ? GPIO_PIN_RESET : GPIO_PIN_SET;
	HAL_GPIO_WritePin(SEL_GS_GPIO_PORT, SEL_GS_PIN, pinState);
}

//********************** Baseband reset *********************
void Baseband_Reset(__IO uint32_t TIME)  //hardware reset
{
    HAL_GPIO_WritePin(BASEBAND_RESET_GPIO_PORT, BASEBAND_RESET, GPIO_PIN_RESET);
    HAL_Delay(TIME);
    HAL_GPIO_WritePin(BASEBAND_RESET_GPIO_PORT, BASEBAND_RESET, GPIO_PIN_SET);
}


//baseband Uart2<----->stm32 Uart1
void USART1_IRQHandler(void)
{
	uint32_t tmp1;
	tmp1 = __HAL_UART_GET_IT_SOURCE(&Uart1Handle, UART_IT_RXNE);
	if(tmp1 != RESET)
	{
		//uint8_t data = Uart1Handle.Instance->DR;
	}
}

//baseband Uart<----->stm32 Uart3
void USART3_IRQHandler(void)
{
	uint32_t tmp1 = 0;
	
	tmp1 = __HAL_UART_GET_IT_SOURCE(&Uart3Handle, UART_IT_RXNE);
	if(tmp1 != RESET)
	{
		//uint8_t c = (Uart3Handle.Instance->DR);
	}
}

void Sky_Grd_USART(void)
{

 /*  if(3==uart3_flag)
    {
         RxBufferData[RxBufferSize] = (Uart3Handle.Instance->DR);
       //  printf("%c",RxBufferData[RxBufferSize] );
         if( RxBufferSize < 0x0A )
        {
            RxBufferSize++;
        }
        else
        {
            RxBufferSize= 0;
        }
      uart3_flag=0;
     }*/

}

//***********************************************************************************

/**
  ==============================================================================
                      ##### STM32_Cy7c68013 Interface #####
  ==============================================================================

      * @brief  STM32_Cy7c68013 Interface
      * @note   Named for master(Cy7c68013)
      * @param
                 (+) PD[0:7]      -- Data_BUS
      *          (+) READ_SYNCLK
                 (+) READ_ID      -- When MCU write to cy7c68013 need to lock the databus,when READ_ID=0,
                                     the mcu is in write mode and tell the 68013 can not write
                 (+) WRITE_LOCK   -- When Cy7c68013 write to mcu need to lock the databus,when WRITE_LOCK=0,
                                     the Cy7c68013 is in write mode and tell the mcu,the mcu can not write
                 (+) WRITE_SYNCLK -- When Cy7c68013 write to MCU need a synclk
                 (+) USBRESET     -- Mcu Reset the Cy7c68013,hardware Reset
      * @{
      */
    /**
      * @}
      */


void Mcu_Write_Reset(void) { MCU_W_GPIO_PORT->BSRR = (uint32_t)MCU_W_PIN << 16;}
void Mcu_Write_Set(void) { MCU_W_GPIO_PORT->BSRR = MCU_W_PIN;}
void Mcu_Clk_Reset(void) { MCU_SCK_GPIO_PORT->BSRR = (uint32_t)MCU_SCK_PIN << 16;}
void Mcu_Clk_Set(void) { MCU_SCK_GPIO_PORT->BSRR = MCU_SCK_PIN;}

void Cy7c68013_Reset(__IO uint32_t TIME)
{
    HAL_GPIO_WritePin(CY7C68013_RESET_PORT, CY7C68013_RESET, GPIO_PIN_RESET);
    HAL_Delay(TIME);
    HAL_GPIO_WritePin(CY7C68013_RESET_PORT, CY7C68013_RESET, GPIO_PIN_SET);
}
void Bus_Set_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin =GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                            |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Bus_Set_Output(void) //
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin =GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                            |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Bus_Set_Data(uint8_t xDATA)
{
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                            |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA,xDATA,GPIO_PIN_SET);
}

void Temp_Delay(uint8_t d){uint8_t i;for(i=0;i<d;i++);}

void Cy7c68013_State_Init(void)
{
    Bus_Set_Input();
    Mcu_Write_Set();
    Mcu_Clk_Reset();
}

void Read_Cy7c68013(void)
{
    uint8_t num =0;

    uint8_t isreading=0;

    Bus_Set_Input();
    while(HAL_GPIO_ReadPin(CY7C68013_W_GPIO_PORT,CY7C68013_W_PIN) == GPIO_PIN_RESET) // wait w low
    {
		if((HAL_GPIO_ReadPin(CY7C68013_SCK_GPIO_PORT,CY7C68013_SCK_PIN) ==\
		GPIO_PIN_SET)&&(isreading==0)) // wait SCK HIGH
		{
			isreading =1;
			Mcu_Write_Reset();
			RXFIFO[num] = (uint8_t)(GPIOA->IDR & 0x00FF);
			num=num+1;
		}
		else if((HAL_GPIO_ReadPin(CY7C68013_SCK_GPIO_PORT,CY7C68013_SCK_PIN) ==\
		GPIO_PIN_RESET)&&(isreading==1)) // wait SCK low
		{
			Mcu_Write_Set();
			isreading = 0;
		}

		else if(HAL_GPIO_ReadPin(CY7C68013_SCK_GPIO_PORT,CY7C68013_SCK_PIN))
		{
			Mcu_Write_Set();
			isreading = 0;
		}
    }
}

void Write_Cy7c68013(uint8_t* iDATA,uint8_t len)
{
    uint8_t i ;
    while(HAL_GPIO_ReadPin(CY7C68013_W_GPIO_PORT,CY7C68013_W_PIN) ==  GPIO_PIN_RESET);  //  high

    for(i=0;i<len;i++)
    {
       while(HAL_GPIO_ReadPin(CY7C68013_W_GPIO_PORT,CY7C68013_W_PIN) == GPIO_PIN_RESET);//high
       Bus_Set_Data(iDATA[i]);
       Mcu_Clk_Set();
       while(HAL_GPIO_ReadPin(CY7C68013_W_GPIO_PORT,CY7C68013_W_PIN) == GPIO_PIN_SET);  //low
       Mcu_Clk_Reset();
    }
	
	Mcu_Clk_Reset();
}

void Rx_Cy7c_Msg(void)   // PC and MCU
{
    uint8_t inum = 0;
    uint8_t jnum = 0;
    Read_Cy7c68013();
    if(PC_HEADOR == RXFIFO[0])
     {
       for(jnum = 0; jnum < RXMSG_SIZE; jnum++ )
       {
         Rx_Addr_From_Cy7c68013[iRead_Addr_Num][jnum]=RXFIFO[jnum];
         RXFIFO[jnum]=0;
       }
       iRead_Addr_Num++;
     }
     else
     {
      for(inum = 0; inum < RXMSG_SIZE; inum++ )
       {
         RXFIFO[jnum]=0;
       }
     }
}

void Hanlde_Cy7c_Msg(void)
{
   uint8_t iNum_read = 0;
   uint8_t irxmsg_size = 0;
   uint32_t RX_CHS = 0;
   uint8_t RX_CHS_BYTE =0;

   for(iNum_read=0;iNum_read<iRead_Addr_Num;iNum_read++)
   {
     for(irxmsg_size=0;irxmsg_size<RXMSG_SIZE-1;irxmsg_size++)
     {
         RX_CHS += Rx_Addr_From_Cy7c68013[0][irxmsg_size];
     }
     RX_CHS_BYTE = ~(RX_CHS & 0x000000FF)+1;
     // Read register of AR8001.
     if((PC_HEADOR == Rx_Addr_From_Cy7c68013[iNum_read][0])&&\
        (PC_REGR == Rx_Addr_From_Cy7c68013[iNum_read][1])&&\
        (RX_CHS_BYTE == Rx_Addr_From_Cy7c68013[iNum_read][RXMSG_SIZE-1]))
     {
        Tx_Addr_To_Cy7c68013[iWrite_Addr_Num][0] = 0x55;
        Tx_Addr_To_Cy7c68013[iWrite_Addr_Num][1] = 0x0F;
        Tx_Addr_To_Cy7c68013[iWrite_Addr_Num][2] = Rx_Addr_From_Cy7c68013[iNum_read][2];
        Tx_Addr_To_Cy7c68013[iWrite_Addr_Num][3] =\
        Spi_Baseband_ReadWrite(spiRead,Rx_Addr_From_Cy7c68013[iNum_read][2],0x00);
        iWrite_Addr_Num ++ ;
        RX_CHS = 0;
        RX_CHS_BYTE = 0;
     }
     //Write register of AR8001.
     else if((PC_HEADOR == Rx_Addr_From_Cy7c68013[iNum_read][0])&&\
        (PC_REGW == Rx_Addr_From_Cy7c68013[iNum_read][1])&&\
        (RX_CHS_BYTE == Rx_Addr_From_Cy7c68013[iNum_read][RXMSG_SIZE-1]))
     {
         Spi_Baseband_ReadWrite(spiWrite,Rx_Addr_From_Cy7c68013[iNum_read][2],Rx_Addr_From_Cy7c68013[iNum_read][3]);
         RX_CHS = 0;
         RX_CHS_BYTE = 0;
     }
     //Enable searching id.
     else if((PC_HEADOR == Rx_Addr_From_Cy7c68013[iNum_read][0])&&\
        (PC_SIDEN == Rx_Addr_From_Cy7c68013[iNum_read][1])&&\
        (RX_CHS_BYTE == Rx_Addr_From_Cy7c68013[iNum_read][RXMSG_SIZE-1]))
     {
       PCState.CmdsearchID = ENABLE;

       RX_CHS_BYTE = 0;
       RX_CHS = 0;
     }
     //Disable searching id.
     else if((PC_HEADOR == Rx_Addr_From_Cy7c68013[iNum_read][0])&&\
        (PC_SIDDIS == Rx_Addr_From_Cy7c68013[iNum_read][1])&&\
        (RX_CHS_BYTE == Rx_Addr_From_Cy7c68013[iNum_read][RXMSG_SIZE-1]))
     {
     //  PCState.CmdsearchID = DISABLE;

       RX_CHS_BYTE = 0;
       RX_CHS = 0;
     }

     //Disable exteral irq.
     else if((PC_HEADOR==Rx_Addr_From_Cy7c68013[iNum_read][0])&&\
        (PC_EICLOSE==Rx_Addr_From_Cy7c68013[iNum_read][1])&&\
        (RX_CHS_BYTE == Rx_Addr_From_Cy7c68013[iNum_read][RXMSG_SIZE-1]))
     {
      //  PCState.CmdEXTI=DISABLE;
      //  HAL_NVIC_DisableIRQ(TIM2_IRQn);
      //  HAL_NVIC_DisableIRQ(TIM3_IRQn);
      //  HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
     //   HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

       RX_CHS = 0;
       RX_CHS_BYTE=0;
     }
     else
     {
        RX_CHS=0;
     }
   }

    memset(Rx_Addr_From_Cy7c68013,0,sizeof(Rx_Addr_From_Cy7c68013));

    iRead_Addr_Num=0;
    SysState.TxcyregOnly=ENABLE;
}


void Hanlde_Cy7c_Msg_NoEXIT(void)
{
   uint8_t iNum_read = 0;
   uint8_t irxmsg_size = 0;
   uint32_t RX_CHS   = 0;
   uint8_t RX_CHS_BYTE =0;

   for(iNum_read=0;iNum_read<iRead_Addr_Num;iNum_read++)
   {
     for(irxmsg_size=0;irxmsg_size<RXMSG_SIZE-1;irxmsg_size++)
     {
         RX_CHS += Rx_Addr_From_Cy7c68013[0][irxmsg_size];
     }
     RX_CHS_BYTE = ~(RX_CHS & 0x000000FF)+1;
     // Read register of AR8001.
     if((PC_HEADOR == Rx_Addr_From_Cy7c68013[iNum_read][0])&&\
        (PC_REGR == Rx_Addr_From_Cy7c68013[iNum_read][1])&&\
        (RX_CHS_BYTE == Rx_Addr_From_Cy7c68013[iNum_read][RXMSG_SIZE-1]))
     {
        Tx_Addr_To_Cy7c68013[iWrite_Addr_Num][0] = 0x55;
        Tx_Addr_To_Cy7c68013[iWrite_Addr_Num][1] = 0x0F;
        Tx_Addr_To_Cy7c68013[iWrite_Addr_Num][2] = Rx_Addr_From_Cy7c68013[iNum_read][2];
        Tx_Addr_To_Cy7c68013[iWrite_Addr_Num][3] =\
        Spi_Baseband_ReadWrite(spiRead,Rx_Addr_From_Cy7c68013[iNum_read][2],0x00);
        iWrite_Addr_Num ++ ;
        RX_CHS = 0;
        RX_CHS_BYTE=0;
     }
     //Write register of AR8001.
     else if((PC_HEADOR == Rx_Addr_From_Cy7c68013[iNum_read][0])&&\
        (PC_REGW == Rx_Addr_From_Cy7c68013[iNum_read][1])&&\
        (RX_CHS_BYTE == Rx_Addr_From_Cy7c68013[iNum_read][RXMSG_SIZE-1]))
     {
         Spi_Baseband_ReadWrite(spiWrite,Rx_Addr_From_Cy7c68013[iNum_read][2],Rx_Addr_From_Cy7c68013[iNum_read][3]);
         RX_CHS = 0;
         RX_CHS_BYTE=0;
     }
     //Enable exteral irq.
     else if((PC_HEADOR==Rx_Addr_From_Cy7c68013[iNum_read][0])&&\
        (PC_EIOPEN==Rx_Addr_From_Cy7c68013[iNum_read][1])&&\
        (RX_CHS_BYTE == Rx_Addr_From_Cy7c68013[iNum_read][RXMSG_SIZE-1]))
     {
    //   PCState.CmdEXTI=ENABLE;
       HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
       HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
       RX_CHS_BYTE=0;
       RX_CHS = 0;
     }
   }

    memset(Rx_Addr_From_Cy7c68013,0,sizeof(Rx_Addr_From_Cy7c68013));

    iRead_Addr_Num=0;
    SysState.TxcyregOnly=ENABLE;
}

void Tx_Cy7c_Msg(void)
{

   uint8_t iNum_write=0;
     if(ENABLE== SysState.TxcyosdOnly)
     {
        SysState.TxcyosdOnly=DISABLE ;
        if(HAL_GPIO_ReadPin(CY7C68013_W_GPIO_PORT,CY7C68013_W_PIN) == GPIO_PIN_RESET)
        {return;}
        Mcu_Write_Reset();
        Bus_Set_Output();
        Write_Cy7c68013(Txosd_Buffer,20);
        Bus_Set_Input();
        Mcu_Write_Set();
     }
     if(ENABLE== SysState.TxcyregOnly)
     {
       SysState.TxcyregOnly=DISABLE ;
       if(HAL_GPIO_ReadPin(CY7C68013_W_GPIO_PORT,CY7C68013_W_PIN) == GPIO_PIN_RESET)
       {return;}
       Mcu_Write_Reset();
       Bus_Set_Output();
       for(iNum_write=0;iNum_write<iWrite_Addr_Num;iNum_write++)
       {

        TXFIFO[0]= Tx_Addr_To_Cy7c68013[iNum_write][0];
        TXFIFO[1]= Tx_Addr_To_Cy7c68013[iNum_write][1];
        TXFIFO[2]= Tx_Addr_To_Cy7c68013[iNum_write][2];
        TXFIFO[3]= Tx_Addr_To_Cy7c68013[iNum_write][3];
        Write_Cy7c68013(TXFIFO,4);
        TXFIFO[0]=0;
        TXFIFO[1]=0;
        TXFIFO[2]=0;
        TXFIFO[3]=0;
     }
     iWrite_Addr_Num=0;
     Bus_Set_Input();
     Mcu_Write_Set();

    memset(Tx_Addr_To_Cy7c68013,0,sizeof(Tx_Addr_To_Cy7c68013));

   }

}

