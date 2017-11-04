
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

#include "stir/listmode/ModifSino.h"
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
ModifSino::
set_defaults()
{}

void
ModifSino::
initialise_keymap()
{
  parser.add_start_key("modif_sino Parameters");
  parser.add_key("input file",&input_filename);
  parser.add_key("raw file",&raw_filename);
  parser.add_key("output filename prefix",&output_filename_prefix);
  parser.add_stop_key("END");
}

bool
ModifSino::
post_processing()
{
  norm_data_ptr = ProjData::read_from_file(input_filename);
  norm_data_info_ptr = norm_data_ptr->get_proj_data_info_ptr()->clone();

  raw_data_ptr = ProjData::read_from_file(raw_filename);
  raw_data_info_ptr = raw_data_ptr->get_proj_data_info_ptr()->clone();

  num_segments = raw_data_ptr->get_num_segments();
  printf("Seg %d\n",num_segments);

  vector<pair<double, double> > frame_times(1, pair<double,double>(0,1));
  frame_defs = TimeFrameDefinitions(frame_times);

#ifdef FRAME_BASED_DT_CORR
  cerr << "ModifSino Using FRAME_BASED_DT_CORR\n";
#else
  cerr << "ModifSino NOT Using FRAME_BASED_DT_CORR\n";
#endif
  return false;
}

ModifSino::
ModifSino()
{
  set_defaults();
}

ModifSino::
ModifSino(const char * const par_filename)
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
ModifSino::
process_new_time_event(const CListTime&)
{}

void
ModifSino::
start_new_time_frame(const unsigned int)
{}


