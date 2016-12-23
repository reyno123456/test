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

all: install

install:
	@mkdir -p $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Device/ST/STM32F7xx/Include/stm32f746xx.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Include/core_cm7.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Include/core_cmInstr.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Include/core_cmFunc.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Include/core_cmSimd.h $(OUTPUT_HEADER_STAGING_DIR)

clean:
	@echo "Cleaning..."
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/stm32f746xx.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/core_cm7.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/core_cmInstr.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/core_cmFunc.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/core_cmSimd.h

.PHONY: all clean install

#########################################################################

