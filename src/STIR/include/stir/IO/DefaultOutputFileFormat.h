//
// $Id: DefaultOutputFileFormat.h,v 1.1 2003/05/23 14:44:42 kris Exp $
//
/*!
  \file
  \ingroup IO
  \brief typedef DefaultOutputFileFormat

  \author Kris Thielemans

  $Date: 2003/05/23 14:44:42 $
  $Revision: 1.1 $
*/
/*
    Copyright (C) 2003-$Date: 2003/05/23 14:44:42 $, IRSL
    See STIR/LICENSE.txt for details
*/

#ifndef __stir_IO_DefaultOutputFileFormat_H__
#define __stir_IO_DefaultOutputFileFormat_H__

#include "stir/IO/InterfileOutputFileFormat.h"


START_NAMESPACE_STIR


/*!
  \ingroup IO
  \brief typedef that defines the default OutputFileFormat 

  This can/will be used by programs that do not want to ask the user
  for the output file format. This typedef could be changed to any of the supported
  derived classes of OutputFileFormat
 */
typedef InterfileOutputFileFormat DefaultOutputFileFormat;

END_NAMESPACE_STIR

#endif
