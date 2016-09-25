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
TARGET_FLASH = ar8020_flash

###############################################################################

all: .depend $(TARGET)

$(TARGET): $(TARGET_CPU0) $(TARGET_CPU1) $(TARGET_CPU2) 


$(TARGET_CPU0):
	@$(MAKE) -C cpu0 all

$(TARGET_CPU1):
	@$(MAKE) -C cpu1 all

$(TARGET_CPU2):
	@$(MAKE) -C cpu2 all

install:
	@cp ./cpu0/$(TARGET_CPU0).txt  .
	@cp ./cpu1/$(TARGET_CPU1).txt  .
	@cp ./cpu2/$(TARGET_CPU2).txt  .
	../../Utility/joint2flash.sh -i $(TARGET_CPU0).txt $(TARGET_CPU1).txt $(TARGET_CPU2).txt -o $(TARGET_FLASH)
	@cp $(TARGET_FLASH) ../../Output

clean:
	@$(MAKE) -C cpu0 clean
	@$(MAKE) -C cpu1 clean
	@$(MAKE) -C cpu2 clean
	@rm -rf $(TARGET_CPU0).txt $(TARGET_CPU1).txt $(TARGET_CPU2).txt 


.PHONY: all clean $(TARGET) $(TARGET_CPU0) $(TARGET_CPU1) $(TARGET_CPU2)

#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################



