/// \file	angles.cpp
/// \brief	Implementation of class AngularPositions
/// \author	Axel Wagner
///
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cerrno>

#include "angles.h"

///\brief Initialization
///
/// This ctor opens the angular encoder, but does not yet read any data from it.
/// The object will not operate correctly until readAngles() has been called.
///
/// \param fileName	The name of the angular encoder file.
/// \throw std::runtime_error if the file cannot be opened.
///
AngularPositions::AngularPositions(const char* fileName)
:	file(fileName),
	lastReset(-1),
	timeSinceStart(0),
	nextTimeSinceStart(0)
{
	using namespace std;

	if (!file)
	{
		switch (errno)
		{
			case ENOENT:
				throw runtime_error(string("File not found: ") + fileName);
			case EACCES:
				throw runtime_error(string("Access denied to file ") + fileName);
			default:
				throw runtime_error(string("Error opening file ") + fileName);
		}
	}

	lastHit = data.end();
}

///\brief Read data for one reset frame into the cache
///
/// This function read all angular data for the given reset frame. The reset
/// count must be larger than the previously processed reset.
///
/// \param resetCnt The reset to be read
/// \throw std::runtime_error on file read errors, or if the reset count is
///   invalid.
///
bool AngularPositions::readAngles(int resetCnt)
{
	using namespace std;

	data.clear();

	if (lastReset > resetCnt)
	{
		// we want to read resets in increasing order!
		ostringstream os;
		os << "Error: request to read angle for reset #" << resetCnt;
		os << "\nbut previous reset was " << lastReset;
		throw runtime_error(os.str());
	}

	data_type	tmp;
	int			reset;

	timeSinceStart = nextTimeSinceStart;

	// loop until we find the right reset marker
	while (lastReset < resetCnt)
	{
		file >> reset >> tmp.timeStamp >> tmp.angle;
		if (!file)
			throw runtime_error("Error reading encoder file");
		if (0 == reset)
			// store the elapsed time since start of measurement (in ms)
            timeSinceStart = uint32_t(tmp.angle * 1000);
		if (0 == tmp.timeStamp)
		{
			// reset marker found
			lastReset = reset;
			data.push_back(tmp);
		}
	}

	// loop to read the angular data
	for (;;)
	{
		file >> reset >> tmp.timeStamp >> tmp.angle;
		if (!file)
		{
			// the terminating "0 0 0" marker is missing in some files.
			// We tolerate that if the reset frame seems to contain something.
			if (data.size() < 10 || (data.back().angle >= 0.0 && data.back().angle <= 360.0))
				throw runtime_error("Error reading encoder file");
			return true;
		}
		if (0 == tmp.timeStamp)
		{
			// reset marker found
			// Note: there is a "0 0 0" marker at the end of the file. We must read and
			// ignore it.
			if (0 != reset)
				lastReset = reset;
			else
				// The "time since start" which we have read in this reset record
				// belongs to the next reset. We store it for later use.
                nextTimeSinceStart = uint32_t(tmp.angle * 1000);
			break;
		}
		if (reset != lastReset)
		{
			ostringstream os;
			os << "Data error in encoder file:\n";
			os << "  Expected reset #" << lastReset << ", found reset #" << reset;
			throw runtime_error(os.str());
		}
		tmp.timeStamp *= 64;
		data.push_back(tmp);
	}

	lastHit = data.end();
	return true;
}

///\brief Find the angle for a given timestamp
///
/// This function interpolates the angle for a timestamp. It first tries in the
/// interval given by lastHitPrev and lastHit. If that fails, it retries by
/// incrementing the interval by one position. If that fails, too, it does
/// a binary search on the whole data set.
/// This optimization works fast if the timestamps are used in increasing
/// time order, with small increments. That is the normal case in the
/// coincidence sorting.
///
/// \param timeStamp The time for which the angle has to be found
/// \return the interpolated angle for the time stamp, or -1 if the timestamp
///  cannot be found.
///
/// \note The error handling with a special return value is preferred here
/// against exceptions, because the caller has to check the angular range
/// anyway, and therefore does not need special handling.
///
AngularPositions::angle_type AngularPositions::findAngle(AngularPositions::time_type timeStamp)
{
	bool found = false;
	if (lastHit != data.end())
	{
		// try to optimize by using last hit
		found = lastHit->timeStamp > timeStamp && lastHitPrev->timeStamp < timeStamp;
		if (!found && lastHit->timeStamp <timeStamp)
		{
			// failed, try again with advancing hit pos by one
			lastHitPrev = lastHit;
			lastHit++;
			if (lastHit == data.end())
				return -1;
			found = lastHit->timeStamp > timeStamp && lastHitPrev->timeStamp < timeStamp;
		}
	}

	if (!found)
	{
		// Last hit checking failed, we do it the hard way by binary search
		lastHit = std::lower_bound(data.begin(), data.end(), data_type(timeStamp));
		// Note: lower_bound() returns an iterator to the first element which is equal or larger than
		// the given value.

		if (lastHit == data.end() || lastHit == data.begin())
			// not found!
			return -1;

		lastHitPrev = lastHit - 1;
	}

	// Finally, do the interpolation
	return lastHitPrev->angle + ((lastHit->angle - lastHitPrev->angle) / (lastHit->timeStamp - lastHitPrev->timeStamp)) * (timeStamp - lastHitPrev->timeStamp);
}
