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

all:
	@$(MAKE) -C cpu0 all
	@$(MAKE) -C cpu1 all
	@$(MAKE) -C cpu2 all

clean:
	@$(MAKE) -C cpu0 clean
	@$(MAKE) -C cpu1 clean
	@$(MAKE) -C cpu2 clean

.PHONY: all clean

#########################################################################

