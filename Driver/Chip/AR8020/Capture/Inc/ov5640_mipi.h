/* ----------------------------------------------------------------------
 * $Date:        2016.12.09
 * $Revision:    V0.01
 *
 * Project:      
 * Title:        ov5640_mipi.h
 *
 * Version 0.01
 *       
 *  
 *----------------------------------------------------------------------------
 *
 * 
 *---------------------------------------------------------------------------*/

 /**
  ******************************************************************************
  * @file    ov5640_mipi.h
  * @author  
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  *
  ******************************************************************************
  */

#ifndef	OV5640_MIPI_H
#define OV5640_MIPI_H

#include <stdint.h>
#include <stdbool.h>
#include "data_type.h"
#include "i2c.h"
#include "debuglog.h"


/*******************Macro define**************************/
#define	OV5640_I2C_ADDR		(0x78 >> 1)
#define	OV5640_COMPONENT	(I2C_Component_0)

enum ov5640_mode {
	ov5640_mode_MIN = 0,
	ov5640_mode_VGA_640_480 = 0,
	ov5640_mode_QVGA_320_240 = 1,
	ov5640_mode_NTSC_720_480 = 2,
	ov5640_mode_PAL_720_576 = 3,
	ov5640_mode_720P_1280_720 = 4,
	ov5640_mode_1080P_1920_1080 = 5,
	ov5640_mode_QSXGA_2592_1944 = 6,
	ov5640_mode_QCIF_176_144 = 7,
	ov5640_mode_XGA_1024_768 = 8,
	ov5640_mode_MAX = 8,
	ov5640_mode_INIT = 0xff, /*only for sensor init*/
};

enum ov5640_frame_rate {
	ov5640_15_fps,
	ov5640_30_fps
};

/* image size under 1280 * 960 are SUBSAMPLING
 *  * image size upper 1280 * 960 are SCALING
 *   */
enum ov5640_downsize_mode {
	SUBSAMPLING,
	SCALING,
};

struct reg_value {
	uint16_t uint16_tRegAddr;
	uint8_t uint8_tVal;
	uint8_t uint8_tMask;
	uint32_t uint32_tDelay_ms;
};

struct ov5640_mode_info {
	enum ov5640_mode mode;
	enum ov5640_downsize_mode dn_mode;
	uint32_t width;
	uint32_t height;
	struct reg_value *init_data_ptr;
	uint32_t init_data_size;
};
/*******************Function declaration**************************/

/**
* @brief	write OV5640 register with value val 
* @param  	reg	OV5640 register address
* @param  	val	data need to write  
* @retval 	0:write success, -1:write failed
* @note   
*/
int32_t OV5640_write_reg(uint16_t reg, uint8_t val);

/**
* @brief	read OV5640 register 
* @param  	reg	OV5640 register address
* @param  	val	read data  
* @retval 	0:read success, -1:read failed
* @note   
*/
int32_t OV5640_read_reg(uint16_t reg, uint8_t *val);

/**
* @brief	 
* @param  	 
* @retval 	
* @note   
*/
void OV5640_stream_on(void);

/**
* @brief	 
* @param  	 
* @retval 	
* @note   
*/
void OV5640_stream_off(void);

/**
* @brief	 
* @param  	 
* @retval 	
* @note   
*/
void OV5640_set_night_mode(void);

/**
* @brief	 
* @param  	 
* @retval 	
* @note   
*/
int32_t OV5640_get_HTS(void);

/**
* @brief	 
* @param  	 
* @retval 	
* @note   
*/
int32_t OV5640_get_VTS(void);

/**
* @brief	 
* @param  	 
* @retval 	
* @note   
*/
int32_t OV5640_set_VTS(int32_t VTS);

/**
* @brief	 
* @param  	 
* @retval 	
* @note   
*/
int32_t OV5640_get_shutter(void);

/**
* @brief	 
* @param  	 
* @retval 	
* @note   
*/
int32_t OV5640_set_shutter(int32_t shutter);

/**
* @brief	 
* @param  	 
* @retval 	
* @note   
*/
int32_t OV5640_get_gain16(void);

/**
* @brief	 
* @param  	 
* @retval 	
* @note   
*/
int32_t OV5640_set_gain16(int32_t gain16);

/**
* @brief	 
* @param  	 
* @retval 	
* @note   
*/
int32_t OV5640_get_light_freq(void);

/**
* @brief	 
* @param  	 
* @retval 	
* @note   
*/
void OV5640_init(void);

/**
* @brief	 download OV5640 settings to sensor through i2c
* @param  	 
* @retval 	
* @note   
*/
uint32_t OV5640_download_firmware(struct reg_value *pModeSetting, uint32_t ArySize);

/*#define OV5640_VOLTAGE_ANALOG               2800000
#define OV5640_VOLTAGE_DIGITAL_CORE         1500000
#define OV5640_VOLTAGE_DIGITAL_IO           1800000

#define MIN_FPS 15
#define MAX_FPS 30
#define DEFAULT_FPS 30

#define OV5640_XCLK_MIN 6000000
#define OV5640_XCLK_MAX 24000000

#define OV5640_CHIP_ID_HIGH_BYTE	0x300A
#define OV5640_CHIP_ID_LOW_BYTE		0x300B*/



#endif
