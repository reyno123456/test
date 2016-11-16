#ifndef _UPDATA_H_
#define _UPDATA_H_

enum updata_error
{
    UPDATA_SUCCESS = 0,
    UPDATA_READFILE= -1,
    UPDATA_OPENFILE= -2

};

void UPDATA_UpdataFromSDToNor(void);
void UPDATA_UpdataFromUsbToNor(void);
void UPDATA_BootFromSD(void);
void UPDATA_BootFromUSB(void);
void command_updata(void);
/*
void UPDATA_UpdataBootloaderFromUart(void);
void UPDATA_UpdataBootloaderFromSD(void);
void UPDATA_UpdataBootloaderFromUSB(void);
*/
#endif