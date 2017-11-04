//
// $Id: LMF_Interfile.cxx,  Exp $
//
/*!
  \file 
  \ingroup ClearPET_library

  \brief    Convertions from ClearPET  to Interfile images
 
  \author Sanida Mustafovic
  \author Kris Thielemans
  \author Monica Vieira Martins
  \author Christian Morel
  
  \date $Date:  $
  \version $Revision:  $
*/

#include "stir/IO/interfile.h"
#include "stir/utilities.h"
#include "stir/Scanner.h"
#include "stir/listmode/LMF_Interfile.h"
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
using std::flush;
#endif

START_NAMESPACE_STIR

ProjDataFromStream* create_ClearPET_PDFS(shared_ptr<iostream> const& s,
					 const long file_offset, 
					 float ring_radius,
					 int nbphi, 
					 int nbtheta,
					 float dtheta,
					 float dxyr,
					 float ring_spacing,
					 const int max_num_of_axialpos,
					 const int max_num_of_bins,
					 bool OPS_flag) // KT+CM added flag
  
{
  
  int limit;
  vector<int> segment_seq(nbtheta);
  limit = static_cast<int>(floor(nbtheta/2));
  
  VectorWithOffset<int> min_ring_difference(-limit,limit);
  VectorWithOffset<int> max_ring_difference(-limit,limit);
  VectorWithOffset<int> num_axial_pos_per_segment(-limit,limit);

  const float radius = ring_radius;
  for (int i=-limit;i<=limit;i++) {
       segment_seq[i+limit] = -i;
       min_ring_difference[i] = i;
       max_ring_difference[i] = i;
       // KT+CM added
       if (OPS_flag == true)
	 num_axial_pos_per_segment[i] = max_num_of_axialpos;
       else {
	 num_axial_pos_per_segment[i] = max_num_of_axialpos - abs(i);	   
       }
  }
  
  int num_rings = max_num_of_axialpos;
  int num_bins = max_num_of_bins;
  int max_num_views = nbphi;
  
  float bin_size = dxyr;

  Scanner::Type type_v = Scanner::Unknown_scanner;
  const string name = "ClearPET";
  
  int num_detectors_per_ring = max_num_views*2; 
  float RingRadius_v = radius;
  float intrTilt_v = 0;

  shared_ptr<Scanner> scanner_sptr = new Scanner(type_v,
						 name,
						 num_detectors_per_ring,
						 num_rings,
						 num_bins,
						 num_bins,
						 67.5,1,1.15,
						 bin_size,intrTilt_v,4,1,8,8,1,1,2);
  
  ProjDataInfoCylindricalArcCorr *proj_data_info_arccorr_ptr = new ProjDataInfoCylindricalArcCorr(scanner_sptr,
												  bin_size,
												  num_axial_pos_per_segment,
												  min_ring_difference,
												  max_ring_difference,
												  max_num_views, 
												  num_bins);
  
  // KT consistency check on theta
  for (int seg_num = proj_data_info_arccorr_ptr->get_min_segment_num();
       seg_num <= proj_data_info_arccorr_ptr->get_max_segment_num();
       ++seg_num) {
    const double theta = atan(proj_data_info_arccorr_ptr->get_tantheta(Bin(seg_num,0,0,0)));
    if (fabs(theta - dtheta * seg_num) > 1E-3) warning("create_ClearPET_PDFS: not-matching theta at segment %d: should be %g, found %g\n", seg_num, dtheta * seg_num, theta);
  }

  // consistency check on axial sampling
  {
    const float new_ax_sampling = proj_data_info_arccorr_ptr->get_axial_sampling(0);
    if (fabs(new_ax_sampling - ring_spacing) > 1E-5) warning("create_ClearPET_PDFS: not-matching axial sampling at segment 0: should be %g, found %g\n", dxyr, new_ax_sampling);
  }    
  
  const ProjDataFromStream::StorageOrder storage_order = ProjDataFromStream::Segment_View_AxialPos_TangPos;
  const float scale_factor =1; 
  
  return new ProjDataFromStream(proj_data_info_arccorr_ptr,
				s,
				file_offset,
				segment_seq,
				storage_order,
				NumericType::FLOAT,
				ByteOrder::little_endian,
				scale_factor);

}//end of create_ClearPET__PDFS

Succeeded write_interfile_header_for_ClearPET_sinogram(const string& ClearPET_file_name,
						       const ProjDataFromStream* pdfs )
{
  char header_file_name[max_filename_length];
  strcpy(header_file_name,ClearPET_file_name.c_str());
  replace_extension(header_file_name,".hs");
  
  return write_basic_interfile_PDFS_header(header_file_name, ClearPET_file_name,*pdfs);
}

int round(double num){
  double fraction;
  double value;
  fraction = modf(num,&value);
  
  if (num>0)
    {
      if (fraction<=0.5)
	return static_cast<int>(floor(num));
      else
	return  static_cast<int>(floor(num+0.5));
    }
  else  
    {
      if (fraction<=-0.5)
	return  static_cast<int>(floor(num));
      else
	return  static_cast<int>(floor(num+0.5));
    }
  
}

END_NAMESPACE_STIR
