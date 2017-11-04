//
// $Id: conv_to_ecat7.cxx,v 1.6 2004/09/14 13:59:58 kris Exp $
//

/*! 
\file
\ingroup ECAT_utilities
\brief Conversion from interfile (or any format that we can read) 
  to ECAT 7 cti (image and sinogram data)
\author Kris Thielemans
\author PARAPET project
$Date: 2004/09/14 13:59:58 $
$Revision: 1.6 $


This programme is used to convert image or projection data into CTI ECAT 7 data (input 
can be any format currently supported by the library). It normally should be run as 
follows
<pre>conv_to_ecat7 output_ECAT7_name input_filename1 [input_filename2 ...] scanner_name
</pre>
(for images)
<pre>conv_to_ecat7 -s output_ECAT7_name  input_filename1 [input_filename2 ...]
</pre>
(for emission projection data)
<pre>conv_to_ecat7 -a output_ECAT7_name  input_filename1 [input_filename2 ...]
</pre>
(for sinogram-attenuation data)<br>
If there are no command line parameters, the user is asked for the filenames and options 
instead. The data will be assigned a frame number in the 
order that they occur on the command line.<br>
See buildblock/Scanner.cxx for supported scanner names, but examples are ECAT 953, 
ART, Advance. ECAT HR+, etc. If the scanner_name contains a space, the scanner name has to 
be surrounded by double quotes (&quot;) when used as a command line argument.
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2004/09/14 13:59:58 $, IRSL
    See STIR/LICENSE.txt for details
*/

#ifdef HAVE_LLN_MATRIX

#include "stir/DiscretisedDensity.h"
#include "stir/ProjData.h"
#include "stir/shared_ptr.h"
#include "stir/utilities.h"
#include "stir/Succeeded.h"
#include "stir/IO/stir_ecat7.h"

#include <iostream>
#include <vector>
#include <string>

#ifndef STIR_NO_NAMESPACES
using std::cerr;
using std::endl;
using std::vector;
using std::string;
#endif

USING_NAMESPACE_STIR
USING_NAMESPACE_ECAT
USING_NAMESPACE_ECAT7



