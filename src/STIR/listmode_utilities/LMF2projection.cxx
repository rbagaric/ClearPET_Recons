//
// $Id: LMF_projection.cxx
//
/*!
  \file 
  \ingroup ClearPET_utilities

  \brief  
 
  \author Monica Vieira Martins
  \author Christian Morel
  
  \date $Date:  $
  \version $Revision $
*/
// LMF sinogram builder   

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


#ifdef USE_SegmentByView
#include "stir/ProjDataInterfile.h"
#include "stir/SegmentByView.h"
#else
#include "stir/ProjDataFromStream.h"
#include "stir/IO/interfile.h"
#include "stir/Array.h"
#include "stir/IndexRange3D.h"
#endif

//typedef float elem_type;

#include <fstream>
#include <iostream>
#include <vector>
#include "stir/utilities.h"
#include "stir/IO/interfile.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lmf.h" 
#include "stir/listmode/LMF_ClearPET.h"
#include "stir/listmode/LMF_Interfile.h"
#include "stir/Scanner.h"
#include "stir/SegmentByView.h"
#include "stir/ProjDataFromStream.h"
#include "stir/ProjDataInfoCylindricalArcCorr.h"
#include "stir/ProjDataInfo.h"
#include "stir/CPUTimer.h"
#include "stir/Array.h"
#include "stir/IndexRange3D.h"

#ifndef STIR_NO_NAMESPACES
using std::fstream;
using std::ifstream;
using std::iostream;
using std::ofstream;
using std::streampos;
using std::cin;
using std::cout;
using std::cerr;
using std::flush;
#endif

USING_NAMESPACE_STIR

#ifdef USE_SegmentByView
typedef SegmentByView<elem_type> segment_type;
#else
#error does not work at the moment
#endif

// Normal Tangent Reference Frame (NTRF)
struct NTRF {
  int radial, tangential, axial;
};

// Scanner topology (ST)
struct SCANNER_TOPOLOGY {
  NTRF rsectors, modules, submodules, crystals, layers;
};
struct SCANNER_CHECK_LIST {
  SCANNER_TOPOLOGY scanTopo;
  double radius, det_size, ring_spacing;
  int nb_det_per_ring, nb_rings;
};

float get_numerical_contentLMFdata(char info[charNum]);

REBINNING_INFOS set_RebinningInfos(REBINNING_INFOS rebinning_infos, SCANNER_CHECK_LIST scanCheckList);

SCANNER_CHECK_LIST fill_ScannerCheckList(SCANNER_CHECK_LIST scanCheckList, LMF_ccs_encodingHeader *pEncoH);

int check_ScannerCheckLists(int check, SCANNER_CHECK_LIST normCheckList, SCANNER_CHECK_LIST scanCheckList);

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



