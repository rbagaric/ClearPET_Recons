/// \file	rootreader.h
/// \brief	Declaration of class RootReader
/// \author	Axel Wagner
///
#ifndef rootreader_h_included
#define rootreader_h_included

#include <fstream>
#include "inttypes.h"

struct CoincEvent;

///\brief Gate file input from coincidence file
///
/// This is a minimal interface to read Root coincidence files for the rebinner.
/// This format is pretty simple: each record consists of 3 16-bit word:
///  - the rotation angle in units of 1/10 degree
///	 - the crystal id of the first crystal
///	 - the crystal id of the second crystal
/// Crystal IDs are coded just like used in class CoincEvent.
///
class RootReader
{
public:
	RootReader(const char* fileName);

	bool ReadEvent(CoincEvent& coincEvent);

private:
	std::ifstream	file;
};

#endif // rootreader_h_included
