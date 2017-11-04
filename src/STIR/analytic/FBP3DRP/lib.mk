#
# $Id: lib.mk,v 1.2 2004/10/25 00:03:54 kris Exp $
#

dir := analytic/FBP3DRP
$(dir)_LIB_SOURCES:= \
	ColsherFilter.cxx  FBP3DRPReconstruction.cxx

#$(dir)_REGISTRY_SOURCES:= registries.cxx

include $(WORKSPACE)/lib.mk

