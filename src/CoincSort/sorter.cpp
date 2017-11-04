#include <algorithm>
#include <climits>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <deque>
#include <memory>
#include <sstream>
#include <stdexcept>
#if !defined(WIN32)
#include <sys/times.h>
#include <unistd.h>
#endif

#define MULTI_COINC_STATS 0

#include "angles.h"
#include "evttypes.h"
#include "miniLMF.h"
#include "sorter.h"

Allowed::Allowed(int dim)
:	dimension(dim),
	fov(0)
{
	if (dim < 1 || dim > 40)
		throw std::range_error("AllowedMatrix: dimension out of range");
	matrix = new bool[dim*dim];
	std::memset(matrix, 0, sizeof(bool)*dim*dim);
}

inline
bool Allowed::operator()(int det1, int det2) const
{
#if 0
// This is old code. The new code with the matrix is not faster,
// but more flexible for unusual configurations.
	int d = std::abs(det1-det2);
	return d >= minDeltaDet && d <= maxDeltaDet;
#else
	return matrix[det1*dimension+det2];
#endif
}


/**\brief Fill matrix for standard FOV setup
 The standard FOV setup allows the opposite N detectors for
coincidence checking. To be symmetric, N should be an odd number.
This assumes a full ring setup with a total "dimension" detectors.

\param newFOV the number N of opposite detectors to be allowed
*/
void Allowed::SetFOV(int newFov)
{
	if (newFov < 1 || newFov >= dimension)
		throw std::range_error("AllowedMatrix: fov value out of range");

	fov = newFov;
	std::memset(matrix, 0, sizeof(bool)*dimension*dimension);

	int minDeltaDet = (dimension-fov+1)/2;
	int maxDeltaDet = (dimension+fov-1)/2;

	for (int i = 0; i < dimension; ++i)
	{
		for (int j = i+1; j < dimension; ++j)
		{
			int d = j-i;
			if (d >= minDeltaDet && d <= maxDeltaDet)
			{
				matrix[i*dimension+j] = 1;
				matrix[j*dimension+i] = 1;
			}
		}
	}
}

/** \brief Load the matrix from a stream
 The data on the stream is exepcted to be in this format:
- One line per detector
- empty lines or lines starting with '#' are ignored
- lines start with the detector number (starting at 1!),
  followed by an equal sign, followed by a series of 0 or 1
Example for FOV 9 on a 20 detector ring:
   1=00000011111111100000
   2=00000001111111110000
   3=00000000111111111000
   4=00000000011111111100
   5=00000000001111111110
   6=00000000000111111111
   7=10000000000011111111
   8=11000000000001111111
   9=11100000000000111111
  10=11110000000000011111
  11=11111000000000001111
  12=11111100000000000111
  13=11111110000000000011
  14=11111111000000000001
  15=11111111100000000000
  16=01111111110000000000
  17=00111111111000000000
  18=00011111111100000000
  19=00001111111110000000
  20=00000111111111000000

\param is the stream from which the matrix is loaded
\throw std::runtime_error on any kind of error in the file
*/
void Allowed::LoadMatrix(std::istream& is)
{
	using namespace std;

	memset(matrix, 0, sizeof(bool)*dimension*dimension);
	fov = 0; // zero means non standard setup

	string line;
	while (getline(is, line))
	{
		unsigned i = 0;
		// ignore white space at beginning of line:
		while (i < line.length() && isspace(line[i]))
			++i;
		if (i >= line.length() || line[i] == '#')
			// empty line or comment, ignore
			continue;
		if (line.length()-i < unsigned(dimension+2))
			// we need at least dimension+2 characters in line
			throw runtime_error("Invalid contents of matrix file");
		// read the detector number:
		if (!isdigit(line[i]))
			throw runtime_error("Invalid contents of matrix file");
		int det1 = line[i] - '0';
		++i;
		if (isdigit(line[i]))
		{
			det1 = 10*det1 + line[i] - '0';
			++i;
		}
		if (det1 == 0 || det1 > dimension)
			throw runtime_error("Detector number out of range in matrix file");
		if (isdigit(line[i]))
			throw runtime_error("Detector number out of range in matrix file");
		if (line[i] != '=')
			throw runtime_error("Invalid contents of matrix file");
		++i;
		--det1;
		for (int det2 = 0; det2 < dimension; ++det2, ++i)
		{
			if (i >= line.length() || (line[i] != '0' && line[i] != '1'))
				throw runtime_error("Invalid contents of matrix file");
			matrix[det1*dimension+det2] = line[i] == '1';
		}
	}

	// finally, check if matrix is valid, that means:
	// - matrix[i,i] is false for all i
	// - matrix[i,j] == matrix[j,i] for all i and j
	for (int det1 = 0; det1 < dimension; ++det1)
	{
		if (matrix[det1*dimension+det1])
			throw runtime_error("Invalid matrix definition in matrix file");
		for (int det2 = det1+1; det2 < dimension; ++det2)
		{
			if (matrix[det1*dimension+det2] != matrix[det2*dimension+det1])
				throw runtime_error("Nonsymmetric matrix definition in matrix file");
		}
	}
}


