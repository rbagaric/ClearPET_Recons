#
# $Id: exe.mk,v 1.2 2004/08/13 12:54:51 kris Exp $
#

dir:=analytic/FBP2D

$(dir)_SOURCES = \
	FBP2D.cxx

include $(WORKSPACE)/exe.mk