int main(int argc, char * argv[])
{
  
  if (argc < 2) {    
    cerr << "Usage: " << argv[0] << " [outfilename] infilename\n";    
    exit(EXIT_FAILURE);
  }
  
  const string output_filename = argv[1];  
  // adding the extension ".s"  to the output filename
  char *output_file_name=new char [max_filename_length];
  strcpy(output_file_name,output_filename.c_str());
  replace_extension(output_file_name,".s");
  shared_ptr<iostream> output = new fstream (output_file_name, ios::out|ios::binary); 
  if (!*output)
    error("Error opening output file %s\n",output_file_name);

  string input_filename = argv[1];
  char *input_file_name = argv[1];
  if (argc == 3) {
    input_filename = argv[2];
    input_file_name = argv[2];
  }
  // adding the extension ".ccs"  to the output filename
  char *nameOfFileCCS = new char [max_filename_length];
  strcpy(nameOfFileCCS,input_filename.c_str());
  cout << "\nscan_file" << flush;
  replace_extension(nameOfFileCCS,".ccs");

  //definition needed to read ccs header
  static struct LMF_ccs_encodingHeader *pEncoH = NULL;
            
  //opening and reading file.cch, filling in structure LMF_cch  
  if(LMFcchReader(input_file_name)) exit(EXIT_FAILURE);
  
  //opening file.ccs
  FILE *pfCCS=NULL;                                
  pfCCS = open_CCS_file2(nameOfFileCCS);  /* open the LMF binary file */    
  fseek(pfCCS,0L,0);                      /* find the begin of file */
  //allocate and fill in the encoding header structure 
  pEncoH = readHead(pfCCS);

  //fill scanner check list
  SCANNER_CHECK_LIST scanCheckList = fill_ScannerCheckList(scanCheckList, pEncoH);

  //setting REBINNING_INFOS
  REBINNING_INFOS rebinning_infos = {0};
  rebinning_infos = set_RebinningInfos(rebinning_infos, scanCheckList);
  
  //define ring radius 
  float ring_radius = rebinning_infos.ring_radius;

  //define ring spacing
  float ring_spacing = rebinning_infos.ring_spacing;

  //define number of views
  int nbphi =rebinning_infos.nbphi;

  //define bin size
  float dxyr = rebinning_infos.bin_size;

  //define number of rings
  int max_num_of_axial_pos = rebinning_infos.nbz;
  int nbtheta = rebinning_infos.nbtheta;
  int max_num_of_bins = rebinning_infos.max_num_of_bins;
  const bool interactive = ask("Output coords to stdout",false);
   
  cout << "\nComputing sinograms by analytical method\n"; 
  double  dtheta = atan(rebinning_infos.ring_spacing/(2*rebinning_infos.ring_radius));
  rebinning_infos.dtheta = dtheta;
  ProjDataFromStream* proj_data_from_stream_ptr =  create_ClearPET_PDFS(output,
									output->tellp(),
									ring_radius,
									nbphi, 
									nbtheta,
									dtheta, 
									dxyr,
									ring_spacing,
									max_num_of_axial_pos, 
									max_num_of_bins,false);

  // get proj_data_info for use in the rebinning below
  const ProjDataInfoCylindrical * proj_data_info_ptr =
    dynamic_cast<const ProjDataInfoCylindrical *>(proj_data_from_stream_ptr->get_proj_data_info_ptr());
  

  //*********** open output file
  /*string template_proj_data_name = "seg_15.hs";
  shared_ptr<ProjDataInfo> template_proj_data_info_ptr;

  shared_ptr<ProjData> template_proj_data_ptr =
    ProjData::read_from_file(template_proj_data_name);

  template_proj_data_info_ptr = 
    template_proj_data_ptr->get_proj_data_info_ptr()->clone();

  shared_ptr<iostream> output_cp;
  shared_ptr<ProjData> proj_data_ptr;

  {

    const string output_filename = "titi";
    
    proj_data_ptr = 
      construct_proj_data(output_cp, output_filename, template_proj_data_info_ptr);
  }*/


  assert(proj_data_info_ptr != NULL);  
  assert(proj_data_info_ptr->get_tantheta(Bin(0,0,0,0)) != 0);
  
  write_interfile_header_for_ClearPET_sinogram(output_file_name,proj_data_from_stream_ptr);
  delete output_file_name;
        
  int num_segments =  proj_data_from_stream_ptr->get_num_segments();
  VectorWithOffset<segment_type *> segments (proj_data_from_stream_ptr->get_min_segment_num(), 
					     proj_data_from_stream_ptr->get_max_segment_num());

  VectorWithOffset<segment_type *> norm_segments (proj_data_from_stream_ptr->get_min_segment_num(), 
					     proj_data_from_stream_ptr->get_max_segment_num());

  const int num_segments_in_memory = ask_num("Number of segments in memory ", 0, num_segments, num_segments);
    
  // Finally, do the real work  
  CPUTimer timer;
  timer.start();
  int ievent = 0;
  int ievent_stored = 0;

  //Here starts the main loop which will store the  data.
  //For each start_segment_index, we check which events occur in the
  // segments between start_segment_index and start_segment_index+num_segments_in_memory.    
  for (int start_segment_index = proj_data_from_stream_ptr->get_min_segment_num(); 
       start_segment_index <= proj_data_from_stream_ptr->get_max_segment_num(); 
       start_segment_index += num_segments) {
    
    cout << "\nProcessing segments: " << endl;
    
    const int end_segment_index = std::min( proj_data_from_stream_ptr->get_max_segment_num()+1, 
					    start_segment_index + num_segments_in_memory) - 1;
    
    allocate_segments(segments, start_segment_index, end_segment_index,proj_data_from_stream_ptr->get_proj_data_info_ptr());

    int tang_pos_x=0;
    int axial_pos_z=0;
    int azimuthal_angle=0;
    int co_polar_angle =0;
    double x1,y1,z1,x2,y2,z2;
    
    int max_nb_events_stored = ask_num("Max nb of events to store (0 means all) ", 0, 100000000, 0);

    while(findXYZinLMFfile(pfCCS,&x1,&y1,&z1,&x2,&y2,&z2,pEncoH)){
      //cout << "\nx1= " << x1 << " x2= " << x2 << " y1= " << y1 << " y2=  " << y2 << " z1= " << z1 << " z2= " << z2  << flush;
      
      interfile_analytical(x1,y1,z1,x2,y2,z2, rebinning_infos,&tang_pos_x, &axial_pos_z, &azimuthal_angle,&co_polar_angle, *proj_data_info_ptr);	
      const int seg = co_polar_angle ;
      
      if (interactive) {      
	printf("\nSeg %4d view %4d ax_pos %4d tang_pos %4d ", seg, azimuthal_angle, axial_pos_z, tang_pos_x);
	printf("photons: 1: (%.2f,%.2f,%.2f) 2: (%.2f,%.2f,%.2f) ", x1, y1, z1, x2, y2, z2);
      }         
      
      if(check_range(seg, start_segment_index, end_segment_index,tang_pos_x,axial_pos_z,azimuthal_angle,proj_data_from_stream_ptr))
	{	    	
	  if (interactive) cout << "stored " << ievent << " event" << flush;
	  ievent++;
	  ievent_stored ++;
	  if (ievent%50000==0) cout << "\r" << ievent << flush;
	  (*segments[seg])[azimuthal_angle][axial_pos_z][tang_pos_x] += 1;
	  if ((max_nb_events_stored !=0)&&(ievent_stored >= max_nb_events_stored)) break;
      }	
      else
	{
	  if (interactive) printf("ignored");
	  ievent++;
	}
    }
    save_and_delete_segments(output, segments, start_segment_index, end_segment_index, *proj_data_from_stream_ptr);  
  }

  cout << "\n" << flush;
  
  if(pfCCS) {
    fclose(pfCCS);
    printf("Scan file CCS closed\n");
  }

  LMFcchReaderDestructor();
  destroyReadHead();//ccsReadHeadDestructor
  destroy_findXYZinLMFfile(pEncoH);
  (*output).flush();
  timer.stop();
  
  cout  << "\nThis took " << timer.value() << "s CPU time." << endl;
  cout  << ievent_stored << " events stored out of " << ievent << " events treated." << endl;
  
  return EXIT_SUCCESS;
}

