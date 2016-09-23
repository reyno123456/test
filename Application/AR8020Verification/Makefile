#top file directory
TOP_DIR ?= ../..

# build file directory
BUILD_DIR ?= $(TOP_DIR)/Build

# binary file director
OUTPUT_DIR ?= $(TOP_DIR)/Output
OUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Inc
OUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Lib

# utility file director
UTILITY_DIR ?= $(TOP_DIR)/Utility

# current directory
CURDIR ?= $(shell pwd)

include $(BUILD_DIR)/config.mk

###############################################################################

# executable file
TARGET = ar8020_all
TARGET_CPU0 = ar8020_cpu0
TARGET_CPU1 = ar8020_cpu1
TARGET_CPU2 = ar8020_cpu2

###############################################################################

all: .depend $(TARGET)

$(TARGET): $(TARGET_CPU0) $(TARGET_CPU2) 

$(TARGET_CPU0):
	@$(MAKE) -C cpu0 all

$(TARGET_CPU1):
	@$(MAKE) -C cpu1 all

$(TARGET_CPU2):
	@$(MAKE) -C cpu2 all


clean:
	@$(MAKE) -C cpu0 clean
	@$(MAKE) -C cpu1 clean
	@$(MAKE) -C cpu2 clean


.PHONY: all clean $(TARGET) $(TARGET_CPU0) $(TARGET_CPU1) $(TARGET_CPU2)

#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################