void
ModifSino::
process_data()
{
  double sino_seg2_A29[117][80] ;
  /*double Sum = 0.0 ;
  double Moy = 1.0 ;
  //float Max_norm = 0.0 ;
  //float FC = 1.0 ;
  float Max_normFC = 0.0;*/
  CPUTimer timer;
  timer.start();

  VectorWithOffset<segment_type *> segments_out (raw_data_info_ptr->get_min_segment_num(), raw_data_info_ptr->get_max_segment_num());
  //VectorWithOffset<segment_type *> segments_temp (norm_data_info_ptr->get_min_segment_num(), norm_data_info_ptr->get_max_segment_num());

  //*********** open output file
  shared_ptr<iostream> output;
  shared_ptr<ProjData> proj_data_out_ptr;

  char rest[50];
  current_frame_num = 1;
//  sprintf(rest, "_f%dg1d0b0", current_frame_num);
  const string output_filename = output_filename_prefix + rest;

  //proj_data_out_ptr = construct_proj_data(output, output_filename, norm_data_info_ptr);
  proj_data_out_ptr = construct_proj_data(output, output_filename, raw_data_info_ptr);
  allocate_segments(segments_out, raw_data_ptr->get_min_segment_num(), raw_data_ptr->get_max_segment_num(), proj_data_out_ptr->get_proj_data_info_ptr());

  for (int segment_index = raw_data_ptr->get_min_segment_num(); segment_index <= raw_data_ptr->get_max_segment_num(); segment_index++)
  {
    //Max_norm = 0.0 ;

    printf("Segments %d\n",segment_index);
    cerr << "Processing next batch of segments" <<endl;

//    SegmentByView<elem_type> segNorm = norm_data_ptr->get_segment_by_view(segment_index);
    SegmentByView<elem_type> segNorm = norm_data_ptr->get_segment_by_view(2);
    //SegmentByView<elem_type> segRaw = raw_data_ptr->get_segment_by_view(segment_index);

    /*int compt_axial = 0 ;
    int compt_view = 0 ;
    int compt_tang = 0 ;*/
//printf("nb_plan_max %d\n",compt_axial);

    /*for (int axial_index = norm_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= norm_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
    {
      compt_axial++;
    }  //end axial
    for (int view_index = norm_data_ptr->get_min_view_num(); view_index <= norm_data_ptr->get_max_view_num(); view_index++)
    {
      compt_view++;
    }
    for (int tangential_index = norm_data_ptr->get_min_tangential_pos_num(); tangential_index <= norm_data_ptr->get_max_tangential_pos_num(); tangential_index++)
    {
      compt_tang++;
    }*/

    /*int nb_plan_max = compt_axial;
printf("nb_plan_max %d\n",nb_plan_max);
    int nb_view_max = compt_view;
printf("nb_view_max %d\n",nb_view_max);
    int nb_tang_max_init = compt_tang;
printf("nb_tang_max_init %d\n",nb_tang_max_init);*/

    //int nb_tang_max = 117 ;
    //double sino_E[nb_tang_max][nb_plan_max][nb_view_max];



    for (int axial_index = raw_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= raw_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
    {
      for (int view_index = raw_data_ptr->get_min_view_num(); view_index <= raw_data_ptr->get_max_view_num(); view_index++)
      {
        for (int tangential_index = raw_data_ptr->get_min_tangential_pos_num(); tangential_index <= raw_data_ptr->get_max_tangential_pos_num(); tangential_index++)
        {
//printf("nb_plan_max %d\n",compt_axial);
            //sino_E[tangential_index+(int)(nb_tang_max/2)][axial_index][view_index] = (*segments_norm[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()];
//if (tangential_index<-40) (*segments_out[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] = 0.0;
//else if (tangential_index>=-40 && tangential_index<=40)(*segments_out[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] = (*segments_norm[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()];
//else if (tangential_index>40) (*segments_out[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] = 0.0;
sino_seg2_A29[tangential_index+58][view_index] = segNorm[view_index][29][tangential_index];
//(*segments_out[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] = (*segments_norm[2])[bin_out.view_num()][28][bin_out.tangential_pos_num()] /* (*segments_raw[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()]*/;
        } //end tangential
      } //end view
    }  //end axial
  }

  for (int segment_index = raw_data_ptr->get_min_segment_num(); segment_index <= raw_data_ptr->get_max_segment_num(); segment_index++)
  {
    //Max_norm = 0.0 ;

    printf("Segments %d\n",segment_index);
    cerr << "Processing next batch of segments" <<endl;

    SegmentByView<elem_type> segRaw = raw_data_ptr->get_segment_by_view(segment_index);

    for (int axial_index = raw_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= raw_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
    {
      for (int view_index = raw_data_ptr->get_min_view_num(); view_index <= raw_data_ptr->get_max_view_num(); view_index++)
      {
        for (int tangential_index = raw_data_ptr->get_min_tangential_pos_num(); tangential_index <= raw_data_ptr->get_max_tangential_pos_num(); tangential_index++)
        {
//(*segments_out[segment_index])[view_index][axial_index][tangential_index] = (*segments_out[2])[view_index][28][tangential_index] ;
(*segments_out[segment_index])[view_index][axial_index][tangential_index] = sino_seg2_A29[tangential_index+58][view_index] * segRaw[view_index][axial_index][tangential_index];
		}
	  }
	}
  } // end of for loop for segment range

  save_and_delete_segments(output, segments_out, raw_data_ptr->get_min_segment_num(), raw_data_ptr->get_max_segment_num(), *proj_data_out_ptr);
  proj_data_out_ptr = 0;

//printf("Somme %f, Moyenne %f\n",Sum,Moy);
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































// Début des loop

/*    for (int axial_index = norm_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= norm_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
    {
      for (int view_index = norm_data_ptr->get_min_view_num(); view_index <= norm_data_ptr->get_max_view_num(); view_index++)
      {
        Sum = 0.0 ;
        Moy = 1.0;
        Max_normFC = 0;
        int compteur_tang = 0;

        for (int tangential_index = norm_data_ptr->get_min_tangential_pos_num(); tangential_index <= norm_data_ptr->get_max_tangential_pos_num(); tangential_index++)
        {
          bin_out.segment_num() = segment_index;
          bin_out.view_num() = view_index;
          bin_out.axial_pos_num() = axial_index;
          bin_out.tangential_pos_num() = tangential_index;
          bin_out.set_bin_value(1);

          if (bin_out.get_bin_value()>0
	  	&& bin_out.tangential_pos_num() >= proj_data_out_ptr->get_min_tangential_pos_num()
	 	&& bin_out.tangential_pos_num() <= proj_data_out_ptr->get_max_tangential_pos_num()
		&& bin_out.axial_pos_num() >= proj_data_out_ptr->get_min_axial_pos_num(bin_out.segment_num())
		&& bin_out.axial_pos_num() <= proj_data_out_ptr->get_max_axial_pos_num(bin_out.segment_num())
		)
  	  {
            if ( (*segments_norm[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] == 0.0 )
            {
              (*segments_out[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] = 0.0;
            }
            else (*segments_out[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] = 1.0 //logexp((*segments_norm[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()]);

            //(*segments_out[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] = (*segments_norm[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()];

            //Sum += (*segments_norm[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()];
            //compteur_tang++;
          }
        } //end tangential

        //Moy = Sum/(norm_data_ptr->get_max_tangential_pos_num() - norm_data_ptr->get_min_tangential_pos_num()+1);
        //Moy = Sum/compteur_tang;


        for (int tangential_index = norm_data_ptr->get_min_tangential_pos_num(); tangential_index <= norm_data_ptr->get_max_tangential_pos_num(); tangential_index++)
        {

          bin_out.segment_num() = segment_index;
          bin_out.view_num() = view_index;
          bin_out.axial_pos_num() = axial_index;
          bin_out.tangential_pos_num() = tangential_index;
          bin_out.set_bin_value(1);

          if (bin_out.get_bin_value()>0
	    	&& bin_out.tangential_pos_num() >= proj_data_out_ptr->get_min_tangential_pos_num()
	 	&& bin_out.tangential_pos_num() <= proj_data_out_ptr->get_max_tangential_pos_num()
		&& bin_out.axial_pos_num() >= proj_data_out_ptr->get_min_axial_pos_num(bin_out.segment_num())
		&& bin_out.axial_pos_num() <= proj_data_out_ptr->get_max_axial_pos_num(bin_out.segment_num())
		)
  	  {
             if ( (*segments_out[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] < 0 )
            {
              (*segments_out[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] = 0;
            }

            if(Moy!=0.0)
            {
              (*segments_out[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] = (*segments_norm[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()]/Moy;
            }
          }
        } //end tangential
     } //end view
    }  //end axial*/
