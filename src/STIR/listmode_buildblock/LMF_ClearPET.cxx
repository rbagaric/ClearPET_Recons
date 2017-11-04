//
// $Id: LMF_ClearPet.cxx,Exp $
//
/*!
  \file 
  \ingroup ClearPET_library

  \brief  
 
  \author Monica Vieira Martins
  \author Christian Morel
  
  \date $Date:  $
  \version $Revision:  $
*/

#include "stir/listmode/LMF_ClearPET.h"
#include "stir/ProjDataInfoCylindrical.h"
#include "stir/Array.h"
#include "stir/IndexRange3D.h"


#ifndef STIR_NO_NAMESPACES
using std::min;
using std::flush;
using std::endl;
#endif

START_NAMESPACE_STIR

//In this function we consider the ClearPET axis system. 
void interfile_analytical(double x1, double y1,double  z1,
			  double x2, double y2, double z2,
			  REBINNING_INFOS rebinning_infos,
			  int* tang_pos_x_ptr,
			  int* axial_pos_z_ptr,
			  int* azimuthal_angle_ptr,
			  int* co_polar_angle_ptr,
			  const ProjDataInfoCylindrical& proj_data_info)
{  

  // ClearPET -> STIR coordinates
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

  // Set (x1,y1,z1) with z1=min(z1,z2)
  if (z1 > z2) {
    double x_tmp = x1;
    double y_tmp = y1;
    double z_tmp = z1;

    x1 = x2;
    y1 = y2;
    z1 = z2;
    x2 = x_tmp;
    y2 = y_tmp;
    z2 = z_tmp;
  }

  double s = 0.;
  double phi = 0.;
  double zz = 0.;
  double theta = 0.;
  
  // compute s, phi, theta, and zz
  theta = asin((z2-z1)/sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)+(z2-z1)*(z2-z1)));
  phi = atan2((y2-y1),(x2-x1));

  if (phi <0.) {
    phi += _PI;
    theta *= -1.;
  }
   
  s = y1*cos(phi)-x1*sin(phi);

  {
    // compute intersecttion of LOR with cylindre acceptance
    double zz1 = -100.;
    double zz2 = -100.;
    double radius = rebinning_infos.ring_radius;
    double b = -(x2*cos(phi)+y2*sin(phi));
    double delta = b*b-(x2*x2+y2*y2)+radius*radius;    
    if (delta >= 0) {
      delta = sqrt(delta);
      zz1 = z2 + tan(theta)*(b+delta);
      zz2 = z2 + tan(theta)*(b-delta);
    }
    zz = zz1;
    if (zz1 > zz2) zz = zz2;
  }
   
  double  co_polar_angle = theta;
  double azimuthal_angle = phi;
  double tang_pos_x = s;            
  double axial_pos = zz;
  //double axial_pos = z1;    
  const double dphi =_PI/rebinning_infos.nbphi;
  
  // now do the transformation to index coordinates
  // keep centred range around 0 for co_polar_angle_index
  int co_polar_angle_index = round((co_polar_angle)/rebinning_infos.dtheta);
  
  
  int tang_pos_x_index = round(tang_pos_x/rebinning_infos.bin_size);
  int axial_pos_index = round(axial_pos/rebinning_infos.ring_spacing);
  int azimuthal_angle_index =round(azimuthal_angle/dphi);
  if (co_polar_angle_index < proj_data_info.get_min_segment_num() ||
      co_polar_angle_index > proj_data_info.get_max_segment_num())
    {
      // segment_num is out of range, we just give up	
      *co_polar_angle_ptr = co_polar_angle_index;
      return;
    }
  
  // check consitency in azimuthal_angle_index when equal to max_view_num(): reverse to 0, -co_polar_angle_index and -tang_pos_x_index
  if ((azimuthal_angle_index==rebinning_infos.nbphi)&&((int) (azimuthal_angle/dphi)<azimuthal_angle_index)) 
    {
      azimuthal_angle_index = 0;
      co_polar_angle_index *= -1; 
      tang_pos_x_index *= -1;
    }
  // KT&SM moved assert down    
  assert(azimuthal_angle_index>=0);
  assert(azimuthal_angle_index<rebinning_infos.nbphi);
  
  *tang_pos_x_ptr = tang_pos_x_index;
  *axial_pos_z_ptr = axial_pos_index;
  *azimuthal_angle_ptr = azimuthal_angle_index;   
  *co_polar_angle_ptr = co_polar_angle_index;

}//end of interfile_analytical

bool check_range(const int seg,const int start_segment_index, const  int end_segment_index,
		 const int tang_pos_x, const int axial_pos_z, const int azimuthal_angle, 
		 const  ProjDataFromStream* proj_data_from_stream_ptr)
{
  if (
      seg >= start_segment_index && seg<=end_segment_index
      && tang_pos_x>= proj_data_from_stream_ptr->get_min_tangential_pos_num()
      && tang_pos_x<= proj_data_from_stream_ptr->get_max_tangential_pos_num()
      && axial_pos_z>=proj_data_from_stream_ptr->get_min_axial_pos_num(seg)
      && axial_pos_z<=proj_data_from_stream_ptr->get_max_axial_pos_num(seg)
      && azimuthal_angle>=0
      && azimuthal_angle<=proj_data_from_stream_ptr->get_max_view_num() 
      )
    return 1;
  else return 0;
}


/*
void allocate_segments( VectorWithOffset<segment_type *>& segments,
			const int start_segment_index, 
			const int end_segment_index,
			const ProjDataInfo* proj_data_info_ptr)
{
  
  for (int seg=start_segment_index ; seg<=end_segment_index; seg++)
    {         
      segments[seg] = 
	new Array<3,elem_type>(IndexRange3D(0, proj_data_info_ptr->get_num_views()-1, 
					    0, proj_data_info_ptr->get_num_axial_poss(seg)-1,
					    -(proj_data_info_ptr->get_num_tangential_poss()/2), 
					    proj_data_info_ptr->get_num_tangential_poss()-(proj_data_info_ptr->get_num_tangential_poss()/2)-1));
      
      
      fprintf(stderr,"views %d %d, axial pos %d %d, tangential pos %d %d, segment %d\n",
	      
	      0,proj_data_info_ptr->get_num_views()-1, 
	      0, proj_data_info_ptr->get_num_axial_poss(seg)-1,
	      -(proj_data_info_ptr->get_num_tangential_poss()/2),
	      proj_data_info_ptr->get_num_tangential_poss()-(proj_data_info_ptr->get_num_tangential_poss()/2)-1,seg);
    }
}


void save_and_delete_segments(shared_ptr<iostream>& output,
			      VectorWithOffset<segment_type *>& segments,
			      const int start_segment_index, 
			      const int end_segment_index, 
			      ProjDataFromStream * proj_data_from_stream_ptr)
{    
  for (int seg=start_segment_index; seg<=end_segment_index; seg++)
    {
      //segment_type* segment = segments[seg];  
      (*segments[seg]).write_data(*output);     
      delete segments[seg];      
      //delete segment;
      
    }
}
*/



END_NAMESPACE_STIR
