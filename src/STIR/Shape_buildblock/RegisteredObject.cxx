//
// $Id: RegisteredObject.cxx,v 1.5 2004/09/14 18:18:10 kris Exp $
//
/*!

  \file
  \ingroup Shape3D
  \brief instantiations of RegisteredObject for classes in Shape_buildblock
  (only useful for Microsoft Visual Studio 6.0)

  \author Kris Thielemans

  $Date: 2004/09/14 18:18:10 $
  $Revision: 1.5 $
*/
/*
    Copyright (C) 2000- $Date: 2004/09/14 18:18:10 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/
// note: include has to be before #ifdef as it's in this file that
// __STIR_REGISTRY_NOT_INLINE is defined
#include "stir/RegisteredObject.h"

#ifdef __STIR_REGISTRY_NOT_INLINE

#pragma message("instantiating RegisteredObject<Shape3D>")
#include "stir/Shape/Shape3D.h"

// and others
START_NAMESPACE_STIR

template <typename Root>
RegisteredObject<Root>::RegistryType& 
RegisteredObject<Root>::registry ()
{
  static RegistryType the_registry("None", 0);
  return the_registry;
}

#  ifdef _MSC_VER
// prevent warning message on reinstantiation, 
// note that we get a linking error if we don't have the explicit instantiation below
#  pragma warning(disable:4660)
#  endif

template RegisteredObject<Shape3D>; 

END_NAMESPACE_STIR

#endif
