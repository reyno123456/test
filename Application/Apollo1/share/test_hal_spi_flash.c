#include <stdint.h> 
#include <stdlib.h>
#include "hal_spi.h"
#include "test_hal_spi_flash.h" 
#include "debuglog.h" 

#define     CLKRATE_MHZ         (12)
/*====================================================*/
/*             WinBond SPI_FLASH Test FUNC            */
/*====================================================*/ 
//Read W25QXX_FLASH ID
void TEST_SPI_init(uint8_t index)
{
    STRU_HAL_SPI_INIT st_spiInitInfo = 
    {
        .u16_halSpiBaudr = CLKRATE_MHZ,
        .e_halSpiPolarity = HAL_SPI_POLARITY_HIGH,
        .e_halSpiPhase = HAL_SPI_PHASE_2EDGE,
    };
    HAL_SPI_MasterInit(index, &st_spiInitInfo);
}

uint16_t Test_WbFlashID(uint8_t SPI_BASE_ADDR)
{
    uint16_t device_id = 0;
    uint8_t u8Wdate[] = {0x90, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t u8Rbuf[6]; 
   // while(1)
    {
    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8Wdate, sizeof(u8Wdate),u8Rbuf, 6);  
    }
    device_id |= u8Rbuf[4]<< 8;     //DeviceID: 16bits
    device_id |= u8Rbuf[5] ;

    return device_id ;
}
void command_WbFlashID(char *SPI_BASE_ADDR)
{

    uint8_t u8SpiAddr = strtoul(SPI_BASE_ADDR, NULL, 0);
    TEST_SPI_init(u8SpiAddr);
    dlog_info("ID %d \n",Test_WbFlashID(u8SpiAddr));

}

//
void Test_WbFlashCommand (ENUM_SPI_COMPONENT SPI_BASE_ADDR, uint8_t cmd)
{
    uint8_t u8Wdate[] = {cmd};
    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8Wdate, sizeof(u8Wdate),NULL, 0);
}

uint8_t Test_WbFlashReadReg (ENUM_SPI_COMPONENT SPI_BASE_ADDR, uint8_t cmd)
{
    uint8_t u8Wdate[] = {cmd, 0};
    uint8_t u8Rbuf[2];
    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8Wdate, sizeof(u8Wdate),u8Rbuf, 2);
    
    return u8Rbuf[1];
}

void Test_WbFlashWriteData (ENUM_SPI_COMPONENT SPI_BASE_ADDR,uint8_t wr_instruc, uint32_t addr, uint8_t data)
{
    uint8_t u8Wdate[] = {wr_instruc, (addr >>16) & 0x000000FF, (addr >> 8) & 0x000000FF, (addr & 0x000000FF),data};    
    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8Wdate, sizeof(u8Wdate),NULL, 0);
}

uint8_t Test_WbFlashReadData (ENUM_SPI_COMPONENT SPI_BASE_ADDR,uint8_t rd_instruc, uint32_t addr)
{
    uint8_t u8Wdate[] = {rd_instruc,  (addr >>16) & 0x000000FF, (addr >> 8) & 0x000000FF,(addr & 0x000000FF), 0x00};
    uint8_t u8Rbuf[5]; 
    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8Wdate, sizeof(u8Wdate),u8Rbuf, 5);

    return u8Rbuf[4] ;
}

uint8_t Test_WbBlockErase(ENUM_SPI_COMPONENT SPI_BASE_ADDR, uint8_t cmd, uint32_t addr)
{ 
    uint8_t u8Wdate[] = {cmd, (addr >>16) & 0x000000FF, (addr >> 8) & 0x000000FF, (addr & 0x000000FF)}; 

    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8Wdate, sizeof(u8Wdate),NULL, 0);

}
void command_TestWbBlockErase(char *SPI_BASE_ADDR, char *cmd, char *addr)
{
    uint8_t u8Cmd = strtoul(cmd, NULL, 0);
    uint32_t u32Addr = strtoul(addr, NULL, 0);
    ENUM_SPI_COMPONENT u8SpiAddr = strtoul(SPI_BASE_ADDR, NULL, 0);
    TEST_SPI_init(u8SpiAddr);
    Test_WbBlockErase(u8SpiAddr, u8Cmd, u32Addr);
 
}
void command_TestWbFlash(char *spi_base)
{
    uint8_t u8WEL = 0;
    uint8_t u8BUSY = 0;
    uint8_t u8WPS = 0;
    uint8_t u8S1 = 0;
    uint8_t u8S3 = 0;
    uint8_t u8read_val = 0;
    uint8_t i = 0;
    uint16_t  u8wb_flash_id = 0;
    uint8_t u8wr_dat = 0;
    uint8_t u8rd_dat = 0;
    uint32_t u32_data;

    uint8_t SPI_BASE_ADDR = strtoul(spi_base, NULL, 0);

    if (5 == SPI_BASE_ADDR)
    {
       u32_data = *(uint32_t *)(0x40B00088);
       u32_data &= 0xFFFFF00F; 
       u32_data |= 0x00000550; // bit11~4 = 01010101B
       *(uint32_t *)(0x40B00088) = u32_data;
    }

    TEST_SPI_init(SPI_BASE_ADDR);
    //Device_ID Check 
    u8wb_flash_id = Test_WbFlashID(SPI_BASE_ADDR) ;
    if(u8wb_flash_id == W25Q128_ID)
    {
        dlog_info("Scan WB_Flash Pass!\n");
    }
    else 
    {
        dlog_info("Scan WB_Flash Error! \n:");
    }
 
    //Winbond_SPI_FLASH Setting 
    dlog_info("WB WRITE START\n");

    
    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done

    Test_WbFlashCommand(SPI_BASE_ADDR,0x98);         //global unlock

    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //ready to accept another command

    u8S3 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x15);       //read status reg3; [2] u8WPS;
    u8WPS = u8S3 & 0x04;
    if(!u8WPS)
    {
        dlog_info("u8WPS is zero\n");    //individual lock is disable
    }
    

    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done

    Test_WbBlockErase(SPI_BASE_ADDR,0x20,0x00);        // erase instruction 0X20:4KB;0X52: 32KB; 0XD8:64KB
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //eady to accept another command

    //========================Write data to WB flash======================================
    dlog_info("Flash write Start Check ....\n"); 
    dlog_info("\n"); 
    u8wr_dat = 0 ;
    for(i=0;i<60;i++)
    {
        u8wr_dat = (i % 256) ;

        Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable

        do{
            u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
            u8WEL = u8S1 & 0x02;
        }while(!u8WEL);               //write enable done
        Test_WbFlashWriteData(SPI_BASE_ADDR,0x02,i,u8wr_dat);         //write data i to addr i;
        do{
            u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
            u8BUSY= u8S1 & 0x01;
        }while(u8BUSY);               //write done

    }
    dlog_info("Write data done\n");        //individual lock is disable

    //========================read data from WB flash====================================
    dlog_info("Flash Read Start Check ....\n");    
    u8rd_dat = 0 ;
    for(i=0;i<60;i++)
    {
        u8rd_dat = (i % 256);
        Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
        do{
            u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
            u8WEL = u8S1 & 0x02;
        }while(!u8WEL);               //write enable done

        u8read_val = Test_WbFlashReadData(SPI_BASE_ADDR,0x03,i);  //read data in addr i;        
        do{
            u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
            u8BUSY= u8S1 & 0x01;
        }while(u8BUSY);               //write done
        dlog_info("%x %x \n",u8read_val,u8rd_dat);
    }
    Test_WbFlashCommand(SPI_BASE_ADDR,0x04); //write disable

}

