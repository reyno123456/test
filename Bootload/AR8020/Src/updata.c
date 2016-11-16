#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "quad_spi_ctrl.h"
#include "nor_flash.h"
#include "debuglog.h"
#include "stm32f746xx.h"
#include "updata.h"
#include "interrupt.h"
#include "serial.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_host.h"
#include "systicks.h"
#include "command.h"

#define UPDATA_SD_DEBUGE           
#define UPDATA_USB_DEBUGE          
#define UPDATA_UART_DEBUGE         
#define UPDATA_DEBUGE              
#define UPDATA_DEBUGE_ERROR          

#define UPDATA_SD                  
#define UPDATA_USB                 
#define UART_UPDATA                

#ifdef UPDATA_DEBUGE
#define DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define DLOG_INFO(...)
#endif

#ifdef UPDATA_DEBUGE_ERROR
#define ERROR_DLOG_INFO(...) dlog_error(__VA_ARGS__)
#else
#define ERROR_DLOG_INFO(...)
#endif


#ifdef  UPDATA_SD_DEBUGE
#define UPDATA_SD
#define SD_DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define SD_DLOG_INFO(...)
#endif


#ifdef  UPDATA_USB_DEBUGE
#define UPDATA_USB
#define USB_DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define USB_DLOG_INFO(...)
#endif


#ifdef  UPDATA_UART_DEBUGE
#define UART_UPDATA
#define UART_DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define UART_DLOG_INFO(...)
#endif




#define BUFF_NUM                   (3)
#define RDWR_SECTOR_SIZE           (1024*4) //or (1024*32)
#define RDWR_SIZE                  (1024*32) //or (1024*32)
#define RDWR_BLOCK_SIZE            (1024*64) //or (1024*32)

#define APP_ADDR_OFFSET            (0x10000)

#define ITCM0_START                0x00000000
#define ITCM1_START                0x44100000
#define ITCM2_START                0xB0000000
#define MCU2_CPU_WAIT              0x40B000CC  /* ENABLE CPU1 */
#define MCU3_CPU_WAIT              0xA0030088  /* ENABLE CPU2 */
#define MCU0_VECTOR_TABLE_REG      0xE000ED08

#define COPY_ERASE_ADDR            0x0000042A
#define RUN_ERASE_ADDR             0x0000042D

#include "sd_host.h"

