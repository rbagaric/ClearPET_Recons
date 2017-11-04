//
// $Id: DiscretisedDensity.inl,v 1.3 2003/05/27 10:51:58 kris Exp $
//

/*!
  \file 
  \ingroup densitydata! 
  \brief  inline implementation for DiscretisedDensity

  \author Sanida Mustafovic 
  \author Kris Thielemans 
  \author (help from Alexey Zverovich)
  \author PARAPET project

  $Date: 2003/05/27 10:51:58 $

  $Revision: 1.3 $

*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2003/05/27 10:51:58 $, IRSL
    See STIR/LICENSE.txt for details
*/

START_NAMESPACE_STIR

template<int num_dimensions, typename elemT>
DiscretisedDensity<num_dimensions,elemT>::DiscretisedDensity()
{}

template<int num_dimensions, typename elemT>
DiscretisedDensity<num_dimensions, elemT>::
DiscretisedDensity(const IndexRange<num_dimensions>& range_v,
		   const CartesianCoordinate3D<float>& origin_v)
  : Array<num_dimensions,elemT>(range_v),
    origin(origin_v)    
{}

template<int num_dimensions, typename elemT>
void
DiscretisedDensity<num_dimensions, elemT>::
set_origin(const CartesianCoordinate3D<float> &origin_v)
{
  origin = origin_v;
}

template<int num_dimensions, typename elemT>
const CartesianCoordinate3D<float>& 
DiscretisedDensity<num_dimensions, elemT>::
get_origin()  const 
{ return origin; }

END_NAMESPACE_STIR
