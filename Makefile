#
# Makefile for msp430
#
# 'make' builds everything
# 'make clean' deletes everything except source files and Makefile
# You need to set TARGET, MCU and SOURCES for your project.
# TARGET is the name of the executable file to be produced 
# $(TARGET).elf $(TARGET).hex and $(TARGET).txt nad $(TARGET).map are all generated.
# The TXT file is used for BSL loading, the ELF can be used for JTAG use
# 
TARGET      = fram
CPU         = msp430
MCU         = msp430f249
TOOLCHAIN   = $(shell pwd)/toolchain
# List all the source files here
# eg if you have a source file foo.c then list it here
SOURCES = fram.c uart.c spi.c
# Include are located in the Include directory
INCLUDES = 
# Add or subtract whatever MSPGCC flags you want. There are plenty more
#######################################################################################
CFLAGS   = -mcpu=$(CPU) -mmcu=$(MCU) -std=c99 -g -Os -Wall -Wunused -Itoolchain/msp430-gcc-support-files/include $(INCLUDES)
ASFLAGS  = -mcpu=$(CPU) -mmcu=$(MCU) -x assembler-with-cpp -Wa,-gstabs
LDFLAGS  = -mcpu=$(CPU) -mmcu=$(MCU) -Wl,-Map=$(TARGET).map -Ltoolchain/msp430-gcc-support-files/include
########################################################################################
CC           = $(TOOLCHAIN)/current/bin/msp430-elf-gcc
LD           = $(TOOLCHAIN)/current/bin/msp430-elf-ld
AR           = $(TOOLCHAIN)/current/bin/msp430-elf-ar
AS           = $(TOOLCHAIN)/current/bin/msp430-elf-gcc
GASP         = $(TOOLCHAIN)/current/bin/msp430-elf-gasp
NM           = $(TOOLCHAIN)/current/bin/msp430-elf-nm
OBJCOPY      = $(TOOLCHAIN)/current/bin/msp430-elf-objcopy
RANLIB       = $(TOOLCHAIN)/current/bin/msp430-elf-ranlib
STRIP        = $(TOOLCHAIN)/current/bin/msp430-elf-strip
SIZE         = $(TOOLCHAIN)/current/bin/msp430-elf-size
READELF      = $(TOOLCHAIN)/current/bin/msp430-elf-readelf
MAKETXT      = srec_cat
CP           = cp -p
RM           = rm -f
MV           = mv
########################################################################################
# the file which will include dependencies
DEPEND = $(SOURCES:.c=.d)
# all the object files
OBJECTS = $(SOURCES:.c=.o)
all: $(TARGET).elf $(TARGET).hex $(TARGET).txt 
$(TARGET).elf: $(OBJECTS)
	echo "Linking $@"
	$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@
	echo
	echo ">>>> Size of Firmware <<<<"
	$(SIZE) $(TARGET).elf
	echo
%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@
%.txt: %.hex
	$(MAKETXT) -O $@ -TITXT $< -I
%.o: %.c
	echo "Compiling $<"
	$(CC) -c $(CFLAGS) -o $@ $<
# rule for making assembler source listing, to see the code
%.lst: %.c
	$(CC) -c $(CFLAGS) -Wa,-anlhd $< > $@
# include the dependencies unless we're going to clean, then forget about them.
ifneq ($(MAKECMDGOALS), clean)
-include $(DEPEND)
endif
# dependencies file
# includes also considered, since some of these are our own
# (otherwise use -MM instead of -M)
%.d: %.c
	echo "Generating dependencies $@ from $<"
	$(CC) -M ${CFLAGS} $< >$@
.SILENT:
.PHONY:	clean
clean:
	-$(RM) $(OBJECTS)
	-$(RM) $(TARGET).map
	-$(RM) $(TARGET).elf $(TARGET).hex $(TARGET).txt
	-$(RM) $(TARGET).lst
	-$(RM) $(SOURCES:.c=.lst)
	-$(RM) $(DEPEND)

tc:
	-(test -d toolchain || mkdir toolchain)
	-wget -c 'http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSPGCC/latest/exports/msp430-gcc-7.3.2.154_macos.7z' -O toolchain/msp430-gcc-7.3.2.154_macos.7z
	-(cd toolchain && test -d msp430-gcc-7.3.2.154_macos || 7z x msp430-gcc-7.3.2.154_macos.7z)
	-(cd toolchain && test -L current || ln -s msp430-gcc-7.3.2.154_macos current)
	-wget -c 'http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSPGCC/6_0_1_0/exports/msp430-gcc-support-files-1.205.zip' -O toolchain/msp430-gcc-support-files-1.205.zip
	-(cd toolchain && test -d msp430-gcc-support-files || unzip msp430-gcc-support-files-1.205.zip)