//
// $Id: zoom.cxx,v 1.9 2002/01/28 15:45:00 kris Exp $
//
/*!
  \file 
  \ingroup buildblock

  \brief Implementations of the zoom functions

  \author Kris Thielemans
  \author Claire Labbe
  \author PARAPET project

  $Date: 2002/01/28 15:45:00 $
  $Revision: 1.9 $

*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2002/01/28 15:45:00 $, IRSL
    See STIR/LICENSE.txt for details
*/
/* Modification history:
   - First versions by CL and KT (sinogram version based on C code by Matthias Egger (using linear interpolation).
   - CL introduced overlap interpolation.
   - KT moved interpolation to separate function overlap_interpolate, removing bugs.
   - KT introduced 3D zooming for images.
   - KT converted to new design
 */
   
#include <cmath>
#include "stir/interpolate.h"
#include "stir/zoom.h"
#include "stir/VoxelsOnCartesianGrid.h" 
#include "stir/PixelsOnCartesianGrid.h" 
#include "stir/Viewgram.h"
#include "stir/RelatedViewgrams.h"
#include "stir/ProjDataInfoCylindricalArcCorr.h"
#include "stir/IndexRange3D.h"
#include "stir/IndexRange2D.h"

START_NAMESPACE_STIR

// TODO all these are terribly wasteful with memory allocations
// main reason: we cannot have segments with viewgrams of different sizes (et al)
// also they need to be converted to the new design
// as we don't need them at the moment, I can't be bothered...
#if 0
void zoom_segment (SegmentByView& segment, 
                   const float zoom, const float azimuthal_angle_sampling, const float y_offset_in_mm, 
                   const int new_size, const float azimuthal_angle_sampling)
{

  // First check if there is anything to do at all, 
  // if not, return original segment.

  if (new_size == segment.get_num_bins() &&
      zoom == 1.0 && x_offset_in_mm == 0.0 && y_offset_in_mm == 0.0) 
    return;

  // KT 17/01/2000 use local copy of scan_info instead of segment.scan_info
  ScanInfo scan_info = segment.scan_info;
  scan_info.set_num_bins(new_size);
  scan_info.set_bin_size(segment.scan_info.get_bin_size() / zoom);
    
  const int minsize = -new_size/2;
  const int maxsize = minsize+new_size-1;
 
  // TODO replace by get_empty_segment or so
  SegmentByView 
    out_segment(Tensor3D<float>(segment.get_min_view(), segment.get_max_view(),
				segment.get_min_axial_pos_num(), segment.get_max_axial_pos_num(), 
				minsize, maxsize),
		scan_info,
		segment.get_segment_num(),
		segment.get_min_axial_pos_num_difference(),
		segment.get_max_ring_difference());

  for (int view = segment.get_min_view(); view <= segment.get_max_view(); view++) 
    {
      Viewgram<float> viewgram = segment.get_viewgram(view);
      zoom_viewgram(viewgram,
		    zoom, x_offset_in_mm, y_offset_in_mm,
		    new_size, azimuthal_angle_sampling);
      out_segment.set_viewgram(viewgram);
    }

  segment = out_segment;
}


#endif

void
zoom_viewgrams (RelatedViewgrams<float>& in_viewgrams, 
	       const float zoom, 
	       const int min_tang_pos_num, const int max_tang_pos_num,
	       const float x_offset_in_mm, const float y_offset_in_mm)
{
  if (min_tang_pos_num == in_viewgrams.get_min_tangential_pos_num() &&
      max_tang_pos_num == in_viewgrams.get_max_tangential_pos_num() &&
      zoom == 1.0 && x_offset_in_mm == 0.0 && y_offset_in_mm == 0.0) 
    return;
    
  ProjDataInfo * new_proj_data_info_ptr =
    in_viewgrams.get_proj_data_info_ptr()->clone();
  ProjDataInfoCylindricalArcCorr* new_proj_data_info_arccorr_ptr =
    dynamic_cast<ProjDataInfoCylindricalArcCorr*>(new_proj_data_info_ptr);

  if ( new_proj_data_info_arccorr_ptr==0)
    error("zoom_viewgram does not support non-arccorrected data. Sorry\n");
  
  new_proj_data_info_arccorr_ptr->set_min_tangential_pos_num(min_tang_pos_num);
  new_proj_data_info_arccorr_ptr->set_max_tangential_pos_num(max_tang_pos_num);
  
  new_proj_data_info_arccorr_ptr->
    set_tangential_sampling(new_proj_data_info_arccorr_ptr->
			      get_tangential_sampling() / zoom);

  RelatedViewgrams<float> 
    out_viewgrams = 
    new_proj_data_info_arccorr_ptr->
      get_empty_related_viewgrams(in_viewgrams.get_basic_view_segment_num(),
				  in_viewgrams.get_symmetries_ptr()->clone());

  {
    RelatedViewgrams<float>::iterator out_iter = out_viewgrams.begin();
    RelatedViewgrams<float>::const_iterator in_iter = in_viewgrams.begin();
    for (; out_iter != out_viewgrams.end(); ++out_iter, ++in_iter)
      zoom_viewgram(*out_iter, *in_iter,
		    x_offset_in_mm, y_offset_in_mm);
  }

  in_viewgrams = out_viewgrams;
}

