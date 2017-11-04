//
// $Id: IndexRange4D.h,v 1.3 2004/03/15 17:30:40 kris Exp $
//
/*! 
  \file
  \ingroup Array
 
  \brief  This file declares the class IndexRange4D.

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

#ifndef __IndexRange4D_H__
#define __IndexRange4D_H__

#include "stir/IndexRange.h"

START_NAMESPACE_STIR

/*!
  \ingroup Array
  \brief A convenience class for 4D index ranges

  This class is derived from IndexRange<4>. It just provides some 
  extra constructors for making 'regular' ranges.
*/
class IndexRange4D : public IndexRange<4>

{
private:
  typedef IndexRange<4> base_type;

public:
  inline IndexRange4D();
  inline IndexRange4D(const IndexRange<4>& range_v);
  inline IndexRange4D(const int min_1, const int max_1,
                      const int min_2, const int max_2,
                      const int min_3, const int max_3,
		      const int min_4, const int max_4);
  inline IndexRange4D(const int length_1, 
                      const int length_2, 
                      const int length_3, 
                      const int length_4);
};


END_NAMESPACE_STIR

#include "stir/IndexRange4D.inl"

#endif
