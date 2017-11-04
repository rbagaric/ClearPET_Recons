//
// $Id: buildblock_registries.cxx,v 1.6 2002/05/07 15:01:56 kris Exp $
//
/*!

  \file
  \ingroup buildblock

  \brief File that registers all RegisterObject children in buildblock

  \author Kris Thielemans
  
  $Date: 2002/05/07 15:01:56 $
  $Revision: 1.6 $
*/
/*
    Copyright (C) 2000- $Date: 2002/05/07 15:01:56 $, IRSL
    See STIR/LICENSE.txt for details
*/

#include "stir/SeparableCartesianMetzImageFilter.h"
#include "stir/MedianImageFilter3D.h"
#include "stir/ChainedImageProcessor.h"
#include "stir/ThresholdMinToSmallPositiveValueImageProcessor.h"
#include "stir/SeparableConvolutionImageFilter.h"

START_NAMESPACE_STIR

static MedianImageFilter3D<float>::RegisterIt dummy;
static SeparableCartesianMetzImageFilter<float>::RegisterIt dummy2;
static ChainedImageProcessor<3,float>::RegisterIt dummy3;
static ThresholdMinToSmallPositiveValueImageProcessor<float>::RegisterIt dummy4;
static SeparableConvolutionImageFilter<float>::RegisterIt dummy5;
END_NAMESPACE_STIR