void
zoom_viewgram (Viewgram<float>& in_view, 
	       const float zoom, 
	       const int min_tang_pos_num, const int max_tang_pos_num,
	       const float x_offset_in_mm, const float y_offset_in_mm)
{   
  if (min_tang_pos_num == in_view.get_min_tangential_pos_num() &&
      max_tang_pos_num == in_view.get_max_tangential_pos_num() &&
      zoom == 1.0 && x_offset_in_mm == 0.0 && y_offset_in_mm == 0.0) 
    return;
    
  ProjDataInfo * new_proj_data_info_ptr =
    in_view.get_proj_data_info_ptr()->clone();
  ProjDataInfoCylindricalArcCorr* new_proj_data_info_arccorr_ptr =
    dynamic_cast<ProjDataInfoCylindricalArcCorr*>(new_proj_data_info_ptr);

  if ( new_proj_data_info_arccorr_ptr==0)
    error("zoom_viewgram does not support non-arccorrected data. Sorry\n");
  
  new_proj_data_info_arccorr_ptr->set_min_tangential_pos_num(min_tang_pos_num);
  new_proj_data_info_arccorr_ptr->set_max_tangential_pos_num(max_tang_pos_num);
  
  new_proj_data_info_arccorr_ptr->
    set_tangential_sampling(new_proj_data_info_arccorr_ptr->
			      get_tangential_sampling() / zoom);

  Viewgram<float> 
    out_view = new_proj_data_info_arccorr_ptr->
                     get_empty_viewgram(
					in_view.get_view_num(),
					in_view.get_segment_num());

  zoom_viewgram(out_view, in_view,    
		x_offset_in_mm, y_offset_in_mm);

  in_view = out_view;
}

void
zoom_viewgram (Viewgram<float>& out_view, 
	       const Viewgram<float>& in_view, 
	       const float x_offset_in_mm, const float y_offset_in_mm)
{   
  // minimal checks on compatibility
  assert(in_view.get_proj_data_info_ptr()->get_num_views() == 
	 out_view.get_proj_data_info_ptr()->get_num_views());
  assert(in_view.get_view_num() == out_view.get_view_num());
  assert(in_view.get_proj_data_info_ptr()->get_num_segments() == 
	 out_view.get_proj_data_info_ptr()->get_num_segments());
  assert(in_view.get_segment_num() == out_view.get_segment_num());
  assert(in_view.get_min_axial_pos_num() == out_view.get_min_axial_pos_num());
  assert(in_view.get_max_axial_pos_num() == out_view.get_max_axial_pos_num());

  // get the pointers to the arc-corrected ProjDataInfo
  const ProjDataInfoCylindricalArcCorr* in_proj_data_info_arccorr_ptr =
    dynamic_cast<const ProjDataInfoCylindricalArcCorr*>(in_view.get_proj_data_info_ptr());
  const ProjDataInfoCylindricalArcCorr* out_proj_data_info_arccorr_ptr =
    dynamic_cast<const ProjDataInfoCylindricalArcCorr*>(out_view.get_proj_data_info_ptr());

  if (in_proj_data_info_arccorr_ptr==0 ||
      out_proj_data_info_arccorr_ptr==0)
    error("zoom_viewgram does not support non-arccorrected data. Sorry\n");

  
  const float in_bin_size = 
    in_proj_data_info_arccorr_ptr->get_tangential_sampling();
  const float out_bin_size = 
    out_proj_data_info_arccorr_ptr->get_tangential_sampling();

  const float zoom = in_bin_size / out_bin_size;

  if (out_view.get_min_tangential_pos_num() == in_view.get_min_tangential_pos_num() &&
      out_view.get_max_tangential_pos_num() == in_view.get_max_tangential_pos_num() &&
      zoom == 1.0F && x_offset_in_mm == 0.0F && y_offset_in_mm == 0.0F) 
    return;
    
  const float phi =
     in_proj_data_info_arccorr_ptr->
      get_phi(Bin(in_view.get_segment_num(), in_view.get_view_num(), 0,0));

  // compute offset in tangential_sampling_in units
  const float offset = 
    (x_offset_in_mm*cos(phi) +y_offset_in_mm*sin(phi))/ in_bin_size;

  for (int axial_pos_num= out_view.get_min_axial_pos_num(); axial_pos_num <= out_view.get_max_axial_pos_num(); axial_pos_num++)
    {
      overlap_interpolate(out_view[axial_pos_num], in_view[axial_pos_num], zoom, offset, false);
    }    
}



