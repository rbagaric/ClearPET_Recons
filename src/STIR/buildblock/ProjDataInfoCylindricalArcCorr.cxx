//
// $Id: ProjDataInfoCylindricalArcCorr.cxx,v 1.8 2004/12/14 15:19:34 kris Exp $
//
/*!

  \file
  \ingroup projdata

  \brief Implementation of non-inline functions of class 
  ProjDataInfoCylindricalArcCorr

  \author Sanida Mustafovic
  \author Kris Thielemans
  \author PARAPET project

  $Date: 2004/12/14 15:19:34 $

  $Revision: 1.8 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2004/12/14 15:19:34 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/

#include "stir/ProjDataInfoCylindricalArcCorr.h"
#include "stir/Bin.h"
#include "stir/round.h"
#include "stir/LORCoordinates.h"
#ifdef BOOST_NO_STRINGSTREAM
#include <strstream.h>
#else
#include <sstream>
#endif

#ifndef STIR_NO_NAMESPACES
using std::endl;
using std::ends;
#endif


START_NAMESPACE_STIR
ProjDataInfoCylindricalArcCorr:: ProjDataInfoCylindricalArcCorr()
{}

ProjDataInfoCylindricalArcCorr:: ProjDataInfoCylindricalArcCorr(const shared_ptr<Scanner> scanner_ptr,float bin_size_v,								
								const  VectorWithOffset<int>& num_axial_pos_per_segment,
								const  VectorWithOffset<int>& min_ring_diff_v, 
								const  VectorWithOffset<int>& max_ring_diff_v,
								const int num_views,const int num_tangential_poss)
								:ProjDataInfoCylindrical(scanner_ptr,
								num_axial_pos_per_segment,
								min_ring_diff_v, max_ring_diff_v,
								num_views, num_tangential_poss),
								bin_size(bin_size_v)								
								
{}


void
ProjDataInfoCylindricalArcCorr::set_tangential_sampling(const float new_tangential_sampling)
{bin_size = new_tangential_sampling;}



ProjDataInfo*
ProjDataInfoCylindricalArcCorr::clone() const
{
  return static_cast<ProjDataInfo*>(new ProjDataInfoCylindricalArcCorr(*this));
}

string
ProjDataInfoCylindricalArcCorr::parameter_info()  const
{

#ifdef BOOST_NO_STRINGSTREAM
  // dangerous for out-of-range, but 'old-style' ostrstream seems to need this
  char str[50000];
  ostrstream s(str, 50000);
#else
  std::ostringstream s;
#endif  
  s << "ProjDataInfoCylindricalArcCorr := \n";
  s << ProjDataInfoCylindrical::parameter_info();
  s << "tangential sampling := " << get_tangential_sampling() << endl;
  s << "End :=\n";
  return s.str();
}


Bin
ProjDataInfoCylindricalArcCorr::
get_bin(const LOR<float>& lor) const

{
#if 1

  Bin bin;
  LORAs2Points<float> lor_coords;
  if (lor.get_intersections_with_cylinder(lor_coords, get_ring_radius()) == Succeeded::no)
    {
      bin.set_bin_value(-1);
      return bin;
      }

  bin.segment_num() = 0;
  const double dtheta =  atan(get_axial_sampling(bin.segment_num())/(2*get_ring_radius()));

  CartesianCoordinate3D<float> c1 = lor_coords.p1();

//printf("%f	%f	%f	",c1.x(),c1.y(),c1.z());

  CartesianCoordinate3D<float> c2 = lor_coords.p2();

//printf("%f	%f	%f\n",c2.x(),c2.y(),c2.z());

  CartesianCoordinate3D<float> d = c2 - c1;

//printf("Diff position x: %f	position y: %f	position z: %f\n",d.x(),d.y(),d.z());

  // Set c1 with c1.z()=min(c1.z(),c2.z())
  if (d.z() < 0)
    {
      const CartesianCoordinate3D<float> tmp_coords = c1;
      c1 = c2;
      c2 = tmp_coords;
      d *= -1;
      /*d.x() *= -1;
      d.y() *= -1;
      d.z() *= -1;*/
    }

  double s = 0.;
  double phi = 0.;
  double theta = 0.;
  double delta = 0;

  // compute s, phi, and theta or delta
  delta = -d.z()/get_ring_spacing();
  theta = -asin(d.z()/sqrt(square(d.x())+square(d.y())+square(d.z())));
  phi = atan2(d.y(),d.x());

  if (phi <0.) {
    phi += _PI;
    theta *= -1.;
    delta *= -1.;
  }

  s = c1.y()*cos(phi) - c1.x()*sin(phi);

