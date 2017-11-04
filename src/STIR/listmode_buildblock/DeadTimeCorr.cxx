
// (Note: can currently NOT be disabled)
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


#include "stir/utilities.h"

#include "stir/listmode/DeadTimeCorr.h"
#include "stir/listmode/CListRecord.h"
#include "stir/listmode/CListModeData.h"
#include "stir/ProjDataInfoCylindricalNoArcCorr.h"

#include "stir/Scanner.h"
#ifdef USE_SegmentByView
#include "stir/ProjDataInterfile.h"
#include "stir/SegmentByView.h"
#else
#include "stir/ProjDataFromStream.h"
#include "stir/IO/interfile.h"
#include "stir/Array.h"
#include "stir/IndexRange3D.h"
#endif
#include "stir/ParsingObject.h"
#include "stir/TimeFrameDefinitions.h"
#include "stir/CPUTimer.h"
#include "stir/recon_buildblock/TrivialBinNormalisation.h"
#include "stir/is_null_ptr.h"

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
#endif

START_NAMESPACE_STIR

#ifdef USE_SegmentByView
typedef SegmentByView<elem_type> segment_type;
#else
#error does not work at the moment
#endif


static void
allocate_segments(VectorWithOffset<segment_type *>& segments,
                       const int start_segment_index,
	               const int end_segment_index,
                       const ProjDataInfo* proj_data_info_ptr);

static void
save_and_delete_segments(shared_ptr<iostream>& output,
			      VectorWithOffset<segment_type *>& segments,
			      const int start_segment_index,
			      const int end_segment_index,
			      ProjData& proj_data);
static
shared_ptr<ProjData>
construct_proj_data(shared_ptr<iostream>& output,
                    const string& output_filename,
                    const shared_ptr<ProjDataInfo>& proj_data_info_ptr);

void
DeadTimeCorr::
set_defaults()
{
	ITrate = 0;
	P2 = 0;
	P3 = 0;
	P4 = 0;
	P5 = 0;
	P6 = 0;
	P7 = 0;
	P8 = 0;
}

void
DeadTimeCorr::
initialise_keymap()
{
  parser.add_start_key("deadtimecorr Parameters");
  parser.add_key("input file",&sinoE_filename);
  parser.add_key("DAQ duration in seconds",&time_duration);
  //parser.add_key("raw file",&raw_filename);
  parser.add_key("output filename prefix",&output_filename_prefix);
  parser.add_key("intrinsic true rate", &ITrate);
  parser.add_key("P2", &P2);
  parser.add_key("P3", &P3);
  parser.add_key("P4", &P4);
  parser.add_key("P5", &P5);
  parser.add_key("P6", &P6);
  parser.add_key("P7", &P7);
  parser.add_key("P8", &P8);
  parser.add_stop_key("END");
}

bool
DeadTimeCorr::
post_processing()
{
  sinoE_data_ptr = ProjData::read_from_file(sinoE_filename);
  sinoE_data_info_ptr = sinoE_data_ptr->get_proj_data_info_ptr()->clone();

  /*raw_data_ptr = ProjData::read_from_file(raw_filename);
  raw_data_info_ptr = raw_data_ptr->get_proj_data_info_ptr()->clone();*/

  num_segments = sinoE_data_ptr->get_num_segments();
  printf("Seg %d\n",num_segments);

  vector<pair<double, double> > frame_times(1, pair<double,double>(0,1));
  frame_defs = TimeFrameDefinitions(frame_times);

#ifdef FRAME_BASED_DT_CORR
  cerr << "DeadTimeCorr Using FRAME_BASED_DT_CORR\n";
#else
  cerr << "DeadTimeCorr NOT Using FRAME_BASED_DT_CORR\n";
#endif
  return false;
}

DeadTimeCorr::
DeadTimeCorr()
{
  set_defaults();
}

DeadTimeCorr::
DeadTimeCorr(const char * const par_filename)
{
  set_defaults();
  if (par_filename!=0)
    {
        if (parse(par_filename)==false)
	error("Exiting\n");
    }
  else
    ask_parameters();
}


void
DeadTimeCorr::
process_new_time_event(const CListTime&)
{}

void
DeadTimeCorr::
start_new_time_frame(const unsigned int)
{}


