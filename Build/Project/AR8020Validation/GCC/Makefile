CROSS_COMPILE = Tools/arm-2010q1/bin/arm-uclinuxeabi-

OBJS += start.o \
	main.o \
	test_func.o \
	Driver/UART/Src/serial.o \
	Driver/SD/Src/sd_core.o \
	Driver/SD/Src/sd_card.o \
	Driver/SD/Src/sd_host.o \
	Lib/Src/interrupt.o \
	Lib/Src/command.o \
	Lib/Src/memory.o \
	Kernel/croutine.o \
	Kernel/event_groups.o \
	Kernel/list.o \
	Kernel/queue.o \
	Kernel/tasks.o \
	Kernel/timers.o \
	Kernel/port.o \
	Kernel/heap_4.o \
	Kernel/cmsis_os.o \
	Kernel/System/system_stm32f7xx.o \
	Middlewares/FatFs/Src/diskio.o \
	Middlewares/FatFs/Src/ff.o \
	Middlewares/FatFs/Src/ff_gen_drv.o \
	Middlewares/FatFs/Port/sd_diskio.o


INCLUDES += -I./Driver/SD/Inc \
			-I./Driver/UART/Inc \
            -I./Lib/Inc \
	        -I./Include \
	        -I./Kernel/Inc \
	        -I./Kernel/System \
	        -I./Middlewares/FatFs/Inc \
	        -I./Middlewares/FatFs/Port

TARGET = ar8020

$(TARGET).bin: $(OBJS) $(LIBS)
	$(CROSS_COMPILE)ld -Bstatic -T$(TARGET).lds -o $(TARGET).elf $^
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
	rm *.o driver/src/*.o kernel/*.o lib/src/*.o *.elf *.bin *.dis *.mif *.map



