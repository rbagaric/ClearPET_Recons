//
// $Id: VoxelsOnCartesianGrid.inl,v 1.5 2004/06/30 17:09:14 kris Exp $
//
/*!
  \file 
  \ingroup densitydata  
  \brief inline implementations for the VoxelsOnCartesianGrid class 

  \author Sanida Mustafovic 
  \author Kris Thielemans 
  \author PARAPET project

  $Date: 2004/06/30 17:09:14 $
  $Revision: 1.5 $

*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2004/06/30 17:09:14 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/
START_NAMESPACE_STIR


template<class elemT>
CartesianCoordinate3D<float> 
VoxelsOnCartesianGrid<elemT>::get_voxel_size() const
{
  return CartesianCoordinate3D<float>(this->get_grid_spacing());
}




END_NAMESPACE_STIR
