///\file	geometry.cpp
///\brief	Implementation of class Geometry
///\author	Axel Wagner

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "geometry.h"

const double pi = 4.0*std::atan(1.0);
const double degToRad = pi / 180.0;

typedef std::vector<std::string> StringList;

enum ValueType
{
	ValDistance,	///< value is a size or distance, unit is mm
	ValAngle,		///< value is an angle, unit is rad
	ValNumber		///< value without dimension
};

class IsValueName: public std::unary_function<std::string, bool>
{
public:
	IsValueName(const std::string& theName): name(theName) {}

	bool operator()(const std::string& line)
	{
		return 0 == line.find(name);
	}
private:
	std::string name;
};

///\brief Find a geometry description value in string list
///
///\param	list	the string list, loaded from geometry file
///\param	name	name of the description item, including colon
///\param	valType	type of the value
///
///\return the value in rad if it is an angle, otherwise in mm
///\throw	std::runtime_error if the value is not found  or could not be decoed
template <class T>
T GetGeometryValue(const StringList& list, const std::string& name, ValueType valType)
{
	using namespace std;

	StringList::const_iterator i = std::find_if(list.begin(), list.end(), IsValueName(name));
	if (i == list.end())
		throw std::runtime_error(name + " not found in geometry file");

	T value;
	std::string unit;

	std::istringstream is(*i);
	is.seekg(name.length());	// skip name of entry
	is >> value;
	if (valType != ValNumber)
		// value type requires unit, read it
		is >> unit;
	if (is.fail())
		throw std::runtime_error(string("Invalid data in geometry file, line:\n")+*i);

	if (valType == ValAngle)
	{
		// Angular values: we accept degree only. Values are returned in rad
		if (unit != "degree")
			throw std::runtime_error(string("Invalid unit in geometry file, line:\n")+*i);
		value = static_cast<T>(value*degToRad);	// just to keep compiler silent
	} else if (valType == ValDistance)
	{
		// Distance values: we accept cm or mm. Values are returned in mm
		if (unit == "cm")
			value *= 10;
		else if (unit != "mm")
			throw std::runtime_error(string("Invalid unit in geometry file, line:\n")+*i);
	}

	return value;
}