int main(int argc, char *argv[])
{
  char cti_name[1000], scanner_name[1000] = "";
  vector<string> filenames;
  bool its_an_image = true;
  bool write_as_attenuation = false;
  
  if(argc>=4)
  {
    if (strcmp(argv[1],"-s")==0 || strcmp(argv[1],"-a")==0)
      {
	its_an_image = false;
        write_as_attenuation = strcmp(argv[1],"-a")==0;
        strcpy(cti_name,argv[2]);
        int num_files = argc-3;
        argv+=3;
        filenames.reserve(num_files);
        for (; num_files>0; --num_files, ++argv)
          filenames.push_back(*argv);	
      }
    else 
      {
	its_an_image = true;
	strcpy(cti_name,argv[1]);	
        int num_files = argc-3;
        argv+=2;
        filenames.reserve(num_files);
        for (; num_files>0; --num_files, ++argv)
          filenames.push_back(*argv);	
	strcpy(scanner_name,*argv);
      }
  }  
  else 
  {
    cerr<< "\nConversion from data to ECAT7 CTI.\n"
	<< "Multiples files can be written to a single ECAT 7 file.\n"
        << "The data will be assigned a frame number in the "
        << "order that they occur on the command line.\n\n"
        << "Usage: 3 possible forms depending on data type\n"
	<< "For sinogram data:\n"
	<< "\tconv_to_ecat7 -s output_ECAT7_name orig_filename1 [orig_filename2 ...]\n"
	<< "For sinogram-attenuation data:\n"
	<< "\tconv_to_ecat7 -a output_ECAT7_name orig_filename1 [orig_filename2 ...]\n"
	<< "For image data:\n"
	<< "\tconv_to_ecat7 output_ECAT7_name orig_filename1 [orig_filename2 ...] scanner_name\n"
	<< "scanner_name has to be recognised by the Scanner class\n"
	<< "Examples are : \"ECAT 953\", \"RPT\" etc.\n"
	<< "(the quotes are required when used as a command line argument)\n\n"
	<< "I will now ask you the same info interactively...\n\n";
    
    its_an_image = ask("Converting images?",true);
    if (!its_an_image)
      write_as_attenuation = ask("Write as attenuation data?",false);

    int num_files = ask_num("Number of files",1,10000,1);
    filenames.reserve(num_files);
    char cur_name[max_filename_length];
    for (; num_files>0; --num_files)
    {
      ask_filename_with_extension(cur_name,"Name of the input file? ",its_an_image?".hv":".hs");
      filenames.push_back(cur_name);
    }
    
    ask_filename_with_extension(cti_name,"Name of the ECAT7 file? ",
      its_an_image ? ".img" : ".scn");
  }

  if (its_an_image)
  {

    shared_ptr<Scanner> scanner_ptr = 
      strlen(scanner_name)==0 ?
      Scanner::ask_parameters() :
      Scanner::get_scanner_from_name(scanner_name);

    // read first image
    cerr << "Reading " << filenames[0] << endl;
    shared_ptr<DiscretisedDensity<3,float> > density_ptr =
      DiscretisedDensity<3,float>::read_from_file(filenames[0]);
  
    Main_header mhead;
    make_ECAT7_main_header(mhead, *scanner_ptr, filenames[0], *density_ptr);
    mhead.num_frames = filenames.size();
    mhead.acquisition_type =
      mhead.num_frames>1 ? DynamicEmission : StaticEmission;

    MatrixFile* mptr= matrix_create (cti_name, MAT_CREATE, &mhead);
    if (mptr == 0)
    {
      warning("conv_to_ecat7: error opening output file %s\n", cti_name);
      return EXIT_FAILURE;
    }
    unsigned int frame_num = 1;

    while (1)
    {
      if (DiscretisedDensity_to_ECAT7(mptr,
                                      *density_ptr, 
                                      frame_num)
                                      == Succeeded::no)
      {
        matrix_close(mptr);
        return EXIT_FAILURE;
      }
      if (++frame_num > filenames.size())
      {
        matrix_close(mptr);
        return EXIT_SUCCESS;
      }
      cerr << "Reading " << filenames[frame_num-1] << endl;
      density_ptr =
        DiscretisedDensity<3,float>::read_from_file(filenames[frame_num-1]);
    }
  }
  else 
  {
 
    // read first data set
    cerr << "Reading " << filenames[0] << endl;
    shared_ptr<ProjData > proj_data_ptr =
      ProjData::read_from_file(filenames[0]);
  
    Main_header mhead;
    make_ECAT7_main_header(mhead, filenames[0], *proj_data_ptr->get_proj_data_info_ptr(),
			   write_as_attenuation,
			   NumericType::SHORT);
    mhead.num_frames = filenames.size();
    if (!write_as_attenuation)
      {
	mhead.acquisition_type =
	  mhead.num_frames>1 ? DynamicEmission : StaticEmission;
      }
    MatrixFile* mptr= matrix_create (cti_name, MAT_CREATE, &mhead);
    if (mptr == 0)
    {
      warning("conv_to_ecat7: error opening output file %s\n", cti_name);
      return EXIT_FAILURE;
    }

    unsigned int frame_num = 1;

    while (1)
    {
      if (ProjData_to_ECAT7(mptr,
                            *proj_data_ptr, 
                            frame_num)
                            == Succeeded::no)
      {
        matrix_close(mptr);
        return EXIT_FAILURE;
      }
      if (++frame_num > filenames.size())
      {
        matrix_close(mptr);
        return EXIT_SUCCESS;
      }
      cerr << "Reading " << filenames[frame_num-1] << endl;
      proj_data_ptr =
        ProjData::read_from_file(filenames[frame_num-1]);
    }
  }  
}


#endif

