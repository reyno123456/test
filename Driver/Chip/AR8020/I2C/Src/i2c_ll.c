#include <stddef.h>
#include <stdint.h>
#include "data_type.h"
#include "reg_rw.h"
#include "i2c_ll.h"
#include "i2c.h"
#include "debuglog.h"

static STRU_I2C_Controller stru_i2cControllerArray[MAX_I2C_CONTOLLER_NUMBER] = 
{
    {
        .u32_i2cRegBaseAddr = BASE_ADDR_I2C0,
    },
    {
        .u32_i2cRegBaseAddr = BASE_ADDR_I2C1,
    },
    {
        .u32_i2cRegBaseAddr = BASE_ADDR_I2C2,
    },
    {
        .u32_i2cRegBaseAddr = BASE_ADDR_I2C3,
    },
    {
        .u32_i2cRegBaseAddr = BASE_ADDR_I2C4,
    },
};

STRU_I2C_Controller* I2C_LL_GetI2CController(EN_I2C_COMPONENT en_i2cComponent)
{
    STRU_I2C_Controller* ptr_i2cController = NULL;

    if (en_i2cComponent < sizeof(stru_i2cControllerArray)/sizeof(stru_i2cControllerArray[0]))
    {
    	ptr_i2cController = &stru_i2cControllerArray[en_i2cComponent];
    }
    
    return ptr_i2cController;
}

