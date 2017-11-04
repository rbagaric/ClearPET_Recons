/// \file	miniLMFReader.cpp
/// \brief	Implementation of class MiniLMFReader
/// \author	Axel Wagner
///

#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <string>

#include "clearpet.h"
#include "coincevent.h"
#include "miniLMFReader.h"

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

/// Returns an integer with bits 0..nBits set to one.
inline uint16_t AllBitsOne(int nBits)
{
    return uint16_t((1 << nBits) - 1);
}

// The binary file contains bytes in network byte order, which means
// little endian. Here we read an uint16 value from a byte array in the
// correct order. Note that this code is independent of the byte order
// of the machine the code is running on.
//
inline uint16_t bytesToShort(uint8_t* p)
{
	return (uint16_t(p[0]) << 8) + p[1];
}

uint16_t ReadHeaderEntry(std::ifstream& file)
{
	uint8_t	word[2];
	file.read(reinterpret_cast<char*>(word), 2);
	if (file.fail() || file.gcount() != 2)
		throw std::runtime_error("Error reading LMF header");
	return bytesToShort(word);
}

/// Read the LMF file header.
/// See LMF specs for details.
int CheckHeader(std::ifstream& file)
{
	using namespace std;

	uint16_t	found;

	// encoding rule:
	uint16_t expected = MakeID( AllBitsOne(CoincEvent::bitsSectors), 0, AllBitsOne(CoincEvent::bitsSubMods), 0, AllBitsOne(CoincEvent::bitsLayers));
	found = ReadHeaderEntry(file);
	if (found != expected)
	{
		ostringstream os;
		os << "Invalid encoding rule. Expecting "
		   << hex << expected << " but found " << hex << found;
		throw std::runtime_error(os.str());
	}

	// Note about "sectors", "modules" and "submodules":
	// The ClearPET scanners has 20 cassettes (sectors) with 4 PMTs (modules).
	// We do not have submodules, therefore the submodule count is 1.

	// general scanner description:
	// (20 sectors, 4 Modules, 1 SubModule, 64 Pixels, 2 Layers)
	expected = MakeID( ClearPET::numSectors-1, ClearPET::numModules-1, 0, ClearPET::numPixels-1, ClearPET::numLayers-1);
	found = ReadHeaderEntry(file);
	if (found != expected)
	{
		ostringstream os;
		os << "Invalid general scanner description. Expecting "
		   << hex << expected << " but found " << hex << found;
		throw std::runtime_error(os.str());
	}

	// tangential scanner description:
	// (20 sectors, 1 Modules, 1 SubModule, 8 Pixels, 2 Layers(radial))
	expected = MakeID( ClearPET::numSectors-1, 0, 0, ClearPET::numPixelsTangential-1, ClearPET::numLayers-1);
	found = ReadHeaderEntry(file);
	if (found != expected)
	{
		ostringstream os;
		os << "Invalid tangential scanner description. Expecting "
		   << hex << expected << " but found " << hex << found;
		throw std::runtime_error(os.str());
	}

	// axial scanner description:
	// (1 sectors, 4 Modules, 1 SubModule, 8 Pixels, 1 Layers)
	expected = MakeID( 0, ClearPET::numModules-1, 0, ClearPET::numPixelsAxial-1, 0);
	found = ReadHeaderEntry(file);
	if (found != expected)
	{
		ostringstream os;
		os << "Invalid axial scanner description. Expecting "
		   << hex << expected << " but found " << hex << found;
		throw std::runtime_error(os.str());
	}

	// number of record types:
	// (we use only event records)
	found = ReadHeaderEntry(file);
	if (1 != found)
		throw std::runtime_error("Invalid number of record types");

	// coincidence event record encoding pattern:
	// This is encoded follwing the rule "TTTTcdEnNNgbsGfR" where:
	//  TTTT  0: record tag = event records
	//  c     1: coincidence events are recorded
	//  d     1: detector ID is recorded
	//  E     1: energy is recorded
	//  n     0: neighbour energy is NOT recorded
	//  NN    0: neighbouring order = no neighbours
	//  g     1: azimuthal position of gantry is recorded
	//  b     1: axial position of the bed is recorded
	//  s     0: external source position is NOT recorded
	//  G     0: data is NOT simulated
	//  f     1: FPGA neighbour is recorded
	//  R     0: reserved bit
	// results in 0x0E32, record size 16 bytes
	// or
	//  TTTT  0: record tag = event records
	//  c     1: coincidence events are recorded
	//  d     1: detector ID is recorded
	//  E     0: energy is recorded
	//  n     0: neighbour energy is NOT recorded
	//  NN    0: neighbouring order = no neighbours
	//  g     1: azimuthal position of gantry is recorded
	//  b     0: axial position of the bed is recorded
	//  s     0: external source position is NOT recorded
	//  G     0: data is NOT simulated
	//  f     0: FPGA neighbour is recorded
	//  R     0: reserved bit
	// results in 0x0C20, record size 10 bytes
	found = ReadHeaderEntry(file);
	if (found == 0x0E32)
		return 16;
	else if (found == 0x0C20)
		return 10;
	else
		throw std::runtime_error("Invalid coincidence record encoding pattern");
}


MiniLMFReader::MiniLMFReader(const char* fileName)
:	file(fileName, std::ios::in|std::ios::binary),
	recordSize(0),
    buffer(0)
{
    if (!file)
        throw std::runtime_error(std::string("error opening ") + fileName);

	// Read and analyze the the header, calculate the event record size
	recordSize = CheckHeader(file);

	// allocate buffer of correct size
	buffer = new uint8_t[recordSize];
}

MiniLMFReader::~MiniLMFReader()
{
	delete[] buffer;
}


/// Write one event to the LMF file.
/// See LMF specs for details.
bool MiniLMFReader::ReadEvent(CoincEvent& coincEvent)
{
	file.read(reinterpret_cast<char*>(buffer), recordSize);
	if (file.eof() || file.gcount() != recordSize)
		// not enough data, stop reading
		return false;

	if (0 != (buffer[0] & 0x80))
		throw std::runtime_error("Invalid record tag in event record");
	coincEvent.id1 = bytesToShort(buffer+4);
	coincEvent.id2 = bytesToShort(buffer+6);
	coincEvent.angle = bytesToShort(buffer+8);

	return true;
}
