# $Id$
#
#  Copyright (c) 2012 Sebastian Southen <southen@gmail.com>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

#PORT = /dev/ttyUSB0
#PORT = /dev/ttyACM0
PORT = /dev/tty.usbmodemfd121
#MCU = atmega1281
MCU = atmega2560
#F_CPU = 8000000
F_CPU = 16000000L
FORMAT = ihex
#UPLOAD_RATE = 19200
UPLOAD_RATE = 115200

TARGET = OpenDoorControl
#ARDUINO = /usr/share/arduino/hardware/arduino/variants/mega
#PATH=${PATH}:/Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/bin
ARDUINO = /usr/share/arduino/hardware/arduino/cores/arduino
LIBS = WString.cpp Print.cpp LiquidCrystal.cpp File.cpp HardwareSerial.cpp SD.cpp SdFile.cpp SdVolume.cpp Sd2Card.cpp Wire.cpp new.cpp main.cpp
#vpath %.cpp $(subst ${ },:,$(wildcard $(ARDUINO)/*/))
vpath %.c /usr/share/arduino/hardware/arduino/cores/arduino
vpath %.c $(subst ${ },:,$(wildcard /usr/share/arduino/libraries/*/))
vpath %.c $(subst ${ },:,$(wildcard /usr/share/arduino/libraries/*/utility))
vpath %.cpp /usr/share/arduino/hardware/arduino/cores/arduino
vpath %.cpp $(subst ${ },:,$(wildcard /usr/share/arduino/libraries/*/))
vpath %.cpp $(subst ${ },:,$(wildcard /usr/share/arduino/libraries/*/utility))
SRC = wiring.c wiring_digital.c wiring_analog.c twi.c
CXXSRC = $(TARGET).cpp $(LIBS) aux.cpp control.cpp general.cpp interrupts.cpp RFID.cpp SDfiles.cpp RTClib/RTClib.cpp
#CXXSRC = $(wildcard *.cpp) $(LIBS)


# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
DEBUG = stabs

OPT = s

# Place -D or -U options here
CDEFS = -DF_CPU=$(F_CPU) -DARDUINO=100
CXXDEFS = -DF_CPU=$(F_CPU) -DARDUINO=100


# Place -I options here
#  /usr/share/arduino/hardware/arduino/variants/...
#-I/usr/share/arduino/hardware/arduino/cores/arduino
CINCS = -I$(ARDUINO) \
 -I/usr/share/arduino/hardware/arduino/variants/mega \
 -I/usr/share/arduino/libraries/Wire/utility
CXXINCS = -I$(ARDUINO) \
 -I/usr/share/arduino/libraries/LiquidCrystal \
 -I/usr/share/arduino/libraries/Wire \
 -I/usr/share/arduino/libraries/SPI \
 -I/usr/share/arduino/libraries/SD \
 -I/usr/share/arduino/libraries/SD/utility \
 -I/usr/share/arduino/libraries/Sha \
 -I/usr/share/arduino/libraries/RTClib \
 -I/usr/share/arduino/libraries/Ethernet

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99
CDEBUG = -g$(DEBUG)
CWARN = -Wall -Wstrict-prototypes
CTUNING = -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
#CEXTRA = -Wa,-adhlns=$(<:.c=.lst)

CFLAGS = $(CDEBUG) $(CDEFS) $(CINCS) -O$(OPT) $(CWARN) $(CSTANDARD) $(CEXTRA)
CXXFLAGS = $(CDEFS) $(CINCS) $(CXXINCS) -O$(OPT)
#ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs 
LDFLAGS = 


# Programming support using avrdude. Settings and variables.
#> man avrdude
#AVRDUDE_PROGRAMMER = arduino
AVRDUDE_PROGRAMMER = stk500v2
AVRDUDE_PORT = $(PORT)
#AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
AVRDUDE_WRITE_FLASH = -D -U flash:w:$(TARGET).hex:i
AVRDUDE_FLAGS = -F -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b $(UPLOAD_RATE)

# Program settings
CC = avr-gcc
CXX = avr-g++
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
REMOVE = rm -f
MV = mv -f

# Define all object files.
OBJ = $(SRC:.c=.o) $(CXXSRC:.cpp=.o) $(ASRC:.S=.o)

# Define all listing files.
LST = $(ASRC:.S=.lst) $(CXXSRC:.cpp=.lst) $(SRC:.c=.lst)

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS)
ALL_CXXFLAGS = -mmcu=$(MCU) -I. $(CXXFLAGS)
ALL_ASFLAGS = -mmcu=$(MCU) -I. -x assembler-with-cpp $(ASFLAGS)


# Default target.
all: build

build: elf hex eep

elf: $(TARGET).elf
hex: $(TARGET).hex
eep: $(TARGET).eep
lss: $(TARGET).lss 
sym: $(TARGET).sym

# Program the device.  
upload: $(TARGET).hex $(TARGET).eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)




# Convert ELF to COFF for use in debugging / simulating in AVR Studio or VMLAB.
COFFCONVERT=$(OBJCOPY) --debugging \
--change-section-address .data-0x800000 \
--change-section-address .bss-0x800000 \
--change-section-address .noinit-0x800000 \
--change-section-address .eeprom-0x810000 


coff: $(TARGET).elf
	$(COFFCONVERT) -O coff-avr $(TARGET).elf $(TARGET).cof


extcoff: $(TARGET).elf
	$(COFFCONVERT) -O coff-ext-avr $(TARGET).elf $(TARGET).cof


.SUFFIXES: .elf .hex .eep .lss .sym

.elf.hex:
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

.elf.eep:
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
.elf.lss:
	$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
.elf.sym:
	$(NM) -n $< > $@



# Link: create ELF output file from object files.
$(TARGET).elf: $(OBJ)
	$(CC) $(ALL_CFLAGS) $(OBJ) --output $@ $(LDFLAGS)


# Compile: create object files from C++ source files.
.cpp.o:
	$(CXX) -c $(ALL_CXXFLAGS) $< -o $@
#	$(CXX) -c $(ALL_CXXFLAGS) $< -o $(@F)

# Compile: create object files from C source files.
.c.o:
	$(CC) -c $(ALL_CFLAGS) $< -o $@ 


# Compile: create assembler files from C source files.
#.c.s:
#	$(CC) -S $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files.
#.S.o:
#	$(CC) -c $(ALL_ASFLAGS) $< -o $@


# Target: clean project.
## keep $(TARGET).hex
clean:
	$(REMOVE) $(TARGET).eep $(TARGET).cof $(TARGET).elf \
	$(TARGET).map $(TARGET).sym $(TARGET).lss \
	$(OBJ) $(LST) $(SRC:.c=.s) $(SRC:.c=.d) $(CXXSRC:.cpp=.s) $(CXXSRC:.cpp=.d)

.PHONY:	all build elf hex eep lss sym program coff extcoff clean #depend

