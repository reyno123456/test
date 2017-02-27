#top file directory
TOP_DIR ?= ../../..

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

CPU_NUMBER = cpu2

CFLAGS += $(CPU2_CFLAGS)

# libs
LD_LIBS := -lhal -lAR8020drv -lcommon -lgcc

LDFLAGS += -Bstatic
LDFLAGS += -L$(OUTPUT_LIB_STAGING_DIR) -L$(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER) -L$(CROSS_COMPILE_LIB_PATH) $(LD_LIBS)

# source file(s), including *.s *.c file(s)
SRC_S   := $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/start.s

ifeq ($(APPLICATION_SCENARIO), sky)
    SRC_C   := main_sky.c command.c test_BB.c

endif

ifeq ($(APPLICATION_SCENARIO), ground)
    SRC_C   := main_ground.c command.c test_BB.c
endif

CFLAGS += -I$(OUTPUT_HEADER_STAGING_DIR) -I$(TOP_DIR)/Configure/Include -I../share

# object file(s)
OBJ_C   := $(patsubst %.s,%.o,$(SRC_S)) $(patsubst %.c,%.o,$(SRC_C))

# executable file
TARGET = ar8020_cpu2.bin

###############################################################################

all: .depend $(TARGET)

$(TARGET): $(OBJ_C)
	$(LD) $^ $(LDFLAGS) -T$(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/ar8020.lds -o $(TARGET).elf 
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis
	$(NM) $(TARGET).elf > $(TARGET).map
	@cp $(TARGET) $(OUTPUT_LIB_STAGING_DIR)

clean:
	rm -f $(OBJ_C) .depend 
	rm -f $(TARGET) $(TARGET).elf  $(TARGET).bin  $(TARGET).dis  $(TARGET).mif  $(TARGET).map $(OUTPUT_DIR)/$(TARGET).mif 


.PHONY: all clean $(TARGET)

#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################



