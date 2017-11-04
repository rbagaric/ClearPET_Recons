//
// $Id: test_VoxelsOnCartesianGrid.cxx,v 1.4 2001/12/20 21:18:05 kris Exp $
//
/*!

  \file
  \ingroup test
  
  \brief Test programme for VoxelsOnCartesianGrid and image hierarchy
    
   \author Sanida Mustafovic
   \author Kris Thielemans
   \author PARAPET project
      
   $Date: 2001/12/20 21:18:05 $        
   $Revision: 1.4 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2001/12/20 21:18:05 $, IRSL
    See STIR/LICENSE.txt for details
*/

#include "stir/VoxelsOnCartesianGrid.h"
#include "stir/ProjDataInfo.h"
#include "stir/ProjDataInfoCylindricalArcCorr.h"
#include "stir/Scanner.h"
#include "stir/IndexRange.h"

#include <iostream>
#include <math.h>
#include "stir/RunTests.h"

START_NAMESPACE_STIR

/*!
  \ingroup test
  \brief Test class for VoxelsOnCartesianGrid and image hierarchy

*/
class VoxelsOnCartesianGridTests : public RunTests
{
public:
  void run_tests();
};


void
VoxelsOnCartesianGridTests::run_tests()

{ 
  cerr << "Tests for VoxelsOnCartesianGrid and the image hierarchy\n";
  
  CartesianCoordinate3D<float> origin (0,1,2);  
  CartesianCoordinate3D<float> grid_spacing (3,4,5); 
  
  IndexRange<3> 
    range(CartesianCoordinate3D<int>(0,-15,-14),
          CartesianCoordinate3D<int>(4,14,15));
  
  Array<3,float> test1(range);
  
  {
    cerr << "Tests with default constructor\n";
    
    VoxelsOnCartesianGrid<float>  ob1;
    
    // Check set.* & constructor
    
    ob1.set_origin(origin);
    ob1.set_grid_spacing (grid_spacing);
    
    check_if_equal( ob1.get_grid_spacing(), grid_spacing,"test on grid_spacing");
    check_if_equal( ob1.get_origin(), origin, "test on origin");
  }
  
  {
    cerr << "Tests with 2nd constructor (array, origin, grid_spacing)\n";
    
    VoxelsOnCartesianGrid<float>  ob2(test1,origin, grid_spacing);
    test1[1][12][5] = float(5.5);
    test1[4][5][-5] = float(4.5);
    
    check_if_equal( ob2.get_grid_spacing(),grid_spacing, "test on grid_spacing");
    check_if_equal( ob2.get_origin(), origin, "test on origin");
    check_if_equal( test1.sum(), 10.F, "test on arrays");
  }
  {
    
    cerr << "Tests with 3rd constructor(index_range, origin, grid_spacing)\n";
    VoxelsOnCartesianGrid<float>  ob3(range,origin, grid_spacing);
    
    check( ob3.get_index_range() == range, "test on range");
    check_if_equal( ob3.get_grid_spacing(),grid_spacing, "test on grid_spacing");
    check_if_equal( ob3.get_origin(), origin, "test on origin");
    
  }
  
  shared_ptr<Scanner> scanner_ptr = new Scanner(Scanner::E953);
  shared_ptr<ProjDataInfo> proj_data_info_ptr = 
    ProjDataInfo::ProjDataInfoCTI(scanner_ptr, 
				  /*span=*/1, 
				  /*max_delta=*/5,
				  /*num_views=*/8,
				  /*num_tang_poss=*/16);
  
  {
    cerr << "Tests with constructor with ProjDataInfo with default sizes\n";
    
    const float zoom=2.3F;
    //KT 10/12/2001 removed make_xy_size_odd things
    
    VoxelsOnCartesianGrid<float>
      ob4(*proj_data_info_ptr,zoom,origin);
    
    IndexRange<3> obtained_range = ob4.get_index_range();
    CartesianCoordinate3D<int> low_bound, high_bound;
    check(obtained_range.get_regular_range(low_bound, high_bound), "test regular range");
    
    // KT 11/09/2001 adapted as this constructor now takes zoom into account
    const bool is_arccorrected =
      dynamic_cast<ProjDataInfoCylindricalArcCorr const *>(proj_data_info_ptr.get()) != 0;
    check(is_arccorrected, "ProjDataInfoCTI should have returned arc-corrected data");
    if (is_arccorrected)
    {
      const int FOVradius_in_bins = 
	max(proj_data_info_ptr->get_max_tangential_pos_num(),
	    -proj_data_info_ptr->get_min_tangential_pos_num());
      const int diameter_int = 
	2*static_cast<int>(ceil(FOVradius_in_bins * zoom)) + 1;

      check_if_equal(low_bound, CartesianCoordinate3D<int>(0,-(diameter_int/2),-(diameter_int/2)),
		     "test on index range: lower bounds");
      check_if_equal(high_bound, CartesianCoordinate3D<int>(30,+(diameter_int/2),+(diameter_int/2)),
		     "test on index range: higher bounds");
    }
    check_if_equal(ob4.get_grid_spacing(), 
                   CartesianCoordinate3D<float>(scanner_ptr->get_ring_spacing()/2,
                                                scanner_ptr->get_default_bin_size()/zoom,
                                                scanner_ptr->get_default_bin_size()/zoom),
                   "test on grid spacing");
    check_if_equal(ob4.get_origin(), origin);
  }
  {
    
    cerr << "Tests with constructor with ProjDataInfo with non-default sizes\n";
    // KT 10/12/2001 changed to allow for new format of constructor, and add z_size
    const int xy_size = 100;
    const float zoom=3.1F;
    const int min_xy = -(xy_size/2);
    const int max_xy = -(xy_size/2)+xy_size-1;
    const int z_size = 9;

    VoxelsOnCartesianGrid<float>
      ob5(*proj_data_info_ptr,zoom,origin,CartesianCoordinate3D<int>(z_size,xy_size,xy_size));
    IndexRange<3> obtained_range = ob5.get_index_range();
    CartesianCoordinate3D<int> low_bound, high_bound;
    check(obtained_range.get_regular_range(low_bound, high_bound), "test regular range");
    
    check_if_equal(low_bound, CartesianCoordinate3D<int>(0,min_xy,min_xy),"test on index range: lower bounds");
    check_if_equal(high_bound, CartesianCoordinate3D<int>(z_size-1,max_xy,max_xy),"test on index range: higher bounds");
    check_if_equal(ob5.get_grid_spacing(), 
                   CartesianCoordinate3D<float>(scanner_ptr->get_ring_spacing()/2,
                                                scanner_ptr->get_default_bin_size()/zoom,
                                                scanner_ptr->get_default_bin_size()/zoom),
                   "test on grid spacing");
    check_if_equal(ob5.get_origin(), origin);
    
    {
      cerr << "Tests get_empty_voxels_on_cartesian_grid\n";
      
      shared_ptr< VoxelsOnCartesianGrid<float> > emp =ob5.get_empty_voxels_on_cartesian_grid();
      
      IndexRange<3> obtained_range2 = emp->get_index_range();
      check_if_equal( emp->get_origin(), ob5.get_origin(), "test on origin");  
      check_if_equal( emp->get_grid_spacing(), ob5.get_grid_spacing(),"test on grid_spacing");
      check(emp->get_index_range() == ob5.get_index_range(),"test on index range");
      
    }
    
    {
      cerr << "Tests get_empty_discretised_density()\n";
      
      shared_ptr<DiscretisedDensity<3,float> > emp = ob5.get_empty_discretised_density(); 
      
      VoxelsOnCartesianGrid<float>* emp1 =
        dynamic_cast<VoxelsOnCartesianGrid<float>* >(emp.get());
      check(emp1 != 0, "test on pointer conversion from get_empty_discretised_density");
      
      IndexRange<3> obtained_range3 = emp1->get_index_range();
      check_if_equal( emp->get_origin(), ob5.get_origin(), "test on origin");  
      check_if_equal( emp1->get_grid_spacing(), ob5.get_grid_spacing(),"test on grid_spacing");
      check(emp->get_index_range() == ob5.get_index_range(),"test on index range");
    }
  }
}

END_NAMESPACE_STIR


USING_NAMESPACE_STIR


int main()
{
  VoxelsOnCartesianGridTests tests;
  tests.run_tests();
  return tests.main_return_value();
}