//gets numericalValue from struct contentLMFdata
float get_numerical_contentLMFdata(char info[charNum])

{
  char field[charNum];
  contentLMFdata structData = {0};
  initialize(field);  
  strcpy(field,info);
  structData = getLMF_cchNumericalValue(field);
  return structData.numericalValue;
}


//searches for  information in file.ccs and in file.cch. Fills in REBINNING INFOS
REBINNING_INFOS set_RebinningInfos(REBINNING_INFOS rebinning_infos, SCANNER_CHECK_LIST scanCheckList)
{
  //check rotation
  double azimuth = get_numerical_contentLMFdata("azimuthal step");
  cout << "\nAzimuthal step: " << azimuth << flush;

  //ring radius
  double radius = scanCheckList.radius;
  rebinning_infos.ring_radius = ask_num("Ring radius ",0.,2.*radius,radius);

  //number of views
  rebinning_infos.nbphi = scanCheckList.nb_det_per_ring/2;
  int nbphi = (int) ask_num("Number of azimuthal positions ",rebinning_infos.nbphi/2.,rebinning_infos.nbphi*4.,rebinning_infos.nbphi/1.);
  rebinning_infos.nbphi = nbphi;

  //bin size  
  double sampling_bin_size = scanCheckList.det_size;
  cout << "\nTangential detector size: " << sampling_bin_size << flush;
  double geometric_bin_size = _PI*rebinning_infos.ring_radius/rebinning_infos.nbphi;
  rebinning_infos.bin_size = ask_num("Bin size ",0.,geometric_bin_size*2.,sampling_bin_size/2.);

  //number of rings
  rebinning_infos.nbz = scanCheckList.nb_rings;

  //ring spacing
  double ring_spacing = scanCheckList.ring_spacing;
  rebinning_infos.ring_spacing = ask_num("Ring spacing ", 0., 100., ring_spacing);
 

  //number of segments
  int maxNumberOfSegments = 2*scanCheckList.nb_rings-1;
  rebinning_infos.nbtheta = ask_num("Number of segments (even number) ", 0, maxNumberOfSegments , maxNumberOfSegments);

  //number of bins
  int maxNumberOfTangentialPos = (int) (rebinning_infos.ring_radius*2./rebinning_infos.bin_size);
  int default_maxNumberOfTangentialPos = 2*maxNumberOfTangentialPos/3;
  rebinning_infos.max_num_of_bins = ask_num("Number of tangential positions ", 0, maxNumberOfTangentialPos, default_maxNumberOfTangentialPos);
  
  return rebinning_infos;
}

