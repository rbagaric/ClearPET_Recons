//
// $Id: ProjDataFromStream.h,v 1.7 2003/05/27 10:51:14 kris Exp $
//
/*!

  \file
  \ingroup projdata
  \brief Declaration of class ProjDataFromStream

  \author Sanida Mustafovic
  \author Kris Thielemans
  \author Claire Labbe
  \author PARAPET project

  $Date: 2003/05/27 10:51:14 $

  $Revision: 1.7 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2003/05/27 10:51:14 $, IRSL
    See STIR/LICENSE.txt for details
*/
#ifndef __ProjDataFromStream_H__
#define __ProjDataFromStream_H__

#include "stir/ProjData.h" 
#include "stir/NumericType.h"
#include "stir/ByteOrder.h"
#include "stir/shared_ptr.h"

#include <iostream>
#include <vector>

#ifndef STIR_NO_NAMESPACES
using std::istream;
using std::iostream;
using std::streamoff;
using std::vector;
#endif

START_NAMESPACE_STIR


/*!
  \ingroup projdata
  \brief A class which reads/writes projection data from/to a (binary) stream.

  Mainly useful for Interfile data.

  \warning Data have to be contiguous.
  \warning The parameter make_num_tangential_poss_odd (used in various 
  get_ functions) is temporary and will be removed soon.

*/
class ProjDataFromStream : public ProjData
{
public:

  enum StorageOrder {
    Segment_AxialPos_View_TangPos, Segment_View_AxialPos_TangPos, 
    Unsupported };
    
  static  ProjDataFromStream* ask_parameters(const bool on_disk = true);

#if 0   
  //! Empty constructor
  ProjDataFromStream()
    {}
#endif    
    
  //! constructor taking all necessary parameters
  /*! 
    \param segment_sequence_in_stream has to be set according to the order
    in which the segments occur in the stream. segment_sequence_in_stream[i]
    is the segment number of the i-th segment in the stream.
  */
  ProjDataFromStream (shared_ptr<ProjDataInfo> const& proj_data_info_ptr,
		      shared_ptr<iostream> const& s, 
		      const streamoff offs, 
		      const vector<int>& segment_sequence_in_stream,
		      StorageOrder o = Segment_View_AxialPos_TangPos,
		      NumericType data_type = NumericType::FLOAT,
		      ByteOrder byte_order = ByteOrder::native,  
		      float scale_factor = 1 );

  //! as above, but with a default value for segment_sequence_in_stream
  /*! The default value for segment_sequence_in_stream is a vector with
    values min_segment_num, min_segment_num+1, ..., max_segment_num
  */
  ProjDataFromStream (shared_ptr<ProjDataInfo> const& proj_data_info_ptr,
		      shared_ptr<iostream> const& s, 
		      const streamoff offs = 0, 
		      StorageOrder o = Segment_View_AxialPos_TangPos,
		      NumericType data_type = NumericType::FLOAT,
		      ByteOrder byte_order = ByteOrder::native,  
		      float scale_factor = 1 );
  //! Obtain the storage order
  inline StorageOrder get_storage_order() const;
    
  //! Get the offset -Changed into streamoff from int
  //inline int get_offset_in_stream() const;
  inline streamoff get_offset_in_stream() const;
    
  //! Get the data_type in the stream 
  inline NumericType get_data_type_in_stream() const;
    
  //! Get the byte order
  inline ByteOrder get_byte_order_in_stream() const;   
    
  //! Get the segment sequence
  inline vector<int> get_segment_sequence_in_stream() const;
    
  //! Get & set viewgram 
  Viewgram<float> get_viewgram(const int view_num, const int segment_num,const bool make_num_tangential_poss_odd=false) const;
  Succeeded set_viewgram(const Viewgram<float>& v);
    
  //! Get & set sinogram 
  Sinogram<float> get_sinogram(const int ax_pos_num, const int segment_num,const bool make_num_tangential_poss_odd=false) const; 
  Succeeded set_sinogram(const Sinogram<float>& s);
    
  //! Get all sinograms for the given segment
  SegmentBySinogram<float> get_segment_by_sinogram(const int segment_num) const;
  //! Get all viewgrams for the given segment
  SegmentByView<float> get_segment_by_view(const int segment_num) const;
    
    
  //! Set all sinograms for the given segment
  Succeeded set_segment(const SegmentBySinogram<float>&);
  //! Set all viewgrams for the given segment
  Succeeded set_segment(const SegmentByView<float>&);

  //! Get scale factor
  float get_scale_factor() const;  

    
protected:
  //! the stream with the data
  shared_ptr<iostream> sino_stream;

private:
  //! offset of the whole 3d sinogram in the stream
  streamoff  offset;
  
  
  //!the order in which the segments occur in the stream
  vector<int> segment_sequence;
  
  inline int find_segment_index_in_sequence(const int segment_num) const;
  
  StorageOrder storage_order;
  
  NumericType on_disk_data_type;
  
  ByteOrder on_disk_byte_order;
  
  // scale_factor is only used when reading data from file. Data are stored in
  // memory as float, with the scale factor multiplied out
  float scale_factor;
  
  //! Calculate the offset for the given segmnet
  streamoff get_offset_segment(const int segment_num) const;
  
  //! Calculate offsets for viewgram data  
  vector<streamoff> get_offsets(const int view_num, const int segment_num) const;
  //! Calculate offsets for sinogram data
  vector<streamoff> get_offsets_sino(const int ax_pos_num, const int segment_num) const;
    
  
};

END_NAMESPACE_STIR

#include "stir/ProjDataFromStream.inl"

#endif