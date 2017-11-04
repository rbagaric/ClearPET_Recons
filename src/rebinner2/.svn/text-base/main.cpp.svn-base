#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "clearpet.h"
#include "options.h"
#include "rebinner.h"
#include "version.h"

const char* progTitle = "Rebinner";
const char* progName = "rebinner2";

// Note: regarding the buffer size options, read the comment in filereader.cpp

static
void usage(const char* err)
{
	using namespace std;

	cerr << "Error: " << err;
	cerr << "\n\nusage: " << progName << " options input_file\n";
	cerr << "with the following options:\n";
	cerr << " -f=fov      FOV setting: odd number of opposite cassettes for coincidence check (default 7)\n";
	cerr << " -n=count    number of events to process (all if zero or not specified)\n";
	cerr << " -o=file     name of output file (required)\n";
	cerr << " -s          run silently, with only minimum progress output\n";
	cerr << " -t=tangdim  sinogram tangential dimension, depends on radius and FOV (required)\n";
	cerr << " -v          verbose output, not possible with -s\n";
	cerr << endl;
}

#if _MSC_VER
int __cdecl main(int argc, char **argv )
#else
int main(int argc, char **argv )
#endif
{
	using namespace std;

#ifdef WIN32
	progName = std::strchr(argv[0], '\\');
#else
	progName = std::strchr(argv[0], '/');
#endif
	if (!progName)
		progName = argv[0];
	else
		progName++;

	OptionsParser parser(argc, argv);
	if (!parser.HasOption("s"))
	{
		// Welcome message:
		cout << "ClearPET " << progTitle << " V" << VER_STRING;
		cout << "\n" VER_COPYRIGHT "\n" << endl;
	}

	// set some defaults:
    bool	silent = false;
    bool	verbose = false;
    int		maxCount = 0;
	int		fov = 7;

	// analyze parameters
	string coincFileName;
	if (0 == parser.GetArgumentCount())
	{
		usage("Missing input file");
		return 1;
	} else if (1 != parser.GetArgumentCount())
	{
		usage("Too many input files");
		return 1;
	} else
		coincFileName = parser.GetArgument(0);

	string outputName = parser.GetOption("o");
	if (outputName.empty())
	{
		usage("Missing name of output file (option -o)");
		return 1;
	}

	string s = parser.GetOption("t");
	if (s.empty())
	{
		usage("Missing tangential dimension (option -t)");
		return 1;
	}
	int tangentialDimension = atoi(s.c_str());
	if (tangentialDimension < 50 || tangentialDimension > 500)
	{
		usage("Invalid value of tangential dimension (option -t)");
		return 1;
	}

	s = parser.GetOption("n");
	if (0 != s.length())
		maxCount = atoi(s.c_str());
	if (maxCount < 0)
	{
		usage("Invalid value of option -n");
		return 1;
	}

	s = parser.GetOption("f");
	if (0 != s.length())
		fov = atoi(s.c_str());
	if (fov < 1 || fov > ClearPET::numSectors)
	{
		usage("Invalid value of option -f");
		return 1;
	}
	if (0 == (fov & 1) && fov < ClearPET::numSectors)
	{
		usage("Value of option -f must be an odd number");
		return 1;
	}

	if (parser.HasOption("s"))
		silent = true;
	if (parser.HasOption("v"))
		verbose = true;
	if (silent && verbose)
	{
		usage("Cannot be silent and verbose (-s and -v)");
		return 1;
	}

	Rebinner rebinner;
	rebinner.SetNoise(silent, verbose);
	rebinner.SetFOV(fov);

	try {
		time_t t1, t2;
        std::time(&t1);
		rebinner.Run(coincFileName, tangentialDimension, maxCount, outputName);
		std::time(&t2);
		if (!silent)
		{
			rebinner.Report(cout);
			cout << "Rebinning duration: " << (t2-t1) << "s" << endl;
		}
	}
	catch (std::exception& e)
	{
		cerr << e.what();
		cerr << "\nProgram aborted" << endl;
		return 1;
	}
	catch (...) {
		cerr << "Unknown C++ exception";
		cerr << "\nProgram aborted" << endl;
		return 1;
	}

	return 0;
}
