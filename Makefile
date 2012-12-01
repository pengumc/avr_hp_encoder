# Name: Makefile
# Author: Michiel van der Coelen
# email: Michiel.van.der.coelen@gmail.com
# date: 2012-12-01

MMCU = atmega88
AVRDUDE_MCU = m88
AVRDUDE_PROGRAMMERID = usbasp
AVRDUDE_PORT = USB
F_CPU = 12000000
NAME = encoder


OBJECTS = $(NAME).o 
CFLAGS = -DF_CPU=$(F_CPU) -std=c99 -Wall -Os -mmcu=$(MMCU) -I.
CC = avr-gcc
SIZE = avr-size
OBJCOPY = avr-objcopy

.PHONY: all test
all: $(NAME).hex
	$(SIZE) $(NAME).hex

$(NAME).hex: $(NAME).elf
	$(OBJCOPY) -O ihex $(NAME).elf $(NAME).hex
	
$(NAME).elf: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(NAME).elf $(OBJECTS)

#compile src files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@


clean:
	rm -f $(OBJECTS) $(NAME).elf

program: $(NAME).hex
	avrdude -c $(AVRDUDE_PROGRAMMERID) -p $(AVRDUDE_MCU) -k $(AVRDUDE_PORT) -U flash:w:$(NAME).hex

test:
	avrdude -c $(AVRDUDE_PROGRAMMERID) -p $(AVRDUDE_MCU) -v