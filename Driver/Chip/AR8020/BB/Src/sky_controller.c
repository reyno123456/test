/**
  ******************************************************************************
  * @file    sky_controller c file.
  * @author  Artosyn AE/FAE Team
  * @version V1.0
  * @date    03-21-2016
  * @brief
  *
  *
  ******************************************************************************
  */
#include "sky_controller.h"
#include "config_functions_sel.h"

#include <timer.h>
#include "interrupt.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "reg_rw.h"
#include "config_baseband_register.h"
#include "config_baseband_frqdata.h"
#include "sys_peripheral_communication.h"
#include "BB_ctrl.h"
#include "sys_peripheral_init.h"
#include "debuglog.h"

extern Sys_FlagTypeDef  SysState;
extern uint8_t Txosd_Buffer[];

Sky_FlagTypeDef SkyState;
Sky_HanlderTypeDef SkyStruct;
IDRx_TypeDef IdStruct;

uint32_t Device_ID[6]={0};//flash stored.

static init_timer_st sky_timer0_0;
static init_timer_st sky_timer0_1;

void Sky_Parm_Initial(void)
{
    SkyStruct.RCChannel=9;
    SkyStruct.ITChannel=0;
    SkyStruct.Timerirqcnt=0 ;
    SkyStruct.OptID=0;
    SkyStruct.FindIDcnt=0;
    SkyStruct.IDsearchcnt =0;
    SkyStruct.IDmatcnt=0;
    SkyStruct.en_agcmode = UNKOWN_AGC;
    SkyStruct.workfrq = 0xff;
    SkyStruct.Changeqammode = 0;

    SkyState.Rcmissing = DISABLE;
    SkyState.CmdsearchID = ENABLE;
    SkyState.Cmdtestmode = DISABLE;
    
    Sky_Timer0_Init();
    Sky_Timer1_Init();
    
    reg_IrqHandle(BB_RX_ENABLE_VECTOR_NUM, wimax_vsoc_rx_isr);
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);
}


#if 0
/**
* @brief STORED ID in FLASH.
*
* @illustrate
*
 (+) Flash_Write().
 (+) Flash_Read().
*/
uint8_t Flash_Write(uint32_t Start_Addr,uint32_t *p_data,uint32_t size)
{
}

uint8_t Flash_Read(uint32_t Start_Addr,uint32_t *p_data,uint32_t size)
{
}
#endif


void Sky_Write_Rchopfrq(void)
{
    SkyStruct.RCChannel ++;
    if(SkyStruct.RCChannel >=  MAX_RC_FRQ_SIZE)
    {
        SkyStruct.RCChannel = 0;
    }
    
    BB_set_Rcfrq(SkyStruct.RCChannel);
}


void Sky_Write_Itfrq(uint8_t ch)
{
    if(SkyStruct.workfrq != ch)
    {
        BB_set_ITfrq(ch);
        SkyStruct.workfrq = ch;
        printf("S=>%d\r\n", ch);
    }
}

void Sky_set_RCids(uint8_t ids[])
{
    uint8_t i;
    uint8_t addr[] = {FEC_7, FEC_8, FEC_9, FEC_10, FEC_11};
    for(i=0; i < sizeof(addr); i++)
    {
        BB_WriteReg(PAGE2, addr[i], ids[i]);
    }

    BB_set_Rcfrq(SKY_RC_FRQ_INIT);
    BB_softReset(BB_SKY_MODE );
}

/**
  * @brief  Sky Baseband PAGE2 reg[D9] value.
  * @illustrate
  *
     (+) Reg[E9].bit0 -- Id_Match.
     (+) Reg[E9].bit1 -- Crc_Check_Ok.
     (+) Reg[E9].bit7 -- Rc_Err_Flg.
     (+) Reg[E9]=0   <--> no 14ms irq.
  */
uint8_t Sky_Id_Match(void)
{
    static int total_count = 0;
    static int lock_count = 0;
    uint8_t data = BB_ReadReg(PAGE2, FEC_4_RD) & 0x03;

    total_count ++;
    lock_count += ((data) ? 1 : 0);
        
    if(total_count > 500)
    {
        printf("-L:%d-\n", lock_count);    
        total_count = 0;
        lock_count = 0;
    }
    
    return (data) ? 1 : 0;
}

uint8_t  Min_of_Both(uint8_t a, uint8_t b ){return((a < b) ? a : b);}

/**
 * @brief Record the ID and power if Reg[E9]bit1=1 , stored in IdStruct[].
 */
