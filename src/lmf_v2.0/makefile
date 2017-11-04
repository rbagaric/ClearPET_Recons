INCLUDE = -I./includes/

#ROOTCFLAGS   := $(shell root-config --cflags)
#ROOTCFLAGS +=  -O -Wall -fPIC
ROOTCFLAGS =  -O -Wall -fPIC
LIB = ./lib/libLMF.a 
CFLAGS = $(INCLUDE) -g -Wall -D_FILE_OFFSET_BITS=64

#-pg
# -efence
LDFLAGS = $(INCLUDE) -g 
#-pg


CSRC = $(wildcard src/*.c)
#CCSRC = $(wildcard src/*.cc)
SRC = $(CSRC)
#SRC += $ $(CCSRC)
OBJ = $(patsubst src/%.c,obj/%.o,$(CSRC))
#OBJ += $(patsubst src/%.cc,obj/%.o,$(CCSRC))

all : $(LIB) TAGS END_OF_COMPILE

obj/%.o : src/%.c
	gcc $(CFLAGS) -c -o $@ $<

obj/%.o : src/%.cc
	gcc $(CFLAGS) $(ROOTCFLAGS) -c -o $@ $<



$(LIB) : $(OBJ)
	ar rc $@ $^ 

END_OF_COMPILE :
	@echo ""
	@echo ""
	@echo ""
	@echo "Congratulations : compilation and linkage of lib/libLMF.a over !!!"
	@echo "Please try make in ./exemples to compile our examples"
	@echo ""
	@echo ""


clean :		 
	-rm *~
	-rm core
	-rm *#
	-rm obj/*.o
	-rm $(LIB)
	-clear
	-ls

PATHLMFCOMMON = /opt/lmf
install : 
	-chmod -R 700 $(PATHLMFCOMMON)
	-rm -rf $(PATHLMFCOMMON)/$(LIB)
	-rm -rf $(PATHLMFCOMMON)/includes/*	
	-cp $(LIB) $(PATHLMFCOMMON)/$(LIB)
	-cp includes/* $(PATHLMFCOMMON)/includes/
	-chmod -R 555 $(PATHLMFCOMMON)

TAGS :
	etags src/*.c includes/*.h
