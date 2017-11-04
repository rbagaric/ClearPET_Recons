# makefile for ClearPET cylindrical mask program
# Author: Axel Wagner, raytest

BINARY = mask_cyl

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
ifeq ($(RANDOMS),1)
CFLAGS = $(FLAGS) -DDELAYED_COINC_CHECK=1
endif

CC = g++
LDFLAGS = -L/usr/lib/ -lm

%.o  : %.cpp
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

############################
# Dependencies
#
OPTIONS_H = options.h
VERSION_H = version.h

main.cpp:   $(VERSION_H) $(OPTIONS_H)

options.cpp: $(OPTIONS_H)


############################
# Make targets
#
OBJECTS = main.o options.o

$(BINARY): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.PHONY: clean install
clean:
	rm $(OBJECTS) $(BINARY)
install:
	cp $(BINARY) ~/bin