extern Diskio_drvTypeDef  SD_Driver;
void (* redirct_erase_funct)(uint32_t)=(void *)(RUN_ERASE_ADDR);
void UPDATA_EraseSector(uint32_t i)
{   
    uint32_t busy_flag = 0;
    volatile uint32_t* ptr_regAddr = NULL;
    //enable write
    *((volatile uint32_t*)0x40C00040)=0;
    //checkbuys
    do
    {
        ptr_regAddr =  (uint32_t*)0x40C00000;
        busy_flag = (*ptr_regAddr) & 0x1;
    }while(busy_flag);
    //erase
    *( (volatile uint32_t*)0x40C00090)=i;
    *( (volatile uint32_t*)0x40C00050)=0;
    //checkbuys
    do
    {  
        ptr_regAddr = (uint32_t*)0x40C00000;
        busy_flag = (*ptr_regAddr) & 0x1;
    }while(busy_flag);
    return ;
}
void UPDATA_NOR_FLASH_Erase(void)
{
    void *memfun=(void *)(COPY_ERASE_ADDR);    
    void (* funct2)(uint32_t)=UPDATA_EraseSector;
    memcpy(memfun, (funct2 -3), (uint32_t)(&UPDATA_NOR_FLASH_Erase) - (uint32_t)(&UPDATA_EraseSector) +3);

}
int8_t UPDATA_CopyDataToNor(FIL MyFile,uint32_t u32_addrOffset)
{
	FRESULT           fileResult;
    uint32_t          u32_bytesRead= RDWR_SECTOR_SIZE;
    uint8_t           u8_arrayRecData[RDWR_SECTOR_SIZE];
    uint32_t          u32_recDataSum = 0;
    uint32_t          u32_norAddr = u32_addrOffset;
    uint8_t rtext[100];
    int i=0;
    UPDATA_NOR_FLASH_Erase();
    DLOG_INFO("Nor flash init start ...");
    NOR_FLASH_Init();
    DLOG_INFO("Nor flash init end   ...");
    __asm volatile ("cpsid i");
    __asm volatile ("cpsid f");
    while(RDWR_SECTOR_SIZE == u32_bytesRead)
    {
        
        memset(u8_arrayRecData,0,RDWR_SECTOR_SIZE);
        fileResult = f_read(&MyFile, u8_arrayRecData, RDWR_SECTOR_SIZE, (void *)&u32_bytesRead);
        if((fileResult != FR_OK))
        {
            ERROR_DLOG_INFO("Cannot Read from the file \n");
            f_close(&MyFile);
            return UPDATA_READFILE;
        }
        else
        {
            DLOG_INFO("f_read success %d!",u32_bytesRead);
            u32_recDataSum+=u32_bytesRead;            
            DLOG_INFO("EraseSector start %x!",u32_norAddr);
            redirct_erase_funct(u32_norAddr);          
            NOR_FLASH_WriteByteBuffer(u32_norAddr, u8_arrayRecData, u32_bytesRead);  
            
            #if 1
//            #if (UPDATA_USB_DEBUGE | UPDATA_SD_DEBUGE)
            {
	            uint32_t u32_checkSumArray = 0;
	            uint32_t u32_checkSumNor =0;
	            uint32_t  u8_val =0;
	            uint32_t  i =0;
	            for(i=0;i<u32_bytesRead;i++)
	            {
	                u32_checkSumArray+=u8_arrayRecData[i];
	            }
	            for(i=0;i<u32_bytesRead;i++)
	            {
	                //QUAD_SPI_ReadByte(u32_norAddr+i, &u8_val);
	                u32_checkSumNor +=u8_val;
	            }
	            DLOG_INFO("number of rec data %d  u32_checkSumArray %d u32_checkSumNor %d\n ", u32_bytesRead,u32_checkSumArray,u32_checkSumNor);
            }
            #endif
            u32_norAddr += RDWR_SECTOR_SIZE;            
        }        
    }
    DLOG_INFO(" number of totol data %d\n ", u32_recDataSum);
    __asm volatile ("cpsie f");
    __asm volatile ("cpsie i");

    return UPDATA_SUCCESS;
}