//check topology consisitency between encoding headers of scan file and normalisation file
SCANNER_CHECK_LIST fill_ScannerCheckList(SCANNER_CHECK_LIST scanCheckList, LMF_ccs_encodingHeader *pEncoH)
{
  //fill scanner topology
  scanCheckList.scanTopo.rsectors.tangential = pEncoH->scannerTopology.numberOfSectors;
  scanCheckList.scanTopo.rsectors.axial = pEncoH->scannerTopology.numberOfRings;
  scanCheckList.scanTopo.modules.tangential = pEncoH->scannerTopology.tangentialNumberOfModules;
  scanCheckList.scanTopo.modules.axial = pEncoH->scannerTopology.axialNumberOfModules;
  scanCheckList.scanTopo.submodules.tangential = pEncoH->scannerTopology.tangentialNumberOfSubmodules;
  scanCheckList.scanTopo.submodules.axial = pEncoH->scannerTopology.axialNumberOfSubmodules;
  scanCheckList.scanTopo.crystals.tangential = pEncoH->scannerTopology.tangentialNumberOfCrystals;
  scanCheckList.scanTopo.crystals.axial = pEncoH->scannerTopology.axialNumberOfCrystals;
  scanCheckList.scanTopo.layers.radial = pEncoH->scannerTopology.radialNumberOfLayers;

  //fill scanner parameters
  scanCheckList.radius = get_numerical_contentLMFdata("ring diameter")/2;
  scanCheckList.det_size = get_numerical_contentLMFdata("crystal tangential pitch");
  scanCheckList.ring_spacing = get_numerical_contentLMFdata("crystal axial pitch");
  scanCheckList.nb_det_per_ring = pEncoH->scannerTopology.numberOfSectors;
  scanCheckList.nb_det_per_ring *=  pEncoH->scannerTopology.tangentialNumberOfModules;
  scanCheckList.nb_det_per_ring *= pEncoH->scannerTopology.tangentialNumberOfSubmodules;
  scanCheckList.nb_det_per_ring *= pEncoH->scannerTopology.tangentialNumberOfCrystals;

  scanCheckList.nb_rings = pEncoH->scannerTopology.numberOfRings;
  scanCheckList.nb_rings *= pEncoH->scannerTopology.axialNumberOfModules;
  scanCheckList.nb_rings *= pEncoH->scannerTopology.axialNumberOfSubmodules;
  scanCheckList.nb_rings *= pEncoH->scannerTopology.axialNumberOfCrystals;

  return scanCheckList;
}

