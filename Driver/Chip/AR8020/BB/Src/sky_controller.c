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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "reg_rw.h"
#include "timer.h"
#include "interrupt.h"

#include "config_baseband_register.h"
#include "debuglog.h"
#include "sys_event.h"


Sky_FlagTypeDef     SkyState;
Sky_HanlderTypeDef  SkyStruct;
STRU_RCID_power     IdStruct;

static init_timer_st sky_timer0_0;
static init_timer_st sky_timer0_1;

static void Sky_set_ITQAM_and_notify(EN_BB_QAM mod);
static void Sky_Timer1_Init(void);
static void Sky_Timer0_Init(void);

void Sky_Parm_Initial(void)
{
    SkyStruct.RCChannel     = 0;
    SkyStruct.ITChannel     = 0;
    SkyStruct.Timerirqcnt   = 0;
    SkyStruct.OptID         = 0;
    SkyStruct.FindIDcnt     = 0;
    SkyStruct.IDsearchcnt   = 0;
    SkyStruct.IDmatcnt      = 0;
    SkyStruct.en_agcmode    = UNKOWN_AGC;
    SkyStruct.workfrq       = 0xff;
    SkyStruct.cur_QAM       = MOD_MAX;

    SkyState.Rcmissing      = DISABLE;
    SkyState.CmdsearchID    = ENABLE;
    SkyState.Cmdtestmode    = DISABLE;
    
    Sky_Timer0_Init();
    Sky_Timer1_Init();

    Sky_set_ITQAM_and_notify(MOD_16QAM);
    reg_IrqHandle(BB_RX_ENABLE_VECTOR_NUM, wimax_vsoc_rx_isr);
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);
}


/** 
 * @brief       Get the BB can support the max bitreate in current modulation.       
 * @retval      max bitreate(Mbps)
 */
static int BB_Sky_GetsupportBR(EN_BB_QAM QAM, EN_BB_LDPC ldpc, EN_BB_BW bw)
{
    if(QAM == MOD_BPSK)
    {
        return 2;
    }
    else if(QAM == MOD_4QAM)
    {
        return 5;
    }
    else if(QAM == MOD_16QAM)
    {
        return 10;
    }
    else if(QAM == MOD_64QAM)
    {
        return 13;
    }
}

static void Sky_set_ITQAM_and_notify(EN_BB_QAM mod)
{    
    BB_set_QAM(mod);
    printf("!!Q=>%d\r\n", mod);

    STRU_SysEvent_BB_ModulationChange event;
    event.BB_MAX_support_br = BB_Sky_GetsupportBR(mod, 0, 0);    
    SYS_EVENT_Notify(SYS_EVENT_ID_BB_SUPPORT_BR_CHANGE, (void*)&event);
}


void Sky_Write_RChopfrq(void)
{
    SkyStruct.RCChannel ++;
    if(SkyStruct.RCChannel >=  MAX_RC_FRQ_SIZE)
    {
        SkyStruct.RCChannel = 0;
    }
    
    BB_set_Rcfrq(SkyStruct.RCChannel);
}


void Sky_Write_ITfrq(uint8_t ch)
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
    
    IdStruct[i].aagc1 = BB_ReadReg(PAGE2, AAGC_2_RD);
    IdStruct[i].aagc2 = BB_ReadReg(PAGE2, AAGC_3_RD);

    if(0 == IdStruct[i].aagc1 || IdStruct[i].aagc1 > IdStruct[i].aagc2)
    {
        IdStruct[i].aagcmin = IdStruct[i].aagc2;
    }
    else
    {
        IdStruct[i].aagcmin = IdStruct[i].aagc1;
    }
}

uint8_t Sky_Getopt_Id(void)
{
    uint8_t inum  = 0;
    uint8_t mData = 0;
    uint8_t Optid = 0;
    
    mData = IdStruct[0].aagcmin;
    for(inum = 1; inum < SkyStruct.IDmatcnt; inum++)
    {
        //Agc low means power is high
        if((IdStruct[inum].aagcmin <= mData)&& (IdStruct[inum].aagcmin > 0) )
        {
            mData = IdStruct[inum].aagcmin;
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
            Sky_Write_RChopfrq();
            SkyStruct.IDmatcnt++;
            BB_softReset(BB_SKY_MODE );
        }

        if(SkyStruct.IDsearchcnt >= ID_SEARCH_MAX_TIMES)
        {
            SkyStruct.IDsearchcnt =0;
            Sky_Write_RChopfrq();
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

            printf("RCid:%02x%02x%02x%02x%02x\r\n", \
                IdStruct[SkyStruct.OptID].rcid5, IdStruct[SkyStruct.OptID].rcid4, IdStruct[SkyStruct.OptID].rcid3, \
                IdStruct[SkyStruct.OptID].rcid2, IdStruct[SkyStruct.OptID].rcid1);

            /*
            * Todo: Add Event to notify
            */

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
            Sky_Write_RChopfrq();
            SkyStruct.IDsearchcnt =0;
            SkyState.CmdsearchID = DISABLE;
        }
        else if(SkyStruct.IDsearchcnt>=ID_SEARCH_MAX_TIMES)
        {
            Sky_Write_RChopfrq();
            BB_softReset(BB_SKY_MODE );
            SkyStruct.IDsearchcnt =0;
        }
        else
        {
            SkyStruct.IDsearchcnt ++;
        }
    }
}

void Sky_Rc_Hopping(void)
{
    Sky_Write_RChopfrq();
    if(Sky_Id_Match())
    {
        uint8_t data0, data1;

        data0 = BB_ReadReg(PAGE2, IT_FREQ_TX_0);
        data1 = BB_ReadReg(PAGE2, IT_FREQ_TX_1);
        if((data0 == data1) && (0x0e==(data1 >> 4)))
        {
            Sky_Write_ITfrq(data0 & 0x0F);
        }

        data0 = BB_ReadReg(PAGE2,QAM_CHANGE_0);
        data1 = BB_ReadReg(PAGE2,QAM_CHANGE_1);
        
        if(data0 == data1 && (data0&0xFC) == 0xF0)
        {
            EN_BB_QAM mod = (EN_BB_QAM)(data0&0x03);
            if(SkyStruct.cur_QAM != mod)
            {
                Sky_set_ITQAM_and_notify(mod);
                SkyStruct.cur_QAM = mod;
            }
        }

        SkyStruct.Rcunlockcnt=0;
    }
    else
    {
        if(SkyStruct.rc_error)
        {
            SkyStruct.Rcunlockcnt++;
        }
        if(MAX_RC_FRQ_SIZE <= SkyStruct.Rcunlockcnt)
        {
            SkyStruct.Rcunlockcnt = 0;
            SkyState.CmdsearchID  = ENABLE;
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
        Sky_Write_RChopfrq();
        BB_softReset(BB_SKY_MODE );
    }
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