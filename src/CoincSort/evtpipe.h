/// \file	evtpipe.cpp
/// \brief	Declaration of class AngularPositions
/// \author	Axel Wagner
///
///\page about_event_buffers About Event buffering
///
/// In the coincidence sorter, we sometimes run two sorters at the same
/// time. One is the real sorter, the other one a random coincidence sorter
/// which uses delayed time stamps. Both sorters use nearly the same data,
/// just with some delay. It would be waste of time to load the data for
/// both sorters independently. But because of the time delay, a simple
/// buffering is not good enough. A more complicated buffer management
/// is needed.
///
/// The class EventBuffer declares the interface for such improved buffers.
/// It has the concept of "buffer users". Each buffer user has to register
/// itself before using the buffer, and unregister if the usage is over.
/// It will receive a callback when the buffer needs to be refilled. In this
/// callback, it must copy all unprocessed events into a local buffer. This
/// concept allows to use a single large buffer, and a smaller second buffer
/// for unprocessed events.
///
/// The class EventPipe is the buffer user. It read events from the buffers
/// as needed, and saves the most recent event in a decoded format.
///
#ifndef evtpipe_h_included
#define evtpipe_h_included

#include <string>
#include <vector>

#include "noncopy.h"
#include "evttypes.h"
#include "filereader.h"

class EventPipe;

///\brief ABC for shared event buffer
///
/// This class represents a buffer of events, which is loaded
/// from some source, and which is used by several EventPipe
/// instances. The class manages the notifications to all buffer
/// users when the buffer needs to be refilled by one user.
///
class EventBuffer: private noncopyable {
public:
	virtual ~EventBuffer();

	// this function should return a name for the buffer, e.g. a file name
	virtual std::string GetName() const = 0;

	// Check if buffer source is valid
	virtual bool IsValid() const = 0;

	bool NextBlock();

	void	RegisterUser(EventPipe* p);
	void	UnregisterUser(EventPipe* p);

protected:
    BinaryEvent*		buffer;
	BinaryEvent*		bufferEnd;

	EventBuffer() {};

	virtual bool Refill() = 0;

private:
	typedef std::vector<EventPipe*> list_type;
	list_type	bufferUsers;
};


///\brief Event data stream
///
/// This class represents a pipe of binary events, from
/// whatever source.
///  It's main functions are reading, decoding and delivery
/// of event data. It also caches the most recently read event.
///
class EventPipe: private noncopyable {
public:
	EventPipe();
	~EventPipe();

	bool	NextReset(int reset);
	bool	NextEvent();

	int		GetReset() const { return resetCnt; }
	TimeStamp GetTimeStamp() const { return eventData.GetTimeStamp(); }
	int		Sector() const { return eventData.GetModule(); }
	int 	Energy() const { return eventData.GetEnergy(); }
	int 	Pixel() const { return eventData.GetPixel(); }
	int 	Neighbour() const { return eventData.GetNeighbour(); }
	int		Layer() const { return eventData.GetLayer(); }
	int 	PM() const { return eventData.GetPM(); }
	BinaryEvent GetBinaryData() const { return eventData.GetBinaryEvent(); }

	const EventData& GetEventData() const { return eventData; }

	void SetRandomDelay(int delay) { randomDelay = delay; }
	void AssignBuffer(EventBuffer* aBuffer);

	bool IsValid() const;
#if INPUT_32BIT
	bool LoadModuleInfo();
#endif

private:
	EventBuffer*	eventBuffer;
	int			resetCnt;
	EventData	eventData;

	BinaryEvent*		localCopy;
	BinaryEvent*		bufferPtr;
	BinaryEvent*		bufferEnd;
	BinaryEvent*		refilledPtr;
	BinaryEvent*		refilledEnd;
	int			randomDelay;

#if INPUT_32BIT
	TimeStamp	previousTime;
	int			modCount;
	int			modules[4];

	bool HandleSpecialWord(BinaryEvent sw);
#endif

	friend class EventBuffer;
	void	SaveBuffer();
	void	BufferRefilled(BinaryEvent* bufStart, BinaryEvent* bufEnd);
};

#endif // evtpipe_h_included