int8_t UPDATA_CopyDataToTCM(FIL MyFile,uint32_t u32_TCMADDR)
{
	FRESULT           fileResult;
    uint32_t          u32_cpuAppSize = 0;
	uint32_t          u32_bytesRead= RDWR_SECTOR_SIZE;
    uint8_t           u8_arrayRecData[RDWR_SECTOR_SIZE];
	char *            dst = (char *)(0x21000000);
    uint32_t          u32_iCount = 0;
    uint32_t          u32_addr = u32_TCMADDR;
    uint32_t          u32_readCount = 0;
    uint32_t          i = 0;
    uint32_t u32_checkSumArray = 0;
	uint32_t u32_checkSumNor =0;
    uint32_t          tmp=0;
    
	#if 1
	while(RDWR_SECTOR_SIZE == u32_bytesRead)
    {
        
        memset(u8_arrayRecData,0,RDWR_SECTOR_SIZE);
        fileResult = f_read(&MyFile, u8_arrayRecData, RDWR_SECTOR_SIZE, (void *)&u32_bytesRead);
        if((fileResult != FR_OK))
        {
            ERROR_DLOG_INFO("Cannot Read from the file \n");
            f_close(&MyFile);
            return UPDATA_READFILE;
        }

        u32_checkSumArray = 0;
        u32_checkSumNor = 0;
        for(i=0;i<u32_bytesRead;i++)
        {
            u32_checkSumArray+=u8_arrayRecData[i];
        }
        DLOG_INFO("number of rec data %d  u32_checkSumArray %d u32_checkSumNor %d\n ", u32_bytesRead,u32_checkSumArray,u32_checkSumNor);
           
        if(0 == u32_iCount)
        {
        	u32_cpuAppSize = (u8_arrayRecData[0]) | (u8_arrayRecData[1] <<8) | (u8_arrayRecData[2] <<16) | (u8_arrayRecData[3] <<24);
			SD_DLOG_INFO("u32_cpuAppSize 0x%x icount %d", u32_cpuAppSize,u32_iCount);
        	memcpy((char *)(u32_addr), &u8_arrayRecData[4], RDWR_SECTOR_SIZE -4);
        	u32_addr =u32_addr + (RDWR_SECTOR_SIZE -4);
        	u32_cpuAppSize = u32_cpuAppSize - (RDWR_SECTOR_SIZE -4);
        	u32_readCount =u32_readCount + (RDWR_SECTOR_SIZE -4);
        	u32_iCount =1;
        	SD_DLOG_INFO("u32_cpuAppSize 0x%x icount %d", u32_cpuAppSize,u32_iCount);
        }
        else
        {
        	if(u32_cpuAppSize >= RDWR_SECTOR_SIZE)
			{
				u32_readCount += RDWR_SECTOR_SIZE;
				u32_cpuAppSize -= RDWR_SECTOR_SIZE;
				memcpy((char *)(u32_addr), u8_arrayRecData, RDWR_SECTOR_SIZE);
				u32_addr +=RDWR_SECTOR_SIZE;
				SD_DLOG_INFO("u32_cpuAppSize 0x%x u32_readCount %d", u32_cpuAppSize,u32_readCount);
			}
			else
			{
				u32_readCount += u32_cpuAppSize;
				tmp=u32_cpuAppSize;
				SD_DLOG_INFO("number of data 0x%x tmp %d\n", u32_readCount,tmp);
				u32_readCount =0;
				memcpy((char *)(u32_addr), u8_arrayRecData, u32_cpuAppSize);
				if(u32_bytesRead<RDWR_SECTOR_SIZE)
					break;
				if(2 == u32_iCount)
				{
					u32_addr = ITCM2_START;
					u32_iCount =2;
				}
				else
				{
					u32_addr = ITCM1_START;
					u32_iCount =2;
				}
				
				u32_cpuAppSize = (u8_arrayRecData[tmp]) | (u8_arrayRecData[tmp+1] <<8 )
				| (u8_arrayRecData[tmp+2] <<16) | (u8_arrayRecData[tmp+3] <<24);

				SD_DLOG_INFO("u32_cpuAppSize 0x%x, %x\n", u32_cpuAppSize,RDWR_SECTOR_SIZE - tmp -4);

				u32_readCount += (RDWR_SECTOR_SIZE - tmp -4);
				u32_cpuAppSize = u32_cpuAppSize - u32_readCount;
				memcpy((char *)(u32_addr), &u8_arrayRecData[tmp+4], RDWR_SECTOR_SIZE - tmp -4);				
				u32_addr += (RDWR_SECTOR_SIZE - tmp -4);
				SD_DLOG_INFO("u32_cpuAppSize 0x%x u32_readCount %x\n", u32_cpuAppSize,u32_readCount);								
			}
        }
        for(i=0;i<u32_bytesRead;i++)
        {
            u32_checkSumNor+=u8_arrayRecData[i];
        }
        DLOG_INFO("number of rec data %d  u32_checkSumArray %d u32_checkSumNor %d\n ", u32_bytesRead,u32_checkSumArray,u32_checkSumNor);
           
     
    }
    #endif
    #if 0
	memset(u8_arrayRecData,0,RDWR_SECTOR_SIZE);
    fileResult = f_read(&MyFile, u8_arrayRecData, 4, (void *)&u32_bytesRead);
    if((u32_bytesRead == 0) || (fileResult != FR_OK))
    {
        ERROR_DLOG_INFO("Cannot Read from the file \n");
        f_close(&MyFile);
        return UPDATA_OPENFILE;
    }
	u32_cpuAppSize = u8_arrayRecData[0] | u8_arrayRecData[1] <<8 | u8_arrayRecData[2] <<16 | u8_arrayRecData[3] <<24;
	SD_DLOG_INFO("u32_cpuAppSize 0x%x", u32_cpuAppSize);

	u32_readCount = RDWR_SECTOR_SIZE;
	u32_bytesRead = RDWR_SECTOR_SIZE; 		
	u32_iCount = 0; 

    while(RDWR_SECTOR_SIZE == u32_bytesRead)
	{
		memset(u8_arrayRecData,0,RDWR_SECTOR_SIZE);
		if(u32_cpuAppSize >= RDWR_SECTOR_SIZE)
		{
			u32_readCount = RDWR_SECTOR_SIZE;
			u32_cpuAppSize -= RDWR_SECTOR_SIZE;
		}
		else
		{
			u32_readCount = u32_cpuAppSize;
		}
		fileResult = f_read(&MyFile, u8_arrayRecData, u32_readCount, (void *)&u32_bytesRead);
		u32_checkSumArray = 0;
		for(i=0;i<u32_bytesRead;i++)
        {
            u32_checkSumArray+=u8_arrayRecData[i];
        }
        DLOG_INFO("number of rec data %d  u32_checkSumArray %d u32_checkSumNor %d\n ", u32_bytesRead,u32_checkSumArray);
	    if((u32_bytesRead == 0) || (fileResult != FR_OK) || (u32_readCount != u32_bytesRead))
	    {
	        ERROR_DLOG_INFO("Cannot Read from the file \n");
	        f_close(&MyFile);
	        return UPDATA_READFILE;
	    }
	    else
	    {
			SD_DLOG_INFO("copy addr %x  number of data %d\n",u32_addr,u32_bytesRead);
            memcpy((char *)(u32_addr), u8_arrayRecData, u32_bytesRead);
			u32_addr += RDWR_SECTOR_SIZE;
			u32_iCount += u32_bytesRead;
			#if 1
			{
	            uint32_t u32_checkSumArray = 0;
	            uint32_t u32_checkSumNor =0;
	            char  *u8_val =(char *)(u32_addr-RDWR_SECTOR_SIZE);

	            for(i=0;i<u32_bytesRead;i++)
	            {
	                u32_checkSumArray+=u8_arrayRecData[i];
	            }
	            for(i=0;i<u32_bytesRead;i++)
	            {
	                u32_checkSumNor +=*(u8_val+i);
	            }
	            DLOG_INFO("number of rec data %d  u32_checkSumArray %d u32_checkSumNor %d\n ", u32_bytesRead,u32_checkSumArray,u32_checkSumNor);
            }
            #endif					    		
		}
	    
	}
	SD_DLOG_INFO(" number of totol data %x\n ",u32_iCount);
	#endif	
	return UPDATA_SUCCESS;

}


