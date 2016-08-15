CROSS_COMPILE = /opt/toolchain/arm-2010q1/bin/arm-uclinuxeabi-

PWD = $(shell pwd)
WORK_DIR := $(PWD)/../../../../

OBJS_APP = $(WORK_DIR)/Application/AR8020Verification/main.o

OBJS_LIB_USB_Device = $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/AUDIO/Src/usbd_audio.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/AUDIO/Src/usbd_audio_if_template.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc_if_template.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/CustomHID/Src/usbd_customhid.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/CustomHID/Src/usbd_customhid_if_template.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/DFU/Src/usbd_dfu.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/DFU/Src/usbd_dfu_media_template.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/HID/Src/usbd_hid.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_bot.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_data.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_scsi.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_storage_template.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/Template/Src/usbd_template.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Core/Src/usbd_conf_template.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Core/Src/usbd_core.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.o \
                      $(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Core/Src/usbd_ioreq.o


OBJS_LIB_USB_HOST = $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/AUDIO/Src/usbh_audio.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/CDC/Src/usbh_cdc.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/HID/Src/usbh_hid.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_keybd.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_mouse.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_parser.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/MSC/Src/usbh_msc_bot.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/MSC/Src/usbh_msc.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/MSC/Src/usbh_msc_scsi.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/MTP/Src/usbh_mtp.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/MTP/Src/usbh_mtp_ptp.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Core/Src/usbh_conf_template.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Core/Src/usbh_core.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Core/Src/usbh_ctlreq.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Core/Src/usbh_ioreq.o \
                    $(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Core/Src/usbh_pipes.o

OBJS_LIB_CMSIS = $(WORK_DIR)/Library/ThirdParty/CMSIS_RTOS/cmsis_os.o 

OBJS_LIB_ARTOSYN = $(WORK_DIR)/Library/Artosyn/Lib/Src/interrupt.o \
                   $(WORK_DIR)/Library/Artosyn/Lib/Src/command.o \
                   $(WORK_DIR)/Library/Artosyn/Lib/Src/memory.o

OBJS_FATFS = $(WORK_DIR)/Library/ThirdParty/FatFs/Src/diskio.o \
             $(WORK_DIR)/Library/ThirdParty/FatFs/Src/ff.o \
             $(WORK_DIR)/Library/ThirdParty/FatFs/Src/ff_gen_drv.o \
             $(WORK_DIR)/Library/ThirdParty/FatFs/Port/sd_diskio.o

OBJS_DRIVER_HAL = $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.o \
		  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma2d.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma_ex.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_flash_ex.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_gpio.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_hcd.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_i2c.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_i2c_ex.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_ltdc.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_pwr.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_pwr_ex.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_rcc.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_rcc_ex.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_sdram.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_sram.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_hal_uart.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_ll_fmc.o \
                  $(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Src/Src/stm32f7xx_ll_usb.o

OBJS_DRIVER = $(WORK_DIR)/Driver/Chip/AR8020/UART/Src/serial.o \
              $(WORK_DIR)/Driver/Chip/AR8020/SD/Src/sd_core.o \
              $(WORK_DIR)/Driver/Chip/AR8020/SD/Src/sd_card.o \
              $(WORK_DIR)/Driver/Chip/AR8020/SD/Src/sd_host.o

OBJS_KERNEL = $(WORK_DIR)/Kernel/FreeRTOS/croutine.o \
              $(WORK_DIR)/Kernel/FreeRTOS/event_groups.o \
              $(WORK_DIR)/Kernel/FreeRTOS/list.o \
              $(WORK_DIR)/Kernel/FreeRTOS/queue.o \
              $(WORK_DIR)/Kernel/FreeRTOS/tasks.o \
              $(WORK_DIR)/Kernel/FreeRTOS/timers.o \
              $(WORK_DIR)/Kernel/FreeRTOS/port.o \
              $(WORK_DIR)/Kernel/FreeRTOS/heap_4.o \
              $(WORK_DIR)/Kernel/FreeRTOS/System/system_stm32f7xx.o

OBJS_BOOTLOAD = ${WORK_DIR}/Bootload/AR8020Verification/start.o

OBJS =
OBJS += $(OBJS_BOOTLOAD)
OBJS += $(OBJS_KERNEL)
OBJS += $(OBJS_DRIVER)
OBJS += $(OBJS_DRIVER_HAL)
OBJS += $(OBJS_LIB_FATFS)
OBJS += $(OBJS_LIB_ARTOSYN)
OBJS += $(OBJS_LIB_CMSIS)
OBJS += $(OBJS_LIB_USB_HOST)
OBJS += $(OBJS_LIB_USB_Device)
OBJS += $(OBJS_APP)

INCLUDES += -I$(WORK_DIR)/Driver/Chip/AR8020/SD/Inc \
	    -I$(WORK_DIR)/Driver/Chip/AR8020/UART/Inc \
            -I$(WORK_DIR)/Library/Artosyn/Lib/Inc \
	    -I$(WORK_DIR)/Configuration/Include \
	    -I$(WORK_DIR)/Kernel/FreeRTOS/Inc \
            -I$(WORK_DIR)/Kernel/FreeRTOS/System \
            -I$(WORK_DIR)/Driver/HAL/STM32F7xx_HAL_Driver/Inc \
	    -I$(WORK_DIR)/Library/ThirdParty/FatFs/Inc \
	    -I$(WORK_DIR)/Library/ThirdParty/FatFs/Port \
            -I$(WORK_DIR)/Library/ThirdParty/CMSIS/Include \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/AUDIO/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/CDC/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/HID/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/MSC/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Class/MTP/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Host_Library/Core/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/AUDIO/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/CDC/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/CustomHID/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/DFU/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/HID/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/MSC/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Class/Template/Inc \
            -I$(WORK_DIR)/Library/ThirdParty/STM32_USB_Device_Library/Core/Inc

TARGET = ar8020

$(TARGET).bin: $(OBJS) $(LIBS)
	#$(CROSS_COMPILE)ld -Bstatic -T$(TARGET).lds -o $(TARGET).elf $^
	$(CROSS_COMPILE)ld -Bstatic -o $(TARGET).elf $^
	$(CROSS_COMPILE)objcopy -O binary $(TARGET).elf $@
	$(CROSS_COMPILE)objdump -D $(TARGET).elf > $(TARGET).dis
	$(CROSS_COMPILE)nm $(TARGET).elf > $(TARGET).map
	./Tools/bin2mif $@ $(TARGET).mif 32768 64
	cp $(TARGET).mif  /mnt/hgfs/mzhao

%.o: %.c
	$(CROSS_COMPILE)gcc $(INCLUDES) -mthumb -mcpu=cortex-m4 -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -std=c99 -c $< -o $@

#%.o: %.S
#	$(CROSS_COMPILE)gcc $(INCLUDES) -O2 -mthumb -mcpu=cortex-m4 -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -c $< -o $@

clean:
	rm -f $(OBJS)
	#rm *.o driver/src/*.o kernel/*.o lib/src/*.o *.elf *.bin *.dis *.mif *.map