uint8_t I2C_LL_IOCtl(STRU_I2C_Controller* ptr_i2cController, ENUM_I2C_CMD_ID en_i2cCommandID, uint32_t* ptr_i2cCommandVal)
{
    if (ptr_i2cController == NULL)
    {
        dlog_error("ptr_i2cController = %p\n", ptr_i2cController);
        return FALSE;
    }

    if (ptr_i2cCommandVal == NULL)
    {
    	dlog_error("ptr_i2cCommandVal = %p\n", ptr_i2cCommandVal);
        return FALSE;
    }
    
    STRU_I2C_Type* i2c_reg = (STRU_I2C_Type*)ptr_i2cController->u32_i2cRegBaseAddr;

    if (i2c_reg == NULL)
    {
        dlog_error("i2c_reg = %p\n", i2c_reg);
        return FALSE;
    }

    switch(en_i2cCommandID)
    {
    case I2C_CMD_SET_MODE:
        if (*ptr_i2cCommandVal == I2C_Master_Mode)
        {
            /* i2c master initialization */

            i2c_reg->IC_ENABLE &= ~(0x1);               // disbale i2c

            if (ptr_i2cController->parameter.master.speed == I2C_Standard_Speed)  // stardard speed
            {
                i2c_reg->IC_CON &= ~(7<<2);             // [4]:addr_mst in 7-bit; [3]:addr_slv in 7-bit; [2:1]=01,standard mode;
                i2c_reg->IC_CON |= (3<<0 | 3<<5);       // [1]; [0]:enable master-mode;[6]:disbale i2c slave-only mode; [5]:enable ic_restart
                i2c_reg->IC_FS_SCL_HCNT = 0xc8;         // set high period of SCL = 0x1e(2d-fs) | 0xc8(ss) | 0x03(hs)
                i2c_reg->IC_FS_SCL_LCNT = 0xeb;         // set low period of  SCL = 0x4b(50-fs) | 0xeb(ss) | 0x08(hs)
            }
            else if (ptr_i2cController->parameter.master.speed == I2C_Fast_Speed)  // fast speed
            {
                i2c_reg->IC_CON &= ~(3<<3 | 1<<1);      // [4]:addr_mst in 7-bit; [3]:addr_slv in 7-bit; [2:1]=2,fast mode;
                i2c_reg->IC_CON |= (5<<0 | 3<<5);       // [2]; [0]:enable master-mode;[6]:disbale i2c slave-only mode; [5]:enable ic_restart
                i2c_reg->IC_FS_SCL_HCNT = 0x2d;         // set high period of SCL = 0x1e(2d-fs)
                i2c_reg->IC_FS_SCL_LCNT = 0x50;         // set low period of  SCL = 0x4b(50-fs)
            }
            else if (ptr_i2cController->parameter.master.speed == I2C_High_Speed)  // high mode
            {
                i2c_reg->IC_CON &= ~(3<<3);             // [4]:addr_mst in 7-bit; [3]:addr_slv in 7-bit; [2:1]=11,high speed mode;
                i2c_reg->IC_CON |= (7<<0 | 3<<5);       // [2:1]=0x11; [0]:enable master-mode;[6]:disbale i2c slave-only mode; [5]:enable ic_restart
                i2c_reg->IC_FS_SCL_HCNT = 0x03;         // set high period of SCL = 0x03(hs)
                i2c_reg->IC_FS_SCL_LCNT = 0x08;         // set low period of  SCL = 0x08(hs)
            }
            
            i2c_reg->IC_TAR = ptr_i2cController->parameter.master.addr;  // set address of target slave --0x051 for E2PROM
            i2c_reg->IC_FS_SPKLEN = 0x1;                // set the min spike suppression limit
            i2c_reg->IC_INTR_MASK = 0x0fff;             // unmask all the interrupts
            i2c_reg->IC_TX_TL = 0x04;                   // set TX fifo threshold level
            i2c_reg->IC_RX_TL = 0x04;                   // set RX fifo threshold level --for as a receiver
            i2c_reg->IC_ENABLE |= 0x01;                 // enable the i2c

            ptr_i2cController->en_i2cMode = I2C_Master_Mode;
        }
        else if (*ptr_i2cCommandVal == I2C_Slave_Mode)
        {
            /* i2c slave initialization */
            
            i2c_reg->IC_ENABLE &= ~(0x1);                              // disable i2c
            i2c_reg->IC_SAR  = ptr_i2cController->parameter.slave.addr;   // set the slave address
            i2c_reg->IC_FS_SPKLEN = 0x1;                // set the min spike suppression limit
            i2c_reg->IC_CON &= ~(3<<3 | 1<<6 | 3<<0);   // [4],[3]:support 7-bit address; [6]:enbale i2c slave-only mode; [0]:disable master-mode
            i2c_reg->IC_CON |= (1<<2 | 1<<5);           // [5]:enable the restart mode; [2:1]=2, fast mode

            // i2c slave config
            //i2c_reg->IC_INTR_MASK = 0x0fff;           // unmask all the interrupts
            i2c_reg->IC_INTR_MASK = 0x0064;             // unmask all the interrupts
            i2c_reg->IC_ENABLE |= 0x1;                  // enable i2c

            ptr_i2cController->en_i2cMode = I2C_Slave_Mode;
        }
        break;
    case I2C_CMD_SET_M_SPEED:
    	ptr_i2cController->parameter.master.speed = (ENUM_I2C_Speed)(*ptr_i2cCommandVal);
        break;
    case I2C_CMD_SET_M_TARGET_ADDRESS:
    	ptr_i2cController->parameter.master.addr = (uint16_t)(*ptr_i2cCommandVal);
        break;
    case I2C_CMD_SET_M_WRITE_DATA:
        if (ptr_i2cController->en_i2cMode == I2C_Master_Mode)
        {
            unsigned int pre_rd;
            unsigned int data = (*ptr_i2cCommandVal) & 0xFF;
            pre_rd = Reg_Read32((unsigned int)i2c_reg + 0x10);
            pre_rd = pre_rd & 0xfe000;              // Write enable, IC_DATA_CMD[8]=0x0
            data |= pre_rd;
            i2c_reg->IC_DATA_CMD = data;
        }
        else
        {
            dlog_error("Error: Write data in slave I2C mode!");
            return FALSE;
        }
        break;
    case I2C_CMD_SET_S_SLAVE_ADDRESS:
    	ptr_i2cController->parameter.slave.addr = (uint16_t)(*ptr_i2cCommandVal);
        break;
    case I2C_CMD_GET_M_READ_DATA:
        if (ptr_i2cController->en_i2cMode == I2C_Master_Mode)
        {
            uint32_t data_tmp;
            i2c_reg->IC_DATA_CMD |= (1<<8);         // Read enable, IC_DATA_CMD[8]=0x1
            
            /* Wait till data is ready */
            while(Reg_Read32((unsigned int)i2c_reg + 0x70) & (1 << 5)) //i2c_reg->IC_STATUS
            {
                I2C_LL_Delay(1000);
            }

            /* Read data */
            *ptr_i2cCommandVal = Reg_Read32((unsigned int)i2c_reg + 0x10) & 0xFF; //i2c_reg->IC_DATA_CMD;
        }
        else
        {
            dlog_error("Error: Read data in slave I2C mode!");
            return FALSE;
        }
        break;
    case I2C_CMD_GET_M_IDLE:
    	*ptr_i2cCommandVal =  Reg_Read32((unsigned int)i2c_reg + 0x70) & (1 << 5) ? 0 : 1; //i2c_reg->IC_STATUS;
        break;
    case I2C_CMD_GET_M_TXFIFO_EMPTY:
    	*ptr_i2cCommandVal =  Reg_Read32((unsigned int)i2c_reg + 0x70) & (1 << 2) ? 1 : 0; //i2c_reg->IC_STATUS;
    	break;
    default:
        return FALSE;
    }
    
    return TRUE;
}

void I2C_LL_Delay(unsigned int delay)
{
    volatile int i = delay;
    while(i > 0)
    {
        i--;
    }
}


