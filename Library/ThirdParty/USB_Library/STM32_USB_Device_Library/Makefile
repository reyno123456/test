# build file directory
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
LIB = libstusbdevice.a

# source file(s), including c file(s)
SRC_C   := Core/Src/usbd_conf_template.c \
           Core/Src/usbd_core.c \
           Core/Src/usbd_ctlreq.c \
           Core/Src/usbd_ioreq.c \
           Class/Template/Src/usbd_template.c
ifeq ($(USB_DEV_CLASS_AUDIO_ENABLE), 1)
SRC_C   += Class/AUDIO/Src/usbd_audio.c \
           Class/AUDIO/Src/usbd_audio_if_template.c
endif
ifeq ($(USB_DEV_CLASS_CDC_ENABLE), 1)
SRC_C   += Class/CDC/Src/usbd_cdc.c \
           Class/CDC/Src/usbd_cdc_if_template.c
endif
ifeq ($(USB_DEV_CLASS_CUSTOMHID_ENABLE), 1)
SRC_C   += Class/CustomHID/Src/usbd_customhid.c \
           Class/CustomHID/Src/usbd_customhid_if_template.c
endif
ifeq ($(USB_DEV_CLASS_DFU_ENABLE), 1)
SRC_C   += Class/DFU/Src/usbd_dfu.c \
           Class/DFU/Src/usbd_dfu_media_template.c
endif
ifeq ($(USB_DEV_CLASS_HID_ENABLE), 1)
SRC_C   += Class/HID/Src/usbd_hid.c
endif
ifeq ($(USB_DEV_CLASS_MSC_ENABLE), 1)
SRC_C   += Class/MSC/Src/usbd_msc_bot.c \
           Class/MSC/Src/usbd_msc.c \
           Class/MSC/Src/usbd_msc_data.c \
           Class/MSC/Src/usbd_msc_scsi.c \
           Class/MSC/Src/usbd_msc_storage_template.c
endif

INCDIRS :=  -ICore/Inc \
            -IClass/AUDIO/Inc \
            -IClass/CDC/Inc \
            -IClass/CustomHID/Inc \
            -IClass/DFU/Inc \
            -IClass/HID/Inc \
            -IClass/MSC/Inc \
            -IClass/Template/Inc \
            -I$(TOP_DIR)/Configure/Include \
            -I$(TOP_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Inc \
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

