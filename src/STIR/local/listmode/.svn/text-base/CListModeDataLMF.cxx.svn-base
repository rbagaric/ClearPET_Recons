//
// $Id: CListModeDataLMF.cxx,v 1.4 2004/10/24 12:32:19 kris Exp $
//
/*!
  \file
  \ingroup ClearPET_utilities
  \brief Implementation of class CListModeDataLMF
  \author Monica Vieira Martins
  \author Christian Morel
  \author Kris Thielemans
      
  $Date: 2004/10/24 12:32:19 $
  $Revision: 1.4 $
*/
/*
    Crystal Clear Collaboration
    Copyright (C) 2003 IPHE/UNIL, CH-1015 Lausanne
    Copyright (C) 2003- $Date: 2004/10/24 12:32:19 $, Hammersmith Imanet Ltd

    This software is distributed under the terms 
    of the GNU Lesser General  Public Licence (LGPL)
    See STIR/LICENSE.txt for details
*/


#include "local/stir/listmode/CListModeDataLMF.h"
#include "local/stir/listmode/CListRecordLMF.h"
#include "stir/Succeeded.h"
#include "stir/is_null_ptr.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>

#ifndef STIR_NO_NAMESPACES
using std::fstream;
using std::ifstream;
using std::iostream;
using std::ofstream;
using std::cerr;
using std::cout;
using std::flush;
using std::endl;
using std::min;
using std::max;
using std::streamsize;
using std::streampos;
#endif


FILE *pfCCS=NULL;

START_NAMESPACE_STIR

CListModeDataLMF::
CListModeDataLMF(const string& listmode_filename)
  : listmode_filename(listmode_filename)
{
  //opening and reading file.cch, filling in structure LMF_cch  

  // string::c_str() returns a "const char *", but LMFcchReader takes a "char*"
  // (which is a bad idea, unless you really want to modify the string)
  // at the moment, I gamble that LMFcchReader does not modify the string
  // TODO check (and ideally change argument types of LMFcchReader)

  if(LMFcchReader(const_cast<char*>(listmode_filename.c_str())))
    exit(EXIT_FAILURE);

  // adding the extension ".ccs"  to the listmode filename
  char *nameOfFileCCS = new char [80];
  strcpy(nameOfFileCCS,listmode_filename.c_str());
  strcat (nameOfFileCCS,".ccs");

  //opening file.ccs
  pfCCS = open_CCS_file2(nameOfFileCCS);  /* open the LMF binary file */

  if(pfCCS==NULL)
    error("Cannot open list mode file %s",listmode_filename.c_str());

  fseek(pfCCS,0L,0);                      /* find the begin of file */
  //allocate and fill in the encoding header structure 

  pEncoH = readHead(pfCCS);

}

CListModeDataLMF::
~CListModeDataLMF()
{
  if(pfCCS) {
    fclose(pfCCS);
  }

  LMFcchReaderDestructor();
  destroyReadHead();//ccsReadHeadDestructor
  destroy_findXYZinLMFfile(pEncoH);
}

std::string
CListModeDataLMF::
get_name() const
{
  return listmode_filename;
}

std::time_t 
CListModeDataLMF::
get_scan_start_time_in_secs_since_1970() const
{
  return std::time_t(-1);
}


shared_ptr <CListRecord> 
CListModeDataLMF::
get_empty_record_sptr() const
{
  return new CListRecordLMF();
}