//printf("Phi = %f,    s = %f,    theta = %f,    delta = %f\n\n\n",phi,s,theta,delta);
//printf("get_ring_spacing %f\n",get_ring_spacing());

  // Use theta would correspond to a spherical arc correction rather than a cylindrical arc correction
  //bin.segment_num() = round(theta / dtheta);

  bin.segment_num() = round(delta);

//printf("segment %d\n",bin.segment_num());
 
 if ((bin.segment_num()>get_max_segment_num()) ||
      (bin.segment_num()<get_min_segment_num()))
    {
      bin.set_bin_value(-1);
      return bin;
    }
  assert(get_max_ring_difference(bin.segment_num())==-get_min_ring_difference(-bin.segment_num()));

  bin.view_num() = round(phi / get_azimuthal_angle_sampling());

//printf("view %d\n",bin.view_num());

  assert(bin.view_num()>=0);
  assert(bin.view_num()<=get_num_views());

  bin.tangential_pos_num() = round(s / get_tangential_sampling());
//printf("tangential %d\n",bin.tangential_pos_num());

  if (bin.tangential_pos_num() < get_min_tangential_pos_num() ||
      bin.tangential_pos_num() > get_max_tangential_pos_num())
    {
      bin.set_bin_value(-1);
      return bin;
    }

  bin.axial_pos_num() = round(c1.z()/get_axial_sampling(bin.segment_num()));
//printf("axial %d\n\n\n",bin.axial_pos_num());

  if (bin.axial_pos_num() < get_min_axial_pos_num(bin.segment_num()) ||
      bin.axial_pos_num() > get_max_axial_pos_num(bin.segment_num()))
    {
      bin.set_bin_value(-1);
      return bin;
    }

  // check consitency in view_num when equal to max_view_num(): reverse to 0, -segment_num and -tangential_pos_num
  if ((bin.view_num()==get_num_views())&&((int) (phi/get_tangential_sampling())<bin.view_num()))
    {
      bin.view_num() = 0;
      bin.segment_num() *= -1;
      bin.tangential_pos_num() *= -1;
    }

  bin.set_bin_value(1);
  return bin;

#else

  Bin bin;
  LORInAxialAndSinogramCoordinates<float> lor_coords;
  if (lor.change_representation(lor_coords, get_ring_radius()) == Succeeded::no)
    {
      bin.set_bin_value(-1);
      return bin;
    }

  // first find view 
  // unfortunately, phi ranges from [0,Pi[, but the rounding can
  // map this to a view which corresponds to Pi anyway.
  bin.view_num() = round(lor_coords.phi() / get_azimuthal_angle_sampling());
  assert(bin.view_num()>=0);
  assert(bin.view_num()<=get_num_views());
  const bool swap_direction =
    bin.view_num() > get_max_view_num();
  if (swap_direction)
    bin.view_num()-=get_num_views();

  bin.tangential_pos_num() = round(lor_coords.s() / get_tangential_sampling());
  if (swap_direction)
    bin.tangential_pos_num() *= -1;

  if (bin.tangential_pos_num() < get_min_tangential_pos_num() ||
      bin.tangential_pos_num() > get_max_tangential_pos_num())
    {
      bin.set_bin_value(-1);
      return bin;
    }

