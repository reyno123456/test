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
LIB = libusb.a

# source file(s), including c file(s)
SRC_C   := STM32_USB_Host_Library/Class/MSC/Src/usbh_msc_bot.c \
           STM32_USB_Host_Library/Class/MSC/Src/usbh_msc.c \
           STM32_USB_Host_Library/Class/MSC/Src/usbh_msc_scsi.c \
           STM32_USB_Host_Library/Core/Src/usbh_conf.c \
           STM32_USB_Host_Library/Core/Src/usbh_core.c \
           STM32_USB_Host_Library/Core/Src/usbh_ctlreq.c \
           STM32_USB_Host_Library/Core/Src/usbh_ioreq.c \
           STM32_USB_Host_Library/Core/Src/usbh_pipes.c

#SRC_C   := STM32_USB_Host_Library/Class/AUDIO/Src/usbh_audio.c \
           STM32_USB_Host_Library/Class/CDC/Src/usbh_cdc.c \
           STM32_USB_Host_Library/Class/HID/Src/usbh_hid.c \
           STM32_USB_Host_Library/Class/HID/Src/usbh_hid_keybd.c \
           STM32_USB_Host_Library/Class/HID/Src/usbh_hid_mouse.c \
           STM32_USB_Host_Library/Class/HID/Src/usbh_hid_parser.c \
           STM32_USB_Host_Library/Class/MSC/Src/usbh_msc_bot.c \
           STM32_USB_Host_Library/Class/MSC/Src/usbh_msc.c \
           STM32_USB_Host_Library/Class/MSC/Src/usbh_msc_scsi.c \
           STM32_USB_Host_Library/Class/MTP/Src/usbh_mtp.c \
           STM32_USB_Host_Library/Class/MTP/Src/usbh_mtp_ptp.c \
           STM32_USB_Host_Library/Core/Src/usbh_conf.c \
           STM32_USB_Host_Library/Core/Src/usbh_core.c \
           STM32_USB_Host_Library/Core/Src/usbh_ctlreq.c \
           STM32_USB_Host_Library/Core/Src/usbh_ioreq.c \
           STM32_USB_Host_Library/Core/Src/usbh_pipes.c

INCDIRS :=  -ISTM32_USB_Host_Library/Core/Inc \
            -ISTM32_USB_Host_Library/Class/AUDIO/Inc \
            -ISTM32_USB_Host_Library/Class/CDC/Inc \
            -ISTM32_USB_Host_Library/Class/HID/Inc \
            -ISTM32_USB_Host_Library/Class/MSC/Inc \
            -ISTM32_USB_Host_Library/Class/MTP/Inc \
            -I../CMSIS/Include \
            -I../CMSIS/Device/ST/STM32F7xx/Include \
            -I$(TOP_DIR)/Kernel/FreeRTOS/CMSIS_RTOS \
            -I$(TOP_DIR)/Kernel/FreeRTOS/include \
            -I$(TOP_DIR)/Configure/Include \
            -I$(TOP_DIR)/Kernel/FreeRTOS/portable/GCC/ARM_CM7/r0p1 \
            -I../../Artosyn/Common/Inc

SRC_C   += STM32_USB_Device_Library/Core/Src/usbd_conf.c \
           STM32_USB_Device_Library/Core/Src/usbd_core.c \
           STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
           STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
ifeq ($(USB_DEV_CLASS_AUDIO_ENABLE), 1)
SRC_C   += STM32_USB_Device_Library/Class/AUDIO/Src/usbd_audio.c \
           STM32_USB_Device_Library/Class/AUDIO/Src/usbd_audio_if_template.c
endif
ifeq ($(USB_DEV_CLASS_CDC_ENABLE), 1)
SRC_C   += STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c \
           STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc_if_template.c
endif
ifeq ($(USB_DEV_CLASS_CUSTOMHID_ENABLE), 1)
SRC_C   += STM32_USB_Device_Library/Class/CustomHID/Src/usbd_customhid.c \
           STM32_USB_Device_Library/Class/CustomHID/Src/usbd_customhid_if_template.c
endif
ifeq ($(USB_DEV_CLASS_DFU_ENABLE), 1)
SRC_C   += STM32_USB_Device_Library/Class/DFU/Src/usbd_dfu.c \
           STM32_USB_Device_Library/Class/DFU/Src/usbd_dfu_media_template.c
endif
ifeq ($(USB_DEV_CLASS_HID_ENABLE), 1)
SRC_C   += STM32_USB_Device_Library/Class/HID/Src/usbd_hid.c \
           STM32_USB_Device_Library/Class/HID/Src/usbd_hid_desc.c
endif
ifeq ($(USB_DEV_CLASS_MSC_ENABLE), 1)
SRC_C   += STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_bot.c \
           STM32_USB_Device_Library/Class/MSC/Src/usbd_msc.c \
           STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_data.c \
           STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_scsi.c \
           STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_storage_template.c
endif

INCDIRS +=  -ISTM32_USB_Device_Library/Core/Inc \
            -ISTM32_USB_Device_Library/Class/AUDIO/Inc \
            -ISTM32_USB_Device_Library/Class/CDC/Inc \
            -ISTM32_USB_Device_Library/Class/CustomHID/Inc \
            -ISTM32_USB_Device_Library/Class/DFU/Inc \
            -ISTM32_USB_Device_Library/Class/HID/Inc \
            -ISTM32_USB_Device_Library/Class/MSC/Inc \
            -ISTM32_USB_Device_Library/Class/Template/Inc \
            -ISTM32F7xx_HAL_Driver/Inc

SRC_C += STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c \
         STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c \
         STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_usb.c \
         STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pcd.c \
         STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pcd_ex.c \
         STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_hcd.c \
         STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c \
         STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c

INCDIRS += -ISTM32F7xx_HAL_Driver/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SRAM/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/WirelessInfo/Inc



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
	@mkdir -p $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Device_Library/Core/Inc/usbd_core.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Device_Library/Core/Inc/usbd_conf.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Device_Library/Core/Inc/usbd_def.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Device_Library/Core/Inc/usbd_ctlreq.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Device_Library/Core/Inc/usbd_ioreq.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Device_Library/Class/HID/Inc/usbd_hid.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Device_Library/Class/HID/Inc/usbd_hid_desc.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_def.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_rcc.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_rcc_ex.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_cortex.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_pcd.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_pcd_ex.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_hcd.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32F7xx_HAL_Driver/Inc/stm32f7xx_ll_usb.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Host_Library/Core/Inc/usbh_core.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Host_Library/Core/Inc/usbh_conf.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Host_Library/Core/Inc/usbh_def.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Host_Library/Core/Inc/usbh_ctlreq.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Host_Library/Core/Inc/usbh_ioreq.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Host_Library/Core/Inc/usbh_pipes.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Host_Library/Class/MSC/Inc/usbh_msc.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Host_Library/Class/MSC/Inc/usbh_msc_bot.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f STM32_USB_Host_Library/Class/MSC/Inc/usbh_msc_scsi.h  $(OUTPUT_HEADER_STAGING_DIR)



clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) $(OBJ_CPP) .depend $(OUTPUT_LIB_STAGING_DIR)/$(LIB)

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

