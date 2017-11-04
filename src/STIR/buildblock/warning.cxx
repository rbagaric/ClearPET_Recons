//
// $Id: warning.cxx,v 1.5 2004/09/10 16:56:23 kris Exp $
//
/*!
  \file 
 
  \brief defines the warning() function

  \author Kris Thielemans
  \author PARAPET project

  $Date: 2004/09/10 16:56:23 $

  $Revision: 1.5 $

*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2004/09/10 16:56:23 $, Hammersmith Imanet Ltd
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

void warning(const char *const s, ...)
{
  va_list ap;
  va_start(ap, s);
  const unsigned size=10000;
  char tmp[size];
  const int returned_size= vsnprintf(tmp,size, s, ap);
  if (returned_size<0)
    std::cerr << "\nWARNING: error formatting warning message" << std::endl;
  else
    {
      std::cerr << "\nWARNING: " << tmp <<std::endl;
      if (static_cast<unsigned>(returned_size)>=size)
	std::cerr << "\nWARNING: previous warning message truncated as it exceeds "
		  << size << "bytes" << std::endl;
    }
}

END_NAMESPACE_STIR
