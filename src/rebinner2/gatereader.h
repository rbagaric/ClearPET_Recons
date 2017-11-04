/// \file	gatereader.h
/// \brief	Declaration of class GateReader
/// \author	Axel Wagner
///
#ifndef gatereader_h_included
#define gatereader_h_included

#include <fstream>
#include "inttypes.h"

struct CoincEvent;

///\brief Gate file input from coincidence file
///
/// This is a minimal interface to read Gate coincidence files for the rebinner.
///
class GateReader
{
public:
	GateReader(const char* fileName);
	~GateReader();

	bool ReadEvent(CoincEvent& coincEvent);

private:
	std::ifstream	file;
	std::string		filename;
	int				lineNo;
};

#endif // gatereader_h_included
