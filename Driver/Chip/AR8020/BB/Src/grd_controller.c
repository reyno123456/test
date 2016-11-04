#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "config_functions_sel.h"
#include "debuglog.h"
#include "interrupt.h"
#include "timer.h"
#include "reg_rw.h"
#include "config_baseband_frqdata.h"
#include "config_baseband_register.h"
#include "sys_peripheral_communication.h"
#include "BB_ctrl.h"
#include "sys_peripheral_init.h"
#include "grd_controller.h"

extern Sys_FlagTypeDef SysState;
extern uint8_t Txosd_Buffer[];

static init_timer_st init_timer0_0;
static init_timer_st init_timer0_1;

Grd_FlagTypeDef   GrdState;
Grd_HandleTypeDef GrdStruct;
Grd_QAMTypeDef    GrdQam;

static uint8_t  Timer1_Delay1_Cnt = 0;

void Grd_Parm_Initial(void)
{
    GrdStruct.RCChannel=1;
    GrdStruct.ITManualChannel=0;
    GrdStruct.ITAutoChannel=0;
    GrdStruct.Sweepfrqunlock=0xff;
    GrdStruct.Sweepfrqlock=0;
    GrdStruct.ITTxChannel=0;
    GrdStruct.Harqcnt=0;
    GrdStruct.ReHarqcnt=0;
    GrdStruct.ITTxCnt=0;
    GrdStruct.CgITfrqspan=0;
    GrdStruct.SweepCyccnt=0;
    GrdStruct.ITunlkcnt=0;
    GrdStruct.workfrqcnt = 0xff;

    GrdState.ITManualmode = DISABLE;
    GrdState.ITAutomode = ENABLE;
    GrdState.GetITfrq = DISABLE;
    GrdState.RegetITfrq = DISABLE;
    GrdState.Endsweep = DISABLE;
    GrdState.GetfrqOnly= ENABLE;
    GrdState.FEClock = DISABLE;
    GrdState.Allowjglock=DISABLE;

    GrdQam.Down16qam12=DISABLE;
    GrdQam.Downqpsk12=DISABLE;
    GrdQam.Downbpsk12=DISABLE;
    
    GrdQam.Up64qam12=DISABLE;
    GrdQam.Up16qam12=DISABLE;
    GrdQam.Upqpsk12=DISABLE;
    
    Grd_Timer0_Init();
    Grd_Timer1_Init();

    reg_IrqHandle(BB_TX_ENABLE_VECTOR_NUM, wimax_vsoc_tx_isr);
    INTR_NVIC_EnableIRQ(BB_TX_ENABLE_VECTOR_NUM);
}

void BB_Grd_Id_Initial(void)
{
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT39_32_REG, RC_ID_BIT39_32);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT31_24_REG, RC_ID_BIT31_24);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT23_16_REG, RC_ID_BIT23_16);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT15_08_REG, RC_ID_BIT15_08);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT07_00_REG, RC_ID_BIT07_00);
}


void Grd_RC_Controller(void)
{
    GrdStruct.RCChannel ++;
    if(GrdStruct.RCChannel >=  MAX_RC_FRQ_SIZE)
    {
        GrdStruct.RCChannel = 0;
    }

    BB_set_Rcfrq(GrdStruct.RCChannel);
}

/**
      * @brief  Ground Image Transmission Section.
      *
      * @illustrate
      *
         (+) Ground write image transmissions frequency in baseband.
            --BB_set_sweepfrq();
            --Grd_Write_Itfrq();
         (+) Grd_Id_Initial().
         (+) Grd_RC_Controller().

      * @{
      *
      * @}
      */

/*!< Specifies the struct of sweeping power with image transmissions.*/
struct SWEEP_POWER SP[MAX_RC_FRQ_SIZE]={0};
/*!< Specifies the frequency sequency in method of sweeping power.*/
struct FRQ_SEQ_ORDER FSO[MAX_RC_FRQ_SIZE]={0};
/*!< Specifies the data structure of working at current / alternative / others.*/
struct CURRENT_ALTER_OTHERS_FRQ CAOF[MAX_RC_FRQ_SIZE]={0};
/*!< Specifies the data structure of SNR 8 times in one cycle.*/
struct SNR_PERCYC SNRPC[SNRNUM_PER_CYC]={0};
/*!< Specifies the data structure of SNR 4*14ms in 4 cycles.
   The last list is average.*/
uint32_t Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS][FRQ_SNR_BLOCK_LISTS]={0};
/*!< Specifies the data structure of SNR 32*14ms in 32 cycles.
   The last list is average.*/
uint32_t Snr_Qam_Block[QAM_SNR_BLOCK_ROWS][QAM_SNR_BLOCK_LISTS]={0};
/*!< Specifies the data structure of ldpc err num in every cycles.*/
uint16_t  Ldpc_Block[LDPC_STATIC_NUM]={0};

//write IT ch and 
void Grd_Write_Itfrq(uint8_t ch)
{
    if(GrdStruct.workfrqcnt != ch)
    {
        BB_set_ITfrq(ch);
        //Notify sky
        BB_WriteReg(PAGE2, IT_FREQ_TX_0, 0xE0 + ch);
        BB_WriteReg(PAGE2, IT_FREQ_TX_1, 0xE0 + ch);

        GrdStruct.workfrqcnt = ch;        
        printf("G=>%d\r\n", ch);       
    }
}


