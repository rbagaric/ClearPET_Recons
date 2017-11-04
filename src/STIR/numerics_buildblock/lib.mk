#
# $Id: lib.mk,v 1.2 2004/07/09 15:34:56 kris Exp $
#
dir:=numerics_buildblock

$(dir)_LIB_SOURCES := \
  fourier.cxx 

#$(dir)_REGISTRY_SOURCES:= $(dir)_registries.cxx

include $(WORKSPACE)/lib.mk


