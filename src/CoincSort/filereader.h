/// \file	filereader.h
/// \brief	Declaration of class FileReader
/// \author	Axel Wagner
#ifndef filereader_h_included
#define filereader_h_included

/**\page about_file_io_speed About file IO speed

Things I tried to speed up the file IO when writing/testing this code:
- Defragmentation of hard disk. This is quite important.
- Use large test data sets. Otherwise, some data might be held in the file
 chache, and results are not reproducible.
- Use low level IO functions (open/read/close) instead of stream IO functions
 (fopen/fread/fclose) or C++ streams. It seems that the stream functions
 waste time by additional buffering/copying of data.
- There was no significant difference between C-libray low level IO and
 native IO (CreateFile/ReadFile) under Win32.
- Overlapped IO under Windows. This was disappointing, the program was
 running with only half speed. I assume that read-ahead is not active
 when using overlapped IO.
- Reading data in a background thread. This also did not improve speed,
 maybe because switching between threads takes too much time.
- Reduce number of input files. If each preprocessor produces one presorted
 single event file instead of 4, the file IO (and maybe also the sorting)
 speed increases, because the code has to handle less files, and the OS
 can access the files more efficiently.
- Switched from a 64-bit to a 32-bit event format. This reduces the amount
 of data to be loaded by factor of two, and increases IO speed.
*/

#include "inttypes.h"

/// This class represents an interface to read from event files.
/// It hides some potential implementation details.
//
class FileReader {
public:
#if INPUT_32BIT
	typedef uint32_t	data_type;
#else
	typedef uint64_t	data_type;
#endif

	static void SetBufferSizes(int calcBufSize, int ioBufSize);

	FileReader();
	~FileReader();

	void Assign(const char* name);
	bool IsValid() const;
	uint64_t GetEventsInFile() const { return eventsInFile; }

    int32_t ReadBlock(data_type** ppBuffer);

private:
	static unsigned int		calcBufferSize;
	static int				fileBufferSize;

	int         hFile;
    uint64_t    eventsInFile;
    char*       buffer;
};

#endif // filereader_h_included
