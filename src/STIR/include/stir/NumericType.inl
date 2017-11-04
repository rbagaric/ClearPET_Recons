//
// $Id: NumericType.inl,v 1.5 2001/12/20 21:18:03 kris Exp $
//
/*!
  \file 
  \ingroup buildblock 
  \brief Implementation of inline methods of class NumericType.

  \author Kris Thielemans 
  \author PARAPET project

  $Date: 2001/12/20 21:18:03 $

  $Revision: 1.5 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2001/12/20 21:18:03 $, IRSL
    See STIR/LICENSE.txt for details
*/

START_NAMESPACE_STIR

NumericType::NumericType(Type t)
: id(t)
{}

bool NumericType::operator==(NumericType type) const
{ 
  return id == type.id; 
}

bool NumericType::operator!=(NumericType type) const
{ 
  return !(*this == type); 
}

END_NAMESPACE_STIR
