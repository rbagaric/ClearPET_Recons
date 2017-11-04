# makefile for ClearPET coincidence sorter
# Author: Axel Wagner, raytest

BINARY = sorter2

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

# Enable this if the randoms output via delayed coincidence check should
# be compiled in. Note that this can be turned off during runtime, too.
ifndef RANDOMS
RANDOMS = 1
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
NONCOPY_H = noncopy.h
INTTYPES_H = inttypes.h
LOOKUPPIXEL_H = lookupPixel.h $(INTTYPES_H)
OPTIONS_H = options.h
VERSION_H = version.h
FILEREADER_H = filereader.h $(INTTYPES_H)
STATS_H = stats.h $(INTTYPES_H)
MINILMF_H = miniLMF.h $(INTTYPES_H)
ANGLES_H = angles.h $(INTTYPES_H) $(NONCOPY_H)
EVTTYPES_H = evttypes.h $(INTTYPES_H) $(LOOKUPPIXEL_H)
EVTPIPE_H = evtpipe.h $(NONCOPY_H) $(EVTTYPES_H) $(FILEREADER_H)
SORTER_H = sorter.h $(EVTPIPE_H) $(FILEREADER_H)

angles.cpp: $(ANGLES_H)

evtpipe.cpp: $(EVTPIPE_H)

filereader.cpp: $(FILEREADER_H)

lookupPixel.cpp: $(LOOKUPPIXEL_H) $(INTTYPES_H)

main.cpp:   $(SORTER_H) $(VERSION_H) $(OPTIONS_H)

miniLMF.cpp: $(MINILMF_H)

options.cpp: $(OPTIONS_H)

sorter.cpp: $(SORTER_H) $(ANGLES_H) $(EVTTYPES_H) $(MINILMF_H)

stats.cpp:  $(STATS_H)


############################
# Make targets
#
OBJECTS = angles.o evtpipe.o filereader.o lookupPixel.o main.o miniLMF.o \
    options.o sorter.o stats.o

$(BINARY): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.PHONY: clean install
clean:
	rm $(OBJECTS) $(BINARY)
install:
	cp $(BINARY) ~/bin
