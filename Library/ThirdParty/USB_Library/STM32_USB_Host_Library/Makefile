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
LIB = libstusbhost.a

# source file(s), including c file(s)
SRC_C   := Class/AUDIO/Src/usbh_audio.c \
           Class/CDC/Src/usbh_cdc.c \
           Class/HID/Src/usbh_hid.c \
           Class/HID/Src/usbh_hid_keybd.c \
           Class/HID/Src/usbh_hid_mouse.c \
           Class/HID/Src/usbh_hid_parser.c \
           Class/MSC/Src/usbh_msc_bot.c \
           Class/MSC/Src/usbh_msc.c \
           Class/MSC/Src/usbh_msc_scsi.c \
           Class/MTP/Src/usbh_mtp.c \
           Class/MTP/Src/usbh_mtp_ptp.c \
           Core/Src/usbh_conf_template.c \
           Core/Src/usbh_core.c \
           Core/Src/usbh_ctlreq.c \
           Core/Src/usbh_ioreq.c \
           Core/Src/usbh_pipes.c

INCDIRS :=  -ICore/Inc \
            -IClass/AUDIO/Inc \
            -IClass/CDC/Inc \
            -IClass/HID/Inc \
            -IClass/MSC/Inc \
            -IClass/MTP/Inc \
            -I../CMSIS/Include \
            -I../CMSIS/Device/ST/STM32F7xx/Include \
            -I$(TOP_DIR)/Kernel/FreeRTOS/CMSIS_RTOS \
            -I$(TOP_DIR)/Kernel/FreeRTOS/include \
            -I$(TOP_DIR)/Configure/Include \
            -I$(TOP_DIR)/Kernel/FreeRTOS/portable/GCC/ARM_CM7/r0p1

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

