#ifndef OV5640_H
#define	OV5640_H

#include <stdlib.h>
#include <string.h>
#include "debuglog.h"
#include "i2c.h"
#include "ov5640_mipi.h"

void command_TestOv5640(void);

void command_TestOv5640Write(unsigned char *subAddr, unsigned char *value);

void command_TestOv5640Read(unsigned char *subAddr);


#endif
