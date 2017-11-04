//
// $Id: error.cxx,v 1.5 2004/09/10 16:56:14 kris Exp $
//
/*!
  \file 
 
  \brief defines the error() function

  \author Kris Thielemans
  \author PARAPET project

  $Date: 2004/09/10 16:56:14 $
  $Revision: 1.5 $

*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2004/09/10 16:56:14 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/
#include "stir/common.h"

#include <cstdarg>
#include <stdlib.h>
#include <iostream>
/* Warning: vsnprintf is only ISO C99. So your compiler might not have it.
   Visual Studio can be accomodated with the following work-around
*/
#ifdef BOOST_MSVC
#define vsnprintf _vsnprintf
#endif

START_NAMESPACE_STIR

void error(const char *const s, ...)
{  
  va_list ap;
  va_start(ap, s);
  const unsigned size=10000;
  char tmp[size];
  const int returned_size= vsnprintf(tmp,size, s, ap);
  if (returned_size<0)
    std::cerr << "\nERROR: but error formatting error message" << std::endl;
  else
    {
      std::cerr << "\nERROR: " << tmp <<std::endl;
      if (static_cast<unsigned>(returned_size)>=size)
	std::cerr << "\nWARNING: previous error message truncated as it exceeds "
		  << size << "bytes" << std::endl;
    }
  exit(EXIT_FAILURE);
}
END_NAMESPACE_STIR