void
zoom_image(VoxelsOnCartesianGrid<float> &image,
           const float zoom,
	   const float x_offset_in_mm, const float y_offset_in_mm, 
	   const int new_size )                          
{
  assert(new_size>=0);
  if(zoom==1 && x_offset_in_mm==0 && y_offset_in_mm==0 && new_size== image.get_x_size()) 
    return;
   
  CartesianCoordinate3D<float>
    voxel_size(image.get_voxel_size().z()/zoom,
	       image.get_voxel_size().y()/zoom,
	       image.get_voxel_size().x());
  CartesianCoordinate3D<float>
    origin(image.get_origin().z(),
	   image.get_origin().y() + y_offset_in_mm,
	   image.get_origin().x() + x_offset_in_mm);

  VoxelsOnCartesianGrid<float> 
    new_image(IndexRange3D(image.get_min_z(), image.get_max_z(),
			   -new_size/2, -new_size/2+new_size-1,
			   -new_size/2, -new_size/2+new_size-1),
	      origin,
	      voxel_size);

  PixelsOnCartesianGrid<float> 
    new_image2D = new_image.get_plane(new_image.get_min_z());
  for (int plane = image.get_min_z(); plane <= image.get_max_z(); plane++)
    {
      zoom_image(new_image2D, image.get_plane(plane));
      new_image.set_plane(new_image2D, plane);
    }
  image=new_image;
    
  assert(image.get_voxel_size() == voxel_size);
}


void
zoom_image(VoxelsOnCartesianGrid<float> &image,
	   const CartesianCoordinate3D<float>& zooms,
	   const CartesianCoordinate3D<float>& offsets_in_mm,
	   const CartesianCoordinate3D<int>& new_sizes)
{

  assert(new_sizes.x()>=0);
  assert(new_sizes.y()>=0);
  assert(new_sizes.z()>=0);
  CartesianCoordinate3D<float>
    voxel_size = image.get_grid_spacing() / zooms;
  
  CartesianCoordinate3D<float>
    origin = image.get_origin() + offsets_in_mm;
  
  VoxelsOnCartesianGrid<float> 
    new_image(IndexRange3D(-new_sizes.z()/2, -new_sizes.z()/2+new_sizes.z()-1,
			   -new_sizes.y()/2, -new_sizes.y()/2+new_sizes.y()-1,
			   -new_sizes.x()/2, -new_sizes.x()/2+new_sizes.x()-1),
	      origin,
	      voxel_size);
  zoom_image(new_image, image);

  image = new_image;
}

