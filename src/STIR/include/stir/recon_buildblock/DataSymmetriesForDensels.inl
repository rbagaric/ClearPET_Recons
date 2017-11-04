//
// $Id: DataSymmetriesForDensels.inl,v 1.3 2002/02/22 12:46:32 kris Exp $
//
/*!

  \file
  \ingroup buildblock

  \brief inline implementations for class DataSymmetriesForDensels

  \author Kris Thielemans
  
  $Date: 2002/02/22 12:46:32 $
  $Revision: 1.3 $
*/
/*
    Copyright (C) 2001- $Date: 2002/02/22 12:46:32 $, IRSL
    See STIR/LICENSE.txt for details
*/

#include "stir/Densel.h"

START_NAMESPACE_STIR
void
DataSymmetriesForDensels::
    get_related_densels(vector<Densel>& rel_b, const Densel& b) const
{
  get_related_densels(rel_b, b,
                   proj_data_info_ptr->get_min_axial_pos_num(b.segment_num()), 
                   proj_data_info_ptr->get_max_axial_pos_num(b.segment_num()),
                   proj_data_info_ptr->get_min_tangential_pos_num(), 
                   proj_data_info_ptr->get_max_tangential_pos_num());
}



END_NAMESPACE_STIR