void Sky_Record_Idpower( uint8_t i )
{
    IdStruct[i].num = i ;

    IdStruct[i].rcid5 = BB_ReadReg(PAGE2, FEC_1_RD);
    IdStruct[i].rcid4 = BB_ReadReg(PAGE2, FEC_2_RD_1);
    IdStruct[i].rcid3 = BB_ReadReg(PAGE2, FEC_2_RD_2);
    IdStruct[i].rcid2 = BB_ReadReg(PAGE2, FEC_2_RD_3);
    IdStruct[i].rcid1 = BB_ReadReg(PAGE2, FEC_2_RD_4);
    IdStruct[i].powerc5= BB_ReadReg(PAGE2, AAGC_2_RD);
    IdStruct[i].powerc6 = BB_ReadReg (PAGE2, AAGC_3_RD);

    if((0 == IdStruct[i].powerc5)||(0 == IdStruct[i].powerc6))
    {
        if(0 == IdStruct[i].powerc5)
        {
            IdStruct[i].powermin = IdStruct[i].powerc6;
        }
        else if (0 == IdStruct[i].powerc6)
        {
            IdStruct[i].powermin = IdStruct[i].powerc5;
        }
    }
    else
    {
       IdStruct[i].powermin = Min_of_Both(IdStruct[i].powerc5, IdStruct[i].powerc6);
    }
}

uint8_t Sky_Getopt_Id(void)
{
    uint8_t inum=0;
    uint8_t mData=0;
    uint8_t Optid=0;
    mData = IdStruct[0].powermin;
    for(inum = 0; inum<= SkyStruct.IDmatcnt; inum++)
    {
        if((IdStruct[inum].powermin <= mData)&&(IdStruct[inum].powermin > 0))
        {
            mData = IdStruct[inum].powermin;
            Optid = inum;
        }
    }
    return Optid;
}



void Sky_Search_Right_ID(void)
{    
    SkyStruct.Rcunlockcnt=0;

    if( SkyStruct.rc_error)
    {
        BB_softReset(BB_SKY_MODE );
    }
        
    if(DISABLE==SkyState.Rcmissing)
    {
        if(SkyStruct.rc_crc_ok)
        {
            Sky_Record_Idpower(SkyStruct.IDmatcnt);
            Sky_Write_Rchopfrq();
            SkyStruct.IDmatcnt++;
            BB_softReset(BB_SKY_MODE );
        }

        if(SkyStruct.IDsearchcnt >= ID_SEARCH_MAX_TIMES)
        {
            SkyStruct.IDsearchcnt =0;
            Sky_Write_Rchopfrq();
        }
        else
        {
            SkyStruct.IDsearchcnt++;
        }

        if( ID_MATCH_MAX_TIMES <= SkyStruct.IDmatcnt )
        {
            SkyStruct.OptID = Sky_Getopt_Id();
            BB_WriteReg(PAGE2, FEC_7 , IdStruct[SkyStruct.OptID].rcid5);
            BB_WriteReg(PAGE2, FEC_8 , IdStruct[SkyStruct.OptID].rcid4);
            BB_WriteReg(PAGE2, FEC_9 , IdStruct[SkyStruct.OptID].rcid3);
            BB_WriteReg(PAGE2, FEC_10, IdStruct[SkyStruct.OptID].rcid2);
            BB_WriteReg(PAGE2, FEC_11, IdStruct[SkyStruct.OptID].rcid1);

            Device_ID[0] = Device_ID[0]|IdStruct[SkyStruct.OptID].rcid5;
            Device_ID[1] = Device_ID[1]|IdStruct[SkyStruct.OptID].rcid4;
            Device_ID[2] = Device_ID[2]|IdStruct[SkyStruct.OptID].rcid3;
            Device_ID[3] = Device_ID[3]|IdStruct[SkyStruct.OptID].rcid2;
            Device_ID[4] = Device_ID[4]|IdStruct[SkyStruct.OptID].rcid1;

            printf("RCid:%02x%02x%02x%02x%02x\r\n", \
                IdStruct[SkyStruct.OptID].rcid5, IdStruct[SkyStruct.OptID].rcid4, IdStruct[SkyStruct.OptID].rcid3, \
                IdStruct[SkyStruct.OptID].rcid2, IdStruct[SkyStruct.OptID].rcid1);

            //Device ID stored in FLASH
            //Sky_Flash_Write( FLASH_START_PAGE_ADDRESS,Device_ID,5);
            //Sky_Flash_Read(FLASH_START_PAGE_ADDRESS,read_id,5);

            SkyState.CmdsearchID = DISABLE;
            SkyStruct.IDsearchcnt=0;
            SkyStruct.IDmatcnt=0;
        }
        else
        {
            SkyState.CmdsearchID = ENABLE;
        }
    }
    else
    {
        if(Sky_Id_Match())
        {
            Sky_Write_Rchopfrq();
            SkyStruct.IDsearchcnt =0;
            SkyState.CmdsearchID = DISABLE;
        }
        else if(SkyStruct.IDsearchcnt>=ID_SEARCH_MAX_TIMES)
        {
            Sky_Write_Rchopfrq();
            BB_softReset(BB_SKY_MODE );
            SkyStruct.IDsearchcnt =0;
        }
        else
        {
            SkyStruct.IDsearchcnt ++;
        }
    }
}

