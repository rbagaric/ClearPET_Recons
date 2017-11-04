/// \file	evtpipe.cpp
/// \brief	Implementation of classes EventBuffer and EventPipe
/// \author	Axel Wagner
///
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>	// std::mem_fun
#include <cstring>	// std::memcpy
#include "evtpipe.h"

EventBuffer::~EventBuffer()
{
	using namespace std;

	for_each(bufferUsers.begin(), bufferUsers.end(), mem_fun(&EventPipe::SaveBuffer));
};

void EventBuffer::RegisterUser(EventPipe* p)
{
	bufferUsers.push_back(p);
}

void EventBuffer::UnregisterUser(EventPipe* p)
{
	list_type::iterator i = std::find(bufferUsers.begin(), bufferUsers.end(), p);
	if (i != bufferUsers.end())
		bufferUsers.erase(i);
}

/// \brief Load new block of data
///
/// This function is called by one of the buffer users, when it has processed
/// the buffer contents and requires a new block of data.
///
/// The function first notifies all buffer users (including the caller) that
/// the buffer will be destroyed, by calling their SaveBuffer() method. The
/// buffer users will create a local copy of all unprocessed events.
///  Then it calls the virtual function Refill(), which must be implemented
/// in subclasses to do the real work of loading new data.
///  If successful, all buffer users are notified about the new data by
/// calling their BufferRefilled() method.
///
/// \return	true on success, false if the reload failed
///
bool EventBuffer::NextBlock()
{
	using namespace std;

	for_each(bufferUsers.begin(), bufferUsers.end(), mem_fun(&EventPipe::SaveBuffer));

	if (!Refill())
		return false;

	for (list_type::iterator i = bufferUsers.begin(); i != bufferUsers.end(); ++i)
		(*i)->BufferRefilled(buffer, bufferEnd);

	return true;
}


EventPipe::EventPipe()
:	eventBuffer(0)
,	resetCnt(-1)
,	localCopy(0)
,	bufferPtr(0), bufferEnd(0)
,	randomDelay(0)
#if INPUT_32BIT
,	previousTime(0)
,	modCount(0)
#endif
{
}

EventPipe::~EventPipe()
{
	if (eventBuffer)
		eventBuffer->UnregisterUser(this);
	delete[] localCopy;
}

bool EventPipe::IsValid() const
{
	if (!eventBuffer)
		return false;
	else
		return eventBuffer->IsValid();
}

void EventPipe::AssignBuffer(EventBuffer* aBuffer)
{
	if (eventBuffer == aBuffer)
		return;

	if (eventBuffer)
		eventBuffer->UnregisterUser(this);
	eventBuffer = aBuffer;
	if (aBuffer)
		aBuffer->RegisterUser(this);
}

// This function is called by the EventPipe which maintains the buffer,
// just before it overwrites or destroys the buffer. The EventPipeDelayed
// object copies the unprocessed event into a local buffer.
void EventPipe::SaveBuffer()
{
	if (localCopy)
	{
		if (bufferPtr != bufferEnd)
			std::cout << "Warning: data overflow in event pipe for sector " << eventData.GetModule() << std::endl;
		delete[] localCopy;
		localCopy = 0;
		bufferPtr = refilledPtr;
		bufferEnd = refilledEnd;
	}
	if (bufferPtr < bufferEnd) {
    	int unprocessed = bufferEnd - bufferPtr;
//		std::cout << "Allocating local buffer for " << unprocessed << " events in event pipe for sector " << eventData.GetModule() << std::endl;
		localCopy = new BinaryEvent[unprocessed];
		std::memcpy(localCopy, bufferPtr, unprocessed * sizeof(BinaryEvent));
		bufferPtr = localCopy;
		bufferEnd = localCopy + unprocessed;
	}
	refilledPtr = refilledEnd = 0;
}

// This function is called by the EventPipe which maintains the buffer,
// just after it has (re)filled the buffer. The EventPipeDelayed
// updates its buffer pointers.
void EventPipe::BufferRefilled(BinaryEvent* bufStart, BinaryEvent* bufEnd)
{
	if (localCopy) {
		if (refilledPtr)
			std::cout << "Warning: data overflow in event pipe for sector " << eventData.GetModule() << std::endl;
		refilledPtr = bufStart;
		refilledEnd = bufEnd;
	} else {
		bufferPtr = bufStart;
		bufferEnd = bufEnd;
	}
}

