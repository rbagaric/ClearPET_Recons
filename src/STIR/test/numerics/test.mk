#
# $Id: test.mk,v 1.1 2005/06/03 17:45:53 kris Exp $
# 
dir := test/numerics

$(dir)_TEST_SOURCES := \
	test_matrices.cxx

##################################################
# rules to ignore registries
# note: have to be before include statement as that changes value of $(dir)
${DEST}$(dir)/test_matrices: ${DEST}$(dir)/test_matrices${O_SUFFIX} $(STIR_LIB) 
	$(LINK) $(EXE_OUTFLAG)$(@)$(EXE_SUFFIX) $< $(STIR_LIB)  $(LINKFLAGS) $(SYS_LIBS)


include $(WORKSPACE)/test.mk

