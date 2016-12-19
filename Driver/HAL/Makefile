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
# static library name
LIB = libhal.a

# source file(s), including c file(s)
SRC_C := Src/hal_i2c.c

INCDIRS := -IInc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/I2C/Inc

# object file(s)
OBJ_C     := $(patsubst %.c,%.o,$(SRC_C))

all: .depend $(LIB) install

$(LIB): $(OBJ_C)
	@echo "Generating static library: " $(notdir $@)
	$(AR) $(ARFLAGS) $@ $(OBJ_C)

install:
	@echo "Installing static library: $(LIB)"
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)
	@cp -f Inc/hal_i2c.h $(OUTPUT_HEADER_STAGING_DIR)

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) .depend $(OUTPUT_LIB_STAGING_DIR)/$(LIB)
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_i2c.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

