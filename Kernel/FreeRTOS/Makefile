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
LIB = libfreertos.a

# source file(s), including c file(s)
SRC_C   := croutine.c \
           event_groups.c \
           list.c \
           queue.c \
           tasks.c \
           timers.c \
           portable/GCC/ARM_CM7/r0p1/port.c \
           portable/MemMang/heap_4.c \
           CMSIS_RTOS/cmsis_os.c \
           FatFs/Src/diskio.c \
           FatFs/Src/ff.c \
           FatFs/Src/ff_gen_drv.c \
           FatFs/Port/sd_diskio.c


INCDIRS := -Iinclude \
           -Iportable/GCC/ARM_CM7/r0p1 \
           -ICMSIS_RTOS \
           -I$(TOP_DIR)/Configure/Include \
           -I$(TOP_DIR)/Library/ThirdParty/USB_Library/STM32F7xx_HAL_Driver/Inc \
           -I$(TOP_DIR)/Library/ThirdParty/CMSIS/Include


INCDIRS +=  -IFatFs/Inc \
            -IFatFs/Port \
            -I$(TOP_DIR)/Library/ThirdParty/CMSIS/Device/ST/STM32F7xx/Include \
            -I$(TOP_DIR)/Library/ThirdParty/CMSIS/Include \
            -I$(TOP_DIR)/Driver/Chip/AR8020/UART/Inc \
            -I$(TOP_DIR)/Driver/Chip/AR8020/SD/Inc \
            -I$(TOP_DIR)/Library/Artosyn/Common/Inc

 
# object file(s)
OBJ_C   := $(patsubst %.c,%.o,$(SRC_C))

CFLAGS +=

all: .depend $(LIB) install

$(LIB):  $(OBJ_C)
	@echo "Generating static library: " $(notdir $@)
	@$(AR) $(ARFLAGS) $@ $^

install:
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)
	@mkdir -p $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)
	@cp -f include/FreeRTOS.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f include/projdefs.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f include/portable.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f include/deprecated_definitions.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f portable/GCC/ARM_CM7/r0p1/portmacro.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f include/mpu_wrappers.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f include/task.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f include/list.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f CMSIS_RTOS/cmsis_os.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f include/timers.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f include/queue.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f include/semphr.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f include/event_groups.h $(OUTPUT_HEADER_STAGING_DIR)

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) $(OBJ_CPP) .depend $(OUTPUT_LIB_STAGING_DIR)/$(LIB)
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/FreeRTOS.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/projdefs.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/portable.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/deprecated_definitions.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/portmacro.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/mpu_wrappers.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/task.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/list.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/cmsis_os.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/timers.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/queue.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/semphr.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/event_groups.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

