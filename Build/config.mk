# cross compile...
CROSS_COMPILE = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/bin/arm-none-eabi-

CC      = $(CROSS_COMPILE)gcc
AR      = $(CROSS_COMPILE)ar
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
NM      = $(CROSS_COMPILE)nm

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

DEFS = -mthumb -mcpu=cortex-m7 -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -std=c99 -c -DSTM32F746xx

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

CHIP = AR8020
export CHIP

###############################################################################

# make all .c
%.o:	%.c
	@echo "Compling: " $(addsuffix .c, $(basename $(notdir $@)))
	@$(CC) $(CFLAGS) -c $< -o $@

