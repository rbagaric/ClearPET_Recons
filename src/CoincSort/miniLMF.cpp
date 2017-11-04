/// \file	miniLMF.cpp
/// \brief	Implementation of class MiniLMF
/// \author	Axel Wagner
///

#include <stdexcept>
#include <string>

#include "miniLMF.h"
#include "evttypes.h"

///\brief use compact format if 1
///
/// The compact format lacks some information which is not used in the
/// normal rebinning process, or which is constant. This is:
///  - energies of the events
///  - axial bed position (constant, zero)
///  - FPGA neighbours
/// This saves 6 bytes per coincidence event, reducing the record size from
/// 16 to 10 bytes
#define COMPACT_FORMAT 1

#if COMPACT_FORMAT
const int recordSize = 16;
#else
const int recordSize = 10;
#endif

// The buffer size should be a multiple of the event record size:
const int BufferSize = recordSize*0x4000;	// 256k

MiniLMF::MiniLMF(const char* fileName)
:	file(fileName, std::ios::out|std::ios::binary),
	lastTime(0),
    buffer( new uint8_t[BufferSize] ),
	bytesInBuffer(0)
{
    if (!file)
        throw std::runtime_error(std::string("error opening ") + fileName);
}

MiniLMF::~MiniLMF()
{
	if (bytesInBuffer > 0)
        // note: don't throw exceptions here, we're in the destructor!
		file.write(reinterpret_cast<char*>(buffer), bytesInBuffer);
	delete[] buffer;
}

/// Returns a detector ID for the encoding rule used for the
/// commercial ClearPET scanner.
/// See LMF specs for details.
inline
uint16_t MakeID(int sector, int mod, int subMod, int pixel, int layer)
{
	int result = sector;
	result = (result << MiniLMF::bitsMods) + mod;
	result = (result << MiniLMF::bitsSubMods) + subMod;
	result = (result << MiniLMF::bitsPixels) + pixel;
	result = (result << MiniLMF::bitsLayers) + layer;
	return uint16_t(result);
}

/// Returns an integer with bits 0..nBits set to one.
inline uint16_t AllBitsOne(int nBits)
{
    return uint16_t((1 << nBits) - 1);
}

// The binary file contains bytes in network byte order, which means
// little endian. Here we store an uint16 value in a byte array in the
// correct order. Note that this code is independent of the byte order
// of the machine the code is running on.
//
inline uint8_t* shortToBytes(uint8_t* p, uint16_t x)
{
    *p++ = uint8_t(x >> 8);
	*p++ = uint8_t(x & 0xFF);
	return p;
}

// This define prepare for a future extension: adding count rate records.
// Those records might be useful for image corrections.
#define USE_COUNTRATE_RECORDS 0		// At the moment, this extension is disabled.

/// Write the LMF file header.
/// See LMF specs for details.
void MiniLMF::WriteHeader()
{
#if USE_COUNTRATE_RECORDS
	uint8_t	header[2*7];
#else
	uint8_t	header[2*6];
#endif
	uint8_t*	p = header;

	// encoding rule:
	p = shortToBytes(p, MakeID( AllBitsOne(bitsSectors), 0, AllBitsOne(bitsSubMods), 0, AllBitsOne(bitsLayers)));

	// Note about "sectors", "modules" and "submodules":
	// The ClearPET scanners has 20 cassettes (sectors) with 4 PMTs (modules).
	// We do not have submodules, therefore the submodule count is 1.

	// general scanner description:
	// (20 sectors, 4 Modules, 1 SubModule, 64 Pixels, 2 Layers)
	p = shortToBytes(p, MakeID( numSectors-1, numMods-1, 0, numPixels-1, numLayers-1));

	// tangential scanner description:
	// (20 sectors, 1 Modules, 1 SubModule, 8 Pixels, 2 Layers(radial))
	p = shortToBytes(p, MakeID( numSectors-1, 0, 0, numPixelsTangential-1, numLayers-1));

	// axial scanner description:
	// (1 sectors, 4 Modules, 1 SubModule, 8 Pixels, 1 Layers)
	p = shortToBytes(p, MakeID( 0, numMods-1, 0, numPixelsAxial-1, 0));

	// number of record types:
	// (we use event records and count rate records)
#if USE_COUNTRATE_RECORDS
	p = shortToBytes(p, 2);
#else
	p = shortToBytes(p, 1);
#endif

	// coincidence event record encoding pattern:
	// This is encoded follwing the rule "TTTTcdEnNNgbsGfR" where:
#if COMPACT_FORMAT
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
	p = shortToBytes(p, 0x0E32);
#else
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
	p = shortToBytes(p, 0x0C20);
#endif

#if USE_COUNTRATE_RECORDS
	// count rate record encoding pattern:
	// This is encoded follwing the rule "TTTTsSScFrbRRRRR" where:
	//  TTTT  1000: record tag = count rate records
	//  s     1: total singles rate is recorded
	//  SS   10: singles rates listed per module (=PMT)
	//  c     1: total coincidence rate is recorded
	//  F     0: NO total random count rate
	//  r     0: NO rotations speed
	//  b     0: NO bed or axial speed
	//  R     0: reserved bits
	p = shortToBytes(p, 0x8D00);
#endif

	file.write(reinterpret_cast<char*>(header), sizeof(header));
	if (!file)
        throw std::runtime_error("Error writing header data to coincidence output file");
}

