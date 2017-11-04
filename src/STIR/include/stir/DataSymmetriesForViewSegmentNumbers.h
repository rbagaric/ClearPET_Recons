//
// $Id: DataSymmetriesForViewSegmentNumbers.h,v 1.6 2003/05/27 10:51:14 kris Exp $
//
/*!
  \file
  \ingroup projdata

  \brief Declaration of class DataSymmetriesForViewSegmentNumbers

  \author Kris Thielemans
  \author PARAPET project

  $Date: 2003/05/27 10:51:14 $
  $Revision: 1.6 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2003/05/27 10:51:14 $, IRSL
    See STIR/LICENSE.txt for details
*/
#ifndef __DataSymmetriesForViewSegmentNumbers_H__
#define __DataSymmetriesForViewSegmentNumbers_H__

#include "stir/common.h"
#include <vector>

#ifndef STIR_NO_NAMESPACES
using std::vector;
#endif

START_NAMESPACE_STIR

class ViewSegmentNumbers;

#if 0
class ViewSegmentIndexRange;
#endif


/*!
  \brief A class for encoding/finding symmetries. Works only on
  ViewSegmentNumbers (instead of Bin).

  This class (mainly used in RelatedViewgrams and the projectors)
  is useful to store and use all information on symmetries
  common between the image representation and the projection data.

  The class mainly defines members to find \c basic ViewSegmentNumbers. These form a 
  'basis' for all ViewSegmentNumbers in the sense that all ViewSegmentNumbers
  can be obtained by using symmetry operations on the 'basic' ones.
*/
class DataSymmetriesForViewSegmentNumbers
{
public:

  virtual ~DataSymmetriesForViewSegmentNumbers();

  virtual DataSymmetriesForViewSegmentNumbers * clone() const = 0;

#if 0
  // TODO
  //! returns the range of the indices for basic view/segments
  virtual ViewSegmentIndexRange
    get_basic_view_segment_index_range() const = 0;
#endif

  //! fills in a vector with all the view/segments that are related to 'v_s' (including itself)
  virtual void
    get_related_view_segment_numbers(vector<ViewSegmentNumbers>&, const ViewSegmentNumbers& v_s) const = 0;

  //! returns the number of view_segment_numbers related to 'v_s'
  /*! The default implementation is in terms of get_related_view_segment_numbers, which will be 
      slow of course */
  virtual int
    num_related_view_segment_numbers(const ViewSegmentNumbers& v_s) const;

  /*! \brief given an arbitrary view/segment, find the basic view/segment
  
  sets 'v_s' to the corresponding 'basic' view/segment and returns true if
  'v_s' is changed (i.e. it was NOT a basic view/segment).
  */  
  virtual bool
    find_basic_view_segment_numbers(ViewSegmentNumbers& v_s) const = 0;

  /*! \brief test if a view/segment is 'basic' 

  The default implementation uses find_basic_view_segment_numbers
  */
  virtual bool
    is_basic(const ViewSegmentNumbers& v_s) const;
};

END_NAMESPACE_STIR

#endif

