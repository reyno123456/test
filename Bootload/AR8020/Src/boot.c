#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "quad_spi_ctrl.h"
#include "nor_flash.h"
#include "debuglog.h"
#include "systicks.h"
#include "boot.h"
#include "command.h"
#include "systicks.h"


//#define UPGRADE_DEBUGE           (1)
//#define UPGRADE_DEBUGE_ERROR

#ifdef UPGRADE_DEBUGE
#define BOOT_DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define BOOT_DLOG_INFO(...)
#endif

#ifdef UPGRADE_DEBUGE_ERROR
#define ERROR_DLOG_INFO(...) dlog_error(__VA_ARGS__)
#else
#define ERROR_DLOG_INFO(...)
#endif

volatile static uint8_t g_u8CheckSum = 0;

void (* redirect_sector_erase)(uint32_t)=(void *)(RUN_SECTOR_ERASE_ADDR);
void (* redirect_block_erase)(uint32_t)=(void *)(RUN_BLOCK_ERASE_ADDR);
/****************************************
boot App
*****************************************/
void BOOTLOAD_bootApp(void)
{
    command_uninit();
    SysTicks_UnInit();
    dlog_info("Enable cpu1 ...");
    dlog_output(200);
    *((volatile uint32_t*)MCU1_CPU_WAIT) = 0;

    dlog_info("Enable cpu2 ...");
    dlog_output(200);
    *((volatile uint32_t*)MCU2_CPU_WAIT) = 0;

    BOOT_DLOG_INFO("Jump to cpu0 ...");
    dlog_output(200);
    *((uint32_t*)MCU0_VECTOR_TABLE_REG) = 0;
    (*((void(*)())((*((uint32_t*)(ITCM0_START+4))))))();
}
/****************************************
copy from nor to TICM
*****************************************/
void BOOTLOAD_copyFromNorToITCM(void)
{
    uint32_t iCount = 0;

    uint8_t* cpu0_app_size_addr = (uint8_t*)APPLICATION_IMAGE_START;
    uint32_t cpu0_app_size = GET_WORD_FROM_ANY_ADDR(cpu0_app_size_addr);
    uint32_t cpu0_app_start_addr = APPLICATION_IMAGE_START + 4;

    uint8_t* cpu1_app_size_addr = (uint8_t*)(cpu0_app_start_addr + cpu0_app_size);
    uint32_t cpu1_app_size = GET_WORD_FROM_ANY_ADDR(cpu1_app_size_addr);
    uint32_t cpu1_app_start_addr = cpu0_app_start_addr + cpu0_app_size + 4;

    uint8_t* cpu2_app_size_addr = (uint8_t*)(cpu1_app_start_addr + cpu1_app_size);
    uint32_t cpu2_app_size = GET_WORD_FROM_ANY_ADDR(cpu2_app_size_addr);
    uint32_t cpu2_app_start_addr = cpu1_app_start_addr + cpu1_app_size + 4;

    BOOT_DLOG_INFO("cpu0_app_start_addr 0x%x", cpu0_app_start_addr);
    BOOT_DLOG_INFO("cpu0_app_size 0x%x", cpu0_app_size);

    BOOT_DLOG_INFO("cpu1_app_start_addr 0x%x", cpu1_app_start_addr);
    BOOT_DLOG_INFO("cpu1_app_size 0x%x", cpu1_app_size);

    BOOT_DLOG_INFO("cpu2_app_start_addr 0x%x", cpu2_app_start_addr);
    BOOT_DLOG_INFO("cpu2_app_size 0x%x", cpu2_app_size);

    dlog_output(1000);
    memcpy((void*)ITCM0_START, (void*)cpu0_app_start_addr, cpu0_app_size);
    memcpy((void*)ITCM1_START, (void*)cpu1_app_start_addr, cpu1_app_size);
    memcpy((void*)ITCM2_START, (void*)cpu2_app_start_addr, cpu2_app_size);
}
/****************************************
erase sector
*****************************************/
void BOOTLOAD_EraseSector(uint32_t i)
{   
    uint32_t busy_flag = 0;
    volatile uint32_t* ptr_regAddr = NULL;
    //enable write
    *((volatile uint32_t*)0x40C00040)=0;
    //checkbusy
    do
    {
        ptr_regAddr =  (uint32_t*)0x40C00000;
        busy_flag = (*ptr_regAddr) & 0x1;
    }while(busy_flag);
    //erase
    *( (volatile uint32_t*)0x40C00090)=i;
    *( (volatile uint32_t*)0x40C00050)=0;
    //checkbusy
    do
    {  
        ptr_regAddr = (uint32_t*)0x40C00000;
        busy_flag = (*ptr_regAddr) & 0x1;
    }while(busy_flag);
    return ;
}
/****************************************
copy function of erase sector to ITCM
*****************************************/
void BOOTLOAD_RepickSectorErase(void)
{
    void *memfun=(void *)(COPY_SECTOR_ERASE_ADDR);    
    void (* funct2)(uint32_t)=BOOTLOAD_EraseSector;
    memcpy(memfun, (funct2 -3), (uint32_t)(&BOOTLOAD_RepickSectorErase) - (uint32_t)(&BOOTLOAD_EraseSector) +3);

}
/****************************************
erase block
*****************************************/
void BOOTLOAD_EraseBlock(uint32_t i)
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
    *( (volatile uint32_t*)0x40C00094)=i;
    *( (volatile uint32_t*)0x40C00054)=0;
    //checkbuys
    do
    {  
        ptr_regAddr = (uint32_t*)0x40C00000;
        busy_flag = (*ptr_regAddr) & 0x1;
    }while(busy_flag);
    return ;
}
/****************************************
copy function of erase block to ITCM
*****************************************/
void BOOTLOAD_RepickBlockErase(void)
{
    void *memfun=(void *)(COPY_BLOCK_ERASE_ADDR);    
    void (* funct2)(uint32_t)=BOOTLOAD_EraseBlock;
    memcpy(memfun, (funct2 -3), (uint32_t)(&BOOTLOAD_RepickBlockErase) - (uint32_t)(&BOOTLOAD_EraseBlock) +3);

}
/****************************************
Copy Data To Nor
*****************************************/
int8_t BOOTLOAD_CopyDataToNor(FIL MyFile,uint32_t u32_addrOffset)
{
    FRESULT           fileResult;
    uint32_t          u32_bytesRead= RDWR_SECTOR_SIZE;
    uint8_t           u8_arrayRecData[RDWR_SECTOR_SIZE];
    uint32_t          u32_recDataSum = 0;
    uint32_t          u32_norAddr = u32_addrOffset;
    int i=0;
    BOOTLOAD_RepickSectorErase();
    BOOT_DLOG_INFO("Nor flash init start ...");
    NOR_FLASH_Init();
    BOOT_DLOG_INFO("Nor flash init end   ...");
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
            return BOOTLOAD_READFILE;
        }
        else
        {
            BOOT_DLOG_INFO("f_read success %d!",u32_bytesRead);
            u32_recDataSum+=u32_bytesRead;            
            BOOT_DLOG_INFO("EraseSector start %x!",u32_norAddr);
            redirect_sector_erase(u32_norAddr);          
            NOR_FLASH_WriteByteBuffer(u32_norAddr, u8_arrayRecData, u32_bytesRead);  
            
            if(0 != g_u8CheckSum)
            {
                uint32_t u32_checkSumArray = 0;
                uint32_t u32_checkSumNor =0;
                uint8_t  u8_val =0;
                uint32_t  i =0;
                for(i=0;i<u32_bytesRead;i++)
                {
                    u32_checkSumArray+=u8_arrayRecData[i];
                }
                for(i=0;i<u32_bytesRead;i++)
                {
                    QUAD_SPI_ReadByte(u32_norAddr+i, &u8_val);
                    u32_checkSumNor +=u8_val;
                }
                BOOT_DLOG_INFO("number of rec data %d  u32_checkSumArray %d u32_checkSumNor %d\n ", u32_bytesRead,u32_checkSumArray,u32_checkSumNor);
            }
            
            u32_norAddr += RDWR_SECTOR_SIZE;            
        }        
    }
    BOOT_DLOG_INFO(" number of totol data %d\n ", u32_recDataSum);
    __asm volatile ("cpsie f");
    __asm volatile ("cpsie i");
    g_u8CheckSum = 0;
    return BOOTLOAD_SUCCESS;
}
int8_t BOOTLOAD_CopyDataToNorWithChecksum(FIL MyFile,uint32_t u32_TCMADDR)
{
    g_u8CheckSum =1;
    return BOOTLOAD_CopyDataToITCM(MyFile,u32_TCMADDR);
}
int8_t BOOTLOAD_CopyDataToITCM(FIL MyFile,uint32_t u32_TCMADDR)
{
    FRESULT           fileResult;
    uint32_t          u32_cpuAppSize = 0;
    uint32_t          u32_bytesRead= RDWR_SECTOR_SIZE;
    uint8_t           u8_arrayRecData[RDWR_SECTOR_SIZE];
    uint32_t          u32_iCount = 0;
    uint32_t          u32_addr = u32_TCMADDR;
    uint32_t          u32_readCount = 0;
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
            return BOOTLOAD_READFILE;
        }                   
        if(0 == u32_iCount)
        {
            u32_cpuAppSize = (uint32_t)(u8_arrayRecData[0]&0xFF) | ((uint32_t)(u8_arrayRecData[1]&0xFF)) <<8 | 
            (((uint32_t)(u8_arrayRecData[2]&0xFF)) <<16) | (((uint32_t)(u8_arrayRecData[3]&0xFF)) <<24);
            BOOT_DLOG_INFO("u32_cpuAppSize 0x%x icount %d", u32_cpuAppSize,u32_iCount);
            memcpy((char *)(u32_addr), &u8_arrayRecData[4], RDWR_SECTOR_SIZE -4);
            u32_addr =u32_addr + (RDWR_SECTOR_SIZE -4);
            u32_readCount = u32_cpuAppSize - (RDWR_SECTOR_SIZE -4);
            u32_iCount =1;
            BOOT_DLOG_INFO("u32_cpuAppSize 0x%x icount %d", u32_cpuAppSize,u32_iCount);
        }
        else
        {
            if(u32_readCount >= RDWR_SECTOR_SIZE)
            {
                u32_readCount -= RDWR_SECTOR_SIZE;
                memcpy((char *)(u32_addr), u8_arrayRecData, RDWR_SECTOR_SIZE);
                u32_addr +=RDWR_SECTOR_SIZE;
                BOOT_DLOG_INFO("u32_cpuAppSize 0x%x u32_readCount %d", u32_cpuAppSize,u32_readCount);
            }
            else
            {
                tmp=u32_readCount;
                BOOT_DLOG_INFO("number of data 0x%x tmp %d\n", u32_readCount,tmp);                
                memcpy((char *)(u32_addr), u8_arrayRecData, u32_readCount);
                u32_readCount =0;
                if(u32_bytesRead<RDWR_SECTOR_SIZE)
                    break;
                if(2 == u32_iCount)
                {
                    u32_addr = ITCM2_START;
                    u32_iCount =3;
                }
                else
                {
                    u32_addr = ITCM1_START;
                    u32_iCount =2;
                }
                
                u32_cpuAppSize = (uint32_t)(u8_arrayRecData[tmp]&0xFF) | ((uint32_t)(u8_arrayRecData[tmp+1]&0xFF)) <<8 | 
                (((uint32_t)(u8_arrayRecData[tmp+2]&0xFF)) <<16) | (((uint32_t)(u8_arrayRecData[tmp+3]&0xFF)) <<24);
                BOOT_DLOG_INFO("u32_cpuAppSize 0x%x, %x\n", u32_cpuAppSize,RDWR_SECTOR_SIZE - tmp -4);
                u32_readCount = u32_cpuAppSize - (RDWR_SECTOR_SIZE - tmp -4);
                memcpy((char *)(u32_addr), &u8_arrayRecData[tmp+4], RDWR_SECTOR_SIZE - tmp -4);             
                u32_addr += (RDWR_SECTOR_SIZE - tmp -4);
                BOOT_DLOG_INFO("u32_cpuAppSize 0x%x u32_readCount %x\n", u32_cpuAppSize,u32_readCount);                               
            }
        }
        
        dlog_info("upgrade cpu%d %d%%",u32_iCount-1,((u32_cpuAppSize-u32_readCount)*100/u32_cpuAppSize));
        dlog_output(100);
        if(0 != g_u8CheckSum)
        {
            uint32_t  u32_checkSumArray = 0;
            uint32_t  u32_checkSumNor =0;
            uint32_t          i = 0;
            for(i=0;i<u32_bytesRead;i++)
            {
                u32_checkSumArray+=u8_arrayRecData[i];
            }
            for(i=0;i<u32_bytesRead;i++)
            {
                u32_checkSumNor+=u8_arrayRecData[i];
            }
            BOOT_DLOG_INFO("number of rec data %d  u32_checkSumArray %d u32_checkSumNor %d\n ", u32_bytesRead,u32_checkSumArray,u32_checkSumNor);
        }     
    }
    #endif
    #if 0
    memset(u8_arrayRecData,0,RDWR_SECTOR_SIZE);
    fileResult = f_read(&MyFile, u8_arrayRecData, 4, (void *)&u32_bytesRead);
    if((u32_bytesRead == 0) || (fileResult != FR_OK))
    {
        ERROR_DLOG_INFO("Cannot Read from the file \n");
        f_close(&MyFile);
        return BOOTLOAD_OPENFILE;
    }
    u32_cpuAppSize = u8_arrayRecData[0] | u8_arrayRecData[1] <<8 | u8_arrayRecData[2] <<16 | u8_arrayRecData[3] <<24;
    BOOT_DLOG_INFO("u32_cpuAppSize 0x%x", u32_cpuAppSize);

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
        BOOT_DLOG_INFO("number of rec data %d  u32_checkSumArray %d u32_checkSumNor %d\n ", u32_bytesRead,u32_checkSumArray);
        if((u32_bytesRead == 0) || (fileResult != FR_OK) || (u32_readCount != u32_bytesRead))
        {
            ERROR_DLOG_INFO("Cannot Read from the file \n");
            f_close(&MyFile);
            return BOOTLOAD_READFILE;
        }
        else
        {
            BOOT_DLOG_INFO("copy addr %x  number of data %d\n",u32_addr,u32_bytesRead);
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
                BOOT_DLOG_INFO("number of rec data %d  u32_checkSumArray %d u32_checkSumNor %d\n ", u32_bytesRead,u32_checkSumArray,u32_checkSumNor);
            }
            #endif                              
        }
        
    }
    BOOT_DLOG_INFO(" number of totol data %x\n ",u32_iCount);
    #endif
    g_u8CheckSum = 0;  
    return BOOTLOAD_SUCCESS;

}
int8_t BOOTLOAD_CopyDataToITCMWithChecksum(FIL MyFile,uint32_t u32_TCMADDR)
{
    g_u8CheckSum = 1;
    return BOOTLOAD_CopyDataToITCM(MyFile,u32_TCMADDR);
}
void check(void)
{

    uint8_t* cpu0_app_size_addr = (uint8_t*)APPLICATION_IMAGE_START;
    uint32_t cpu0_app_size = GET_WORD_FROM_ANY_ADDR(cpu0_app_size_addr);
    uint32_t cpu0_app_start_addr = APPLICATION_IMAGE_START + 4;

    uint8_t* cpu1_app_size_addr = (uint8_t*)(cpu0_app_start_addr + cpu0_app_size);
    uint32_t cpu1_app_size = GET_WORD_FROM_ANY_ADDR(cpu1_app_size_addr);
    uint32_t cpu1_app_start_addr = cpu0_app_start_addr + cpu0_app_size + 4;

    uint8_t* cpu2_app_size_addr = (uint8_t*)(cpu1_app_start_addr + cpu1_app_size);
    uint32_t cpu2_app_size = GET_WORD_FROM_ANY_ADDR(cpu2_app_size_addr);
    uint32_t cpu2_app_start_addr = cpu1_app_start_addr + cpu1_app_size + 4;

    BOOT_DLOG_INFO("cpu0_app_start_addr 0x%x", cpu0_app_start_addr);
    BOOT_DLOG_INFO("cpu0_app_size 0x%x", cpu0_app_size);

    BOOT_DLOG_INFO("cpu1_app_start_addr 0x%x", cpu1_app_start_addr);
    BOOT_DLOG_INFO("cpu1_app_size 0x%x", cpu1_app_size);

    BOOT_DLOG_INFO("cpu2_app_start_addr 0x%x", cpu2_app_start_addr);
    BOOT_DLOG_INFO("cpu2_app_size 0x%x", cpu2_app_size);
    uint32_t u32_checkSumArray = 0;
    uint32_t u32_checkSumNor =0;
    uint32_t  i =0; 
    char  *u8_val =(char *)(ITCM0_START);
    char  *u8_val2 =(char *)(cpu0_app_start_addr);  
    
    for(i=0;i<cpu0_app_size;i++)
    {
        if(i%(1024*4) == 0)
        {
            BOOT_DLOG_INFO("TICM %d FLASH %d\n ",u32_checkSumNor,u32_checkSumArray);
            u32_checkSumArray = 0;
            u32_checkSumNor =0;
        }
        u32_checkSumNor +=*(u8_val+i);
        u32_checkSumArray +=*(u8_val2+i);               
    }
    BOOT_DLOG_INFO("TICM %d FLASH %d\n ",u32_checkSumNor,u32_checkSumArray);
    
    u32_checkSumArray = 0;
    u32_checkSumNor =0;
    u8_val =(char *)(ITCM1_START);
    u8_val2 =(char *)(cpu1_app_start_addr); 
    for(i=0;i<cpu1_app_size;i++)
    {
        if(i%(1024*4) == 0)
        {
            BOOT_DLOG_INFO("TICM %d FLASH %d\n ",u32_checkSumNor,u32_checkSumArray);
            u32_checkSumArray = 0;
            u32_checkSumNor =0;
        }
        u32_checkSumNor +=*(u8_val+i);
        u32_checkSumArray +=*(u8_val2+i);        
    }
    BOOT_DLOG_INFO("TICM %d FLASH %d\n ",u32_checkSumNor,u32_checkSumArray);
    
    u32_checkSumArray = 0;
    u32_checkSumNor =0;

    u8_val =(char *)(ITCM2_START);
    u8_val2 =(char *)(cpu2_app_start_addr);
    for(i=0;i<cpu2_app_size;i++)
    {
        if(i%(1024*4) == 0)
        {
            BOOT_DLOG_INFO("TICM %d FLASH %d\n ",u32_checkSumNor,u32_checkSumArray);
            u32_checkSumArray = 0;
            u32_checkSumNor =0;
        }
        u32_checkSumNor +=*(u8_val+i);
        u32_checkSumArray +=*(u8_val2+i);        
    }
    BOOT_DLOG_INFO("TICM %d FLASH %d\n ",u32_checkSumNor,u32_checkSumArray);

}