FileBuffer::FileBuffer()
{}

void FileBuffer::SetFile(const std::string& name)
{
	fileName = name;
	fileReader.Assign(fileName.c_str());
}

bool FileBuffer::Refill()
{
//	clock_t t1 = clock();
	// Note: this call also assigns buffer! (TODO: avoid this nasty side effect)
    int32_t n = fileReader.ReadBlock(&buffer);
	if (n > 0)
		bufferEnd = buffer+n;
	else
		bufferEnd = buffer = 0;
//	clock_t t2 = clock();
//	cout << "Buffer reload in " << double(t2-t1)/CLOCKS_PER_SEC*1000.0 << "ms\n";
	return n > 0;
};

void FileBuffer::SetBufferSizes(int calcBufSize, int ioBufSize)
{
	FileReader::SetBufferSizes(calcBufSize, ioBufSize);
}


bool Sorter::isSilent = false;

Sorter::Sorter(int cassCount, int coincWin)
:	coincWindow(nsToTimeStamp(coincWin))
,	allowed(cassCount)
,	innerRadius(135.6)
,	angularOffset(0.0)
,	coincCount(0)
,	coincCountWritten(0)
,	eventCount(0)
#if DELAYED_COINC_CHECK
,	randomDelay(0), randomsCount(0)
#endif
{
	(void)SetFOV(7);
	(void)SetEnergyWindow(350, 750);
}

///\brief Add another input file
///
/// The names of input files are stored in the fileList member.
/// The function checks if a file name is added twice, and returns false
/// in such a case.
///
bool Sorter::AddInputFile(const std::string& filename)
{
	using namespace std;

	if (fileList.end() != find(fileList.begin(), fileList.end(), filename))
		return false;

	fileList.push_back(filename);
	return true;
}

/// Set allowed matrix by setting a standard FOV of opposite cassettes.
//
bool Sorter::SetFOV(int numOppositeCass)
{
	allowed.SetFOV(numOppositeCass);
	return true;
}

/// Set allowed matrix by loading it from a file
//
void Sorter::SetMatrix(const std::string& fileName)
{
	std::ifstream is(fileName.c_str());
	allowed.LoadMatrix(is);
}

/// Set a new energy window.
/// Energies are given in keV, and translated here to internal units
//
bool Sorter::SetEnergyWindow(int minErg, int maxErg)
{
	using namespace std;

	if (minErg >= maxErg || minErg > 1270 || maxErg < 1)
		throw std::runtime_error("Invalid energy window");

	minEnergy = (minErg + 2) / 5;
	maxEnergy = (maxErg + 2) / 5;
	return true;
}

// Note about heap handling (its tricky, but efficient!)
//
// The heap is just an ordinary vector. Any container with a random
// access iterator would do it. In general, a heap is a data structure
// which is used as a binary tree, where every node is "larger" than
// its child nodes. In addition, the heap tree is balanced in a way that
// all levels except the last one are full.
//
// Our heap holds EventPipe pointers. After filling, the heap is sorted
// by the PipeCompare functor, and thereby the pipe with the oldest
// event is in the first element of the heap.
//
// std::pop_heap() will move the first element of the heap to the
// last position, and rebuild the heap. After that operation, we find
// the pipe with the oldest event at back() and the pipe with the
// second oldest event at front(). This is just what we need to
// compare those elements.
//
// If we find a coincidence, we pop_heap() the second oldest event
// and move it to second last position in the heap vector.
// We reread that pipe and insert it back into the heap.
//
// In any case, we discard the oldest event. We reread the pipe at
// last position in the heap vector and insert it back into the heap.
//
// In case of error when reading the next event, the pipe is removed
// from the heap.
//
// The processing continues while more than one pipe is remaining in
// the heap vector.

