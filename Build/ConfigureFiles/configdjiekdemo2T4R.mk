# cross compile...
CROSS_COMPILE = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/bin/arm-none-eabi-
CROSS_COMPILE_LIB_GCC_PATH = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/lib/gcc/arm-none-eabi/5.2.1/armv7e-m
CROSS_COMPILE_LIB_PATH = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/arm-none-eabi/lib/armv7e-m

APPLICATION_DIR ?= $(TOP_DIR)/Application/DJIEKDemo


CC      = $(CROSS_COMPILE)gcc
AR      = $(CROSS_COMPILE)ar
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
NM      = $(CROSS_COMPILE)nm
AS      = $(CROSS_COMPILE)as

ARFLAGS = cr
RM = -rm -rf
MAKE = make

CFLAGS = #-Wall
DEBUG ?= n

ifeq ($(DEBUG), y)
CPU0_CFLAGS = -g
CPU1_CFLAGS = -g
CPU2_CFLAGS = -O1 -g
DEBREL = Debug
else
CPU0_CFLAGS = -O2 -s
CPU1_CFLAGS = -O2 -s
CPU2_CFLAGS = -O1 -s
DEBREL = Release
endif

DEFS = -mthumb -mcpu=cortex-m7 -mlittle-endian -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -std=c99 -c -Wa,-mimplicit-it=thumb -Werror -DSTM32F746xx -DUSE_USB_HS -DUSE_HAL_DRIVER -DUSE_WINBOND_SPI_NOR_FLASH

DEFS += -DUSE_BB_REG_CONFIG_BIN -DUSE_ADV7611_EDID_CONFIG_BIN -DBBRF_2T4R

CFLAGS += $(DEFS)

LDFLAGS = -L$(CROSS_COMPILE_LIB_GCC_PATH) -L$(CROSS_COMPILE_LIB_PATH) $(LIBS)

INCDIRS =

CFLAGS += $(INCDIRS)

###############################################################################

suffix = $(notdir $(CURDIR))
export suffix

# export to other Makefile
export CC
export CFLAGS
export INCDIRS
export AR
export ARFLAGS
export RM
export AS

export CPU0_CFLAGS
export CPU1_CFLAGS
export CPU2_CFLAGS

CHIP = AR8020
BOOT = AR8020
BOARD = EKDemo

export CHIP
export BOOT
export BOARD

export USB_DEV_CLASS_HID_ENABLE = 1
export CROSS_COMPILE_LIB_PATH

export BB_REG_CFG_BIN_FILE_NAME = 001_cfg_bb_reg.bin
export HDMI_EDID_CFG_BIN_FILE_NAME = 002_cfg_adv_7611_edid.bin
export CFG_BIN_FILE_NAME_LIST = $(BB_REG_CFG_BIN_FILE_NAME) $(HDMI_EDID_CFG_BIN_FILE_NAME)

###############################################################################

# make all .c
%.o:	%.c
	@echo "Compling: " $(addsuffix .c, $(basename $(notdir $@)))
	@$(CC) $(CFLAGS) -c $< -o $@

