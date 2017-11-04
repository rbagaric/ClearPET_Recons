//
// $Id: lm_to_projdata.cxx,v 1.9 2004/03/19 14:57:19 kris Exp $
//
/*!
  \file 
  \ingroup listmode_utilities

  \brief Program to bin listmode data to 3d sinograms

  \see class LmToProjData for info on parameter file format

  \author Kris Thielemans
  \author Sanida Mustafovic
  
  $Date: 2004/03/19 14:57:19 $
  $Revision: 1.9 $
*/
/*
    Copyright (C) 2000- $Date: 2004/03/19 14:57:19 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/

#include "stir/listmode/LmToProjData.h"

#ifndef STIR_NO_NAMESPACES
using std::cerr;
#endif

USING_NAMESPACE_STIR



int main(int argc, char * argv[])
{
  
  if (argc!=1 && argc!=2) {
    cerr << "Usage: " << argv[0] << " [par_file]\n";
    exit(EXIT_FAILURE);
  }
  LmToProjData application(argc==2 ? argv[1] : 0);
  application.process_data();

  return EXIT_SUCCESS;
}