#if INPUT_32BIT
const uint32_t SpecialWordMarker = 0x80000000;
const uint32_t SpecialWordMask = 0x0FFFFFFF;
const uint32_t ResetMarker = 0x8;
const uint32_t BridgeMarker = 0x9;
const uint32_t ModInfoMarker = 0xA;
const uint32_t TimeStampMask = 0x000FFFFF;
#else
const unsigned int ResetMarker = 0xFFF;
#endif

#if INPUT_32BIT
bool EventPipe::HandleSpecialWord(BinaryEvent sw)
{
	uint32_t type = sw >> 28; // extract marker type bits
	sw &= SpecialWordMask; // mask away the marker type bits
	switch (type)
	{
		case ResetMarker:
			// It is a reset word, we note that
			resetCnt = sw;
			previousTime = 0;
			// no more events for this reset frame
			return false;

		case BridgeMarker:
			// It is bridge word, we note that
			previousTime += sw;
			// there may be more events for this reset frame
			return true;

		case ModInfoMarker:
		{
			// It is a module info word
			modCount = (sw >> 26) + 1;
			for (int i = 0; i < modCount; ++i)
			{
				modules[i] = sw & 0x3F;
				sw >>= 6;
			}
			// if this event is in the middle of the data stream, it's an error
			return false;
		}

		default:
			// other words are not yet defined, we ignore them
			return true;
	}
}
#endif

#if INPUT_32BIT
bool EventPipe::LoadModuleInfo()
{
	(void)NextEvent();
	return modCount != 0;
}
#endif

bool EventPipe::NextEvent()
{
	using namespace std;

#if 0
	if (resetCnt > reset)
		// no more events for that reset count
		return false;
#endif

	for (;;) {
		if (bufferPtr == bufferEnd)
		{
			if (localCopy) {
				// we were working on a local copy, switch back to shared buffer
				delete[] localCopy;
				localCopy = 0;
				bufferPtr = refilledPtr;
				bufferEnd = refilledEnd;
			}
			if (bufferPtr == bufferEnd && !eventBuffer->NextBlock())
				// failed to get new block
				return false;
		}
		BinaryEvent tmp = *bufferPtr++;
#if INPUT_32BIT
		if (tmp & SpecialWordMarker)
		{
			if (!HandleSpecialWord(tmp))
				return false;
		} else
		{
			// Decode event data using the EventData ctor
			eventData = EventData(tmp);
			eventData.FixModule(modules);
			eventData.AddTime(previousTime + randomDelay*Sector());
			previousTime += tmp & TimeStampMask;
			return true;
		}
#else
		if (tmp < ResetMarker) {
			// It was a reset marker, we note that
			int r = int(tmp & 0xFFF);
			resetCnt = r;
			// no more events for this reset frame
			return false;
		} else
		{
			// Decode event data using the EventData ctor
			eventData = EventData(tmp);
			eventData.AddTime(randomDelay*Sector());
			return true;
		}
#endif
	}
}

bool EventPipe::NextReset(int reset)
{
	using namespace std;

	if (resetCnt > reset)
	{
		// no more events for that reset count
		return false;
	}

    if (resetCnt == reset)
	{
		// we already have read this reset marker, get next event
        return NextEvent();
	}

    if (-1 == resetCnt && reset == 0)
    {
        // it's the first reset frame, and many singles file do not have an initial reset marker
        if (NextEvent())
        {
            resetCnt = 0;
            return true;
        }
        // NextEvent() found a reset marker, or error
        if (-1 == resetCnt)
            return false;   // was an error in NextEvent
        else
            return NextEvent(); // was a reset marker, get and decode next event
    }

	for (;;) {
		if (bufferPtr == bufferEnd)
		{
			if (localCopy) {
				// we were working on a local copy, switch back to shared buffer
				delete[] localCopy;
				localCopy = 0;
				bufferPtr = refilledPtr;
				bufferEnd = refilledEnd;
			}
			if (bufferPtr == bufferEnd && !eventBuffer->NextBlock())
				// failed to get new block
				return false;
		}
		BinaryEvent tmp = *bufferPtr++;
#if INPUT_32BIT
		if ((tmp >> 28) == ResetMarker)
		{
			HandleSpecialWord(tmp);
#else
		if (tmp < ResetMarker)
		{
			// It was a reset marker, we note that and load the next event
			int r = int(tmp & 0xFFF);
			resetCnt = r;
#endif
			if (resetCnt > reset)
			{
				// no more events for that reset count
				return false;
			}
			else if (resetCnt == reset)
                // found the reset, decode the next event
                return NextEvent();
		}
	}
}

