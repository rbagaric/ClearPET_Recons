
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

#include "stir/listmode/ScattersByGaussianFit.h"
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

#include "stir/lmmin.h"
#include "stir/lm_eval.h"
//#include "stir/scatterfit.h"

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

/*double max(double a, double b)
{
  if (a<b) return b;
  else return a;
}

double min(double a, double b)
{
  if (a<b) return a;
  else return b;
}*/

double erf(double x);


double my_fit_function( double t, double* p )
{
        //PS: return ( p[0] + p[1]*t + p[2]*t*t );
        t = (t-p[1]) / p[2];
        return ( p[0]*exp(-0.5*(t*t)) );
}

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
ScattersByGaussianFit::
set_defaults()
{}

void
ScattersByGaussianFit::
initialise_keymap()
{
  parser.add_start_key("scatters_by_gaussian_fit Parameters");
  parser.add_key("input file",&input_filename);
  //parser.add_key("raw file",&raw_filename);
  parser.add_key("DAQ duration in seconds",&TimeDAQ);
  parser.add_key("output filename prefix",&output_filename_prefix);
  parser.add_stop_key("END");
}

bool
ScattersByGaussianFit::
post_processing()
{
  emis_data_ptr = ProjData::read_from_file(input_filename);
  emis_data_info_ptr = emis_data_ptr->get_proj_data_info_ptr()->clone();
  
  /*raw_data_ptr = ProjData::read_from_file(raw_filename);
  raw_data_info_ptr = raw_data_ptr->get_proj_data_info_ptr()->clone();*/

  num_segments = emis_data_ptr->get_num_segments();
  printf("Seg %d\n",num_segments);

  vector<pair<double, double> > frame_times(1, pair<double,double>(0,1));
  frame_defs = TimeFrameDefinitions(frame_times);

#ifdef FRAME_BASED_DT_CORR
  cerr << "ScatterCorrection Using FRAME_BASED_DT_CORR\n";
#else
  cerr << "ScatterCorrection NOT Using FRAME_BASED_DT_CORR\n";
#endif
  return false;
}

ScattersByGaussianFit::
ScattersByGaussianFit()
{
  set_defaults();
}

ScattersByGaussianFit::
ScattersByGaussianFit(const char * const par_filename)
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
ScattersByGaussianFit::
process_new_time_event(const CListTime&)
{}

void
ScattersByGaussianFit::
start_new_time_frame(const unsigned int)
{}