class PipeCompare
{
public:
	bool operator() (const EventPipe* ep1, const EventPipe* ep2) {
//		return ep1->TimeStamp() > ep2->TimeStamp();
		TimeStamp dt = ep1->GetTimeStamp() - ep2->GetTimeStamp();
		if (0 == dt)
			return ep1->Sector() > ep2->Sector();
		else
            return int64_t(dt) > 0;
	}
};

/// Check if two events are coincident
/// We only check the timestamp here, no other parameters
///
/// \note It is assumed that ed2 has a larger timestamp than ed1. In this program,
/// this is assured by the sorting stage.
inline
bool Sorter::IsCoincident(const EventData& ed1, const EventData& ed2) const
{
//	return (ed2.GetTimeStamp() - ed1.GetTimeStamp()) < coincWindow;
	TimeStamp dt = ed2.GetTimeStamp() - ed1.GetTimeStamp();
	return dt < coincWindow;
}

inline
bool Sorter::IsEnergyValid(int erg) const
{
    return erg >= minEnergy && erg <= maxEnergy;
}

/// Check if a coincidence event is allowed
/// We check detectors and the energy window.
//
inline
bool Sorter::IsAllowed(const EventData& ed1, const EventData& ed2) const
{
	if (!allowed(ed1.GetModule(), ed2.GetModule()) )
		return false;

#if INPUT_32BIT
	return true;
#else
	return IsEnergyValid(ed1.GetEnergy()) && IsEnergyValid(ed2.GetEnergy());
#endif
}

// This is a helper class to create LMF output during sorting.
// It encapsulated all output related data.
//
// Its main interface during sorting is WriteCoinc(), which is
// called by the sortCycle template.
//
class LMFOutput {
public:
#if MULTI_COINC_STATS
    enum { MaxMultiCoinc = 5 };
#endif

	LMFOutput(const std::string& filename, const std::string& encName, const Sorter& rSorter, double angleOffset)
	:	lmfFile(filename.c_str()),
		angles(encName.c_str()),
		angularOffset(angleOffset),
		coincCount(0),
		coincCountWritten(0),
		sorter(rSorter)
	{
		lmfFile.WriteHeader();
	}

	void WriteCoinc(const EventData& ed1, const EventData& ed2);
	void AddEvent(const EventData& data);

	bool	NextReset(int resetCnt);

	uint64_t GetCoincCount() const { return coincCount; }
	uint64_t GetCoincCountWritten() const { return coincCountWritten; }

private:
    typedef std::deque<EventData> LastEventFifo; // note: deque is much more efficient that list!
    MiniLMF				lmfFile;
	AngularPositions	angles;
	double				angularOffset;
	LastEventFifo       lastEvents;

	uint64_t		coincCount;
	uint64_t		coincCountWritten;
	const Sorter&	sorter;
#if MULTI_COINC_STATS
    uint64_t     coincCounter[MaxMultiCoinc];
#endif

	void ProcessCoincidences();
};


bool LMFOutput::NextReset(int resetCnt)
{
	// Process coincidences on all events cached in lastEvents,
	// because no further events will arrive in this reset frame.
	while (lastEvents.size() > 1)
	{
		ProcessCoincidences();
	}

#if MULTI_COINC_STATS
    if (resetCnt > 1)
    {
        using namespace std;
        cout << "Coinc summary for reset #" << (resetCnt-1) << ":\n";
        for (int i = 0; i < MaxMultiCoinc; ++i)
            cout << "[" << i << "] = " << coincCounter[i] << "\n";
    }
    std::memset(coincCounter, 0, sizeof(coincCounter));
#endif

    lastEvents.clear();
	return angles.readAngles(resetCnt);
}