uint8_t Grd_Baseband_Fec_Lock(void)
{
    static uint8_t status = 0xff;
    uint8_t data = BB_ReadReg(PAGE2, FEC_5_RD) & 0x01;
    if(status != data)
    {
        printf("ML:%d\r\n", data);
        status = data;
    }
    return data;
}

void Grd_Sweeping_Energy_Statistic(uint8_t ch)
{
    uint32_t  Energy_Hgh = 0;
    uint32_t  Energy_Mid = 0;
    uint32_t  Energy_Low = 0;
    
    Energy_Low = BB_ReadReg(PAGE2, SWEEP_ENERGY_LOW);
    Energy_Mid = BB_ReadReg(PAGE2, SWEEP_ENERGY_MID);
    Energy_Hgh = BB_ReadReg(PAGE2, SWEEP_ENERGY_HIGH);
    
    SP[ch].frqchannel= ch;
    SP[ch].powerall1=SP[ch].powerall2;
    SP[ch].powerall2=SP[ch].powerall3;
    SP[ch].powerall3=SP[ch].powerall4;
    SP[ch].powerall4=((Energy_Hgh<<16)|(Energy_Mid<<8)|Energy_Low);
    SP[ch].poweravr=(SP[ch].powerall1+SP[ch].powerall2+SP[ch].powerall3+SP[ch].powerall4)>>2;

    if((SP[ch].powerall1>=SP[ch].powerall2)&&(SP[ch].powerall1>=SP[ch].powerall3)&&(SP[ch].powerall1>=SP[ch].powerall4))
    {
        SP[ch].powerwave =SP[ch].powerall1 - SP[ch].poweravr;
    }
    else if((SP[ch].powerall2>=SP[ch].powerall1)&&(SP[ch].powerall2>=SP[ch].powerall3)&&(SP[ch].powerall2>=SP[ch].powerall4))
    {
        SP[ch].powerwave =(SP[ch].powerall2-SP[ch].poweravr);
    }
    else if((SP[ch].powerall3>=SP[ch].powerall1)&&(SP[ch].powerall3>=SP[ch].powerall2)&&(SP[ch].powerall3>=SP[ch].powerall4))
    {
        SP[ch].powerwave =(SP[ch].powerall3-SP[ch].poweravr);
    }
    else if((SP[ch].powerall4>=SP[ch].powerall1)&&(SP[ch].powerall4>=SP[ch].powerall2)&&(SP[ch].powerall4>=SP[ch].powerall3))
    {
        SP[ch].powerwave =(SP[ch].powerall4-SP[ch].poweravr);
    }

    Txosd_Buffer[8]=((SP[ch].poweravr&0xFF0000)>>16);
    Txosd_Buffer[9]=((SP[ch].poweravr&0xFF00)>>8);
    Txosd_Buffer[10]=SP[ch].poweravr;
}

/**
  * @brief  Sort the frq channel from sweeping results.
  * @param  num: the number of image transmissions frq channel.
  *
  */
void Grd_Itfrq_Sort(uint8_t num)   // sort  SP[]-->FSO[]
{
    uint8_t i=0,j=0,k=0;
    uint32_t k_temp=0;
    uint8_t num_temp=0;
    uint8_t iw_temp=0;
    uint8_t m=num;

    for(k=0;k<num;k++)
    {
        FSO[k].frqchannel = SP[k].frqchannel;
        FSO[k].poweravr = SP[k].poweravr;
        FSO[k].powerwave = SP[k].powerwave;
    }
    for( i=1; i<num; i++ )
    {
        m-=1;
        for(j=0;j<m;j++)
        {
            if(FSO[j].poweravr > FSO[j+1].poweravr)
            {
                k_temp = FSO[j].poweravr;
                num_temp = FSO[j].frqchannel;
                iw_temp = FSO[j].powerwave;
                FSO[j].poweravr = FSO[j+1].poweravr;
                FSO[j].frqchannel = FSO[j+1].frqchannel;
                FSO[j].powerwave = FSO[j+1].powerwave;
                FSO[j+1].poweravr = k_temp;
                FSO[j+1].frqchannel = num_temp;
                FSO[j+1].powerwave = iw_temp;
            }
        }
    }
}

void Grd_Alterfrq_Updute(uint8_t Itfrqchannel)
{
    uint8_t m_caf = MAX_RC_FRQ_SIZE;
    uint8_t i = 0;
    uint8_t j = 0;
    uint32_t k_temp = 0;
    uint8_t num_temp = 0;
    uint8_t iw_temp = 0;

    for( i=1; i<MAX_RC_FRQ_SIZE; i++ )
    {
        m_caf -= 1;
        for(j=1;j<m_caf;j++)
        {
            if(CAOF[j].poweravr > CAOF[j+1].poweravr)
            {
                k_temp = CAOF[j].poweravr;
                num_temp = CAOF[j].frqchannel;
                iw_temp = CAOF[j].powerwave;
                CAOF[j].poweravr = CAOF[j+1].poweravr;
                CAOF[j].frqchannel = CAOF[j+1].frqchannel;
                CAOF[j].powerwave = CAOF[j+1].powerwave;
                CAOF[j+1].poweravr = k_temp;
                CAOF[j+1].frqchannel = num_temp;
                CAOF[j+1].powerwave = iw_temp;
            }
        }
    }
}

