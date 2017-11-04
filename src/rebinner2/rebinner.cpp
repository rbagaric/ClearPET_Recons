#include <cstdio>
#include <iostream>


#include "geometry.h"
#if USE_GATE_INPUT
#include "gatereader.h"
#elif USE_ROOT_INPUT
#include "rootreader.h"
#else
#include "miniLMFReader.h"
#endif
#include "rebinner.h"
#include "sinogram.h"


Rebinner::Rebinner()
:	isSilent(false)
,	isVerbose(false)
,	totalEventCount(0)
,	goodEventCount(0)
{
	SetFOV(7);
}

void Rebinner::SetFOV(int fov)
{
	minDeltaDet = (ClearPET::numSectors-fov+1)/2;
	maxDeltaDet = (ClearPET::numSectors+fov-1)/2;
}

bool Rebinner::InsideFOV(const CoincEvent& coinc) const
{
	int sector1 = (coinc.id1 & CoincEvent::maskSectors) >> CoincEvent::shiftSectors;
	int sector2 = (coinc.id2 & CoincEvent::maskSectors) >> CoincEvent::shiftSectors;
	int d = std::abs(sector1-sector2);
	return d >= minDeltaDet && d <= maxDeltaDet;
}

///\brief Rebinner run
///
/// This is the real worker.
//
void Rebinner::Run(const std::string& coincFile, int tangentialDimension, int maxCount, const std::string& outputName)
{
	using namespace std;

#if USE_GATE_INPUT
	string coincDataName = coincFile + ".dat";
	Geometry geometry("gate_geometry.cch");
	GateReader coincReader(coincDataName.c_str());
#elif USE_ROOT_INPUT
	string coincDataName = coincFile + ".dat";
	Geometry geometry("gate_geometry.cch");
	RootReader coincReader(coincDataName.c_str());
#else
	string coincHeaderName = coincFile + ".cch";
	string coincDataName = coincFile + ".ccs";
///\todo In violation with the LMF header, we use the same name for cch and ccs file.
/// We should read the name of the data file from the header. This is a bit complicated
/// because we must use the path of the header file, but the name of the data file.
/// Currently, the coinc sorter produces both files with same name, but different extensions,
/// so the code will work.

	Geometry geometry(coincHeaderName);
	MiniLMFReader coincReader(coincDataName.c_str());
#endif
	Sinogram sinogram(geometry, tangentialDimension, isVerbose);

	totalEventCount = 0;
	goodEventCount = 0;

	CoincEvent	coincEvent;
	bool		allEvents = maxCount == 0;
	while (coincReader.ReadEvent(coincEvent))
	{
		++totalEventCount;
		if (InsideFOV(coincEvent) && sinogram.Add(geometry.GetLOR(coincEvent)))
			++goodEventCount;
		if (!allEvents && goodEventCount == maxCount)
			break;
	}

	string headerName = outputName + ".hs";
	string dataName = outputName + ".s";
	sinogram.SaveHeaderFile(headerName, geometry);
	try
	{
		sinogram.SaveDataFile(dataName);
	}
	catch (...)
	{
		remove(headerName.c_str());
		remove(dataName.c_str());
		throw;
	}
}

void Rebinner::Report(std::ostream& os)
{
	using namespace std;

	os << "Rebinner Report:\n";
	os << totalEventCount << " events processed, "
	   << goodEventCount << " counted in sinogram\n";
	os << endl;
}
