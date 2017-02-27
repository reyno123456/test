# build file directory
TOP_DIR ?= ../../..

# build file directory
BUILD_DIR ?= $(TOP_DIR)/Build

# binary file director
OUTPUT_DIR ?= $(TOP_DIR)/Output
OUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Inc
OUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Lib
OUTPUT_BIN_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Bin
UTILITY_DIR ?= $(TOP_DIR)/Utility
CONFIGURE_DIR ?= $(TOP_DIR)/Configure/Include/
SDK_DIR ?= $(OUTPUT_DIR)/AR8020SDK
SDK_DIR_VERSION ?= $(OUTPUT_DIR)/"AR8020SDK_""`sed -n '1,1p' ../../../Document/version`"

# current directory
CURDIR ?= $(shell pwd)

###############################################################################

LIBALL := $(TOP_DIR)/Driver/Chip \
        $(TOP_DIR)/Driver/Board \
        $(TOP_DIR)/Driver/HAL \
        $(TOP_DIR)/Kernel/FreeRTOS \
        $(TOP_DIR)/Library/Artosyn/Common \
        $(TOP_DIR)/Library/ThirdParty/CMSIS \
        $(TOP_DIR)/Library/ThirdParty/MP3Encoder \
        $(TOP_DIR)/Bootload

LIB0 := $(TOP_DIR)/Driver/Chip \
        $(TOP_DIR)/Driver/Board \
        $(TOP_DIR)/Driver/HAL \
        $(TOP_DIR)/Kernel/FreeRTOS \
        $(TOP_DIR)/Library/Artosyn/Common \
        $(TOP_DIR)/Library/ThirdParty/CMSIS \
        $(TOP_DIR)/Library/ThirdParty/MP3Encoder \
        $(TOP_DIR)/Bootload

LIB1 := $(TOP_DIR)/Driver/Chip \
        $(TOP_DIR)/Driver/Board \
        $(TOP_DIR)/Driver/HAL \
        $(TOP_DIR)/Library/Artosyn/Common

LIB2 := $(TOP_DIR)/Driver/Chip \
        $(TOP_DIR)/Driver/Board \
        $(TOP_DIR)/Driver/HAL \
        $(TOP_DIR)/Library/Artosyn/Common

all:
	$(MAKE) clean0 CPU_NUMBER=cpu0
	$(MAKE) all0   CPU_NUMBER=cpu0
	$(MAKE) clean1 CPU_NUMBER=cpu1
	$(MAKE) all1   CPU_NUMBER=cpu1
	$(MAKE) clean2 CPU_NUMBER=cpu2
	$(MAKE) all2   CPU_NUMBER=cpu2

clean:
	$(MAKE) clean0 CPU_NUMBER=cpu0
	$(MAKE) clean1 CPU_NUMBER=cpu1
	$(MAKE) clean2 CPU_NUMBER=cpu2
	$(MAKE) -C $(APPLICATION_DIR) clean

all0: MKARGS := all
all0: $(LIB0)

all1: MKARGS := all
all1: $(LIB1)

all2: MKARGS := all
all2: $(LIB2)

PROJS ?= verification
-include ../../config.mk

MKCONFIG ?= ../../ConfigureFiles/$(addprefix config,$(addsuffix .mk,$(PROJS)))

help:
	@printf "make configure PROJS=<OPTION>  --configure project-related config.mk\n\
	If not input PROJS=<option>, it will configure verification.mk\n\
	                     <OPTION>: $(subst config,,$(subst .mk,,$(notdir $(wildcard ../../ConfigureFiles/*.mk))))\n\
	make                           --build the static library\n\
	make <OPTION>                  --build the application\n\
	                     <OPTION>: sky ground\n\
	make sdk                       --create a tar archive of the sdk directory, called AR8020SDK_<version>.tar.gz\n"
configure:
	@echo "configure makefile for $(PROJS) ..."
	@$(RM) -rf ../../config.mk
	@cp  $(MKCONFIG) ../../config.mk

$(LIBALL):
	@echo "make $(MKARGS) ..."
	@$(MAKE) -C $@ $(MKARGS)

sdk: 
	@echo "make sdk ..."
	@mkdir -p $(SDK_DIR) $(SDK_DIR)/Build $(SDK_DIR)/Utility $(SDK_DIR)/Application 
	@cp -r $(CONFIGURE_DIR) $(SDK_DIR)/Inc
	@cp -r $(OUTPUT_HEADER_STAGING_DIR) $(OUTPUT_LIB_STAGING_DIR) $(SDK_DIR)
	@cp -r $(APPLICATION_DIR)/* $(SDK_DIR)/Application
	@cp -r $(UTILITY_DIR)/joint2flash.sh $(UTILITY_DIR)/txt2bin.exe $(UTILITY_DIR)/imageinfo $(SDK_DIR)/Utility
	@cp -r $(OUTPUT_BIN_STAGING_DIR) $(SDK_DIR)/
	$(UTILITY_DIR)/modifyMKpath.sh
	@cp -r ../../*.mk $(SDK_DIR)/Build
	@tar -cvf  $(SDK_DIR_VERSION).tar.gz $(SDK_DIR)
	@rm -rf $(SDK_DIR)

ground:
	@echo "make ground ..."
	@echo $(APPLICATION_DIR)
	@$(MAKE) ground -C $(APPLICATION_DIR)
	@cp $(APPLICATION_DIR)/*.bin $(OUTPUT_DIR)

sky:
	@echo "make sky ..."
	@echo $(APPLICATION_DIR)
	@$(MAKE) sky -C $(APPLICATION_DIR)
	@cp $(APPLICATION_DIR)/*.bin $(OUTPUT_DIR)

clean0: MKARGS := clean
clean0: $(LIB0)

clean1: MKARGS := clean
clean1: $(LIB1)

clean2: MKARGS := clean
clean2: $(LIB2)

distclean: clean
	@echo "make distcleaning..."
	@$(RM) -rf  $(OUTPUT_DIR)/*
	@$(RM) -rf  ../../config.mk

.PHONY: all help sdk configure ground sky clean distclean all0 all1 all2 clean0 clean1 clean2 $(LIBALL)
