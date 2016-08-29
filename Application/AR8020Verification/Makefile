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

CPU_NUMBER = cpu0

# libs
LD_LIBS := -lbootload -lfreertos -lusb -lAR8020drv -lcommon 

LDFLAGS += -Bstatic
LDFLAGS += -L$(OUTPUT_LIB_STAGING_DIR) -L$(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER) $(LD_LIBS)

# source file(s), including c file(s)
SRC_C   := main.c test_freertos.c

CFLAGS += -I$(OUTPUT_HEADER_STAGING_DIR) -I$(TOP_DIR)/Configure/Include

# object file(s)
OBJ_C   := $(patsubst %.c,%.o,$(SRC_C))

# executable file
TARGET = ar8020

###############################################################################

all: .depend $(TARGET)

$(TARGET): $(OBJ_C)
	$(LD) $^ $(LDFLAGS) -T$(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/$(TARGET).lds -o $(TARGET).elf 
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis
	$(NM) $(TARGET).elf > $(TARGET).map
	$(UTILITY_DIR)/bin2mif $@ $(TARGET).mif 32768 64
	cp $(TARGET).mif  $(OUTPUT_DIR)

clean:
	rm -f $(OBJ_C) .depend 
	rm -f $(TARGET) $(TARGET).elf  $(TARGET).bin  $(TARGET).dis  $(TARGET).mif  $(TARGET).map $(OUTPUT_DIR)/$(TARGET).mif


.PHONY: all clean $(TARGET)

#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################