/// \brief Coincidence processing and output
///
/// This function takes out the first event from lastEvents, and processes
/// coincidences to all other remaining events.
/// Processing includes checking if the combination is allowed, getting
/// the angle from the time stamp, and writing the coincidence event to
/// the LMF output.
///
/// \pre lastEvents must contain at least one element
/// \post lastElements has one element less (the first one)
///
inline
void LMFOutput::ProcessCoincidences()
{
	EventData oldEvent = lastEvents.front();
	lastEvents.pop_front();

#if MULTI_COINC_STATS
    int nCoinc = lastEvents.size();
    if (nCoinc >= MaxMultiCoinc)
        nCoinc = MaxMultiCoinc-1;
    coincCounter[nCoinc]++;
#endif

	for (LastEventFifo::iterator i = lastEvents.begin(); i != lastEvents.end(); ++i)
	{
		if (sorter.IsAllowed(oldEvent, *i))
			WriteCoinc(oldEvent, *i);
	}
}

void LMFOutput::AddEvent(const EventData& data)
{
#if !INPUT_32BIT
	// apply energy filter before any coincidence handling:
    if (!sorter.IsEnergyValid(data.GetEnergy()))
		return;
#endif

	// Note: according to Christian Morel, it is important to use all possible
	// event combinations in case of multiple coincidence. Therefore, we store
	// all events in the coincidence window into a list, sorted by time.
	// As soon as an event arrives which is no more in coincidence with the first
	// event in that list, that first event is taken out of the list and all
	// coincidences with it to the remaining events in the list are processed.
	// The following cases are possible:
	// - only one event in the list:
	//  This is the non-coincidence case. The event is taken out of the list
	//  and ignored.
	// - Two events in the list:
	//  This is the usual coincidence case. The coincidence of the two elements
	// is processed, then the first element is taken out. The second one stays
	// in the list, and the processing is repeated, because the second event
	// might have coincidences to other events.
	// - More than two events in the list:
	//  This is the multiple coincidence case. The coincidences of the first
	// element with all other elements is processed, then the first element is
	// taken out, and the processing is repeated.
	// Finally, the new event is pushed on the list.
	while (!lastEvents.empty() && !sorter.IsCoincident(lastEvents.front(), data))
	{
        ProcessCoincidences();
	}

    lastEvents.push_back(data);
}

// This function does the real LMF output
void LMFOutput::WriteCoinc(const EventData& ed1, const EventData& ed2)
{
	using namespace std;

	++coincCount;

	//calculation of the gantry rotational position based on the encoder information
	AngularPositions::angle_type angle = angles.findAngle(ed1.GetTimeStamp());

	if (angle < 0 || angle > 360)
		// event outside angular range, ignore it
		return;

	angle -= angularOffset;
	if (angle < 0)
		angle += 360.0;
	else if (angle >= 360.0)
		angle -= 360.0;
	uint16_t iAngle = static_cast<uint16_t>(10*angle+0.5);
	lmfFile.WriteEvent(iAngle, 0, ed1, ed2, angles.GetTimeSinceStart());
	++coincCountWritten;
}


/*
class RandomsCounter: public Sorter::Output
{
public:
	RandomsCounter()
	:	coincCount(0)
	{}
	void WriteCoinc(const EventPipe& ep1, const EventPipe& ep2);

	int GetCoincCount() const { return coincCount; }

private:
	int		coincCount;
};

// This function just counts the random coincidences
void RandomsCounter::WriteCoinc(const EventPipe&, const EventPipe&)
{
	using namespace std;

	++coincCount;
//	if (0 == coincCount%200000 && !Sorter::IsSilent())
//		cout <<"random #" << coincCount << endl;
}
*/


// Function for processing one sort cycle.
// This function is used for both sorting: normal and delayed coincidences. The
// Output instance passed in will handle the difference in ouput.
//
inline
void sortCycle(std::vector<EventPipe*>& heap, LMFOutput& output, uint64_t& eventCount)
{
	using namespace std;

	// move top of heap to back():
	pop_heap(heap.begin(), heap.end(), PipeCompare());
	EventPipe* oldest = heap.back();
	output.AddEvent(oldest->GetEventData());
	// refill oldest event pipe
	if (oldest->NextEvent())
	{
		++eventCount;
		push_heap(heap.begin(), heap.end(), PipeCompare());
	}
	else
		// read error, remove pipe from vector
		heap.pop_back();
}


