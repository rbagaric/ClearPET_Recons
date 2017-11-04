/**
 *\file		coincevent.h
 *\brief	Declarations related to Coincidence events
 *\author	Axel Wagner
 */
#ifndef coincevent_h_included
#define coincevent_h_included

#include "inttypes.h"

///\brief Data structure describing a coincidence event
struct CoincEvent
{
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
		bitsLayers = 1,

		shiftSectors = 16-bitsSectors,
		shiftModules = shiftSectors-bitsMods,
		shiftSubMods = shiftModules - bitsSubMods,
		shiftPixels = shiftSubMods- bitsPixels,
		shiftLayers = shiftPixels - bitsLayers,

		maskSectors = 0xF800,
		maskModules = 0x0600,
		maskSubMods = 0x0180,
		maskPixels = 0x007E,
		maskLayers = 0x0001
	};

	uint16_t	id1;	///< crystal id of first single event
	uint16_t	id2;	///< crystal id of second single event
	uint16_t	angle;	///< gantry rotation angle at time of coincidence, in 1/10 degrees
};

#endif // coincevent_h_included
