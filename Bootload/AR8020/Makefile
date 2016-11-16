# top file directory
TOP_DIR ?= ../..

# build file directory
BUILD_DIR ?= $(TOP_DIR)/Build

# binary file director
OUTPUT_DIR ?= $(TOP_DIR)/Output
OUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Inc
OUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Lib
SRC_C_DIR ?= $(TOP_DIR)/Driver/Chip/AR8020

# utility file director
UTILITY_DIR ?= $(TOP_DIR)/Utility

# current directory
CURDIR ?= $(shell pwd)

include $(BUILD_DIR)/config.mk

###############################################################################
# libs
CPU_NUMBER := cpu0
#LD_LIBS := -lfreertos -lAR8020drv -lcommon  -lusb
LD_LIBS := -lAR8020drv -lcommon -lboard -lfreertos

LDFLAGS += -Bstatic
LDFLAGS += -L$(OUTPUT_LIB_STAGING_DIR) -L$(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER) $(LD_LIBS)
                 
# source file(s), including c file(s)
SRC_S   :=  Src/start.s 
SRC_C   :=  Src/main.c \
			Src/command.c \
			Src/sd_boot.c \
			Src/boot.c \
			$(SRC_C_DIR)/SD/Src/sd_core.c \
         	$(SRC_C_DIR)/SD/Src/sd_card.c \
         	$(SRC_C_DIR)/SD/Src/sd_host.c \

CFLAGS +=   -I./Inc \
			-I$(OUTPUT_HEADER_STAGING_DIR) \
			-I$(TOP_DIR)/Configure/Include \
			-I$(SRC_C_DIR)/SD/Inc \
			-I$(SRC_C_DIR)/SysCtl/Inc \
			-I$(TOP_DIR)/Library/ThirdParty/CMSIS/Device/ST/STM32F7xx/Include \
            -I$(TOP_DIR)/Library/ThirdParty/CMSIS/Include \

# object file(s)
OBJ_C   := $(patsubst %.s,%.o,$(SRC_S)) $(patsubst %.c,%.o,$(SRC_C)) 

#$(patsubst %.s,%.o,$(SRC_C))

# executable file
TARGET = ar8020_boot
TARGET_FLASH = ar8020_boot.txt

###############################################################################

all: .depend $(TARGET) install

$(TARGET): $(OBJ_C)
	$(LD) $^ $(LDFLAGS) -TSrc/ar8020_boot.lds -o $(TARGET).elf 
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D -S $(TARGET).elf > $(TARGET).dis
	$(NM) $(TARGET).elf > $(TARGET).map
	../../Utility/bin2flash.exe -f $(TARGET) -o $(TARGET_FLASH)

install:
	@cp $(TARGET_FLASH) $(OUTPUT_LIB_STAGING_DIR)/ 
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/cpu0 
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/cpu1 
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/cpu2
	@cp -f cpu0/ar8020.lds $(OUTPUT_LIB_STAGING_DIR)/cpu0/
	@cp -f cpu0/start.s $(OUTPUT_LIB_STAGING_DIR)/cpu0/
	@cp -f cpu1/ar8020.lds $(OUTPUT_LIB_STAGING_DIR)/cpu1/
	@cp -f cpu1/start.s $(OUTPUT_LIB_STAGING_DIR)/cpu1/
	@cp -f cpu2/ar8020.lds $(OUTPUT_LIB_STAGING_DIR)/cpu2/
	@cp -f cpu2/start.s $(OUTPUT_LIB_STAGING_DIR)/cpu2/
	cp ar8020_boot /LnxShare/zyma/ar8020_boot.bin

clean:
	rm -f $(OBJ_C) .depend 
	rm -f $(TARGET) $(TARGET).elf  $(TARGET).bin  $(TARGET).dis  $(TARGET).mif  $(TARGET).map $(OUTPUT_DIR)/$(TARGET).mif $(TARGET_FLASH)
	rm -f $(OUTPUT_LIB_STAGING_DIR)/$(TARGET_FLASH)
	rm -f $(OUTPUT_LIB_STAGING_DIR)/cpu0/ar8020.lds $(OUTPUT_LIB_STAGING_DIR)/cpu0/start.s
	rm -f $(OUTPUT_LIB_STAGING_DIR)/cpu1/ar8020.lds $(OUTPUT_LIB_STAGING_DIR)/cpu1/start.s
	rm -f $(OUTPUT_LIB_STAGING_DIR)/cpu2/ar8020.lds $(OUTPUT_LIB_STAGING_DIR)/cpu2/start.s

.PHONY: all clean $(TARGET)

#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################