/// Sorting of coincidences
/// This is the real worker.
//
void Sorter::Sort(const std::string& encoderFile, const std::string& outputFile, int firstReset, int lastReset)
{
	using namespace std;

#if DELAYED_COINC_CHECK
	bool delayedCoinc = randomDelay > 0;
#endif
	FileBuffer	buffer[MaxCassCount];	// must be declared before pipe array!
	EventPipe	pipe[MaxCassCount];

	eventCount = 0;
	coincCount = 0;
	coincCountWritten = 0;
	totalDAQTime = 0.0;
	double totalEventsInFiles = 0;

#if !defined(WIN32)
	long clock_ticks = sysconf(_SC_CLK_TCK);
#endif

	unsigned numFiles = fileList.size();
	for (unsigned i = 0; i < numFiles; ++i)
	{
		buffer[i].SetFile( fileList[i].c_str() );
		pipe[i].AssignBuffer(&buffer[i]);
#if INPUT_32BIT
		if (!pipe[i].LoadModuleInfo())
			throw std::runtime_error(string("Error loading module info from input file ") + fileList[i]);
#endif
		totalEventsInFiles += buffer[i].GetEventsInFile();
	}

#define PERF_TEST 0
#if PERF_TEST
    bool ok = true;
    while (ok)
    {
	    for (int i = 0; ok && i < numFiles; ++i)
	    {
            ok = buffer[i].Refill();
        }
    }
    return;
#endif

	LMFOutput coincOutput(outputFile + ".ccs", encoderFile, *this, angularOffset);
	std::auto_ptr<LMFOutput> randomsOutput;
#if DELAYED_COINC_CHECK
	if (delayedCoinc)
		randomsOutput.reset(new LMFOutput(outputFile + "_randoms.ccs", encoderFile, *this, angularOffset));
#endif
//	RandomsCounter	randomsOutput;

	// This is the heap vector:
	vector<EventPipe*> evtHeap;
	evtHeap.reserve(numFiles);
#if DELAYED_COINC_CHECK
	uint64_t	countDummy = 0;
	EventPipe delayedPipe[MaxCassCount];
	vector<EventPipe*> delayedEvtHeap;
	delayedEvtHeap.reserve(numFiles);
	if (delayedCoinc)
	{
		for (unsigned i = 0; i < numFiles; ++i)
		{
			delayedPipe[i].SetRandomDelay(randomDelay);
			delayedPipe[i].AssignBuffer(&buffer[i]);
#if INPUT_32BIT
			if (!delayedPipe[i].LoadModuleInfo())
				throw std::runtime_error(string("Error loading module info from input file ") + fileList[i]);
#endif
		}
	}
#endif

	if (0 == lastReset)
		lastReset = INT_MAX-1;
	// reset count loop
	// Since events with different resets are per definition not
	// coincident, we process only events with same reset. That way,
	// we only need to compare timestamps.
	for (int resetCnt = firstReset; resetCnt <= lastReset; ++resetCnt)
	{
		// prepare statistics per reset:
		uint64_t singlesPerReset = eventCount;
		uint64_t coincsPerReset = coincOutput.GetCoincCount();
#if DELAYED_COINC_CHECK
		uint64_t randomsPerReset = delayedCoinc ? randomsOutput->GetCoincCount() : 0;
#endif
		TimeStamp	firstEventTime = 0;
		TimeStamp	lastEventTime = 0;
#if WIN32
		clock_t tStart = clock();
#else
		struct tms tmsStart;
		clock_t tStart = times(&tmsStart);
#endif

		// remove remainig elements from previous loop
		evtHeap.clear();
#if DELAYED_COINC_CHECK
		delayedEvtHeap.clear();
#endif

		// Fill the heap vector, read events, and build heap.
		for (unsigned i = 0; i < numFiles; ++i)
		{
			if (pipe[i].IsValid() && pipe[i].NextReset(resetCnt-1))
			{
				++eventCount;
				evtHeap.push_back(&pipe[i]);
				push_heap(evtHeap.begin(), evtHeap.end(), PipeCompare());
#if DELAYED_COINC_CHECK
				if (delayedCoinc && delayedPipe[i].IsValid() && delayedPipe[i].NextReset(resetCnt-1))
				{
					delayedEvtHeap.push_back(&(delayedPipe[i]));
					push_heap(delayedEvtHeap.begin(), delayedEvtHeap.end(), PipeCompare());
				}
#endif
			}
		}
		if (evtHeap.size() < 2)
		{
			// Not enough pipes left with unprocessed events, end sorting.
			break;
		}

		// load the angular data for this reset:
		if (!coincOutput.NextReset(resetCnt))
			break;
#if DELAYED_COINC_CHECK
		if (delayedCoinc && !randomsOutput->NextReset(resetCnt))
			break;
#endif

		// pipe processing loop
		for (;;)
		{
//			if (lastEventTime > evtHeap.front()->GetTimeStamp())
//			{
//				printf("Error: event timestamp is less than previous: %llx < %llx\n",
//					evtHeap.front()->GetTimeStamp(), lastEventTime);
//			}
			lastEventTime  = evtHeap.front()->GetTimeStamp();
			if (0 == firstEventTime)
				firstEventTime = lastEventTime;

#if DELAYED_COINC_CHECK
			if (delayedCoinc)
			{
				// Note: here we use the following properties of the EventPipes:
				// - Because of the delay, the events in delayedEvtHeap will be processed
				//  a little bit after those in evtHeap. Therefore, if evtHeap contains enough
				//  pipes for sorting, delayedEvtHeap can be sorted, too.
				// - If all events in evtHeap are processed, there still may be some events
				//  in delayedEvtHeap.
				// - We must process the events in time order, otherwise it might happen that
				//  delayedEvtHeap members get empty.
				if (evtHeap.size() > 1)
				{
					if (evtHeap.front()->GetTimeStamp() <= delayedEvtHeap.front()->GetTimeStamp())
						sortCycle(evtHeap, coincOutput, eventCount);
					else
						sortCycle(delayedEvtHeap, *randomsOutput, countDummy);
				}
				else if (delayedEvtHeap.size() > 1)
					sortCycle(delayedEvtHeap, *randomsOutput, countDummy);
				else
					break;
			}
			else
#endif
			{
				if (evtHeap.size() <= 1)
					break;
				sortCycle(evtHeap, coincOutput, eventCount);
			}
		} // while (1)

		singlesPerReset = eventCount - singlesPerReset;
		coincsPerReset = coincOutput.GetCoincCount() - coincsPerReset;
		double t = timeStampToS(lastEventTime-firstEventTime); // measurement time of this reset in seconds
		totalDAQTime += t;

#if DELAYED_COINC_CHECK
		if (delayedCoinc)
			randomsPerReset = randomsOutput->GetCoincCount() - randomsPerReset;
#endif
		double percent;
		if (lastReset != INT_MAX-1)
			percent = double(resetCnt) / double(lastReset-firstReset+1) *100.0;
		else
			percent = double(eventCount) / double(totalEventsInFiles) *100.0;
		cout.setf(ios_base::fixed, ios_base::floatfield);
		if (!IsSilent())
		{
			cout.precision(2);
			cout << "processing for reset frame #" << resetCnt << " done.\n";
			cout << "  reset frame duration: " << t << "s\n";
			cout << "  singles: " << singlesPerReset << " = " << singlesPerReset/t << "/s\n";
			cout << "  coincs : " << coincsPerReset << " = " << coincsPerReset/t << "/s\n";
#if WIN32
			clock_t tEnd = clock();
			cout << "  Processing time: " << double(tEnd-tStart)/CLOCKS_PER_SEC << "s\n";
#else
			struct tms tmsEnd;
			clock_t tEnd = times(&tmsEnd);
			cout << "  Processing time: " << double(tEnd-tStart)/clock_ticks << "s / ";
			cout << double(tmsEnd.tms_utime-tmsStart.tms_utime)/clock_ticks << "s / ";
			cout << double(tmsEnd.tms_stime-tmsStart.tms_stime)/clock_ticks << "s\n";
#endif
#if DELAYED_COINC_CHECK
			if (delayedCoinc)
				cout << "  randoms: " << randomsPerReset << " = " << randomsPerReset/t << "/s\n";
#endif
		}
		cout.precision(1);
		cout << percent << "% complete\n";
		cout.setf(ios_base::scientific, ios_base::floatfield);
		cout.precision(6);
		cout.flush();
	} // for (resetCnt)

	coincCount = coincOutput.GetCoincCount();
	coincCountWritten = coincOutput.GetCoincCountWritten();
#if DELAYED_COINC_CHECK
	if (delayedCoinc)
		randomsCount = randomsOutput->GetCoincCount();
	else
		randomsCount = -1;
#endif

    WriteCCH(outputFile);
    if (delayedCoinc)
	    WriteCCH(outputFile + "_randoms");
}