static uint8_t Sky_set_QAM(uint8_t mode)
{
    uint8_t reg = BB_ReadReg(PAGE2, TX_2);
    BB_WriteReg(PAGE2, TX_2, (reg & 0x3f) | mode);
}

void Sky_Rc_Hopping(void)
{
    Sky_Write_Rchopfrq();
    if(Sky_Id_Match())
    {
        uint8_t data0, data1;
        SkyStruct.Rcunlockcnt=0;
        data0 = BB_ReadReg(PAGE2, IT_FREQ_TX_0);
        data1 = BB_ReadReg(PAGE2, IT_FREQ_TX_1);
        if((data0 == data1) && (0x0e==(data1 >> 4)))
        {
            Sky_Write_Itfrq(data0 & 0x0F);
        }

        data0 = BB_ReadReg(PAGE2,QAM_CHANGE_0);
        data1 = BB_ReadReg(PAGE2,QAM_CHANGE_1);
        if(SkyStruct.Changeqammode!=data0 && data0 == data1 &&
            (data0==0xF1 || data0==0xF3 || data0==0xF7 || data0==0xF9))
        {
            SkyStruct.Changeqammode = data0;
            printf("Q=>0x%x\r\n",data0);
            if(0xF1==data0)
            {
                Sky_set_QAM(QAM_BPSK);
            }
            else if(0xF3==data0)
            {
                Sky_set_QAM(QAM_4QAM);
            }
            else if(0xF7==data0)
            {
                Sky_set_QAM(QAM_16QAM);
            }
            else if(0xF9==data0)
            {
                Sky_set_QAM(QAM_64QAM);
            }
        }
    }
    else
    {
        if(SkyStruct.rc_error)
        {
            SkyStruct.Rcunlockcnt++;
        }
        if(MAX_RC_FRQ_SIZE <= SkyStruct.Rcunlockcnt)
        {
            SkyStruct.Rcunlockcnt=0;
            SkyState.CmdsearchID = ENABLE;
            SkyState.Rcmissing = ENABLE;
            SkyStruct.IDmatcnt = ID_SEARCH_MAX_TIMES;
        }
    }
}

void Sky_Adjust_AGCGain(void)
{
    uint8_t rx1_gain = BB_ReadReg(PAGE2, AAGC_2_RD);
    uint8_t rx2_gain = BB_ReadReg(PAGE2, AAGC_3_RD);

    if((rx1_gain >= POWER_GATE)&&(rx2_gain >= POWER_GATE) \
        && SkyStruct.en_agcmode != FAR_AGC)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_FAR);

        SkyStruct.en_agcmode = FAR_AGC;
        printf("=>F", rx1_gain, rx2_gain);
    }
     
    if( ((rx1_gain < POWER_GATE)&&(rx2_gain < POWER_GATE)) \
        && (rx1_gain > 0x00) && (rx2_gain >0x00)  \
        && SkyStruct.en_agcmode != NEAR_AGC)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_NEAR);
        SkyStruct.en_agcmode = NEAR_AGC;
        printf("=>N", rx1_gain, rx2_gain);
    }
}

void Sky_Adjust_AGCGain_SearchID(uint8_t i)
{
    BB_WriteReg(PAGE0, AGC_2, (i==0) ? AAGC_GAIN_FAR:AAGC_GAIN_NEAR);
}

void Sky_Hanlde_SpecialIrq(void)
{
    uint8_t reg = BB_ReadReg(PAGE2, FEC_4_RD); //can't use the skystruct, because the 14ms intr may not happen
    if(reg == 0)
    {
        if(SkyStruct.CntAGCGain>2)
        {
            SkyStruct.CntAGCGain=0;
            Sky_Adjust_AGCGain_SearchID(0);
        }
        else
        {
            SkyStruct.CntAGCGain++;
            Sky_Adjust_AGCGain_SearchID(1);
        }
        Sky_Write_Rchopfrq();
        BB_softReset(BB_SKY_MODE );
    }
}

