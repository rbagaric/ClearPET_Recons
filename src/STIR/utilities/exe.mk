#
# $Id: exe.mk,v 1.5 2004/08/04 16:17:31 kris Exp $
#

dir:=utilities

$(dir)_SOURCES:=manip_image.cxx \
	manip_projdata.cxx \
	display_projdata.cxx \
	convecat6_if.cxx \
	do_linear_regression.cxx \
	postfilter.cxx \
	compare_projdata.cxx \
	compare_image.cxx \
        conv_to_ecat6.cxx \
	extract_segments.cxx \
	correct_projdata.cxx \
	stir_math.cxx \
	list_projdata_info.cxx \
	create_projdata_template.cxx \
	SSRB.cxx \
	ecat_swap_corners.cxx \
	poisson_noise.cxx \
	get_time_frame_info.cxx \
	generate_image.cxx \
	list_ROI_values.cxx \
	zoom_image.cxx \
  	rebin_projdata.cxx

ifeq ($(HAVE_LLN_MATRIX),1)
  # yes, the LLN files seem to be there, so we can compile 
  # ifheaders_for_ecat7 etc as well
  $(dir)_SOURCES += ifheaders_for_ecat7.cxx conv_to_ecat7.cxx
endif

${DEST}$(dir)/poisson_noise: ${DEST}$(dir)/poisson_noise$(O_SUFFIX) \
   $(STIR_LIB) 
	$(LINK) $(EXE_OUTFLAG)$@ $< \
		 $(STIR_LIB)  $(LINKFLAGS) $(SYS_LIBS)



include $(WORKSPACE)/exe.mk
