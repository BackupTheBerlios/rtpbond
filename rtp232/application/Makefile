###############################################################################
# Makefile for the project aeb
###############################################################################

## General Flags
PROJECT = aeb
MCU = atmega128
TARGET = aeb.elf
CC = avr-gcc.exe

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -DREQUIRES_SERIAL -DREQUIRES_RTP -DAPPLICATION_SERIALBRIDGE -DREQUIRES_QUEUE -DREQUIRES_DOUBLYLINKEDLIST -DIP4_SUBNET=0x00FFFFFF -gdwarf-2 -DzIP4_ADDRESS=0xAA00A8C0 -DzIP4_GATEWAY=0x0100A8C0 -DIP4_GATEWAY=0x01b04581 -DIP4_REMOTE_HOST_DATA=0x7db04581 -DIP4_REMOTE_HOST_CONFIG=0x7db04581 -DIP4_REMOTE_PORT_CONFIG=7778 -DMAC_GROUP1=0x0200 -DMAC_GROUP2=0x0000 -DMAC_GROUP3=0x0001 -DIP4_ADDRESS=0x7fb04581   -DREQUIRES_RCFG   -DF_CPU=14745600UL -O3 -fsigned-char
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=aeb.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = aeb.o arp.o rtp.o ethernet.o ip.o udp.o tftp.o serial.o clock.o timer.o doublyLinkedList.o queue.o rtc.o com0.o enc28j60.o nic.o bootloader.o serialbridge.o watchdog.o netlib.o commands.o parser.o rcfg.o config.o com1_debug.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) aeb.hex aeb.eep aeb.lss size

## Compile
aeb.o: ../src/aeb.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

arp.o: ../src/modules/arp/arp.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

rtp.o: ../src/modules/rtp/rtp.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ethernet.o: ../src/modules/ethernet/ethernet.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ip.o: ../src/modules/ip/ip.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

udp.o: ../src/modules/udp/udp.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

tftp.o: ../src/modules/tftp/tftp.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

serial.o: ../src/modules/serial/serial.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

clock.o: ../src/modules/clock/clock.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

timer.o: ../src/modules/timer/timer.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

doublyLinkedList.o: ../src/toolkit/doublyLinkedList/doublyLinkedList.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

queue.o: ../src/toolkit/queue/queue.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

rtc.o: ../src/hal/rtc/rtc.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

com0.o: ../src/hal/com/com0.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

enc28j60.o: ../src/hal/nic/enc28j60.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

nic.o: ../src/hal/nic/nic.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

bootloader.o: ../src/applications/bootloader/bootloader.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

serialbridge.o: ../src/applications/serial/serialbridge.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

watchdog.o: ../src/modules/watchdog/watchdog.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

netlib.o: ../src/toolkit/netlib/netlib.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

commands.o: ../src/modules/rcfg/commands.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

parser.o: ../src/modules/rcfg/parser.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

rcfg.o: ../src/modules/rcfg/rcfg.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

config.o: ../src/config.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

com1_debug.o: ../src/hal/com/com1_debug.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) aeb.elf dep/* aeb.hex aeb.eep aeb.lss aeb.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

