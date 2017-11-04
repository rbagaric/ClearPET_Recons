/// \file	miniLMFReader.h
/// \brief	Declaration of class MiniLMFReader
/// \author	Axel Wagner
///
#ifndef miniLMFReader_h_included
#define miniLMFReader_h_included

#include <fstream>
#include "inttypes.h"

struct CoincEvent;

///\brief LMF file input from coincidence file
///
/// This is a minimal interface to read LMF files for the rebinner.
///
/// Coding this ourselves instead of using the LMF library avoids the use
/// of the LMF GPL and makes the code more compact. However, we give up
/// the high level of flexibility which exists in the LMF library.
///
class MiniLMFReader
{
public:
	MiniLMFReader(const char* fileName);
	~MiniLMFReader();

	bool ReadEvent(CoincEvent& coincEvent);

private:
	std::ifstream	file;
	int				recordSize;
	uint8_t*        buffer;
};

#endif // miniLMFReader_h_included