void 
zoom_image(VoxelsOnCartesianGrid<float> &image_out, 
	   const VoxelsOnCartesianGrid<float> &image_in)
{

/*
     interpolation routine uses the following relation:
         x_in_index = x_out_index/zoom  + offset

     compare to 'physical' coordinates
         x_phys = (x_index - x_ctr_index) * voxel_size.x + origin.x
       
     as x_in_phys == x_out_phys, we find
         (x_in_index - x_in_ctr_index)* voxel_size_in.x + origin_in.x ==
           (x_out_index - x_out_ctr_index)* voxel_size_out.x + origin_out.x
        <=>
         x_in_index = x_in_ctr_index +
                      ((x_out_index- x_out_ctr_index) * voxel_size_out.x 
	                + origin_out.x - origin_in.x) 
		      / voxel_size_in.x
  */
  const float zoom_x = 
    image_in.get_voxel_size().x() / image_out.get_voxel_size().x();
  const float zoom_y = 
    image_in.get_voxel_size().y() / image_out.get_voxel_size().y();
  const float zoom_z = 
    image_in.get_voxel_size().z() / image_out.get_voxel_size().z();
  const float x_offset = 
    ((image_out.get_origin().x() - image_in.get_origin().x()) 
     / image_in.get_voxel_size().x()) + 
    (image_in.get_max_x() + image_in.get_min_x())/2.F -
    (image_out.get_max_x() + image_out.get_min_x())/2.F / zoom_x;
  const float y_offset = 
    ((image_out.get_origin().y() - image_in.get_origin().y()) 
     / image_in.get_voxel_size().y()
    ) + 
    (image_in.get_max_y() + image_in.get_min_y())/2.F -
    (image_out.get_max_y() + image_out.get_min_y())/2.F / zoom_y;
  const float z_offset = 
    ((image_out.get_origin().z() - image_in.get_origin().z()) 
     / image_in.get_voxel_size().z()
    ) + 
    (image_in.get_max_z() + image_in.get_min_z())/2.F -
    (image_out.get_max_z() + image_out.get_min_z())/2.F / zoom_z;

  
  if(zoom_x==1.0F && zoom_y==1.0F && zoom_z==1.0F &&
     x_offset == 0.F && y_offset == 0.F && z_offset == 0.F &&
     image_in.get_index_range() == image_out.get_index_range()
     )
    {
      image_out = image_in;
      return;
    }

  // TODO creating a lot of new images here...
  Array<3,float> 
    temp(IndexRange3D(image_in.get_min_z(), image_in.get_max_z(),
		      image_in.get_min_y(), image_in.get_max_y(),
		      image_out.get_min_x(), image_out.get_max_x()));

  for (int z=image_in.get_min_z(); z<=image_in.get_max_z(); z++)
    for (int y=image_in.get_min_y(); y<=image_in.get_max_y(); y++)
      overlap_interpolate(temp[z][y], image_in[z][y], zoom_x, x_offset);

  Array<3,float> temp2(IndexRange3D(image_in.get_min_z(), image_in.get_max_z(),
				    image_out.get_min_y(), image_out.get_max_y(),
				    image_out.get_min_x(), image_out.get_max_x()));

  for (int z=image_in.get_min_z(); z<=image_in.get_max_z(); z++)
    overlap_interpolate(temp2[z], temp[z], zoom_y, y_offset);

  overlap_interpolate(image_out, temp2, zoom_z, z_offset);

}

void
zoom_image(PixelsOnCartesianGrid<float> &image2D_out, 
           const PixelsOnCartesianGrid<float> &image2D_in)
{
  /*
    see above for how to find zoom and offsets
  */

  const float zoom_x = 
    image2D_in.get_pixel_size().x() / image2D_out.get_pixel_size().x();
  const float zoom_y = 
    image2D_in.get_pixel_size().y() / image2D_out.get_pixel_size().y();
  const float x_offset = 
    ((image2D_out.get_origin().x() - image2D_in.get_origin().x()) 
     / image2D_in.get_pixel_size().x()
    ) + 
    (image2D_in.get_max_x() + image2D_in.get_min_x())/2.F -
    (image2D_out.get_max_x() + image2D_out.get_min_x())/2.F / zoom_x;
  const float y_offset = 
    ((image2D_out.get_origin().y() - image2D_in.get_origin().y()) 
     / image2D_in.get_pixel_size().y()
    ) + 
    (image2D_in.get_max_y() + image2D_in.get_min_y())/2.F -
    (image2D_out.get_max_y() + image2D_out.get_min_y())/2.F / zoom_y;
  
  if(zoom_x==1.0F && zoom_y==1.0F &&
     x_offset == 0.F && y_offset == 0.F &&
     image2D_in.get_index_range() == image2D_out.get_index_range()
     )
  {
    image2D_out = image2D_in;
    return;
  }

  Array<2,float> 
    temp(IndexRange2D(image2D_in.get_min_y(), image2D_in.get_max_y(),
		      image2D_out.get_min_x(), image2D_out.get_max_x()));

  for (int y=image2D_in.get_min_y(); y<=image2D_in.get_max_y(); y++)
    overlap_interpolate(temp[y], image2D_in[y], zoom_x, x_offset);

  overlap_interpolate(image2D_out, temp, zoom_y, y_offset);   
}

END_NAMESPACE_STIR
