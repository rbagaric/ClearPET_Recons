#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cstring>

#include "options.h"
#include "sorter.h"
#include "version.h"

const char* progTitle = "Coincidence Sorter";
const char* progName = "sorter2";

// Note: regarding the buffer size options, read the comment in filereader.cpp

static
void usage(const char* err)
{
	using namespace std;

//	cout << "Test: sizeof(off_t) is " << sizeof(off_t) << "\n\n";

	cerr << "Error: " << err;
	cerr << "\n\nusage: " << progName << " options input_files\n";
	cerr << "with 1..20 input file names and the following options:\n";
	cerr << " -s		   run silently, with only minimum progress output\n";
	cerr << " -a=angName   name of angular encoder file - no default\n";
	cerr << " -o=outName   name of output file - no default\n";
	cerr << " -c=coincWin  coincidence window in ns [1..1000] - default=20ns\n";
	cerr << " -n=numCass   number of cassettes [1..20] - default=20\n";
	cerr << " -f=FOV       field of view, by (odd) number of opposite cassettes - default=7\n";
	cerr << " -m=matrix    name of matrix file used to check allowed coincidences\n";
	cerr << " -rfrom=n     number of first frame to be processed - default=1(first)\n";
	cerr << " -rto=n       number of last reset frame to be processed - default=0(all)\n";
#if !INPUT_32BIT
	cerr << " -emin=erg    minimum energy [keV] of valid events - default=350keV\n";
	cerr << " -emax=erg    maximum energy [keV] of valid events - default=750keV\n";
#endif
#if DELAYED_COINC_CHECK
	cerr << " -t=delay     delay time [ns] for delayed coincidence generation - default=none\n";
#endif
    cerr << " -b=bufSize   calculation buffer size, in units of 1kB - default=2048kB\n";
	cerr << " -B=bufSize   file io buffer size, in units of 1kB - default=4kB\n";
    cerr << " -R=radius    Inner ring radius, in mm - default=135mm\n";
    cerr << " -A=offset    angular offset, in degrees - default=0\n";
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
#ifdef __GNUC__
		cout << " (GNU C++ compiled)";
#elif __MWERKS__
		cout << " (MW CodeWarrior compiled)";
#elif _MSC_VER
		cout << " (VC++ compiled)";
#endif
#if INPUT_32BIT
		cout << "\nfor 32-bit single event files\n";
#else
		cout << "\nfor 64-bit single event files\n";
#endif
		cout << VER_COPYRIGHT "\n" << endl;
	}

	// set some defaults:
	int coincWin = 20;
	int numCass = 20;
	int numFOV = 0;
	int firstReset = 1;
	int lastReset = 0;
	int minErg = 350;
	int maxErg = 750;
	int calcBufSize = 2048;
	int ioBufSize = 4;
    double  radius = 135.6;
    double	angularOffset = 0.0;
#if DELAYED_COINC_CHECK
	int randomDelay = 0;
#endif
    bool	silent = false;

	if (0 == parser.GetArgumentCount())
	{
		usage("Missing input files");
		return 1;
	}

	string outFileName = parser.GetOption("o|out");
	if (0 == outFileName.length())
	{
		usage("Missing name of output file (option -o or -out)");
		return 1;
	}

	string encoderFileName = parser.GetOption("a");
	if (0 == encoderFileName.length())
	{
		usage("Missing name of angular encoder file (option -a)");
		return 1;
	}

	string matrixFileName = parser.GetOption("m");

	if (parser.HasOption("s"))
		silent = true;

	std::string s = parser.GetOption("c|coinc");
	if (0 != s.length())
		coincWin = atoi(s.c_str());

	s = parser.GetOption("n|numcass");
	if (0 != s.length())
		numCass = atoi(s.c_str());
	if (numCass < 1 || numCass > Sorter::MaxCassCount)
	{
		usage("Invalid number of cassettes (option -n or -numcass)");
		return 1;
	}
	if (parser.GetArgumentCount() > numCass)
	{
		// More input file than cassettes, that's impossible
		usage("Too many input files");
		return 1;
	}

	s = parser.GetOption("f|fov");
	if (0 != s.length())
		numFOV = atoi(s.c_str());

	s = parser.GetOption("rfrom");
	if (0 != s.length())
	{
		firstReset = atoi(s.c_str());
		if (firstReset < 1)
		{
			usage("first reset number must be larger than zero (option -nfrom)");
			return 1;
		}
	}

	s = parser.GetOption("rto");
	if (0 != s.length())
		lastReset = atoi(s.c_str());
	if (lastReset != 0 && lastReset < firstReset)
	{
		usage("first reset number must be less or equal than last reset number");
		return 1;
	}

	// TODO: check the limits of arguments. When changing, also change
	// the text in usage().

