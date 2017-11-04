/// \file	GateReader.cpp
/// \brief	Implementation of class GateReader
/// \author	Axel Wagner
///

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <string>

#include "clearpet.h"
#include "coincevent.h"
#include "gatereader.h"

/// Returns a detector ID for the encoding rule used for the
/// commercial ClearPET scanner.
/// See LMF specs for details.
inline
uint16_t MakeID(int sector, int mod, int subMod, int pixel, int layer)
{
	int result = sector;
	result = (result << CoincEvent::bitsMods) + mod;
	result = (result << CoincEvent::bitsSubMods) + subMod;
	result = (result << CoincEvent::bitsPixels) + pixel;
	result = (result << CoincEvent::bitsLayers) + layer;
	return uint16_t(result);
}

GateReader::GateReader(const char* pFileName)
:	file(pFileName, std::ios::in)
,	filename(pFileName)
,	lineNo(0)
{
    if (!file)
        throw std::runtime_error(std::string("error opening ") + filename);
}

GateReader::~GateReader()
{
}


/// Read one event from the Gate coincidence file.
/// See Gate manual (chapter 10.1.4 "Description of the ASCII file content") for details.
bool GateReader::ReadEvent(CoincEvent& coincEvent)
{
	std::string line;
	if (!getline(file, line))
		return false;
	++lineNo;

	double timeStamp, angularPosition;
	int		rsector, submod, mod, pixel, layer;

	std::istringstream is(line);
	
	double	d1, d2, d3, e;
	int		i1, i2, i3, i4;
	is >> i1 >> i2 >> i3 >> d1 >> d2 >> d3;
	is >> timeStamp;
	is >> e >> d1 >> d2 >> d3;
	is >> i1 >> rsector >> submod >> mod >> pixel >> layer;
	is >> i1 >> i2 >> i3 >> i4;
	is >> d1 >> angularPosition;
	coincEvent.id1 = MakeID(rsector, mod, submod, pixel, layer);
	angularPosition = 360.0 - angularPosition;
	while (angularPosition < 0)
		angularPosition += 360.0;
	coincEvent.angle = static_cast<uint16_t>(angularPosition * 10);
	// debugging:
//	std::cout << "Evt1: " << rsector << " " << mod << " " << pixel << " " << layer << "\n";

	is >> i1 >> i2 >> i3 >> d1 >> d2 >> d3;
	is >> timeStamp;
	is >> e >> d1 >> d2 >> d3;
	is >> i1 >> rsector >> submod >> mod >> pixel >> layer;
	is >> i1 >> i2 >> i3 >> i4;
	is >> d1 >> angularPosition;
	coincEvent.id2 = MakeID(rsector, mod, submod, pixel, layer);
	// debugging:
//	std::cout << "Evt2: " << rsector << " " << mod << " " << pixel << " " << layer << "\n";
//	std::cout << "Angular pos: " << angularPosition << "\n";

	if (is.fail())
	{
		std::ostringstream msg;
		msg << "error parsing gate coincidence file " << filename;
		msg << " on line " << lineNo;
        throw std::runtime_error(msg.str());
	}

	return true;
}