#if defined (UPDATA_SD)

void UPDATA_UpdataFromSDToNor(void)
{    
    FRESULT fileResult;
    FIL MyFile;
FATFS SDFatFs;  /* File system object for SD card logical drive */
char SDPath[4]; /* SD card logical drive path */   
    SD_DLOG_INFO("UPDATA_UpdataFromSDToNor\n");
    if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
    {
        SD_DLOG_INFO("FATFS_LinkDriver error \n");
        return ;
    }    
    if ((fileResult = f_mount(&SDFatFs, (TCHAR const*)SDPath, 1)) != FR_OK)
    {

        SD_DLOG_INFO("f_mount = %d\n", fileResult);
        SD_DLOG_INFO("f_mount error!\n");
        return ;
    }
    SD_DLOG_INFO("SD mount ok \n");
    fileResult = f_open(&MyFile, "ar8020.bin", FA_READ);
    if (FR_OK != fileResult)
    {
        ERROR_DLOG_INFO("open or create file error: %d\n", fileResult);
        return ;
    }
    if(UPDATA_CopyDataToNor(MyFile,APP_ADDR_OFFSET) != 0)
    {
    	return;    	
    }	

    f_close(&MyFile);
    fileResult = FATFS_UnLinkDriver(SDPath);
    return;

}
void UPDATA_BootFromSD(void)
{
    FRESULT           fileResult;  
    FIL MyFile;
FATFS SDFatFs;  /* File system object for SD card logical drive */
char SDPath[4]; /* SD card logical drive path */
    SD_DLOG_INFO("UPDATA_SDBoot\n");    
    if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
    {
        SD_DLOG_INFO("FATFS_LinkDriver error \n");
        return ;
    }    
    if ((fileResult = f_mount(&SDFatFs, (TCHAR const*)SDPath, 1)) != FR_OK)
    {

        SD_DLOG_INFO("f_mount = %d\n", fileResult);
        SD_DLOG_INFO("f_mount error!\n");
        return ;
    }
    SD_DLOG_INFO("SD mount ok \n");
    fileResult = f_open(&MyFile, "ar8020.bin", FA_READ);
    if (FR_OK != fileResult)
    {
        ERROR_DLOG_INFO("open or create file error: %d\n", fileResult);
        return ;
    }   
	if(UPDATA_CopyDataToTCM(MyFile,ITCM0_START) != 0)
    {
        return;     
    }  	 
   /* if(UPDATA_CopyDataToTCM(ITCM1_START) != 0)
    {
        return;     
    }
    if(UPDATA_CopyDataToTCM(ITCM2_START) != 0)
    {
        return;     
    }*/
	f_close(&MyFile);
    fileResult = FATFS_UnLinkDriver(SDPath);
    
    SysTicks_DelayMS(20);
    command_uninit();
    SysTicks_UnInit();
    SD_DLOG_INFO("Enable cpu1 ...");
    *((volatile uint32_t*)MCU2_CPU_WAIT) = 0;
    
    SD_DLOG_INFO("Enable cpu2 ...");
    *((volatile uint32_t*)MCU3_CPU_WAIT) = 0;
    SD_DLOG_INFO("Jump to cpu0 ...");
    dlog_output(200);
    *((uint32_t*)MCU0_VECTOR_TABLE_REG) = 0;
    (*((void(*)())((*((uint32_t*)(ITCM0_START+4))))))();     

    return;
}
void UPDATA_UpdataBootloaderFromSD(void)
{
    FRESULT fileResult;
FIL MyFile;
FATFS SDFatFs;  /* File system object for SD card logical drive */
char SDPath[4]; /* SD card logical drive path */   
    SD_DLOG_INFO("UPDATA_UpdataBootloaderFromSD\n");
    if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
    {
        SD_DLOG_INFO("FATFS_LinkDriver error \n");
        return ;
    }    
    if ((fileResult = f_mount(&SDFatFs, (TCHAR const*)SDPath, 1)) != FR_OK)
    {

        SD_DLOG_INFO("f_mount = %d\n", fileResult);
        SD_DLOG_INFO("f_mount error!\n");
        return ;
    }
    SD_DLOG_INFO("SD mount ok \n");
    fileResult = f_open(&MyFile, "ar8020.bin", FA_READ);
    if (FR_OK != fileResult)
    {
        ERROR_DLOG_INFO("open or create file error: %d\n", fileResult);
        return ;
    }
    if(UPDATA_CopyDataToNor(MyFile,0) != 0)
    {
        return;     
    }   

    f_close(&MyFile);
    fileResult = FATFS_UnLinkDriver(SDPath);
    return;

}
#endif

