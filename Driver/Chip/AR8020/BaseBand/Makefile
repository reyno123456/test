CROSS_COMPILE = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/bin/arm-none-eabi-

TARGET = baseband

PWD = $(shell pwd)
WORK_DIR := $(PWD)/../../../..

OUTPUT_DIR := $(WORK_DIR)/Output

UTILITY_DIR := $(WORK_DIR)/Utility
HAL_DIR := $(WORK_DIR)/Library/ThirdParty/USB_Library/STM32F7xx_HAL_Driver/Src
BASEBAND_DRV_DIR := $(WORK_DIR)/Driver/Chip/AR8020/BaseBand

OBJS_APP =	$(BASEBAND_DRV_DIR)/config_baseband_frqdata.o \
			$(BASEBAND_DRV_DIR)/grd_controller.o \
			$(BASEBAND_DRV_DIR)/main.o \
			$(BASEBAND_DRV_DIR)/sky_controller.o \
			$(BASEBAND_DRV_DIR)/sys_peripheral_init.o \
			$(BASEBAND_DRV_DIR)/sys_peripheral_communication.o

OBJS_LIB_ARTOSYN = $(WORK_DIR)/Library/Artosyn/Common/Src/interrupt.o \
                   $(WORK_DIR)/Library/Artosyn/Common/Src/debuglog.o \
		   $(WORK_DIR)/Library/Artosyn/Common/Src/command.o \
		   $(WORK_DIR)/Library/Artosyn/Common/Src/printf.o \
                   $(WORK_DIR)/Library/Artosyn/Common/Src/string.o

OBJS_DRIVER_HAL = $(HAL_DIR)/stm32f7xx_hal.o \
                  $(HAL_DIR)/stm32f7xx_hal_cortex.o \
				  $(HAL_DIR)/stm32f7xx_hal_dma.o \
                  $(HAL_DIR)/stm32f7xx_hal_dma2d.o \
                  $(HAL_DIR)/stm32f7xx_hal_dma_ex.o \
                  $(HAL_DIR)/stm32f7xx_hal_flash.o \
                  $(HAL_DIR)/stm32f7xx_hal_flash_ex.o \
                  $(HAL_DIR)/stm32f7xx_hal_gpio.o \
                  $(HAL_DIR)/stm32f7xx_hal_hcd.o \
                  $(HAL_DIR)/stm32f7xx_hal_i2c.o \
                  $(HAL_DIR)/stm32f7xx_hal_i2c_ex.o \
                  $(HAL_DIR)/stm32f7xx_hal_ltdc.o \
                  $(HAL_DIR)/stm32f7xx_hal_pwr.o \
                  $(HAL_DIR)/stm32f7xx_hal_pwr_ex.o \
                  $(HAL_DIR)/stm32f7xx_hal_rcc.o \
                  $(HAL_DIR)/stm32f7xx_hal_rcc_ex.o \
                  $(HAL_DIR)/stm32f7xx_hal_sdram.o \
                  $(HAL_DIR)/stm32f7xx_hal_sram.o \
                  $(HAL_DIR)/stm32f7xx_hal_uart.o \
				  $(HAL_DIR)/stm32f7xx_hal_tim.o \
				  $(HAL_DIR)/stm32f7xx_hal_tim_ex.o \
                  $(HAL_DIR)/stm32f7xx_ll_fmc.o \
                  $(HAL_DIR)/stm32f7xx_ll_usb.o

OBJS_DRIVER = $(WORK_DIR)/Driver/Chip/AR8020/UART/Src/serial.o

OBJS_BOOTLOAD = ${WORK_DIR}/Bootload/AR8020/cpu2/start.o

OBJS =
OBJS += $(OBJS_BOOTLOAD)
OBJS += $(OBJS_DRIVER)
OBJS += $(OBJS_DRIVER_HAL)
OBJS += $(OBJS_LIB_ARTOSYN)
OBJS += $(OBJS_LIB_CMSIS)
OBJS += $(OBJS_APP)

INCLUDES += -I$(WORK_DIR)/Configure \
            -I$(WORK_DIR)/Configure/Include \
			-I$(WORK_DIR)/Driver/Chip/AR8020/UART/Inc \
            -I$(WORK_DIR)/Library/Artosyn/Common/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/CMSIS/Device/ST/STM32F7xx/Include \
			-I$(WORK_DIR)/Library/ThirdParty/USB_Library/STM32F7xx_HAL_Driver/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/CMSIS/Include \
            -I$(WORK_DIR)/Application/AR8020Verification

CFLAGS += -DSTM32F746xx -D_AR8020_VERIFICATION_
LDFLAGS += -Bstatic

$(TARGET).bin: $(OBJS) $(LIBS)
	$(CROSS_COMPILE)ld $(LDFLAGS) -T$(WORK_DIR)/Bootload/AR8020/cpu2/ar8020.lds -o $(TARGET).elf $^
	$(CROSS_COMPILE)objcopy -O binary $(TARGET).elf $@
	$(CROSS_COMPILE)objdump -D $(TARGET).elf > $(TARGET).dis
	$(CROSS_COMPILE)nm $(TARGET).elf > $(TARGET).map
	$(UTILITY_DIR)/bin2mif $@ $(TARGET).mif 32768 64
	cp $(TARGET).mif  $(OUTPUT_DIR)

%.o: %.c
	$(CROSS_COMPILE)gcc $(INCLUDES) $(CFLAGS) -mthumb -mcpu=cortex-m7 -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -std=c99 -c $< -o $@

#%.o: %.S
#	$(CROSS_COMPILE)gcc $(INCLUDES) -O2 -mthumb -mcpu=cortex-m4 -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -c $< -o $@

clean:
	rm -f $(OBJS) 
	rm -f  $(TARGET).elf  $(TARGET).bin  $(TARGET).dis  $(TARGET).mif  $(TARGET).map $(OUTPUT_DIR)/$(TARGET).mif

