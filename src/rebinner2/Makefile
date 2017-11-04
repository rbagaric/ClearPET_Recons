# makefile for ClearPET rebinner version 2
# Author: Axel Wagner, raytest

BINARY = rebinner2

# main target:
.PHONY: all
all: $(BINARY)

############################
# Compilation Options
#
# Note; when changing one of these options, be sure to run a "make clean"
# in order to force recompilation of all the code.
# Each option may also be specified on the make command line. I such a case
# it overrides the default settings in this file.

# Enable this if boost libraries are available on the target system
ifndef BOOST
BOOST = 0
endif

FLAGS = -Wall -O3 -fexceptions -DHAS_STDINT=1 `getconf LFS_CFLAGS`
ifeq ($(BOOST),1)
CFLAGS = $(FLAGS) -DHAS_BOOST=1
endif

CC = g++
LDFLAGS = -L/usr/lib/ -lm

%.o  : %.cpp
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

############################
# Dependencies
#
CLEARPET_H = clearpet.h
INTTYPES_H = inttypes.h
LOR_H = lor.h
NONCOPY_H = noncopy.h
OPTIONS_H = options.h
VERSION_H = version.h
COINCEVENT_H = coincevent.h  $(INTTYPES_H)
GEOMETRY_H = geometry.h $(CLEARPET_H) $(COINCEVENT_H) $(LOR_H)
SINOGRAM_H = sinogram.h $(COINCEVENT_H) $(LOR_H) $(NONCOPY_H)
MINILMFREADER_H = miniLMFReader.h $(INTTYPES_H)
GATEREADER_H = gatereader.h $(INTTYPES_H)
ROOTREADER_H = rootreader.h $(INTTYPES_H)
REBINNER_H = rebinner.h $(NONCOPY_H)

geometry.cpp: $(GEOMETRY_H)

main.cpp:   $(REBINNER_H) $(VERSION_H) $(OPTIONS_H)

crystals_main.cpp:   $(GEOMETRY_H)

miniLMFReader.cpp: $(MINILMFREADER_H) $(CLEARPET_H) $(COINCEVENT_H)

gatereader.cpp: $(GATEREADER_H) $(CLEARPET_H) $(COINCEVENT_H)

rootreader.cpp: $(ROOTREADER_H) $(CLEARPET_H) $(COINCEVENT_H)

options.cpp: $(OPTIONS_H)

rebinner.cpp: $(REBINNER_H) $(SINOGRAM_H) $(MINILMFREADER_H) $(GEOMETRY)

rebinner_gate.o: rebinner.cpp $(REBINNER_H) $(SINOGRAM_H) $(GATEREADER_H) $(GEOMETRY)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -DUSE_GATE_INPUT=1 -o rebinner_gate.o rebinner.cpp

rebinner_root.o: rebinner.cpp $(REBINNER_H) $(SINOGRAM_H) $(ROOTREADER_H) $(GEOMETRY)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -DUSE_ROOT_INPUT=1 -o rebinner_root.o rebinner.cpp

sinogram.cpp:  $(SINOGRAM_H) $(GEOMETRY)


############################
# Make targets
#
OBJECTS = geometry.o main.o miniLMFReader.o options.o rebinner.o sinogram.o

$(BINARY): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

rebinner2_gate: geometry.o main.o gatereader.o options.o rebinner_gate.o sinogram.o
	$(CC) geometry.o main.o gatereader.o options.o rebinner_gate.o sinogram.o -o $@

rebinner2_root: geometry.o main.o rootreader.o options.o rebinner_root.o sinogram.o
	$(CC) geometry.o main.o rootreader.o options.o rebinner_root.o sinogram.o -o $@

crystals: geometry.o crystals_main.o
	$(CC) geometry.o crystals_main.o -o $@

.PHONY: clean install
clean:
	rm *.o $(BINARY) crystals rebinner2_gate rebinner2_root
install: $(BINARY)
	cp $(BINARY) ~/bin
