//
// $Id: interfile_keyword_functions.cxx,v 1.3 2001/12/20 21:17:53 kris Exp $
//
/*!
  \file
  \ingroup buildblock

  \brief Implementations for standardise_interfile_keyword

  \author Kris Thielemans
  \author PARAPET project

  $Date: 2001/12/20 21:17:53 $
  $Revision: 1.3 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2001/12/20 21:17:53 $, IRSL
    See STIR/LICENSE.txt for details
*/

#include "stir/interfile_keyword_functions.h"
#include <ctype.h>

START_NAMESPACE_STIR

string 
standardise_interfile_keyword(const string& keyword)
{
  string::size_type cp =0;		//current index
  char const * const white_space = " \t_!";
  
  // skip white space
  cp=keyword.find_first_not_of(white_space,0);
  
  if(cp==string::npos)
    return string();
  
  // remove trailing white spaces
  const string::size_type eok=keyword.find_last_not_of(white_space);
  
  string kw;
  kw.reserve(eok-cp+1);
  bool previous_was_white_space = false;
  while (cp <= eok)
  {
    if (isspace(static_cast<int>(keyword[cp])) || keyword[cp]=='_' || keyword[cp]=='!')
    {
      if (!previous_was_white_space)
      {
        kw.append(1,' ');
        previous_was_white_space = true;
      }
      // else: skip this white space character
    }
    else
    {
      kw.append(1,static_cast<char>(tolower(static_cast<int>(keyword[cp]))));
      previous_was_white_space = false;
    }
    ++cp;
  }
  return kw;
}


END_NAMESPACE_STIR
