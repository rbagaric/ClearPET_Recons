#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <math.h>
#include <stdexcept>

#include "options.h"
#include "version.h"

const char* progTitle = "Cylindrical mask";
const char* progName = "mask_cyl";

static
void usage(const char* err)
{
	using namespace std;

	cerr << "Error: " << err;
	cerr << "\n\nusage: " << progName << " options input_file\n";
	cerr << "with the following options:\n";
	cerr << " -s		   run silently, with only minimum progress output\n";
	cerr << " -R=radius    radius of cylinder in mm\n";
	cerr << endl;
}

void Process(const std::string& name, double radius, bool silent)
{
	using namespace std;

	// open input file, and extract inforamtions:
	ifstream header(name.c_str());
	string line;
	if (!getline(header, line))
		throw runtime_error("Error reading input file");
	if (!line.compare(0, 11, string("!INTERFILE")))
		throw runtime_error("Invalid file format of input file");

	int nx = 0;
	int ny = 0;
	int nz = 0;
	int offs = 0;
	double scale = 0.0;
	string datafile;
	while (getline(header, line))
	{
		size_t p = line.find(":=");
		if (p == string::npos)
			continue;
		int e = p-1;
		while (e >= 0 && isspace(line[e]))
			--e;
		if (e < 0)
			continue;
		int s = 0;
		while (isspace(line[s]))
			++s;
		string key = line.substr(s, e-s+1);
		s = p+2;
		while (s < line.length() && isspace(line[s]))
		{
			++s;
		}
		e = line.length()-1;
		while (e > s && isspace(line[e]))
			--e;
		string value = line.substr(s, e-s+1);

		if (key == "!matrix size [1]")
			nx = atoi(value.c_str());
		else if (key == "!matrix size [2]")
			ny = atoi(value.c_str());
		else if (key == "!matrix size [3]")
			nz = atoi(value.c_str());
		else if ((key == "scaling factor (mm/pixel) [1]")
			|| (key == "scaling factor (mm/pixel) [2]")
			|| (key == "scaling factor (mm/pixel) [3]"))
		{
			double sc;
			istringstream ss(value);
			ss >> sc;
			if (!ss || sc < 0.001 || sc > 10)
				throw runtime_error("Invalid pixel scaling factor");
			if (scale == 0.0)
				scale = sc;
			else if (fabs(scale-sc)/scale > 0.01)
				throw runtime_error("image does nont have uniform pixel scaling factors");
		}
		else if (key == "!matrix size [3]")
			nz = atoi(value.c_str());
		else if (key == "!matrix size [3]")
			nz = atoi(value.c_str());
		else if (key == "name of data file")
			datafile = value;
		else if (key == "data offset in bytes[1]")
			offs = atoi(value.c_str());
		else if (key == "number of dimensions")
		{
			if (value != "3")
				throw runtime_error("Invalid data format");
		}
		else if (key == "!number format")
		{
			if (value != "float")
				throw runtime_error("Invalid data format");
		}
		else if (key == "!number of bytes per pixel")
		{
			if (value != "4")
				throw runtime_error("Invalid data format");
		}
	}

	header.close();
	if (nx < 10 || nx > 1000
	|| ny < 10 || ny > 1000
	|| nz < 10 || nz > 1000
	|| scale == 0.0
	|| datafile.empty())
		throw runtime_error("Invalid contents of input file");

	if (!silent)
		cout << "Processing file " << datafile
			<< " for nx=" << nx << " ny=" << ny << " nz=" << nz
			<< " pixsize=" << scale << "mm radius=" << radius << "mm" << endl;

	float* buf = new float[nx*ny*nz];
	fstream data(datafile.c_str(), ios::in|ios::out);
	if (!data)
		throw runtime_error("Error opening data file");
	data.seekg(offs);
	data.read(reinterpret_cast<char*>(buf), nx*ny*nz*sizeof(float));
	if (data.bad())
	{
		delete[] buf;
		throw runtime_error("Error reading data file");
	}

	// erase all pixels where X*X+Y*Y >= R*R
	float* p = buf;
	radius /= scale;
	double r2 = radius*radius;
	double x0 = nx/2.0;
	double y0 = ny/2.0;
	for (int z = 0; z < nz; ++z)
	{
		for (int y = 0; y < ny; ++y)
		{
			double y2 = (y-y0)*(y-y0);
			for (int x = 0; x < nx; ++x)
			{
				if ((x-x0)*(x-x0)+y2 >= r2)
					*p = 0.0;
				++p;
			}
		}
	}

	data.seekp(offs);
	data.write(reinterpret_cast<char*>(buf), nx*ny*nz*sizeof(float));
	if (data.bad())
	{
		delete[] buf;
		throw runtime_error("Error writing data file");
	}

	// cleanup
	delete[] buf;
	if (!silent)
		cout << "Complete" << endl;
}




#if _MSC_VER
int __cdecl main(int argc, char **argv )
#else
int main(int argc, char **argv )
#endif
{
	using namespace std;

#ifdef WIN32
	progName = strchr(argv[0], '\\');
#else
	progName = strchr(argv[0], '/');
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
#elif _MSC_VER
		cout << " (VC++ compiled)";
#endif
		cout << "\n" VER_COPYRIGHT "\n" << endl;
	}

	// set some defaults:
	double radius = 0;
    bool	silent = false;

	if (0 == parser.GetArgumentCount())
	{
		usage("Missing input file");
		return 1;
	}
	else if (1 != parser.GetArgumentCount())
	{
		usage("Too many input files");
		return 1;
	}

	std::string s = parser.GetOption("R");
	if (0 != s.length())
	{
		istringstream ss(s);
		ss >> radius;
		if (!ss || radius < 1 || radius >1000)
		{
			usage("Invalid radius with option -R");
			return 1;
		}
	}
	silent = parser.HasOption("s");

	try
	{
		Process(parser.GetArgument(0), radius, silent);
	}
	catch (std::exception& e)
	{
		cerr << e.what();
		cerr << "\nProgram aborted" << endl;
		return 1;
	}
	catch (...)
	{
		cerr << "Unknown C++ exception";
		cerr << "\nProgram aborted" << endl;
		return 1;
	}

	return 0;
}