#if defined (UART_UPDATA)
#include "timer.h"

#define BASEADR 0x2007FFFF

char* g_pDst  = (char *)BASEADR;
init_timer_st g_stTimer;
volatile uint32_t g_u32TimCountUp = 0;
volatile uint32_t u32_gRecCount = 0;

void UPDATA_TimerIRQHandler(void)
{
    TIM_ClearNvic(g_stTimer);  
    g_u32TimCountUp ++;
}

void UPDATA_IRQHandler(void)
{
    uint32_t          u32_isrType;
    uint32_t          u32_status;
    volatile uart_type   *uart_regs =(uart_type *)UART4_BASE;
    u32_status     = uart_regs->LSR;
    u32_isrType    = uart_regs->IIR_FCR;
    if (UART_IIR_RECEIVEDATA == (u32_isrType & UART_IIR_RECEIVEDATA))
    {
        if ((u32_status & UART_LSR_DATAREADY) == UART_LSR_DATAREADY)
        {
            *g_pDst = uart_regs->RBR_THR_DLL;
            g_pDst++;
            u32_gRecCount++;
            g_u32TimCountUp=0;

        }
    }
}

void UPDATA_TimerStart(void)
{
    g_stTimer.base_time_group = 0;
    g_stTimer.time_num = 0;
    g_stTimer.ctrl |= TIME_ENABLE | USER_DEFINED;                     
    g_u32TimCountUp = 0;
    TIM_RegisterTimer(g_stTimer, 1000*1000/TIM_CLC_MHZ*32);  
    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, UPDATA_TimerIRQHandler);
    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM);
    TIM_StartTimer(g_stTimer);
}

void UPDATA_UartInit(void)
{
    uart_init(4, 360000);
    reg_IrqHandle(UART_INTR4_VECTOR_NUM, UPDATA_IRQHandler);
    INTR_NVIC_EnableIRQ(UART_INTR4_VECTOR_NUM);
}

