//
// $Id: Coordinate3D.inl,v 1.7 2004/09/06 13:14:49 kris Exp $
//

/*!
  \file 
  \ingroup Coordinate  
  \brief inline implementations for the Coordinate3D<coordT> class 

  \author Sanida Mustafovic 
  \author Kris Thielemans 
  \author PARAPET project

  $Date: 2004/09/06 13:14:49 $

  $Revision: 1.7 $

*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2004/09/06 13:14:49 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/



START_NAMESPACE_STIR

template <class coordT>
Coordinate3D<coordT>::Coordinate3D()
  : base_type()
{}

template <class coordT>
Coordinate3D<coordT>::Coordinate3D(const coordT& c1, 
				   const coordT& c2, 
				   const coordT& c3)
  : base_type()
{
  (*this)[1] = c1;
  (*this)[2] = c2;
  (*this)[3] = c3;
}

template <class coordT>
Coordinate3D<coordT>::Coordinate3D(const base_type& c)
  : base_type(c)
{}

template <class coordT>
Coordinate3D<coordT>&
Coordinate3D<coordT>::operator=(const base_type& c)
{
  base_type::operator=(c);
  return *this;
}


END_NAMESPACE_STIR