/**
  * @brief  Sweeping as (1,2,3...) before FEC locked in ground.
  * @note   the sweeping sequency is different from that after locked.
  *
  */
void Grd_Sweeping_Before_Fec_Locked(void)
{
    if(GrdStruct.Sweepfrqunlock == 0xff)
    {
        BB_set_sweepfrq(0);
        GrdStruct.Sweepfrqunlock = 0; //To get engergy next cycle
    }
    else
    {
        Grd_Sweeping_Energy_Statistic(GrdStruct.Sweepfrqunlock);
        Txosd_Buffer[7]= GrdStruct.Sweepfrqunlock;

        GrdStruct.Sweepfrqunlock++;
        if(GrdStruct.Sweepfrqunlock >= MAX_RC_FRQ_SIZE)
        {
            GrdStruct.Sweepfrqunlock = 0;
            if(GrdStruct.SweepCyccnt >= 6)
            {
                GrdStruct.SweepCyccnt = 0;
                GrdState.Endsweep = ENABLE;
            }
            else
            {
                GrdStruct.SweepCyccnt++;
            }
        }
        
        BB_set_sweepfrq(GrdStruct.Sweepfrqunlock );
        Grd_Itfrq_Sort(MAX_RC_FRQ_SIZE);
    }
}


void Grd_Sweeping_After_Fec_Locked(void)
{
    BB_set_sweepfrq(CAOF[GrdStruct.Sweepfrqlock].frqchannel);
    Grd_Sweeping_Energy_Statistic(CAOF[GrdStruct.Sweepfrqlock].frqchannel);
    CAOF[GrdStruct.Sweepfrqlock].frqchannel = SP[CAOF[GrdStruct.Sweepfrqlock].frqchannel].frqchannel;
    CAOF[GrdStruct.Sweepfrqlock].poweravr = SP[CAOF[GrdStruct.Sweepfrqlock].frqchannel].poweravr;
    CAOF[GrdStruct.Sweepfrqlock].powerwave = SP[CAOF[GrdStruct.Sweepfrqlock].frqchannel].powerwave;
    Grd_Alterfrq_Updute(MAX_RC_FRQ_SIZE);

    GrdStruct.Sweepfrqlock += 1;
    
    if(GrdStruct.Sweepfrqlock >= MAX_RC_FRQ_SIZE)
    {
        GrdStruct.Sweepfrqlock = 0;
    }
}

/**
  * @brief  Get the best working frq channel according to sweeping results.
  * @param  iflag: the diff case of getting image transmissions frq channel.
    (1) get the diff image frq channel to lock.
    (2) get the first image transmission frq channel to FEC lock.
    (3) image transmissions frq channel hopping.
  */
void Grd_Get_Itfrq(uint8_t iflag)
{
    uint8_t icur=0;
    uint8_t icnt=0;
    uint8_t icur2=0;
    uint8_t  num_temp=0;
    uint32_t aver_temp=0;
    uint32_t wa_temp=0;
    
    GrdStruct.ITTxCnt = 1;
    GrdState.Allowjglock=DISABLE;

    switch (iflag)
    {
        case 0 :
            {
                for(icnt=0; icnt<MAX_RC_FRQ_SIZE; icnt++)
                {
                    if(GrdStruct.ITTxChannel != FSO[icnt].frqchannel)
                    {
                        GrdStruct.ITTxChannel = FSO[icnt].frqchannel;
                        for(icur=0;icur<MAX_RC_FRQ_SIZE;icur++)
                        {
                            CAOF[icur].frqchannel = FSO[icur].frqchannel;
                            CAOF[icur].poweravr = FSO[icur].poweravr;
                            CAOF[icur].powerwave = FSO[icur].powerwave;
                        }
                    }
                }
                printf("Ch0:%d\r\n", GrdStruct.ITTxChannel);
            } 
            break;

        case 1 :
            {
                GrdStruct.ITTxChannel = FSO[0].frqchannel;
                printf("Ch1:%d\r\n", GrdStruct.ITTxChannel);
                
                for(icur=0;icur<MAX_RC_FRQ_SIZE;icur++)
                {
                    CAOF[icur].frqchannel=FSO[icur].frqchannel;
                    CAOF[icur].poweravr=FSO[icur].poweravr;
                    CAOF[icur].powerwave=FSO[icur].powerwave;
                }
            }
            break;
            
        case 2 :
            {
                GrdStruct.ITTxChannel= CAOF[1].frqchannel;                
                printf("Ch2:%d\n", GrdStruct.ITTxChannel);

                num_temp    = CAOF[0].frqchannel;
                aver_temp   = CAOF[0].poweravr;
                wa_temp     = CAOF[0].powerwave;

                for(icur2=0;icur2<MAX_RC_FRQ_SIZE-1;icur2++)
                {
                    CAOF[icur2].frqchannel  = CAOF[icur2+1].frqchannel;
                    CAOF[icur2].poweravr    = CAOF[icur2+1].poweravr;
                    CAOF[icur2].powerwave   = CAOF[icur2+1].powerwave;
                }

                CAOF[MAX_RC_FRQ_SIZE-1].frqchannel   = num_temp;
                CAOF[MAX_RC_FRQ_SIZE-1].poweravr     = aver_temp;
                CAOF[MAX_RC_FRQ_SIZE-1].powerwave    = wa_temp;
            } 
            break;
    }
}


