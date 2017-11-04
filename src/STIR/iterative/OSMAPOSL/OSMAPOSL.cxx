//
// $Id: OSMAPOSL.cxx,v 1.5 2003/05/23 18:09:19 kris Exp $
//
/*!

  \file
  \ingroup main_programs
  \brief main() for OSMAPOSLReconstruction

  \author Matthew Jacobson
  \author Kris Thielemans
  \author PARAPET project

  $Date: 2003/05/23 18:09:19 $
  $Revision: 1.5 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2003/05/23 18:09:19 $, IRSL
    See STIR/LICENSE.txt for details
*/
#include "stir/OSMAPOSL/OSMAPOSLReconstruction.h"
#include "stir/Succeeded.h"


USING_NAMESPACE_STIR

#ifdef PARALLEL
int master_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{

  OSMAPOSLReconstruction reconstruction_object(argc>1?argv[1]:"");
  

  return reconstruction_object.reconstruct() == Succeeded::yes ?
           EXIT_SUCCESS : EXIT_FAILURE;


}

