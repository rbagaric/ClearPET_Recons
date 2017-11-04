/**
 *\file		crystals_main.cpp
 *\brief	A little tool to list all crystal positions in the clearpet scanner
 *\author	Axel Wagner
 *
 * The program expects a file named "Coincidences.cch" with the geometrical
 * description of the scanner. Using this file, the radius or the sector shift
 * of the system can be adjusted.
 */
#include <iostream>

#include "geometry.h"

#if _MSC_VER
int __cdecl main(int argc, char **argv )
#else
int main(int argc, char **argv )
#endif
{
	using namespace std;
	using namespace ClearPET;

	Geometry geometry("Coincidences.cch");
	CoincEvent evt;
	evt.angle = 0;
	for (int sector = 0; sector < numSectors; ++sector)
		for (int module = 0; module < numModules; ++module)
			for (int pixel = 0; pixel< numPixels; ++pixel)
			{
				evt.id1 = (sector << CoincEvent::shiftSectors)
					+ (module << CoincEvent::shiftModules)
					+ (pixel << CoincEvent::shiftPixels);
				evt.id2 = evt.id1 + 1;	// Layer1
				LOR lor = geometry.GetLOR(evt);
				cout << evt.id1 << '\t' << lor.p1.x << '\t' << lor.p1.y << '\t' << lor.p1.z << '\n';
				cout << evt.id2 << '\t' << lor.p2.x << '\t' << lor.p2.y << '\t' << lor.p2.z << '\n';
			}

	return 0;
}
