//
// $Id: EllipsoidalCylinder.inl,v 1.3 2004/03/19 11:09:47 kris Exp $
//
/*!
  \file
  \ingroup Shape

  \brief Inline-implementations of class EllipsoidalCylinder

  \author Sanida Mustafovic
  \author Kris Thielemans
  $Date: 2004/03/19 11:09:47 $
  $Revision: 1.3 $
*/
/*
    Copyright (C) 2000- $Date: 2004/03/19 11:09:47 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/
START_NAMESPACE_STIR

float EllipsoidalCylinder:: get_geometric_volume()const
 {
   return (radius_a*radius_b*_PI*length);
 }

Shape3D* EllipsoidalCylinder:: clone() const
{
  return static_cast<Shape3D *>(new EllipsoidalCylinder(*this));
}
void 
EllipsoidalCylinder::scale(const CartesianCoordinate3D<float>& scale3D)
{
  origin *= scale3D;
  length *= scale3D.z();
  radius_b *= scale3D.y();
  radius_a *= scale3D.x();
}

END_NAMESPACE_STIR
