//
// $Id: display_projdata.cxx,v 1.3 2001/12/20 21:18:05 kris Exp $
//
/*!
  \file 
  \ingroup utilities
 
  \brief  This programme displys projection data by segment

  \author Kris Thielemans
  \author PARAPET project

  $Date: 2001/12/20 21:18:05 $
  $Revision: 1.3 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2001/12/20 21:18:05 $, IRSL
    See STIR/LICENSE.txt for details
*/
#include "stir/ProjData.h"
#include "stir/Sinogram.h"
#include "stir/SegmentByView.h"
#include "stir/SegmentBySinogram.h"

#include "stir/display.h"
#include "stir/utilities.h"


#ifndef STIR_NO_NAMESPACES
using std::cout;
#endif

USING_NAMESPACE_STIR


int main(int argc, char *argv[])
{
    if(argc<2) {
        cout<<"Usage: display_projdata <header file name> (*.hs)\n";
        exit(EXIT_FAILURE);
    }

  
      
    shared_ptr<ProjData>  s3d = ProjData::read_from_file(argv[1]);

    do {
        int segment_num = ask_num("Which segment number do you want to display", 
                                  s3d->get_min_segment_num(), s3d->get_max_segment_num(), 0);

        if(ask_num("Display as SegmentByView (0) or BySinogram (1)?", 0,1,0)==0) 
	  {
            SegmentByView <float> segment= s3d->get_segment_by_view(segment_num);
	    const float maxi =
	      ask_num("Maximum in color scale (default is actual max)",0.F,2*segment.find_max(),segment.find_max());
            display(segment,maxi);
	  }
        else
	  {
            SegmentBySinogram<float> segment = s3d->get_segment_by_sinogram(segment_num);  
	    const float maxi =
	      ask_num("Maximum in color scale (default is actual max)",0.F,2*segment.find_max(),segment.find_max());
            display(segment,maxi);
	  }
    }
    while (ask("Display another segment ?",true));

    return EXIT_SUCCESS;
}
