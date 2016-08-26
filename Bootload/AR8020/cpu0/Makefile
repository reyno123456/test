#top file directory
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
LIB = libbootload.a

CPU_NUMBER = cpu0

# source file(s), including c file(s)
SRC_C   := start.c

INCDIRS := -I$(TOP_DIR)/Library/Artosyn/Common/Inc
 
# object file(s)
OBJ_C   := $(patsubst %.c,%.o,$(SRC_C))

CFLAGS +=

all: .depend $(LIB) $(LIB_2) install

$(LIB):  $(OBJ_C)
	@echo "Generating static library: " $(notdir $@)
	@$(AR) $(ARFLAGS) $@ $^

install:
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@cp -f ar8020.lds $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(LIB_2) $(OBJ_C) .depend $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/$(LIB) $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/ar8020.lds

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

