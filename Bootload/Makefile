# top file directory
TOP_DIR ?= ..

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
	@$(MAKE) -C $(BOOT)/Boot -f Makefile clean
	@$(MAKE) -C $(BOOT)/Boot -f Makefile all
	@$(MAKE) -C $(BOOT)/Upgrade -f Makefile clean
	@$(MAKE) -C $(BOOT)/Upgrade -f Makefile all

clean:
	@$(MAKE) -C $(BOOT)/Boot -f Makefile clean
	@$(MAKE) -C $(BOOT)/Upgrade -f Makefile clean

.PHONY: all clean

#########################################################################

