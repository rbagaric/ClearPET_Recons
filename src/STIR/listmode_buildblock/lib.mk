#
# $Id: lib.mk,v 1.1 2004/03/25 11:26:51 kris Exp $
#
dir := listmode_buildblock

$(dir)_LIB_SOURCES = \
	CListEvent.cxx \
	CListModeData.cxx \
	CListModeDataFromStream.cxx \
	CListModeDataECAT.cxx \
	CListRecordECAT962.cxx \
	CListRecordECAT966.cxx \
	LmToProjData.cxx \
	FullNorm.cxx \
	LMF_ClearPET.cxx \
	LMF_Interfile.cxx \
        ModifSino.cxx \
	SmoothSino.cxx \
        RatioAttenSino.cxx \
        SubtractSino.cxx \
	AddSino.cxx \
        DivideSino.cxx \
	MultiplySino.cxx \
	AverageAngleNorme.cxx \
	FillGap.cxx \
	ScattersByGaussianFit.cxx \
        DeadTimeCorr.cxx \
        SubtractSinoNegA0.cxx \
        SubtractSinoScatterNegA0.cxx \
	AttenNorm.cxx \
	AverageSino.cxx \
	RedimSino.cxx \
	GaussienneNorme.cxx \
	SymTang.cxx \
	MaskProp.cxx \
	ScattersByGaussianFitLeipzig.cxx


#$(dir)_REGISTRY_SOURCES:= $(dir)_registries.cxx

include $(WORKSPACE)/lib.mk
