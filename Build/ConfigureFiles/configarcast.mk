# cross compile...
CROSS_COMPILE              = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/bin/arm-none-eabi-
CROSS_COMPILE_LIB_GCC_PATH = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/lib/gcc/arm-none-eabi/5.2.1/armv7e-m
CROSS_COMPILE_LIB_PATH     = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/arm-none-eabi/lib/armv7e-m

CC      = $(CROSS_COMPILE)gcc
CXX     = $(CROSS_COMPILE)g++
AR      = $(CROSS_COMPILE)ar
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
NM      = $(CROSS_COMPILE)nm
AS      = $(CROSS_COMPILE)as

ARFLAGS = cr
RM = -rm -rf
MAKE = make

suffix = $(notdir $(CURDIR))

export suffix

export CC
export CXX
export AS
export AR
export CFLAGS
export CXXFLAGS
export INCDIRS
export LDFLAGS
export ARFLAGS
export MAKE
export RM

###############################################################################

DEBUG ?= n

ifeq ($(DEBUG), y)
CPU0_COMPILE_FLAGS = -g
CPU1_COMPILE_FLAGS = -g
CPU2_COMPILE_FLAGS = -O1 -g
DEBREL = Debug
else
CPU0_COMPILE_FLAGS = -g
CPU1_COMPILE_FLAGS = -g
CPU2_COMPILE_FLAGS = -O1 -g
DEBREL = Release
endif

export CPU0_COMPILE_FLAGS
export CPU1_COMPILE_FLAGS
export CPU2_COMPILE_FLAGS

CFLAGS = #-Wall

DEFS =

CPU_DEFS = -mthumb -mcpu=cortex-m7 -mlittle-endian -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -c -Wa,-mimplicit-it=thumb -Werror

INCDIRS =

LDFLAGS = -L$(CROSS_COMPILE_LIB_GCC_PATH) -L$(CROSS_COMPILE_LIB_PATH) $(LIBS)

###############################################################################

export CHIP = AR8020
export BOOT = AR8020
export BOARD = ARCast

export USB_DEV_CLASS_HID_ENABLE = 1

export BB_REG_CFG_BIN_FILE_NAME = 001_cfg_bb_reg.bin
export HDMI_EDID_CFG_BIN_FILE_NAME = 002_cfg_it_66021_edid.bin
export CFG_BIN_FILE_NAME_LIST = $(BB_REG_CFG_BIN_FILE_NAME) $(HDMI_EDID_CFG_BIN_FILE_NAME)

FUNCTION_DEFS = -DSTM32F746xx -DUSE_USB_HS -DUSE_HAL_DRIVER -DUSE_WINBOND_SPI_NOR_FLASH -DUSE_BB_REG_CONFIG_BIN -DDUSE_IT66021_EDID_CONFIG_BIN

CFLAGS += $(DEFS) $(CPU_DEFS) $(FUNCTION_DEFS) -std=c99 $(INCDIRS)

CXXFLAGS += $(DEFS) $(CPU_DEFS) $(INCDIRS)

APPLICATION_DIR ?= $(TOP_DIR)/Application/ARCast

###############################################################################
