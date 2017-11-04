#
# $Id: exe.mk,v 1.1 2004/11/12 17:57:37 kris Exp $
#

dir:=examples

$(dir)_SOURCES = \
	demo1.cxx demo2.cxx demo3.cxx

include $(WORKSPACE)/exe.mk