/// Write one event to the LMF file.
/// See LMF specs for details.
void MiniLMF::WriteEvent(uint16_t angle, uint16_t bedPos,
	const EventData& ed1, const EventData& ed2, uint32_t timeOffset)
{
#if COMPACT_FORMAT
	bedPos; timeOffset; // unused
#else
	const uint64_t evtTimeResolution = 390000000L;	// 390e6
	uint32_t evtTime = uint32_t(ed1.GetTimeStamp() / evtTimeResolution) + timeOffset;

	uint32_t relativeTime;
	if (0 == lastTime)
		// first event, relative time is zero
		relativeTime = 0;
	else
		relativeTime = evtTime - lastTime;
	lastTime = evtTime;
#endif

	if (bytesInBuffer+recordSize > BufferSize)
	{
		file.write(reinterpret_cast<char*>(buffer), bytesInBuffer);
		if (!file)
            throw std::runtime_error("Error writing event data to coincidence output file");
		bytesInBuffer = 0;
	}

    uint8_t*	p = buffer + bytesInBuffer;
#if COMPACT_FORMAT
	// time data are not used in rebinner, but must be present. We just write zeroes.
	*p++ = 0;
	*p++ = 0;
	*p++ = 0;
	*p++ = 0;
#else
	p[3] = 0;	// time of flight, unused
	p[2] = uint8_t(relativeTime & 0xFF);
	p[1] = uint8_t((relativeTime >> 8) & 0xFF);
	p[0] = uint8_t((relativeTime >> 16) & 0x7F);	// Bit 7 must be zero here!
	p += 4;
#endif

	p = shortToBytes(p, MakeID(ed1.GetModule(), ed1.GetPM(), 0, ed1.GetPixel(), ed1.GetLayer()));
	p = shortToBytes(p, MakeID(ed2.GetModule(), ed2.GetPM(), 0, ed2.GetPixel(), ed2.GetLayer()));
	p = shortToBytes(p, angle);
#if !COMPACT_FORMAT
	p = shortToBytes(p, bedPos);

	*p++ = uint8_t(ed1.GetEnergy());
	*p++ = uint8_t(ed2.GetEnergy());
	*p++ = uint8_t(ed1.GetNeighbour());
	*p++ = uint8_t(ed2.GetNeighbour());
#endif

	// Note: Be careful when changing the event record data: if the record size is
	// changed, the constant recordSize must be adjusted!

	bytesInBuffer += recordSize;
}

#if USE_COUNTRATE_RECORDS
/* TODO:
bool MiniLMF::WriteCountRates(....)
{
	const int countRateRecordSize = ?;

	if (bytesInBuffer+countRateRecordSize > BufferSize)
	{
		file.write(reinterpret_cast<char*>(buffer), bytesInBuffer);
		if (!file)
			return false;
		bytesInBuffer = 0;
	}

	byte*	p = buffer + bytesInBuffer;
	...

	bytesInBuffer += countRateRecordSize;

	return true;
}
*/
#endif