void Sorter::Report(std::ostream& os)
{
	using namespace std;

	os << "Sorter Report:";
	os << "\n  Energy window: " << 5*minEnergy << ".." << 5*maxEnergy << "keV";
	if (allowed.GetFOV())
		os << "\n  Field of view: " << allowed.GetFOV() << " opposite cassettes";
	else
		os << "\n  Field of view: " << " loaded from matrix file";
	os << "\n  Coincidence window: " << timeStampToNs(coincWindow) << "ns";
#if DELAYED_COINC_CHECK
	if (randomDelay > 0)
		os << "\n  Delayed random coincidence check delay: " << timeStampToNs(randomDelay) << "ns";
#endif
	os.setf(ios_base::fixed, ios_base::floatfield);
	os.precision(2);
	os << "\n  Scanner inner radius: " << innerRadius << "mm";
	os << "\n\n  Total DAQ time: " << totalDAQTime << "s";
	os << "\n  Number of events processed: " << eventCount;
	os << "\n  Number of coincidences: " << coincCount;
	os << "\n  Number of coincidences written to LMF: " << coincCountWritten;
	if (eventCount > 0)
		os << "\n  Coincidence rate: " << setprecision(3) << (2*100.0*coincCount)/eventCount << "%";
#if DELAYED_COINC_CHECK
	if (randomDelay)
	{
		os << "\n  Number of random coincidences: " << randomsCount;
		if (eventCount > 0)
		{
			os.precision(1);
			os << "\n  Random coincidence rate: " << (2*100.0*randomsCount)/eventCount << "%";
			if (coincCount)
				os << " (=" << (100.0*randomsCount)/coincCount << "% of all coincs)";
		}
	}
#endif
	os << endl;
	os.setf(ios_base::scientific, ios_base::floatfield);
	os.precision(6);

	// Note for concidence rate calculation:
	// If all events are coincident, there are exactly twice as much events
	// as coincidences. Therefore the factor 2 in the formula.
}


