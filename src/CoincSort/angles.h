/// \file	angles.h
/// \brief	Declaration of class AngularPositions
/// \author	Axel Wagner
///
#ifndef angles_h_included
#define angles_h_included

#include <fstream>
#include <vector>

#include "inttypes.h"
#include "noncopy.h"

///\brief Interface to angular encoder files
///
/// This class handles the interconnection of timestamps and angular positions.
/// These data are loaded from an encoder file and cached in this class.
/// The data are sorted by timestamp. The typical operation for this class
/// is to find an angle for a given timestamp. Normally, there will be no
/// exact match, and a linear interpolation will give approximate results.
///
/// Typically, the list will be processed in increasing time steps, which
/// are often smaller than the time steps in the list. Therefore, the class
/// remembers the last hit, which has a big chance to be a hit in the next
/// call.
///
class AngularPositions: private noncopyable {
public:
    typedef uint64_t time_type;
	typedef double angle_type;

	AngularPositions(const char* fileName);

	bool		readAngles(int resetCnt);

	angle_type	findAngle(time_type timeStamp);

    uint32_t		GetTimeSinceStart() const { return timeSinceStart; }

private:

	///\brief Data type which combines timestamp and angle
	struct data_type {
		time_type	timeStamp;
		angle_type	angle;

		data_type(time_type t=0): timeStamp(t), angle(0.0) {}

		/// The data will be sorted by time, and this is the comparison operator
		friend bool operator < (const data_type& t1, const data_type& t2)
		{
			return t1.timeStamp < t2.timeStamp;
		}
	};

	typedef std::vector<data_type> list_type;
	typedef list_type::const_iterator hitpos_type;

	std::ifstream	file;			///< the opened encoder file
	list_type		data;			///< list of cached data, filled by readAngles()
	hitpos_type 	lastHit;		///< points to last element in list which matched, or data.end() if no match exists.
	hitpos_type 	lastHitPrev;	///< points to the element before lastHit
	int				lastReset;		///< number of current reset frame
    uint32_t		timeSinceStart;		///< time since start of DAQ at beginning of current reset frame, in ms
	uint32_t		nextTimeSinceStart;	///< time since start of DAQ at beginning of next reset frame, in ms
};

#endif // angles_h_included
