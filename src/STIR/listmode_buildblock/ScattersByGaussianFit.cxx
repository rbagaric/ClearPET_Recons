/*************************
Code pour un sino de rat 117
***************************/



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
  //parser.add_key("raw file",&raw_filename)parser.add_key("DAQ duration in seconds",&TimeDAQ);
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
  int T_new = 0;
  int l = 8; //view
  int V_new = 0;
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

  VectorWithOffset<segment_type *> segments_scatter (emis_data_info_ptr->get_min_segment_num(), emis_data_info_ptr->get_max_segment_num());

  //*********** open output file
  shared_ptr<iostream> output;
  shared_ptr<ProjData> proj_data_out_ptr;

  current_frame_num = 1;
  const string output_filename = output_filename_prefix;

  proj_data_out_ptr = construct_proj_data(output, output_filename, emis_data_info_ptr);
  allocate_segments(segments_scatter, emis_data_ptr->get_min_segment_num(), emis_data_ptr->get_max_segment_num(), proj_data_out_ptr->get_proj_data_info_ptr());

  for (int segment_index = emis_data_ptr->get_min_segment_num(); segment_index <= emis_data_ptr->get_max_segment_num(); segment_index++)
  {
    printf("Segments %d\n",segment_index);
    cerr << "Processing next batch of segments" <<endl;

    SegmentByView<elem_type> segEmis = emis_data_ptr->get_segment_by_view(segment_index);

    int compt_obj = 0 ;
    double sftot = 0.0 ;

    int compt_axial = emis_data_ptr->get_max_axial_pos_num(segment_index)+1 - emis_data_ptr->get_min_axial_pos_num(segment_index);
    int compt_view = emis_data_ptr->get_max_view_num()+1 - emis_data_ptr->get_min_view_num();
    int compt_tang = emis_data_ptr->get_max_tangential_pos_num()+1 - emis_data_ptr->get_min_tangential_pos_num();

    int nb_plan_max = compt_axial;
printf("nb_plan_max %d\n",nb_plan_max);
    int nb_view_max = compt_view;
printf("nb_view_max %d\n",nb_view_max);
    int nb_tang_max_init = compt_tang;
printf("nb_tang_max_init %d\n",nb_tang_max_init);

    double Factor=1.0;
    double SFobj=1.0;
    double ObjCrit=1.0;
    double reshapesfcorr=1.0;

    int nb_tang_max = nb_tang_max_init;
    if (nb_tang_max_init > 100)
	  nb_tang_max -=20;

    float  sino_E[nb_tang_max][nb_plan_max][nb_view_max];
    float  sino_EInit[nb_tang_max][nb_plan_max][nb_view_max];

    if( nb_tang_max_init > 100 )
    {
      nb_tang_max = nb_tang_max_init-20 ;

      for (int axial_index = emis_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= emis_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
      {
        for (int view_index = emis_data_ptr->get_min_view_num(); view_index <= emis_data_ptr->get_max_view_num(); view_index++)
        {
          for (int tangential_index = emis_data_ptr->get_min_tangential_pos_num()+10; tangential_index <= emis_data_ptr->get_max_tangential_pos_num()-10; tangential_index++)
          {
              sino_E[tangential_index+(int)(nb_tang_max/2)][axial_index][view_index] = segEmis[view_index][axial_index][tangential_index];
	      }
	    }
      }

	  Factor = 0.3 ;
	  SFobj = 35.0 ;
	  ObjCrit = 75.0 ;
	  reshapesfcorr = 1.0 ;
    }
    else
    {
      nb_tang_max = nb_tang_max_init ;
      for (int axial_index = emis_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= emis_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
      {
        for (int view_index = emis_data_ptr->get_min_view_num(); view_index <= emis_data_ptr->get_max_view_num(); view_index++)
        {
          for (int tangential_index = emis_data_ptr->get_min_tangential_pos_num(); tangential_index <= emis_data_ptr->get_max_tangential_pos_num(); tangential_index++)
          {
			sino_E[tangential_index+(int)(nb_tang_max/2)][axial_index][view_index] = segEmis[view_index][axial_index][tangential_index];
          } // end tangential
        } // end view
      } // end axial

	  Factor = 0.4 ;
	  SFobj = 23.0 ;
	  ObjCrit = 85.0 ;
	  reshapesfcorr = 0.0 ;
    } //end else


      /*for (int axial_index = emis_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= emis_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
      {
        for (int view_index = emis_data_ptr->get_min_view_num(); view_index <= emis_data_ptr->get_max_view_num(); view_index++)
        {
          for (int tangential_index = emis_data_ptr->get_min_tangential_pos_num()+10; tangential_index <= emis_data_ptr->get_max_tangential_pos_num()-10; tangential_index++)
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

    for (int A = 0 ; A < nb_plan_max ; A++)
    {
      for (int V = 0 ; V < nb_view_max ; V++)
      {
        for (int T = 0 ; T < nb_tang_max ; T++)
        {
printf("Tang %d View %d Axial %d sinoE %f sinoemis %f\n",tangential_index+(int)(nb_tang_max/2),view_index,axial_index,sino_E[tangential_index+(int)(nb_tang_max/2)][axial_index][view_index],(*segments_emis[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()]);
//printf("Tang %d View %d Axial %d sinoE %f\n",T,V,A,sino_E[T][A][V]);
}}}}*/


    float  sfcorr[nb_tang_max][nb_plan_max][nb_view_max];
    //double object[nb_tang_max][nb_plan_max][nb_view_max];
    float  sfvec[nb_plan_max][nb_view_max];
    float  gp[nb_tang_max][nb_plan_max][nb_view_max];
    //double maximoPD[nb_plan_max][nb_view_max];
    //double sfobject[1];

    float  sino_E_lissT[nb_tang_max][nb_plan_max][nb_view_max];

    int cont[nb_view_max][nb_plan_max];
    double compteur_somme_total = 0.0 ;
    float  compteur_somme_profil[nb_plan_max][nb_view_max];
    float  OP[nb_plan_max][nb_view_max];

    float  centerofmass[nb_plan_max][nb_view_max];
    float  sumcenterofmass[nb_plan_max][nb_view_max];

    for ( int A = 0 ; A<nb_plan_max ; A++)
    {
      for ( int V = 0 ; V<nb_view_max ; V++)
      {
        compteur_somme_profil[A][V] = 0.0 ;
        OP[A][V] = 0.0 ;
        centerofmass[A][V] = 0.0 ;
        sumcenterofmass[A][V] = 0.0 ;
        for (int T = 0 ; T < nb_tang_max ; T++)
        {
          sino_E_lissT[T][A][V] = 0.0 ;
          sino_EInit[T][A][V] = sino_E[T][A][V];
        }
      }
    }

// Recherche des sinos vides
    for ( int A = 0 ; A<nb_plan_max ; A++)
    {
      for ( int V = 0 ; V<nb_view_max ; V++)
      {
        for(int L=(int)(0.3*nb_tang_max) ; L <= 0.7*nb_tang_max ; L++)
        {
          if ( sino_E[L][A][V] == 0.0 )
            cont[V][A] = cont[V][A]+1;
        } //end L
      } // end V
    }  //end A


    for (int A = 0 ; A < nb_plan_max ; A++)
    {
      for (int V = 0 ; V < nb_view_max ; V++)
      {
        for (int T = 0 ; T < nb_tang_max ; T++)
        {
//printf("Tang %d View %d Axial %d sinoE %f compt_tot %f\n",T,V,A,sino_E[T][A][V],compteur_somme_total);

//if ( segment_index==0 && A==30 && V==40 ) printf ("tang %d, valeur %f\n",T,sino_E[T][A][V]);

          compteur_somme_total += sino_E[T][A][V] ;
          compteur_somme_profil[A][V] += sino_E[T][A][V] ;
          sumcenterofmass[A][V] += T*sino_E[T][A][V];
//if ( segment_index==0 && A==30 && V==40 )printf ("sumcenterofmass %f\n",sumcenterofmass[A][V]);
        } //end T
      } // end V
    }  //end A

//printf("%f\n",compteur_somme_total);

    for ( int A = 0 ; A<nb_plan_max ; A++)
    {
      for ( int V = 0 ; V<nb_view_max ; V++)
      {
        OP[A][V] = (double)(compteur_somme_profil[A][V]/compteur_somme_total)*1000000. ;
        centerofmass[A][V] = (double)(sumcenterofmass[A][V]/compteur_somme_profil[A][V]);
        //printf("%f    %f\n",OP[A][V],centerofmass[A][V]);
      }
    }

//printf("Debug 1\n");
//printf("%f    %f\n",OP[20][40],centerofmass[20][40]);
    for ( int A = 0 ; A<nb_plan_max ; A++)
    {

// Recherche des plans != 0
      for ( int V = 0 ; V<nb_view_max ; V++)
      {
        for (int T = 0; T < nb_tang_max ; T++)
        {
          somme += sino_E[T][A][V];
        } //end T
      } //end V

      if (somme!=0.0)
      {
// Lissage tangential k=10
        for ( int V = 0 ; V<nb_view_max ; V++)
        {
          for (int T = 0; T < nb_tang_max ; T++)
          {
            compteur = 0 ;
            for ( int i=-k/2; i<=k/2; i++)
            {
              T_new = T + i;

              if ( T_new >= 0 && T_new < nb_tang_max )
              {
                sino_E_lissT[T][A][V] += sino_E[T_new][A][V];
//if ( segment_index==0 && A==30 && V==40 ) printf ("tang %d, E %f, E_liss %f\n",T,sino_E[T][A][V],sino_E_lissT[T][A][V]);
                compteur++;
              }
            }
            if (compteur != 0)
              sino_E_lissT[T][A][V] /= (compteur);

//if ( segment_index==0 && A==30 && V==40 ) printf ("tang %d, valeur %f\n",T,sino_E_lissT[T][A][V]);
          } //end T
        } // end V

// Remise a 0 de emis
        for ( int V = 0 ; V<nb_view_max ; V++)
        {
          for (int T = 0; T < nb_tang_max ; T++)
          {
            sino_E[T][A][V] = 0.0 ;
          } //end T
        } // end V

// Lissage view l=8
        for ( int V = 0 ; V<nb_view_max ; V++)
        {
          for (int T = 0; T < nb_tang_max ; T++)
          {
            compteur = 0;
            for ( int j=-l/2; j<=l/2; j++)
            {
              V_new = V + j;

              if ( V_new >= 0 && V_new < nb_view_max )
              {
                sino_E[T][A][V] += sino_E_lissT[T][A][V_new];
                compteur++;
              }
            }
            if (compteur != 0)
              sino_E[T][A][V] /= (compteur);

//if ( segment_index==0 && A==30 && V==40 ) printf ("tang %d, E %f, E_liss %f\n",T,sino_E[T][A][V],sino_E_lissT[T][A][V]);
          } //end T
        } // end V

// Sino lisse dans un tableau de taille nb_tang_max
        double sinolisse[nb_tang_max];
        for ( int V = 0 ; V<nb_view_max ; V++)
        {
          for (int T = 0; T < nb_tang_max ; T++)
          {
            sinolisse[T] = sino_E[T][A][V];
//if ( segment_index==0 && A==30 && V==40 ) printf ("tang %d, valeur %f\n",T,sinolisse[T]);
          } //end T

// Linear Fit
          double Xcoord1[nb_tang_max];
          double SFcorrected[nb_tang_max];
          double premder[nb_tang_max];
          double result[2];
          double maxpremder = 0.0 ;
          double minpremder = 1000000000.0 ;
          int J = 0 ;
          int K = 0 ;
          int LowLimit;
          int HighLimit;
          int centergauss;
          int LimitLL;
          int LimitLH;
          int LimitHL;
          int LimitHH;
          //int counter = -1 ;

          int objpresence;
          double SF;

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
/*if ( segment_index==0 && A==30 && V==40 )
{
  printf("maxpremder %f, J %d, minpremder %f, K %d\n", maxpremder, J, minpremder, K);
  for ( int t=0 ; t < nb_tang_max ; t++)
  {
    printf("maxpremder %f, J %d, minpremder %f, K %d\n", maxpremder, J, minpremder, K);
    printf("premder %f\n", premder[t]);
  }
}*/
//if ( segment_index==0 && A==30 && V==40 )printf ("OP %f,  centerofmass %f\n",OP[A][V],centerofmass[A][V]);
          if (OP[A][V] > ObjCrit)
          {
            objpresence = 1 ;
            LowLimit = round(Factor*J);
            HighLimit = round(nb_tang_max-(Factor*(nb_tang_max-K)));
            centergauss = (int)(centerofmass[A][V]) ;
            //centergauss = (K+J)/2 ;
            SF = SFobj ;
            compt_obj++;
          }
          else
          {
            objpresence = 0 ;
            LowLimit = (int)(0.2*nb_tang_max);
            HighLimit = (int)(0.8*nb_tang_max);
            centergauss = (int)(centerofmass[A][V]) ;
            //centergauss = (HighLimit+LowLimit)/2 ;
            //SF = 100.0 ;
            SF = 0.0 ;
            J = LowLimit ;
            K = HighLimit ;
          }

          LimitLL = 0 ; //Limite basse de la queue de distribution a gauche
          LimitLH = LowLimit ; //Limite haute de la queue de distribution a gauche
          LimitHL = HighLimit ; //Limite basse de la queue de distribution a droite
          LimitHH = nb_tang_max ; //Limite haute de la queue de distribution a droite

          //int nb2_tang_max = (LimitLH - LimitLL) + 1 + (LimitHH - LimitHL) + 1 + 2 ;

          double gaussprof[nb_tang_max];
          //double gaussprofDEC[nb_tang_max];
          //double SinoSC[nb2_tang_max];
          //double XcoordSC[nb2_tang_max];


          /*double averlow = sino_E[LimitLH][A][V] ;
          double averhigh = sino_E[LimitHL][A][V] ;
          double aver = (double)((averlow + averhigh) / 2.) ;
          double averA = (double)((averlow + aver) / 2.) ;
          double averB = (double)((averhigh + aver) / 2.) ;
          double averX = (double)((LimitHL + LimitLH) / 2.) ;
          double averAX = (double)((averX + LimitLH) / 2.) ;
          double averBX = (double)((LimitHL + averX) / 2.) ;*/


          //double b = (J+K)/2.0 ;
          double b = centergauss ;
          //double c = (K-J)/1.5 ;
          double c = (K-J)/1.75 ;

          if (b < 0.0 || b > nb_tang_max)
            b = nb_tang_max/2.0;
          if (c < 20.0)
            c = 20.0;
          if (c > 30.0)
            c = 30.0;

          double area = compteur_somme_profil[A][V]*SF/100.0 ;
          double a = area / ( -1.25331*c*erf( (-56.56854 + 0.70710*b)/c ) + 1.25331*c*erf( (0.70710*b)/c ) );
//if ( segment_index==0 && A==1 /*&& V==40*/ )printf ("a %f, b %f, c %f, total %f, area %f\n",a,b,c,compteur_somme_profil[A][V],area);
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
//if ( segment_index==0 && A==1 /*&& (i==40) && V==40*/ )printf ("P[0] %f\n",p[0]);
          lm_minimize( m_dat, n_p, p, lm_evaluate_default, lm_print_default, &data, &control );
//if ( segment_index==0 && A==1 /*&& (i==40) && V==40*/ )printf ("P[0] %f\n",p[0]);

          // print results:
          //printf( "status: %s after %d evaluations\n", lm_shortmsg[control.info], control.nfev );

//Verif
/*if ( bin_out.segment_num()==0 && bin_out.view_num()==40 && bin_out.axial_pos_num()==30 )
{
  printf(" p0: %f p1: %f p2: %f\n",p[0], p[1], p[2]);
}*/

          if (area==0)
            p[0] = a ;

//if ( segment_index==0 && A==1 /*&& (i==40) && V==40*/ )printf ("P[0] %f\n",p[0]);

          if ( p[1] > nb_tang_max || p[1]<0)
          {
            p[0] = a ;
            p[1] = b ;
            p[2] = c ;
          }
          if ( p[2] > 100 || p[2]<0)
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
            SFcorrected[i] = sino_EInit[i][A][V] - gaussprof[i] ;
//if ( segment_index==0 && A==1 && (i==40) /*&& V==40*/ )printf ("P[0] %f, gauss %f, sino_E %f, SFcorr %f\n",p[0], gaussprof[i],sino_EInit[i][A][V],SFcorrected[i]);
          }

          for ( int i=0 ; i < nb_tang_max ; i++)
          {
            sfcorr[i][A][V] = SFcorrected[i] ;
            gp[i][A][V] = gaussprof[i];
          }

          double compt_SFcorrected = 0.0 ;
          for ( int i=0 ; i < nb_tang_max ; i++)
          {
            compt_SFcorrected += SFcorrected[i] ;
          }

          sfvec[A][V] = 100.0 - ( (compt_SFcorrected/compteur_somme_profil[A][V])*100. ) ;

//if ( segment_index==0 && A==30 && V==40 )printf ("sfvec %f\n",sfvec[A][V]);
          if (objpresence == 1.0) sftot+=sfvec[A][V];

          for (int T = 0; T < nb_tang_max ; T++)
          {
            bin_out.segment_num() = segment_index;
            bin_out.view_num() = V;
            bin_out.axial_pos_num() = A;
            bin_out.tangential_pos_num() = T-(int)(nb_tang_max/2);
            bin_out.set_bin_value(1);

            if (bin_out.get_bin_value()>0
	  	  && bin_out.tangential_pos_num() >= proj_data_out_ptr->get_min_tangential_pos_num()
	 	  && bin_out.tangential_pos_num() <= proj_data_out_ptr->get_max_tangential_pos_num()
		  && bin_out.axial_pos_num() >= proj_data_out_ptr->get_min_axial_pos_num(bin_out.segment_num())
		  && bin_out.axial_pos_num() <= proj_data_out_ptr->get_max_axial_pos_num(bin_out.segment_num())
		  )
  	    {
              (*segments_scatter[bin_out.segment_num()])[bin_out.view_num()][bin_out.axial_pos_num()][bin_out.tangential_pos_num()] = /*gaussprof*/SFcorrected[T] ;
            }
          } //end tangential

        } // end view

      } //end if somme!=0
      else
      {
        for ( int view_index = 0 ; view_index<nb_view_max ; view_index++)
        {
          for (int T = 0; T < nb_tang_max ; T++)
          {
            int tt = T-(int)(nb_tang_max/2);

            if (tt >= proj_data_out_ptr->get_min_tangential_pos_num()
			&& tt <= proj_data_out_ptr->get_max_tangential_pos_num()
			&& A >= proj_data_out_ptr->get_min_axial_pos_num(bin_out.segment_num())
			&& A <= proj_data_out_ptr->get_max_axial_pos_num(bin_out.segment_num())
			)
			{
              (*segments_scatter[segment_index])[view_index][A][tt] = segEmis[view_index][A][tt];
            }
          } // end tang
        } //end view

      } //end else somme!=0/
    }  //end axial


    if (reshapesfcorr == 1 )
    {
      for (int axial_index = emis_data_ptr->get_min_axial_pos_num(segment_index); axial_index <= emis_data_ptr->get_max_axial_pos_num(segment_index); axial_index++)
      {
        for (int view_index = emis_data_ptr->get_min_view_num(); view_index <= emis_data_ptr->get_max_view_num(); view_index++)
        {
          for (int tangential_index = emis_data_ptr->get_min_tangential_pos_num(); tangential_index <= emis_data_ptr->get_max_tangential_pos_num(); tangential_index++)
          {
            if ( tangential_index<emis_data_ptr->get_min_tangential_pos_num()+10 || tangential_index>emis_data_ptr->get_max_tangential_pos_num()-10 )
              (*segments_scatter[segment_index])[view_index][axial_index][tangential_index] = segEmis[view_index][axial_index][tangential_index];
          } // end tang
        } //end view
      } //end axial
    } // end if reshapesfcorr


/*Affichage log */
    //printf("################################\n");
    printf("Factor = %f\n",Factor);
    printf("Profiles (plan-angle) where object was detected = %d \n", compt_obj);
    printf("Mean SF in planes where ogject was found = %f\n", (double)(sftot/compt_obj));
    printf("################################\n");

  } // end of for loop for segment range

  save_and_delete_segments(output, segments_scatter, emis_data_info_ptr->get_min_segment_num(), emis_data_info_ptr->get_max_segment_num(), *proj_data_out_ptr);
  proj_data_out_ptr = 0;

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