#if !INPUT_32BIT
	s = parser.GetOption("emin");
	if (0 != s.length())
	{
		minErg = atoi(s.c_str());
		if (minErg < 0 || minErg > 1250)
		{
			usage("Invalid minimum energy (option -emin)");
			return 1;
		}
	}

	s = parser.GetOption("emax");
	if (0 != s.length())
	{
		maxErg = atoi(s.c_str());
		if (maxErg < minErg || maxErg > 1250)
		{
			usage("Invalid maximum energy (option -emax)");
			return 1;
		}
	}
#endif

	s = parser.GetOption("b");
	if (0 != s.length())
	{
		calcBufSize = atoi(s.c_str());
		if (calcBufSize < 1 || calcBufSize > 10*1024)
		{
			usage("Invalid calculation buffer size (option -b)");
			return 1;
		}
	}

	s = parser.GetOption("B");
	if (0 != s.length())
	{
		ioBufSize = atoi(s.c_str());
		if (ioBufSize < 1 || ioBufSize > 10*1024)
		{
			usage("Invalid IO buffer size (option -B)");
			return 1;
		}
	}

	s = parser.GetOption("R");
	if (0 != s.length())
	{
        std::istringstream is (s);
        is >> radius;
		if (radius < 100 || radius > 300)
		{
			usage("Invalid inner radius (option -R). Allowed range is 100..300mm.");
			return 1;
		}
	}

	s = parser.GetOption("A");
	if (0 != s.length())
	{
        std::istringstream is (s);
        is >> angularOffset;
		if (angularOffset < -180 || angularOffset > 180)
		{
			usage("Invalid angualr offset (option -A). Allowed range is -180..180 degrees.");
			return 1;
		}
	}

	if (coincWin < 1 || coincWin > 1000)
	{
		usage("Invalid coincidence window (option -c or -coinc)");
		return 1;
	}

#if DELAYED_COINC_CHECK
	s = parser.GetOption("t");
	if (0 != s.length())
	{
		randomDelay = atoi(s.c_str());
		if (randomDelay < 0 || randomDelay > 10000)
		{
			usage("Invalid random check delay time (option -t)");
			return 1;
		}
		// Note: we allow the delay to be smaller than the coincidence window.
		// This is useful for software validation. In order to avoid mistakes,
		// we emit a warning in this case.
		if (randomDelay > 0 && randomDelay <= coincWin)
			cout << "Warning: random coincidence delay is not larger than coinc window!" << endl;
	}
#endif

	if (0 != matrixFileName.length() && 0 != numFOV)
	{
		usage("Only one of the option -f or -m may be used");
		return 1;
	} else if (0 == matrixFileName.length() && 0 == numFOV)
		// use default
		numFOV = 7;

	FileBuffer::SetBufferSizes(calcBufSize, ioBufSize);
	Sorter sorter(numCass, coincWin);
	if (numFOV)
	{
		if (!sorter.SetFOV(numFOV)) {
			usage("Invalid field of view (option -f or -fov)");
			return 1;
		}
	} else
	{
		try { sorter.SetMatrix(matrixFileName); }
		catch (std::exception& e)
		{
			cerr << e.what() << std::endl;
			return 1;
		}
	}
	if (!sorter.SetEnergyWindow(minErg, maxErg))
	{
		usage("Invalid energy window (option -minErg and/or -maxErg)");
		return 1;
	}
	sorter.SetSilent(silent);
#if DELAYED_COINC_CHECK
	sorter.SetRandomDelay(randomDelay);
#endif
    sorter.SetRadius(radius);
    sorter.SetAngularOffset(angularOffset);

	for (int i = 0; i < parser.GetArgumentCount(); ++i)
	{
		if (!sorter.AddInputFile(parser.GetArgument(i)))
		{
			usage("Duplicate input file");
			return 1;
		}
	}

	try {
		time_t t1, t2;
        std::time(&t1);
		sorter.Sort(encoderFileName, outFileName, firstReset, lastReset);
		std::time(&t2);
		if (!silent)
		{
			sorter.Report(cout);
			cout << "Sorting duration: " << (t2-t1) << "s" << endl;
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
