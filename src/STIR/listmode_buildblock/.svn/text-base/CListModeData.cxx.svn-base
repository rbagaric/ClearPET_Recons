//
// $Id: CListModeData.cxx,v 1.7 2004/03/19 15:25:11 kris Exp $
//
/*!
  \file
  \ingroup listmode
  \brief Implementation of class CListModeData 
    
  \author Kris Thielemans
      
  $Date: 2004/03/19 15:25:11 $
  $Revision: 1.7 $
*/
/*
    Copyright (C) 2003- $Date: 2004/03/19 15:25:11 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/

#include "stir/listmode/CListModeDataFromStream.h"
#include "stir/listmode/CListModeDataECAT.h"
#include "local/stir/listmode/CListModeDataLMF.h"

START_NAMESPACE_STIR

CListModeData::
CListModeData()
{
}

CListModeData::
~CListModeData()
{}

CListModeData*
CListModeData::
read_from_file(const string& filename)
{
  //return new CListModeDataFromStream(filename);
  //return new CListModeDataECAT(filename);
  return new CListModeDataLMF(filename);
}

const Scanner* 
CListModeData::
get_scanner_ptr() const
{
  return scanner_ptr.get();
}
END_NAMESPACE_STIR
