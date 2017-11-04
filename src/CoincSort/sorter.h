///\file	sorter.h
///\brief	class Sorter and related declatations
///\author	Axel Wagner
#ifndef sorter_h_included
#define sorter_h_included

#include "evtpipe.h"
#include "filereader.h"

#include <fstream>


class EventData;
class MiniLMF;
class AngularPositions;

///\brief Conversion from nanoseconds to timestamp values
///\note Since timestamps are counted in a 0.39ns tick,
/// the resulting timestamp value is rounded to the
/// next possible value.
///
inline TimeStamp nsToTimeStamp(int ns)
{
	return (64*ns+12)/25;
}

///\brief Conversion from timestamp values to nanoseconds
///\note Since timestamps are counted in a 0.39ns tick,
/// the resulting nanosecond value is slightly less
/// accurate.
///\see timeStampToS for an exact result
///
inline long timeStampToNs(TimeStamp ts)
{
	return long((ts*25+32)/64);
}

///\brief Conversion from timestamp values to seconds
///\note Result is double, and thereby exact.
///
inline double timeStampToS(TimeStamp ts)
{
	return ts*(25e-9/64);
}


///\brief Event Buffer for singles files
///
/// This class implements the EventBuffer interface for
/// files which store single events.
///
/// File data will be read into buffers which size can
/// be adjusted (globally) by SetBufferSizes().
///
/// The most of the work is done by a FileReader instance.
///
class FileBuffer: public EventBuffer
{
public:
	FileBuffer();

	virtual std::string GetName() const { return fileName; }
	virtual bool IsValid() const { return fileReader.IsValid(); }

	uint64_t GetEventsInFile() const { return fileReader.GetEventsInFile(); }
	void	SetFile(const std::string& name);

	static void SetBufferSizes(int calcBufSize, int ioBufSize);

private:
	FileReader	fileReader;
	std::string	fileName;

	virtual bool Refill();
};

///\brief Coincidence allowance checker
///
/// This class is used to encapsulate the check wether coincidence
/// between two detectors is allowed. The operator() is
/// used to perform that check.
///
/// The class stores this information in a matrix. Usually,
/// this matrix is initialized by calling SetFOV(), which assumes
/// a circular detector setup at equal distances, and allows
/// coincidence between opposite detectors.
///
/// An alternative is the function LoadMatrix(), which can load
/// the matrix from a file. This can be used for any kind of
/// detector setup.
///
class Allowed
{
public:
	Allowed(int dim);
	~Allowed() { delete[] matrix; }

	bool operator()(int det1, int det2) const;

	void LoadMatrix(std::istream& is);
	void SetFOV(int fov);

	int	GetDimension() const { return dimension; }
	int GetFOV() const { return fov; }

private:
	int		dimension;
	int		fov;
	bool*	matrix;
};

///\brief Coincidence sorter
///
/// This class is doing the event sorting. The coincidence check is
/// delegated into the Output classes, because handling of multiple
/// coincidences is easier that way.
///
class Sorter {
public:
	enum {
		MaxCassCount = 20
	};

	Sorter(int cassCount, int coincWin);

	bool AddInputFile(const std::string& filename);
	bool SetEnergyWindow(int minErg, int maxErg);
	bool SetFOV(int numOppositeCass);
	void SetMatrix(const std::string& fileName);
#if DELAYED_COINC_CHECK
	void SetRandomDelay(long delay) { randomDelay = (int)nsToTimeStamp(delay); }
#endif
    void SetRadius(double r) { innerRadius = r; }
    void SetAngularOffset(double a) { angularOffset = a; }

	void Sort(const std::string& encoderFile, const std::string& outputFile, int firstReset, int lastReset);

	static bool IsSilent() { return isSilent; }
	static void SetSilent(bool silent) { isSilent = silent; }

	void Report(std::ostream& os);
	bool WriteCCH(const std::string& filename);

	bool IsCoincident(const EventData& ed1, const EventData& ed2) const;
    bool IsEnergyValid(int erg) const;
	bool IsAllowed(const EventData& ed1, const EventData& ed2) const;

private:
	typedef std::vector<std::string> FileList;

	FileList	fileList;
	TimeStamp	coincWindow;
	Allowed	allowed;
	int		minEnergy, maxEnergy;
    double  innerRadius;    ///< inner radius of scanner, in mm
	double	angularOffset;	///< offset to be subtracted from rotation angle, in degrees

	static bool	isSilent;
	uint64_t	coincCount;
	uint64_t	coincCountWritten;
	uint64_t	eventCount;
#if DELAYED_COINC_CHECK
	int     randomDelay;
	uint64_t	randomsCount;
#endif
	double	totalDAQTime;	// in seconds
};

#endif // sorter_h_included