void
DeadTimeCorr::
process_data()
{

  double sum_seg = 0.0;
  double TplusS = 0.0 ;
  double TplusS_ext = 0.0 ;
  double DT_CF = 0.0 ;

  Bin bin_out;
  CPUTimer timer;
  timer.start();

  VectorWithOffset<segment_type *> segments_out (sinoE_data_info_ptr->get_min_segment_num(), sinoE_data_info_ptr->get_max_segment_num());

  //*********** open output file
  shared_ptr<iostream> output;
  shared_ptr<ProjData> proj_data_out_ptr;

  current_frame_num = 1;
  const string output_filename = output_filename_prefix;

  proj_data_out_ptr = construct_proj_data(output, output_filename, sinoE_data_info_ptr);

  allocate_segments(segments_out, sinoE_data_ptr->get_min_segment_num(), sinoE_data_ptr->get_max_segment_num(), proj_data_out_ptr->get_proj_data_info_ptr());

  for (int segment_index = sinoE_data_ptr->get_min_segment_num(); segment_index <= sinoE_data_ptr->get_max_segment_num(); segment_index++)
  {
    printf("Segments %d\n",segment_index);
    cerr << "Processing next batch of segments" <<endl;

    SegmentByView<elem_type> segSinoE = sinoE_data_ptr->get_segment_by_view(segment_index);

    for (int axial_index = sinoE_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= sinoE_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
    {
      for (int view_index = sinoE_data_ptr->get_min_view_num(); view_index <= sinoE_data_ptr->get_max_view_num(); view_index++)
      {
        for (int tangential_index = sinoE_data_ptr->get_min_tangential_pos_num(); tangential_index <= sinoE_data_ptr->get_max_tangential_pos_num(); tangential_index++)
        {
		  sum_seg += segSinoE[view_index][axial_index][tangential_index];
        } //end tangential
      } //end view
    }  //end axial
  } // end of for loop for segment range


  // Calculate True+Scatter rate by subtracting intrinsic true rate.
  // Note that the input sinogram must be random corrected, so randoms do not bother us here.
  TplusS = (sum_seg/time_duration) - ITrate ;
  if (TplusS <= 0.0)
  {
    printf("Error: T+S is less than zero!\n");
    printf("Using correction Factor 1.0\n");
    DT_CF = 1.0;
  }
  else
  {
    // Extrapolate from measured T+S to "real" T+S using a polynomial:
    TplusS_ext = TplusS*(1 + TplusS*(P2 + TplusS*(P3 + TplusS*(P4 + TplusS*(P5 + TplusS*(P6 + TplusS*(P7 + TplusS*P8)))))));

    DT_CF = TplusS_ext/TplusS;

    printf("T+S %f\n",TplusS);
    printf("T+S Extr %f\n",TplusS_ext);
    printf("Percentage DT: %f %%\n", 100.0*(1-1/DT_CF));
    printf("Correction Factor %f\n",DT_CF);
  }


  for (int segment_index = sinoE_data_ptr->get_min_segment_num(); segment_index <= sinoE_data_ptr->get_max_segment_num(); segment_index++)
  {
    printf("Segments %d\n",segment_index);
    cerr << "Processing next batch of segments" <<endl;

    SegmentByView<elem_type> segSinoE = sinoE_data_ptr->get_segment_by_view(segment_index);

    for (int axial_index = sinoE_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= sinoE_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
    {
      for (int view_index = sinoE_data_ptr->get_min_view_num(); view_index <= sinoE_data_ptr->get_max_view_num(); view_index++)
      {
        for (int tangential_index = sinoE_data_ptr->get_min_tangential_pos_num(); tangential_index <= sinoE_data_ptr->get_max_tangential_pos_num(); tangential_index++)
        {
		  (*segments_out[segment_index])[view_index][axial_index][tangential_index] = DT_CF*segSinoE[view_index][axial_index][tangential_index];
        } //end tangential
      } //end view
    }  //end axial
  } // end of for loop for segment range

  save_and_delete_segments(output, segments_out, sinoE_data_ptr->get_min_segment_num(), sinoE_data_ptr->get_max_segment_num(), *proj_data_out_ptr);
  proj_data_out_ptr = 0;

  timer.stop();
  cerr << "\nThis took " << timer.value() << "s CPU time." << endl;
}



void
allocate_segments( VectorWithOffset<segment_type *>& segments,
		  const int start_segment_index,
		  const int end_segment_index,
		  const ProjDataInfo* proj_data_info_ptr)
{
  for (int seg=start_segment_index ; seg<=end_segment_index; seg++)
  {
#ifdef USE_SegmentByView
    segments[seg] = new SegmentByView<elem_type>(proj_data_info_ptr->get_empty_segment_by_view (seg));
#else
    segments[seg] = new Array<3,elem_type>(IndexRange3D(0, proj_data_info_ptr->get_num_views()-1,
				      0, proj_data_info_ptr->get_num_axial_poss(seg)-1,
				      -(proj_data_info_ptr->get_num_tangential_poss()/2),
				      proj_data_info_ptr->get_num_tangential_poss()-(proj_data_info_ptr->get_num_tangential_poss()/2)-1));
#endif
  }
}

void
save_and_delete_segments(shared_ptr<iostream>& output,
			 VectorWithOffset<segment_type *>& segments,
			 const int start_segment_index,
			 const int end_segment_index,
			 ProjData& proj_data)
{
  for (int seg=start_segment_index; seg<=end_segment_index; seg++)
  {
    {
#ifdef USE_SegmentByView
      proj_data.set_segment(*segments[seg]);
#else
      (*segments[seg]).write_data(*output);
#endif
      delete segments[seg];
    }

  }
}



static
shared_ptr<ProjData>
construct_proj_data(shared_ptr<iostream>& output,
                    const string& output_filename,
                    const shared_ptr<ProjDataInfo>& proj_data_info_ptr)
{
  vector<int> segment_sequence_in_stream(proj_data_info_ptr->get_num_segments());
  {
    std::vector<int>::iterator current_segment_iter =
      segment_sequence_in_stream.begin();
    for (int segment_num=proj_data_info_ptr->get_min_segment_num();
         segment_num<=proj_data_info_ptr->get_max_segment_num();
         ++segment_num)
      *current_segment_iter++ = segment_num;
  }

#ifdef USE_SegmentByView
  // don't need output stream in this case
  return new ProjDataInterfile(proj_data_info_ptr, output_filename, ios::out,
                               segment_sequence_in_stream,
                               ProjDataFromStream::Segment_View_AxialPos_TangPos,
		               OUTPUTNumericType);
#else

  // this code would work for USE_SegmentByView as well, but the above is far simpler...
  output = new fstream (output_filename.c_str(), ios::out|ios::binary);
  if (!*output)
    error("Error opening output file %s\n",output_filename.c_str());

  shared_ptr<ProjDataFromStream> proj_data_ptr =
    new ProjDataFromStream(proj_data_info_ptr, output,
                           /*offset=*/0,
                           segment_sequence_in_stream,
                           ProjDataFromStream::Segment_View_AxialPos_TangPos,
		           OUTPUTNumericType);
  write_basic_interfile_PDFS_header(output_filename, *proj_data_ptr);
  return proj_data_ptr;
#endif
}


END_NAMESPACE_STIR