int check_ScannerCheckLists(int check, SCANNER_CHECK_LIST normCheckList, SCANNER_CHECK_LIST scanCheckList)
{
  check = 1;
 
  //check topology consistency
  if (normCheckList.scanTopo.rsectors.tangential != scanCheckList.scanTopo.rsectors.tangential) {
    check = 0;
    cout << "Norm and scan files have different numbers of tangential rsectors\n" << flush;
  }  
  if (normCheckList.scanTopo.rsectors.axial != scanCheckList.scanTopo.rsectors.axial) {
    check = 0;
    cout << "Norm and scan files have different numbers of axial rsectors\n" << flush;
  }  
  if (normCheckList.scanTopo.modules.tangential != scanCheckList.scanTopo.modules.tangential) {
    check = 0;
    cout << "Norm and scan files have different numbers of tangential modules\n" << flush;
  }  
  if (normCheckList.scanTopo.modules.axial != scanCheckList.scanTopo.modules.axial) {
    check = 0;
    cout << "Norm and scan files have different numbers of axial modules\n" << flush;
  }  
  if (normCheckList.scanTopo.submodules.tangential != scanCheckList.scanTopo.submodules.tangential) {
    check = 0;
    cout << "Norm and scan files have different numbers of tangential submoduless\n" << flush;
  }  
  if (normCheckList.scanTopo.submodules.axial != scanCheckList.scanTopo.submodules.axial) {
    check = 0;
    cout << "Norm and scan files have different numbers of axial submodules\n" << flush;
  }  
  if (normCheckList.scanTopo.crystals.tangential != scanCheckList.scanTopo.crystals.tangential) {
    check = 0;
    cout << "Norm and scan files have different numbers of tangential crystals\n" << flush;
  }  
  if (normCheckList.scanTopo.crystals.axial != scanCheckList.scanTopo.crystals.axial) {
    check = 0;
    cout << "Norm and scan files have different numbers of axial crystals\n" << flush;
  }  
  if (normCheckList.scanTopo.layers.radial != scanCheckList.scanTopo.layers.radial) {
    check = 0;
    cout << "Norm and scan files have different numbers of radial layers\n" << flush;
  }

  //check parameters consistency
  if (normCheckList.radius != scanCheckList.radius) {
    check = 0;
    cout << "Norm and scan files have different radii\n" << flush;
  }
  if (normCheckList.det_size != scanCheckList.det_size) {
    check = 0;
    cout << "Norm and scan files have different tangential detector size\n" << flush;
  }
  if (normCheckList.ring_spacing != scanCheckList.ring_spacing) {
    check = 0;
    cout << "Norm and scan files have different ring spacings\n" << flush;
  }
  if (normCheckList.nb_det_per_ring != scanCheckList.nb_det_per_ring) {
    check = 0;
    cout << "Norm and scan files have different numbers of detectors per ring\n" << flush;
  }
  if (normCheckList.nb_rings != scanCheckList.nb_rings) {
    check = 0;
    cout << "Norm and scan files have different numbers of rings\n" << flush;
  }
  
  return check;
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
    segments[seg] = new SegmentByView<elem_type>(
    	proj_data_info_ptr->get_empty_segment_by_view (seg)); 
#else
    segments[seg] = 
      new Array<3,elem_type>(IndexRange3D(0, proj_data_info_ptr->get_num_views()-1, 
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


