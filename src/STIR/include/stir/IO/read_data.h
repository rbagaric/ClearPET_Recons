// $Id: read_data.h,v 1.5 2004/09/14 22:51:09 kris Exp $
#ifndef __stir_IO_read_data_H__
#define __stir_IO_read_data_H__

/*!
  \file 
  \ingroup Array_IO 
  \brief declarations of read_data() functions for reading Arrays from file

  \author Kris Thielemans

  $Date: 2004/09/14 22:51:09 $
  $Revision: 1.5 $
*/
/*
    Copyright (C) 2004- $Date: 2004/09/14 22:51:09 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/

#include "stir/common.h"

START_NAMESPACE_STIR

class ByteOrder;
class Succeeded;
class NumericType;
template <class T> class NumericInfo;
template <int num_dimensions, class elemT> class Array;

#if defined(_MSC_VER) && _MSC_VER==1200
// VC 6.0 cannot compile this when the templates are declared first, 
// and defined in the .inl
#  define __STIR_WORKAROUND_TEMPLATES 1
#endif

#ifndef __STIR_WORKAROUND_TEMPLATES

/*! \ingroup Array_IO
  \brief Read the data of an Array from file.

  Only the data will be written, not the dimensions, start indices, nor byte-order.
  Hence, this should only used for low-level IO.

  \a IStreamT is supposed to be stream or file type (see implementations for 
  read_data_1d()).

  \warning When an error occurs, the function immediately returns. 
  However, the data might have been partially read from \a s.
*/
template <int num_dimensions, class IStreamT, class elemT>
inline Succeeded 
read_data(IStreamT& s, Array<num_dimensions,elemT>& data, 
	  const ByteOrder byte_order=ByteOrder::native);

/*! \ingroup Array_IO
  \brief Read the data of an Array from file as a different type.

  This function essentially first calls convert_data() to construct
  an array with elements of type \a InputType, and then calls 
  read_data(IStreamT&, const Array<num_dimensions,elemT>&, 
	   const ByteOrder, const bool).
  \see read_data(IStreamT&, const Array<num_dimensions,elemT>&, 
	   const ByteOrder, const bool)

  \see find_scale_factor() for the meaning of \a scale_factor.
*/
template <int num_dimensions, class IStreamT, class elemT, class InputType, class ScaleT>
inline Succeeded 
read_data(IStreamT& s, Array<num_dimensions,elemT>& data, 
	  NumericInfo<InputType> input_type, 
	  ScaleT& scale_factor,
	  const ByteOrder byte_order=ByteOrder::native);

/*! \ingroup Array_IO
  \brief Read the data of an Array from file as a different type.

  \see read_data(IStreamT&, const Array<num_dimensions,elemT>&, 
	   NumericInfo<InputType>, 
	   ScaleT&,
	   const ByteOrder,
	   const bool)
  The only difference is that the input type is now specified using NumericType.
*/
template <int num_dimensions, class IStreamT, class elemT, class ScaleT>
inline Succeeded 
read_data(IStreamT& s, 
	   Array<num_dimensions,elemT>& data, 
	   NumericType type, ScaleT& scale,
	   const ByteOrder byte_order=ByteOrder::native);
#endif

END_NAMESPACE_STIR

#include "stir/IO/read_data.inl"

#endif