//
//$Id: Coordinate4D.inl,v 1.7 2004/09/06 13:14:49 kris Exp $
//


/*!
  \file 
  \ingroup Coordinate  
  \brief inline implementations for the Coordinate4D<coordT> class 

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
Coordinate4D<coordT>::Coordinate4D()
  : base_type()
{}

template <class coordT>
Coordinate4D<coordT>::Coordinate4D(const coordT& c1, 
				   const coordT& c2, 
				   const coordT& c3, 
				   const coordT& c4)
  : base_type()
{
  (*this)[1] = c1;
  (*this)[2] = c2;
  (*this)[3] = c3;
  (*this)[4] = c4;
}

template <class coordT>
Coordinate4D<coordT>::Coordinate4D(const base_type& c)
  : base_type(c)
{}

template <class coordT>
Coordinate4D<coordT>&
Coordinate4D<coordT>::operator=(const base_type& c)
{
  base_type::operator=(c);
  return *this;
}


END_NAMESPACE_STIR