bool Sorter::WriteCCH(const std::string& filename)
{
	std::string name(filename);
	name += ".cch";

	std::ofstream os(name.c_str());
	if (!os)
	{
		std::cout << "Error creating CCH file " << name << std::endl;
		return false;
	}

// Note: the following parameters match all installations except the first
// installation at Lyon. For that first installation, the macro GEOMETRY_LYON
// must be set.
#if GEOMETRY_LYON
    const double rsector_axial_size = 11.16;
    const double module_axial_pitch = 2.85;
#else
    const double rsector_axial_size = 11.11;
    const double module_axial_pitch = 2.76;
#endif

    os << "geometrical design type: 1\n";
	os << "ring diameter: " << (innerRadius/10) << " cm\n";
	os << "rsector axial pitch: 0 cm\n";
	os << "rsector azimuthal pitch: 18 degree\n";
	os << "rsector tangential size: 1.91 cm\n";
	os << "rsector axial size: " << rsector_axial_size << " cm\n";
	os << "module axial size: 1.91 cm\n";
	os << "module tangential size: 1.91 cm\n";
	os << "module axial pitch: " << module_axial_pitch << " cm\n";
	os << "module tangential pitch: 0 cm\n";
	os << "submodule axial size: 1.91 cm\n";
	os << "submodule tangential size: 1.91 cm\n";
	os << "submodule axial pitch: 0.0 mm\n";
	os << "submodule tangential pitch: 0 cm\n";
	os << "crystal axial size: 0.2 cm\n";
	os << "crystal tangential size: 0.2 cm\n";
	os << "crystal radial size: 2.0 cm\n";
	os << "crystal axial pitch: 0.23 cm\n";
	os << "crystal tangential pitch: 0.23 cm\n";
	os << "layer0 radial size: 1.0 cm\n";
	os << "in layer0 interaction length: 5 mm\n";
	os << "layer1 radial size: 1.0 cm\n";
	os << "in layer1 interaction length: 5 mm\n";
	os << "clock time step: " << (25000.0/64) << " ps\n";
	os << "azimuthal step: -0.1 degree\n";
#if GEOMETRY_LYON
	os << "z shift sector 1 mod 2: 7 mm\n";
#else
	os << "z shift sector 0 mod 2: 9.2 mm\n";
#endif
	os << "axial step: 0 cm\n";
	os << "scan file name: " << filename << "\n";

	if (!os)
	{
		std::cout << "Error writing CCH file " << name << std::endl;
		return false;
	}

	return true;
}
