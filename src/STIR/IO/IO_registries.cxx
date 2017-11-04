//
// $Id: IO_registries.cxx,v 1.4 2003/05/19 06:47:35 kris Exp $
//
/*!

  \file
  \ingroup IO

  \brief File that registers all RegisterObject children in IO

  \author Kris Thielemans
  
  $Date: 2003/05/19 06:47:35 $
  $Revision: 1.4 $
*/
/*
    Copyright (C) 2002- $Date: 2003/05/19 06:47:35 $, IRSL
    See STIR/LICENSE.txt for details
*/

#include "stir/IO/InterfileOutputFileFormat.h"
#include "stir/IO/ECAT6OutputFileFormat.h"
#ifdef HAVE_LLN_MATRIX
#include "stir/IO/ECAT7OutputFileFormat.h"
#endif

START_NAMESPACE_STIR

static InterfileOutputFileFormat::RegisterIt dummy1;
START_NAMESPACE_ECAT
START_NAMESPACE_ECAT6
static ECAT6OutputFileFormat::RegisterIt dummy2;
END_NAMESPACE_ECAT6
#ifdef HAVE_LLN_MATRIX
START_NAMESPACE_ECAT7
static ECAT7OutputFileFormat::RegisterIt dummy3;
END_NAMESPACE_ECAT7
#endif
END_NAMESPACE_ECAT
END_NAMESPACE_STIR
