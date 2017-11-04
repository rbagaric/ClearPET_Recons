//
// $Id: ParsingObject.cxx,v 1.6 2003/07/18 10:06:05 kris Exp $
//
/*!

  \file
  \ingroup buildblock
  \brief Non-linline implementations for class ParsingObject

  \author Kris Thielemans
  \author Sanida Mustafovic

  $Date: 2003/07/18 10:06:05 $
  $Revision: 1.6 $
*/
/*
    Copyright (C) 2000- $Date: 2003/07/18 10:06:05 $, IRSL
    See STIR/LICENSE.txt for details
*/
#include "stir/ParsingObject.h"
#include <fstream>

#ifndef STIR_NO_NAMESPACE
using std::ifstream;
#endif

START_NAMESPACE_STIR

ParsingObject::ParsingObject() 
:
  keymap_is_initialised(false)
{}

  
  

ParsingObject::ParsingObject(const ParsingObject& par)
:
  keymap_is_initialised(false)
  {}


ParsingObject&
ParsingObject::operator =(const ParsingObject& par)
{
  if (&par == this) return *this;
  keymap_is_initialised = false;
  return *this;
}

void 
ParsingObject::
set_defaults()
{}

void
ParsingObject::
initialise_keymap()
{}

bool 
ParsingObject::
post_processing()
{ return false; }

void 
ParsingObject::
set_key_values()
{}

//void
bool
ParsingObject:: parse(istream& in) 
{ 
  // potentially remove the if() and always call initialise_keymap
  if (!keymap_is_initialised)
  {
    initialise_keymap(); 
    keymap_is_initialised = true;
  }
  set_key_values();
  if (!parser.parse(in))
  {
    warning("Error parsing.\n"); 
    return false;
  }
  else if (post_processing()==true)
    {
      warning("Error post processing keyword values.\n"); 
      return false;
    }
  else
    return true;
}


//void
bool
ParsingObject::parse(const char * const filename)
{
  ifstream hdr_stream(filename);
  if (!hdr_stream)
  { 
    error("ParsingObject::parse: couldn't open file %s\n", filename);
    return false;
  }
  return parse(hdr_stream);
}

void
ParsingObject::ask_parameters()
{
  // potentially remove the if() and always call initialise_keymap
  if (!keymap_is_initialised)
  {
    initialise_keymap(); 
    keymap_is_initialised = true;
  }
  // TODO drop next line
  set_defaults();
  set_key_values();

  while(true)
  {
    parser.ask_parameters();

    if (post_processing()==true)
    {
      warning("\nError post processing keyword values. Doing it all over again...\n"); 
    }
    else
      return;
  }
}

string
ParsingObject::parameter_info() 
{ 
  if (!keymap_is_initialised)
  {
    initialise_keymap(); 
    keymap_is_initialised = true;
  }
  set_key_values();
  return parser.parameter_info(); 
}

END_NAMESPACE_STIR