Succeeded
CListModeDataLMF::
get_next_record(CListRecord& record) const
{
  if (is_null_ptr(pfCCS))
    return Succeeded::no;
  // check type

  assert(dynamic_cast<CListRecord *>(&record) != 0);

  // TODO ignores time

  double x1, y1, z1, x2, y2, z2;
  if (!findXYZinLMFfile(pfCCS,
		        &x1, &y1, &z1, &x2, &y2, &z2,
			pEncoH))
     return Succeeded::no;

  // Correspondance between Gate and STIR coordinates
  // ClearPET Gate -> STIR coordinates
  {
    double x1_tmp = x1;
    double x2_tmp = x2;
    double y1_tmp = y1;
    double y2_tmp = y2;

    y1 = -x1_tmp;
    y2 = -x2_tmp;
    x1 = y1_tmp;
    x2 = y2_tmp;
  }

  // ClearPET Lausanne -> STIR coordinates
  /*{
    y1 *= -1.;
    y2 *= -1.;
  }*/

  // ClearPET Raytest Lyon -> STIR coordinates
  /*{
    double x1_tmp = x1;
    double x2_tmp = x2;
    double y1_tmp = y1;
    double y2_tmp = y2;

    y1 = y1_tmp*cos(300.0*3.14/180.0)+x1_tmp*sin(300.0*3.14/180.0);
    y2 = y2_tmp*cos(300.0*3.14/180.0)+x2_tmp*sin(300.0*3.14/180.0);
    x1 = x1_tmp*cos(300.0*3.14/180.0)-y1_tmp*sin(300.0*3.14*180.0);
    x2 = x2_tmp*cos(300.0*3.14/180.0)-y2_tmp*sin(300.0*3.14*180.0);
  }*/

  CListEventDataLMF event_data;
  event_data.pos1().x() = static_cast<float>(x1);
  event_data.pos1().y() = static_cast<float>(y1);
  event_data.pos1().z() = static_cast<float>(z1);

//printf("Event 1 position x: %f	position y: %f	position z: %f\n",event_data.pos1().x(),event_data.pos1().y(),event_data.pos1().z());

  event_data.pos2().x() = static_cast<float>(x2);
  event_data.pos2().y() = static_cast<float>(y2);
  event_data.pos2().z() = static_cast<float>(z2);

//printf("Event 2 position x: %f	position y: %f	position z: %f\n",event_data.pos2().x(),event_data.pos2().y(),event_data.pos2().z());

//printf("%f	%f	%f\n%f	%f	%f\n",event_data.pos1().x(),event_data.pos1().y(),event_data.pos1().z(),event_data.pos2().x(),event_data.pos2().y(),event_data.pos2().z());

  // somehow we have to force record to be a coincidence event.
  // this can be done by assigning, but it is rather wasteful. Better make an appropriate member of CListRecordLMF.
  static_cast<CListRecordLMF&>(record) = event_data;
  return Succeeded::yes;
}

Succeeded
CListModeDataLMF::
reset()
{
  if (is_null_ptr(pfCCS))
    return Succeeded::no;

  if (!fseek(pfCCS,0L,0))                      /* find the begin of file */
    return Succeeded::no;
  else
    return Succeeded::yes;
}

// TODO do ftell and fseek really tell/change about the current listmode event
// or is there another LMF function?
CListModeData::SavedPosition
CListModeDataLMF::
save_get_position() 
{

  if (!is_null_ptr(pfCCS)) {
  // TODO should somehow check if ftell() worked and return an error if it didn't
    const unsigned long pos = ftell(pfCCS);
    saved_get_positions.push_back(pos);
    return saved_get_positions.size()-1;
  }
} 

Succeeded
CListModeDataLMF::
set_get_position(const CListModeDataLMF::SavedPosition& pos)
{
  if (is_null_ptr(pfCCS))
    return Succeeded::no;

  assert(pos < saved_get_positions.size());
  if (fseek(pfCCS, saved_get_positions[pos], 0))
    return Succeeded::no;
  else
    return Succeeded::yes;
}

#if 0
vector<unsigned long> 
CListModeDataLMF::
get_saved_get_positions() const
{
  return saved_get_positions;
}

void 
CListModeDataLMF::
set_saved_get_positions(const vector<unsigned long>& poss)
{
  saved_get_positions = poss;
}
#endif
END_NAMESPACE_STIR