void UPDATA_UpdataBootloaderFromUart(void)
{

    memset(&g_stTimer, 0, sizeof(init_timer_st));
    UART_DLOG_INFO("Nor flash init start ...");
    NOR_FLASH_Init();
    NOR_FLASH_EraseBlock(BASEADR);
    UART_DLOG_INFO("Nor flash EraseBlock end");
    
    UPDATA_UartInit();
    UPDATA_TimerStart();        
    while(g_u32TimCountUp<3)
    {
              ;
    }
    UART_DLOG_INFO("uart rec data %d\n",u32_gRecCount);
    __asm volatile ("cpsid i");
    NOR_FLASH_WriteByteBuffer(0, (void *)BASEADR, RDWR_BLOCK_SIZE);
    __asm volatile ("cpsie i");
    #if 1
//    #if (UPDATA_UART_DEBUGE)
    {
        uint32_t u32_checkSumArray = 0;
        uint32_t u32_checkSumNor =0;
        uint8_t  u8_val =0;
        uint8_t  i =0;
        g_pDst  = (char *)BASEADR;
        for(i=0;i<u32_gRecCount;i++)
        {
            u32_checkSumArray+=*g_pDst;
            g_pDst++;
        }
        for(i=0;i<u32_gRecCount;i++)
        {
            QUAD_SPI_ReadByte((i), &u8_val);
            u32_checkSumNor +=u8_val;
        }
        DLOG_INFO("number of rec data %d  u32_checkSumArray %d u32_checkSumNor %d\n ", u32_gRecCount,u32_checkSumArray,u32_checkSumNor);
    }
    #endif
}
#endif
#if 0
#if defined (UPDATA_USB)
#include "usbh.h"
USBH_HandleTypeDef   hUSBHost;
FATFS                USBH_fatfs;
void UPDATA_UpdataFromUsbToNor(void)
{
    FRESULT fileResult;
    FIL     MyFile;
    usbh();
    USB_DLOG_INFO("memory stick ok \n");

    fileResult = f_open(&MyFile, "ar8020.bin", FA_READ);
    if (FR_OK != fileResult)
    {
        ERROR_DLOG_INFO("open or create file error: %d\n", fileResult);
        return;
    }

    if(UPDATA_CopyDataToNor(MyFile,APP_ADDR_OFFSET) != 0)
    {
    	return;    	
    }	
    f_close(&MyFile);
    return;
}
void UPDATA_BootFromUSB(void)
{
    FRESULT           fileResult;
    FIL               MyFile;    
    
    USB_DLOG_INFO("UPDATA_USBBoot\n");

    fileResult = f_open(&MyFile, "ar8020.bin", FA_READ);
    if (FR_OK != fileResult)
    {
        ERROR_DLOG_INFO("open or create file error: %d\n", fileResult);

        return;
    }
	if(UPDATA_CopyDataToTCM(MyFile,ITCM0_START) != 0)
    {
    	return;    	
    }
    if(UPDATA_CopyDataToTCM(MyFile,ITCM1_START) != 0)
    {
    	return;    	
    }
    if(UPDATA_CopyDataToTCM(MyFile,ITCM2_START) != 0)
    {
    	return;    	
    }
	f_close(&MyFile);

    USB_DLOG_INFO("Enable cpu1 ...");
    dlog_output(200);
    *((volatile uint32_t*)MCU2_CPU_WAIT) = 0;

    USB_DLOG_INFO("Enable cpu2 ...");
    dlog_output(200);
    *((volatile uint32_t*)MCU3_CPU_WAIT) = 0;

    USB_DLOG_INFO("Jump to cpu0 ...");
    dlog_output(200);
    *((uint32_t*)MCU0_VECTOR_TABLE_REG) = 0;
    (*((void(*)())((*((uint32_t*)(ITCM0_START+4))))))();     

    return;
}
void UPDATA_UpdataBootloaderFromUSB(void)
{
    FRESULT           fileResult;
    FIL               MyFile;        
    SD_DLOG_INFO("UPDATA_UpdataBootloaderFromSD\n");    
    fileResult = f_open(&MyFile, "ar8020_boot.bin", FA_READ);

    if (FR_OK != fileResult)
    {
        ERROR_DLOG_INFO("open or create file error: %d\n", fileResult);

        return;
    } 
    if(UPDATA_CopyDataToNor(MyFile,0) != 0)
    {
        return;     
    }   

    f_close(&MyFile);

}
#endif
#endif