PORT      := /dev/tty.usbserial-*

DEVICE = atmega128
F_CPU = 16000000UL
HFUSE = 0x09
LFUSE = 0xFF

AVRDUDE = avrdude -c dragon_jtag -P usb -p $(DEVICE)
OBJECTS = main.o cloudgen.o

COMPILE = avr-gcc -g -mmcu=$(DEVICE) -Wall -Wextra -Werror -Os -std=gnu99 -funsigned-bitfields -fshort-enums \
                  -DF_CPU=$(F_CPU)

all: main.hex

fuse:
	$(AVRDUDE) -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m efuse:w:0xFF:m

install: main.hex
	$(AVRDUDE) -U flash:w:main.hex:i

clean:
	rm -f reset main.hex main.elf $(OBJECTS)

disasm:	main.elf
	avr-objdump -d main.elf

size: main.elf
	avr-size -C --mcu=$(DEVICE) main.elf

debug: main.elf
	avarice -g --part $(DEVICE) --dragon --jtag usb --file main.elf :4242

.c.o:
	$(COMPILE) -c $< -o $@

main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex