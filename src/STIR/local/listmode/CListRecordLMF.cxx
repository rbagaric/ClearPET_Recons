//
// $Id: CListRecordLMF.cxx,v 1.0 2005/02/01 15:32:28 cmorel Exp $
//
/*!
  \file
  \ingroup listmode
  \Implementation of classes CListEventDataLMF and CListRecordLMF 
  for listmode events for the ClearPET LMF.
    
  \author Christian Morel
      
  $Date: 2005/02/01 15:32:28 $
  $Revision: 1.0 $
*/
/*
    Copyright (C) 2005- $Date: 2005/02/01 15:32:28 $, LPHE-EPFL
    See STIR/LICENSE.txt for details
*/
// TODO add check on size of event

#include "local/stir/listmode/CListRecordLMF.h"
#include "stir/listmode/CListRecordECAT966.h"
#include "stir/ProjDataInfoCylindricalNoArcCorr.h"
#include "stir/Bin.h"
#include "stir/Succeeded.h"
#include <algorithm>
#ifndef STIR_NO_NAMESPACES
using std::swap;
using std::streamsize;
using std::streampos;
#endif

START_NAMESPACE_STIR


void
CListEventDataLMF::
get_detection_coordinates(CartesianCoordinate3D<float>& coord_1,
			  CartesianCoordinate3D<float>& coord_2) const
  { 
    CListEventDataLMF event_data;
    event_data.get_detection_coordinates(coord_1, coord_2);
  }

END_NAMESPACE_STIR
