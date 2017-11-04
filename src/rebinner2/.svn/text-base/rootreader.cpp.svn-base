/// \file	RootReader.cpp
/// \brief	Implementation of class RootReader
/// \author	Axel Wagner
///

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <string>

#include "clearpet.h"
#include "coincevent.h"
#include "rootreader.h"

RootReader::RootReader(const char* pFileName)
:	file(pFileName, std::ios::in|std::ios::binary)
{
    if (!file)
        throw std::runtime_error(std::string("error opening ") + pFileName);
}

bool RootReader::ReadEvent(CoincEvent& coincEvent)
{
	uint16_t	data[3];
	if (!file.read(reinterpret_cast<char*>(data), 6))
		return false;
	coincEvent.angle = data[0];
	coincEvent.id1 = data[1];
	coincEvent.id2 = data[2];
	// Gate/Root use inverted rotation. We just invert the rotation angle here:
	while (coincEvent.angle > 3600)
		coincEvent.angle -= 3600;
	coincEvent.angle = 3600 - coincEvent.angle;
	return true;
}