void
ScattersByGaussianFit::
process_data()
{
  double somme = 0.0;

// Force du lissage
  int k = 10; //tangential
  int tangential_index_new = 0;
  int l = 8; //view
  int view_index_new = 0;
  int compteur = 0;

// variable de limite de profile
  int ll = 0;
  int lh = 0;

//Pour le fit Gaussien
/*  const int n = 117;
  int gapStart = 30;
  int gapEnd = 80;
  double x0 = 55;
  double fwhm = 40;
  double a = 1000;
  double y[n];
  FitResult result;*/



  Bin bin_out;
  CPUTimer timer;
  timer.start();
  
  VectorWithOffset<segment_type *> segments_emis (emis_data_info_ptr->get_min_segment_num(), emis_data_info_ptr->get_max_segment_num());
  //VectorWithOffset<segment_type *> segments_raw (raw_data_info_ptr->get_min_segment_num(), raw_data_info_ptr->get_max_segment_num());
  VectorWithOffset<segment_type *> segments_scatter (emis_data_info_ptr->get_min_segment_num(), emis_data_info_ptr->get_max_segment_num());
  //VectorWithOffset<segment_type *> segments_temp (norm_data_info_ptr->get_min_segment_num(), norm_data_info_ptr->get_max_segment_num());

  //*********** open output file
  shared_ptr<iostream> output;
  shared_ptr<ProjData> proj_data_out_ptr;

  char rest[50];
  current_frame_num = 1;
//  sprintf(rest, "_f%dg1d0b0", current_frame_num);
  const string output_filename = output_filename_prefix + rest;

  proj_data_out_ptr = construct_proj_data(output, output_filename, emis_data_info_ptr);



  for (int segment_index = emis_data_ptr->get_min_segment_num(); segment_index <= emis_data_ptr->get_max_segment_num(); segment_index++)
  {

    printf("Segments %d\n",segment_index);
    cerr << "Processing next batch of segments" <<endl;
    const int end_segment_index = (int)(min( proj_data_out_ptr->get_max_segment_num()+1, segment_index + num_segments)-1);

    allocate_segments(segments_emis, segment_index, end_segment_index, emis_data_ptr->get_proj_data_info_ptr());
    //allocate_segments(segments_raw, segment_index, end_segment_index, raw_data_ptr->get_proj_data_info_ptr());
    allocate_segments(segments_scatter, segment_index, end_segment_index, proj_data_out_ptr->get_proj_data_info_ptr());
    //allocate_segments(segments_temp, segment_index, end_segment_index, proj_data_out_ptr->get_proj_data_info_ptr());

    *segments_emis[segment_index] = emis_data_ptr->get_segment_by_view(segment_index);
    //*segments_raw[segment_index]  = raw_data_ptr->get_segment_by_view(segment_index);



    int compt_axial = 0 ;
    int compt_view = 0 ;
    int compt_tang = 0 ;

    for (int axial_index = emis_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= emis_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
    {
      compt_axial++;
    }  //end axial
    for (int view_index = emis_data_ptr->get_min_view_num(); view_index <= emis_data_ptr->get_max_view_num(); view_index++)
    {
      compt_view++;
    }
    for (int tangential_index = emis_data_ptr->get_min_tangential_pos_num(); tangential_index <= emis_data_ptr->get_max_tangential_pos_num(); tangential_index++)
    {
      compt_tang++;
    }



    int nb_plan_max = compt_axial;
printf("nb_plan_max %d\n",nb_plan_max);
    int nb_view_max = compt_view;
printf("nb_view_max %d\n",nb_view_max);
    int nb_tang_max = compt_tang;
printf("nb_tang_max %d\n",nb_tang_max);

    int cont[nb_view_max][nb_plan_max];
    double compteur_somme_total = 0.0 ;
    double compteur_somme_profil[nb_plan_max][nb_view_max];
    double OP[nb_plan_max][nb_view_max];
    for ( int A = 0 ; A<nb_plan_max ; A++)
    {
      for ( int V = 0 ; V<nb_view_max ; V++)
      {
        compteur_somme_profil[A][V] = 0.0 ;
        OP[A][V] = 0.0 ;
      }
    }



// Recherche des sinos vides
    for (int axial_index = emis_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= emis_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
    {
      for (int view_index = emis_data_ptr->get_min_view_num(); view_index <= emis_data_ptr->get_max_view_num(); view_index++)
      {
        for(int L=(int)(0.3*nb_tang_max) ; L <= 0.7*nb_tang_max ; L++)
        {
          bin_out.segment_num() = segment_index;
          bin_out.view_num() = view_index;
          bin_out.axial_pos_num() = axial_index;
          bin_out.tangential_pos_num() = L;
          bin_out.set_bin_value(1);

          if (bin_out.get_bin_value()>0
	  	&& bin_out.tangential_pos_num() >= proj_data_out_ptr->get_min_tangential_pos_num()
	 	&& bin_out.tangential_pos_num() <= proj_data_out_ptr->get_max_tangential_pos_num()
		&& bin_out.axial_pos_num() >= proj_data_out_ptr->get_min_axial_pos_num(bin_out.segment_num())
		&& bin_out.axial_pos_num() <= proj_data_out_ptr->get_max_axial_pos_num(bin_out.segment_num())
		) 
  	  {
            if ( (*segments_emis[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] == 0.0 ) cont[view_index][axial_index]=cont[view_index][axial_index]+1;
          }
        } //end L
      } // end view
    }  //end axial


    for (int axial_index = emis_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= emis_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
    {
      for (int view_index = emis_data_ptr->get_min_view_num(); view_index <= emis_data_ptr->get_max_view_num(); view_index++)
      {
    for (int tangential_index = emis_data_ptr->get_min_tangential_pos_num(); tangential_index <= emis_data_ptr->get_max_tangential_pos_num(); tangential_index++)
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
             compteur_somme_total += (*segments_emis[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] ;
             compteur_somme_profil[axial_index][view_index] += (*segments_emis[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] ;
          }
        } //end tangential
      } // end view
    }  //end axial


    for ( int A = 0 ; A<nb_plan_max ; A++)
    {
      for ( int V = 0 ; V<nb_view_max ; V++)
      {
        OP[A][V] = (double)(compteur_somme_profil[A][V]/compteur_somme_total)*1000000. ;
      }
    }


    for (int axial_index = emis_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= emis_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
    {


// Recherche des plans != 0

      for (int view_index = emis_data_ptr->get_min_view_num(); view_index <= emis_data_ptr->get_max_view_num(); view_index++)
      {

        for (int tangential_index = emis_data_ptr->get_min_tangential_pos_num(); tangential_index <= emis_data_ptr->get_max_tangential_pos_num(); tangential_index++)
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

            somme += (*segments_emis[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()];

          } 
        } //end tangential
      } //end view


      if (somme!=0.0) {


// Lissage tangential k=10
        for (int view_index = emis_data_ptr->get_min_view_num(); view_index <= emis_data_ptr->get_max_view_num(); view_index++)
        {
          for (int tangential_index = emis_data_ptr->get_min_tangential_pos_num(); tangential_index <= emis_data_ptr->get_max_tangential_pos_num(); tangential_index++)
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

              compteur = 0 ;
              for ( int i=-k/2; i<=k/2; i++)
              {
                tangential_index_new = tangential_index + i;

                if ( tangential_index_new >= emis_data_ptr->get_min_tangential_pos_num() && tangential_index_new <= emis_data_ptr->get_max_tangential_pos_num() )
                {
                  (*segments_scatter[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] += (*segments_emis[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][tangential_index_new];
                  compteur++;
                }
              }
              if (compteur != 0) (*segments_scatter[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] /= (compteur);

            }
          } //end tangential
        } // end view


// Remise a 0 de emis
        for (int view_index = emis_data_ptr->get_min_view_num(); view_index <= emis_data_ptr->get_max_view_num(); view_index++)
        {
          for (int tangential_index = emis_data_ptr->get_min_tangential_pos_num(); tangential_index <= emis_data_ptr->get_max_tangential_pos_num(); tangential_index++)
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
              (*segments_emis[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] = 0.0 ;
            }
          } //end tangential
        } // end view


// Lissage view l=8
        for (int view_index = emis_data_ptr->get_min_view_num(); view_index <= emis_data_ptr->get_max_view_num(); view_index++)
        {
          for (int tangential_index = emis_data_ptr->get_min_tangential_pos_num(); tangential_index <= emis_data_ptr->get_max_tangential_pos_num(); tangential_index++)
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

              compteur = 0;
              for ( int j=-l/2; j<=l/2; j++)
              {
                view_index_new = view_index + j;

                if ( view_index_new >= emis_data_ptr->get_min_view_num() && view_index_new <= emis_data_ptr->get_max_view_num() )
                {
                  (*segments_emis[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] += (*segments_scatter[bin_out.segment_num()])[view_index_new][bin_out.axial_pos_num()][bin_out.tangential_pos_num()];
                compteur++;
                }
              }
              if (compteur != 0) (*segments_emis[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] /= (compteur);
            }
          } //end tangential
        } // end view



// Sino lisse dans un tableau de taille nb_tang_max
        double sinolisse[nb_tang_max];
        for (int view_index = emis_data_ptr->get_min_view_num(); view_index <= emis_data_ptr->get_max_view_num(); view_index++)
        {
          for (int tangential_index = emis_data_ptr->get_min_tangential_pos_num(); tangential_index <= emis_data_ptr->get_max_tangential_pos_num(); tangential_index++)
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
              sinolisse[tangential_index+(int)(nb_tang_max/2)] = (*segments_emis[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()];
            }
          } //end tangential


// Linear Fit

          double Xcoord1[nb_tang_max];
          double SFcorrected[nb_tang_max];
          double premder[nb_tang_max];
          double result[2];
          double maxpremder = 0.0 ;
          double minpremder = 1000000000.0 ;
          int J = 0 ;
          int K = 0 ;
          //double factor = 0.62 ;
          //double factor = ((compteur_somme_total/TimeDAQ)-51300.)/(-65000.) ;
          double factor = 0.4 ;
          //printf("Somme total %f , Factor %f\n",compteur_somme_total,factor);
          int LowLimit;
          int HighLimit;
          int centergauss;
          int LimitLL;
          int LimitLH;
          int LimitHL;
          int LimitHH;
          int counter = -1 ;



          // generation d'un tableau avec les elts 1..nb_plan_max, cad la X de mon fit
          for (int i = 0 ; i < nb_tang_max; i++)
          {
            Xcoord1[i] = i ; 
          }

          for (int i = 0 ; i < nb_tang_max ; i++)
          {
            if ( i < 2 || i > nb_tang_max-3)
            {
              premder[i] = 0.0;
            }
            else
            {
              ll = i-2 ;
              lh = i+2 ;
              double A[(lh-ll)+1];
              double B[(lh-ll)+1];
              double sumX = 0.0 ;
              double sumY = 0.0 ;
              int compt_lin = 0 ;
              double num = 0.0 ;
              double denom = 0.0 ;

              for ( int j=ll ; j<= lh ; j++)
              {
                 B[j] = sinolisse[j];
                 A[j] = Xcoord1[j] ;
//Verif
//if ( bin_out.segment_num()==0 && bin_out.view_num()==40 && bin_out.axial_pos_num()==30 ) printf("B %f A %f\n", B[j],A[j]);
              }


              for ( int j=ll ; j<= lh ; j++)
              {
                sumX = sumX + A[j] ;
                sumY = sumY + B[j] ;
                compt_lin++ ;
              }
                
              sumX /= compt_lin;
              sumY /= compt_lin;

              for ( int j=ll ; j<= lh ; j++)
              {
                num = num + ((A[j]-sumX)*(B[j]-sumY)) ;
                denom = denom + ((A[j]-sumX)*(A[j]-sumX)) ;
//Verif
//if ( bin_out.segment_num()==0 && bin_out.view_num()==40 && bin_out.axial_pos_num()==30 ) printf("num %f denom %f\n", num, denom);
              }

              result[1] = num/denom ;
              result[0] = sumY - (result[1]*sumX);
//Verif
//if ( bin_out.segment_num()==0 && bin_out.view_num()==40 && bin_out.axial_pos_num()==30 ) printf("result1 %f result0 %f\n", result[1], result[0]);

              premder[i] = result[1];
//Verif
//if ( bin_out.segment_num()==0 && bin_out.view_num()==40 && bin_out.axial_pos_num()==30 ) printf("premder %f\n", premder[i]);

            } //end else
          //} //end for i

          //for (int i = 0 ; i < nb_tang_max; i++)
          //{
            if (maxpremder < premder[i] )
            {
              maxpremder = premder[i];
              J = i ;
            }
            if (minpremder > premder[i] )
            {
              minpremder = premder[i];
              K = i ;
            }
          } //end for i ligne 495


//Verif
/*if ( bin_out.segment_num()==-5 && bin_out.view_num()==0 && bin_out.axial_pos_num()==11 )
{
  printf("maxpremder %f, J %d, minpremder %f, K %d\n", maxpremder, J, minpremder, K);
  for ( int t=0 ; t < nb_tang_max ; t++)
  {
    printf("maxpremder %f, J %d, minpremder %f, K %d\n", maxpremder, J, minpremder, K);
    printf("premder %f\n", premder[t]);
  }
}*/

          //if (maxpremder > 0.5)
//printf("OP %f\n", OP[axial_index][view_index]);
          if (OP[axial_index][view_index] > 200.)
          {
            LowLimit = round(factor*J);
            HighLimit = round(nb_tang_max-(factor*(nb_tang_max-K)));
            centergauss = (K+J)/2 ;
          }
          else
          {
            LowLimit = (int)(0.3*nb_tang_max);
            HighLimit = (int)(0.7*nb_tang_max);
            centergauss = (HighLimit+LowLimit)/2 ;
          }

          LimitLL = 0 ; //Limite basse de la queue de distribution a gauche
          LimitLH = LowLimit ; //Limite haute de la queue de distribution a gauche
          LimitHL = HighLimit ; //Limite basse de la queue de distribution a droite
          LimitHH = nb_tang_max ; //Limite haute de la queue de distribution a droite

          int nb2_tang_max = (LimitLH - LimitLL) + 1 + (LimitHH - LimitHL) + 1 + 2 ;

          double gaussprof[nb_tang_max];
          double gaussprofDEC[nb_tang_max];
          double SinoSC[nb2_tang_max];
          double XcoordSC[nb2_tang_max];


          double averlow = (*segments_emis[segment_index])[view_index][axial_index][LimitLH-(int)(nb_tang_max/2)] ;
          double averhigh = (*segments_emis[segment_index])[view_index][axial_index][LimitHL-(int)(nb_tang_max/2)] ;
          double aver = (double)((averlow + averhigh) / 2.) ;
          double averA = (double)((averlow + aver) / 2.) ;
          double averB = (double)((averhigh + aver) / 2.) ;
          double averX = (double)((LimitHL + LimitLH) / 2.) ;
          double averAX = (double)((averX + LimitLH) / 2.) ;
          double averBX = (double)((LimitHL + averX) / 2.) ;


          double b = (J+K)/2.0 ;
          double c = (K-J)/1.5 ;
          double SF = 23.0 ;
          double area = compteur_somme_profil[axial_index][view_index]*SF/100.0 ;
          double a = area / ( -1.25331*c*erf( (-56.56854 + 0.70710*b)/c ) + 1.25331*c*erf( (0.70710*b)/c ) );
if ( segment_index==0 && axial_index==30 && view_index==40 )printf ("a %f, b %f, c %f, total %f, area %f\n",a,b,c,compteur_somme_profil[axial_index][view_index],area);
          double gaussTEST[nb_tang_max];
          double SinoSCbis[nb_tang_max];
          double XcoordSCbis[nb_tang_max];


          for ( int i=0 ; i < nb_tang_max ; i++)
          {
            double CP = (Xcoord1[i] - b) / c ;
            gaussTEST[i] = a * exp((-CP*CP)/2) ;
          }

          for (int i = 0 ; i < nb_tang_max ; i++)
          {
            if( i <= LimitLH || i >= LimitHL )
            {
              SinoSCbis[i] = sinolisse[i] ;
              XcoordSCbis[i] = i ;
            }
            else
            {
              SinoSCbis[i] = gaussTEST[i];
              XcoordSCbis[i] = i;
            }
          }
          /*for (int i = 0 ; i < nb_tang_max ; i++)
          {
            if( i <= LimitLH || i >= LimitHL )
            {
              counter = counter + 1 ;
              SinoSC[counter] = sinolisse[i] ;
              XcoordSC[counter] = i ;
            }
            else if ( i == LimitLH+1 )
            {
              counter = counter + 1;
              SinoSC[counter] = averA*1;
              XcoordSC[counter] = averAX;
            }
            else if ( i == LimitLH+2 )
            {
              counter = counter + 1;
              SinoSC[counter] = aver*1;
              XcoordSC[counter] = averX;
            }
            else if ( i == LimitLH+3 )
            {
              counter = counter + 1;
              SinoSC[counter] = averB*1;
              XcoordSC[counter] = averBX;
            }
          } //end for i ligne 614
          */

          //FIT GAUSSIEN
          //int m_dat = nb2_tang_max;
          int m_dat = nb_tang_max;
          int n_p =  3;

//if ( aver == 0.0 ) aver=0.05;

          //double p[3] = { aver*2, averX, nb2_tang_max/2}; // use any starting value, but not { 0,0,0 }
          double p[3] = { a, b, c}; // use any starting value, but not { 0,0,0 }

//Verif
/*if ( bin_out.segment_num()==-5 && bin_out.view_num()==0 && bin_out.axial_pos_num()==11 )
{
  printf(" aver*2: %f averX: %f nb2: %i nb2/2: %f\n",aver*2, averX, nb2_tang_max, (double)(nb2_tang_max/2));
  for ( int i=0 ; i<nb2_tang_max ; i++)
  {
    printf("XcoordSC: %f, SinoSC: %f\n", XcoordSC[i], SinoSC[i]);
  }
}*/



          // auxiliary settings:
          lm_control_type control;
          lm_data_type data;

          lm_initialize_control( &control );

          data.user_func = my_fit_function;
          data.user_t = XcoordSCbis;
          data.user_y = SinoSCbis;
          /*data.user_t = XcoordSC;
          data.user_y = SinoSC;*/

          // perform the fit:
          //printf( "modify or replace lm_print_default for less verbous fitting\n");
          lm_minimize( m_dat, n_p, p, lm_evaluate_default, lm_print_default, &data, &control );

          // print results:
          //printf( "status: %s after %d evaluations\n", lm_shortmsg[control.info], control.nfev );

//Verif
if ( bin_out.segment_num()==0 && bin_out.view_num()==40 && bin_out.axial_pos_num()==30 )
{
  printf(" p0: %f p1: %f p2: %f\n",p[0], p[1], p[2]);
}

          if ( p[1] >= nb_tang_max || p[1]<0)
          {
            p[0] = a ;
            p[1] = b ;
            p[2] = c ;
          }

          /*if ( p[1] >= nb_tang_max || p[1]<0)
          {
            p[0] = aver*2 ;
            p[1] = averX ;
            p[2] = nb2_tang_max/2 ;
          }*/

          for ( int i=0 ; i < nb_tang_max ; i++)
          {
            //double Z = (Xcoord1[i] - p[2]) / p[2] ;
            double Z = (Xcoord1[i] - centergauss) / p[2] ;
            gaussprof[i] = p[0] * exp((-Z*Z)/2) ;
          }

          for (int tangential_index = emis_data_ptr->get_min_tangential_pos_num(); tangential_index <= emis_data_ptr->get_max_tangential_pos_num(); tangential_index++)
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
              (*segments_scatter[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] = gaussprof[bin_out.tangential_pos_num()+(int)(nb_tang_max/2)] ;
              //(*segments_scatter[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] =  (*segments_emis[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()]  ;
            }
          } //end tangential

        } // end view

      } //end somme!=0

    }  //end axial



 
    save_and_delete_segments(output, segments_scatter, segment_index, end_segment_index, *proj_data_out_ptr);
    
  } // end of for loop for segment range

  timer.stop();
  cerr << "\nThis took " << timer.value() << "s CPU time." << endl;

}



double erf(double x)
{
  double t, z, retval;
  z = fabs( x ) ;
  t = 1.0 / ( 1.0 + 0.5 *z );
  retval = t * exp( -z * z - 1.26551223 + t *
			( 1.00002368 + t *
			  ( 0.37409196 + t *
			    ( 0.09678418 + t *
			      ( -0.18628806 + t *
			        ( 0.27886807 + t *
			          ( -1.13520398 + t *
			            ( 1.48851587 + t *
			              ( -0.82215223 + t *
			                0.1708727 ) ) ) ) ) ) ) ) );
  if ( x < 0.0 ) return retval - 1.0 ;

  return 1.0 - retval;
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
