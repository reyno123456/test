# cross compile...
CROSS_COMPILE = /home/sbyang/Desktop/gcc-arm-none-eabi-5_2-2015q4/bin/arm-none-eabi-
CROSS_COMPILE_LIB_PATH = /home/sbyang/Desktop/gcc-arm-none-eabi-5_2-2015q4/lib/gcc/arm-none-eabi/5.2.1/armv7-m

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
DEBUG = y

ifeq ($(DEBUG), y)
CFLAGS += -g
DEBREL = Debug
else
CFLAGS += -O2 -s
DEBREL = Release
endif

DEFS = -mthumb -mcpu=cortex-m7 -mlittle-endian -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -std=c99 -c -Wa,-mimplicit-it=thumb -Werror -DSTM32F746xx -DUSE_USB_HS -DUSE_HAL_DRIVER -DUSE_WINBOND_SPI_NOR_FLASH

CFLAGS += $(DEFS)

LDFLAGS = $(LIBS)

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

CHIP = AR8020
BOOT = AR8020
BOARD = AR8020TEST

export CHIP
export BOOT
export BOARD

export USB_DEV_CLASS_HID_ENABLE = 1
export CROSS_COMPILE_LIB_PATH
###############################################################################

# make all .c
%.o:	%.c
	@echo "Compling: " $(addsuffix .c, $(basename $(notdir $@)))
	@$(CC) $(CFLAGS) -c $< -o $@

