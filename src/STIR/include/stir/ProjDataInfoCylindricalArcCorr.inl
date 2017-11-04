//
// $Id: ProjDataInfoCylindricalArcCorr.inl,v 1.5 2003/05/27 10:51:58 kris Exp $
//
/*!

  \file
  \ingroup projdata

  \brief Implementation of inline functions of class 
  ProjDataInfoCylindricalArcCorr

  \author Sanida Mustafovic
  \author Kris Thielemans
  \author PARAPET project

  $Date: 2003/05/27 10:51:58 $

  $Revision: 1.5 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2003/05/27 10:51:58 $, IRSL
    See STIR/LICENSE.txt for details
*/

#include "stir/Bin.h"

START_NAMESPACE_STIR

float
ProjDataInfoCylindricalArcCorr::get_s(const Bin& bin) const
{return bin.tangential_pos_num()*bin_size;}


float
ProjDataInfoCylindricalArcCorr::get_tangential_sampling() const
{return bin_size;}

END_NAMESPACE_STIR

