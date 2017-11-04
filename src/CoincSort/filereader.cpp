/// \file	filereader.cpp
/// \brief	Implementation of class FileReader
/// \author	Axel Wagner

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#if WIN32
#include <io.h>
#define WINDOWS_MEAN_AND_LEAN
#define NOMINMAX
#include <windows.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif

#include "filereader.h"

/**\page about_buffer_sizes Note about buffer sizes

 Experiments on WinXP have shown that buffer sizes are critical for performance.

 In general, a large calculation buffers work faster, because the number of IO
 operations is reduced. If enough memory is available, about 2MB per event file
 work quite well.

 However, it has been found that on WinXP, disk IO with large buffers
 is less effective than IO with smaller buffers, even if the total number of data
 is the same!
 Therefore, we have an additional IO buffer size. This size defines the block size
 in which data will be loaded from disk. On WinXP, best performance is found if
 this IO size is at 4kB, the file system page size.
 (The difference in performance depends on the fragmentation rate of the file
 system. It is smaller for on unfragmented file system, and larger on a very
 much fragmented file system)

 There are not yet any results for other OS, especially Linux. But since buffer
 sizes are tuneable, the optimum buffer sizes can be adjusted with some
 experimentation.
*/

unsigned int FileReader::calcBufferSize  = 2048*1024;
int FileReader::fileBufferSize  = 4*1024;

///\brief Set buffer sizes
///
/// This function should be called before first action with FileReader.
/// The buffer sizes have global effect to all instances of FileReader.
///
void FileReader::SetBufferSizes(int calcBufSize, int ioBufSize)
{
	calcBufferSize = calcBufSize*1024;
	fileBufferSize = ioBufSize*1024;
}


FileReader::FileReader()
:	hFile(-1)
,   eventsInFile(0)
,   buffer(0)
{
}

FileReader::~FileReader()
{
	if (hFile != -1)
		close(hFile);

	delete[] buffer;
}

///\brief Check if the instance is valid (file successfully opened)
///
bool FileReader::IsValid() const
{
	return -1 != hFile;
}

///\brief Assign a file to the FileReader
///
/// This function opens the file and checks the file size. It must be a
/// multiple of the binary event size. The number of events in the file
/// is stored. A buffer is allocated for later file IO.
///
void FileReader::Assign(const char* filename)
{
	using namespace std;

	// Get the file size:
	uint64_t	fileSize;
#if WIN32
	DWORD sizeLow, sizeHigh;
	HANDLE h = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE)
	{
		ostringstream os;
		DWORD err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND)
			os << "File not found: ";
		else if (err == ERROR_ACCESS_DENIED)
			os << "Access denied to ";
		else
			os << "Error " << err << " getting size of file ";
		os << filename;
		throw runtime_error(os.str());
	}
	sizeLow = GetFileSize(h, &sizeHigh);
	if (sizeLow == 0xFFFFFFFF)
	{
		DWORD err = GetLastError();
		if (err != NO_ERROR)
		{
			ostringstream os;
			os << "Error " << err << " getting size of file " << filename;
			CloseHandle(h);
			throw runtime_error(os.str());
		}
	}
	CloseHandle(h);
	fileSize = (uint64_t(sizeHigh) << 32) + sizeLow;
 #else
    // TODO: this is limited to 4GB file size. Is that a problem?
    struct stat results;
	if (0 != stat(filename, &results))
	{
		// An error occurred
		ostringstream os;
		switch (errno)
		{
			case ENOENT:	os << "File not found: "; break;
			case EACCES:	os << "Access denied to "; break;
			default:		os << "Error " << errno << " getting size of file "; break;
		}
		cout << filename;
		throw runtime_error(os.str());
	}
	fileSize = results.st_size;
#endif

	const int evtSize = sizeof(data_type);
	if (0 != (fileSize % evtSize))
	{
		ostringstream os;
		os << "Error: Size of file " << filename << " is not a multiple of " << evtSize <<"!";
		throw runtime_error(os.str());
	}
	eventsInFile = fileSize / evtSize;

	hFile = open(filename, O_RDONLY|O_BINARY);
	if (!IsValid())
		throw runtime_error(string("Error opening file ") + filename);

	if (!buffer)
		buffer = new char[calcBufferSize];
}

///\brief Read a block of data from file
///\param ppBuffer Address of a pointer where the buffer address will be stored.
///\return The number of events read
///
int32_t FileReader::ReadBlock(data_type** ppBuffer)
{
	if (0 == eventsInFile)
		return 0;

	uint32_t eventsInBuffer = calcBufferSize/sizeof(data_type);
	uint32_t nEvents;
	if (eventsInFile > eventsInBuffer)
		nEvents = eventsInBuffer;
	else
		nEvents = static_cast<uint32_t>(eventsInFile);

	char* p = buffer;
    size_t toRead = nEvents * sizeof(data_type);
	while (toRead)
	{
		size_t n1 = std::min(toRead, (size_t)fileBufferSize);
        int nRead = read(hFile, p, size_t(n1));
		if (0 == nRead || nRead == -1)
			return -1;
		toRead -= nRead;
		p += nRead;
	}
    *ppBuffer = reinterpret_cast<data_type*>(buffer);
	eventsInFile -= nEvents;

    return int32_t(nEvents);
}

