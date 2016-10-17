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
LIB = libboard.a

# source file(s), including c file(s)
SRC_C := ADV7611/Src/adv_7611.c \
         NorFlash/Src/nor_flash.c \
         NorFlash/Src/w25q128.c

INCDIRS := -IADV7611/Inc \
           -INorFlash/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/I2C/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/QuadSPI/Inc \
           -I$(TOP_DIR)/Configure \
           -I$(TOP_DIR)/Configure/Include \
           -I$(TOP_DIR)/Library/Artosyn/Common/Inc

# object file(s)
OBJ_C     := $(patsubst %.c,%.o,$(SRC_C))

all: .depend $(LIB) install

$(LIB): $(OBJ_C)
	@echo "Generating static library: " $(notdir $@)
	$(AR) $(ARFLAGS) $@ $(OBJ_C)

install:
	@echo "Installing static library: $(LIB)"
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)
	@mkdir -p $(OUTPUT_HEADER_STAGING_DIR)/$(CPU_NUMBER)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@cp -f ADV7611/Inc/adv_7611.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f NorFlash/Inc/nor_flash.h $(OUTPUT_HEADER_STAGING_DIR)

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) .depend $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/$(LIB)
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/adv_7611.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/nor_flash.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

