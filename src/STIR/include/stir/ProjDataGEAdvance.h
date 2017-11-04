//
// $Id: ProjDataGEAdvance.h,v 1.4 2003/05/27 10:51:14 kris Exp $
//
/*!

  \file
  \ingroup projdata

  \brief Declaration of class ProjDataGEAdvance

  \author Damiano Belluzzo
  \author Kris Thielemans
  \author PARAPET project

  $Date: 2003/05/27 10:51:14 $

  $Revision: 1.4 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2003/05/27 10:51:14 $, IRSL
    See STIR/LICENSE.txt for details
*/
#ifndef __ProjDataGEAdvance_H__
#define __ProjDataGEAdvance_H__

#include "stir/ProjData.h" 
#include "stir/NumericType.h"
#include "stir/ByteOrder.h"
#include "stir/Array.h"

#include <iostream>
#include <vector>

#ifndef STIR_NO_NAMESPACES
using std::iostream;
using std::streamoff;
using std::vector;
#endif

START_NAMESPACE_STIR


/*!
  \ingroup projdata
  \brief A class which reads projection data from a GE Advance
  sinogram file.

  \warning support is still very basic. It assumes max_ring_diff == 11 and arc-corrected data.
  The sinogram has to be extracted from the database using the "official" GE BREAKPOINT strategy.
  (the file should contain 281 bins x 265 segments x 336 views.)

  No writing yet.

*/
class ProjDataGEAdvance : public ProjData
{
public:
    
    static  ProjDataGEAdvance* ask_parameters(const bool on_disk = true);
   
    
    ProjDataGEAdvance (iostream* s);
  
    //! Get & set viewgram 
    Viewgram<float> get_viewgram(const int view_num, const int segment_num,const bool make_num_tangential_poss_odd=false) const;
    Succeeded set_viewgram(const Viewgram<float>& v);
    
    //! Get & set sinogram 
    Sinogram<float> get_sinogram(const int ax_pos_num, const int sergment_num,const bool make_num_tangential_poss_odd=false) const; 
    Succeeded set_sinogram(const Sinogram<float>& s);
 
    
private:
  //the file with the data
  //This has to be a reference (or pointer) to a stream, 
  //because assignment on streams is not defined;
  // TODO make shared_ptr
  iostream* sino_stream;
  //offset of the whole 3d sinogram in the stream
  streamoff  offset;
  
  
  NumericType on_disk_data_type;
  
  ByteOrder on_disk_byte_order;
  
  // view_scaling_factor is only used when reading data from file. Data are stored in
  // memory as float, with the scale factor multiplied out
 
  Array<1,float> view_scaling_factor;
  
  vector<int> num_rings_orig;
  vector<int> segment_sequence_orig;


};

END_NAMESPACE_STIR


#endif
