//
// $Id: Coordinate2D.inl,v 1.7 2004/09/06 13:14:49 kris Exp $
//

/*!
  \file 
 
  \brief inline implementations for the Coordinate2D<coordT> class 

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
Coordinate2D<coordT>::Coordinate2D()
  : base_type()
{}

template <class coordT>
Coordinate2D<coordT>::Coordinate2D(const coordT& c1, 
				   const coordT& c2)
  : base_type()
{
  (*this)[1] = c1;
  (*this)[2] = c2;
}

template <class coordT>
Coordinate2D<coordT>::Coordinate2D(const base_type& c)
  : base_type(c)
{}

template <class coordT>
Coordinate2D<coordT>&
Coordinate2D<coordT>::operator=(const base_type& c)
{
  base_type::operator=(c);
  return *this;
}


END_NAMESPACE_STIR