void Grd_Fecunlock_Getfrq(void)
{
    GrdStruct.ITunlkcnt ++;
    if(GrdStruct.ITunlkcnt >= SPAN_ITUNLK)
    {
        GrdStruct.ITunlkcnt = 0;
        if(ENABLE== GrdState.FEClock)
        {
            Grd_Get_Itfrq(2);
        }
        else
        {
            Grd_Get_Itfrq(0);
        }
    }
}


/**
  * @brief  Build the struct of SNR array.
  * @note   SNR array :
            (+) get the SNR value per 8 times in one cycle.
            (
+) get the SNR array of frq in 4*14ms.
            (+) get the SNR array of QAM in 32*14ms.
  *
  */

#define BB_SNR_REG_0    (0xc0)
#define BB_SNR_REG_1    (0xc1)

void Grd_Getsnr(uint8_t i)  //get SNR value at present
{
    static int pre_snr = 0xffffffff;
    SNRPC[i].num= i;

    SNRPC[i].snrhgh= BB_ReadReg(PAGE2, BB_SNR_REG_0);
    SNRPC[i].snrlow= BB_ReadReg(PAGE2, BB_SNR_REG_1);
      
    SNRPC[i].snrall= (SNRPC[i].snrhgh<<8)|SNRPC[i].snrlow;
}

void Grd_Frqsnr_Array(void)
{
    uint8_t inum=0;
    uint8_t irows=0;
    uint8_t jlists=0;
    uint8_t inum_2=0;

    for(irows=0;irows<FRQ_SNR_BLOCK_ROWS-1;irows++)
    {
        for(jlists=0;jlists<FRQ_SNR_BLOCK_LISTS;jlists++)
        {
            Snr_Frq_Block[irows][jlists]=Snr_Frq_Block[irows+1][jlists];
        }
    }
    for(inum=0;inum<FRQ_SNR_BLOCK_LISTS-1;inum++)
    {
        Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][inum]=SNRPC[inum].snrall;
    }

    for(inum_2=0;inum_2<FRQ_SNR_BLOCK_LISTS-1;inum_2++)
    {
        Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][FRQ_SNR_BLOCK_LISTS-1]+=Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][inum_2];
    }

    Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][FRQ_SNR_BLOCK_LISTS-1]=Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][FRQ_SNR_BLOCK_LISTS-1]>>3;

    Txosd_Buffer[13]=((Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][FRQ_SNR_BLOCK_LISTS-1]&0xFF00)>>8);
    Txosd_Buffer[14]=Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][FRQ_SNR_BLOCK_LISTS-1];
}

void Grd_Qamsnr_Array(void)
{
    uint8_t  inum=0;
    uint8_t  inum2=0;
    uint8_t  irows=0;
    uint8_t  jlists=0;

    for(irows=0;irows<QAM_SNR_BLOCK_ROWS-1;irows++)
    {
        for(jlists=0;jlists<QAM_SNR_BLOCK_LISTS;jlists++)
        {
            Snr_Qam_Block[irows][jlists]=Snr_Qam_Block[irows+1][jlists];
        }
    }
    for(inum=0;inum<QAM_SNR_BLOCK_LISTS-1;inum++)     //the last data
    {
        Snr_Qam_Block[QAM_SNR_BLOCK_ROWS-1][inum]=SNRPC[inum].snrall;
    }
    for(inum2=0;inum2<QAM_SNR_BLOCK_LISTS-1;inum2++)    //Sum
    {
        Snr_Qam_Block[QAM_SNR_BLOCK_ROWS-1][QAM_SNR_BLOCK_LISTS-1] += Snr_Qam_Block[QAM_SNR_BLOCK_ROWS-1][inum2];
    }

    Snr_Qam_Block[QAM_SNR_BLOCK_ROWS-1][QAM_SNR_BLOCK_LISTS-1]= Snr_Qam_Block[QAM_SNR_BLOCK_ROWS-1][QAM_SNR_BLOCK_LISTS-1]>>3;
}

/**
  * @brief  Determine SNR array of ground.
  * @note   Snr_Frq_Block:(4x9)
           ____________________________
           |__|__|__|__|__|__|__|__|__|
           |__|__|__|__|__|__|__|__|__|
           |__|__|__|__|__|__|__|__|__|
           |__|__|__|__|__|__|__|__|__|
           Require:  one of the block is smaller than threshold and the lists is all
                     smaller than threshold, the number >=2.
  *
  */
void Grd_Frq_Snrblock_Determine(uint16_t iMCS)
{
    uint8_t jrows=0;
    uint8_t jlist=0;
    uint8_t snr_list_cnt=0;
    uint8_t snr_rows_cnt=0;

    for(jlist=0;jlist<FRQ_SNR_BLOCK_LISTS-1;jlist++)
    {
        if(Snr_Frq_Block[0][jlist] < iMCS)
        {
            for(jrows=0;jrows<FRQ_SNR_BLOCK_ROWS;jrows++)
            {
                if(Snr_Frq_Block[jrows][jlist]<iMCS)
                    snr_rows_cnt++;
            }
            if(FRQ_SNR_BLOCK_ROWS == snr_rows_cnt)
            {
                snr_list_cnt++;
            }
         }
    }
    if(snr_list_cnt>=2)
    {
        GrdState.RegetITfrq=ENABLE;
    }
}


/**
  * @brief   Calculation the fluctuate and average of sweeping power.
  * @mothod  The alter frq channel greater than or equal to the working frq 3dB and the
             fluctuate is smallest.
  *
  */
