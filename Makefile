
CFLAGS  ?=  -W -Wall -Wextra -Werror -Wundef -Wshadow -Wdouble-promotion \
            -Wformat-truncation -fno-common -Wconversion \
            -g3 -Os -ffunction-sections -fdata-sections \
	    -I. -Iinclude -I$(PWD)/cmsis_core/CMSIS/Core/Include -I$(PWD)/cmsis_f0/Include \
            -mcpu=cortex-m0 -mthumb -mfloat-abi=soft $(EXTRA_CFLAGS) 
LDFLAGS ?= -Tlink.ld -nostartfiles -nostdlib --specs nano.specs -lc -lgcc -Wl,--gc-sections -Wl,-Map=$@.map

EXTRA_CFLAGS ?=

SOURCES = main.c syscalls.c hal.c uart.c
SOURCES += cmsis_f0/Source/Templates/gcc/startup_stm32f072xb.s

build: firmware.bin
firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary $< $@

firmware.elf: cmsis_core cmsis_f0 hal.h link.ld Makefile $(SOURCES)
	arm-none-eabi-gcc $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $@

stflash: firmware.bin
	st-flash --reset write $< 0x08000000


# Flash with J-Link (ST-LINK USB)
jflash: jflash.script
	JLinkExe -commanderscript $<

# Device Configuration (Change this if using a different STM32 variant)
DEVICE = STM32F072RB

jflash.script: firmware.bin
	@echo "device $(DEVICE)" > $@
	@echo "speed 4000" >> $@
	@echo "si SWD" >> $@       # Force SWD interface to avoid manual selection
	@echo "loadbin $< 0x08000000" >> $@
	@echo "r" >> $@            # Reset
	@echo "g" >> $@            # Start execution
	@echo "qc" >> $@           # Quit J-Link


cmsis_core:
	git clone --depth 1 -b 5.4.0 https://github.com/ARM-software/CMSIS_5 $@


cmsis_f0:
	git clone --depth 1 -b v2.3.4 https://github.com/STMicroelectronics/cmsis-device-f0.git cmsis_f0

clean:
	rm -rf firmware.* jflash.script

