//
// $Id: IndexRange3D.h,v 1.3 2004/03/15 17:30:40 kris Exp $
//

#ifndef __IndexRange3D_H__
#define __IndexRange3D_H__
/*! 
  \file
  \ingroup Array
  
  \brief This file declares the class IndexRange3D.

  \author Sanida Mustafovic
  \author Kris Thielemans
  \author PARAPET project

  $Date: 2004/03/15 17:30:40 $

  $Revision: 1.3 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2004/03/15 17:30:40 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/

#include "stir/IndexRange.h"

START_NAMESPACE_STIR


/*! 
   \ingroup Array
   \brief a 'convenience' class for 3D index ranges.
   Provides an easier constructor for regular ranges.
*/
class IndexRange3D : public IndexRange<3>

{
private:
  typedef IndexRange<3> base_type;

public:
  inline IndexRange3D();
  inline IndexRange3D(const IndexRange<3>& range_v);
  inline IndexRange3D(const int min_1, const int max_1,
                      const int min_2, const int max_2,
		      const int min_3, const int max_3);
  inline IndexRange3D(const int length_1, const int length_2, const int length_3);
  
  // TODO add 2 arg constructor with BasicCoordinate<2,int> min,max
};


END_NAMESPACE_STIR

#include "stir/IndexRange3D.inl"

#endif
