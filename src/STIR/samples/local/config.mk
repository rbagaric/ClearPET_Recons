# $Id: config.mk,v 1.3 2004/08/04 16:18:41 kris Exp $
# Sample file for local/config.mk (by Kris Thielemans)
# Use this file to modify defaults for some variables in the 'global' config.mk
# Modify according to your own needs

# set non-default location of Louvain la Neuve files 
LLN_INCLUDE_DIR=$(HOME)/lln/ecat

# set non-default location where files will be installed
INSTALL_PREFIX=/usr/local

# This file only sets EXTRA_OPT
# Changes w.r.t. defaults:
# use all warnings,
# use debug flag -g even in optimised mode
# so, set EXTRA_OPT=-g -Wall
# for some systems, do more...
ifeq ($(SYSTEM),LINUX)
# our Linux system has Pentium III processors
EXTRA_CFLAGS=-g -Wall -march=pentium3
else
ifeq ($(SYSTEM),CYGWIN)
# our NT system has Pentium II processors
# maybe cygwin can't find X include files by default
EXTRA_CFLAGS=-g -Wall -march=pentium2 -I /usr/X11R6/include
else
ifeq ($(SYSTEM),CC)
# our Parsytec CC system has gcc 2.8.1, so we don't use -Wall 
# and have to set the template depth
EXTRA_CFLAGS= -ftemplate-depth-25
else
# default
EXTRA_CFLAGS=-g -Wall
endif
endif
endif
