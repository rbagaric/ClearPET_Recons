
//
// $Id: LMF_Interfile.h         Exp $
//
/*!
  \file 
  \ingroup ClearPET_library

  \brief  
 
  \author Sanida Mustafovic
  \author Kris Thielemans
  \author Monica Vieira Martins
  \author Christian Morel
  
  \date $Date: $
  \version $$
*/

#include "stir/IO/interfile.h"
#include "stir/utilities.h"
#include "stir/Scanner.h"
#include "stir/ProjDataFromStream.h"
#include "stir/ProjDataInfoCylindricalArcCorr.h"
#include "stir/CartesianCoordinate3D.h"
#include "stir/VectorWithOffset.h"
#include "stir/shared_ptr.h"

#include <vector>
#include <fstream>

#ifndef STIR_NO_NAMESPACES
using std::iostream;
using std::ofstream;
using std::fstream;
using std::ifstream;
using std::cout;
using std::endl;
#endif

START_NAMESPACE_STIR

// rounding the number
int round(double num);

// Read the ClearPET file and return pointer to the ProjDatafromStream
//  All parsing is done in here, create_ClearPET_PDFS() is called from here
ProjDataFromStream*
read_ClearPET_PDFS(const string& ClearPET_file_name);
// Create the PDFS for ClearPET- this will take input stream 
//stream + offset as well as the all ... read_ClearPET_PDFS 
// KT+CM added OPS_flag argument
/* OPS_flag is currently uses as follows:
   OPS_flag == true: 
      num_axial_poss[seg] = max_num_axial_poss (appropriate for OPS rebinned data)
   OPS_flag == false:
      num_axial_poss[seg] is smaller taking into account 2 things:
        - axial_sampling[seg] is constant (so already interpolated)
	- only consistent data are allowed 
	  (where the FOV is determined using max_num_axial_poss and num_tang_poss)
*/
ProjDataFromStream*
create_ClearPET_PDFS(shared_ptr<iostream> const& s,const long offset, float ring_radius,
		     int nbphi,
		     int nbtheta,float dtheta,float dxyr,float ring_spacing,
		     const int max_num_axialpos,
		     const int,bool OPS_flag); 

// This function will read ClearPET file and return * pdfs
// extension '.hs' will be added for the header file_name and
// the interfile for the sinogram data will be returned
Succeeded 
write_interfile_header_for_ClearPET_sinogram(const string& ClearPET_file_name,
					     const ProjDataFromStream* pdfs );

END_NAMESPACE_STIR