Geometry::Geometry(const std::string& geomFile)
{
	using namespace std;

	ifstream file(geomFile.c_str());
	if (!file)
		throw runtime_error(string("Error opening scanner geometry file ")+geomFile);

	// read the lines of the geometry file into a buffer:
	StringList geomText;
	while (!file.eof())
	{
		string line;
		getline(file, line);
		// find start of text in line (skips white space)
		size_t pos = line.find_first_not_of(" \t");
		if (pos != string::npos)
		{
			if (pos != 0)
				line.erase(0, pos);
			geomText.push_back(line);
		}
	}

	if (1 != GetGeometryValue<int>(geomText, "geometrical design type:", ValNumber))
		throw runtime_error("Invalid geometrical design type");

	// inner radius of scanner
	radius = 0.5 * GetGeometryValue<double>(geomText, "ring diameter:", ValDistance);
	if (radius < 50 || radius > 150)
		throw runtime_error("Invalid ring diameter");
	// axial/tangential size of crystal
	double crystalSize = GetGeometryValue<double>(geomText, "crystal axial size:", ValDistance);
	if (fabs(crystalSize-2.0) > 0.1)
		throw runtime_error("Invalid crystal size");
	if (crystalSize != GetGeometryValue<double>(geomText, "crystal tangential size:", ValDistance))
		throw runtime_error("Invalid non-square crystal size");
	// radial size of crystal
	crystalDepth = GetGeometryValue<double>(geomText, "crystal radial size:", ValDistance);
	if (fabs(crystalDepth-20.0) > 0.5)
		throw runtime_error("Invalid crystal depth");
	// note: the geometry file gives the total thickness of both layers,
	// but we need the thickness of one layer:
	crystalDepth /= 2.0;
	// note: the geometry file gives the total thickness of both layers,
	// but we need the thickness of one layer:
	// axial/tangential crystal pitch
	crystalPitch = GetGeometryValue<double>(geomText, "crystal axial pitch:", ValDistance);
	if (fabs(crystalPitch-2.3) > 5.0)
		throw runtime_error("Invalid crystal pitch");
	if (crystalPitch != GetGeometryValue<double>(geomText, "crystal tangential pitch:", ValDistance))
		throw runtime_error("Invalid non-square crystal pitch");
	// axial module pitch
	modulePitch = GetGeometryValue<double>(geomText, "module axial pitch:", ValDistance);
	if (fabs(modulePitch-27.6) > 1)
		throw runtime_error("Invalid module pitch");
	// sector azitmuthal pitch
	sectorPitch = GetGeometryValue<double>(geomText, "rsector azimuthal pitch:", ValAngle);
	if (fabs(sectorPitch-18.0*degToRad) > 1)
		throw runtime_error("Invalid rsector pitch");
	azimuthalStep = GetGeometryValue<double>(geomText, "azimuthal step:", ValAngle);
	if (fabs(azimuthalStep) > 0.5*degToRad)
		throw runtime_error("azimuthal step");

	double shift;
	bool oddShift = false;
	StringList::const_iterator i = std::find_if(geomText.begin(), geomText.end(), IsValueName("z shift sector 0 mod 2:"));
	if (i == geomText.end())
	{
		shift = GetGeometryValue<double>(geomText, "z shift sector 1 mod 2:", ValDistance);
		oddShift = true;
	} else
		shift = GetGeometryValue<double>(geomText, "z shift sector 0 mod 2:", ValDistance);
	if (shift < 5 || shift > 10)
		throw runtime_error("Invalid rsector shift");
	axialSectorShift[(int)!oddShift] = 0.0;
	axialSectorShift[(int)oddShift] = shift;

	// Precalculate the positions of the 64 pixels of one PMT. Later,
	// the axial position needs to be adjusted for module and sector shift,
	// the radial position for layer shift and the angular position for
	// sector pitch and gantry rotation.
	for (int i = 0; i < ClearPET::numPixels; ++i)
	{
		const double pixelOffset = (ClearPET::numPixelsTangential-1) / 2.0; 
		pixelPos[i].z = (i / ClearPET::numPixelsAxial) * crystalPitch;
		pixelPos[i].x = radius + crystalDepth/2.0; // adjust radius for center of crystal layer 0
		pixelPos[i].y = ((i % ClearPET::numPixelsTangential) - pixelOffset) * crystalPitch;
	}
}

LOR Geometry::GetLOR(CoincEvent& coincEvent)
{
	LOR result;

	// calculate positions a with given rotation
	// Note that we add pi/2 to the rotation to get into the
	// coordinate system which is compatible with the old rebinner 
	double phi = coincEvent.angle * azimuthalStep - pi/2.0;
	result.p1 = calculateCrystalPosition(coincEvent.id1, phi);
	result.p2 = calculateCrystalPosition(coincEvent.id2, phi);

	return result;
}

///\brief Calculate a crystal position in the scanner
///
/// This function takes a crystal id and calculates the position of the
/// crystal center.
///
///\param	id	crystal id
///\param	phi	rotation of gantry, in rad
///\return	The coordinates of the crystal center, in cartesian system
///\throw	std::runtime_error if an invalid crystal is detected
///
CartCoord Geometry::calculateCrystalPosition(uint16_t id, double phi)
{
	// decode the id into structure numbers:
	int	layer = (id & CoincEvent::maskLayers) >> CoincEvent::shiftLayers;
	int pixel = (id & CoincEvent::maskPixels) >> CoincEvent::shiftPixels;
	int subMod = (id & CoincEvent::maskSubMods) >> CoincEvent::shiftSubMods;
	int mod = (id & CoincEvent::maskModules) >> CoincEvent::shiftModules;
	int sector = (id & CoincEvent::maskSectors) >> CoincEvent::shiftSectors;

	if (subMod != 0 || sector > ClearPET::numSectors)
		throw std::runtime_error("Invalid crystal id");

	// get the pixel position from precalculated table:
	CartCoord result = pixelPos[pixel];

	// adjust radius for second layer:
	if (layer)
		result.x += crystalDepth;
	// adjust angle for sector index:
	phi += sector * sectorPitch;
	// rotate x/y coordinates:
	double x = result.x;
	result.x = x*cos(phi)-result.y*sin(phi);
	result.y = result.y*cos(phi)+x*sin(phi);
	// adjust axial position for module pitch and sector shifts:
	result.z += mod * modulePitch;
	result.z += axialSectorShift[sector & 1];

	return result;
}
