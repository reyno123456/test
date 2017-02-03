# top file directory
TOP_DIR ?= ../..

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
LIB = libhal.a

# source file(s), including c file(s)
SRC_C := Src/hal.c \
         Src/hal_i2c.c \
         Src/hal_sys_ctl.c \
         Src/hal_hdmi_rx.c \
         Src/hal_gpio.c \
         Src/hal_timer.c \
         Src/hal_pwm.c \
         Src/hal_softpwm.c \
         Src/hal_sram.c \
         Src/hal_h264.c \
         Src/hal_usb_host.c \
         Src/hal_usb_device.c \
         Src/hal_usb_otg.c \
         Src/hal_uart.c \
         Src/hal_spi.c \
         Src/hal_bb.c \
         Src/hal_nvic.c \
	 Src/hal_sd.c \
	 Src/hal_dma.c \
	 Src/hal_adc.c \
         Src/hal_nv.c \

INCDIRS := -IInc \
           -I$(TOP_DIR)/Configure/Include \
           -I$(TOP_DIR)/Library/Artosyn/Common/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/I2C/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SysCtl/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/BB/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SRAM/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SD/Inc \
           -I$(TOP_DIR)/Driver/Board/AR8020TEST/ADV7611/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/H264Encoder/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_Device/Core/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_Device/Class/HID/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_LL_Driver/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_Host/Core/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_Host/Class/MSC/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_Host/Class/UVC/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SPI/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/UART/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/DMA/Inc \
           -I$(TOP_DIR)/Driver/Board/AR8020TEST/NORFlash/Inc \
           -I$(TOP_DIR)/Driver/HAL/Inc


# object file(s)
OBJ_C     := $(patsubst %.c,%.o,$(SRC_C))

all: .depend $(LIB) install

$(LIB): $(OBJ_C)
	@echo "Generating static library: " $(notdir $@)
	$(AR) $(ARFLAGS) $@ $(OBJ_C)

install:
	@echo "Installing static library: $(LIB)"
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)
	@mkdir -p $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)
	@cp -f Inc/hal_i2c.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_sys_ctl.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_hdmi_rx.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_sram.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_gpio.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_timer.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_pwm.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_softpwm.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_h264.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_timer.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_ret_type.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_usb_host.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_usb_device.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_usb_otg.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_uart.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_spi.h $(OUTPUT_HEADER_STAGING_DIR) 
	@cp -f Inc/hal_bb.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_nvic.h $(OUTPUT_HEADER_STAGING_DIR) 
	@cp -f Inc/hal_sd.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_dma.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_adc.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_nv.h  $(OUTPUT_HEADER_STAGING_DIR)
clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) .depend $(OUTPUT_LIB_STAGING_DIR)/$(LIB)
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_i2c.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_sys_ctl.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_hdmi_rx.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_sram.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_uart.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_spi.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_bb.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_sd.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_dma.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_adc.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_nv.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_usb_host.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_usb_device.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_usb_otg.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