uint8_t Grd_Sweeppower_Fluctuate_Average(void)     //??????????
{
    uint8_t iwa=0;
    uint8_t wave_cnt=0;

    if(CAOF[0].poweravr >= 2*CAOF[1].poweravr)
    {
        for(iwa=0;iwa<8 ;iwa++)
        {
            if( CAOF[1].powerwave <= CAOF[iwa].powerwave)
                wave_cnt++;
        }
    }
   
    if(wave_cnt >= 7) 
        return 1;
    else 
        return 0;
}


#define LDPC_ERR_LOW_REG    (0xdf)
#define LDPC_ERR_HIGH_REG   (0xde)

void Grd_Ldpc_Err_Num_Statistics(void)    //2 // 2 sec
{
    uint8_t   cpage=0;
    uint8_t   ldpc_cnt=0;
    uint16_t  ldpc_err_num_rd_low=0;
    uint16_t  ldpc_err_num_rd_hgh=0;

    for(ldpc_cnt=0;ldpc_cnt<LDPC_STATIC_NUM-1;ldpc_cnt++)
    {
        Ldpc_Block[ldpc_cnt] = Ldpc_Block[ldpc_cnt+1];
    }
        
    ldpc_err_num_rd_low =  BB_ReadReg(PAGE2, LDPC_ERR_LOW_REG);
    ldpc_err_num_rd_hgh =  BB_ReadReg(PAGE2, LDPC_ERR_HIGH_REG);
    
    Ldpc_Block[LDPC_STATIC_NUM-1] = (ldpc_err_num_rd_hgh<<8)|ldpc_err_num_rd_low;

    GrdStruct.Ldpcreadcnt++;
    if(GrdStruct.Ldpcreadcnt > LDPC_STATIC_NUM)
    {
        GrdState.Ldpcjgflag=ENABLE; //if count more than LDPC_STATIC_NUM times
    }
    else
    {
        GrdState.Ldpcjgflag=DISABLE;
    }
}


uint8_t Grd_Ldpc_Block_Determine(void)
{
    uint8_t arr_cnt=0;
    uint8_t ldpc_err_num=0;

    if(ENABLE == GrdState.Ldpcjgflag)
    {
        for(arr_cnt=0;arr_cnt<LDPC_STATIC_NUM-1;arr_cnt++)
        {
            if(Ldpc_Block[arr_cnt]>0x10)  
            {
                ldpc_err_num++;
            }
        }
        
        if(ldpc_err_num > 0)
        {
            ldpc_err_num=0;
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 0;
    }
}


void Baseband_MsgOSD_Ptf(void)
{
#if 0
    uint8_t iData_page_temp;
    
    Txosd_Buffer[0]=0x55;
    Txosd_Buffer[1]=0xaa;
    
    //  Txosd_Buffer[7]= 0x00;
    iData_page_temp= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);   //page2
    Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp& PAGE_2);

    Txosd_Buffer[8]/*iData_C3_temp */= Spi_Baseband_ReadWrite(spiRead, AAGC_0_RD, 0x00);
    Txosd_Buffer[9]/*iData_C7_temp*/ = Spi_Baseband_ReadWrite(spiRead, AAGC_4_RD, 0x00);
    Txosd_Buffer[10]/*iData_C4_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_1_RD, 0x00);
    Txosd_Buffer[4]/*iData_C5_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_2_RD, 0x00);
    Txosd_Buffer[5]/*iData_C6_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_3_RD, 0x00);
    
    Txosd_Buffer[6]/*iData_Eb_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_6_RD , 0x00);
    Txosd_Buffer[11]/*iData_E0_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_0_RD , 0x00);
    
    Txosd_Buffer[13]/*iData_SNRH_temp*/ = Spi_Baseband_ReadWrite(spiRead, CE_9_RD, 0x00);
    Txosd_Buffer[14]/*iData_SNRL_temp*/ = Spi_Baseband_ReadWrite(spiRead, CE_A_RD, 0x00);
    Txosd_Buffer[12]/*iData_EA_temp*/ = Spi_Baseband_ReadWrite(spiRead, FEC_5_RD, 0x00);
    Txosd_Buffer[15]/*iData_E7_temp*/ = Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_1, 0x00);
    Txosd_Buffer[16]/*iData_E8_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_2, 0x00);
    Txosd_Buffer[17]/*iData_EE_temp*/ = Spi_Baseband_ReadWrite(spiRead, FEC_9_RD, 0x00);
    Txosd_Buffer[18]/*iData_EF_temp*/ = Spi_Baseband_ReadWrite(spiRead, FEC_10_RD, 0x00);
    Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp);

    Txosd_Buffer[19]=0;
    //HAL_UART_Transmit(&Uart1Handle, (u8 *)&Txosd_Buffer, 20, 0xFFFF);
#endif
}

uint8_t Grd_get_QAM(void)
{
    return BB_ReadReg(PAGE2, GRD_FEC_QAM_CR_TLV) & 0x03;
}

