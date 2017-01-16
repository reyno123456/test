# top file directory
TOP_DIR ?= ../../..

# build file directory
BUILD_DIR ?= $(TOP_DIR)/Build

# binary file director
OUTPUT_DIR ?= $(TOP_DIR)/Output
OUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Inc
OUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Lib
OUTPUT_BIN_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Bin

# current directory
CURDIR ?= $(shell pwd)

include $(BUILD_DIR)/config.mk

###############################################################################
# static library name
LIB = libboard.a

CFG_BIN = $(HDMI_EDID_CFG_BIN_FILE_NAME)

# source file(s), including c file(s)
SRC_C := ADV7611/Src/adv_7611.c \
         NORFlash/Src/nor_flash.c \
         NORFlash/Src/w25q128.c \
         MipiCtrl/Src/ov5640_mipi.c


INCDIRS := -IADV7611/Inc \
           -INORFlash/Inc \
           -IMipiCtrl/Inc \
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
	$(CC) -c ADV7611/Src/adv_7611_edid.c -o ADV7611/Src/adv_7611_edid.o
	$(OBJCOPY) -O binary ADV7611/Src/adv_7611_edid.o ADV7611/Src/$(CFG_BIN)

install:
	@echo "Installing static library: $(LIB)"
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@mkdir -p $(OUTPUT_HEADER_STAGING_DIR)/$(CPU_NUMBER)
	@mkdir -p $(OUTPUT_BIN_STAGING_DIR)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@cp -f ADV7611/Inc/adv_7611.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f NORFlash/Inc/nor_flash.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f MipiCtrl/Inc/ov5640_mipi.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f ADV7611/Src/$(CFG_BIN) $(OUTPUT_BIN_STAGING_DIR)/

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) ADV7611/Src/adv_7611_edid.o .depend $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/$(LIB) $(OUTPUT_BIN_STAGING_DIR)/$(CFG_BIN)
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/adv_7611.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/nor_flash.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/ov5640_mipi.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