#if 0
  const int num_rings = 
    get_scanner_ptr()->get_num_rings();
  // TODO WARNING LOR coordinates are w.r.t. centre of scanner, but the rings are numbered with the first ring at 0
  int ring1, ring2;
  if (!swap_direction)
    {
      //ring1 = round(lor_coords.z1()/get_ring_spacing() + (num_rings-1)/2.F);
      //ring2 = round(lor_coords.z2()/get_ring_spacing() + (num_rings-1)/2.F);
      ring1 = round(lor_coords.z1()/get_ring_spacing());
      ring2 = round(lor_coords.z2()/get_ring_spacing());
    }
  else
    {
      //ring2 = round(lor_coords.z1()/get_ring_spacing() + (num_rings-1)/2.F);
      //ring1 = round(lor_coords.z2()/get_ring_spacing() + (num_rings-1)/2.F);
      ring2 = round(lor_coords.z1()/get_ring_spacing());
      ring1 = round(lor_coords.z2()/get_ring_spacing());
    }

  if (!(ring1 >=0 && ring1<get_scanner_ptr()->get_num_rings() &&
	ring2 >=0 && ring2<get_scanner_ptr()->get_num_rings() &&
	get_segment_axial_pos_num_for_ring_pair(bin.segment_num(),
						bin.axial_pos_num(),
						ring1,
						ring2) == Succeeded::yes)
      )
    {
      bin.set_bin_value(-1);
      return bin;
    }
#else
  // find nearest segment
  {
    const float delta =
      (swap_direction 
       ? lor_coords.z1()-lor_coords.z2()
       : lor_coords.z2()-lor_coords.z1()
       )/get_ring_spacing();
    // check if out of acquired range
    // note the +1 or -1, which takes the size of the rings into account
    if (delta>get_max_ring_difference(get_max_segment_num())+1 ||
	delta<get_min_ring_difference(get_min_segment_num())-1)
      {
	bin.set_bin_value(-1);
	return bin;
      } 
    if (delta>=0)
      {
	for (bin.segment_num()=0; bin.segment_num()<get_max_segment_num(); ++bin.segment_num())
	  {
	    if (delta < get_max_ring_difference(bin.segment_num())+.5)
	      break;
	  }
      }
    else
      {
	// delta<0
	for (bin.segment_num()=0; bin.segment_num()>get_min_segment_num(); --bin.segment_num())
	  {
	    if (delta > get_min_ring_difference(bin.segment_num())-.5)
	      break;
	  }
      }
  }
  // now find nearest axial position
  {
    const float m = (lor_coords.z2()+lor_coords.z1())/2;
#if 0
    // this uses private member of ProjDataInfoCylindrical
    // enable when moved
    if (!ring_diff_arrays_computed)
      initialise_ring_diff_arrays();
#ifndef NDEBUG
    bin.axial_pos_num()=0;
    assert(get_m(bin)==- m_offset[bin.segment_num()]);
#endif
    bin.axial_pos_num() =
      round((m + m_offset[bin.segment_num()])/
	    get_axial_sampling(bin.segment_num()));
#else
    bin.axial_pos_num()=0;
    //bin.axial_pos_num() = round((m - get_m(bin))/get_axial_sampling(bin.segment_num()));
    //lmf frame centred on ring 0
    bin.axial_pos_num()=round((m)/get_axial_sampling(bin.segment_num()));
#endif
    if (bin.axial_pos_num() < get_min_axial_pos_num(bin.segment_num()) ||
	bin.axial_pos_num() > get_max_axial_pos_num(bin.segment_num()))
      {
	bin.set_bin_value(-1);
	return bin;
      }
  }
#endif

  bin.set_bin_value(1);
  return bin;
#endif
}
END_NAMESPACE_STIR

