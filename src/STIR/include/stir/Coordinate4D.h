//
// $Id: Coordinate4D.h,v 1.5 2004/03/15 17:45:37 kris Exp $
//
#ifndef __Coordinate4D_H__
#define __Coordinate4D_H__
/*!
  \file 
  \ingroup Coordinate 
  \brief defines the Coordinate4D<coordT> class 

  \author Sanida Mustafovic 
  \author Kris Thielemans 
  \author PARAPET project

  $Date: 2004/03/15 17:45:37 $

  $Revision: 1.5 $

*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2004/03/15 17:45:37 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/



#include "stir/BasicCoordinate.h"

START_NAMESPACE_STIR
/*!
  \ingroup Coordinate 
  \brief a templated class for 4-dimensional coordinates.

   The only new method is a constructor Coordinate4D<coordT>(c1,c2,c3,c4)

   \warning Indices run from 1 to 4. 

*/
template <typename coordT>
class Coordinate4D : public BasicCoordinate<4, coordT>
{
protected:
  typedef BasicCoordinate<4, coordT> base_type;

public:
  inline Coordinate4D();
  inline Coordinate4D(const coordT&, const coordT&, const coordT&, const coordT&);
  inline Coordinate4D(const base_type& c);
  inline Coordinate4D& operator=(const base_type& c);
};

END_NAMESPACE_STIR

#include "stir/Coordinate4D.inl"

#endif

