/// \file	evttypes.h
/// \brief	Declaration of event types
/// \author	Axel Wagner
///
#ifndef evttypes_h_included
#define evttypes_h_included

#include "inttypes.h"
#include "lookupPixel.h"

/// data type for timestamps (resolution 25ns/64 = 0.39ns)
typedef uint64_t TimeStamp;


#if INPUT_32BIT
//---------- Data types for 32bit input files -----------

/// Compressed event data type for the 32-bit binary input files
typedef uint32_t BinaryEvent;

/// \brief Uncompressed event data type for internal use.
///
/// This class stores the compressed event data, and the most
/// important attributes in uncompressed members. Less important
/// attributes are extracted from the compressed data on demand.
///
class EventData {
public:
	explicit EventData(BinaryEvent bindat = 0)
	{
		binaryEvent = bindat;
		module = 0;
		timeStamp = bindat & 0xFFFFF;
		// note: both module and timeStamp are only temporaries here.
		// They will later be changed by AddTime() and FixModule().
	}

	TimeStamp	GetTimeStamp() const { return timeStamp; }
    int GetEnergy() const { return 0; }
	int GetModule() const { return module; }
	int GetPixel() const { return LookupPixel(int(binaryEvent >> 22) & 0x3F); }
	int GetNeighbour() const { return 0; }
	int	GetLayer() const { return int(binaryEvent >> 28) & 0x1; }
	int GetPM() const { return int(binaryEvent >> 29) & 0x3; }
	BinaryEvent	GetBinaryEvent() const { return binaryEvent; }

	void AddTime(TimeStamp delay) { timeStamp += delay; }
	void FixModule(int modules[]) { module = modules[int((binaryEvent >> 20) & 0x3)]; }

private:
	BinaryEvent	binaryEvent;
	TimeStamp	timeStamp;
	int			module;
};

#else
//---------- Data types for 64bit input files -----------

/// Compressed event data type for the 64-bit binary input files
typedef uint64_t BinaryEvent;

/// \brief Uncompressed event data type for internal use.
///
/// This class stores the compressed event data, and the most
/// important attributes in uncompressed members. Less important
/// attributes are extracted from the compressed data on demand.
///
class EventData {
public:
	explicit EventData(BinaryEvent bindat = 0)
	{
		binaryEvent = bindat;
		module = int(bindat & 0x1F);
		timeStamp = bindat >> 26;
	}

	TimeStamp	GetTimeStamp() const { return timeStamp; }
    int GetEnergy() const { return int(binaryEvent >> 18) & 0xFF; }
	int GetModule() const { return module; }
	int GetPixel() const { return LookupPixel(int(binaryEvent >> 12) & 0x3F); }
	int GetNeighbour() const { return int(binaryEvent >> 8) & 0xF; }
	int	GetLayer() const { return int(binaryEvent >> 7) & 0x1; }
	int GetPM() const { return int(binaryEvent >> 5) & 0x3; }
	BinaryEvent	GetBinaryEvent() const { return binaryEvent; }

	void AddTime(int delay) { timeStamp += delay; }

private:
	BinaryEvent	binaryEvent;
	TimeStamp	timeStamp;
	int			module;
};

#endif


#endif // evttypes_h_included
