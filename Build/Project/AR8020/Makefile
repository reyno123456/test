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

LIBS := $(TOP_DIR)/Driver/Chip \
        $(TOP_DIR)/Driver/Board \
        $(TOP_DIR)/Kernel/FreeRTOS \
        $(TOP_DIR)/Library/Artosyn/Common \
        $(TOP_DIR)/Library/ThirdParty/CMSIS \
        $(TOP_DIR)/Library/ThirdParty/USB_Library \
        $(TOP_DIR)/Bootload/AR8020

all: MKARGS := all
all: $(LIBS)

clean: MKARGS := clean
clean: $(LIBS)

$(LIBS):
	@echo "make $(MKARGS) ..."
	@$(MAKE) -C $@ $(MKARGS)

distclean: clean
	@echo "make distcleaning..."
	@$(RM) -rf  $(OUTPUT_DIR)/*

.PHONY: all clean distclean $(LIBS)
