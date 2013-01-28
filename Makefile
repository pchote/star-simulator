PORT      := /dev/tty.usbserial-*

DEVICE = atmega128
F_CPU = 16000000UL
HFUSE = 0x08
LFUSE = 0xFF
BOOTSTART    = 0x1E000

AVRDUDE = avrdude -c dragon_jtag -P usb -p $(DEVICE)
OBJECTS = main.o

BOOTLOADER   = avrdude -c avr109 -p $(DEVICE) -b 9600 -P $(PORT)
BOOT_OBJECTS = bootloader.o


COMPILE = avr-gcc -g -mmcu=$(DEVICE) -Wall -Wextra -Werror -Os -std=gnu99 -funsigned-bitfields -fshort-enums \
                  -DF_CPU=$(F_CPU) -DBOOTSTART=$(BOOTSTART)

all: main.hex bootloader.hex

reset:
	$(CC) -o $@ reset.c

fuse:
	$(AVRDUDE) -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m efuse:w:0xFF:m

install: main.hex reset
	./reset $(PORT)
	$(BOOTLOADER) -U flash:w:main.hex:i
	./reset $(PORT)

jtag: combined.hex
	$(AVRDUDE) -U flash:w:combined.hex:i

clean:
	rm -f reset main.hex main.elf bootloader.hex bootloader.elf combined.hex $(OBJECTS) $(BOOT_OBJECTS)

disasm:	main.elf
	avr-objdump -d main.elf

size: main.elf
	avr-size -C --mcu=$(DEVICE) main.elf

debug: main.elf
	avarice -g --part $(DEVICE) --dragon --jtag usb --file main.elf :4242

debug-bootloader: bootloader.elf
	avarice -g --part $(DEVICE) --dragon --jtag usb --file bootloader.elf :4242

.c.o:
	$(COMPILE) -c $< -o $@

main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex

bootloader.elf: bootloader.o
	$(COMPILE) -Wl,--section-start=.text=$(BOOTSTART) -o bootloader.elf $(BOOT_OBJECTS)

bootloader.hex: bootloader.elf
	rm -f bootloader.hex
	avr-objcopy -j .text -j .data -O ihex bootloader.elf bootloader.hex

combined.hex: bootloader.hex main.hex
	srec_cat bootloader.hex -I main.hex -I -o combined.hex -I
