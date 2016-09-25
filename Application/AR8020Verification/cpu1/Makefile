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

CPU_NUMBER = cpu1

# libs
LD_LIBS := -lbootload -lAR8020drv -lcommon -lgcc

LDFLAGS += -Bstatic
LDFLAGS += -L$(OUTPUT_LIB_STAGING_DIR) -L$(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER) -L$(CROSS_COMPILE_LIB_PATH) $(LD_LIBS)

# source file(s), including c file(s)
SRC_C   := main.c

CFLAGS += -I$(OUTPUT_HEADER_STAGING_DIR) -I$(TOP_DIR)/Configure/Include -I../share

# object file(s)
OBJ_C   := $(patsubst %.c,%.o,$(SRC_C))

# executable file
TARGET = ar8020_cpu1
TARGET_FLASH = ar8020_cpu1.txt

###############################################################################

all: .depend $(TARGET)

$(TARGET): $(OBJ_C)
	$(LD) $^ $(LDFLAGS) -T$(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/ar8020.lds -o $(TARGET).elf 
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis
	$(NM) $(TARGET).elf > $(TARGET).map
	#$(UTILITY_DIR)/bin2mif $@ $(TARGET).mif 32768 64
	#cp $(TARGET).mif  $(OUTPUT_DIR)
	#perl ../../../Utility/mif2ini.pl  $(TARGET).mif
	#mv itcm_init.txt  itcm_init_cpu1.txt

	#for flash boot
	../../../Utility/bin2flash.exe  -f $(TARGET)  -o  $(TARGET_FLASH)
	#cp $(TARGET).map  $(TARGET).elf itcm_init_cpu1.txt /SCRATCH/lab06/Zebu/Zebu_UC_20160531_formin/EMU/RUN_SD/m7_elf/freeatos_itcm/
	#cp load_spiflash_cpu1.txt /SCRATCH/lab06/Zebu/Zebu_UC_20160531_formin/EMU/RUN_SD
clean:
	rm -f $(OBJ_C) .depend 
	rm -f $(TARGET) $(TARGET).elf  $(TARGET).bin  $(TARGET).dis  $(TARGET).mif  $(TARGET).map $(OUTPUT_DIR)/$(TARGET).mif 
	rm -f $(TARGET_FLASH)


.PHONY: all clean $(TARGET)

#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################



