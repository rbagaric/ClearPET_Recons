//
// $Id: DataSymmetriesForViewSegmentNumbers.cxx,v 1.2 2003/05/27 10:49:58 kris Exp $
//
/*!
  \file
  \ingroup projdata 
  \brief Implementations for class DataSymmetriesForViewSegmentNumbers

  \author Kris Thielemans
  \author PARAPET project

  $Date: 2003/05/27 10:49:58 $
  $Revision: 1.2 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2003/05/27 10:49:58 $, IRSL
    See STIR/LICENSE.txt for details
*/

#include "stir/DataSymmetriesForViewSegmentNumbers.h"
#include "stir/ViewSegmentNumbers.h"

START_NAMESPACE_STIR

DataSymmetriesForViewSegmentNumbers::
~DataSymmetriesForViewSegmentNumbers()
{}

int
DataSymmetriesForViewSegmentNumbers::num_related_view_segment_numbers(const ViewSegmentNumbers& vs) const
{
  vector<ViewSegmentNumbers> rel_vs;
  get_related_view_segment_numbers(rel_vs, vs);
  return rel_vs.size();
}

bool
DataSymmetriesForViewSegmentNumbers::
is_basic(const ViewSegmentNumbers& v_s) const
{
  ViewSegmentNumbers copy = v_s;
  return !find_basic_view_segment_numbers(copy);
}

END_NAMESPACE_STIR
