//
// $Id: CListModeDataLMF.h,v 1.3 2004/05/11 19:19:54 kris Exp $
//
/*!
  \file
  \ingroup ClearPET_utilities
  \brief Declaration of class CListModeDataLMF
    
  \author Kris Thielemans
      
  $Date: 2004/05/11 19:19:54 $
  $Revision: 1.3 $
*/
/*
    Copyright (C) 2003- $Date: 2004/05/11 19:19:54 $, Hammersmith Imanet Ltd

    This software is distributed under the terms 
    of the GNU Lesser General  Public Licence (LGPL)
    See STIR/LICENSE.txt for details
*/

#ifndef __stir_listmode_CListModeDataLMF_H__
#define __stir_listmode_CListModeDataLMF_H__

#include "stir/listmode/CListModeDataFromStream.h"
#include "stir/listmode/CListModeData.h"
#include "stir/shared_ptr.h"
#include "lmf.h"

#include <stdio.h>
#include <string>
#include <vector>

#ifndef STIR_NO_NAMESPACES
using std::string;
using std::vector;
#endif

START_NAMESPACE_STIR


//! A class that reads the listmode data from an LMF file
class CListModeDataLMF : public CListModeData
{
public:

  //! Constructor taking a filename
  CListModeDataLMF(const string& listmode_filename);

  // Destructor closes the file and destroys various structures
  ~CListModeDataLMF();

  virtual std::string
    get_name() const;

  virtual
    std::time_t get_scan_start_time_in_secs_since_1970() const;

  virtual 
    shared_ptr <CListRecord> get_empty_record_sptr() const;

  //! LMF listmode data stores delayed events as well (as opposed to prompts)
  virtual bool has_delayeds() const 
    { return true; } // TODO always?

  virtual 
    Succeeded get_next_record(CListRecord& event) const;

  virtual 
    Succeeded reset();

  virtual 
    SavedPosition save_get_position();

  virtual 
    Succeeded set_get_position(const SavedPosition&);

private:

  string listmode_filename;
  // TODO we really want to make this a shared_ptr I think to avoid memory leaks when throwing exceptions
  struct LMF_ccs_encodingHeader *pEncoH;
  FILE *pfCCS;                                

  vector<unsigned long> saved_get_positions;

};

END_NAMESPACE_STIR

#endif
