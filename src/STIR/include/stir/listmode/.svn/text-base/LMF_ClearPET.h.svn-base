//
// $Id: LMF_ClearPet.h,                
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

#define USE_SegmentByView

//#define FRAME_BASED_DT_CORR

// set elem_type to what you want to use for the sinogram elements
// we need a signed type, as randoms can be subtracted. However, signed char could do.

#if defined(USE_SegmentByView) 
   typedef float elem_type;
#  define OUTPUTNumericType NumericType::FLOAT
#else
   #error currently problem with normalisation code!
   typedef short elem_type;
#  define OUTPUTNumericType NumericType::SHORT
#endif


#include "stir/CartesianCoordinate3D.h"
#include "stir/listmode/LMF_Interfile.h"
#include "lmf.h"

#include "stir/ProjDataInfoCylindrical.h"
#include "stir/Array.h"
#include "stir/IndexRange3D.h"
#include "stir/ProjDataFromStream.h"
#include "stir/IO/interfile.h"
#include "stir/ProjDataInfo.h"
#include "stir/listmode/CListModeData.h"
#include "stir/ParsingObject.h"
#include "stir/TimeFrameDefinitions.h"
#include "stir/IO/write_data.h"

#include "stir/recon_buildblock/BinNormalisation.h"


START_NAMESPACE_STIR

//typedef float elem_type;
//typedef Array<3,elem_type> segment_type;
//typedef SegmentByView<elem_type> segment_type;


typedef struct rebinning_infos
{

  float ring_radius;// ring_radius;
  float bin_size;// bin_size 
  float ring_spacing;// ring spacing
  int nbz;// number of samples in X & Z direction
  int nbphi;// number of views
  float dtheta;// sampling distance axialy
  int  nbtheta;// number of segments
  int max_num_of_bins;// number of bins

}REBINNING_INFOS;

void interfile_analytical(double x1, double y1,double  z1,
			  double x2, double y2, double z2,
			  REBINNING_INFOS rebinning_infos,
			  int* tang_pos_x_ptr,
			  int* axial_pos_z_ptr,
			  int* azimuthal_angle_ptr,
			  int* co_polar_angle_ptr,
			  const ProjDataInfoCylindrical& proj_data_info);

//checks if the sinogram indexes are inside the defined range 
bool check_range(const int seg,const int start_segment_index, const  int end_segment_index,
		 const int tang_pos_x, const int axial_pos_z, const int azimuthal_angle, 
		 const  ProjDataFromStream* proj_data_from_stream_ptr);


/*void allocate_segments(VectorWithOffset<segment_type *>& segments,
                       const int start_segment_index, 
	               const int end_segment_index,
                       const ProjDataInfo* proj_data_info_ptr);
      
void save_and_delete_segments(shared_ptr<iostream>& output,
			      VectorWithOffset<segment_type *>& segments,
			      const int start_segment_index, 
			      const int end_segment_index, 
			      ProjDataFromStream* proj_data_from_stream_ptr);
*/

END_NAMESPACE_STIR