void Sky_Osdmsg_Ptf(void)
{
#if 0
    uint8_t iData_page_temp=0;

    Txosd_Buffer[0]=0x55;
    Txosd_Buffer[1]=0xaa;
    /*iData_page_temp= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);   //page2
    Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp& PAGE_2);*/

    Txosd_Buffer[3]=0;
    Txosd_Buffer[7]=0;
    Txosd_Buffer[8]/*iData_C3_temp = Spi_Baseband_ReadWrite(spiRead, AAGC_0_RD, 0x00);*/=0;
    Txosd_Buffer[9]/*iData_C7_temp = Spi_Baseband_ReadWrite(spiRead, AAGC_4_RD, 0x00);*/=0;
    Txosd_Buffer[10]/*iData_C4_temp= Spi_Baseband_ReadWrite(spiRead, AAGC_1_RD, 0x00);*/=0;
    //  Txosd_Buffer[4]/*iData_C5_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_2_RD, 0x00);
    //  Txosd_Buffer[5]/*iData_C6_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_3_RD, 0x00);
    //  Txosd_Buffer[6]/*iData_Eb_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_4_RD , 0x00);

    Txosd_Buffer[4]/*iData_C5_temp*/= BB_ReadReg(PAGE2, AAGC_2_RD);
    Txosd_Buffer[5]/*iData_C6_temp*/= BB_ReadReg(PAGE2, AAGC_3_RD);
    Txosd_Buffer[6]/*iData_Eb_temp*/= BB_ReadReg(PAGE2, FEC_4_RD);

    Txosd_Buffer[11]/*iData_E0_temp= Spi_Baseband_ReadWrite(spiRead, FEC_0_RD , 0x00);*/=0;

    Txosd_Buffer[13]/*iData_SNRH_temp = Spi_Baseband_ReadWrite(spiRead, CE_9_RD, 0x00);*/ =0;
    Txosd_Buffer[14]/*iData_SNRL_temp= Spi_Baseband_ReadWrite(spiRead, CE_A_RD, 0x00);*/ =0;

    Txosd_Buffer[15]/*iData_E7_temp = Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_1, 0x00);*/=0;
    Txosd_Buffer[16]/*iData_E8_temp= Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_2, 0x00);*/=0;
    Txosd_Buffer[17]/*iData_EE_temp = Spi_Baseband_ReadWrite(spiRead, FEC_9_RD, 0x00);*/=0;
    Txosd_Buffer[18]/*iData_EF_temp = Spi_Baseband_ReadWrite(spiRead, FEC_10_RD, 0x00);*/=0;
    //  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp);

    Txosd_Buffer[19]=0;
    SysState.TxcyosdOnly=ENABLE;
#endif
}

//*********************TX RX initial(14ms irq)**************
void wimax_vsoc_rx_isr()
{
    INTR_NVIC_DisableIRQ(BB_RX_ENABLE_VECTOR_NUM);   
    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM);
    TIM_StartTimer(sky_timer0_0);
}

void Sky_TIM0_IRQHandler(void)
{
    Reg_Read32(BASE_ADDR_TIMER0 + TMRNEOI_0);
    INTR_NVIC_ClearPendingIRQ(BB_RX_ENABLE_VECTOR_NUM); //clear pending after TX Enable is LOW. MUST!
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);  
    
    INTR_NVIC_DisableIRQ(TIMER_INTR00_VECTOR_NUM);
    TIM_StopTimer(sky_timer0_0);

    uint8_t rc_data = BB_ReadReg(PAGE2, FEC_4_RD);
    SkyStruct.rc_error = (0x80 == rc_data);
    SkyStruct.rc_crc_ok = ((rc_data & 0x02) ? 1 : 0);
    //uint8_t rc_id_match = ((rc_data & 0x02) ? 1 : 0);
    
    if( ENABLE == SkyState.CmdsearchID)
    {
        Sky_Search_Right_ID();
    }
    else
    {
        SkyState.CmdsearchID = DISABLE;
        SkyState.Rcmissing = ENABLE;
        Sky_Rc_Hopping();
    }
    //Sky_Adjust_AGCGain();
}


void Sky_TIM1_IRQHandler(void)
{
    static int Timer1_Delay2_Cnt = 0;
    INTR_NVIC_ClearPendingIRQ(TIMER_INTR01_VECTOR_NUM);
    if(Timer1_Delay2_Cnt < 560)
    {
        Timer1_Delay2_Cnt ++;
    }
    else
    {
        Sky_Hanlde_SpecialIrq();
        Timer1_Delay2_Cnt  = 0;
    }
}

void Sky_Timer1_Init(void)
{
    sky_timer0_1.base_time_group = 0;
    sky_timer0_1.time_num = 1;
    sky_timer0_1.ctrl = 0;
    sky_timer0_1.ctrl |= TIME_ENABLE | USER_DEFINED;
    TIM_RegisterTimer(sky_timer0_1, 1000);
    reg_IrqHandle(TIMER_INTR01_VECTOR_NUM, Sky_TIM1_IRQHandler);
}

void Sky_Timer0_Init(void)
{
    sky_timer0_0.base_time_group = 0;
    sky_timer0_0.time_num = 0;
    sky_timer0_0.ctrl = 0;
    sky_timer0_0.ctrl |= TIME_ENABLE | USER_DEFINED;

    TIM_RegisterTimer(sky_timer0_0, 6800);

    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, Sky_TIM0_IRQHandler);
}