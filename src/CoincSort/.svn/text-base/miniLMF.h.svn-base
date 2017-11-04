/// \file	miniLMF.h
/// \brief	Declaration of class MiniLMF
/// \author	Axel Wagner
///
#ifndef miniLMF_h_included
#define miniLMF_h_included

#include <fstream>
#include "inttypes.h"

class EventData;

///\brief LMF file output for coincidences
///
/// This is a minimal interface to write LMF files
/// for the coincidence sorter.
///
/// Coding this ourselves instead of using the LMF library avoids the use
/// of the LMF GPL and makes the code more compact. However, we give up
/// the high level of flexibility which exists in the LMF library.
///
class MiniLMF
{
public:
	// The following constants define the number of bits used
	// in encoding the detector IDs. The sum of all bits must
	// be 16.
	// We use constants here for performance reasons, and because
	// they are really device constants in the ClearPET system.
	enum {
		bitsSectors = 5,
		bitsMods = 2,
		bitsSubMods = 2,	// but submods are never used...
		bitsPixels = 6,
		bitsLayers = 1
	};

	// The following constants define the number of elements
	// really existing in the scanner.
	// We use constants here for performance reasons, and because
	// they are really device constants in the ClearPET system.
	enum {
		numSectors = 20,
		numMods = 4,
		numSubMods = 1,
		numPixelsAxial = 8,
		numPixelsTangential = 8,
		numPixels = numPixelsAxial*numPixelsTangential,
		numLayers = 2
	};

	MiniLMF(const char* fileName);
	~MiniLMF();

	void WriteHeader();
	void WriteEvent(uint16_t angle, uint16_t bedPos, const EventData& ep1, const EventData& ep2, uint32_t timeOffset);

private:
	std::ofstream	file;
	uint32_t        lastTime;
	uint8_t*        buffer;
	int				bytesInBuffer;
};

#endif // miniLMF_h_included
