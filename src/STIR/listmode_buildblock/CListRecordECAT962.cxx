//
// $Id: CListRecordECAT962.cxx,v 1.4 2005/04/20 16:50:27 kris Exp $
//
/*
    Copyright (C) 1998- $Date: 2005/04/20 16:50:27 $, Hammersmith Imanet Ltd
    This file is part of STIR.

    This file is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This file is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    See STIR/LICENSE.txt for details
*/
/*!
  \file
  \ingroup listmode
  \brief Implementation of classes CListEventECAT962 and CListRecordECAT962
  for listmode events for the  ECAT 962 (aka Exact HR+).
    
  \author Kris Thielemans
      
  $Date: 2005/04/20 16:50:27 $
  $Revision: 1.4 $
*/

#include "stir/listmode/CListRecordECAT962.h"
#include "stir/ProjDataInfoCylindricalNoArcCorr.h"
#include "stir/Bin.h"
#include "stir/Succeeded.h"

START_NAMESPACE_STIR

// static members

shared_ptr<Scanner>
CListRecordECAT962::
scanner_sptr =
  new Scanner(Scanner::E962);

shared_ptr<ProjDataInfoCylindricalNoArcCorr>
CListRecordECAT962::
uncompressed_proj_data_info_sptr =
   dynamic_cast<ProjDataInfoCylindricalNoArcCorr *>(
   ProjDataInfo::ProjDataInfoCTI(scanner_sptr,
                                 1, scanner_sptr->get_num_rings()-1,
                                 scanner_sptr->get_num_detectors_per_ring()/2,
                                 scanner_sptr->get_default_num_arccorrected_bins
(),
                                 false));

/*	Global Definitions */
static const int  MAXPROJBIN = 512;
/* data for the 962 scanner */
static const int CRYSTALRINGSPERDETECTOR = 8;
//TODO NK check
void
CListEventDataECAT962::
get_sinogram_and_ring_coordinates(
		   int& view_num, int& tangential_pos_num, int& ring_a, int& ring_b) const
{
  const int NumProjBins = MAXPROJBIN;
  const int NumProjBinsBy2 = MAXPROJBIN / 2;

  view_num = view;
  tangential_pos_num = bin;
  /* KT 31/05/98 use >= in comparison now */
  if ( tangential_pos_num >= NumProjBinsBy2 )
      tangential_pos_num -= NumProjBins ;

  ring_a = ( (block_A_ring_bit0 + 2*block_A_ring_bit1) 
	     * CRYSTALRINGSPERDETECTOR ) +  block_A_detector ;
  ring_b = ( (block_B_ring_bit0 + 2*block_B_ring_bit1)
	     * CRYSTALRINGSPERDETECTOR ) +  block_B_detector ;
}

void 
CListEventDataECAT962::
set_sinogram_and_ring_coordinates(
			const int view_num, const int tangential_pos_num, 
			const int ring_a, const int ring_b)
{
  const int NumProjBins = MAXPROJBIN;
  type = 0;
  const unsigned int block_A_ring     = ring_a / CRYSTALRINGSPERDETECTOR;
  block_A_detector = ring_a % CRYSTALRINGSPERDETECTOR;
  const unsigned int block_B_ring     = ring_b / CRYSTALRINGSPERDETECTOR;
  block_B_detector = ring_b % CRYSTALRINGSPERDETECTOR;

  assert(block_A_ring<4);
  block_A_ring_bit0 = block_A_ring | 0x1;
  block_A_ring_bit1 = block_A_ring/2;
  assert(block_B_ring<4);
  block_B_ring_bit0 = block_B_ring | 0x1;
  block_B_ring_bit1 = block_B_ring/2;
  
  bin = tangential_pos_num < 0 ? tangential_pos_num + NumProjBins : tangential_pos_num;
  view = view_num;
}


void 
CListEventDataECAT962::
get_detectors(
		   int& det_num_a, int& det_num_b, int& ring_a, int& ring_b) const
{
  int tangential_pos_num;
  int view_num;
  get_sinogram_and_ring_coordinates(view_num, tangential_pos_num, ring_a, ring_b);

  CListRecordECAT962::
    get_uncompressed_proj_data_info_sptr()->
    get_det_num_pair_for_view_tangential_pos_num(det_num_a, det_num_b, 
						 view_num, tangential_pos_num);
}

void 
CListEventDataECAT962::
set_detectors(
			const int det_num_a, const int det_num_b,
			const int ring_a, const int ring_b)
{
  int tangential_pos_num;
  int view_num;
  const bool swap_rings =
  CListRecordECAT962::
    get_uncompressed_proj_data_info_sptr()->
    get_view_tangential_pos_num_for_det_num_pair(view_num, tangential_pos_num,
						 det_num_a, det_num_b);

  if (swap_rings)
  {
    set_sinogram_and_ring_coordinates(view_num, tangential_pos_num, ring_a, ring_b);
  }
  else
  {
     set_sinogram_and_ring_coordinates(view_num, tangential_pos_num, ring_b, ring_a);
  }
}

// TODO maybe move to ProjDataInfoCylindricalNoArcCorr
static void
sinogram_coordinates_to_bin(Bin& bin, const int view_num, const int tang_pos_num, 
			const int ring_a, const int ring_b,
			const ProjDataInfoCylindrical& proj_data_info)
{
  if (proj_data_info.get_segment_axial_pos_num_for_ring_pair(bin.segment_num(), bin.axial_pos_num(), ring_a, ring_b) ==
      Succeeded::no)
    {
      bin.set_bin_value(-1);
      return;
    }
  bin.set_bin_value(1);
  bin.view_num() = view_num / proj_data_info.get_view_mashing_factor();  
  bin.tangential_pos_num() = tang_pos_num;
}

void 
CListRecordECAT962::
get_bin(Bin& bin, const ProjDataInfo& proj_data_info) const
{
  assert (dynamic_cast<const ProjDataInfoCylindricalNoArcCorr *>(&proj_data_info)!=0);

  int tangential_pos_num;
  int view_num;
  int ring_a;
  int ring_b;
  event_data.get_sinogram_and_ring_coordinates(view_num, tangential_pos_num, ring_a, ring_b);
  sinogram_coordinates_to_bin(bin, view_num, tangential_pos_num, ring_a, ring_b, 
			      static_cast<const ProjDataInfoCylindrical&>(proj_data_info));
}


void
CListRecordECAT962::
get_detection_coordinates(CartesianCoordinate3D<float>& coord_1,
			  CartesianCoordinate3D<float>& coord_2) const
{
  int det_num_a, det_num_b, ring_a, ring_b;
  event_data.get_detectors(det_num_a, det_num_b, ring_a, ring_b);

  uncompressed_proj_data_info_sptr->
    find_cartesian_coordinates_given_scanner_coordinates(coord_1, coord_2,
							 ring_a, ring_b,
							 det_num_a, det_num_b);
}

void 
CListRecordECAT962::
get_uncompressed_bin(Bin& bin) const
{
  int ring_a;
  int ring_b;
  event_data.get_sinogram_and_ring_coordinates(bin.view_num(), bin.tangential_pos_num(), ring_a, ring_b);
  uncompressed_proj_data_info_sptr->
    get_segment_axial_pos_num_for_ring_pair(bin.segment_num(), bin.axial_pos_num(), 
					    ring_a, ring_b);
}  




END_NAMESPACE_STIR
