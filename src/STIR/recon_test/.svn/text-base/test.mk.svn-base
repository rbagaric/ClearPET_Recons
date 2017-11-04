#
# $Id: test.mk,v 1.3 2004/08/04 16:26:25 kris Exp $
# 
dir := recon_test

$(dir)_TEST_SOURCES := test_DataSymmetriesForBins_PET_CartesianGrid.cxx


# rules that do not link with all registries to save time during linking
# Beware: the pattern below is dangerous as it relies on a naming style for the registries.
# Note: have to be before the include below as that resets $(dir)

${DEST}$(dir)/test_DataSymmetriesForBins_PET_CartesianGrid: ${DEST}$(dir)/test_DataSymmetriesForBins_PET_CartesianGrid${O_SUFFIX} \
   $(STIR_LIB) $(filter %recon_buildblock_registries${O_SUFFIX},$(STIR_REGISTRIES))
	$(LINK) $(EXE_OUTFLAG)$@ $< \
		$(filter %recon_buildblock_registries${O_SUFFIX},$(STIR_REGISTRIES)) \
		 $(STIR_LIB)  $(LINKFLAGS) $(SYS_LIBS)

include $(WORKSPACE)/test.mk


