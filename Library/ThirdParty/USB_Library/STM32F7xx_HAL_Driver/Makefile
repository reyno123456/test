# top file directory
TOP_DIR ?= ../../../

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
LIB = libstdrvhal.a

# source file(s), including c file(s)
SRC_C := Src/stm32f7xx_hal.c \
         Src/stm32f7xx_hal_cortex.c \
         Src/stm32f7xx_hal_dma.c \
         Src/stm32f7xx_hal_dma2d.c \
         Src/stm32f7xx_hal_dma_ex.c \
         Src/stm32f7xx_hal_flash.c \
         Src/stm32f7xx_hal_flash_ex.c \
         Src/stm32f7xx_hal_gpio.c \
         Src/stm32f7xx_hal_hcd.c \
         Src/stm32f7xx_hal_i2c.c \
         Src/stm32f7xx_hal_i2c_ex.c \
         Src/stm32f7xx_hal_ltdc.c \
         Src/stm32f7xx_hal_pwr.c \
         Src/stm32f7xx_hal_pwr_ex.c \
         Src/stm32f7xx_hal_rcc.c \
         Src/stm32f7xx_hal_rcc_ex.c \
         Src/stm32f7xx_hal_sdram.c \
         Src/stm32f7xx_hal_sram.c \
         Src/stm32f7xx_hal_uart.c \
         Src/stm32f7xx_ll_fmc.c \
         Src/stm32f7xx_ll_usb.c

INCDIRS := -IInc \
           -I$(TOP_DIR)/Configure/Include \
           -I$(TOP_DIR)/Library/ThirdParty/CMSIS/Device/ST/STM32F7xx/Include \
           -I$(TOP_DIR)/Library/ThirdParty/CMSIS/Include

# object file(s)
OBJ_C   := $(patsubst %.c,%.o,$(SRC_C))

CFLAGS +=

all: .depend $(LIB) install

$(LIB):  $(OBJ_C)
	@echo "Generating static library: " $(notdir $@)
	@$(AR) $(ARFLAGS) $@ $^

install:
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) $(OBJ_CPP) .depend $(OUTPUT_LIB_STAGING_DIR)/$(LIB)

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