void Grd_Itfrq_Hopping(void)
{
    uint8_t iData_harq  = 0;
    uint8_t iData_qam   = 0;
    uint8_t QAM_MODE    = 0;

    GrdStruct.CgITfrqspan++;
    if(GrdStruct.CgITfrqspan> 200)
    {
        GrdStruct.CgITfrqspan = 10;
    }

    GrdStruct.Harqcnt = BB_ReadReg(PAGE2, FEC_5_RD) >>4;
    QAM_MODE = Grd_get_QAM();

    #if 0
    if(GrdStruct.Harqcnt >= 2 )
    {
        printf("Hrq%d\r\n", GrdStruct.Harqcnt);
        GrdStruct.Harqcnt=0;
        switch(QAM_MODE)
        {
            case 0:  
            {                                //   BPSK_90
                if( GrdStruct.CgITfrqspan>=SPAN_ALTFRQ )
                {
                    Grd_Frq_Snrblock_Determine(BPSK1_2);
                    if(ENABLE == GrdState.RegetITfrq)
                    {
                        if(Grd_Sweeppower_Fluctuate_Average())
                        {
                            Grd_Get_Itfrq(2);
                            GrdStruct.CgITfrqspan=0;
                        }
                    }
                }
            }   
            break;
            
            case 1:  
            {                                //   QPSK_B0 (1/2)
                if( GrdStruct.CgITfrqspan>=SPAN_ALTFRQ )
                {
                    Grd_Frq_Snrblock_Determine(QPSK1_2);
                    if(ENABLE ==GrdState.RegetITfrq)
                    {
                        GrdState.RegetITfrq = DISABLE;
                        if(Grd_Sweeppower_Fluctuate_Average())
                        {
                            Grd_Get_Itfrq(2);
                            GrdStruct.CgITfrqspan=0;
                        }
                    }
                }
            }
            break;
            
            case 2:
            {                                //   16QAM (1/2)
                if( GrdStruct.CgITfrqspan>=SPAN_ALTFRQ )
                {
                    Grd_Frq_Snrblock_Determine(QAM16_1_2);
                    if(ENABLE ==GrdState.RegetITfrq)
                    {
                        if(Grd_Sweeppower_Fluctuate_Average())
                        {
                            Grd_Get_Itfrq(2);
                            GrdStruct.CgITfrqspan=0;
                        }
                    }
                }
            }
            break;
            
            case 3:  
            {                                //   64QAM (1/2)
                if( GrdStruct.CgITfrqspan>=SPAN_ALTFRQ )
                {
                    Grd_Frq_Snrblock_Determine(QAM64_1_2);
                    if(ENABLE ==GrdState.RegetITfrq)
                    {
                        if(Grd_Sweeppower_Fluctuate_Average())
                        {
                            Grd_Get_Itfrq(3);
                            GrdStruct.CgITfrqspan=0;
                        }
                    }
                }
            }
            break;
        }    
    }
    #endif
}

void Grd_Working_Qam_Change(void)
{
    uint8_t  icnt = 0;
    uint8_t  snr_cnt=0;
    uint32_t snr_aver = 0;
    uint8_t  iQAM=0;

    iQAM = Grd_get_QAM();
    switch (iQAM)
    {
        case 0:
        {
            for(icnt=0; icnt < SNRNUM_PER_CYC; icnt++)
            {
                if(SNRPC[icnt].snrall> 2*MCS3_0_THRE)       //BPSK  1/2  +3dB
                {
                    SNRPC[icnt].snrall = 2* MCS3_0_THRE;
                }
            }
            Grd_Qamsnr_Array();
            for(snr_cnt=0;snr_cnt<QAM_SNR_BLOCK_ROWS;snr_cnt++)
            {
                snr_aver = snr_aver + Snr_Qam_Block[snr_cnt][QAM_SNR_BLOCK_LISTS-1];
            }
            snr_aver = snr_aver>>5;
            if(snr_aver > MCS3_1_THRE)
            {
                Grd_Ldpc_Err_Num_Statistics();
            }
            else
            {
                GrdStruct.Ldpcreadcnt=0;
            }
            if(Grd_Ldpc_Block_Determine())
            {
                GrdQam.Upqpsk12= ENABLE;
                GrdStruct.Ldpcreadcnt=0;
            }
        } 
        break;

    case 1:                       // 4QAM 1/2 +3dB
        {
            for(icnt=0;icnt<SNRNUM_PER_CYC ;icnt++)
            {
                if(SNRPC[icnt].snrall >2* MCS3_1_THRE)
                {
                    SNRPC[icnt].snrall = 2* MCS3_1_THRE;
                }
            }
            Grd_Qamsnr_Array();
            for(snr_cnt=0;snr_cnt<QAM_SNR_BLOCK_ROWS;snr_cnt++)
            {
                snr_aver = snr_aver + Snr_Qam_Block[snr_cnt][QAM_SNR_BLOCK_LISTS-1];
            }
            snr_aver=snr_aver>>5;
            if(snr_aver > MCS3_3_THRE)
            {
                Grd_Ldpc_Err_Num_Statistics();
            }
            if (snr_aver < MCS3_1_THRE)
            {
                // GrdQam.Downbpsk12= ENABLE;
                GrdStruct.Ldpcreadcnt=0;
            }
            if(Grd_Ldpc_Block_Determine())
            {
                GrdQam.Up16qam12= ENABLE;
                GrdStruct.Ldpcreadcnt=0;
            }
        } 
        break;

    case 2:  //16QAM 1/2  +3dB
        {
            for(icnt=0;icnt<SNRNUM_PER_CYC ;icnt++)
            {
                if(SNRPC[icnt].snrall >2* MCS3_3_THRE)
                {
                    SNRPC[icnt].snrall =2* MCS3_3_THRE;
                }
            }
            Grd_Qamsnr_Array();
            for(snr_cnt=0;snr_cnt<QAM_SNR_BLOCK_ROWS;snr_cnt++)
            {
                snr_aver = snr_aver + Snr_Qam_Block[snr_cnt][QAM_SNR_BLOCK_LISTS-1];
            }
            snr_aver=snr_aver>>5;
            if(snr_aver > MCS3_4_THRE)
            {
                Grd_Ldpc_Err_Num_Statistics();
            }
            else if(snr_aver < MCS3_3_THRE)
            {
                GrdStruct.Ldpcreadcnt=0;
                if(snr_aver <= MCS3_1_THRE) 
                    {}// GrdQam.Downbpsk12= ENABLE;
                else 
                    GrdQam.Downqpsk12 = ENABLE;
            }
            if(Grd_Ldpc_Block_Determine())
            {
                GrdQam.Up64qam12 = ENABLE;
                GrdStruct.Ldpcreadcnt = 0;
            }
        } 
        break;

    case 3:                         //64 QAM 1/2  +3dB
        {
            for(icnt=0;icnt<SNRNUM_PER_CYC ;icnt++)
            {
                if(SNRPC[icnt].snrall >2* MCS3_4_THRE)
                {
                    SNRPC[icnt].snrall = 2*MCS3_4_THRE;
                }
            }
            Grd_Qamsnr_Array();
            for(snr_cnt=0;snr_cnt<QAM_SNR_BLOCK_ROWS;snr_cnt++)
            {
                snr_aver = snr_aver + Snr_Qam_Block[snr_cnt][8];
            }
            snr_aver = snr_aver >> 5;
            if(snr_aver <= MCS3_4_THRE)
            {
                if(snr_aver <= MCS3_3_THRE)
                {
                    if(snr_aver <= MCS3_1_THRE)
                    {
                        //GrdQam.Downbpsk12=ENABLE;
                    }
                    else
                    {
                      GrdQam.Downqpsk12=ENABLE;
                    }
                }
                else
                {
                    GrdQam.Down16qam12=ENABLE;
                }
            }
        } 
        break;
    }
}


