# top file directory
TOP_DIR ?= ../../..

# build file directory
BUILD_DIR ?= $(TOP_DIR)/Build

# binary file director
OUTPUT_DIR ?= $(TOP_DIR)/Output
OUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Inc
OUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Lib

# current directory
CURDIR ?= $(shell pwd)

include $(BUILD_DIR)/config.mk

###############################################################################
# static library name
LIB = libcommon.a

# source file(s), including c file(s)
SRC_C   := Src/string.c \
           Src/printf.c \
           Src/memory.c \
           Src/debuglog.c \
           Src/interrupt.c \
           Src/command.c

INCDIRS :=  -IInc \
            -I$(TOP_DIR)/Driver/Chip/AR8020/UART/Inc \
            -I$(TOP_DIR)/Library/ThirdParty/CMSIS/Device/ST/STM32F7xx/Include \
            -I$(TOP_DIR)/Library/ThirdParty/CMSIS/Include \
            -I$(TOP_DIR)/Driver/Chip/AR8020/SD/Inc \
            -I$(TOP_DIR)/Kernel/FreeRTOS/CMSIS_RTOS \
            -I$(TOP_DIR)/Kernel/FreeRTOS/include \
            -I$(TOP_DIR)/Configure/Include \
            -I$(TOP_DIR)/Kernel/FreeRTOS/portable/GCC/ARM_CM7/r0p1

# object file(s)
OBJ_C   := $(patsubst %.c,%.o,$(SRC_C))

all: .depend $(LIB) install

$(LIB):  $(OBJ_C)
	@echo "Generating static library: " $(notdir $@)
	@$(AR) $(ARFLAGS) $@ $^

install:
	@mkdir -p $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/debuglog.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/interrupt.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/interrupt_internal.h $(OUTPUT_HEADER_STAGING_DIR)
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) $(OBJ_CPP) .depend $(OUTPUT_LIB_STAGING_DIR)/$(LIB) $(OUTPUT_HEADER_STAGING_DIR)/debuglog.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

