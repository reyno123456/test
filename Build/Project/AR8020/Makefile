# build file directory
TOP_DIR ?= ../../..

# build file directory
BUILD_DIR ?= $(TOP_DIR)/Build

# binary file director
OUTPUT_DIR ?= $(TOP_DIR)/Output
OUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Inc
OUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Lib
UTILITY_DIR ?= $(TOP_DIR)/Utility
CONFIGURE_DIR ?= $(TOP_DIR)/Configure/Include/
SDK_DIR ?= $(OUTPUT_DIR)/AR8020SDK
SDK_DIR_VERSION ?= $(OUTPUT_DIR)/"AR8020SDK_""`sed -n '1,1p' ../../../Document/Version`"

# current directory
CURDIR ?= $(shell pwd)

###############################################################################

LIBS := $(TOP_DIR)/Driver/Chip \
        $(TOP_DIR)/Driver/Board \
        $(TOP_DIR)/Kernel/FreeRTOS \
        $(TOP_DIR)/Library/Artosyn/Common \
        $(TOP_DIR)/Library/ThirdParty/CMSIS \
        $(TOP_DIR)/Bootload/
        # $(TOP_DIR)/Bootload/AR8020/Upgrade \
        $(TOP_DIR)/Bootload/AR8020/Origin

all: MKARGS := all
all: $(LIBS)

clean: MKARGS := clean

PROJS ?= verification
include ../../ConfigureFiles/$(addprefix config,$(addsuffix .mk,$(PROJS)))

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

$(LIBS):
	@echo "make $(MKARGS) ..."
	@$(MAKE) -C $@ $(MKARGS)

sdk: 
	@echo "make sdk ..."
	@mkdir -p $(SDK_DIR) $(SDK_DIR)/Build $(SDK_DIR)/Utility $(SDK_DIR)/Application
	@cp -r $(CONFIGURE_DIR) $(SDK_DIR)/Inc
	@cp -r $(OUTPUT_HEADER_STAGING_DIR) $(OUTPUT_LIB_STAGING_DIR) $(SDK_DIR)
	@cp -r $(APPLICATION_DIR)/* $(SDK_DIR)/Application
	@cp -r $(UTILITY_DIR)/joint2flash.sh $(UTILITY_DIR)/txt2bin.exe $(UTILITY_DIR)/imageinfo $(SDK_DIR)/Utility
	$(UTILITY_DIR)/modifyMKpath.sh
	@cp -r ../../*.mk $(SDK_DIR)/Build
	@tar -cvf  $(SDK_DIR_VERSION).tar.gz $(SDK_DIR)
	@rm -rf $(SDK_DIR)

ground: $(LIBS)
	@echo "make ground ..."
	@echo $(APPLICATION_DIR)
	@$(MAKE) ground -C $(APPLICATION_DIR)
	@cp $(APPLICATION_DIR)/*.bin $(OUTPUT_DIR)

sky: $(LIBS)
	@echo "make sky ..."
	@echo $(APPLICATION_DIR)
	@$(MAKE) sky -C $(APPLICATION_DIR)
	@cp $(APPLICATION_DIR)/*.bin $(OUTPUT_DIR)

clean: $(LIBS)
	@$(MAKE) -C $(APPLICATION_DIR) clean 

distclean: clean
	@echo "make distcleaning..."
	@$(RM) -rf  $(OUTPUT_DIR)/*
	@$(RM) -rf  ../../config.mk

.PHONY: all help sdk configure ground sky clean distclean $(LIBS)