void Grd_Txmsg_Qam_Change(void)
{
    if( ENABLE==GrdQam.Downbpsk12)  // BPSK_1/2  0x90
    {       
        BB_WriteReg(PAGE2, QAM_CHANGE_0, 0xF1);        
        BB_WriteReg(PAGE2, QAM_CHANGE_1, 0xF1);        
    }

    if(ENABLE==GrdQam.Downqpsk12)   //QPSK_1/2   0xB0
    {       
        BB_WriteReg(PAGE2, QAM_CHANGE_0, 0xF3);       
        BB_WriteReg(PAGE2, QAM_CHANGE_1, 0xF3);       
    }

    if(ENABLE==GrdQam.Down16qam12)  //16QAM_1/2    0xD0
    {
        BB_WriteReg(PAGE2, QAM_CHANGE_0, 0xF7);
        BB_WriteReg(PAGE2, QAM_CHANGE_1, 0xF7);
    }

    //MCS_UP
    if(ENABLE==GrdQam.Upqpsk12) //QPSK_1/2   0xB0
    {      
        BB_WriteReg(PAGE2, QAM_CHANGE_0, 0xF3);
        BB_WriteReg(PAGE2, QAM_CHANGE_1, 0xF3);
    }

    if(ENABLE==GrdQam.Up16qam12)  //16QAM_1/2    0xD0
    {      
        BB_WriteReg(PAGE2, QAM_CHANGE_0, 0xF7);
        BB_WriteReg(PAGE2, QAM_CHANGE_1, 0xF7);
    }
    if(ENABLE==GrdQam.Up64qam12)  //64QAM_1/2    0xF0
    {
        BB_WriteReg(PAGE2, QAM_CHANGE_0, 0xF9);
        BB_WriteReg(PAGE2, QAM_CHANGE_1, 0xF9);
    }
}


void Grd_Qamflag_Clear(void)
{
    GrdQam.Up16qam12    = DISABLE;
    GrdQam.Up64qam12    = DISABLE;
    GrdQam.Upqpsk12     = DISABLE;
    GrdQam.Down16qam12  = DISABLE;
    GrdQam.Downbpsk12   = DISABLE;
    GrdQam.Downqpsk12   = DISABLE;
}


void Grd_IT_Controller(void)
{
    if(DISABLE == GrdState.FEClock)
    {
        Grd_Sweeping_Before_Fec_Locked();
    }
    else
    {
        Grd_Sweeping_After_Fec_Locked();
    }

    if(ENABLE == GrdState.Endsweep)
    {
        if(ENABLE == GrdState.GetfrqOnly)
        {
            Grd_Get_Itfrq(1);   //when system bootup and end of sweep, select 1 channel for IT.
            GrdState.GetfrqOnly = DISABLE;
        }

        switch(GrdStruct.ITTxCnt) //GrdStruct.ITTxCnt: set to 1 when call Grd_Get_Itfrq.
        {
            case 1:
                GrdStruct.ITTxCnt=2;
                break;

            case 2:
                GrdStruct.ITTxCnt = 0;
                GrdState.Allowjglock = ENABLE;
                Grd_Write_Itfrq(GrdStruct.ITTxChannel);
                Txosd_Buffer[3]= GrdStruct.ITTxChannel;
                break;

            default:
                GrdStruct.ITTxCnt = 0;
                break;
        }
    }
    
    if(ENABLE == GrdState.Allowjglock) //after ch switch, check the lock
    {
        if(Grd_Baseband_Fec_Lock())
        {
            GrdState.FEClock=ENABLE;
            GrdStruct.ITunlkcnt=0;

            if( ENABLE == GrdState.ITManualmode)
            {
                GrdState.ITAutomode = DISABLE;
            }
            else if( ENABLE == GrdState.ITAutomode)
            {
                GrdState.ITManualmode = DISABLE;
                Grd_Itfrq_Hopping();
                Grd_Working_Qam_Change();
                Grd_Txmsg_Qam_Change();
                Grd_Qamflag_Clear();
            }
        }
        else
        {
            Grd_Fecunlock_Getfrq();
        }
    }
}

