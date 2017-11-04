//
// $Id: BackProjectorByBinUsingInterpolation_linear.cxx,v 1.3 2004/03/23 11:59:31 kris Exp $
//
/*!

  \file
  \ingroup projection

  \brief This file defines two private static functions from
  BackProjectorByBinUsingInterpolation, for the case of piecewise 
  linear interpolation.

  \warning This #includes BackProjectorByBinUsingInterpolation_3DCho.cxx 

  This very ugly way of including a .cxx file is used to avoid replication of
  a lot of (difficult) code.

  \author Kris Thielemans
  \author PARAPET project

  $Date: 2004/03/23 11:59:31 $

  $Revision: 1.3 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2004/03/23 11:59:31 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/

#define PIECEWISE_INTERPOLATION 0
#include "BackProjectorByBinUsingInterpolation_3DCho.cxx"
#undef PIECEWISE_INTERPOLATION
