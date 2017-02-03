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

CFLAGS += -O1

CPU_NUMBER = cpu0

# libs
LD_LIBS := -lhal -lfreertos -lboard -lAR8020drv -lcommon -lgcc

LDFLAGS += -Bstatic
LDFLAGS += -L$(OUTPUT_LIB_STAGING_DIR) -L$(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER) -L$(CROSS_COMPILE_LIB_PATH) $(LD_LIBS)

# source file(s), including *.s *.c file(s)
SRC_S   := $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/start.s

ifeq ($(APPLICATION_SCENARIO), sky)
SRC_C   := main_sky.c command.c test_freertos.c test_hal_can.c test_ov5640.c ../share/test_i2c_adv7611.c \
           test_usbh.c test_sd.c test_bbuartcom.c\
           ../share/test_timer.c ../share/test_spi.c test_quadspi.c ../share/test_float.c ../share/test_gpio.c \
           upgrade.c md5.c ../share/testhal_gpio.c ../share/testhal_timer.c ../share/testhal_pwm.c \
           ../share/testhal_softpwm.c ../share/testhal_i2c.c test_hal_uart.c test_hal_spi.c wireless_interface.c
endif

ifeq ($(APPLICATION_SCENARIO), ground)
SRC_C   := main_ground.c command.c test_freertos.c test_hal_can.c test_ov5640.c ../share/test_i2c_adv7611.c \
           test_usbh.c test_sd.c test_bbuartcom.c\
           ../share/test_timer.c ../share/test_spi.c test_quadspi.c ../share/test_float.c ../share/test_gpio.c \
           upgrade.c md5.c ../share/testhal_gpio.c ../share/testhal_timer.c ../share/testhal_pwm.c\
           ../share/testhal_softpwm.c ../share/testhal_i2c.c test_hal_uart.c test_hal_spi.c wireless_interface.c
endif

CFLAGS += -I$(OUTPUT_HEADER_STAGING_DIR) -I$(TOP_DIR)/Configure/Include -I../share

# object file(s)
OBJ_C   := $(patsubst %.s,%.o,$(SRC_S)) $(patsubst %.c,%.o,$(SRC_C))

# executable file
TARGET = ar8020_cpu0.bin

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
	rm -f $(OUTPUT_LIB_STAGING_DIR)/$(TARGET)


.PHONY: all clean $(TARGET)

#########################################################################

include $(BUILD_DIR)/rules.mk

ifneq "$(MAKECMDGOALS)" "clean"
sinclude .depend
endif 

#########################################################################