void Grd_Osdmsg_Ptf(void)
{
    #if 0
    uint8_t iData_page_temp=0;

    Txosd_Buffer[0]=0x55;
    Txosd_Buffer[1]=0xaa;
    iData_page_temp= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);   //page2
    Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp& PAGE_2);
    Txosd_Buffer[6]/*iData_EB_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_6_RD, 0x00);
    Txosd_Buffer[11]/*iData_E0_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_0_RD, 0x00);
    Txosd_Buffer[4]/*iData_C5_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_2_RD, 0x00);
    Txosd_Buffer[5]/*iData_C6_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_3_RD, 0x00);
    Txosd_Buffer[15]/*iData_E7_temp*/ = Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_1, 0x00);
    Txosd_Buffer[16]/*iData_E8_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_2, 0x00);
    Txosd_Buffer[17]/*iData_EE_temp */= Spi_Baseband_ReadWrite(spiRead, FEC_9_RD, 0x00);
    Txosd_Buffer[18]/*iData_EF_temp */= Spi_Baseband_ReadWrite(spiRead, FEC_10_RD, 0x00);
    Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp);

    Txosd_Buffer[19]=0;
    SysState.TxcyosdOnly=ENABLE;
    #endif
}


void wimax_vsoc_tx_isr(void)
{
    INTR_NVIC_DisableIRQ(BB_TX_ENABLE_VECTOR_NUM);
    TIM_StartTimer(init_timer0_0);
    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM);
}


void Grd_TIM0_IRQHandler(void)
{
    Reg_Read32(BASE_ADDR_TIMER0 + TMRNEOI_0);

    //Enable BB_TX intr
    INTR_NVIC_ClearPendingIRQ(BB_TX_ENABLE_VECTOR_NUM); //clear pending after TX Enable is LOW. MUST
    INTR_NVIC_EnableIRQ(BB_TX_ENABLE_VECTOR_NUM);

    //Disable TIM0 intr
    INTR_NVIC_DisableIRQ(TIMER_INTR00_VECTOR_NUM);
    TIM_StopTimer(init_timer0_0);
    
    //Enable TIM1 intr
    TIM_StartTimer(init_timer0_1);
    INTR_NVIC_EnableIRQ(TIMER_INTR01_VECTOR_NUM);   
}


void Grd_TIM1_IRQHandler(void)
{
    Reg_Read32(BASE_ADDR_TIMER0 + TMRNEOI_1);
    switch (Timer1_Delay1_Cnt)
    {
        case 0:
            Timer1_Delay1_Cnt++;
            Grd_Getsnr(0);
            break;
        case 1:
            Timer1_Delay1_Cnt++;
            Grd_Getsnr(1);
            break;
        case 2:
            Timer1_Delay1_Cnt++;
            Grd_RC_Controller();
            Grd_Getsnr(2);
            break;
        
        case 3:
            Timer1_Delay1_Cnt++;
            Grd_IT_Controller();                
            Grd_Getsnr(3);
            break;
        
        case 4:
            Timer1_Delay1_Cnt++;
            Grd_Getsnr(4);
            break;
            
        case 5:
            Timer1_Delay1_Cnt++;
            Grd_Getsnr(5);
            break;
        
        case 6:
            Timer1_Delay1_Cnt++;
            Grd_Getsnr(6);
            break;
        case 7:
            INTR_NVIC_DisableIRQ(TIMER_INTR01_VECTOR_NUM);                
            TIM_StopTimer(init_timer0_1);
            
            Timer1_Delay1_Cnt = 0;
            Grd_Getsnr(7);
            Grd_Frqsnr_Array();
            break;
            
        default:
            Timer1_Delay1_Cnt = 0;
            dlog_error("delay error\n");
            break;
    }
}

void Grd_Timer1_Init(void)
{
    init_timer0_1.base_time_group = 0;
    init_timer0_1.time_num = 1;
    init_timer0_1.ctrl = 0;
    init_timer0_1.ctrl |= TIME_ENABLE | USER_DEFINED;
    TIM_RegisterTimer(init_timer0_1, 1200);
    reg_IrqHandle(TIMER_INTR01_VECTOR_NUM, Grd_TIM1_IRQHandler);
}

void Grd_Timer0_Init(void)
{
    init_timer0_0.base_time_group = 0;
    init_timer0_0.time_num = 0;
    init_timer0_0.ctrl = 0;
    init_timer0_0.ctrl |= TIME_ENABLE | USER_DEFINED;
    
    TIM_RegisterTimer(init_timer0_0, 3600);

    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, Grd_TIM0_IRQHandler);
}